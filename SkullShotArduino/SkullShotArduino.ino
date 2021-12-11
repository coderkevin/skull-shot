#include <Adafruit_NeoPixel.h>
#include <Servo.h>

#define OFF                   -1

#define SERIAL_BAUD_RATE      115200
#define PIN_LED               6
#define PIN_BUTTON            7
#define PIN_SERVO_RIGHT       9
#define PIN_SERVO_LEFT        10
#define PIN_PUMP_FORWARD      16
#define PIN_PUMP_REVERSE      17

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

#define SERVO_LEFT_OPEN       50
#define SERVO_LEFT_CLOSED     165
#define SERVO_RIGHT_OPEN      75
#define SERVO_RIGHT_CLOSED    0

#define PUMP_STOP          0
#define PUMP_FORE          1
#define PUMP_REVERSE       2

#define BUTTON_DEBOUNCE_DELAY 50

#define MODE_IDLE          0
#define MODE_PUMP_START    1
#define MODE_PUMP_RUN      2
#define MODE_PUMP_STOP     3

char pumpMode = PUMP_STOP;
bool buttonState = false;
bool lastButtonState = false;
bool buttonReleased = true;
unsigned long buttonLastDebounceTime = 0;

Adafruit_NeoPixel leds(LED_COUNT, PIN_LED, NEO_GRB + NEO_KHZ800);
Servo servoLeft;
Servo servoRight;

struct AnimationStep {
  char pump;
  float jaw;
  uint32_t eyeColor;
  uint32_t headTop;
  uint32_t headMid;
  uint32_t headLow;
};

uint32_t black = leds.Color(0, 0, 0);
uint32_t red = leds.Color(255, 0, 0);
uint32_t blackRed1 = leds.Color(50, 0, 0);
uint32_t blackRed2 = leds.Color(100, 0, 0);
uint32_t blackRed3 = leds.Color(150, 0, 0);
uint32_t blackRed4 = leds.Color(200, 0, 0);
uint32_t blackRed5 = leds.Color(255, 0, 0);
uint32_t redOrange1 = leds.Color(255, 50, 0);
uint32_t redOrange2 = leds.Color(255, 100, 0);
uint32_t redOrange3 = leds.Color(255, 175, 0);
uint32_t white = leds.Color(255, 255, 255);

// the setup function runs once when you press reset or power the board
void setup() {
  Serial.begin(SERIAL_BAUD_RATE);

  pinMode(PIN_PUMP_FORWARD, OUTPUT);
  pinMode(PIN_PUMP_REVERSE, OUTPUT);
  pinMode(PIN_BUTTON, INPUT_PULLUP);

  setPump(PUMP_STOP);

  if (Serial) {
    Serial.println("----- SkullShot -----");
  }

  leds.begin();
  leds.show();
  leds.setBrightness(100);
}

// the loop function runs over and over again forever
void loop() {
  bool interrupted = false;

  // Wait until the button is no longer held.
  while (buttonState) {
    updateButton(digitalRead(PIN_BUTTON) == LOW);
  }

  interrupted = idle();
  if (!interrupted) {
    return;
  }

  Serial.println("PUMP START");
  interrupted = pumpStart();
  if (interrupted) {
    Serial.println("Pump start interrupted");
    pumpStop();
    return;
  }

  Serial.println("PUMP RUN");
  interrupted = pumpRun();
  if (interrupted) {
    Serial.println("Pump run interrupted");
    pumpStop();
    return;
  }

  Serial.println("Pump run complete");
  Serial.println("PUMP STOP");
  pumpStop();
}

