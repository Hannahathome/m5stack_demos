# M5Stack Core2 + MAX30100 Heart-Rate Monitor (Reference)

A minimal reference for building a touch-screen heart-rate monitor with an M5Stack Core2 and a MAX30100 sensor.

## Features
- Real-time BPM display
- Live pulse waveform
- Haptic pulse on beat
- Three views: Raw, BPM, Graph

## Hardware
- M5Stack Core2
- MAX30100 heart-rate/SpO₂ sensor module
- 4× male-to-female jumper wires

## Wiring (I²C)
| MAX30100 | Core2 | Notes |
|---|---|---|
| VCC | 3.3V | Do **not** use 5V |
| GND | GND | Common ground |
| SDA | GPIO 21 | I²C data |
| SCL | GPIO 22 | I²C clock |

- Default I²C address: `0x57`
- Typical I²C clock: 100 kHz

## Software Setup (Arduino IDE)
1. Install Arduino IDE.
2. Add Board Manager URL (File → Preferences → “Additional Board Manager URLs”):
   ```
   https://m5stack.oss-cn-shenzhen.aliyuncs.com/resource/arduino/package_m5stack_index.json
   ```
3. Boards (Tools → Board → Boards Manager): install **M5Stack by M5Stack**; select **M5Stack-Core2**.
4. Libraries (Tools → Manage Libraries):
   - **M5Core2** (by M5Stack)
   - **Arduino-MAX30100** (by oxullo)
5. Connect Core2 via USB, pick the correct **Port**. Upload speed `921600` works well.

## Build & Upload
1. Open `heartrate_monitor_example.ino`.
2. Click **Upload**. The Core2 will auto-restart after flashing.

## Usage
1. Power on the Core2. It should initialize the MAX30100.
2. Place your index finger gently over the sensor (cover LEDs and photodiode; stay still).
3. Use on-screen controls to switch:
   - **Raw**: IR/RED values and signal strength
   - **BPM**: large BPM readout + beat animation
   - **Graph**: real-time waveform

Haptics: the device vibrates on each detected beat.

## Critical Timing Requirement
The driver must be updated frequently or readings will fail.

```cpp
void loop() {
  sensor.update();      // Must be called every ~10–50 ms
  // ... your code ...
  delay(10);            // Keep delays short; avoid blocking calls
}
```

Guidelines:
- Call `sensor.update()` at ~20–100 Hz.
- Avoid long `delay()`, busy waits, or blocking I/O in `loop()`.
- If updates pause for >2–3 s, restart the device.

## Troubleshooting

**“Sensor not found!”**
- Recheck VCC/GND/SDA/SCL wiring
- Ensure firm connections
- Power-cycle the Core2

**“Place finger on sensor” persists**
- Clean sensor surface
- Try a different finger
- Cover the whole sensor; don’t press hard
- Warm hands help

**BPM shows `---`**
- Hold completely still for 10–30 s
- Slightly adjust finger position
- Check that `sensor.update()` runs regularly

**Unrealistic BPM**
- Reduce motion
- Wait for stabilization
- Confirm no long blocking code in `loop()`

**Device becomes unresponsive**
- Hold power to restart
- Re-flash the sketch if needed

## Technical Notes (Core2 + MAX30100)
- Display: 320×240 capacitive touch
- Haptics: on-board vibration motor for beat feedback
- Power: AXP192 PMIC; keep charged for stable operation
- UI update cadence: ~500 ms; keep sensor updates much faster

## Code Structure (typical)
- **Sensor init**: set up I²C and MAX30100
- **Update loop**: `sensor.update()` + short delay
- **Beat callback**: trigger haptic pulse and UI beat marker
- **UI**: draw BPM/graph/raw panes; minimal work per frame
- **History buffer**: keep recent samples for graph scaling

## Safety
Educational project, **not** a medical device. Do not use for diagnosis or treatment.

## License
Example code is for educational use. Respect licenses of dependencies (M5Stack libraries, Arduino-MAX30100).
