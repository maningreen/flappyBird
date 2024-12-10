#include <SDL2/SDL.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>
#include <math.h>

#define gravity 20
#define jumpStrength 10
#define sqrt2 sqrtf(2.0f)

struct float2 {
  float x;
  float y;
};

struct colour {
  unsigned char r; //chars are 1 byte (0 - 255)
  unsigned char g;
  unsigned char b;
  unsigned char a;
};

struct rect2 {
  SDL_Rect a;
  SDL_Rect b;
};

typedef struct float2 position; //shorthand
typedef struct colour colour;   //i got tired of typing struct all the time last project
typedef struct rect2 pipe;

const colour birdColour = {226, 53, 102, 255};
const colour backround = {220, 134, 159, 255};
const colour black = {0, 0, 0, 255};

void setColour(SDL_Renderer* renderer, colour col) {
  SDL_SetRenderDrawColor(renderer, col.r, col.g, col.b, col.a);
}

void drawCircle(SDL_Renderer* renderer, position pos, float radius, colour col) {
  const float twpi = M_PI * 2;
  const float incr = 1/(radius * twpi);
  setColour(renderer, col);
  for(float i = 0; i <= twpi; i += incr) {
    position cosSine = {SDL_cos(i) * radius, SDL_sin(i) * radius};
    SDL_RenderDrawLine(renderer, pos.x, pos.y, pos.x + cosSine.x, pos.y + cosSine.y);
  }
}

SDL_Rect drawRectWithOutLine(SDL_Renderer* renderer, position pos, position size, colour col, colour colOutline, int outWidth) {
  const unsigned int topOffset = 10;
  SDL_Rect mainBody = {pos.x - size.x/2, pos.y - size.y/2, size.x/2, size.y/2};
  SDL_Rect mainOutLine = {mainBody.x - outWidth/2.0f, mainBody.y - outWidth/2.0f, mainBody.w + outWidth, mainBody.h + outWidth};
  setColour(renderer, colOutline);
  SDL_RenderFillRect(renderer, &mainOutLine);   //think of it like layering pancakes, you want the biggest one on the bottom, but you can still see the edges
  setColour(renderer, col);
  SDL_RenderFillRect(renderer, &mainBody);
  return mainBody;
}

pipe drawPipe(SDL_Renderer* renderer, position pos) {
  pipe out;
  out.a = drawRectWithOutLine(renderer, pos, (position){100, 1000}, birdColour,
                      black, 10);
  out.b = drawRectWithOutLine(renderer, (position){pos.x + 7, pos.y + 1}, (position){126, 75}, birdColour,
                        black, 10);
  return out;
}

int getPipeColliding(pipe in, SDL_Rect rect) {
  SDL_Rect buffer;
  return SDL_IntersectRect(&in.a, &rect, &buffer) || SDL_IntersectRect(&in.b, &rect, &buffer);
}

int main() {
  SDL_Init(SDL_INIT_VIDEO);

  position winSize = {800, 600};
  SDL_Window* window = SDL_CreateWindow("i use arch btw :3", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, winSize.x, winSize.y, SDL_WINDOW_SHOWN);

  SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

  //create bird position
  position birdPos = {winSize.x/10, winSize.y/2.0f};
  float birdV = 0;
  const float birdR = 25;
  SDL_Rect birdBox = {birdPos.x - birdR/sqrt2, birdPos.y - birdR/sqrt2, birdR * sqrt2, birdR * sqrt2};

  unsigned int oldTime = SDL_GetTicks(); //time isn't negative hence the unsigned
  unsigned int newTime;
  float delta;  //no unsigned here (you can't)
  float timer = 0;

  while(1) {
    newTime = SDL_GetTicks();
    delta = (newTime - oldTime)/1000.0f;
    timer += delta;
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        goto end;
      } else if(event.type == SDL_KEYDOWN) {
        if(event.key.keysym.sym == SDLK_q && (event.key.keysym.mod == KMOD_LSHIFT ||  //this ordering does matter
          event.key.keysym.mod == KMOD_RSHIFT)) {
          goto end;
        } else if(event.key.keysym.sym == SDLK_SPACE) {
          birdV = -jumpStrength;
        }
      }
    }

    SDL_SetRenderDrawColor(renderer, backround.r, backround.g, backround.b, backround.a);
    SDL_RenderClear(renderer);

    drawCircle(renderer, birdPos, birdR, birdColour);

    pipe pip = drawPipe(renderer, (position){200, 200});

    if(getPipeColliding(pip, birdBox)) goto end;

    SDL_RenderPresent(renderer);

    if(birdPos.y >= winSize.y || birdPos.y <= 0) {
      goto end;
    }

    birdPos.y += birdV;
    birdBox.y = birdPos.y - birdR/sqrt2;
    birdV += gravity * delta;

    oldTime = newTime;
    SDL_Delay(16);
  }
end:
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}
