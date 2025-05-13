#include <Arduino.h>
#include <SPI.h>

#include <TFT_eSPI.h>

#include <duktape.h>


#define DEBOUNCE_TIME 50
#define JOYSTICK_MIN_THRESH 100
#define JOYSTICK_MAX_THRESH 900

#define JOYSTICK_X_PIN 27
#define JOYSTICK_Y_PIN 26

#define ACTION_BUTTON_PIN 13
#define RESTART_BUTTON_PIN 14
#define PAUSE_BUTTON_PIN 15

#define VIBRATION_PIN 12

TFT_eSPI tft = TFT_eSPI();

duk_context *ctx = duk_create_heap_default();


duk_ret_t jsVibrate(duk_context *ctx) {
    int intensity = duk_get_int(ctx, 0);
    analogWrite(VIBRATION_PIN, intensity);
    return 0;
}


volatile unsigned long actionPressTime = 0;
void actionRelease();
void actionPress() {
    noInterrupts();

    if (millis() - actionPressTime < DEBOUNCE_TIME) {
        interrupts();
        return;
    }

    actionPressTime = millis();
    attachInterrupt(digitalPinToInterrupt(ACTION_BUTTON_PIN), actionRelease, RISING);

    // handle button press
    duk_eval_string(ctx, "vibrate(255);");

    interrupts();
}
void actionRelease() {
    noInterrupts();

    if (millis() - actionPressTime < DEBOUNCE_TIME) {
        interrupts();
        return;
    }

    actionPressTime = millis();
    attachInterrupt(digitalPinToInterrupt(ACTION_BUTTON_PIN), actionPress, FALLING);

    interrupts();
}

volatile unsigned long restartPressTime = 0;
void restartRelease();
void restartPress() {
    noInterrupts();

    if (millis() - restartPressTime < DEBOUNCE_TIME) {
        interrupts();
        return;
    }

    restartPressTime = millis();
    attachInterrupt(digitalPinToInterrupt(RESTART_BUTTON_PIN), restartRelease, RISING);

    // handle button press

    interrupts();
}
void restartRelease() {
    noInterrupts();

    if (millis() - restartPressTime < DEBOUNCE_TIME) {
        interrupts();
        return;
    }

    restartPressTime = millis();
    attachInterrupt(digitalPinToInterrupt(RESTART_BUTTON_PIN), restartPress, FALLING);

    interrupts();
}

volatile unsigned long pausePressTime = 0;
void pauseRelease();
void pausePress() {
    noInterrupts();

    if (millis() - pausePressTime < DEBOUNCE_TIME) {
        interrupts();
        return;
    }

    pausePressTime = millis();
    attachInterrupt(digitalPinToInterrupt(PAUSE_BUTTON_PIN), pauseRelease, RISING);

    // handle button press
    tft.fillScreen(TFT_BLACK);
    tft.drawRect(20, 20, 40, 40, TFT_GREEN);

    interrupts();
}
void pauseRelease() {
    noInterrupts();

    if (millis() - pausePressTime < DEBOUNCE_TIME) {
        interrupts();
        return;
    }

    pausePressTime = millis();
    attachInterrupt(digitalPinToInterrupt(PAUSE_BUTTON_PIN), pausePress, FALLING);

    interrupts();
}


void setup() {
    Serial.begin(9600);

    pinMode(JOYSTICK_X_PIN, INPUT);
    pinMode(JOYSTICK_Y_PIN, INPUT);

    pinMode(ACTION_BUTTON_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(ACTION_BUTTON_PIN), actionPress, FALLING);
    pinMode(RESTART_BUTTON_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(RESTART_BUTTON_PIN), restartPress, FALLING);
    pinMode(PAUSE_BUTTON_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(PAUSE_BUTTON_PIN), pausePress, FALLING);

    pinMode(VIBRATION_PIN, OUTPUT);

    tft.init();

    // takes about 15ms
    tft.fillScreen(TFT_BLACK);
    tft.drawRect(20, 20, 40, 40, TFT_RED);

    // make jsVibrate available in the global scope
    duk_push_global_object(ctx);
    duk_push_c_function(ctx, jsVibrate, 1);
    duk_put_prop_string(ctx, -2, "vibrate");
    duk_pop(ctx); // pop global object
}

bool pressedLeft = false;
bool pressedRight = false;
bool pressedUp = false;
bool pressedDown = false;
void loop() {
    //read joystick values (takes near 100us)
    int joystickX = analogRead(JOYSTICK_X_PIN);
    int joystickY = analogRead(JOYSTICK_Y_PIN);

    if (joystickX < JOYSTICK_MIN_THRESH) {
        if (!pressedLeft) {
            pressedLeft = true;

            // handle joystick left press
            tft.fillScreen(TFT_BLACK);
            tft.drawRect(20, 20, 40, 40, TFT_WHITE);
        }
    } else {
        pressedLeft = false;
    }

    if (joystickX > JOYSTICK_MAX_THRESH) {
        if (!pressedRight) {
            pressedRight = true;

            // handle joystick right press
        }
    } else {
        pressedRight = false;
    }

    if (joystickY < JOYSTICK_MIN_THRESH) {
        if (!pressedUp) {
            pressedUp = true;

            // handle joystick up press
            duk_eval_string(ctx, "vibrate(166);");
        }
    } else {
        pressedUp = false;
    }

    if (joystickY > JOYSTICK_MAX_THRESH) {
        if (!pressedDown) {
            pressedDown = true;

            // handle joystick down press
            duk_eval_string(ctx, "vibrate(0);");
        }
    } else {
        pressedDown = false;
    }


    delay(100);
}
