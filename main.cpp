#include<SDL.h>
#include<SDL_image.h>
#include<SDL_ttf.h>
#include<SDL_mixer.h>
#include<iostream>
#include<fstream>
#include<vector>
#include<algorithm>
#include<string>
#include"collision.h"

//#####################Structs && Maze init#####################//

enum class Direction { RIGHT, UP, LEFT, DOWN };
std::vector<Direction> directions = { Direction::UP, Direction::RIGHT, Direction::DOWN, Direction::LEFT };
Direction pacmanDirection = Direction::RIGHT;
Direction lastInput = pacmanDirection;
static int COOLDOWN_TIME = 60;

struct entity{
	int cooldown;
	SDL_Rect rec;
	Direction direction;
	SDL_Texture* texture;
	SDL_Texture* textureBlue;
	SDL_Texture* textureWhite;
}pacman,blinky,pinky,inky,clyde;

struct level {
	std::vector<SDL_Rect> loadedMaze;
	std::vector<SDL_Rect> loadedDots;
	std::vector<SDL_Rect> loadedBigDots;
};

std::vector<level> levels(4);

extern int maze0[30][60];
extern int maze1[30][60];
extern int maze2[30][60];
extern int maze3[30][60];
static int levelIndex = 0;
extern int spawnPosition[5][10];

//#####################Declaration#####################//

const int SCREEN_WIDTH = 1920;
const int SCREEN_HEIGHT = 960;

void quit();
void timer();
void handleMouse();
void handleKeyPress();

void drawTutorial();
void drawStartMenu();
void drawPauseMenu();
void drawHighScore();
void drawSoundMenu();
std::string getUsername();
void drawOptionsMenu();
void drawGameOverMenu();
void drawVictoryScreen();

int pacghost();
void isDying();
void entities();
void updateGhosts();
void renderEntities();
void invincibleTimer();
void updatePacmanPosition();
void updateGhost(entity &ghost);
bool isValid(int newX, int newY);
bool isIntersection(entity& ghost);
int countPathsInDirection(int x, int y, Direction dir);
Direction getRandomDirection();
Direction getBestDirection(int x, int y, Direction prevDirection);

void dotEater();
void resetGame();
void loadNextLevel();
void drawMaze(int Index);
void initMaze(int maze[30][60], int index);
void text(const char* fontname, int fontsize, const char* text, int x, int y);
void initPosition(int Index, int pacspawnX, int pacspawnY, int bspawnX, int bspawnY,
	int pspawnX, int pspawnY, int ispawnX, int ispawnY, int cspawnX, int cspawnY);

void liveScore();
void loadHighScore();
void saveHighScore();
void updateHighScore(const std::string& username, int newScore);

void musicPlayer(Mix_Chunk* sound, const char* text, int channel);

SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;
static SDL_Event e;

int PACMAN_SPEED = 1;
int newX, newY, dir;
static int SPRITE_SIZE = 32;
static int textureHeight, textureWidth, frameWidth, frameHeight;
SDL_Rect pacmanDeathRec, srcRect;
static int spawnCount = 0;
SDL_Texture* pacmanDeathTexture;
int lives = 6;

int warningTime = 120;
bool isInvincible = false;
int invincibilityTimer = 0;
const int INVINCIBILITY_DURATION = 600;
int menuCount = 0;
bool victory = false;

enum class GameState { START_MENU, OPTIONS_MENU, GAMEPLAY, PAUSE_MENU, NEXT_LEVEL, GAME_OVER, VICTORY , HIGHSCORE, SOUND_MENU, TUTORIAL};
int currentTime, elapsedTime, frameCounter = 0, frameDuration = 60, lastTime;
static bool running = true, isMoving = true, isFullscreen = false, hidecursor = false, collisionDetected = false;
GameState gameState = GameState::START_MENU;
SDL_Rect pauseRect = { 1700,50,32,32 };
SDL_Rect backRect = { 1495,905,32,32 };
SDL_Texture *pauseTexture, *backTexture;

int eatCount = 0;
int currentScore = 0;
std::string currentScoreText, livesText;
std::string usernameInput = "";
std::pair<std::string, int> highScore;

Mix_Chunk* eatDots = nullptr; // 2 3
Mix_Chunk* ghostDeath = nullptr; // 4
Mix_Chunk* pacDeath = nullptr; //5
Mix_Chunk* levelSound = nullptr; // 1
Mix_Chunk* afraidGhost = nullptr; //6
int musicVolume = 64;

///////////////////////////////////////////////////////////////////////////////

int main(int argc, char* args[]) {
	SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, 0, &window, &renderer);
	TTF_Init();
	Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
	initMaze(maze0, 0);
	initMaze(maze1, 1);
	initMaze(maze2, 2);
	initMaze(maze3, 3);
	entities();
	loadHighScore();

	while (running) {
		timer();
		while (SDL_PollEvent(&e)) {
			handleKeyPress();
			handleMouse();
		}
		switch (gameState) {
			case GameState::START_MENU:
				drawStartMenu();
				break;
			case GameState::OPTIONS_MENU:
				drawOptionsMenu();
				break;
			case GameState::PAUSE_MENU:
				drawPauseMenu();
				updateHighScore(usernameInput, currentScore);
				break;
			case GameState::GAMEPLAY:
				SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
				SDL_RenderClear(renderer);
				drawMaze(levelIndex);
				updatePacmanPosition();
				updateGhosts();
				renderEntities();
				dotEater();
				if (lives == 0) gameState = GameState::GAME_OVER;
				liveScore();
				break;
			case GameState::NEXT_LEVEL:
				loadNextLevel();
				break;
			case GameState::GAME_OVER:
				drawGameOverMenu();
				updateHighScore(usernameInput, currentScore);
				break;
			case GameState::VICTORY:
				drawVictoryScreen();
				updateHighScore(usernameInput, currentScore);
				break;
			case GameState::HIGHSCORE:
				drawHighScore();
				break;
			case GameState::SOUND_MENU:
				drawSoundMenu();
				break;
			case GameState::TUTORIAL:
				drawTutorial();
				break;
		}
		SDL_RenderPresent(renderer);
		//SDL_Delay(4);
	}
	quit();
	return 0;
}

//#####################Functions#####################//
void quit()
{
	SDL_DestroyTexture(pacman.texture);
	SDL_DestroyTexture(blinky.texture);
	SDL_DestroyTexture(pinky.texture);
	SDL_DestroyTexture(inky.texture);
	SDL_DestroyTexture(clyde.texture);
	SDL_DestroyTexture(blinky.textureBlue);
	SDL_DestroyTexture(blinky.textureWhite);
	SDL_DestroyTexture(pauseTexture);
	SDL_DestroyTexture(pacmanDeathTexture);
	pacman.texture = nullptr;
	blinky.texture = nullptr;
	pinky.texture = nullptr;
	inky.texture = nullptr;
	clyde.texture = nullptr;
	blinky.textureBlue = nullptr;
	blinky.textureWhite = nullptr;
	pauseTexture = nullptr;
	pacmanDeathTexture = nullptr;
	Mix_FreeChunk(levelSound);
	levelSound = nullptr;
	Mix_FreeChunk(eatDots);
	eatDots = nullptr;
	Mix_FreeChunk(ghostDeath);
	ghostDeath = nullptr;
	Mix_FreeChunk(afraidGhost);
	afraidGhost = nullptr;
	Mix_FreeChunk(pacDeath);
	pacDeath = nullptr;
	SDL_DestroyRenderer(renderer);
	renderer = nullptr;
	SDL_DestroyWindow(window);
	window = nullptr;
	TTF_Quit();
	IMG_Quit();
	SDL_Quit();
}

