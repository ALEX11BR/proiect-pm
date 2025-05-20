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
var snake = [{ x: 5, y: 5 }, { x: 4, y: 5 }];
var direction = DIRECTIONS.RIGHT;
var food = { x: 2, y: 5 }; // Initial food position
var score = 0;
var gameOver = false;
var ticks = 0;


// initial setup
for (var i = 0; i < snake.length; i++) {
	brickMainDraw(snake[i].x, snake[i].y, 1);
}
brickMainDraw(food.x, food.y, 2);


function isCollision(segment) {
	// Check if the coordinates are within the game area
	if (segment.x < 0 || segment.x >= WIDTH || segment.y < 0 || segment.y >= HEIGHT) {
		return true; // Collision with wall
	}

	// Check if the coordinates collide with the snake
	for (var i = 0; i < snake.length; i++) {
		if (snake[i].x === segment.x && snake[i].y === segment.y) {
			return true; // Collision with self
		}
	}

	return false; // No collision
}

function generateFood() {
	// Generate food at a random position
	brickVibrate(180, 1000);

	var newFood;
	do {
		newFood = {
			x: Math.floor(Math.random() * WIDTH),
			y: Math.floor(Math.random() * HEIGHT)
		};
	} while (isCollision(newFood)); // Ensure food is not on the snake
	food = newFood;
	brickMainDraw(food.x, food.y, 2); // Draw the food
}

function moveSnake() {
	// Calculate new head position
	var newHead = {
		x: snake[0].x + direction.x,
		y: snake[0].y + direction.y
	};

	// Check for collisions with walls or self
	if (isCollision(newHead)) {
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
	brickMainDraw(0, 0, 0 + (ticks % 2 === 0));
	ticks++;

	moveSnake();
}

function handleAction() {
	brickTickReset();
	moveSnake();
}

function handleUp() {
	if (direction !== DIRECTIONS.DOWN) {
		direction = DIRECTIONS.UP;
		brickTickReset();
		moveSnake();
	}
}

function handleDown() {
	if (direction !== DIRECTIONS.UP) {
		direction = DIRECTIONS.DOWN;
		brickTickReset();
		moveSnake();
	}
}

function handleLeft() {
	if (direction !== DIRECTIONS.RIGHT) {
		direction = DIRECTIONS.LEFT;
		brickTickReset();
		moveSnake();
	}
}

function handleRight() {
	if (direction !== DIRECTIONS.LEFT) {
		direction = DIRECTIONS.RIGHT;
		brickTickReset();
		moveSnake();
	}
}
