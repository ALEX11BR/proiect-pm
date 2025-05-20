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