void text(const char* fontname, int fontsize, const char* text, int x, int y)
{
	TTF_Font* font = TTF_OpenFont(fontname, fontsize);
	SDL_Color color = { 255, 255, 255 };
	SDL_Surface* surface = TTF_RenderText_Solid(font, text, color);
	if (!surface) {
		std::cerr << "Failed to create surface: " << TTF_GetError() << std::endl;
		TTF_CloseFont(font);
		return;
	}
	SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
	SDL_Rect rect = { x, y, surface->w, surface->h };
	SDL_RenderCopy(renderer, texture, NULL, &rect);
	SDL_DestroyTexture(texture);
	SDL_FreeSurface(surface);
	texture = nullptr;
	TTF_CloseFont(font);
}

void musicPlayer(Mix_Chunk* sound, const char* text, int channel) {
	if (Mix_Playing(channel)) return;
	sound = Mix_LoadWAV(text);
	Mix_PlayChannel(channel, sound, 0);
}

//////////////////////////////////////////////////////////////////////

SDL_Rect wallRect;
void initMaze(int maze[30][60], int Index) {
	for (int y = 0; y < 30; ++y) {
		for (int x = 0; x < 60; ++x) {
			if (maze[y][x] == 3) {
				wallRect = { x * 32, y * 32, 32, 32 };
				levels[Index].loadedMaze.push_back(wallRect);
			}
			if (maze[y][x] == 2) {
				wallRect = { x * 32, y * 32, 32, 32 };
				levels[Index].loadedDots.push_back(wallRect);
			}			
			if (maze[y][x] == 4) {
				wallRect = { x * 32, y * 32, 32, 32 };
				levels[Index].loadedBigDots.push_back(wallRect);
			}
		}
	}
}

void drawMaze(int Index){

	SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
	for (const auto& rect : levels[Index].loadedMaze) {
		SDL_Rect modRect = rect;
		modRect.w = 24;
		modRect.h = 24;
		SDL_RenderFillRect(renderer, &modRect);
	}
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
	for (const auto& rect : levels[Index].loadedDots) {
		SDL_Rect modRect = rect;
		modRect.x += 12;
		modRect.y += 12;
		modRect.w = 6;
		modRect.h = 6;
		SDL_RenderFillRect(renderer, &modRect);
	}
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
	for (const auto& rect : levels[Index].loadedBigDots) {
		SDL_Rect modRect = rect;
		modRect.x += 8;
		modRect.y += 8;
		modRect.w = 12;
		modRect.h = 12;
		SDL_RenderFillRect(renderer, &modRect);
	}
}

void initPosition(int Index, int pacspawnX, int pacspawnY, int bspawnX, int bspawnY, int pspawnX,
	int pspawnY, int ispawnX, int ispawnY, int cspawnX, int cspawnY) {
	if (Index == spawnCount)
	{
		pacman.rec.x = pacspawnX;
		pacman.rec.y = pacspawnY;
		blinky.rec.x = bspawnX;
		blinky.rec.y = bspawnY;
		pinky.rec.x = pspawnX;
		pinky.rec.y = pspawnY;
		inky.rec.x = ispawnX;
		inky.rec.y = ispawnY;
		clyde.rec.x = cspawnX;
		clyde.rec.y = cspawnY;
		spawnCount++;
	}
}

void dotEater() {
	{
		auto it = std::find_if(levels[levelIndex].loadedDots.begin(), levels[levelIndex].loadedDots.end(), [&](const SDL_Rect& rect) {
			int dotX = rect.x / 32;
			int dotY = rect.y / 32;
			int pacmanGridX = (pacman.rec.x + 16) / 32;
			int pacmanGridY = (pacman.rec.y + 16) / 32;
			return (dotX == pacmanGridX && dotY == pacmanGridY);
			});
		if (it != levels[levelIndex].loadedDots.end()) {
			levels[levelIndex].loadedDots.erase(it);
			currentScore += 5;
			eatCount++;
			if(eatCount%2)
				musicPlayer(levelSound, "audio/munch_a.wav", 2);
			else
				musicPlayer(levelSound, "audio/munch_b.wav", 3);
		}
	} 
	{
		auto it = std::find_if(levels[levelIndex].loadedBigDots.begin(), levels[levelIndex].loadedBigDots.end(), [&](const SDL_Rect& rect) {
			int dotX = rect.x / 32;
			int dotY = rect.y / 32;
			int pacmanGridX = (pacman.rec.x + 16) / 32;
			int pacmanGridY = (pacman.rec.y + 16) / 32;
			return (dotX == pacmanGridX && dotY == pacmanGridY);
			});
		if (it != levels[levelIndex].loadedBigDots.end()) {
			levels[levelIndex].loadedBigDots.erase(it);
			isInvincible = true;
			invincibilityTimer = INVINCIBILITY_DURATION;
			currentScore += 10;
			eatCount++;
			if (eatCount % 2)
				musicPlayer(levelSound, "audio/munch_a.wav", 2);
			else
				musicPlayer(levelSound, "audio/munch_b.wav", 3);
		}
	}
	if (levels[levelIndex].loadedDots.size() == 0 and levels[levelIndex].loadedBigDots.size() == 0) {
		levelIndex++;
		loadNextLevel();
	}
}

void loadNextLevel() {
	isInvincible = false;
	if (levelIndex < levels.size()) {
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		SDL_RenderClear(renderer);
		spawnCount = levelIndex;
		drawMaze(levelIndex);
		initPosition(levelIndex, spawnPosition[levelIndex][0], spawnPosition[levelIndex][1], spawnPosition[levelIndex][2],
			spawnPosition[levelIndex][3], spawnPosition[levelIndex][4], spawnPosition[levelIndex][5], spawnPosition[levelIndex][6], 
			spawnPosition[levelIndex][7], spawnPosition[levelIndex][8], spawnPosition[levelIndex][9]);
		renderEntities();
		SDL_RenderPresent(renderer);
		gameState = GameState::GAMEPLAY;
		musicPlayer(eatDots, "audio/pacman_beginning.wav", 1);
		updateHighScore(usernameInput, currentScore);
		SDL_Delay(3000);
	}
	else {
		gameState = GameState::VICTORY;
	}
}

void resetGame() {
	gameState = GameState::GAMEPLAY;
	victory = false;
	initMaze(maze0, 0);
	initMaze(maze1, 1);
	pacmanDirection = Direction::RIGHT;
	isInvincible = false;
	currentScore = 0;
	levelIndex = 0;
	usernameInput = "";
	pacman.rec.x = spawnPosition[levelIndex][0];
	pacman.rec.y = spawnPosition[levelIndex][1];
	blinky.rec.x = spawnPosition[levelIndex][2];
	blinky.rec.y = spawnPosition[levelIndex][3];
	pinky.rec.x = spawnPosition[levelIndex][4];
	pinky.rec.y = spawnPosition[levelIndex][5];
	inky.rec.x = spawnPosition[levelIndex][6];
	inky.rec.y = spawnPosition[levelIndex][7];
	clyde.rec.x = spawnPosition[levelIndex][8];
	clyde.rec.y = spawnPosition[levelIndex][9];
}

