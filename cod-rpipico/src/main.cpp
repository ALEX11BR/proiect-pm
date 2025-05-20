#include <limits.h>

#include <Arduino.h>
#include "LittleFS.h"
#include "pico/stdlib.h"
#include <SPI.h>

#include <TFT_eSPI.h>

#include <duktape.h>


#define JOYSTICK_X_PIN 27
#define JOYSTICK_Y_PIN 26

#define ACTION_BUTTON_PIN 13
#define RESTART_BUTTON_PIN 14
#define PAUSE_BUTTON_PIN 15

#define VIBRATION_PIN 12

#define DRAW_TIME 100
#define JOYSTICK_READ_TIME 50
#define SUBTICK_TIME 50

#define DEBOUNCE_TIME 50
#define JOYSTICK_MIN_THRESH 100
#define JOYSTICK_MAX_THRESH 900

#define MAIN_SCREEN_WIDTH 10
#define MAIN_SCREEN_HEIGHT 20
#define CELL_SIZE 8

#define SCREEN_RENDER_0 0
#define SCREEN_RENDER_1 1
#define SCREEN_RENDER_HALF 2
#define SCREEN_NO_RENDER 0x80

#define BACKGROUND_COLOR TFT_GREENYELLOW
#define FOREGROUND_COLOR TFT_BLACK
#define SECONDARY_COLOR 0b00111'001111'00111


volatile bool screenUpdate = false;
volatile uint8_t mainScreenCells[MAIN_SCREEN_HEIGHT][MAIN_SCREEN_WIDTH];

TFT_eSPI tft = TFT_eSPI();

duk_context *ctx = duk_create_heap_default();

repeating_timer drawTimer;
repeating_timer joystickTimer;
repeating_timer tickTimer;

volatile bool isPaused = false;
volatile long vibrateUntil = LONG_MAX;


bool drawTimerCallback(repeating_timer *t) {
    noInterrupts();

    if (isPaused) {
        interrupts();
        return true;
    }

    if (screenUpdate) {
        screenUpdate = false;

        // draw the main screen
        for (int y = 0; y < MAIN_SCREEN_HEIGHT; y++) {
            for (int x = 0; x < MAIN_SCREEN_WIDTH; x++) {
                if (mainScreenCells[y][x] != SCREEN_NO_RENDER) {
                    switch (mainScreenCells[y][x]) {
                        case SCREEN_RENDER_0:
                            tft.fillRect(x * CELL_SIZE, y * CELL_SIZE, CELL_SIZE, CELL_SIZE, BACKGROUND_COLOR);
                            break;
                        case SCREEN_RENDER_1:
                            tft.drawRect(x * CELL_SIZE + 1, y * CELL_SIZE + 1, CELL_SIZE - 1, CELL_SIZE - 1, FOREGROUND_COLOR);
                            tft.fillRect(x * CELL_SIZE + 3, y * CELL_SIZE + 3, CELL_SIZE - 5, CELL_SIZE - 5, FOREGROUND_COLOR);
                            break;
                        case SCREEN_RENDER_HALF:
                            tft.drawRect(x * CELL_SIZE + 1, y * CELL_SIZE + 1, CELL_SIZE - 1, CELL_SIZE - 1, SECONDARY_COLOR);
                            tft.fillRect(x * CELL_SIZE + 3, y * CELL_SIZE + 3, CELL_SIZE - 5, CELL_SIZE - 5, SECONDARY_COLOR);
                            break;
                    }
                    mainScreenCells[y][x] = SCREEN_NO_RENDER;
                }
            }
        }
    }

    interrupts();
    return true;
}

volatile bool pressedLeft = false;
volatile bool pressedRight = false;
volatile bool pressedUp = false;
volatile bool pressedDown = false;
bool readJoystickTimerCallback(repeating_timer *t) {
    noInterrupts();

    if (isPaused) {
        interrupts();
        return true;
    }

    //read joystick values (takes near 100us)
    int joystickX = analogRead(JOYSTICK_X_PIN);
    int joystickY = analogRead(JOYSTICK_Y_PIN);

    if (joystickX < JOYSTICK_MIN_THRESH) {
        if (!pressedLeft) {
            pressedLeft = true;

            // handle joystick left press
            duk_eval_string(ctx, "handleLeft();");
        }
    } else {
        pressedLeft = false;
    }

    if (joystickX > JOYSTICK_MAX_THRESH) {
        if (!pressedRight) {
            pressedRight = true;

            // handle joystick right press
            duk_eval_string(ctx, "handleRight();");
        }
    } else {
        pressedRight = false;
    }

    if (joystickY < JOYSTICK_MIN_THRESH) {
        if (!pressedUp) {
            pressedUp = true;

            // handle joystick up press
            duk_eval_string(ctx, "handleUp();");
        }
    } else {
        pressedUp = false;
    }

    if (joystickY > JOYSTICK_MAX_THRESH) {
        if (!pressedDown) {
            pressedDown = true;

            // handle joystick down press
            duk_eval_string(ctx, "handleDown();");
        }
    } else {
        pressedDown = false;
    }

    interrupts();
    return true;
}

volatile long subticks = 0;
bool tickTimerCallback(repeating_timer *t) {
    noInterrupts();
    watchdog_update();

    if (isPaused) {
        interrupts();
        return true;
    }

    subticks++;
    if (subticks >= 10) {
        subticks = 0;
        // handle tick
        duk_eval_string(ctx, "handleTick();");
    }

    interrupts();
    return true;
}

