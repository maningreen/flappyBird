#include <SDL2/SDL.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>
#include <math.h>

#define gravity 20
#define jumpStrength 10
#define sqrt2 sqrtf(2.0f)
#define pipeSpeed 100

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

struct drawRect {
  SDL_Rect rec;
  struct colour col;
};


typedef struct float2 position; //shorthand
typedef struct float2 vector2;
typedef struct colour colour;   //i got tired of typing struct all the time last project
typedef struct drawRect rect;

struct pipe {
  SDL_Rect a;
  SDL_Rect b;
  SDL_Rect c;
  SDL_Rect d;
  colour Basecol;
  colour outCol;
  int borderWidth;
};

typedef struct pipe pipe;

#define backround (colour){220, 134, 148, 255}
#define birdColour (colour){226, 53, 102, 255}
#define black (colour){0, 0, 0, 255}

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

void drawRect(SDL_Renderer* renderer, rect* inRect) {
  setColour(renderer, inRect->col);
  SDL_RenderFillRect(renderer, &inRect->rec);
}

void moveRect(SDL_Rect* in, position* offset) {
  in->x += offset->x;
  in->y += offset->y;
}

void movePipe(pipe* in, position* offest) {
  moveRect(&in->a, offest);
  moveRect(&in->b, offest);
  moveRect(&in->c, offest);
  moveRect(&in->d, offest);
}

void drawRectWithOutline(SDL_Renderer* renderer, rect *inRect, int borderWidth, colour col) {
  //first calculate the outline
  SDL_Rect border;
  border.x = inRect->rec.x - borderWidth;
  border.y = inRect->rec.y - borderWidth;
  border.w = inRect->rec.w + (borderWidth << 1);
  border.h = inRect->rec.h + (borderWidth << 1); //<< muls by 2
  drawRect(renderer, &(rect){border, col});
  drawRect(renderer, inRect);
}

void drawPipe(SDL_Renderer* renderer, pipe* in) {
  drawRectWithOutline(renderer, &(rect){in->a, in->Basecol}, in->borderWidth, in->outCol);
  drawRectWithOutline(renderer, &(rect){in->b, in->Basecol}, in->borderWidth, in->outCol);
  drawRectWithOutline(renderer, &(rect){in->c, in->Basecol}, in->borderWidth, in->outCol);
  drawRectWithOutline(renderer, &(rect){in->d, in->Basecol}, in->borderWidth, in->outCol);
}

void initPipe(pipe* in, position* pos) {
  const position capSize = (position){70,20};
  const position midSize = (position){60,1000};
  const float yOff = 250;
  in->a = (SDL_Rect){pos->x - (midSize.x/2 - capSize.x/2), pos->y - midSize.y, midSize.x, midSize.y};
  in->b = (SDL_Rect){pos->x, pos->y, capSize.x, capSize.y};
  in->c = (SDL_Rect){pos->x - (midSize.x/2 - capSize.x/2), pos->y + midSize.y + yOff, midSize.x, -midSize.y};
  in->d = (SDL_Rect){pos->x, pos->y + yOff, capSize.x, capSize.y};
}

int rectColliding(rect* a, rect* b) {
  SDL_Rect buf;
  return SDL_IntersectRect(&a->rec, &b->rec, &buf);
}

int sdlrectColliding(SDL_Rect* a, rect* b) {
  SDL_Rect buf;
  return SDL_IntersectRect(a, &b->rec, &buf);
}

int main() {
  SDL_Init(SDL_INIT_VIDEO);

  position winSize = {1000, 600};
  SDL_Window* window = SDL_CreateWindow("i use arch btw :3", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, winSize.x, winSize.y, SDL_WINDOW_SHOWN);

  SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

  //create bird position
  position birdPos = {winSize.x/10, winSize.y/2.0f};
  float birdV = 0;
  const float birdR = 25;
  SDL_Rect birdBox = {birdPos.x - birdR/sqrt2, birdPos.y - birdR/sqrt2, birdR * sqrt2, birdR * sqrt2};

  unsigned int oldTime = SDL_GetTicks(); //time isn't negative hence the unsigned
  unsigned int newTime;
  float delta;  //no unsigned here (you can't :( )
  float timer = 0;

  pipe testPipe;
  initPipe(&testPipe, &(position){500, 300});
  testPipe.borderWidth = 12;
  testPipe.Basecol = birdColour;
  testPipe.outCol = black;

  rect test = {(SDL_Rect){0,0,100,100}, birdColour};

  //using while wastes an instruction (at least in arm)
loop:
  newTime = SDL_GetTicks();
  delta = (newTime - oldTime)/1000.0f;
  timer += delta;
  SDL_Event event;
  while(SDL_PollEvent(&event)) {
    if(event.type == SDL_QUIT) {
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

  drawRectWithOutline(renderer, &test, 12, black);
  drawCircle(renderer, birdPos, birdR, birdColour);

  drawPipe(renderer, &testPipe);


  SDL_RenderPresent(renderer);

  if(birdPos.y >= winSize.y || birdPos.y <= 0)
    goto end;

  SDL_Rect buffer;
  if(sdlrectColliding(&birdBox, &test))
    goto end;

  birdPos.y += birdV;
  birdBox.y = birdPos.y - birdR/sqrt2;
  birdV += gravity * delta;

  oldTime = newTime;
  SDL_Delay(16);
  goto loop;
end:
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}
