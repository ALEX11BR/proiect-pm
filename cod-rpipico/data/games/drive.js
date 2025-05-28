var lives = 4;

var yPosition = 0;
var xPosition = 5;
var walls = [];
var newWalls = [1, 1]
var newWallsLength = 5;


function collisionCheck() {
	for (var i = 16; i <= 19; i++) {
		if (walls[i][0] > xPosition - 1 || 10 - walls[i][1] <= xPosition + 1) {
			lives--;
			brickGameOver(xPosition, i, lives === 0);
			return;
		}
	}
}

function clearCar() {
	for (var i = 16; i <= 19; i++) {
		for (var j = xPosition - 1; j < xPosition + 2; j++) {
			brickMainDraw(j, i, 0);
		}
	}
}

function drawCar() {
	const carX = xPosition - 1;
	const carY = 16;

	brickMainDraw(carX, carY + 3, 1);
	brickMainDraw(carX + 2, carY + 3, 1);
	brickMainDraw(carX + 1, carY + 2, 1);
	brickMainDraw(carX + 0, carY + 1, 1);
	brickMainDraw(carX + 1, carY + 1, 1);
	brickMainDraw(carX + 2, carY + 1, 1);
	brickMainDraw(carX + 1, carY, 1);

	collisionCheck();
}

function moveForward() {
	walls.unshift(newWalls);
	for (var i = 19; i >= 0; i--) {
		// walls[i + 1] = old value of walls at line i

		for (var j = walls[i + 1][0]; j < walls[i][0]; j++) {
			brickMainDraw(j, i, 1);
		}
		for (var j = walls[i][0]; j < walls[i + 1][0]; j++) {
			brickMainDraw(j, i, 0);
		}

		for (var j = 9 - walls[i + 1][1]; j > 9 - walls[i][1]; j--) {
			brickMainDraw(j, i, 1);
		}
		for (var j = 9 - walls[i][1]; j > 9 - walls[i + 1][1]; j--) {
			brickMainDraw(j, i, 0);
		}
	}
	walls.pop();

	collisionCheck();

	newWallsLength--;
	if (newWallsLength <= 0) {
		newWallsLength = Math.floor(Math.random() * 5) + 5;

		var newNewWalls = [];
		do {
			newNewWalls = [Math.floor(Math.random() * 7), Math.floor(Math.random() * 7)];
		} while (Math.max(newWalls[0], newNewWalls[0]) + 3 > 10 - Math.max(newWalls[1], newNewWalls[1]));

		newWalls = newNewWalls;
	}
}


function handleInit() {
	yPosition = 0;
	xPosition = 5;

	walls = [];
	for (var i = 0; i < 20; i++) {
		walls.push([2, 2]);

		brickMainDraw(0, i, 1);
		brickMainDraw(1, i, 1);
		brickMainDraw(8, i, 1);
		brickMainDraw(9, i, 1);
	}
	newWalls = [1, 1];
	newWallsLength = 5;


	// Draw lives
	for (var i = 0; i < lives; i++) {
		brickSecondaryDraw(i, 0, 1);
	}
	drawCar();
}

function handleTick() {
	moveForward();
}

function handleAction() {
	// TODO
}

function handleUp() {
	brickTickReset();
	moveForward();
}

function handleLeft() {
	clearCar();
	xPosition--;
	drawCar();
}

function handleRight() {
	clearCar();
	xPosition++;
	drawCar();
}

function handleDown() {
	// TODO
}