bool idle() {
  Serial.println("IDLE");

  AnimationStep redEyes[] = {
    { pump: PUMP_STOP, jaw: 0.0, eyeColor: red,        headTop: white, headMid: white, headLow: white },
  };

  AnimationStep redEyesFlicker[] = {
    { pump: PUMP_STOP, jaw: 0.0, eyeColor: red,        headTop: white, headMid: white, headLow: white },
    { pump: PUMP_STOP, jaw: 0.0, eyeColor: redOrange1, headTop: white, headMid: white, headLow: white },
    { pump: PUMP_STOP, jaw: 0.0, eyeColor: redOrange2, headTop: white, headMid: white, headLow: white },
    { pump: PUMP_STOP, jaw: 0.0, eyeColor: redOrange3, headTop: white, headMid: white, headLow: white },
    { pump: PUMP_STOP, jaw: 0.0, eyeColor: redOrange2, headTop: white, headMid: white, headLow: white },
    { pump: PUMP_STOP, jaw: 0.0, eyeColor: redOrange1, headTop: white, headMid: white, headLow: white },
    { pump: PUMP_STOP, jaw: 0.0, eyeColor: red,        headTop: white, headMid: white, headLow: white },
  };

  animate(redEyes, sizeof(redEyes) / sizeof(redEyes[0]), 5000, false);
  animate(redEyesFlicker, sizeof(redEyesFlicker) / sizeof(redEyesFlicker[0]), 50, false);
  return buttonState;
}

bool pumpStart() {
  AnimationStep jawOpen[] = {
    { pump: PUMP_STOP, jaw: 0.0,  eyeColor: blackRed1, headTop: white, headMid: white, headLow: white },
    { pump: PUMP_STOP, jaw: 0.1,  eyeColor: blackRed1, headTop: white, headMid: white, headLow: white },
    { pump: PUMP_STOP, jaw: 0.2 , eyeColor: blackRed1, headTop: white, headMid: white, headLow: white },
    { pump: PUMP_STOP, jaw: 0.3,  eyeColor: blackRed1, headTop: white, headMid: white, headLow: white },
    { pump: PUMP_STOP, jaw: 0.4,  eyeColor: blackRed1, headTop: white, headMid: white, headLow: white },
    { pump: PUMP_STOP, jaw: 0.45, eyeColor: blackRed1, headTop: white, headMid: white, headLow: white },
    { pump: PUMP_STOP, jaw: 0.5,  eyeColor: blackRed2, headTop: white, headMid: white, headLow: white },
    { pump: PUMP_STOP, jaw: 0.55, eyeColor: blackRed2, headTop: white, headMid: white, headLow: white },
    { pump: PUMP_STOP, jaw: 0.6,  eyeColor: blackRed2, headTop: white, headMid: white, headLow: white },
    { pump: PUMP_STOP, jaw: 0.65, eyeColor: blackRed2, headTop: white, headMid: white, headLow: white },
    { pump: PUMP_STOP, jaw: 0.7,  eyeColor: blackRed3, headTop: white, headMid: white, headLow: white },
    { pump: PUMP_STOP, jaw: 0.75, eyeColor: blackRed3, headTop: white, headMid: white, headLow: white },
    { pump: PUMP_STOP, jaw: 0.8,  eyeColor: blackRed4, headTop: white, headMid: white, headLow: white },
    { pump: PUMP_STOP, jaw: 0.85, eyeColor: blackRed4, headTop: white, headMid: white, headLow: white },
    { pump: PUMP_STOP, jaw: 0.9,  eyeColor: blackRed5, headTop: white, headMid: white, headLow: white },
    { pump: PUMP_STOP, jaw: 0.95, eyeColor: blackRed5, headTop: white, headMid: white, headLow: white },
    { pump: PUMP_STOP, jaw: 1.0,  eyeColor: red,       headTop: white, headMid: white, headLow: white },
    { pump: PUMP_STOP, jaw: 1.0,  eyeColor: red,       headTop: white, headMid: white, headLow: white },
  };

  return animate(jawOpen, sizeof(jawOpen) / sizeof(jawOpen[0]), 50, true);
}