/////////////////////////////////////////////////Enitites/////////////////////////////////////////////////

void invincibleTimer() {
	if (isInvincible) {
		invincibilityTimer--;
		if (invincibilityTimer <= 0) {
			isInvincible = false;
		}
	}
}

void timer() {
	currentTime = SDL_GetTicks();
	elapsedTime = currentTime - lastTime;
	if (elapsedTime >= frameDuration) {
		lastTime = currentTime;
		if (isMoving) {
			frameCounter++;
			if (frameCounter >= 6) {
				frameCounter = 0;
			}
		} 
		else {
			frameCounter = 2;
		}
	}
	invincibleTimer();
}

void entities() 
{
	auto multiplier = 1.5/2.0;

	pacman.texture = IMG_LoadTexture(renderer, "sprite/Pacman16.png");
	SDL_QueryTexture(pacman.texture, NULL, NULL, &textureWidth, &textureHeight);
	frameWidth = textureWidth / 6;
	frameHeight = textureHeight / 4;
	pacman.rec.w = frameWidth * 1.5;
	pacman.rec.h = frameHeight * 1.5;

	blinky.textureBlue = IMG_LoadTexture(renderer, "sprite/afraid_0.png");
	blinky.textureWhite = IMG_LoadTexture(renderer, "sprite/afraid_1.png");

	blinky.texture = IMG_LoadTexture(renderer, "sprite/blinky.png");
	SDL_QueryTexture(blinky.texture, NULL, NULL, &textureWidth, &textureHeight);
	blinky.rec.w = textureWidth * multiplier;
	blinky.rec.h = textureHeight * multiplier;

	pinky.texture = IMG_LoadTexture(renderer, "sprite/pinky.png");
	SDL_QueryTexture(pinky.texture, NULL, NULL, &textureWidth, &textureHeight);
	pinky.rec.w = textureWidth * multiplier;
	pinky.rec.h = textureHeight * multiplier;

	inky.texture = IMG_LoadTexture(renderer, "sprite/inky.png");
	SDL_QueryTexture(inky.texture, NULL, NULL, &textureWidth, &textureHeight);
	inky.rec.w = textureWidth * multiplier;
	inky.rec.h = textureHeight * multiplier;

	clyde.texture = IMG_LoadTexture(renderer, "sprite/clyde.png");
	SDL_QueryTexture(clyde.texture, NULL, NULL, &textureWidth, &textureHeight);
	clyde.rec.w = textureWidth * multiplier;
	clyde.rec.h = textureHeight * multiplier;
}

void renderEntities() {
	srcRect = { frameCounter * frameWidth, dir * frameHeight, frameWidth, frameHeight };
	SDL_RenderCopy(renderer, pacman.texture, &srcRect, &pacman.rec);

	if (!isInvincible) {
		SDL_RenderCopy(renderer, blinky.texture, NULL, &blinky.rec);
		SDL_RenderCopy(renderer, pinky.texture, NULL, &pinky.rec);
		SDL_RenderCopy(renderer, inky.texture, NULL, &inky.rec);
		SDL_RenderCopy(renderer, clyde.texture, NULL, &clyde.rec);
	}
	else {
		bool inWarningPeriod = invincibilityTimer <= warningTime;
		bool renderWhite = inWarningPeriod && ((currentTime / 250) % 2 == 0);
		if (renderWhite) {
			SDL_RenderCopy(renderer, blinky.textureWhite, NULL, &blinky.rec);
			SDL_RenderCopy(renderer, blinky.textureWhite, NULL, &pinky.rec);
			SDL_RenderCopy(renderer, blinky.textureWhite, NULL, &inky.rec);
			SDL_RenderCopy(renderer, blinky.textureWhite, NULL, &clyde.rec);
		}
		else {
			SDL_RenderCopy(renderer, blinky.textureBlue, NULL, &blinky.rec);
			SDL_RenderCopy(renderer, blinky.textureBlue, NULL, &pinky.rec);
			SDL_RenderCopy(renderer, blinky.textureBlue, NULL, &inky.rec);
			SDL_RenderCopy(renderer, blinky.textureBlue, NULL, &clyde.rec);
		}
	}
}

bool isValid(int x, int y) {
	if (x < 0 || x >= 60 * SPRITE_SIZE || y < 0 || y >= 30 * SPRITE_SIZE)
		return false;
	SDL_Rect newRect = { x, y, SPRITE_SIZE, SPRITE_SIZE };
	for (const auto& wall : levels[levelIndex].loadedMaze) {
		if (Collision::checkCollision(newRect, wall)) {
			return false;
		}
	}
	return true;
}

void isDying() {
	pacmanDeathRec = pacman.rec;
	pacmanDeathTexture = IMG_LoadTexture(renderer, "sprite/PacmanDeath16.png");
	musicPlayer(pacDeath, "audio/pacman_death.wav", 5);
	for (int i = 0; i < 12; i++) {
		srcRect = { i * 16, 0, 16, 16 };
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, pacmanDeathTexture, &srcRect, &pacman.rec);
		SDL_RenderPresent(renderer);
		SDL_Delay(100);
	}
}

void updatePacmanPosition() {

	if (pacghost() == 10) {
		lives--;
		isDying();
		initPosition(levelIndex, spawnPosition[levelIndex][0], spawnPosition[levelIndex][1], spawnPosition[levelIndex][2],
		spawnPosition[levelIndex][3], spawnPosition[levelIndex][4], spawnPosition[levelIndex][5], spawnPosition[levelIndex][6],
		spawnPosition[levelIndex][7], spawnPosition[levelIndex][8], spawnPosition[levelIndex][9]);
		spawnCount--;
		return;
	}
	newX = pacman.rec.x;
	newY = pacman.rec.y;

	switch (pacmanDirection) {
		case Direction::UP:
			newY -= PACMAN_SPEED;
			break;
		case Direction::DOWN:
			newY += PACMAN_SPEED;
			break;
		case Direction::LEFT:
			newX -= PACMAN_SPEED;
			break;
		case Direction::RIGHT:
			newX += PACMAN_SPEED;
			break;
		default:
			break;
	}

	if (isValid(newX, newY)) {
		pacman.rec.x = newX;
		pacman.rec.y = newY;
	}
}

int pacghost() {
	if( isInvincible == false && (SDL_HasIntersection(&pacman.rec, &blinky.rec) or SDL_HasIntersection(&pacman.rec, &pinky.rec)
		or SDL_HasIntersection(&pacman.rec, &inky.rec) or SDL_HasIntersection(&pacman.rec, &clyde.rec)))
		return 10;
	else if (isInvincible == true && (SDL_HasIntersection(&pacman.rec, &blinky.rec)))
		return 1;
	else if (isInvincible == true && (SDL_HasIntersection(&pacman.rec, &pinky.rec)))
		return 2;
	else if (isInvincible == true && (SDL_HasIntersection(&pacman.rec, &inky.rec)))
		return 3;
	else if (isInvincible == true && (SDL_HasIntersection(&pacman.rec, &clyde.rec)))
		return 4;
	return 0;
}

////////////////////////////////////////////////////////////////////////////////

