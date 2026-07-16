/*
 * HEXAPOD AUTONOMOUS ROBOT
 * Hardware: ESP32 + PCA9685 + 12x SG90 Servo Motors
 * Gait: Tripod Gait (most stable for 6-leg robots)
 * 
 * LEG NUMBERING (top view):
 *   Front-Left(0)   Front-Right(1)
 *   Mid-Left(2)     Mid-Right(3)
 *   Rear-Left(4)    Rear-Right(5)
 *
 * PCA9685 CHANNEL MAPPING:
 *   Leg 0: CH0(coxa), CH1(tibia)
 *   Leg 1: CH2(coxa), CH3(tibia)
 *   Leg 2: CH4(coxa), CH5(tibia)
 *   Leg 3: CH6(coxa), CH7(tibia)
 *   Leg 4: CH8(coxa), CH9(tibia)
 *   Leg 5: CH10(coxa), CH11(tibia)
 *
 * TRIPOD GROUPS:
 *   Group A (lift together): Leg 0, 3, 4  (FL, MR, RL)
 *   Group B (lift together): Leg 1, 2, 5  (FR, ML, RR)
 */

#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

// ─── PCA9685 SETUP ───────────────────────────────────────────────
Adafruit_PWMServoDriver pca = Adafruit_PWMServoDriver(0x40); // Default I2C address

// ESP32 I2C Pins (change if needed)
#define SDA_PIN 21
#define SCL_PIN 22

// SG90 PWM values (50Hz)
#define SERVOMIN  150   // ~0 degrees
#define SERVOMAX  600   // ~180 degrees
#define SERVO_MID 375   // ~90 degrees (neutral)

// ─── SERVO ANGLE HELPER ──────────────────────────────────────────
// Maps 0-180 degree to PCA9685 pulse count
uint16_t angleToPulse(int angle) {
  angle = constrain(angle, 0, 180);
  return map(angle, 0, 180, SERVOMIN, SERVOMAX);
}

void setServo(uint8_t channel, int angle) {
  pca.setPWM(channel, 0, angleToPulse(angle));
}

// ─── LEG STRUCTURE ───────────────────────────────────────────────
struct Leg {
  uint8_t coxaCh;   // PCA9685 channel for coxa (horizontal rotation)
  uint8_t tibiaCh;  // PCA9685 channel for tibia (up/down)
  bool    mirrored; // Right side legs need mirrored angles
};

// 6 legs: FL, FR, ML, MR, RL, RR
Leg legs[6] = {
  {0,  1,  false},  // Leg 0: Front-Left
  {2,  3,  true },  // Leg 1: Front-Right
  {4,  5,  false},  // Leg 2: Mid-Left
  {6,  7,  true },  // Leg 3: Mid-Right
  {8,  9,  false},  // Leg 4: Rear-Left
  {10, 11, true },  // Leg 5: Rear-Right
};

// ─── NEUTRAL / HOME POSITIONS ────────────────────────────────────
// Coxa neutral = 90 (pointing sideways)
// Tibia down   = 60 (leg touching ground) — tune this!
// Tibia up     = 120 (leg lifted)

#define COXA_NEUTRAL   90
#define TIBIA_DOWN     60   // ← Tune this so feet touch ground
#define TIBIA_UP       100  // ← Tune this so feet clear ground
#define COXA_FORWARD   70   // Step forward
#define COXA_BACKWARD  110  // Step backward (push)
#define STEP_DELAY     80   // ms between moves (lower = faster walk)

// Apply coxa angle with mirroring for right-side legs
void setCoxaAngle(uint8_t legIdx, int angle) {
  if (legs[legIdx].mirrored) angle = 180 - angle;
  setServo(legs[legIdx].coxaCh, angle);
}

void setTibiaAngle(uint8_t legIdx, int angle) {
  if (legs[legIdx].mirrored) angle = 180 - angle;
  setServo(legs[legIdx].tibiaCh, angle);
}

// ─── ALL LEGS HOME ───────────────────────────────────────────────
void homePosition() {
  for (int i = 0; i < 6; i++) {
    setCoxaAngle(i, COXA_NEUTRAL);
    setTibiaAngle(i, TIBIA_DOWN);
  }
  delay(900);
}

