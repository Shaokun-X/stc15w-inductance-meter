# Q-Factor Estimation and Calibration

## Purpose

This procedure uses a rough Q estimate to reduce systematic measurement error
caused by inductor loss in the Colpitts oscillator.

It does not attempt to build an accurate Q meter. Instead, it:

- Estimates Q from oscillator frequency, inductance, and DUT resistance.
- Learns how measurement error changes with Q on the actual hardware.
- Applies a small empirical correction to acceptable measurements.
- Rejects measurements whose estimated Q is too low for a trustworthy result.

Q in this document is the quality factor. It is unrelated to transistor
reference designators such as Q1 and Q2.

## Important Limitation

The estimate uses DC resistance as an approximation for total inductor loss.
It does not include all frequency-dependent winding and core losses. The
calculated Q will therefore often be higher than the real Q.

There is no universal Q correction for a sustained Colpitts oscillator. The
correction coefficient must be obtained from measurements made with the final
oscillator hardware.

## Overview

The procedure has two phases:

1. One-time calibration determines the normal frequency calibration and a Q
   correction coefficient named `K`.
2. Runtime measurement estimates the unknown DUT's Q and applies the calibrated
   correction.

## Phase 1: One-Time Calibration

### Step 1: Calibrate the normal frequency conversion

Measure several characterized, high-Q inductors and determine the baseline
frequency-to-inductance conversion:

```text
L_base = A / frequency^2 + B
```

`A` accounts mainly for effective tank capacitance and clock scale. `B`
accounts mainly for fixture and PCB inductance.

Purpose: remove fixed errors before attempting to identify Q-dependent error.
Otherwise, capacitor, clock, and fixture errors could be incorrectly attributed
to Q.

### Step 2: Create calibration points with different losses

Connect known resistors in series with a characterized inductor:

```text
reference inductor + 0 ohm
reference inductor + 4.7 ohm
reference inductor + 10 ohm
reference inductor + 22 ohm
```

The added resistance lowers Q without intentionally changing the reference
inductance.

Repeat this test with several inductance values if possible. This verifies that
one correction remains useful across the complete frequency range.

Purpose: observe how the real oscillator's result changes as resonator loss
increases.

### Step 3: Estimate Q for each calibration point

Calculate total series resistance:

```text
R_total = inductor_DCR + added_resistance
```

Calculate approximate Q:

```text
Q_est = 2 * pi * frequency * L_reference / R_total
```

Use the known reference inductance in this phase, not the measured result.

Purpose: assign an approximate Q value to each calibration measurement.

### Step 4: Calculate residual inductance error

First calculate the baseline inductance from the measured frequency:

```text
L_base = A / frequency^2 + B
```

Then calculate relative error:

```text
error = (L_base - L_reference) / L_reference
```

Example:

```text
L_reference = 1.000 mH
L_base      = 1.040 mH

error = (1.040 - 1.000) / 1.000
error = 0.040
error = 4.0%
```

Purpose: measure only the error that remains after normal frequency
calibration.

### Step 5: Calculate the Q correction coefficient

Use the simple empirical model:

```text
error = K / Q_est
```

Calculate `K` for each calibration point:

```text
K = error * Q_est
```

Example:

```text
error = 0.040
Q_est = 10

K = 0.040 * 10
K = 0.40
```

A useful calibration set might look like this:

| Estimated Q | Residual error | Calculated K |
| ---: | ---: | ---: |
| 20 | 2.0% | 0.40 |
| 10 | 4.0% | 0.40 |
| 7 | 5.5% | 0.385 |

These results support using approximately:

```text
K = 0.40
```

Purpose: reduce the observed Q-dependent error to one coefficient that is easy
to use in firmware.

If calculated `K` values vary widely or change sign, Q does not predict the
error reliably. Do not apply this correction in that case. Use Q only to reject
poor measurements.

## Phase 2: Runtime Measurement

### Step 1: Measure oscillator frequency

Measure the Colpitts oscillator frequency using the hardware counter or PCA
capture path:

```text
frequency = measured_oscillator_frequency
```

Purpose: obtain the primary measurement used to calculate inductance.

### Step 2: Calculate baseline inductance

Use the normal calibrated conversion:

```text
L_base = A / frequency^2 + B
```

Purpose: obtain an initial inductance result before accounting for DUT loss.

### Step 3: Estimate oscillator current

For the proposed Q2 current sink and 680 ohm emitter resistor:

```text
current = Q2_emitter_voltage / 680 ohm
```

Purpose: determine the approximate DC current flowing through the DUT instead
of assuming that transistor and resistor tolerances produce exactly 0.5 mA.

### Step 4: Estimate DUT resistance

Measure the average DC voltage at the TANK node. Remove the oscillator waveform
with a sufficiently high-impedance low-pass measurement path.

