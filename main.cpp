#include <iostream>
#include <random>
#include <vector>

#include <SDL2/SDL.h>

static void draw(SDL_Renderer* windowRenderer);
static void drawTiles(SDL_Renderer* windowRenderer);
static bool createTextures(SDL_Renderer* windowRenderer);
static void destroyTextures();

constexpr uint16_t NUM_TEXTURES = 120;
constexpr uint16_t TEXTURE_WIDTH = 90;
constexpr uint16_t TEXTURE_HEIGHT = 60;

std::random_device dev;
std::mt19937_64 engine{dev()};
SDL_Texture* buffer = nullptr;
SDL_DisplayMode dimension;
std::vector<SDL_Texture*> textures;

int main(int argc, char** argv) {
  SDL_SetHint(SDL_HINT_RENDER_BATCHING, "1");
  SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl");

  if (SDL_InitSubSystem(SDL_INIT_VIDEO) == -1) {
    std::cerr << "SDL_InitSubSystem -1" << std::endl;
    return 1;
  }

  if (SDL_GetCurrentDisplayMode(0, &dimension) != 0) {
    std::cerr << "SDL_GetCurrentDisplayMode non-zero" << std::endl;
    return 1;
  }

  SDL_Window *window =
    SDL_CreateWindow(
        "Demo"
      , SDL_WINDOWPOS_CENTERED
      , SDL_WINDOWPOS_CENTERED
      , dimension.w
      , dimension.h
      , SDL_WINDOW_SHOWN | SDL_WINDOW_FULLSCREEN_DESKTOP
    );
  if (window == nullptr) {
    std::cerr << "SDL_CreateWindow null" << std::endl;
    return 1;
  }

  SDL_Renderer* windowRenderer =
    SDL_CreateRenderer(window, -1, SDL_RENDERER_TARGETTEXTURE | SDL_RENDERER_ACCELERATED);
  if (windowRenderer == nullptr) {
    std::cerr << "SDL_CreateRenderer(window) null" << std::endl;
    return 1;
  }

  SDL_RenderSetLogicalSize(windowRenderer, 1024, 768);

  bool running = true;

  if (!createTextures(windowRenderer)) {
    std::cerr << "Failed to create any of the textures." << std::endl;
    return 1;
  }

  drawTiles(windowRenderer);

  while (running) {
    SDL_Event event;
    bool newEvent = SDL_PollEvent(&event) == 1;

    if (newEvent) {
      switch (event.type) {
        case SDL_WINDOWEVENT:
          switch (event.window.event) {
            case SDL_WINDOWEVENT_CLOSE:
              running = false;
              continue;
          }
          break;
        case SDL_KEYDOWN:
          if (event.key.state == SDL_PRESSED) {
            switch (event.key.keysym.sym) {
              case SDLK_ESCAPE:
                running = false;
                continue;
            }
          }
      }
    }

    draw(windowRenderer);
  }

  destroyTextures();
  SDL_DestroyRenderer(windowRenderer);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}

bool createTextures(SDL_Renderer* windowRenderer) {
  buffer =
    SDL_CreateTexture(
        windowRenderer
      , SDL_PIXELFORMAT_ARGB8888
      , SDL_TEXTUREACCESS_TARGET
      , 1024
      , 768
    );
  if (buffer == nullptr) {
    return false;
  }

  std::uniform_int_distribution<uint8_t> distribution(10, 255);

  textures.reserve(NUM_TEXTURES);
  for (uint16_t i = 0; i < NUM_TEXTURES; ++i) {
    std::vector<uint32_t> pixels;
    pixels.resize(TEXTURE_WIDTH * TEXTURE_HEIGHT);

    uint8_t r = distribution(engine);
    uint8_t g = distribution(engine);
    uint8_t b = distribution(engine);
    uint8_t a = distribution(engine);
    uint32_t color = a << 24 | r << 16 | g << 8 | b;

    SDL_Surface* surface =
      SDL_CreateRGBSurfaceWithFormat(
          0
        , TEXTURE_WIDTH
        , TEXTURE_HEIGHT
        , 32
        , SDL_PIXELFORMAT_ARGB8888
      );
    if (surface == nullptr) {
      std::cerr << "SDL_CreateRGBSurfaceFrom null" << std::endl;
      return 1;
    }

    SDL_FillRect(surface, nullptr, color);

    SDL_Texture* texture =
      SDL_CreateTextureFromSurface(windowRenderer, surface);
    if (texture == nullptr) {
      std::cerr << "SDL_CreateRGBSurfaceFrom null" << std::endl;
      return 1;
    }

    textures.push_back(texture);

    SDL_FreeSurface(surface);
  }

  return true;
}

void drawTiles(SDL_Renderer* windowRenderer) {
  SDL_SetRenderTarget(windowRenderer, buffer);
  SDL_SetRenderDrawColor(windowRenderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
  SDL_RenderClear(windowRenderer);
  for (uint16_t i = 0; i < NUM_TEXTURES; ++i) {
    auto t = textures[i];

    std::uniform_int_distribution<int16_t> distX(0, 1024-TEXTURE_WIDTH);
    std::uniform_int_distribution<int16_t> distY(0, 768-TEXTURE_HEIGHT);

    int x = distX(engine);
    int y = distY(engine);

    SDL_Rect textureDest {x , y , TEXTURE_WIDTH , TEXTURE_HEIGHT};
    SDL_SetTextureBlendMode(t, SDL_BLENDMODE_BLEND);
    SDL_RenderSetClipRect(windowRenderer, &textureDest);
    SDL_RenderCopy(windowRenderer, t, nullptr, &textureDest);
  }
}

void draw(SDL_Renderer* windowRenderer) {
  SDL_SetRenderTarget(windowRenderer, NULL);
  SDL_SetRenderDrawColor(windowRenderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
  SDL_RenderClear(windowRenderer);

  SDL_Rect dest { 0, 0, 1024, 768 };
  SDL_SetTextureBlendMode(buffer, SDL_BLENDMODE_NONE);
  SDL_RenderSetClipRect(windowRenderer, &dest);
  SDL_RenderCopy(windowRenderer, buffer, nullptr, &dest);

  SDL_RenderPresent(windowRenderer);
}

void destroyTextures() {
  SDL_DestroyTexture(buffer);

  for (auto& t : textures) {
    SDL_DestroyTexture(t);
    t = nullptr;
  }
}