bool isIntersection(entity& ghost) {
	int x = ghost.rec.x / 32;
	int y = ghost.rec.y / 32;
	int possibleDirections = 0;

	if (x < 59 && maze0[y][x + 1] != 3) possibleDirections++; // Right
	if (x > 0 && maze0[y][x - 1] != 3) possibleDirections++; // Left
	if (y > 0 && maze0[y - 1][x] != 3) possibleDirections++; // Up
	if (y < 29 && maze0[y + 1][x] != 3) possibleDirections++; // Down

	return possibleDirections > 2;
}

int countPathsInDirection(int x, int y, Direction dir) {
	int count = 0;
	switch (dir) {
	case Direction::RIGHT:
		while (isValid(x + count + 1, y)) count++;
		break;
	case Direction::LEFT:
		while (isValid(x - count - 1, y)) count++;
		break;
	case Direction::UP:
		while (isValid(x, y - count - 1)) count++;
		break;
	case Direction::DOWN:
		while (isValid(x, y + count + 1)) count++;
		break;
	}
	return count;
}

Direction getBestDirection(int x, int y, Direction prevDirection) {
	int maxPaths = -1;
	Direction bestDirection = prevDirection;

	for (Direction dir : directions) {
		if (dir == static_cast<Direction>((static_cast<int>(prevDirection) + 2) % 4)) {
			continue;
		}
		int paths = countPathsInDirection(x, y, dir);
		if (paths > maxPaths) {
			maxPaths = paths;
			bestDirection = dir;
		}
	}
	return bestDirection;
}

Direction getRandomDirection() {
	return static_cast<Direction>(rand() % 4);
}

void updateGhost(entity& ghost) {

	if (pacghost() == 1) {
		blinky.rec.x = spawnPosition[levelIndex][2];
		blinky.rec.y = spawnPosition[levelIndex][3];
		musicPlayer(ghostDeath, "audio/pacman_eatghost.wav", 4);
		currentScore += 50;
		return;
	}
	if (pacghost() == 2) {
		pinky.rec.x = spawnPosition[levelIndex][4];
		pinky.rec.y = spawnPosition[levelIndex][5];
		musicPlayer(ghostDeath, "audio/pacman_eatghost.wav", 4);
		currentScore += 50;
		return;
	}	
	if (pacghost() == 3) {
		inky.rec.x = spawnPosition[levelIndex][6];
		inky.rec.y = spawnPosition[levelIndex][7];
		musicPlayer(ghostDeath, "audio/pacman_eatghost.wav", 4);
		currentScore += 50;
		return;
	}
	if (pacghost() == 4) {
		clyde.rec.x = spawnPosition[levelIndex][8];
		clyde.rec.y = spawnPosition[levelIndex][9];
		musicPlayer(ghostDeath, "audio/pacman_eatghost.wav", 4);
		currentScore += 50;
		return;
	}
	int x = ghost.rec.x / 32;
	int y = ghost.rec.y / 32;
	newX = ghost.rec.x;
	newY = ghost.rec.y;

	if (ghost.cooldown <= 0 && isIntersection(ghost)) {
		ghost.direction = ((rand() % 2) ? getBestDirection(x, y, ghost.direction) : getRandomDirection());
		ghost.cooldown = COOLDOWN_TIME;
	}
	else {
		ghost.cooldown--;
	}

	switch (ghost.direction) {
	case Direction::RIGHT:
		if (x < 59 && isValid(newX+1, newY)) ghost.rec.x += PACMAN_SPEED;
		else ghost.direction = getRandomDirection();
		break;
	case Direction::LEFT:
		if (x > 0 && isValid(newX-1, newY)) ghost.rec.x -= PACMAN_SPEED;
		else ghost.direction = getRandomDirection();
		break;
	case Direction::UP:
		if (y > 0 && isValid(newX, newY-1)) ghost.rec.y -= PACMAN_SPEED;
		else ghost.direction = getRandomDirection();
		break;
	case Direction::DOWN:
		if (y < 29 && isValid(newX, newY+1)) ghost.rec.y += PACMAN_SPEED;
		else ghost.direction = getRandomDirection();
		break;
	}
}

void updateGhosts() {
	updateGhost(blinky);
	updateGhost(pinky);
	updateGhost(inky);
	updateGhost(clyde);
}

////////////////////////////////////////////////////////////////////////////////
void handleKeyPress() {
	if (e.type == SDL_QUIT) running = false;
	if (e.type == SDL_KEYDOWN) {
		switch (e.key.keysym.sym) {
			case SDLK_UP:
			case SDLK_w:
				lastInput = Direction::UP;
				dir = static_cast<int>(Direction::UP);
				break;
			case SDLK_DOWN:
			case SDLK_s:
				lastInput = Direction::DOWN;
				dir = static_cast<int>(Direction::DOWN);
				break;
			case SDLK_LEFT:
			case SDLK_a:
				lastInput = Direction::LEFT;
				dir = static_cast<int>(Direction::LEFT);
				break;
			case SDLK_RIGHT:
			case SDLK_d:
				lastInput = Direction::RIGHT;
				dir = static_cast<int>(Direction::RIGHT);
				break;
			case SDLK_ESCAPE:
				if(gameState != GameState::START_MENU and gameState != GameState::OPTIONS_MENU and
					gameState != GameState::VICTORY and gameState != GameState::SOUND_MENU and
					gameState != GameState::GAME_OVER and gameState != GameState::HIGHSCORE and
					gameState != GameState::SOUND_MENU)
					gameState = ((gameState == GameState::PAUSE_MENU) ? GameState::GAMEPLAY : GameState::PAUSE_MENU);
				break;
			case SDLK_F1:
				hidecursor = !hidecursor;
				SDL_ShowCursor(!hidecursor);
				break;
			case SDLK_f:
				if (e.key.keysym.mod & KMOD_CTRL) {
					isFullscreen = !isFullscreen;
					SDL_SetWindowFullscreen(window, isFullscreen ? SDL_WINDOW_FULLSCREEN : 0);
				}
				break;
			case SDLK_l:
				if(levelIndex != 4) levelIndex++;
				break;
			case SDLK_k:
				if(levelIndex != 0) levelIndex--;
				break;
			case SDLK_o:
				gameState = GameState::OPTIONS_MENU;
				break;
			case SDLK_i:
				isInvincible != isInvincible;
				break;
			case SDLK_h:
				gameState = GameState::GAME_OVER;
				break;			
			case SDLK_j:
				gameState = GameState::VICTORY;
				break;
		}
		pacmanDirection = lastInput;
	}
}

