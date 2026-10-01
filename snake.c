#include "raylib.h"
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

#define SCREEN_WIDTH 960
#define SCREEN_HEIGHT 760
#define CELL_SIZE 28
#define GRID_COLUMNS 28
#define GRID_ROWS 20
#define MAX_SNAKE_LENGTH (GRID_COLUMNS * GRID_ROWS)
#define BOARD_WIDTH (GRID_COLUMNS * CELL_SIZE)
#define BOARD_HEIGHT (GRID_ROWS * CELL_SIZE)
#define BOARD_X ((SCREEN_WIDTH - BOARD_WIDTH) / 2)
#define BOARD_Y 142

typedef struct {
	int x;
	int y;
} Cell;

typedef enum {
	GAME_PLAYING,
	GAME_PAUSED,
	GAME_OVER,
	GAME_WON
} GameMode;

typedef struct {
	Cell segments[MAX_SNAKE_LENGTH];
	int length;
	Cell food;
	int score;
	int bestScore;
	int directionX;
	int directionY;
	int nextDirectionX;
	int nextDirectionY;
	float moveTimer;
	GameMode mode;
} Game;

static bool sameCell(Cell first, Cell second)
{
	return first.x == second.x && first.y == second.y;
}

static void placeFood(Game *game)
{
	bool occupied;

	do {
		game->food.x = GetRandomValue(0, GRID_COLUMNS - 1);
		game->food.y = GetRandomValue(0, GRID_ROWS - 1);
		occupied = false;

		for (int i = 0; i < game->length; i++) {
			if (sameCell(game->food, game->segments[i])) {
				occupied = true;
				break;
			}
		}
	} while (occupied);
}

static void resetGame(Game *game)
{
	int bestScore = game->bestScore;

	game->length = 4;
	game->segments[0] = (Cell){GRID_COLUMNS / 2, GRID_ROWS / 2};
	game->segments[1] = (Cell){GRID_COLUMNS / 2 - 1, GRID_ROWS / 2};
	game->segments[2] = (Cell){GRID_COLUMNS / 2 - 2, GRID_ROWS / 2};
	game->segments[3] = (Cell){GRID_COLUMNS / 2 - 3, GRID_ROWS / 2};
	game->score = 0;
	game->bestScore = bestScore;
	game->directionX = 1;
	game->directionY = 0;
	game->nextDirectionX = 1;
	game->nextDirectionY = 0;
	game->moveTimer = 0.0f;
	game->mode = GAME_PLAYING;
	placeFood(game);
}

static void setDirection(Game *game, int x, int y)
{
	if (x != -game->directionX || y != -game->directionY) {
		game->nextDirectionX = x;
		game->nextDirectionY = y;
	}
}

static void updateGame(Game *game, float deltaTime)
{
	if (game->mode != GAME_PLAYING) return;

	float speed = 0.135f - (float)game->score * 0.002f;
	if (speed < 0.065f) speed = 0.065f;
	game->moveTimer += deltaTime;

	while (game->moveTimer >= speed && game->mode == GAME_PLAYING) {
		game->moveTimer -= speed;
		game->directionX = game->nextDirectionX;
		game->directionY = game->nextDirectionY;

		Cell nextHead = {
			game->segments[0].x + game->directionX,
			game->segments[0].y + game->directionY
		};
		bool eating = sameCell(nextHead, game->food);

		if (nextHead.x < 0 || nextHead.x >= GRID_COLUMNS ||
			nextHead.y < 0 || nextHead.y >= GRID_ROWS) {
			game->mode = GAME_OVER;
			break;
		}

		int collisionLimit = eating ? game->length : game->length - 1;
		for (int i = 0; i < collisionLimit; i++) {
			if (sameCell(nextHead, game->segments[i])) {
				game->mode = GAME_OVER;
				break;
			}
		}
		if (game->mode == GAME_OVER) break;

		if (eating) {
			if (game->length == MAX_SNAKE_LENGTH) {
				game->mode = GAME_WON;
				break;
			}
			game->length++;
			game->score += 10;
			if (game->score > game->bestScore) game->bestScore = game->score;
		}

		for (int i = game->length - 1; i > 0; i--) {
			game->segments[i] = game->segments[i - 1];
		}
		game->segments[0] = nextHead;

		if (eating) placeFood(game);
	}
}

static void drawBoard(void)
{
	Color boardColor = {19, 35, 37, 255};
	Color gridColor = {31, 52, 51, 255};

	DrawRectangleRounded((Rectangle){BOARD_X - 5, BOARD_Y - 5,
		BOARD_WIDTH + 10, BOARD_HEIGHT + 10}, 0.035f, 8,
		(Color){47, 69, 62, 255});
	DrawRectangle(BOARD_X, BOARD_Y, BOARD_WIDTH, BOARD_HEIGHT, boardColor);

	for (int x = 1; x < GRID_COLUMNS; x++) {
		DrawLine(BOARD_X + x * CELL_SIZE, BOARD_Y,
			BOARD_X + x * CELL_SIZE, BOARD_Y + BOARD_HEIGHT, gridColor);
	}
	for (int y = 1; y < GRID_ROWS; y++) {
		DrawLine(BOARD_X, BOARD_Y + y * CELL_SIZE,
			BOARD_X + BOARD_WIDTH, BOARD_Y + y * CELL_SIZE, gridColor);
	}
}

