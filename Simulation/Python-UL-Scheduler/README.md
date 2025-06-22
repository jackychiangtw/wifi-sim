# 802.11 ac/ax/be – Multi-STA Toy UL-Scheduler Simulator

This python UL-Scheduler simulator compares 802.11ac (CSMA + TXOP), 802.11ax (OFDMA) and 802.11be (Multi-RU) under identical traffic loads.

It is not a bit‑accurate MAC/PHY, but a fast sandbox for “what‑if” sweeps on throughput, delay, fairness and spectral efficiency.

---

## 1  How It Works (20‑second overview)

```
Traffic  →  STA queues  →  Channel access
                               ├─ 11ac : CSMA/CA + optional TXOP
                               ├─ 11ax : 1‑RU OFDMA uplink scheduler
                               └─ 11be : multi‑RU OFDMA scheduler
                    ← stats (delay | goodput | spectral‑eff)
```

* Excuate the program by slots.
* STA generates CBR or burst traffic.
* The statistics includes **Avg Delay**, **Spectral Efficiency** and **Throughput**.
* Assume 0% BLER in the all transmission

---

## 2  Quick Start

```bash
python3 wifi_gen_sim.py
```

---

## 3  Command‑Line Arguments (grouped by purpose)

### 3.1  Wi‑Fi PHY / MAC Mechanics

| Argument                     | Meaning                                                             |
| ---------------------------- | ------------------------------------------------------------------- |
| `--phy {ac,ax,be}`           | choose one PHY (omit when using `--compare`)                        |
| `--mcs N`                    | HE/EHT MCS index (0‑11)                                             |
| `--nss {1,2}`                | spatial streams                                                     |
| `--gi {0.8,1.6,3.2}`         | guard interval (µs)                                                 |
| `--ru_bytes B`               | **override** bytes per baseline RU per slot                         |
| `--ru_count N`               | number of 26‑tone RUs forming baseline channel (default 9 = 20 MHz) |
| `--be_ru_multi N`            | max RUs one STA may grab in **be**                                  |
| `--ac_txop_slots N`          | TXOP length for **ac** (1 = disabled)                               |
| `--cw N`                     | initial contention‑window size for **ac**                           |
| `--allow_full_channel_alloc` | let ax/be give *all* RUs to a solo active STA                       |

### 3.2  Scheduler / Resource Allocation

| Argument           | Meaning                                    |
| ------------------ | ------------------------------------------ |
| `--sched {rr,pfs}` | round‑robin or proportional‑fair scheduler |
| `--pfs_tau ms`     | EWMA window used by PF                     |

### 3.3  Station / Topology

| Argument                       | Meaning                                |
| ------------------------------ | -------------------------------------- |
| `--n_sta N`                    | number of stations                     |
| `--n_sta_start / _end / _step` | sweep range when using `--compare_sta` |

### 3.4  Traffic / Load Model

| Argument             | Meaning                              |
| -------------------- | ------------------------------------ |
| `--arrival_rate bps` | offered load **per STA** (mean rate) |
| `--burst_prob p`     | probability a slot is a "burst"      |
| `--burst_factor k`   | rate multiplier during burst         |

### 3.5  Time & Simulation

| Argument           | Meaning               |
| ------------------ | --------------------- |
| `--slot_ms ms`     | slot duration         |
| `--sim_time_ms ms` | total simulation time |

### 3.6  Experiment / Output

| Argument        | Meaning                                |
| --------------- | -------------------------------------- |
| `--compare`     | run ac + ax + be head‑to‑head          |
| `--compare_sta` | sweep STA count (needs start/end/step) |
| `--plot`        | draw Matplotlib charts                 |


---

## 4  Output Metrics

| Metric                      | Definition                               |
| --------------------------- | ---------------------------------------- |
| **Average Delay (ms)**      | mean queueing delay of delivered packets |
| **Spectral Efficiency (%)** | bytes sent ÷ slot capacity               |
| **Throughput (Mbps)**       | aggregate goodput = bytes×8 / run‑time   |

---

## 5 Example

This example shows the simple result among different number of STAs with static traffic.

```
python3 wifi_gen_sim.py --compare_sta --n_sta_start 1 --n_sta_end 29 --n_sta_step 4 --plot --burst_prob 0.1 --burst_factor 10 --arrival_rate 3e6 --allow_full_channel_alloc
```