bool pumpRun() {
  // 1.5 oz shot takes 6 seconds, so run this 6 times.
  AnimationStep pumpRun1Second[] = {
    { pump: PUMP_FORE, jaw: 1.0,  eyeColor: red,       headTop: white, headMid: white, headLow: white },
    { pump: PUMP_FORE, jaw: 1.0,  eyeColor: red,       headTop: white, headMid: white, headLow: white },
    { pump: PUMP_FORE, jaw: 1.0,  eyeColor: red,       headTop: white, headMid: white, headLow: white },
    { pump: PUMP_FORE, jaw: 1.0,  eyeColor: red,       headTop: white, headMid: white, headLow: white },
    { pump: PUMP_FORE, jaw: 1.0,  eyeColor: red,       headTop: white, headMid: white, headLow: white },
    { pump: PUMP_FORE, jaw: 1.0,  eyeColor: red,       headTop: white, headMid: white, headLow: white },
    { pump: PUMP_FORE, jaw: 1.0,  eyeColor: red,       headTop: white, headMid: white, headLow: white },
    { pump: PUMP_FORE, jaw: 1.0,  eyeColor: red,       headTop: white, headMid: white, headLow: white },
    { pump: PUMP_FORE, jaw: 1.0,  eyeColor: red,       headTop: white, headMid: white, headLow: white },
    { pump: PUMP_FORE, jaw: 1.0,  eyeColor: red,       headTop: white, headMid: white, headLow: white },
    { pump: PUMP_FORE, jaw: 1.0,  eyeColor: blackRed5, headTop: white, headMid: white, headLow: white },
    { pump: PUMP_FORE, jaw: 1.0,  eyeColor: blackRed5, headTop: white, headMid: white, headLow: white },
    { pump: PUMP_FORE, jaw: 1.0,  eyeColor: blackRed4, headTop: white, headMid: white, headLow: white },
    { pump: PUMP_FORE, jaw: 1.0,  eyeColor: blackRed4, headTop: white, headMid: white, headLow: white },
    { pump: PUMP_FORE, jaw: 1.0,  eyeColor: blackRed3, headTop: white, headMid: white, headLow: white },
    { pump: PUMP_FORE, jaw: 1.0,  eyeColor: blackRed3, headTop: white, headMid: white, headLow: white },
    { pump: PUMP_FORE, jaw: 1.0,  eyeColor: blackRed4, headTop: white, headMid: white, headLow: white },
    { pump: PUMP_FORE, jaw: 1.0,  eyeColor: blackRed4, headTop: white, headMid: white, headLow: white },
    { pump: PUMP_FORE, jaw: 1.0,  eyeColor: blackRed5, headTop: white, headMid: white, headLow: white },
    { pump: PUMP_FORE, jaw: 1.0,  eyeColor: blackRed5, headTop: white, headMid: white, headLow: white },
  };

  for (int second = 0; second < 6; second++) {
    if (animate(pumpRun1Second, sizeof(pumpRun1Second) / sizeof(pumpRun1Second[0]), 50, true)) {
      // We got interrupted. Return immediately.
      return true;
    }
  }
  return false;
}