// ─── TRIPOD GAIT - FORWARD WALK ──────────────────────────────────
/*
  Tripod A: legs 0, 3, 4 (FL, MR, RL)
  Tripod B: legs 1, 2, 5 (FR, ML, RR)

  One cycle:
  Phase 1: A lifts + moves forward | B pushes backward (stance)
  Phase 2: A puts down            | B stance hold
  Phase 3: B lifts + moves forward| A pushes backward (stance)
  Phase 4: B puts down            | A stance hold
*/

int tripodA[3] = {0, 3, 4};
int tripodB[3] = {1, 2, 5};

// Lift a group, move coxa forward, lower
void swingGroup(int* group, int fwd_angle) {
  // 1. Lift
  for (int i = 0; i < 3; i++)
    setTibiaAngle(group[i], TIBIA_UP);
  delay(STEP_DELAY);

  // 2. Swing coxa forward
  for (int i = 0; i < 3; i++)
    setCoxaAngle(group[i], fwd_angle);
  delay(STEP_DELAY);

  // 3. Lower
  for (int i = 0; i < 3; i++)
    setTibiaAngle(group[i], TIBIA_DOWN);
  delay(STEP_DELAY);
}

// Push a group's coxa backward (propulsion)
void pushGroup(int* group, int push_angle) {
  for (int i = 0; i < 3; i++)
    setCoxaAngle(group[i], push_angle);
}

void walkForwardStep() {
  // Phase 1: Swing A forward, push B backward simultaneously
  for (int i = 0; i < 3; i++) setTibiaAngle(tripodA[i], TIBIA_UP);
  delay(STEP_DELAY);
  for (int i = 0; i < 3; i++) setCoxaAngle(tripodA[i], COXA_FORWARD);
  pushGroup(tripodB, COXA_BACKWARD);
  delay(STEP_DELAY);
  for (int i = 0; i < 3; i++) setTibiaAngle(tripodA[i], TIBIA_DOWN);
  delay(STEP_DELAY);

  // Phase 2: Swing B forward, push A backward simultaneously
  for (int i = 0; i < 3; i++) setTibiaAngle(tripodB[i], TIBIA_UP);
  delay(STEP_DELAY);
  for (int i = 0; i < 3; i++) setCoxaAngle(tripodB[i], COXA_FORWARD);
  pushGroup(tripodA, COXA_BACKWARD);
  delay(STEP_DELAY);
  for (int i = 0; i < 3; i++) setTibiaAngle(tripodB[i], TIBIA_DOWN);
  delay(STEP_DELAY);
}

// ─── TURN LEFT ────────────────────────────────────────────────────
// Left legs push back, right legs push forward → robot turns left
void turnLeft(int steps) {
  int leftLegs[3]  = {0, 2, 4}; // FL, ML, RL
  int rightLegs[3] = {1, 3, 5}; // FR, MR, RR

  for (int s = 0; s < steps; s++) {
    // Lift left, move backward; right pushes forward
    for (int i = 0; i < 3; i++) setTibiaAngle(leftLegs[i], TIBIA_UP);
    delay(STEP_DELAY);
    for (int i = 0; i < 3; i++) setCoxaAngle(leftLegs[i], COXA_BACKWARD);
    for (int i = 0; i < 3; i++) setCoxaAngle(rightLegs[i], COXA_FORWARD);
    delay(STEP_DELAY);
    for (int i = 0; i < 3; i++) setTibiaAngle(leftLegs[i], TIBIA_DOWN);
    delay(STEP_DELAY);

    // Lift right, move forward; left pushes backward
    for (int i = 0; i < 3; i++) setTibiaAngle(rightLegs[i], TIBIA_UP);
    delay(STEP_DELAY);
    for (int i = 0; i < 3; i++) setCoxaAngle(rightLegs[i], COXA_BACKWARD);
    for (int i = 0; i < 3; i++) setCoxaAngle(leftLegs[i], COXA_FORWARD);
    delay(STEP_DELAY);
    for (int i = 0; i < 3; i++) setTibiaAngle(rightLegs[i], TIBIA_DOWN);
    delay(STEP_DELAY);
  }
}