![image](https://github.com/jackychiangtw/wifi-sim/blob/main/Simulation/Python-UL-Scheduler/example.png)

## 6 Analysis Example

> To reduce the cost, simulation time set as 20 to find the trend. Simulation time should better than 10^6.

### 6.1 TXOP 

We evaluate how different TXOP lengths influence 802.11ac uplink throughput and average packet delay under in different total traffic loading and number of STAs.

#### Assumptions
- Channel & PHY: 20 MHz, MCS 9, NSS 1, GI 0.8 µs
- Slot duration: 1 ms
- Number of STAs: {1, 5, 9, 13, 17, 21, 25, 29}
- TXOP lengths: {1, 2, 4, 8} slots
- Traffic model: Per‑STA CBR; offered‑load scaled to reach {10, 30, 50, 70, 90}% of single‑channel capacity

> Spec defines the max of TXOP length is defined as 5.484ms

#### Result

![txop_comparison_plot_load_0_10](burst_comparison_plot_load_0_10.png =300x)![txop_comparison_plot_load_0_30](https://hackmd.io/_uploads/HyUukhSNxx.png =300x)![txop_comparison_plot_load_0_50](https://hackmd.io/_uploads/ByD_12SEgx.png =300x)![txop_comparison_plot_load_0_70](https://hackmd.io/_uploads/SyUOJnBNgg.png =300x)![txop_comparison_plot_load_0_90](https://hackmd.io/_uploads/H1LO12HEgg.png =300x)

#### Discusion 

The advantage of longer TXOP can be observed in the high load:

- Low load: The medium sits idle most of the time. There's no much data in the queue, thus longer TXOP won't be used. 

- High load: Because every extra back‑off hurts. Allowing longer bursts keeps goodput much closer to the offered load and slowing the growth of delay.

### 6.2 Comparison of ac, ax and be

To show the progress of legacy CSMA (802.11ac), OFDMA with 1‑RU per STA (802.11ax), and multi‑RU OFDMA (draft 802.11be). This section compares the different traffic load and number of STAs. 

#### Assumptions
- Channel & PHY: 20 MHz, MCS 9, NSS 1, GI 0.8 µs
- Slot duration: 1 ms
- Number of STAs: {1, 5, 9, 13, 17, 21, 25, 29}
- Schedulers:
    - ac (TXOP 1 & 8)
    - ax (single‑RU, RR)
    - be (multi‑RU = 2, RR)
- Traffic model: Per‑STA CBR; offered‑load scaled to reach {10, 30, 50, 70, 90}% of capacity

#### Result

![all_phy_comparison_plot_load_0_10](https://hackmd.io/_uploads/B1oJxnSNll.png =300x)![all_phy_comparison_plot_load_0_30](https://hackmd.io/_uploads/rkj1lnSVxl.png =300x)![all_phy_comparison_plot_load_0_50](https://hackmd.io/_uploads/rkiye2HVel.png =300x)![all_phy_comparison_plot_load_0_70](https://hackmd.io/_uploads/Hyjkl2rEee.png =300x)![all_phy_comparison_plot_load_0_90](https://hackmd.io/_uploads/Syjyl2BNll.png =300x)

#### Discusion 

- Low load (<10%): User can't aware the protocol choice because all protocol can reach the target throughput and delay is closed to 0.
- Middle load (30~50%): CSMA begins to show its disadvantages, even the high TXOP slots. ax/be hold target throughput and near‑zero delay thanks to collision‑free OFDMA. 
- High load (>70%):
    - **ax** is suffered from the defect of simplfied scheduler and number of RU is lower than number STAs, it has the abnormal latency in the lower STAs.
    - Beside that, **ax**'s performance is closed to **be**.


### 6.3 Burst‑Traffic Sensitivity

To highlight the advtanges of algorithm, we fix 30 STAs and sweep the burst factor. Two schedulers are compared:
- RR – plain round‑robin
- PFS – proportional‑fair

#### Assumptions
- PHYs: ax (1‑RU) & be (multi‑RU = 3)
- Slot duration: 1 ms
- Number of STAs: 30
- Schedulers: RR vs PFS
- Burst mode: `burst_prob = 0.1, burst_factor ∈ {1, 2, 4, 8, 16, 32, 64}`
- Other params: Same 20 MHz, MCS 9, NSS 1, 5 ms slot


#### Result
![burst_comparison_plot_load_0_10](https://hackmd.io/_uploads/SkTbg2BVel.png =300x)![burst_comparison_plot_load_0_30](https://hackmd.io/_uploads/rk6WehrNlx.png =300x)![burst_comparison_plot_load_0_50](https://hackmd.io/_uploads/BJTZlnB4gg.png =300x)![burst_comparison_plot_load_0_70](https://hackmd.io/_uploads/S1aWenSNxx.png =300x)![burst_comparison_plot_load_0_90](https://hackmd.io/_uploads/B1abenSEgx.png =300x)


#### Discussion

Algorithm:
- **RR** has the same performamce in ax and be in the result of all load. In addition, the latency is much more than PFS.

Protocol:
- With **PFS** and burst factor, we can observe that **be**'s latency is lower than **ax** in the high load. 
