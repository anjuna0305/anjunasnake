#include <SDL2/SDL.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_timer.h>
#include <stdlib.h>
#include <string.h>

#define GRID_WIDTH 30
#define GRID_HEIGHT 40
#define RECT_SIZE 15
#define SNAKE_LENGTH (GRID_WIDTH * GRID_HEIGHT)
#define WALL_SIZE 15

#define UP -1
#define DOWN 1
#define LEFT -1
#define RIGHT 1

typedef struct {
  int x;
  int y;
} SnakePoint;

SnakePoint snake[SNAKE_LENGTH];
int snakeLength = 0;
int rear = 0, front = 0;
int directX = 1, directY = 0;
int grid[GRID_WIDTH + 2][GRID_HEIGHT + 2] = {0};
SnakePoint basePoint = {.x = 0, .y = 0};
SnakePoint berryPoint = {.x = 0, .y = 0};
int isBerry = 0;

void initGrid() {
  memset(grid, 0, sizeof(grid));
  for (int i = 0; i < GRID_WIDTH + 2; i++) {
    grid[i][0] = 1;
    grid[i][GRID_HEIGHT + 1] = 1;
  }
  for (int i = 0; i < GRID_HEIGHT + 2; i++) {
    grid[0][i] = 1;
    grid[GRID_WIDTH + 1][i] = 1;
  }
}

void addToGrid(int x, int y) { grid[x + 1][y + 1] = 1; }

void removeFromGrid(int x, int y) { grid[x + 1][y + 1] = 0; }

int isCollide(int x, int y) { return grid[x + 1][y + 1]; }

void enqueue(int x, int y) {
  if (snakeLength >= SNAKE_LENGTH)
    return;

  snake[front].x = x;
  snake[front].y = y;
  addToGrid(x, y);

  snakeLength++;
  front = (front + 1) % SNAKE_LENGTH;
}

SnakePoint dequeue() {
  if (snakeLength <= 0)
    return basePoint;
  SnakePoint temp = snake[rear];

  rear = (rear + 1) % SNAKE_LENGTH;
  snakeLength--;
  removeFromGrid(temp.x, temp.y);
  return temp;
}

void drawSnake(SDL_Renderer *renderer) {
  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

  for (int i = 0; i < snakeLength; i++) {
    SDL_Rect rect = {snake[(rear + i) % SNAKE_LENGTH].x * RECT_SIZE + WALL_SIZE,
                     snake[(rear + i) % SNAKE_LENGTH].y * RECT_SIZE + WALL_SIZE,
                     RECT_SIZE, RECT_SIZE};
    SDL_RenderFillRect(renderer, &rect);
  }
}

void drawWall(SDL_Renderer *renderer) {
  SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);

  SDL_Rect rect0 = {0, 0, RECT_SIZE * GRID_WIDTH + 2 * WALL_SIZE, WALL_SIZE};
  SDL_RenderFillRect(renderer, &rect0);

  SDL_Rect rect1 = {0, 0, WALL_SIZE, RECT_SIZE * GRID_HEIGHT + 2 * WALL_SIZE};
  SDL_RenderFillRect(renderer, &rect1);

  SDL_Rect rect2 = {0, GRID_HEIGHT * RECT_SIZE + WALL_SIZE,
                    RECT_SIZE * GRID_WIDTH + 2 * WALL_SIZE, WALL_SIZE};
  SDL_RenderFillRect(renderer, &rect2);

  SDL_Rect rect3 = {GRID_WIDTH * RECT_SIZE + WALL_SIZE, 0, WALL_SIZE,
                    RECT_SIZE * GRID_HEIGHT + 2 * WALL_SIZE};
  SDL_RenderFillRect(renderer, &rect3);
}

void drawBerry(SnakePoint berry, SDL_Renderer *renderer) {
  SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
  SDL_Rect rect = {(berry.x) * RECT_SIZE + WALL_SIZE,
                   (berry.y) * RECT_SIZE + WALL_SIZE, RECT_SIZE, RECT_SIZE};
  SDL_RenderFillRect(renderer, &rect);
}

void setSnakePosition(int newx, int newy, int index) {
  snake[index].x = newx;
  snake[index].y = newy;
}

SnakePoint getBerryLocation() {
  if (snakeLength >= GRID_WIDTH * GRID_HEIGHT) {
    return basePoint;
  }

  int randX = rand() % GRID_WIDTH;
  int randY = rand() % GRID_HEIGHT;
  if (grid[randX][randY] == 0)
    return (SnakePoint){randX, randY};

  return getBerryLocation();
}

int gameLoop(SDL_Renderer *renderer) {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    if (event.type == SDL_KEYDOWN) {
      switch (event.key.keysym.sym) {
      case SDLK_UP:
        directX = 0;
        directY = directY == DOWN ? DOWN : UP;
        break;

      case SDLK_DOWN:
        directX = 0;
        directY = directY == UP ? UP : DOWN;
        break;

      case SDLK_LEFT:
        directX = directX == RIGHT ? RIGHT : LEFT;
        directY = 0;
        break;

      case SDLK_RIGHT:
        directX = directX == LEFT ? LEFT : RIGHT;
        directY = 0;
        break;

      case SDLK_q:
        return 0;
      }
    }
  }
  if (isBerry == 0) {
    berryPoint = getBerryLocation();
    isBerry = 1;
  }

  SnakePoint head = snake[(front - 1 + SNAKE_LENGTH) % SNAKE_LENGTH];
  int newx, newy;

  if (directY == 0) {
    newx = head.x + directX;
    newy = head.y;
  } else if (directX == 0) {
    newx = head.x;
    newy = head.y + directY;
  }

  if (isCollide(newx, newy))
    return 0;

  enqueue(newx, newy);
  if (berryPoint.x == newx && berryPoint.y == newy) {
    isBerry = 0;
  } else {
    dequeue();
  }

  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
  SDL_RenderClear(renderer);

  drawWall(renderer);
  drawSnake(renderer);
  if (isBerry)
    drawBerry(berryPoint, renderer);
  SDL_RenderPresent(renderer);

  SDL_Delay(100);
  return 1;
}

int main() {
  SDL_Init(SDL_INIT_VIDEO);

  SDL_Window *window = SDL_CreateWindow(
      "anjuna's snake", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
      RECT_SIZE * GRID_WIDTH + 2 * WALL_SIZE,
      RECT_SIZE * GRID_HEIGHT + 2 * WALL_SIZE, 0);

  SDL_Renderer *renderer =
      SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

  initGrid();

  setSnakePosition(5, 5, 0);
  setSnakePosition(5, 5, 0);
  setSnakePosition(5, 5, 0);
  front = 3;
  rear = 0;
  snakeLength = 3;
  //
  // if (directY == 0) {
  //   enqueue(snake[front].x + directX, snake[front].y);
  // } else if (directX == 0) {
  //   enqueue(snake[front].x, snake[front].y + directY);
  // }
  // dequeue();

  int running = 1;
  while (running) {
    running = gameLoop(renderer);
  }
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}