static void drawGame(const Game *game)
{
	const Color cream = {241, 238, 216, 255};
	const Color muted = {145, 164, 151, 255};
	const Color lime = {182, 224, 115, 255};
	const Color deepGreen = {14, 27, 29, 255};

	ClearBackground(deepGreen);

	DrawText("SNAKE", BOARD_X, 37, 38, cream);
	DrawText("GROW SLOW. TURN SHARP.", BOARD_X, 83, 15, muted);

	DrawText("SCORE", 616, 39, 13, muted);
	DrawText(TextFormat("%03i", game->score), 616, 58, 28, cream);
	DrawText("BEST", 770, 39, 13, muted);
	DrawText(TextFormat("%03i", game->bestScore), 770, 58, 28, lime);
	DrawLine(BOARD_X, 116, BOARD_X + BOARD_WIDTH, 116,
		(Color){62, 81, 69, 255});

	drawBoard();

	Vector2 foodCenter = {
		BOARD_X + game->food.x * CELL_SIZE + CELL_SIZE / 2,
		BOARD_Y + game->food.y * CELL_SIZE + CELL_SIZE / 2
	};
	DrawCircleV(foodCenter, CELL_SIZE * 0.36f, (Color){247, 128, 99, 255});
	DrawCircleV((Vector2){foodCenter.x - 3, foodCenter.y - 4},
		CELL_SIZE * 0.12f, (Color){255, 204, 165, 255});

	for (int i = game->length - 1; i >= 0; i--) {
		Rectangle segment = {
			BOARD_X + game->segments[i].x * CELL_SIZE + 2,
			BOARD_Y + game->segments[i].y * CELL_SIZE + 2,
			CELL_SIZE - 4,
			CELL_SIZE - 4
		};
		Color segmentColor = i == 0 ? lime : (Color){116, 173, 104, 255};
		DrawRectangleRounded(segment, 0.28f, 5, segmentColor);
	}

	Cell head = game->segments[0];
	float headX = (float)(BOARD_X + head.x * CELL_SIZE);
	float headY = (float)(BOARD_Y + head.y * CELL_SIZE);
	Vector2 eyeOne;
	Vector2 eyeTwo;
	if (game->directionX != 0) {
		float eyeX = headX + (game->directionX > 0 ? 20.0f : 8.0f);
		eyeOne = (Vector2){eyeX, headY + 9.0f};
		eyeTwo = (Vector2){eyeX, headY + 19.0f};
	} else {
		float eyeY = headY + (game->directionY > 0 ? 20.0f : 8.0f);
		eyeOne = (Vector2){headX + 9.0f, eyeY};
		eyeTwo = (Vector2){headX + 19.0f, eyeY};
	}
	DrawCircleV(eyeOne, 2.0f, (Color){25, 40, 31, 255});
	DrawCircleV(eyeTwo, 2.0f, (Color){25, 40, 31, 255});

	DrawText("ARROWS / WASD  MOVE", BOARD_X, 721, 14, muted);
	DrawText("P  PAUSE     R  RESTART", 612, 721, 14, muted);

	if (game->mode != GAME_PLAYING) {
		Rectangle shade = {BOARD_X, BOARD_Y, BOARD_WIDTH, BOARD_HEIGHT};
		DrawRectangleRec(shade, (Color){8, 18, 19, 190});

		const char *title = "PAUSED";
		const char *subtitle = "PRESS P TO CONTINUE";
		Color titleColor = cream;
		if (game->mode == GAME_OVER) {
			title = "GAME OVER";
			subtitle = "PRESS R TO TRY AGAIN";
			titleColor = (Color){247, 151, 115, 255};
		} else if (game->mode == GAME_WON) {
			title = "BOARD CLEARED";
			subtitle = "PRESS R TO PLAY AGAIN";
			titleColor = lime;
		}

		int titleWidth = MeasureText(title, 38);
		int subtitleWidth = MeasureText(subtitle, 16);
		DrawText(title, SCREEN_WIDTH / 2 - titleWidth / 2,
			BOARD_Y + BOARD_HEIGHT / 2 - 30, 38, titleColor);
		DrawText(subtitle, SCREEN_WIDTH / 2 - subtitleWidth / 2,
			BOARD_Y + BOARD_HEIGHT / 2 + 22, 16, cream);
	}
}

int main(void)
{
	SetRandomSeed((unsigned int)time(NULL));
	InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Snake");
	SetTargetFPS(60);

	Game game = {0};
	resetGame(&game);

	while (!WindowShouldClose()) {
		if (IsKeyPressed(KEY_R)) {
			resetGame(&game);
		} else if (IsKeyPressed(KEY_P)) {
			if (game.mode == GAME_PLAYING) game.mode = GAME_PAUSED;
			else if (game.mode == GAME_PAUSED) game.mode = GAME_PLAYING;
		}

		if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
			setDirection(&game, 0, -1);
		} else if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) {
			setDirection(&game, 0, 1);
		} else if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)) {
			setDirection(&game, -1, 0);
		} else if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) {
			setDirection(&game, 1, 0);
		}

		updateGame(&game, GetFrameTime());

		BeginDrawing();
		drawGame(&game);
		EndDrawing();
	}

	CloseWindow();
	return 0;
}
