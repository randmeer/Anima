#include <sstream>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL2_rotozoom.h>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 450
#define BACKGROUND_COLOR 255, 255, 255, 255

void tick_delta(double& delta_time, Uint64& NOW, Uint64 LAST) {
    LAST = NOW;
    NOW = SDL_GetPerformanceCounter();
    delta_time = (double)((NOW - LAST)*1000 / (double)SDL_GetPerformanceFrequency());
}

int randrange(int min, int max) {
    return min+(std::rand()%(max-min+1));
}

int main(int argc, char ** argv)
{
    // event handling
    bool quit = false;
    SDL_Event event;

    // init SDL
    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_PNG);

    // hints
    SDL_SetHint(SDL_HINT_RENDER_VSYNC, "1");
    SDL_SetHint (SDL_HINT_RENDER_SCALE_QUALITY, "2");

    // window and renderer
    SDL_Window * window = SDL_CreateWindow("Anima",SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    SDL_Renderer * renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    // fps
    #define FPS_INTERVAL 1.0
    Uint32 fps_lasttime = SDL_GetTicks();
    Uint32 fps_current;
    Uint32 fps_frames = 0;

    // delta time
    Uint64 NOW = SDL_GetPerformanceCounter();
    Uint64 LAST = 0;
    double delta_time = 0;

    // keystates
    const Uint8 *state = SDL_GetKeyboardState(NULL);

    // all textures
    SDL_Surface* textures = IMG_Load("../textures.png");

    // player
    SDL_Rect playersrc;
    SDL_Rect playerdest;
    playersrc.x = 0;
    playersrc.y = 0;
    playersrc.w = 10;
    playersrc.h = 20;
    playerdest.x = 0;
    playerdest.y = 0;
    playerdest.w = 50;
    playerdest.h = 100;
    SDL_Surface* playersurf = SDL_CreateRGBSurface(0,10,20,32,0,0,0,0);
    SDL_Texture* playertex = SDL_CreateTextureFromSurface(renderer, playersurf);
    SDL_FreeSurface(playersurf);

    // blocks
    #define BLOCKSIZE 10
    #define BLOCKSCALE 5
    SDL_Rect SrcR;
    SDL_Rect DestR;
    SrcR.x = 0;
    SrcR.y = 0;
    SrcR.w = BLOCKSIZE*BLOCKSCALE;
    SrcR.h = BLOCKSIZE*BLOCKSCALE;
    DestR.x = 0;
    DestR.y = 0;
    DestR.w = BLOCKSIZE*BLOCKSCALE;
    DestR.h = BLOCKSIZE*BLOCKSCALE;
    SDL_Texture *blocktextures[20];
    SDL_Rect BlockDest;
    BlockDest.x = 0;
    BlockDest.y = 0;
    SDL_Surface *blocksurf;
    SDL_Texture *blocktex;
    for (int i = 0; i < 20; i++) {
        blocksurf = SDL_CreateRGBSurface(0,BLOCKSIZE,BLOCKSIZE,32, 0xff, 0xff00, 0xff0000, 0xff000000);
        BlockDest.x = -BLOCKSIZE * (i % 10);
        BlockDest.y = -BLOCKSIZE * ((i/10));
        SDL_BlitSurface(textures, nullptr, blocksurf, &BlockDest);
        blocksurf = rotozoomSurface(blocksurf, 0.0, BLOCKSCALE, 0);
        blocktex = SDL_CreateTextureFromSurface(renderer, blocksurf);
        blocktextures[i] = blocktex;
    }
    SDL_FreeSurface(blocksurf);

    // mouse stuff
    int mouseX = 0;
    int mouseY = 0;
    Uint32 mouse_buttons;

    // level generation stuff
    #define LVL_W 16
    #define LVL_H 10
    int min = 0;
    int max = 7;
    int min_fl = 2;
    int max_fl = 3;
    int last_column_height = 0;
    int column_height;
    int blocks[LVL_W][LVL_H] = { 0 };
    int flattener = randrange(min_fl, max_fl);
    srand( time(NULL) );
    for (int i = 0; i < LVL_W; i++) {
        if (flattener < 0) {
            min = last_column_height - 1;
            max = last_column_height + 1;
            if (min < 0) min = 0;
            if (max > 7) max = 7;
            column_height = randrange(min, max);
            flattener = randrange(min_fl, max_fl);
        }
        else flattener--;
        for (int j = column_height; j >=0; j--) {
            blocks[i][j] = 1;
        }
        last_column_height = column_height;
    }

    // game loop
    while (!quit)
    {

        // clock
        SDL_PollEvent(&event);
        tick_delta(delta_time, NOW, LAST);

        // fps calculation
        fps_frames++;
        if (fps_lasttime < SDL_GetTicks() - FPS_INTERVAL*1000)
        {
            fps_lasttime = SDL_GetTicks();
            fps_current = fps_frames;
            fps_frames = 0;
        }

        // FPS-counter in the window title!
        std::stringstream ss;
        //ss << "delay_std: " << delay_std << " delay_actual: " << delay_actual << " deltatime: " << delta_time << " FPS: " << fps_current;
        ss <<" FPS: " << fps_current;
        SDL_SetWindowTitle(window, ss.str().c_str());

        // clear buffer
        SDL_SetRenderDrawColor(renderer, BACKGROUND_COLOR);
        SDL_RenderClear(renderer);


        // event handling
        switch (event.type)
        {
            case SDL_QUIT:
                quit = true;
                break;
        }

        // key states
        if (state[SDL_SCANCODE_SPACE]) playerdest.y -= 1;

        // mouse states & position
        mouse_buttons = SDL_GetMouseState(&mouseX, &mouseY);
        if (mouse_buttons & SDL_BUTTON_LMASK) ;// do something

        // level generation
        // this works just because window size is hardcoded...
        for (int i = 0; i < LVL_W; i++) {
            DestR.x = i*BLOCKSIZE*BLOCKSCALE;
            for (int j = LVL_H; j >= 0; j--) {
                if (blocks[i][j]) {
                    int x;
                    int _1 = blocks[i-1][j-1];
                    int _2 = blocks[i-1][j];
                    int _3 = blocks[i-1][j+1];
                    int _4 = blocks[i][j+1];
                    int _5 = blocks[i+1][j+1];
                    int _6 = blocks[i+1][j];
                    int _7 = blocks[i+1][j-1];
                    if (_3 && _4 && _5) x = 17;
                    else if (_2 && _3 && _4 && _6) x = 16;
                    else if (_2 && _4 && _5 && _6) x = 15;
                    else if (_2 && _3 && _6) x = 11;
                    else if (_2 && _5 && _6) x = 11;
                    else if (_2 && _3 && _7 && !_4 && !_5 && !_6) x = 14;
                    else if (_1 && _5 && _6 && !_2 && !_3 && !_4) x = 13;
                    else if (_2 && _6) x = 11;
                    else if (_2 && _7) x = 14;
                    else if (_1 && _6) x = 13;
                    else x = 0;
                    DestR.y = WINDOW_HEIGHT-((j+1)*BLOCKSIZE*BLOCKSCALE);
                    SDL_RenderCopy(renderer, blocktextures[x], &SrcR, &DestR);
                }
            }
        }

        // player
        SDL_RenderCopy(renderer, playertex, &playersrc, &playerdest);

        SDL_RenderPresent(renderer);
    }

    // cleanup SDL
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();

    return 0;
}