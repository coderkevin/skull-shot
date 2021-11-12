#include <Adafruit_NeoPixel.h>
#include <Servo.h>

#define SERIAL_BAUD_RATE      115200
#define PIN_LED               6
#define PIN_BUTTON            7
#define PIN_SERVO_RIGHT       9
#define PIN_SERVO_LEFT        10
#define PIN_PUMP_FORWARD      16

#define LED_COUNT             31

#define LED_BACK_TOP          28
#define LED_BACK_MID_LEFT     27
#define LED_BACK_MID_RIGHT    29
#define LED_BACK_BOT_LEFT     26
#define LED_BACK_BOT_RIGHT    30

#define LED_MID_TOP_CENTER    22
#define LED_MID_TOP_LEFT      23
#define LED_MID_TOP_RIGHT     21
#define LED_MID_MID_LEFT      24
#define LED_MID_MID_RIGHT     20
#define LED_MID_BOT_LEFT      25
#define LED_MID_BOT_RIGHT     19

#define LED_FACE_TOP_CENTER   9
#define LED_FACE_TOP_LEFT     8
#define LED_FACE_TOP_RIGHT    10
#define LED_FACE_FORE_LEFT    7
#define LED_FACE_FORE_RIGHT   11
#define LED_FACE_MID_LEFT     6
#define LED_FACE_MID_RIGHT    12
#define LED_FACE_BOT_LEFT     0
#define LED_FACE_BOT_RIGHT    18
#define LED_FACE_DOWN_LEFT    1
#define LED_FACE_DOWN_RIGHT   14

#define LED_JAW_LEFT_1        2
#define LED_JAW_LEFT_2        3
#define LED_JAW_LEFT_3        4

#define LED_JAW_RIGHT_1       15
#define LED_JAW_RIGHT_2       16
#define LED_JAW_RIGHT_3       17

#define LED_EYE_LEFT          5
#define LED_EYE_RIGHT         13

#define SERVO_LEFT_OPEN       60
#define SERVO_LEFT_CLOSED     165
#define SERVO_RIGHT_OPEN      75
#define SERVO_RIGHT_CLOSED    0

#define PUMP_STOPPED          0
#define PUMP_FORWARD          1
#define PUMP_REVERSE          2

#define BUTTON_DEBOUNCE_DELAY 50

char pumpMode = PUMP_STOPPED;
bool buttonState = false;
bool lastButtonState = false;
unsigned long buttonLastDebounceTime = 0;

Adafruit_NeoPixel leds(LED_COUNT, PIN_LED, NEO_GRB + NEO_KHZ800);
Servo servoLeft;
Servo servoRight;

struct AnimationStep {
  float jaw;
  uint32_t eyeColor;
};

AnimationStep redEyesFlicker[] = {
  { jaw: 0.0, eyeColor: leds.Color(255, 0, 0) },
  { jaw: 0.0, eyeColor: leds.Color(255, 20, 0) },
  { jaw: 0.0, eyeColor: leds.Color(255, 40, 0) },
  { jaw: 0.0, eyeColor: leds.Color(255, 60, 0) },
  { jaw: 0.0, eyeColor: leds.Color(255, 40, 0) },
  { jaw: 0.0, eyeColor: leds.Color(255, 20, 0) },
  { jaw: 0.0, eyeColor: leds.Color(255, 0, 0) },
  { jaw: 0.0, eyeColor: leds.Color(255, 20, 0) },
  { jaw: 0.0, eyeColor: leds.Color(255, 40, 0) },
  { jaw: 0.0, eyeColor: leds.Color(255, 60, 0) },
  { jaw: 0.0, eyeColor: leds.Color(255, 40, 0) },
  { jaw: 0.0, eyeColor: leds.Color(255, 20, 0) },
  { jaw: 0.0, eyeColor: leds.Color(255, 0, 0) },
};

AnimationStep jawOpen[] = {
  { jaw: 0.0,  eyeColor: -1 },
  { jaw: 0.05, eyeColor: -1 },
  { jaw: 0.1 , eyeColor: -1 },
  { jaw: 0.15, eyeColor: -1 },
  { jaw: 0.2,  eyeColor: -1 },
  { jaw: 0.25, eyeColor: -1 },
  { jaw: 0.3,  eyeColor: -1 },
  { jaw: 0.35, eyeColor: -1 },
  { jaw: 0.4,  eyeColor: -1 },
  { jaw: 0.45, eyeColor: -1 },
  { jaw: 0.5,  eyeColor: -1 },
  { jaw: 0.55, eyeColor: -1 },
  { jaw: 0.6,  eyeColor: -1 },
  { jaw: 0.65, eyeColor: -1 },
  { jaw: 0.7,  eyeColor: -1 },
  { jaw: 0.75, eyeColor: -1 },
  { jaw: 0.8,  eyeColor: -1 },
  { jaw: 0.85, eyeColor: -1 },
  { jaw: 0.9,  eyeColor: -1 },
  { jaw: 0.95, eyeColor: -1 },
  { jaw: 1.0,  eyeColor: -1 },
};