Calculate the DUT voltage drop:

```text
DUT_voltage = supply_voltage - average_TANK_voltage
```

Calculate DCR:

```text
DCR_est = DUT_voltage / current
```

Purpose: estimate the primary loss responsible for lowering DUT Q.

Very low DCR may be below ADC resolution. In that case, classify the DUT as
high-Q and skip the Q correction. Exact DCR is unnecessary when resistive loss
is too small to measure because the expected correction is also small.

### Step 5: Estimate DUT Q

Use the baseline inductance result:

```text
Q_est = 2 * pi * frequency * L_base / DCR_est
```

Example:

```text
frequency = 48000 Hz
L_base   = 1.050 mH
DCR_est  = 20 ohm

Q_est = 2 * pi * 48000 * 0.001050 / 20
Q_est = 15.8
```

Purpose: estimate how sensitive the measurement may be to oscillator frequency
pulling caused by DUT loss.

### Step 6: Estimate Q-related error

Use the `K` value determined during calibration:

```text
estimated_error = K / Q_est
```

Example using a hypothetical calibrated value of `K = 0.40`:

```text
estimated_error = 0.40 / 15.8
estimated_error = 0.0253
estimated_error = 2.53%
```

Purpose: predict the residual measurement error associated with the estimated
Q.

The `K = 0.40` value is only an example. It must not be copied into firmware
until measurements on the final oscillator produce that value.

### Step 7: Correct the inductance

Apply the empirical correction:

```text
L_corrected = L_base / (1 + estimated_error)
```

Example:

```text
L_corrected = 1.050 mH / 1.0253
L_corrected = 1.024 mH
```

Purpose: remove the systematic Q-dependent error observed during calibration.

The same equation works when `K` is negative. A negative `K` produces a
correction in the opposite direction.

### Step 8: Optionally repeat once

Recalculate Q using the corrected inductance:

```text
Q_est = 2 * pi * frequency * L_corrected / DCR_est
```

Then repeat the error and inductance correction once.

Purpose: remove the small circular dependency between inductance and estimated
Q. More than one extra iteration is unnecessary for a rough correction.

## Result Acceptance

Use conservative provisional limits:

| Condition | Action |
| --- | --- |
| DCR below ADC resolution | Treat as high Q and use `L_base` |
| `Q_est >= 10` | Apply the calibrated correction |
| `5 <= Q_est < 10` | Correct and report reduced confidence |
| `Q_est < 5` | Report `LOW_Q` |
| Absolute correction greater than 5% | Report `LOW_Q` or reduced accuracy |
| Oscillator frequency is unstable | Reject the measurement |
| Oscillator does not start | Report `NO_OSCILLATION` |

These limits must be adjusted using prototype results. Do not apply a large
correction to force a plausible result from a very lossy resonator.

## Runtime Algorithm

```text
1. Measure frequency.

2. Calculate baseline inductance:
   L_base = A / frequency^2 + B

3. Measure oscillator current:
   current = Q2_emitter_voltage / 680 ohm

4. Estimate DUT resistance:
   DCR_est = (supply_voltage - average_TANK_voltage) / current

5. If DCR is below ADC resolution:
   return L_base as a high-Q result

6. Estimate Q:
   Q_est = 2 * pi * frequency * L_base / DCR_est

7. If Q_est < 5:
   return LOW_Q

8. Estimate Q-related error:
   estimated_error = K / Q_est

9. If abs(estimated_error) > 0.05:
   return LOW_Q or REDUCED_ACCURACY

10. Correct inductance:
    L_corrected = L_base / (1 + estimated_error)

11. Optionally recalculate Q and correction once.

12. Return L_corrected and Q_est.
```

## Recommended Calibration Data

Record at least the following for every calibration point:

| Field | Purpose |
| --- | --- |
| Reference inductance | Ground truth |
| Inductor DCR | Base resistive loss |
| Added series resistance | Controlled Q reduction |
| Measured frequency | Raw oscillator result |
| Baseline inductance | Result before Q correction |
| Estimated Q | Loss indicator |
| Relative error | Correction target |
| Calculated K | Model consistency check |
| Oscillator amplitude | Detect clipping or nonlinear operation |
| Frequency stability | Detect unreliable low-Q oscillation |

Test multiple inductance values and constructions. Series resistors reproduce
winding resistance but do not reproduce every type of magnetic core loss.

## Success Criteria

Use the Q correction only if all of the following are true:

- `K` has a consistent sign across calibration points.
- Similar Q values produce similar residual errors.
- Corrected results are better than baseline results on validation inductors
  that were not used to calculate `K`.
- The correction remains small relative to the 5% accuracy target.
- Low-Q or unstable measurements are rejected rather than over-corrected.

If these conditions are not met, retain the baseline calibration and use
`Q_est` only as a measurement-confidence indicator.
