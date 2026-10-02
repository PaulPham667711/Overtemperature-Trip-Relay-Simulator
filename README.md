# ESP32 Inverse-Time Protection Relay Simulator

A microcontroller-based simulation of an overcurrent-style protective relay, implementing an **inverse-time trip curve** (inspired by IEC 60255) and **latching lockout behavior** (analogous to an ANSI device 86 lockout relay) — instead of a simple fixed-threshold alarm. The more severe the fault, the faster the system trips, and once tripped, it stays locked out until the condition genuinely clears — mirroring how real protective relays behave in distribution and transmission systems (e.g. at utilities like Hydro One, Toronto Hydro, or OPG).

▶ **Live demo (Wokwi):** https://wokwi.com/projects/476162989825479681

![Circuit screenshot](Can be found in my repo.)

## Why this project
Most student "sensor + LED alarm" projects trip instantly at a threshold and reset just as instantly. Real protection relays do neither: a mild, brief overage gets a longer delay (it might be a transient), a severe fault trips almost immediately, and once tripped, the system **latches** — it doesn't silently re-close itself the moment conditions improve slightly. This project implements all three behaviors, using temperature as a stand-in for fault current.

## How it works

The system reads a live sensor value (a potentiometer, standing in for a current/temperature sensor) and moves through these states:

| State | Condition | Indicator |
|---|---|---|
| **Normal** | Below reset threshold (75) | Green LED |
| **Caution** | 75–100, rising, not yet tripped | Yellow LED |
| **Counting down** | ≥100, timer running toward trip | Orange LED |
| **Tripped (active fault)** | Latched, temperature still ≥100 | Red LED, servo open |
| **Tripped (cooling)** | Latched, temperature back in 75–100 | Red + Yellow LEDs, servo stays open |
| **Reset** | Drops below 75 | Returns to Normal |

### The trip curve
Instead of tripping the instant the threshold is crossed, wait time shrinks as severity increases:
```
waitTime = K / (1 + (reading − tripThreshold))
```
- Barely over threshold → long wait (avoids nuisance trips from brief transients)
- Far over threshold → wait shrinks toward near-instant (fast response to real danger)

Mirrors the IEC 60255 standard inverse-time relay formula:
```
t = TMS × ( k / ( (I / Ipickup)^α − 1 ) )
```

### Latching / lockout behavior
Once tripped, the system does **not** automatically re-close as temperature drops — it stays latched (servo open, red LED on) through the entire 75–100 "cooling" range, and only fully resets below 75. This is implemented with a `tripped` boolean flag and mirrors the function of a real lockout relay (ANSI device 86), which requires a deliberate reset rather than silently re-energizing after a fault.

A second flag, `inFault`, tracks whether a countdown is currently in progress (separate from whether the system has fully tripped), preventing the countdown's start time from being overwritten on every loop.

## Hardware

| Component | Purpose | ESP32 Pin |
|---|---|---|
| Potentiometer | Simulated sensor input | GPIO32 |
| Red LED | "Tripped" indicator | GPIO4 |
| Yellow LED | "Caution / cooling" indicator | GPIO17 |
| Green LED | "Normal operation" indicator | GPIO2 |
| Orange LED | Countdown status indicator | GPIO16 |
| Servo motor | Simulates breaker opening/closing | GPIO14 |

(Note: GPIO0 and GPIO12 were avoided for signal/sensor pins — both are ESP32 strapping pins used during boot and can behave unreliably as general I/O.)

## Software
- C++ (Arduino framework) for ESP32
- Uses the **ESP32Servo** library (the classic Arduino `Servo.h` isn't compatible with ESP32's timer architecture)
- Built and tested first in [Wokwi](https://wokwi.com) before real hardware

## Bugs found and fixed during development
- **Fake hysteresis:** initial version reset immediately below 100°C instead of requiring a drop below the separate 75°C reset threshold — fixed by adding the `tripped` latch.
- **millis() race condition:** calling `millis()` twice (once for the loop snapshot, once for the fault start time) risked an unsigned integer underflow if a millisecond ticked between calls, causing instant false trips — fixed by reusing a single `currentMillis` snapshot throughout the loop.

## What I'd add next
- Replace the potentiometer with a real current sensor (ACS712) or temperature sensor
- Add a **primary/backup relay coordination** scheme — two relays with different time multiplier settings, so a backup only trips if the primary fails to clear the fault
- Add breaker status feedback (confirm the relay/servo actually moved, rather than assuming it did)
- Add a physical reset button instead of relying purely on temperature dropping, for a more realistic "manual lockout reset"
- Log trip events with timestamp and severity to build a real fault record

## Files in this repo
- `Trip Relay simulator with ESP32 (1).zip` — including my diagram's code, libraries, etc.
- `README.md` — this file
- `First_Diagram` - screenshot of my Diagram.