void handleMouse() {
    int mouseX, mouseY;
    switch (e.type) {
        case SDL_MOUSEBUTTONDOWN:
            SDL_GetMouseState(&mouseX, &mouseY);

            if (mouseX >= 900 && mouseX <= 1100 && gameState == GameState::START_MENU) {
                if (mouseY >= 400 && mouseY <= 440) {
					resetGame();
					usernameInput = getUsername();
					menuCount++;
                    gameState = GameState::NEXT_LEVEL;
                }                
				else if (mouseY >= 350 && mouseY <= 390) {
					usernameInput = getUsername();
					if (menuCount == 0) {
						SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "", "No Previous Incomplete Game Found", nullptr);
						menuCount++;
					}
					if (victory == true) {
						resetGame();
						SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "", "You Already Achieved Victory in Previous Game", nullptr);

					}
					gameState = GameState::NEXT_LEVEL;
				}
				else if (mouseY >= 450 && mouseY <= 490) {
					gameState = GameState::HIGHSCORE;
				}
                else if (mouseY >= 500 && mouseY <= 540) {
                    gameState = GameState::OPTIONS_MENU;
                }
                else if (mouseY >= 550 && mouseY <= 590) {
                    const SDL_MessageBoxButtonData buttons[] = {
                        { SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, 2, "NO" },
                        { SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, 1, "Yes" },
                    };
                    const SDL_MessageBoxColorScheme colorScheme = {
                        {
                            { 255, 255, 255 }, // Background color
                            { 0, 0, 0 },       // Text color
                            { 0, 0, 0 },       // Button border color
                            { 255, 0, 0 },     // Button background color
                            { 0, 255, 0 }      // Button selected color
                        }
                    };
                    const SDL_MessageBoxData messageboxdata = {
                        SDL_MESSAGEBOX_INFORMATION,
                        NULL,
                        "Exit Game",
                        "Are you sure you want to exit?",
                        SDL_arraysize(buttons),
                        buttons,
                        &colorScheme
                    };
                    int buttonid;
                    if (SDL_ShowMessageBox(&messageboxdata, &buttonid) < 0) {
                        SDL_Log("error displaying message box");
                    }
                    if (buttonid == 1) {
                        SDL_Log("Exiting...");
						running = false;
                    } else if (buttonid == 0 || buttonid == 2) {
                        SDL_Log("Cancelled exit");
                    } else {
                        SDL_Log("no selection");
                    }
                }
            }
			else if (mouseX >= 900 && mouseX <= 1100 && gameState == GameState::OPTIONS_MENU) {
				if (mouseY >= 450 && mouseY <= 490) {
					gameState = GameState::TUTORIAL;
				}
				else if (mouseY >= 500 && mouseY <= 540) {
					gameState = GameState::SOUND_MENU;
				}
			}
			else if (gameState == GameState::PAUSE_MENU) {
				if (mouseX >= 1495 && mouseX <= 1800 && mouseY >= 900 && mouseY <= 950) {
					gameState = GameState::START_MENU;
					SDL_RenderPresent(renderer);
					return;
				}
			}
			else if (gameState == GameState::VICTORY or gameState == GameState::HIGHSCORE or gameState == GameState::TUTORIAL
				or gameState == GameState::GAME_OVER or gameState == GameState::OPTIONS_MENU) {
				if (mouseX >= 1495 && mouseX <= 1800 && mouseY >= 900 && mouseY <= 950) {
					gameState = GameState::START_MENU;
					SDL_RenderPresent(renderer);
					return;
				}
			}
			break;
    }
}

////////////////////////////////////////////////////////////////////////////////

void liveScore() {
	int live;
	if (lives == 6) live = 5;
	else live = lives;
	livesText = "Lives " + std::to_string(live) + "  "+ "Score " + std::to_string(currentScore);
	text("font/HeavyDataNerdFont-Regular.ttf", 28, livesText.c_str(), 50, 30);
}

void loadHighScore() {
	std::ifstream file("highscore.txt");
	std::string username;
	int score;
	if(file >> username >> score) {
		highScore = std::make_pair(username, score);
	}
	file.close();
}

void saveHighScore() {
	std::ofstream file("highscore.txt");
	file << highScore.first << " " << highScore.second << std::endl;
	file.close();
}

void updateHighScore(const std::string& username, int newScore) {
	if (newScore > highScore.second) {
		highScore = std::make_pair(username, newScore);
		saveHighScore();
	}
}

////////////////////////////////////////////////////////UI//////////////////////////////////////////////////////////

void drawStartMenu()
{
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);
	text("font/HeavyDataNerdFont-Regular.ttf", 50, "Pacman", 940, 100);
	text("font/HeavyDataNerdFont-Regular.ttf", 24, "Continue", 952, 350);
	text("font/HeavyDataNerdFont-Regular.ttf", 24, "New Game", 950, 400);
	text("font/HeavyDataNerdFont-Regular.ttf", 24, "Highscore", 947, 450);
	text("font/HeavyDataNerdFont-Regular.ttf", 24, "Options", 961, 500);
	text("font/HeavyDataNerdFont-Regular.ttf", 24, "Quit", 977, 550);
	//SDL_RenderPresent(renderer);
}

void drawOptionsMenu() {
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);
	text("font/HeavyDataNerdFont-Regular.ttf", 24, "Tutorial", 950, 450);
	text("font/HeavyDataNerdFont-Regular.ttf", 24, "Sound", 960, 500);
	text("font/HeavyDataNerdFont-Regular.ttf", 32, "Return of Title Menu", 1540, 900);
	backTexture = IMG_LoadTexture(renderer, "tilemap/back.png");
	SDL_RenderCopy(renderer, backTexture, NULL, &backRect);
	//SDL_RenderPresent(renderer);
}

void drawPauseMenu()
{
	text("font/HeavyDataNerdFont-Regular.ttf", 32, "Paused", 1750, 45);
	pauseTexture = IMG_LoadTexture(renderer, "tilemap/pauseImage.png");
	SDL_RenderCopy(renderer, pauseTexture, NULL, &pauseRect);
	text("font/HeavyDataNerdFont-Regular.ttf", 32, "Return of Title Menu", 1540, 900);
	backTexture = IMG_LoadTexture(renderer, "tilemap/back.png");
	SDL_RenderCopy(renderer, backTexture, NULL, &backRect);
	//SDL_RenderPresent(renderer);
}

void drawGameOverMenu()
{
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);
	text("font/HeavyDataNerdFont-Regular.ttf", 32, "Game Over", 935, 200);
	text("font/HeavyDataNerdFont-Regular.ttf", 32, "Return of Title Menu", 1540, 900);
	backTexture = IMG_LoadTexture(renderer, "tilemap/back.png");
	SDL_RenderCopy(renderer, backTexture, NULL, &backRect);
	//SDL_RenderPresent(renderer);
}

void drawVictoryScreen()
{
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);
	text("font/HeavyDataNerdFont-Regular.ttf", 32, "Victory", 950, 500);
	text("font/HeavyDataNerdFont-Regular.ttf", 32, "Return of Title Menu", 1540, 900);
	backTexture = IMG_LoadTexture(renderer, "tilemap/back.png");
	SDL_RenderCopy(renderer, backTexture, NULL, &backRect);
	//SDL_RenderPresent(renderer);
	victory = true;
}

void drawHighScore() {
	std::string highScoreText = highScore.first + ": " + std::to_string(highScore.second);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);
	text("font/HeavyDataNerdFont-Regular.ttf", 32, "Highsocre", 945, 200);
	text("font/HeavyDataNerdFont-Regular.ttf", 32, highScoreText.c_str(), 950, 500);
	text("font/HeavyDataNerdFont-Regular.ttf", 32, "Return of Title Menu", 1540, 900);
	backTexture = IMG_LoadTexture(renderer, "tilemap/back.png");
	SDL_RenderCopy(renderer, backTexture, NULL, &backRect);
}

