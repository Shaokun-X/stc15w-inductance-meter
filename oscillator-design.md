# NPN Colpitts Oscillator Design

## Goal

Design a 5 V LC oscillator for measuring inductors from 10 uH to 1 H with the
STC15W408AS. The target accuracy is approximately 5% for general-purpose
inductors with up to approximately 1 kohm DCR at the high-inductance end.

The oscillator produces a frequency determined primarily by the unknown
inductor and precision capacitors. The MCU measures this frequency and derives
the inductance.

This design measures small-signal inductance. It does not characterize
inductance under DC bias or near core saturation.

## Proposed Circuit

Use a common-base NPN Colpitts oscillator with a second NPN transistor as a
current sink:

```text
                         +5 V
                           |
                          Lx  unknown inductor
                           |
              TANK o-------+------- collector Q1
                    |               2N3904
                   C1                  |
                  22 nF                |
                    |                  |
               TAP o---------------- emitter Q1
                    |                  |
                   C2               collector Q2
                  22 nF              2N3904
                    |                  |
                   GND              emitter
                                       |
                                     680 ohm
                                       |
                                      GND
```

Bias the transistor bases separately:

```text
Q1 base bias:

+5 V -- 18 kohm --+-- 10 kohm -- GND
                  |
                Q1 base
                  |
             10 uF || 100 nF
                  |
                 GND

Q2 base bias:

+5 V -- 33 kohm --+-- 8.2 kohm -- GND
                  |
                Q2 base
                  |
             10 uF || 100 nF
                  |
                 GND
```

Place `100 nF || 10 uF` directly across the 5 V supply near the oscillator.

## Circuit Operation

Q1 supplies the negative resistance needed to replace losses in the LC tank.
The capacitive divider formed by C1 and C2 provides positive feedback at the
tank resonant frequency.

Q2 supplies an approximately constant 0.4 to 0.5 mA current. It is used instead
of a resistor below Q1 because its collector presents a much higher AC
impedance. A simple emitter resistor would load C2 and disturb the oscillator,
particularly at the low-frequency end.

The Q2 current is approximately:

```text
I = (VB2 - 0.65 V) / 680 ohm
```

The actual current will vary with transistor gain, base-emitter voltage, and
divider loading. Approximately 0.35 to 0.55 mA is expected.

This DC current flows through the unknown inductor. A 1 kohm DUT therefore
causes approximately 0.5 V of DC drop at 0.5 mA.

## Resonant Frequency

The effective capacitance of the Colpitts divider is:

```text
Ceq = C1 * C2 / (C1 + C2)
```

With C1 and C2 both equal to 22 nF:

```text
Ceq = 11 nF
```

The approximate oscillation frequency is:

```text
f = 1 / (2 * pi * sqrt(Lx * Ceq))
```

Expected frequencies are:

| Inductance | Approximate frequency |
| ---: | ---: |
| 10 uH | 480 kHz |
| 100 uH | 152 kHz |
| 1 mH | 48 kHz |
| 10 mH | 15.2 kHz |
| 100 mH | 4.8 kHz |
| 1 H | 1.52 kHz |

One capacitor range covers the complete target range. A switched capacitor
range is not initially recommended because it would reduce tank Q and add
switch resistance, capacitance, and calibration uncertainty.

## Component Selection

| Reference | Initial value or part | Notes |
| --- | --- | --- |
| Q1, Q2 | 2N3904 or BC547B | General-purpose small-signal NPN |
| C1, C2 | 22 nF, 1% | C0G/NP0 or polypropylene film |
| Q1 bias upper | 18 kohm | From 5 V to Q1 base |
| Q1 bias lower | 10 kohm | From Q1 base to ground |
| Q2 bias upper | 33 kohm | From 5 V to Q2 base |
| Q2 bias lower | 8.2 kohm | From Q2 base to ground |
| Q2 emitter | 680 ohm | Sets approximately 0.5 mA |
| Base bypass | 10 uF and 100 nF | One pair at each base |
| Supply bypass | 10 uF and 100 nF | Place close to Q1 and Q2 |

C1 and C2 should be rated for at least 10 V. X7R and similar high-k ceramic
capacitors should not be used because their capacitance varies with voltage,
temperature, and age. Their variation would directly affect the calculated
inductance.

## Oscillation and DCR

For equal divider capacitors, the approximate magnitude of the negative
resistance is:

```text
|Rneg| = gm * L / (4 * Ceq)
```

