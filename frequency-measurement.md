# Frequency Measurement Strategy

- MCU clock: **36 MHz**
- Signal range: **1-500 kHz**
- Use **PCA external counting** via `ECI`, meaning using timer to measure time and PCA module to count.
- Set `CMOD.CPS2:CPS0 = 011`.
- ECI maximum: `SYSclk/2 = 18 MHz`, safely above 500 kHz.
- Sample continuous `CH:CL` counts every **100 ms** using another timer.

```c
delta = (uint16_t)(current - previous);
frequency_hz = (uint32_t)delta * 10;
```

| Input frequency | Pulses in 100 ms | 1-count relative resolution |
|---:|---:|---:|
| 1 kHz | 100 | 1% |
| 10 kHz | 1,000 | 0.1% |
| 100 kHz | 10,000 | 0.01% |
| 500 kHz | 50,000 | 0.002% |

The maximum interval count is 50,000, below the 16-bit limit of 65,535. Unsigned subtraction handles one counter wrap automatically. Use a longer gate if better low-frequency resolution is required.