std::string getUsername() {
	SDL_StartTextInput();
	bool stop = false;

	while (!stop) {
		while (SDL_PollEvent(&e) != 0) {
			if (e.type == SDL_KEYDOWN) {
				if (e.key.keysym.sym == SDLK_BACKSPACE && usernameInput.length() > 0) {
					usernameInput.pop_back();
				}
				else if (e.key.keysym.sym == SDLK_RETURN || e.key.keysym.sym == SDLK_KP_ENTER) {
					stop = true;
				}
			}
			else if (e.type == SDL_TEXTINPUT) {
				usernameInput += e.text.text;
			}
		}
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		SDL_RenderClear(renderer);
		text("font/HeavyDataNerdFont-Regular.ttf", 32, "Write Your Name", 100, 200);
		text("font/HeavyDataNerdFont-Regular.ttf", 28, usernameInput.c_str(), 100, 300);
		text("font/HeavyDataNerdFont-Regular.ttf", 24, "Press Enter", 100, 700);
		SDL_RenderPresent(renderer);
	}
	SDL_StopTextInput();
	return usernameInput;
}

void drawSoundMenu() {
	std::string volumeStr = std::to_string(musicVolume);
	SDL_StartTextInput();
	bool stop = false;

	while (!stop) {
		while (SDL_PollEvent(&e) != 0) {
			if (e.type == SDL_KEYDOWN) {
				if (e.key.keysym.sym == SDLK_BACKSPACE && volumeStr.length() > 0) {
					volumeStr.pop_back();
				}
				else if (e.key.keysym.sym == SDLK_RETURN || e.key.keysym.sym == SDLK_KP_ENTER) {
					stop = true;
				}
			}
			else if (e.type == SDL_TEXTINPUT) {
				if (isdigit(e.text.text[0])) {
					volumeStr += e.text.text;
				}
			}
		}
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		SDL_RenderClear(renderer);
		text("font/HeavyDataNerdFont-Regular.ttf", 32, "Volume (0-128)", 950, 200);
		text("font/HeavyDataNerdFont-Regular.ttf", 28, volumeStr.c_str(), 950, 300);
		text("font/HeavyDataNerdFont-Regular.ttf", 24, "Press Enter", 950, 700);
		SDL_RenderPresent(renderer);
	}
	SDL_StopTextInput();
	musicVolume = std::stoi(volumeStr);
	Mix_Volume(-1, musicVolume);
	gameState = GameState::OPTIONS_MENU;
}

void drawTutorial() {
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);
	text("font/FiraCodeNerdFont-Regular.ttf", 24, "Objective: Navigate Pacman through a maze, eating all dots while avoiding ghosts.", 50, 50);
	text("font/FiraCodeNerdFont-Regular.ttf", 24, "Controls: Use WASD / arrow keys to move Pacman.", 50, 80);
	text("font/FiraCodeNerdFont-Regular.ttf", 24, "Eat power pellets to turn the tables on ghosts for a short time.", 50, 110);
	text("font/FiraCodeNerdFont-Regular.ttf", 24, "Every dots contain 10 points, powerpill contain 20 points and eating afaraid ghosts gives 100 points.", 50, 140);
	text("font/FiraCodeNerdFont-Regular.ttf", 24, "Game highscore gets saved at pause menu, game over menu, vicoty menu and at the start of every level.", 50, 170);
	text("font/FiraCodeNerdFont-Regular.ttf", 28, "Some Cheats and shortcuts (Might break the game):", 50, 210);
	text("font/FiraCodeNerdFont-Regular.ttf", 24, "Press I to toggle Invincible mode", 50, 240);
	text("font/FiraCodeNerdFont-Regular.ttf", 24, "Press L to go to next level", 50, 270);
	text("font/FiraCodeNerdFont-Regular.ttf", 24, "Press K to get back to previous level", 50, 300);
	text("font/FiraCodeNerdFont-Regular.ttf", 24, "Press O to toggle options menu", 50, 330);
	text("font/FiraCodeNerdFont-Regular.ttf", 24, "Press H to get Game Over Screen", 50, 360);
	text("font/FiraCodeNerdFont-Regular.ttf", 24, "Press J to get Victory Screen", 50, 390);
	text("font/FiraCodeNerdFont-Regular.ttf", 24, "Press Escape to pause the game", 50, 420);
	text("font/FiraCodeNerdFont-Regular.ttf", 24, "Press F1 to show/hide the mouse", 50, 450);
	text("font/FiraCodeNerdFont-Regular.ttf", 24, "Press Ctrl+F to fullscreen(Experimential)", 50, 480);
	text("font/HeavyDataNerdFont-Regular.ttf", 24, "Return of Title Menu", 1540, 900);
	backTexture = IMG_LoadTexture(renderer, "tilemap/back.png");
	SDL_RenderCopy(renderer, backTexture, NULL, &backRect);
}

////////////////////////////////////////////////////////MAZE//////////////////////////////////////////////////////////

int spawnPosition[5][10] = {
	{634, 486, 858, 285, 858, 678, 1286, 678, 1286, 285},// 0
	{936, 478, 710, 35, 1250, 30, 710, 895, 1250, 895},// c
	{895, 480, 260, 195, 260, 735, 1600, 195, 1600, 735},// sarif
	{964, 129, 963, 545, 1543, 124, 388, 800, 1475, 197},// IAR
};