At approximately 0.5 mA collector current, Q1 transconductance is around
19 mS. With 10 uH and 11 nF, the available negative resistance is approximately
4.3 ohm. A 10 uH DUT with several ohms of DCR may consequently fail to start.

Available negative resistance increases with inductance. Supporting around
1 kohm DCR is therefore plausible near 1 H but is not a valid requirement for
every inductance value. The practical limit must be established with real
parts.

Failure to oscillate should be reported as a low-Q or excessive-loss condition,
not converted into an inductance value.

## Output Detection

Take the measurement signal from `TAP` rather than directly from `TANK`. TAP is
the lower-impedance point and can tolerate more detector loading.

A tentative AC-coupled detector input is:

```text
TAP -- 100 nF -- 10 kohm -- detector input
                              |
                         1 Mohm to +5 V
                              |
                         1 Mohm to GND
```

The coupling capacitor removes the approximately 1 V DC level at TAP. The two
1 Mohm resistors bias the detector input near 2.5 V while keeping tank loading
small.

Possible detectors are:

- The MCU comparator for initial experiments.
- A protected digital GPIO if the signal reliably crosses its logic thresholds.
- A fast external comparator or Schmitt buffer for a more predictable final
  design.

The detector is outside the oscillator feedback loop. A nearly constant
detector propagation delay therefore cancels when measuring the interval
between equivalent edges. Delay variation, unequal edge delays, and insufficient
output pulse width can still cause errors near the upper frequency limit.

The detector output should preferably pass through a divide-by-16 counter
before reaching PCA capture. The expected divided frequency range is
approximately 95 Hz to 30 kHz.

## MCU Measurement

Use hardware PCA capture rather than servicing every oscillator edge in an
external-interrupt ISR. The frequency measurement path should be:

```text
Colpitts TAP -> detector -> divide by 16 -> PCA capture
```

The firmware should extend the 16-bit PCA counter across overflows, capture two
equivalent divided edges, and calculate the elapsed counter ticks. The original
oscillator frequency is 16 times the measured divider output frequency.

The initial inductance calculation is:

```text
L = 1 / (4 * pi^2 * f^2 * Ceq)
```

The final implementation should use calibrated coefficients to absorb actual
capacitor value, fixture inductance, clock error, and consistent oscillator
loading.

## Calibration

At least two characterized inductors should be used to calibrate the design.
A useful calibrated model is:

```text
L = A / f^2 + B
```

`A` accounts primarily for effective capacitance and clock scale. `B` accounts
primarily for fixture and PCB inductance. Calibration parts should not also be
used as the only validation parts.

Low-Q frequency pulling is not necessarily linear and may not be corrected by
this model. Validation must include inductors with different DCR values.

## Prototype Validation

1. Assemble the oscillator with short tank connections and no MCU attached.
2. Verify the Q1 base voltage is approximately 1.7 V.
3. Verify the TAP DC voltage is approximately 1.0 to 1.1 V.
4. Verify the inductor current remains below approximately 0.5 mA.
5. Test representative 10 uH, 1 mH, and 1 H inductors.
6. Measure TANK and TAP using a low-capacitance oscilloscope probe.
7. Confirm startup, stable amplitude, and an approximately sinusoidal tank
   waveform.
8. Compare measured frequency against the ideal calculation.
9. Add the detector and confirm it does not measurably shift oscillator
   frequency.
10. Test inductors of similar nominal value but different DCR and construction.
11. Determine the lowest supported Q and define the firmware failure criteria.
12. Validate the divider and PCA capture path using a signal generator before
    relying on oscillator measurements.

## Layout Guidance

- Keep Lx, C1, C2, and Q1 connections short.
- Keep the TANK and TAP nodes away from MCU clocks and UART traces.
- Give the analog oscillator its own local supply decoupling.
- Connect analog ground to the digital ground at a quiet point.
- Do not route a high-current digital return through the oscillator ground.
- Place the detector after a short, high-impedance connection to TAP.
- Provide test points at TANK, TAP, Q1 base, and Q2 base.
- Keep the DUT connector and its return close together to minimize fixture
  inductance.

## Open Items

- Verify startup and amplitude over the complete L and DCR range.
- Confirm whether the MCU comparator output can route directly to PCA or an
  external pin.
- Characterize the MCU comparator at approximately 480 kHz if it is retained.
- Select the final detector and divide-by-16 device.
- Determine whether amplitude limiting or current adjustment is needed for
  nonlinear ferrite inductors.
- Establish calibration standards and the supported minimum Q.

The listed values are a prototype starting point. They must be verified by
simulation and bench measurement before the circuit is committed to a PCB.
