# HEXAPOD-ROBOT
# SIXpack: Open-Source Autonomous Hexabot Robot 🤖🦀

SIXpack is an open-source, fully 3D-printed 6-legged robot platform designed for robotics enthusiasts and engineering students. Powered by an **ESP32-CAM** module and a **PCA9685 16-Channel PWM driver**, the robot sets up a local Wi-Fi Access Point upon boot, allowing users to stream live video feeds and control locomotion seamlessly via a touchscreen web browser interface.

---

## 📸 System Architecture
* **Limb Configuration:** Symmetrical 6-leg setup driven by 12 micro servo motors (2-DOF per leg implementing independent sweep and lift articulations).
* **Control Mechanism:** Local webserver hosted directly on the ESP32-CAM with touchscreen button navigation support.

---

## 🛠️ Hardware Requirements & Bill of Materials (BOM)

*   **Microcontroller:** 1x ESP32-CAM module
*   **Actuator Driver:** 1x PCA9685 16-Channel 12-bit I2C PWM board
*   **Actuators:** 12x Micro Servo Motors (Compatible with MG90s, MF90, or SG90)
*   **Power Source:** Battery holder equipped with 2x 18650 LiPo batteries (System operates on ~7.4V)
*   **Control Hardware:** 1x Sliding DPDT 2P2T Switch (SS23D32 F3E4)
*   **Chassis:** 3D-printed body links (Body frame, Lid, Head piece, and Leg links)
*   **Fasteners:** Pan Head self-tapping screws ($M2 \times 4$, $M2 \times 8$, and $M2 \times 12$)
*   **Miscellaneous:** Female Jumper wires & 1x8 Socket Headers

---

## 🔌 Hardware Wiring & Interfacing Connections

The system uses a common power distribution scheme where the 7.4V battery pack supplies the high-current demands of the servos while utilizing the ESP32-CAM's voltage lines for I2C logic processing.

### Master Schematic Overview
Follow this pin-to-pin mapping layout precisely to establish communications:

<img width="952" height="547" alt="image" src="https://github.com/user-attachments/assets/469aea25-64cc-47f1-9668-81caef7a3160" />

<img width="3000" height="4000" alt="1000138960" src="https://github.com/user-attachments/assets/a4bea1c3-7fe3-4c63-bac4-fd4de6489620" />

<img width="3000" height="4000" alt="1000138961" src="https://github.com/user-attachments/assets/f3d49d37-b060-494d-9db5-3848ead56ac8" />



