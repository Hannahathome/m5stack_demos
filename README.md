# Example Codes for M5Stack + Paper Voxels Demo  
This repository contains Arduino example codes for three creative systems you can build using **M5Stack units**, combined with **LEGO** and **paper**.  
The examples range from **games** to **musical instruments** and even a **personal health system**.  

## Example 1: Game 
**Units used:**  
- M5Core2  
- Unit Joystick  
- Unit Button  
- Unit Hub/ Grove Hub  

**Description:**  
Control a block with the joystick and change its color by pressing the button.  
This simple setup is perfect as a starting point for puzzle-style games.  

**example 1+ -->** Add a second player by plugging in the CardKB unit!  

---
## Example 2: Mini theremin  
**Units used:**  
- M5Core2  
- Unit Joystick
- Unit Hub/ Grove Hub  
- Unit Heart  

**Description:**  
A theremin is an electronic instrument played without touch. In this version:  
- The **heart rate sensor** controls the frequency  
- The **joystick** controls the amplitude  

Experiment with sound and have fun making music on this compact digital instrument!  

---
## Example 3: Snack Tracker  
**Units used:**  
- M5Core / Basic 
- Unit Mini Scales  
- Unit Button
- Unit Hub/ Grove Hub  

**Description:**  
Keep track of your snacks with this smart snack tracker. The system constantly weighs your snack bowl.  

Option 1: Tracking your snacks
- Want to understand when and how much you snack througout the day? Let thissimple system track your snack consumption throughout the day.
- Each time to crave a snack but do not eat it, press the button.
- Each time you want a snack, just take it from the scale
At the end of the day, you can see your stats and timeline, showing how much weight in snacks was eaten, and how often you decided against eating a snack. 

Option 2: Snack thief
Have someone around your house or office eating your snacks? Build this to keep them at bay. 
- Want a snack? Take some out and press the hidden button to let the system know it’s you.  
- If snacks are removed **without a button press** within 30 seconds, an alarm will sound and the screen will flash red to snitch on the **snack thief**!  

---

# Useful information for designing new enclosures

| Unit                     | Dimensions (mm)     | Documentation Link |
|--------------------------|---------------------|--------------------|
| M5Core / Basic           | 54 × 54 × 17        | [Docs](https://docs.m5stack.com/en/core/basic) |
| M5Core2                  | 54 × 54 × 16.5      | [Docs](https://docs.m5stack.com/en/core/core2) |
| Unit Mini Scales         | 40 × 24 × 18        | [Docs](https://docs.m5stack.com/en/unit/Unit-Mini%20Scales) |
| Unit Button              | 32 × 24 × 8         | [Docs](https://docs.m5stack.com/en/unit/button) |
| Unit Heart               | 32 × 24 × 8         | [Docs](https://docs.m5stack.com/en/unit/heart) |
| Unit Joystick            | 48 × 24 × 32        | [Docs](https://docs.m5stack.com/en/unit/joystick_1.1) |
| Unit Hub / Grove Hub     | 32 × 24 × 10.8      | [Docs](https://docs.m5stack.com/en/unit/hub) |

---

# How to build
## Prepare you computer:
1. `choco install arduino-cli`
2. `arduino-cli core update-index`
3. `arduino-cli core install esp32:esp32@2.0.11`
4. `arduino-cli lib install M5Core2`
`
## Build the program
From within the wanted example:
1. `cd` into the example that you want to build and upload
2. `arduino-cli compile --fqbn esp32:esp32:m5stack-core-esp32`
3. Figure out what port you need to use `arduino-cli board list`
4. `arduino-cli upload --fqbn esp32:esp32:m5stack-core-esp32 --port <YOUR COM PORT HERE>`    