int maze0[30][60] = {
		{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
		{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
		{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
		{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
		{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
		{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
		{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
		{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
		{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
		{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 4, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 4, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
		{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
		{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 2, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 2, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
		{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 2, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 2, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
		{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 2, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 2, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
		{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 3, 3, 3, 3, 3, 3, 3, 2, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 2, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
		{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 1, 2, 2, 2, 2, 4, 2, 2, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 2, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
		{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 3, 3, 3, 3, 3, 3, 3, 2, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 2, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
		{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 2, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 2, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
		{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 2, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 2, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
		{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 2, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 2, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
		{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
		{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 4, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 4, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
		{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
		{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
		{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
		{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
		{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
		{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
		{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
		{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
};

int maze1[30][60] = {
		{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
		{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 4, 2, 2, 2, 2, 2, 2, 2, 3, 3, 2, 2, 2, 2, 2, 2, 2, 4, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
		{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 2, 3, 3, 3, 3, 3, 3, 2, 3, 3, 2, 3, 3, 3, 3, 3, 3, 2, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
		{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 2, 3, 2, 2, 2, 2, 3, 2, 3, 3, 2, 3, 2, 2, 2, 2, 3, 2, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
		{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 2, 3, 2, 3, 3, 2, 3, 2, 3, 3, 2, 3, 2, 3, 3, 2, 3, 2, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
		{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 2, 3, 2, 3, 3, 2, 3, 2, 3, 3, 2, 3, 2, 3, 3, 2, 3, 2, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
		{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 2, 2, 2, 3, 3, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 2, 2, 2, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
		{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 2, 3, 2, 3, 3, 2, 3, 2, 3, 3, 2, 3, 2, 3, 3, 2, 3, 2, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
		{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 2, 3, 2, 2, 2, 4, 3, 2, 3, 3, 2, 3, 4, 2, 2, 2, 3, 2, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
		{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 2, 3, 3, 3, 3, 3, 3, 2, 3, 3, 2, 3, 3, 3, 3, 3, 3, 2, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
		{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 2, 2, 2, 2, 2, 2, 2, 2, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
		{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 3, 3, 3, 2, 3, 2, 3, 2, 2, 2, 2, 3, 2, 3, 2, 3, 3, 3, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
		{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 2, 3, 2, 3, 3, 3, 3, 3, 3, 2, 3, 2, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
		{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 2, 3, 2, 2, 2, 2, 2, 2, 2, 2, 3, 2, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
		{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 3, 3, 3, 2, 3, 2, 3, 3, 1, 1, 3, 3, 2, 3, 2, 3, 3, 3, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
		{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 2, 2, 2, 2, 2, 2, 3, 1, 1, 1, 1, 3, 2, 2, 2, 2, 2, 2, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
		{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 3, 3, 3, 2, 3, 2, 3, 3, 3, 3, 3, 3, 2, 3, 2, 3, 3, 3, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
		{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 2, 3, 2, 2, 2, 2, 2, 2, 2, 2, 3, 2, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
		{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 2, 3, 2, 3, 3, 3, 3, 3, 3, 2, 3, 2, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
		{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 3, 3, 3, 2, 3, 2, 3, 4, 3, 3, 4, 3, 2, 3, 2, 3, 3, 3, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
		{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 2, 2, 2, 2, 3, 2, 3, 2, 2, 2, 2, 3, 2, 3, 2, 2, 2, 2, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
		{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 3, 3, 2, 3, 3, 2, 3, 3, 2, 2, 3, 3, 2, 3, 3, 2, 3, 3, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
		{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
		{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 3, 3, 2, 3, 3, 2, 3, 3, 3, 3, 3, 3, 2, 3, 3, 2, 3, 3, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
		{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 2, 2, 2, 3, 3, 2, 2, 2, 3, 3, 2, 2, 2, 3, 3, 2, 2, 2, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
		{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 2, 3, 3, 3, 3, 3, 3, 2, 3, 3, 2, 3, 3, 3, 3, 3, 3, 2, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
		{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
		{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 2, 3, 3, 3, 3, 3, 3, 2, 3, 3, 2, 3, 3, 3, 3, 3, 3, 2, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
		{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 4, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 4, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
		{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
};

int maze2[30][60] = {
		{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
		{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
		{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
		{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
		{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
		{ 1, 1, 1, 1, 1, 1, 1, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 1, 1, 1, 1, 1, 1, 1, 1 },
		{ 1, 1, 1, 1, 1, 1, 1, 3, 1, 4, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 4, 1, 3, 1, 1, 1, 1, 1, 1, 1, 1 },
		{ 1, 1, 1, 1, 1, 1, 1, 3, 4, 4, 3, 3, 3, 3, 3, 2, 3, 3, 3, 3, 3, 2, 3, 3, 3, 3, 3, 2, 3, 2, 3, 3, 3, 3, 3, 2, 3, 3, 3, 3, 3, 3, 2, 3, 3, 3, 3, 3, 3, 4, 4, 3, 1, 1, 1, 1, 1, 1, 1, 1 },
		{ 1, 1, 1, 1, 1, 1, 1, 3, 2, 3, 1, 1, 1, 1, 3, 2, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 2, 3, 2, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 2, 3, 1, 1, 1, 1, 1, 3, 2, 3, 1, 1, 1, 1, 1, 1, 1, 1 },
		{ 1, 1, 1, 1, 1, 1, 1, 3, 2, 3, 1, 1, 1, 1, 3, 2, 3, 2, 3, 3, 3, 3, 3, 3, 3, 2, 3, 2, 3, 2, 3, 2, 3, 3, 3, 3, 3, 3, 3, 3, 2, 3, 2, 3, 1, 1, 1, 1, 1, 3, 2, 3, 1, 1, 1, 1, 1, 1, 1, 1 },
		{ 1, 1, 1, 1, 1, 1, 1, 3, 2, 3, 1, 1, 1, 1, 3, 2, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 2, 3, 2, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 2, 3, 1, 1, 1, 1, 1, 3, 2, 3, 1, 1, 1, 1, 1, 1, 1, 1 },
		{ 1, 1, 1, 1, 1, 1, 1, 3, 2, 3, 1, 1, 1, 1, 3, 2, 3, 2, 3, 3, 3, 3, 3, 3, 3, 2, 3, 2, 3, 2, 3, 2, 3, 3, 3, 3, 3, 3, 3, 3, 2, 3, 2, 3, 1, 1, 1, 1, 1, 3, 2, 3, 1, 1, 1, 1, 1, 1, 1, 1 },
		{ 1, 1, 1, 1, 1, 1, 1, 3, 2, 3, 3, 3, 3, 3, 3, 2, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 2, 3, 2, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 2, 3, 3, 3, 3, 3, 3, 3, 2, 3, 1, 1, 1, 1, 1, 1, 1, 1 },
		{ 1, 1, 1, 1, 1, 1, 1, 3, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 2, 3, 3, 3, 3, 3, 3, 2, 3, 2, 3, 2, 3, 2, 3, 3, 3, 3, 3, 3, 3, 2, 3, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 1, 1, 1, 1, 1, 1, 1, 1 },
		{ 1, 1, 1, 1, 1, 1, 1, 3, 2, 3, 3, 3, 3, 3, 3, 2, 3, 3, 2, 2, 2, 2, 2, 2, 2, 2, 3, 2, 3, 2, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 2, 3, 3, 3, 3, 3, 3, 3, 2, 3, 1, 1, 1, 1, 1, 1, 1, 1 },
		{ 1, 1, 1, 1, 1, 1, 1, 3, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 2, 3, 3, 3, 3, 3, 3, 2, 3, 2, 2, 2, 3, 2, 3, 3, 3, 3, 3, 3, 3, 2, 3, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 1, 1, 1, 1, 1, 1, 1, 1 },
		{ 1, 1, 1, 1, 1, 1, 1, 3, 2, 3, 3, 3, 3, 3, 3, 2, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 2, 3, 2, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 2, 3, 3, 3, 3, 3, 3, 3, 2, 3, 1, 1, 1, 1, 1, 1, 1, 1 },
		{ 1, 1, 1, 1, 1, 1, 1, 3, 2, 3, 1, 1, 1, 1, 3, 2, 3, 2, 3, 3, 3, 3, 3, 3, 3, 2, 3, 2, 3, 2, 3, 2, 3, 3, 3, 3, 3, 3, 3, 3, 2, 3, 2, 3, 1, 1, 1, 1, 1, 3, 2, 3, 1, 1, 1, 1, 1, 1, 1, 1 },
		{ 1, 1, 1, 1, 1, 1, 1, 3, 2, 3, 1, 1, 1, 1, 3, 2, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 2, 3, 2, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 2, 3, 1, 1, 1, 1, 1, 3, 2, 3, 1, 1, 1, 1, 1, 1, 1, 1 },
		{ 1, 1, 1, 1, 1, 1, 1, 3, 2, 3, 1, 1, 1, 1, 3, 2, 3, 2, 3, 3, 3, 3, 3, 3, 3, 2, 3, 2, 3, 2, 3, 2, 3, 3, 3, 3, 3, 3, 3, 3, 2, 3, 2, 3, 1, 1, 1, 1, 1, 3, 2, 3, 1, 1, 1, 1, 1, 1, 1, 1 },
		{ 1, 1, 1, 1, 1, 1, 1, 3, 2, 3, 1, 1, 1, 1, 3, 2, 3, 2, 3, 3, 3, 3, 3, 3, 3, 2, 3, 2, 3, 2, 3, 2, 3, 3, 3, 3, 3, 3, 3, 3, 2, 3, 2, 3, 1, 1, 1, 1, 1, 3, 2, 3, 1, 1, 1, 1, 1, 1, 1, 1 },
		{ 1, 1, 1, 1, 1, 1, 1, 3, 2, 3, 1, 1, 1, 1, 3, 2, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 2, 3, 2, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 2, 3, 1, 1, 1, 1, 1, 3, 2, 3, 1, 1, 1, 1, 1, 1, 1, 1 },
		{ 1, 1, 1, 1, 1, 1, 1, 3, 4, 4, 3, 3, 3, 3, 3, 2, 3, 3, 3, 3, 3, 2, 3, 3, 3, 3, 3, 2, 3, 2, 3, 3, 3, 3, 3, 3, 2, 3, 3, 3, 3, 3, 2, 3, 3, 3, 3, 3, 3, 4, 4, 3, 1, 1, 1, 1, 1, 1, 1, 1 },
		{ 1, 1, 1, 1, 1, 1, 1, 3, 1, 4, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 4, 1, 3, 1, 1, 1, 1, 1, 1, 1, 1 },
		{ 1, 1, 1, 1, 1, 1, 1, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 1, 1, 1, 1, 1, 1, 1, 1 },
		{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
		{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
		{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
		{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
		{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 }
};

int maze3[30][60] = {
		{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
		{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
		{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
		{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 4, 3, 2, 3, 2, 2, 2, 2, 3, 2, 2, 2, 2, 3, 2, 3, 4, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
		{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 2, 2, 2, 3, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
		{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 3, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 2, 3, 3, 3, 2, 2, 3, 2, 2, 3, 3, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 3, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
		{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 4, 2, 3, 1, 1, 1, 1, 1, 1, 1, 1, 3, 2, 2, 3, 3, 2, 2, 3, 3, 3, 2, 2, 3, 3, 2, 2, 3, 4, 2, 2, 2, 2, 2, 2, 2, 3, 2, 4, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
		{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 3, 2, 3, 1, 1, 1, 1, 1, 1, 1, 1, 3, 2, 3, 3, 2, 2, 3, 3, 1, 3, 3, 2, 2, 3, 3, 2, 3, 2, 3, 3, 3, 3, 3, 3, 2, 3, 2, 3, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
		{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 2, 2, 3, 3, 3, 3, 1, 1, 3, 3, 3, 3, 2, 3, 2, 2, 3, 3, 1, 3, 1, 3, 3, 2, 2, 3, 2, 3, 2, 3, 1, 1, 1, 1, 3, 2, 3, 2, 2, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
		{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 2, 2, 2, 2, 4, 3, 1, 1, 3, 2, 2, 2, 2, 2, 2, 3, 3, 1, 3, 1, 3, 1, 3, 3, 2, 2, 2, 3, 2, 3, 1, 1, 1, 1, 3, 2, 3, 2, 3, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
		{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 2, 3, 3, 3, 2, 3, 1, 1, 3, 2, 3, 3, 3, 2, 3, 3, 1, 3, 1, 1, 1, 3, 1, 3, 3, 2, 3, 3, 2, 3, 1, 1, 1, 1, 3, 2, 3, 2, 2, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
		{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 2, 3, 3, 3, 2, 3, 1, 1, 3, 2, 3, 3, 3, 2, 3, 1, 3, 1, 1, 1, 1, 1, 3, 1, 3, 2, 2, 3, 2, 3, 1, 1, 1, 1, 3, 2, 3, 2, 3, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
		{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 2, 3, 3, 3, 2, 3, 1, 1, 3, 2, 3, 3, 3, 2, 3, 1, 3, 1, 1, 1, 1, 1, 3, 1, 3, 2, 3, 3, 2, 3, 1, 1, 1, 1, 3, 2, 3, 2, 2, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
		{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 2, 3, 3, 3, 2, 3, 1, 1, 3, 2, 3, 3, 3, 2, 3, 1, 3, 1, 1, 1, 1, 1, 3, 1, 3, 2, 2, 3, 2, 3, 1, 1, 1, 1, 3, 2, 3, 2, 3, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
		{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 2, 3, 3, 3, 2, 3, 1, 1, 3, 2, 3, 3, 3, 2, 3, 1, 3, 1, 1, 1, 1, 1, 3, 1, 3, 2, 3, 3, 2, 3, 3, 3, 3, 3, 3, 2, 3, 2, 2, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
		{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 2, 3, 3, 3, 2, 3, 1, 1, 3, 2, 3, 3, 3, 2, 3, 1, 3, 1, 1, 1, 1, 1, 3, 1, 3, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 4, 3, 2, 3, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
		{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 2, 3, 3, 3, 2, 3, 1, 1, 3, 2, 3, 3, 3, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 3, 3, 2, 2, 3, 3, 3, 3, 3, 3, 3, 2, 2, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
		{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 2, 3, 3, 3, 2, 3, 1, 1, 3, 2, 3, 3, 3, 2, 3, 4, 3, 2, 2, 4, 2, 2, 3, 4, 3, 2, 2, 3, 2, 3, 1, 3, 3, 2, 2, 2, 2, 2, 3, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
		{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 2, 3, 3, 3, 2, 3, 1, 1, 3, 2, 3, 3, 3, 2, 3, 2, 3, 2, 3, 3, 3, 2, 3, 2, 3, 2, 3, 3, 2, 3, 3, 1, 3, 3, 2, 2, 3, 2, 2, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
		{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 2, 3, 3, 3, 2, 3, 1, 1, 3, 2, 3, 3, 3, 2, 3, 2, 3, 2, 3, 3, 3, 2, 3, 2, 3, 2, 2, 3, 2, 3, 3, 3, 1, 3, 3, 2, 2, 2, 2, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
		{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 2, 2, 2, 2, 2, 3, 1, 1, 3, 4, 2, 2, 2, 2, 3, 2, 3, 2, 3, 3, 3, 2, 3, 2, 3, 2, 3, 3, 2, 3, 2, 3, 3, 1, 3, 3, 2, 2, 3, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
		{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 3, 2, 3, 3, 3, 3, 1, 1, 3, 3, 3, 3, 3, 2, 3, 2, 3, 2, 3, 3, 3, 2, 3, 2, 3, 2, 2, 3, 2, 3, 2, 3, 3, 3, 1, 3, 3, 2, 2, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
		{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 2, 2, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 2, 3, 2, 3, 2, 3, 3, 3, 2, 3, 2, 3, 2, 3, 3, 2, 3, 2, 3, 2, 3, 3, 1, 3, 3, 2, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
		{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 3, 2, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 2, 3, 2, 3, 2, 3, 3, 3, 2, 3, 2, 3, 2, 2, 3, 2, 3, 2, 3, 2, 2, 3, 3, 1, 3, 2, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
		{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 3, 2, 3, 2, 3, 3, 3, 2, 3, 2, 3, 2, 3, 3, 2, 3, 2, 3, 2, 2, 2, 3, 3, 3, 2, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
		{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 4, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
		{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
		{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
		{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
		{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 }
};
