# NS3 802.11e QoS Supported EDCA Simulation

This simulation demonstrates the result of Analysis of paper [IEEE 802.11e for QoS support in wireless LANs](https://ieeexplore.ieee.org/document/1265851). This paper is explaned at [this note](https://github.com/jackychiangtw/wifi-sim/blob/main/802.11Background/802.11_Background_Knowledge.md)

## Background

To support QoS features, Enhanced Distributed Channel Access (EDCA) assigns different Arbitration inter-frame spacing (AIFS) and backoff (CWmin/max) to let high prority data is  transmitted earily. 

![paper_qos](https://github.com/jackychiangtw/wifi-sim/blob/main/Simulation/NS-3_QoS_EDCA/Paper_edca_qos_aifs_cw.png)

## Parameter

- Number of STAs: 4 type of QoS STA.
- Traffic:  `--start/--end/--step` sweep the offered load. 
    - `--start=3 --end=30 --step=3` from 3 → 30 Mb/s in 3 Mb/s steps.
- Simulation time: 5 s (0-1 s warm-up, 1-5 s measurement).
- UDP packet length= 1472 Byte.


## Run

```bash 
./ns3 run "wifi-txop-4QBSS --start=3 --end=30 --step=3"
```

## Result

- This Simulation's result
![simulation_result](https://github.com/jackychiangtw/wifi-sim/blob/main/Simulation/NS-3_QoS_EDCA/Simulation_result.png)

- Paper' result
![paper_result](https://github.com/jackychiangtw/wifi-sim/blob/main/Simulation/NS-3_QoS_EDCA/Paper_result.png)

## Discussion

The ns-3 simulation reproduces the 802.11e study: the four AC curves line up with the paper’s figure. This simulation successfully mirrors the paper’s results and confirms its core conclusion—EDCA guarantees voice/video dominance while best-effort and background traffic collapse at high load. 

This same framework can now be extended (e.g., by switching WifiHelper to 802.11ax/be and enabling UL-OFDMA or EHT features) to study next-generation Wi-Fi behaviour under identical traffic mixes.








