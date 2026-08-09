#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_ADXL345_U.h>

// ===== PIN DEFINITIONS =====
#define IR_PIN    18
#define TRIG_PIN  16
#define ECHO_PIN  15
#define SDA_PIN   20
#define SCL_PIN   19
#define LED_PIN   13
#define LED_FALL  14
#define BUZZ_PIN  4

// ===== THRESHOLDS =====
#define FALL_THRESHOLD  20.0
#define DANGER_DIST     30
#define WARNING_DIST    50

Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified(12345);

// ===== IR SENSOR FUNCTION =====
bool isObjectDetected() {
  int count = 0;
  for (int i = 0; i < 10; i++) {
    if (digitalRead(IR_PIN) == LOW) {
      count++;
    }
    delay(5);  // reduced from 10 to 5
  }
  return count >= 7;
}

// ===== ULTRASONIC FUNCTION =====
float getDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  // reduced timeout from 30000 to 15000
  long duration = pulseIn(ECHO_PIN, HIGH, 15000);
  if (duration == 0) return -1; // return -1 if no reading
  return duration * 0.034 / 2;
}

// ===== ALERT FUNCTIONS =====
void irAlert() {
  digitalWrite(LED_PIN,  HIGH);
  digitalWrite(BUZZ_PIN, HIGH);
  delay(200);
  digitalWrite(LED_PIN,  LOW);
  digitalWrite(BUZZ_PIN, LOW);
  delay(200);
}

void ultrasonicWarning() {
  digitalWrite(LED_PIN, HIGH);
  delay(200);
  digitalWrite(LED_PIN, LOW);
  delay(200);
}

void ultrasonicDanger() {
  digitalWrite(LED_PIN,  HIGH);
  digitalWrite(BUZZ_PIN, HIGH);
  delay(100);
  digitalWrite(LED_PIN,  LOW);
  digitalWrite(BUZZ_PIN, LOW);
  delay(100);
}

void fallAlert() {
  Serial.println("FALL DETECTED!");
  for (int i = 0; i < 5; i++) {
    digitalWrite(LED_FALL, HIGH);
    digitalWrite(BUZZ_PIN, HIGH);
    delay(300);
    digitalWrite(LED_FALL, LOW);
    digitalWrite(BUZZ_PIN, LOW);
    delay(300);
  }
}

void allOff() {
  digitalWrite(LED_PIN,  LOW);
  digitalWrite(LED_FALL, LOW);
  digitalWrite(BUZZ_PIN, LOW);
}

// ===== SETUP =====
void setup() {
  Serial.begin(115200);
  Wire.begin(SDA_PIN, SCL_PIN);

  pinMode(IR_PIN,    INPUT);
  pinMode(TRIG_PIN,  OUTPUT);
  pinMode(ECHO_PIN,  INPUT);
  pinMode(LED_PIN,   OUTPUT);
  pinMode(LED_FALL,  OUTPUT);
  pinMode(BUZZ_PIN,  OUTPUT);

  allOff();

  if (!accel.begin()) {
    Serial.println("ADXL345 not detected! Check wiring.");
    while (1);
  }

  accel.setRange(ADXL345_RANGE_16_G);
  Serial.println("Smart Helmet Ready!");
}

// ===== MAIN LOOP =====
void loop() {

  // ----- 1. FALL DETECTION (highest priority) -----
  sensors_event_t event;
  accel.getEvent(&event);
  float totalAccel = sqrt(
    event.acceleration.x * event.acceleration.x +
    event.acceleration.y * event.acceleration.y +
    event.acceleration.z * event.acceleration.z
  );

  Serial.print("Accel: ");
  Serial.println(totalAccel);

  if (totalAccel > FALL_THRESHOLD) {
    fallAlert();
    return;
  }

  // ----- 2. ULTRASONIC DISTANCE CHECK -----
  float distance = getDistance();
  
  if (distance == -1) {
    Serial.println("Ultrasonic: no reading");
  } else {
    Serial.print("Distance: ");
    Serial.print(distance);
    Serial.println(" cm");
  }

  if (distance > 0 && distance <= DANGER_DIST) {
    Serial.println("DANGER!");
    ultrasonicDanger();
    return;
  } else if (distance > DANGER_DIST && distance <= WARNING_DIST) {
    Serial.println("WARNING!");
    ultrasonicWarning();
    return;
  }

  // ----- 3. IR SENSOR CHECK -----
  if (isObjectDetected()) {
    Serial.println("IR - Object detected!");
    irAlert();
    return;
  }

  // ----- ALL CLEAR -----
  allOff();
}