void pumpStop() {
  AnimationStep pumpFinish[] = {
    // Reverse for 1 second
    { pump: PUMP_REVERSE, jaw: 1.0,  eyeColor: red, headTop: white, headMid: white, headLow: white },
    { pump: PUMP_REVERSE, jaw: 1.0,  eyeColor: red, headTop: white, headMid: white, headLow: white },
    { pump: PUMP_REVERSE, jaw: 1.0,  eyeColor: red, headTop: white, headMid: white, headLow: white },
    { pump: PUMP_REVERSE, jaw: 1.0,  eyeColor: red, headTop: white, headMid: white, headLow: white },
    { pump: PUMP_REVERSE, jaw: 1.0,  eyeColor: red, headTop: white, headMid: white, headLow: white },
    { pump: PUMP_REVERSE, jaw: 1.0,  eyeColor: red, headTop: white, headMid: white, headLow: white },
    { pump: PUMP_REVERSE, jaw: 1.0,  eyeColor: red, headTop: white, headMid: white, headLow: white },
    { pump: PUMP_REVERSE, jaw: 1.0,  eyeColor: red, headTop: white, headMid: white, headLow: white },
    { pump: PUMP_REVERSE, jaw: 1.0,  eyeColor: red, headTop: white, headMid: white, headLow: white },
    { pump: PUMP_REVERSE, jaw: 1.0,  eyeColor: red, headTop: white, headMid: white, headLow: white },
    { pump: PUMP_REVERSE, jaw: 1.0,  eyeColor: red, headTop: white, headMid: white, headLow: white },
    { pump: PUMP_REVERSE, jaw: 1.0,  eyeColor: red, headTop: white, headMid: white, headLow: white },
    { pump: PUMP_REVERSE, jaw: 1.0,  eyeColor: red, headTop: white, headMid: white, headLow: white },
    { pump: PUMP_REVERSE, jaw: 1.0,  eyeColor: red, headTop: white, headMid: white, headLow: white },
    { pump: PUMP_REVERSE, jaw: 1.0,  eyeColor: red, headTop: white, headMid: white, headLow: white },
    { pump: PUMP_REVERSE, jaw: 1.0,  eyeColor: red, headTop: white, headMid: white, headLow: white },
    { pump: PUMP_REVERSE, jaw: 1.0,  eyeColor: red, headTop: white, headMid: white, headLow: white },
    { pump: PUMP_REVERSE, jaw: 1.0,  eyeColor: red, headTop: white, headMid: white, headLow: white },
    { pump: PUMP_REVERSE, jaw: 1.0,  eyeColor: red, headTop: white, headMid: white, headLow: white },
    { pump: PUMP_REVERSE, jaw: 1.0,  eyeColor: red, headTop: white, headMid: white, headLow: white },
  };

  AnimationStep jawSnapShut[] = {
    { pump: PUMP_STOP, jaw: 1.0, eyeColor: red,       headTop: white, headMid: white, headLow: white },
    { pump: PUMP_STOP, jaw: 0.8, eyeColor: red,       headTop: white, headMid: white, headLow: white },
    { pump: PUMP_STOP, jaw: 0.6, eyeColor: blackRed5, headTop: white, headMid: white, headLow: white },
    { pump: PUMP_STOP, jaw: 0.4, eyeColor: blackRed5, headTop: white, headMid: white, headLow: white },
    { pump: PUMP_STOP, jaw: 0.2, eyeColor: blackRed4, headTop: white, headMid: white, headLow: white },
    { pump: PUMP_STOP, jaw: 0.0, eyeColor: blackRed4, headTop: white, headMid: white, headLow: white },
    { pump: PUMP_STOP, jaw: 0.0, eyeColor: blackRed3, headTop: white, headMid: white, headLow: white },
    { pump: PUMP_STOP, jaw: 0.0, eyeColor: blackRed2, headTop: white, headMid: white, headLow: white },
    { pump: PUMP_STOP, jaw: 0.0, eyeColor: blackRed1, headTop: white, headMid: white, headLow: white },
    { pump: PUMP_STOP, jaw: 0.0, eyeColor: blackRed1, headTop: white, headMid: white, headLow: white },
  };

  animate(pumpFinish, sizeof(pumpFinish) / sizeof(pumpFinish[0]), 50, true);
  animate(jawSnapShut, sizeof(jawSnapShut) / sizeof(jawSnapShut[0]), 50, true);
}

