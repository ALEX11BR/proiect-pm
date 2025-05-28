const GAMES = ["/games/snake.js", "/games/drive.js"]
const GRAPHICS = [
	[
		// Snake
		[1, 1, 1, 1, 1, 1, 1, 1, 1, 1],
		[1, 0, 0, 0, 0, 0, 0, 0, 0, 0],
		[1, 1, 0, 1, 1, 1, 0, 0, 1, 1],
		[0, 1, 0, 1, 0, 1, 0, 1, 0, 1],
		[1, 1, 0, 1, 0, 1, 0, 1, 1, 1],
		[1, 0, 0, 0, 0, 0, 0, 0, 0, 0],
		[1, 0, 1, 0, 0, 0, 0, 1, 1, 0],
		[1, 0, 1, 0, 1, 0, 1, 1, 1, 0],
		[1, 0, 1, 1, 0, 0, 1, 0, 0, 0],
		[1, 0, 1, 0, 1, 0, 0, 1, 1, 0],
		[1, 0, 0, 0, 0, 0, 0, 0, 0, 0],
		[1, 1, 1, 1, 1, 1, 1, 1, 1, 1],
		[0, 0, 0, 0, 0, 0, 0, 0, 0, 1],
		[0, 2, 0, 0, 1, 1, 1, 1, 1, 1],
		[0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
		[0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
		[0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
		[0, 1, 0, 0, 0, 0, 0, 0, 1, 0],
		[1, 0, 0, 0, 0, 0, 0, 0, 0, 1],
		[0, 1, 0, 0, 0, 0, 0, 0, 1, 0],
	],
	[
		// Drive
		[0, 1, 1, 0, 0, 0, 0, 0, 0, 0],
		[0, 1, 0, 1, 0, 0, 1, 0, 1, 0],
		[0, 1, 0, 1, 0, 1, 0, 0, 0, 0],
		[0, 1, 0, 1, 0, 1, 0, 0, 1, 0],
		[0, 1, 1, 0, 0, 1, 0, 0, 1, 0],
		[0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
		[0, 0, 0, 0, 0, 0, 0, 1, 1, 0],
		[0, 0, 0, 0, 0, 0, 1, 1, 1, 0],
		[0, 0, 0, 0, 0, 0, 1, 0, 0, 0],
		[0, 0, 0, 0, 0, 0, 0, 1, 1, 0],
		[0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
		[0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
		[0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
		[0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
		[0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
		[0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
		[0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
		[0, 1, 0, 0, 0, 0, 0, 0, 1, 0],
		[1, 0, 0, 0, 0, 0, 0, 0, 0, 1],
		[0, 1, 0, 0, 0, 0, 0, 0, 1, 0],
	],
]
const ANIMATIONS = [
	//Snake
	function (ticks) {
		brickMainDraw(3, 13, + (ticks % 2 === 0));
		brickMainDraw(9, 0, 1 - (ticks % 2 === 0));
	},

	// Drive
	function (ticks) {
		for (var i = 6; i <= 11; i++) {
			for (var j = 1; j <= 4; j++) {
				brickMainDraw(j, i, 0);
			}
		}

		var carY = 6 + (ticks % 4 > 1) + (ticks % 2);
		var carX = 1 + Math.floor((ticks % 4) / 2);

		brickMainDraw(carX, carY, 1);
		brickMainDraw(carX + 2, carY, 1);
		brickMainDraw(carX + 1, carY + 1, 1);
		brickMainDraw(carX + 0, carY + 2, 1);
		brickMainDraw(carX + 1, carY + 2, 1);
		brickMainDraw(carX + 2, carY + 2, 1);
		brickMainDraw(carX + 1, carY + 3, 1);
	}
]

var selectedGame = 0;
var ticks = 0;

function drawSelectedGame() {
	for (var i = 0; i < 20; i++) {
		for (var j = 0; j < 10; j++) {
			brickMainDraw(j, i, GRAPHICS[selectedGame][i][j]);
		}
	}
}

function handleInit() {
	ticks = 0;
	drawSelectedGame();
}

function handleTick() {
	ticks++;
	ANIMATIONS[selectedGame](ticks);
}

function handleAction() {
	brickLoad(GAMES[selectedGame])
}

function handleUp() {
	selectedGame--;
	if (selectedGame < 0) {
		selectedGame = GAMES.length - 1;
	}
	drawSelectedGame();
}

function handleLeft() {
	selectedGame--;
	if (selectedGame < 0) {
		selectedGame = GAMES.length - 1;
	}
	drawSelectedGame();
}

function handleRight() {
	selectedGame++;
	if (selectedGame >= GAMES.length) {
		selectedGame = 0;
	}
	drawSelectedGame();
}

function handleDown() {
	selectedGame++;
	if (selectedGame >= GAMES.length) {
		selectedGame = 0;
	}
	drawSelectedGame();
}
