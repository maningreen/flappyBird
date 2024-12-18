#include <SDL2/SDL.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>
#include <math.h>
#include <time.h>

#define gravity 20
#define jumpStrength 10
#define sqrt2 sqrtf(2.0f)
#define pipeSpeed 5
#define pipesOnScreen 2
#define maxChangePipes 250
#define maxPipePos (position){-50, 400} //min, max
#define pointsPerPipe 10

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

int min(int a, int b) {
  return a > b ? b : a;
}

int max(int a, int b) {
  return a > b ? a : b;
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

int pipeColliding(SDL_Rect* a, pipe* b) {
  SDL_Rect buf;
  return SDL_IntersectRect(a, &b->a, &buf) || SDL_IntersectRect(a, &b->b, &buf) || 
  SDL_IntersectRect(a, &b->c, &buf) || SDL_IntersectRect(a, &b->d, &buf);
}

int main() {

  srand(time(NULL));

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
  float delta;  //no unsigned here (you can't) :(
  float timer = 0;

  pipe pipeList[pipesOnScreen];
  for(int i = 0; i < pipesOnScreen; i++) {
    initPipe(&pipeList[i], &(position){(winSize.x + 60)/pipesOnScreen * i + winSize.x, winSize.y/2.0f});
    pipeList[i].Basecol = birdColour;
    pipeList[i].outCol = black;
    pipeList[i].borderWidth = 5;
  }

  int score = 0;

  //using while wastes an instruction (cmp)
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

  drawCircle(renderer, birdPos, birdR, birdColour);
  for(int i = 0; i < pipesOnScreen; i++) {
    drawPipe(renderer, &pipeList[i]);
    movePipe(&pipeList[i], &(position){-pipeSpeed, 0});
    if(pipeList[i].b.x < -60) {
      srand(time(NULL));
      int lastY = pipeList[i != pipesOnScreen - 1 ? i : 0].b.y;
      int off = rand() % (maxChangePipes << 1) - maxChangePipes;
      int newPos = max(min(lastY + off, maxPipePos.y), maxPipePos.x);
      initPipe(&pipeList[i], &(position){winSize.x + 5, newPos});
      score += pointsPerPipe;
    } else if(pipeColliding(&birdBox, &pipeList[i])) {
      goto end;
    }
  }

  SDL_RenderPresent(renderer);

  if(birdPos.y >= winSize.y || birdPos.y <= 0)
    goto end;

  birdPos.y += birdV;
  birdBox.y = birdPos.y - birdR/sqrt2; //reason we use sqrt 2 is because sin(pi/2) == sqrt2
                                       //specifically that is 90 degrees, and thats where a square with 4 intersections must lie
  birdV += gravity * delta;            //and we do birdr/sqrt2 because...... i forgot why to be honest

  oldTime = newTime;
  SDL_Delay(20);
  goto loop;
end:
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();

  printf("your score was %d\n", score);

  return 0;
}