bool animate(AnimationStep* steps, int length, int stepDelay, bool useJaw) {
  Serial.println("animate");
  if (useJaw) {
    servoLeft.attach(PIN_SERVO_LEFT);
    servoRight.attach(PIN_SERVO_RIGHT);
  }

  for (int stepIndex = 0; stepIndex < length; stepIndex++) {
    Serial.print(".");
    AnimationStep& step = steps[stepIndex];

    if (updateButton(digitalRead(PIN_BUTTON) == LOW)) {
      Serial.println();
      Serial.println("Button changed:" + buttonState);
      // The mode changed
      if (useJaw) {
        servoLeft.detach();
        servoRight.detach();
      }
      return true;
    }

    if (useJaw) {
      setJawPosition(step.jaw);
    }

    setPump(step.pump);

    ledsEyeColor(step.eyeColor);
    ledsHeadTop(step.headTop);
    ledsHeadMid(step.headMid);
    ledsHeadLow(step.headLow);
    ledsJaw(step.headLow);
    leds.show();

    delay(stepDelay);
  }
  Serial.println();

  if (useJaw) {
    servoLeft.detach();
    servoRight.detach();
  }
  return false;
}

void setPump(char mode) {
  if (pumpMode != mode) {
    Serial.print("PUMP:");
    Serial.println(mode == PUMP_FORE ? "FORE" : (mode == PUMP_REVERSE ? "REVERSE" : "STOP"));

    pumpMode = mode;
    digitalWrite(PIN_PUMP_FORWARD, mode == PUMP_FORE ? HIGH : LOW);
    digitalWrite(PIN_PUMP_REVERSE, mode == PUMP_REVERSE ? HIGH : LOW);
  }
}

bool updateButton(boolean pressed) {
  if (pressed != lastButtonState) {
    buttonLastDebounceTime = millis();
  }

  if ((millis() - buttonLastDebounceTime) > BUTTON_DEBOUNCE_DELAY) {
    if (pressed != buttonState) {
      buttonState = pressed;
      Serial.print("Button: ");
      Serial.println(pressed);
      return true;
    }
  }

  lastButtonState = pressed;
  return false;
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
}

void ledsHeadTop(uint32_t color) {
  leds.setPixelColor(LED_BACK_TOP, color);
  leds.setPixelColor(LED_MID_TOP_CENTER, color);
  leds.setPixelColor(LED_MID_TOP_LEFT, color);
  leds.setPixelColor(LED_MID_TOP_RIGHT, color);
  leds.setPixelColor(LED_FACE_TOP_CENTER, color);
  leds.setPixelColor(LED_FACE_TOP_LEFT, color);
  leds.setPixelColor(LED_FACE_TOP_RIGHT, color);
}

void ledsHeadMid(uint32_t color) {
  leds.setPixelColor(LED_FACE_FORE_LEFT, color);
  leds.setPixelColor(LED_FACE_FORE_RIGHT, color);
  leds.setPixelColor(LED_BACK_MID_LEFT, color);
  leds.setPixelColor(LED_BACK_MID_RIGHT, color);
  leds.setPixelColor(LED_MID_MID_LEFT, color);
  leds.setPixelColor(LED_MID_MID_RIGHT, color);
  leds.setPixelColor(LED_FACE_MID_LEFT, color);
  leds.setPixelColor(LED_FACE_MID_RIGHT, color);
}

void ledsHeadLow(uint32_t color) {
  leds.setPixelColor(LED_BACK_BOT_LEFT, color);
  leds.setPixelColor(LED_BACK_BOT_RIGHT, color);
  leds.setPixelColor(LED_MID_BOT_LEFT, color);
  leds.setPixelColor(LED_MID_BOT_RIGHT, color);
  leds.setPixelColor(LED_FACE_BOT_LEFT, color);
  leds.setPixelColor(LED_FACE_BOT_RIGHT, color);
  leds.setPixelColor(LED_FACE_DOWN_LEFT, color);
  leds.setPixelColor(LED_FACE_DOWN_RIGHT, color);
}

void ledsJaw(uint32_t color) {
  leds.setPixelColor(LED_JAW_LEFT_1, color);
  leds.setPixelColor(LED_JAW_LEFT_2, color);
  leds.setPixelColor(LED_JAW_LEFT_3, color);
  leds.setPixelColor(LED_JAW_RIGHT_1, color);
  leds.setPixelColor(LED_JAW_RIGHT_2, color);
  leds.setPixelColor(LED_JAW_RIGHT_3, color);
}