AnimationStep jawSnapShut[] = {
  { jaw: 1.0, eyeColor: -1 },
  { jaw: 0.8, eyeColor: -1 },
  { jaw: 0.6, eyeColor: -1 },
  { jaw: 0.4, eyeColor: -1 },
  { jaw: 0.2, eyeColor: -1 },
  { jaw: 0.0, eyeColor: -1 },
  { jaw: 0.0, eyeColor: -1 },
  { jaw: 0.0, eyeColor: -1 },
  { jaw: 0.0, eyeColor: -1 },
  { jaw: 0.0, eyeColor: -1 },
};

// the setup function runs once when you press reset or power the board
void setup() {
  Serial.begin(SERIAL_BAUD_RATE);

  pinMode(PIN_PUMP_FORWARD, OUTPUT);
  pinMode(PIN_BUTTON, INPUT_PULLUP);

  setPump(PUMP_STOPPED);

  if (Serial) {
    Serial.println("----- SkullShot -----");
  }

  leds.begin();
  leds.show();
  leds.setBrightness(100);
  //ledsFillColor(leds.Color(255, 255, 255));
}

// the loop function runs over and over again forever
void loop() {
  //updateButton(digitalRead(PIN_BUTTON) == LOW);
  //setPump(PUMP_FORWARD);

  ledsEyeColor(leds.Color(255, 0, 0));
  delay(15000);
  animate(redEyesFlicker, sizeof(redEyesFlicker) / sizeof(redEyesFlicker[0]), 50, false);

  //delay(5000);
  //animate(jawOpen, sizeof(jawOpen) / sizeof(jawOpen[0]), 50);
  //delay(5000);
  //animate(jawSnapShut, sizeof(jawSnapShut) / sizeof(jawSnapShut[0]), 50);
}

void setPump(char mode) {
  if (pumpMode != mode) {
    pumpMode = mode;
    digitalWrite(PIN_PUMP_FORWARD, mode == PUMP_FORWARD ? HIGH : LOW);

    Serial.print("PUMP: ");
    Serial.println(mode == PUMP_FORWARD ? "FORWARD" : mode == PUMP_REVERSE ? "REVERSE" : "STOPPED");
  }
}

void updateButton(boolean pressed) {
  if (pressed != lastButtonState) {
    buttonLastDebounceTime = millis();
  }

  if ((millis() - buttonLastDebounceTime) > BUTTON_DEBOUNCE_DELAY) {
    if (pressed != buttonState) {
      buttonState = pressed;

      if (pressed) {
        Serial.println("Button pressed");
        setPump(PUMP_FORWARD);
      } else {
        Serial.println("Button released");
        setPump(PUMP_STOPPED);
      }
    }
  }

  lastButtonState = pressed;
}

void animate(AnimationStep* steps, int length, int stepDelay, bool useJaw) {
  if (useJaw) {
    servoLeft.attach(PIN_SERVO_LEFT);
    servoRight.attach(PIN_SERVO_RIGHT);
  }

  for (int stepIndex = 0; stepIndex < length; stepIndex++) {
    AnimationStep& step = steps[stepIndex];

    if (useJaw) {
      setJawPosition(step.jaw);
    }

    if (step.eyeColor != -1) {
      ledsEyeColor(step.eyeColor);
    }

    delay(stepDelay);
  }

  if (useJaw) {
    servoLeft.detach();
    servoRight.detach();
  }
}

/**
 * Sets the Jaw position.
 * @param position 0.0 = closed, 1.0 = open
 */
void setJawPosition(float position) {
  int leftValue = ((SERVO_LEFT_CLOSED - SERVO_LEFT_OPEN) * (1-position)) + SERVO_LEFT_OPEN;
  int rightValue = ((SERVO_RIGHT_OPEN - SERVO_RIGHT_CLOSED) * position) + SERVO_RIGHT_CLOSED;

  servoLeft.write(leftValue);
  servoRight.write(rightValue);
}

void ledsFillColor(uint32_t color) {
  for (int i = 0; i < leds.numPixels(); i++) {
    leds.setPixelColor(i, color);
  }
  leds.show();
}

void ledsEyeColor(uint32_t color) {
  leds.setPixelColor(LED_EYE_LEFT, color);
  leds.setPixelColor(LED_EYE_RIGHT, color);
  leds.show();
}
