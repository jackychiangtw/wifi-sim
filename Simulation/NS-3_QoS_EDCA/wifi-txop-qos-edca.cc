/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- *
 * wifi-txop-4QBSS.cc  ―  ns-3.40 可編譯版
 * ------------------------------------------------------------
 *   1 AP + 4 STA，每 STA 分別收 VO / VI / BE / BK 四種 AC 流量
 *   掃描 --start 到 --end (Mbps)，間隔 --step，輸出吞吐到 output_file.txt
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/wifi-module.h"
#include "ns3/qos-txop.h"          // 用於調整 EDCA 參數

using namespace ns3;
NS_LOG_COMPONENT_DEFINE ("WifiTxop4QBSS");

int
main (int argc, char *argv[])
{
  /* ---------- 1. 參數 ---------- */
  double offeredStart = 3.0;         // Mbps
  double offeredEnd   = 30.0;        // Mbps
  double offeredStep  = 3.0;         // Mbps
  double simTime      = 5.0;         // s (前 1 秒暖機)
  uint16_t port       = 5001;
  const uint32_t pktSize = 1472;     // bytes

  CommandLine cmd;
  cmd.AddValue ("start", "first offered load (Mbps)", offeredStart);
  cmd.AddValue ("end",   "last offered load (Mbps)",  offeredEnd);
  cmd.AddValue ("step",  "step of offered load (Mbps)", offeredStep);
  cmd.Parse (argc, argv);

  std::ofstream out ("output_file.txt");
  if (!out.is_open ())
    {
      NS_LOG_ERROR ("cannot open output_file.txt");
      return 1;
    }

  /* ---------- 2. 主迴圈：不同 offered load ---------- */
  for (double offered = offeredStart; offered <= offeredEnd + 1e-9; offered += offeredStep)
    {
      NS_LOG_INFO ("=== Offered traffic: " << offered << " Mbit/s ===");
      out << "OfferedTraffic " << offered << std::endl;

      /* 2.1 節點 */
      NodeContainer sta; sta.Create (4);
      NodeContainer ap;  ap.Create  (1);

      /* 2.2 Channel / PHY */
      YansWifiChannelHelper channel = YansWifiChannelHelper::Default(); // ← 關鍵修正
      YansWifiPhyHelper     phy;
      phy.SetChannel (channel.Create ());
      phy.SetPcapDataLinkType (WifiPhyHelper::DLT_IEEE802_11_RADIO);

      /* 2.3 MAC / Device */
      WifiHelper wifi;
      wifi.SetStandard (WIFI_STANDARD_80211a);
      wifi.SetRemoteStationManager ("ns3::IdealWifiManager");

      Ssid ssid = Ssid ("network");
      WifiMacHelper mac;

      mac.SetType ("ns3::StaWifiMac",
                   "QosSupported", BooleanValue (true),
                   "Ssid",         SsidValue (ssid));
      NetDeviceContainer staDev[4];
      for (uint32_t i = 0; i < 4; ++i)
        staDev[i] = wifi.Install (phy, mac, sta.Get (i));

      mac.SetType ("ns3::ApWifiMac",
                   "QosSupported",        BooleanValue (true),
                   "Ssid",               SsidValue (ssid),
                   "EnableBeaconJitter", BooleanValue (false));
      NetDeviceContainer apDev = wifi.Install (phy, mac, ap.Get (0));

      /* 2.4 AP 端 EDCA 參數 */
      Ptr<WifiMac> apMac = apDev.Get (0)->GetObject<WifiNetDevice> ()->GetMac ();
      PointerValue pv;

      auto setEdca = [&] (const std::string &name, uint32_t aifsn,
                          uint32_t cwMin, uint32_t cwMax)
        {
          apMac->GetAttribute (name, pv);
          Ptr<QosTxop> q = pv.Get<QosTxop> ();
          q->SetAifsn (aifsn);
          q->SetMinCw (cwMin);
          q->SetMaxCw (cwMax);
        };

      setEdca ("VO_Txop", 2, 3,    7);      // VO
      setEdca ("VI_Txop", 2, 7,   15);      // VI
      setEdca ("BE_Txop", 3, 15, 1023);     // BE
      setEdca ("BK_Txop", 7, 15, 1023);     // BK

      /* 2.5 Mobility */
      MobilityHelper mob;
      mob.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
      mob.Install (sta);
      mob.Install (ap);

      /* 2.6 Internet */
      InternetStackHelper stack;
      stack.Install (sta);
      stack.Install (ap);

      Ipv4AddressHelper ipv4;
      ipv4.SetBase ("10.0.0.0", "255.255.255.0");
      Ipv4InterfaceContainer ifSta[4];
      for (uint32_t i = 0; i < 4; ++i)
        ifSta[i] = ipv4.Assign (staDev[i]);
      ipv4.Assign (apDev);   // AP 不用記錄

      /* 2.7 UDP Server on STA */
      UdpServerHelper srv (port);
      ApplicationContainer srvApp[4];
      for (uint32_t i = 0; i < 4; ++i)
        {
          srvApp[i] = srv.Install (sta.Get (i));
          srvApp[i].Start (Seconds (0));
          srvApp[i].Stop  (Seconds (simTime - 2));  // 提前 2 秒關閉
        }

      /* 2.8 UDP Client on AP（OnOff） */
      auto mkClient = [&] (Ipv4Address dst, uint8_t dscp)
        {
          InetSocketAddress addr (dst, port);
          addr.SetTos (dscp);                       // DSCP 映到 EDCA
          OnOffHelper onoff ("ns3::UdpSocketFactory", addr);
          onoff.SetAttribute ("OnTime",
                              StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
          onoff.SetAttribute ("OffTime",
                              StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
          onoff.SetAttribute ("PacketSize", UintegerValue (pktSize));
          uint64_t bps = static_cast<uint64_t> (offered * 1e6);
          onoff.SetAttribute ("DataRate", DataRateValue (DataRate (bps)));
          ApplicationContainer app = onoff.Install (ap.Get (0));
          app.Start (Seconds (1.0));
          app.Stop  (Seconds (simTime));
        };

      mkClient (ifSta[0].GetAddress (0), 0xa0); // VI
      mkClient (ifSta[1].GetAddress (0), 0xc0); // VO
      mkClient (ifSta[2].GetAddress (0), 0x00); // BE
      mkClient (ifSta[3].GetAddress (0), 0x20); // BK

      /* 2.9 執行模擬 */
      Simulator::Stop (Seconds (simTime));
      Simulator::Run ();

      /* 2.10 吞吐計算 (Mbit/s) */
      auto rxPkt = [] (ApplicationContainer &a)
        { return a.Get (0)->GetObject<UdpServer> ()->GetReceived (); };

      double rx[4];
      for (uint32_t i = 0; i < 4; ++i) rx[i] = rxPkt (srvApp[i]);

      Simulator::Destroy ();

      auto tp = [&] (double r)
        { return (r * pktSize * 8.0) / ((simTime - 1.0) * 1e6); };

      double tpVI = tp (rx[0]),
             tpVO = tp (rx[1]),
             tpBE = tp (rx[2]),
             tpBK = tp (rx[3]);

      NS_LOG_INFO ("VI " << tpVI << "  VO " << tpVO
                   << "  BE " << tpBE << "  BK " << tpBK);

      out << "AC_VI " << tpVI << std::endl
          << "AC_VO " << tpVO << std::endl
          << "AC_BE " << tpBE << std::endl
          << "AC_BK " << tpBK << std::endl << std::endl;
    }

  out.close ();
  return 0;
}