// ─── TURN RIGHT ───────────────────────────────────────────────────
void turnRight(int steps) {
  int leftLegs[3]  = {0, 2, 4};
  int rightLegs[3] = {1, 3, 5};

  for (int s = 0; s < steps; s++) {
    for (int i = 0; i < 3; i++) setTibiaAngle(rightLegs[i], TIBIA_UP);
    delay(STEP_DELAY);
    for (int i = 0; i < 3; i++) setCoxaAngle(rightLegs[i], COXA_BACKWARD);
    for (int i = 0; i < 3; i++) setCoxaAngle(leftLegs[i], COXA_FORWARD);
    delay(STEP_DELAY);
    for (int i = 0; i < 3; i++) setTibiaAngle(rightLegs[i], TIBIA_DOWN);
    delay(STEP_DELAY);

    for (int i = 0; i < 3; i++) setTibiaAngle(leftLegs[i], TIBIA_UP);
    delay(STEP_DELAY);
    for (int i = 0; i < 3; i++) setCoxaAngle(leftLegs[i], COXA_BACKWARD);
    for (int i = 0; i < 3; i++) setCoxaAngle(rightLegs[i], COXA_FORWARD);
    delay(STEP_DELAY);
    for (int i = 0; i < 3; i++) setTibiaAngle(leftLegs[i], TIBIA_DOWN);
    delay(STEP_DELAY);
  }
}

// ─── AUTONOMOUS BEHAVIOR ─────────────────────────────────────────
/*
  Simple autonomous mode (no sensor):
  - Walk forward N steps
  - Random turn (left/right)
  - Repeat

  If you add HC-SR04 ultrasonic sensor later:
  - If obstacle < threshold → stop → turn → continue
  Uncomment sensor section below when ready!
*/

// ── Optional: HC-SR04 Ultrasonic ─────────────────────────────────
// #define TRIG_PIN 18
// #define ECHO_PIN 19
// #define OBSTACLE_CM 20  // Stop if obstacle within 20cm

// long readDistanceCM() {
//   digitalWrite(TRIG_PIN, LOW);
//   delayMicroseconds(2);
//   digitalWrite(TRIG_PIN, HIGH);
//   delayMicroseconds(10);
//   digitalWrite(TRIG_PIN, LOW);
//   long duration = pulseIn(ECHO_PIN, HIGH, 30000);
//   return duration * 0.034 / 2;
// }

int walkSteps = 0;
int maxStepsBeforeTurn = 8; // Walk 8 steps then turn

// ─── SETUP ───────────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);
  Serial.println("Hexapod Booting...");

  Wire.begin(SDA_PIN, SCL_PIN);
  pca.begin();
  pca.setOscillatorFrequency(27000000);
  pca.setPWMFreq(50); // SG90 requires 50Hz

  delay(100);

  // Uncomment if using ultrasonic:
  // pinMode(TRIG_PIN, OUTPUT);
  // pinMode(ECHO_PIN, INPUT);

  homePosition();
  Serial.println("Home position set. Starting autonomous walk...");
  delay(1000);
}

// ─── MAIN LOOP ───────────────────────────────────────────────────
void loop() {
  // ── Optional obstacle check ──
  // long dist = readDistanceCM();
  // if (dist > 0 && dist < OBSTACLE_CM) {
  //   Serial.println("Obstacle! Turning...");
  //   homePosition();
  //   (random(2) == 0) ? turnLeft(4) : turnRight(4);
  //   homePosition();
  //   walkSteps = 0;
  //   return;
  // }

  // Walk forward
  if (walkSteps < maxStepsBeforeTurn) {
    Serial.print("Walking step: ");
    Serial.println(walkSteps + 1);
    walkForwardStep();
    walkSteps++;
  } else {
    // Random turn
    homePosition();
    int direction = random(2); // 0 = left, 1 = right
    int turnSteps = random(2, 6); // 2-5 turn steps

    if (direction == 0) {
      Serial.println("Turning LEFT");
      turnLeft(turnSteps);
    } else {
      Serial.println("Turning RIGHT");
      turnRight(turnSteps);
    }

    homePosition();
    walkSteps = 0;
    maxStepsBeforeTurn = random(5, 15); // Randomize next walk distance
  }
}