int64_t vibrateEndCallback(alarm_id_t t, void *data) {
    analogWrite(VIBRATION_PIN, 0);
    return 0;
}

int64_t tickTimerAddCallback(alarm_id_t t, void *data) {
    add_repeating_timer_ms(SUBTICK_TIME, tickTimerCallback, NULL, &tickTimer);
    return 0;
}


duk_ret_t brickVibrate(duk_context *ctx) {
    // Parameters:
    // * intensity: 0-255
    // * duration: duration in milliseconds
    int intensity = duk_get_int(ctx, 0);
    int duration = duk_get_int(ctx, 1);

    analogWrite(VIBRATION_PIN, intensity);
    add_alarm_in_ms(duration, vibrateEndCallback, NULL, false);
    return 0;
}

duk_ret_t brickMainDraw(duk_context *ctx) {
    // Parameters:
    // * x: x coordinate
    // * y: y coordinate
    // * color: color to draw (1 = on, 0 = off)
    int x = duk_get_int(ctx, 0);
    int y = duk_get_int(ctx, 1);
    int color = duk_get_int(ctx, 2);

    if (x < 0 || x >= MAIN_SCREEN_WIDTH || y < 0 || y >= MAIN_SCREEN_HEIGHT) {
        return 0; // Out of bounds
    }

    switch (color) {
        case 0:
            mainScreenCells[y][x] = SCREEN_RENDER_0;
            break;
        case 1:
            mainScreenCells[y][x] = SCREEN_RENDER_1;
            break;
        case 2:
            mainScreenCells[y][x] = SCREEN_RENDER_HALF;
            break;
    }
    screenUpdate = true;

    return 0;
}

duk_ret_t brickTickReset(duk_context *ctx) {
    subticks = 0;
    return 0;
}

const duk_function_list_entry brickFunctionList[] = {
    {"brickVibrate", brickVibrate, 2},
    {"brickMainDraw", brickMainDraw, 3},
    {"brickTickReset", brickTickReset, 0},
    {NULL, NULL, 0}
};


volatile unsigned long actionPressTime = 0;
void actionRelease();
void actionPress() {
    noInterrupts();

    if (millis() - actionPressTime < DEBOUNCE_TIME) {
        interrupts();
        return;
    }

    if (isPaused) {
        interrupts();
        return;
    }

    actionPressTime = millis();
    attachInterrupt(digitalPinToInterrupt(ACTION_BUTTON_PIN), actionRelease, RISING);

    // handle button press
    duk_eval_string(ctx, "handleAction();");

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

    watchdog_reboot(0, 0, 0);

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
    isPaused = !isPaused;

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
    Serial.begin(115200);

    tft.init();

    // takes about 15ms
    //tft.fillScreen(TFT_BLACK);
    //tft.drawRect(60, 20, 40, 40, TFT_RED);

    duk_push_global_object(ctx);
    duk_put_function_list(ctx, -1, brickFunctionList);
    duk_pop(ctx);

    pinMode(JOYSTICK_X_PIN, INPUT);
    pinMode(JOYSTICK_Y_PIN, INPUT);

    pinMode(ACTION_BUTTON_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(ACTION_BUTTON_PIN), actionPress, FALLING);
    pinMode(RESTART_BUTTON_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(RESTART_BUTTON_PIN), restartPress, FALLING);
    pinMode(PAUSE_BUTTON_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(PAUSE_BUTTON_PIN), pausePress, FALLING);

    pinMode(VIBRATION_PIN, OUTPUT);

    alarm_pool_init_default();

    // boot screen
    tft.fillScreen(TFT_YELLOW);
    tft.setTextSize(4);
    tft.setTextColor(TFT_BLACK);
    tft.drawString("Brick", 7, 7, 1);
    tft.drawString("9999", 7, 40, 1);
    tft.drawString("games", 7, 65, 1);

    tft.fillRect(0, 160 - 34, 128, 34, TFT_BLACK);
    tft.drawCircle(17, 160 - 17, 15, TFT_WHITE);
    tft.drawLine(12, 160 - 17, 22, 160 - 17, TFT_WHITE);
    tft.drawLine(17, 160 - 32, 17 - 10, 160 - 6, TFT_WHITE);
    tft.drawLine(17, 160 - 32, 17 + 10, 160 - 6, TFT_WHITE);
    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE);
    tft.drawString("ALEX POPA", 36, 160 - 21, 1);

    delay(1500);

    add_repeating_timer_ms(DRAW_TIME, drawTimerCallback, NULL, &drawTimer);
    add_repeating_timer_ms(JOYSTICK_READ_TIME, readJoystickTimerCallback, NULL, &joystickTimer);
    add_repeating_timer_ms(SUBTICK_TIME, tickTimerCallback, NULL, &tickTimer);
    watchdog_enable(SUBTICK_TIME * 40, false);

    LittleFS.begin();

    tft.fillScreen(BACKGROUND_COLOR);
    tft.drawLine(MAIN_SCREEN_WIDTH * CELL_SIZE + 1, 0, MAIN_SCREEN_WIDTH * CELL_SIZE + 1, tft.height(), FOREGROUND_COLOR);

    File gameFile = LittleFS.open("/games/snake.js", "r");
    duk_eval_string(ctx, gameFile.readString().c_str());
    gameFile.close();

}

void loop() {

}
