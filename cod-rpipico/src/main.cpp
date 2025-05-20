#include <Arduino.h>
#include <SPI.h>
#include "pico/stdlib.h"

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

#define DEBOUNCE_TIME 50
#define JOYSTICK_MIN_THRESH 100
#define JOYSTICK_MAX_THRESH 900

#define MAIN_SCREEN_WIDTH 10
#define MAIN_SCREEN_HEIGHT 20
#define CELL_SIZE 8

#define SCREEN_RENDER_0 0
#define SCREEN_RENDER_1 1
#define SCREEN_NO_RENDER 2


const char gameScript[] = R"(
// Snake game for 10x20 screen

// Game constants
const WIDTH = 10;
const HEIGHT = 20;
const DIRECTIONS = {
	UP: { x: 0, y: -1 },
	DOWN: { x: 0, y: 1 },
	LEFT: { x: -1, y: 0 },
	RIGHT: { x: 1, y: 0 }
};

// Game state
var snake = [{ x: 5, y: 5 }, { x: 4, y: 5 }]; // Snake starts in the middle
var direction = DIRECTIONS.RIGHT; // Next direction to move
var food = { x: 2, y: 5 }; // Initial food position
var score = 0;
var gameOver = false;

// initial setup
for (var i = 0; i < snake.length; i++) {
	brickMainDraw(snake[i].x, snake[i].y, 1);
}
brickMainDraw(food.x, food.y, 1);

function generateFood() {
	// Generate food at a random position
	var newFood;
	do {
		newFood = {
			x: Math.floor(Math.random() * WIDTH),
			y: Math.floor(Math.random() * HEIGHT)
		};
	} while (snake.some(function (segment) {return segment.x === newFood.x && segment.y === newFood.y;})); // Ensure food is not on the snake
	food = newFood;
	brickMainDraw(food.x, food.y, 1); // Draw the food
}

function moveSnake() {
	// Calculate new head position
	var newHead = {
		x: snake[0].x + direction.x,
		y: snake[0].y + direction.y
	};

	// Check for collisions with walls or self
	if (newHead.x < 0 || newHead.x >= WIDTH || newHead.y < 0 || newHead.y >= HEIGHT || snake.some(function (segment) {return segment.x === newHead.x && segment.y === newHead.y;})) {
		gameOver = true;
		return;
	}

	// Add new head to the snake
	snake.unshift(newHead);
	brickMainDraw(newHead.x, newHead.y, 1); // Draw the new head

	// Check if food is eaten
	if (newHead.x === food.x && newHead.y === food.y) {
		score++;
		generateFood();
	} else {
		brickMainDraw(snake[snake.length - 1].x, snake[snake.length - 1].y, 0); // Clear the tail
		snake.pop(); // Remove the tail
	}
}

// Event handlers
function handleTick() {
	moveSnake();
}

function handleAction() {

}

function handleUp() {
	if (direction !== DIRECTIONS.DOWN) {
		direction = DIRECTIONS.UP;
	}
	moveSnake();
}

function handleDown() {
	if (direction !== DIRECTIONS.UP) {
		direction = DIRECTIONS.DOWN;
	}
	moveSnake();
}

function handleLeft() {
	if (direction !== DIRECTIONS.RIGHT) {
		direction = DIRECTIONS.LEFT;
	}
	moveSnake();
}

function handleRight() {
	if (direction !== DIRECTIONS.LEFT) {
		direction = DIRECTIONS.RIGHT;
	}
	moveSnake();
}

)";


volatile bool screenUpdate = false;
volatile uint8_t mainScreenCells[MAIN_SCREEN_HEIGHT][MAIN_SCREEN_WIDTH];

TFT_eSPI tft = TFT_eSPI();

duk_context *ctx = duk_create_heap_default();

repeating_timer drawTimer;
repeating_timer joystickTimer;


duk_ret_t brickVibrate(duk_context *ctx) {
    // Parameters:
    // * intensity: 0-255
    int intensity = duk_get_int(ctx, 0);
    analogWrite(VIBRATION_PIN, intensity);
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

    noInterrupts();

    mainScreenCells[y][x] = color;
    screenUpdate = true;

    interrupts();
    return 0;
}

const duk_function_list_entry brickFunctionList[] = {
    {"brickVibrate", brickVibrate, 1},
    {"brickMainDraw", brickMainDraw, 3},
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

    // duktape reset context
    tft.fillScreen(TFT_BLACK);
    duk_destroy_heap(ctx);
    ctx = duk_create_heap_default();
    duk_push_global_object(ctx);
    duk_put_function_list(ctx, -1, brickFunctionList);
    duk_pop(ctx);

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


bool drawTimerCallback(repeating_timer *t) {
    noInterrupts();

    if (screenUpdate) {
        screenUpdate = false;

        // draw the main screen
        for (int y = 0; y < MAIN_SCREEN_HEIGHT; y++) {
            for (int x = 0; x < MAIN_SCREEN_WIDTH; x++) {
                if (mainScreenCells[y][x] != SCREEN_NO_RENDER) {
                    if (mainScreenCells[y][x] == SCREEN_RENDER_0) {
                        tft.fillRect(x * CELL_SIZE, y * CELL_SIZE, CELL_SIZE, CELL_SIZE, TFT_BLACK);
                    } else if (mainScreenCells[y][x] == SCREEN_RENDER_1) {
                        tft.fillRect(x * CELL_SIZE, y * CELL_SIZE, CELL_SIZE, CELL_SIZE, TFT_WHITE);
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
bool readJoystickCallback(repeating_timer *t) {
    noInterrupts();

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


void setup() {
    Serial.begin(9600);

    tft.init();

    // takes about 15ms
    tft.fillScreen(TFT_BLACK);
    tft.drawRect(60, 20, 40, 40, TFT_RED);

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

    add_repeating_timer_ms(DRAW_TIME, drawTimerCallback, NULL, &drawTimer);
    add_repeating_timer_ms(JOYSTICK_READ_TIME, readJoystickCallback, NULL, &joystickTimer);

    delay(1000);
    duk_eval_string(ctx, gameScript);
}

void loop() {
    delay(500);
    duk_eval_string(ctx, "handleTick();");
}
