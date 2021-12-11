#include <sstream>
#include <cmath>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL2_rotozoom.h>
#include <SDL_ttf.h>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 450
#define BACKGROUND_COLOR 255, 255, 255, 255
#define LVL_W 18
#define LVL_H 10
#define SCALE 5
#define BLOCKSIZE 10

void tick_delta(double& delta_time, Uint64& NOW, Uint64 LAST) {
    LAST = NOW;
    NOW = SDL_GetPerformanceCounter();
    delta_time = (double)((NOW - LAST)*1000 / (double)SDL_GetPerformanceFrequency());
}

int randrange(int min, int max) {
    return min+(std::rand()%(max-min+1));
}

void generate_column(int column, int (&blocks)[LVL_W][LVL_H], int& min, int& max, int& min_fl, int& max_fl, int& flattener, int& column_height, int& last_column_height) {
    for (int i = 0; i < LVL_H; i++) {
        blocks[column][i] = 0;
    }
    if (flattener <= 0) {
        min = last_column_height - 1;
        max = last_column_height + 1;
        if (min < 0) min = 0;
        if (max > 6) max = 6;
        column_height = randrange(min, max);
        flattener = randrange(min_fl, max_fl);
    }
    else flattener--;
    for (int j = column_height; j >=0; j--) {
        blocks[column][j] = 1;
    }
    last_column_height = column_height;
}

void texture_column(int column, int (&blocks)[LVL_W][LVL_H]) {
    for (int j = LVL_H; j >= 0; j--) {
        if (blocks[column][j] > 0) {
            int x;
            int _1 = blocks[column - 1][j - 1];
            int _2 = blocks[column - 1][j];
            int _3 = blocks[column - 1][j + 1];
            int _4 = blocks[column][j + 1];
            int _5 = blocks[column + 1][j + 1];
            int _6 = blocks[column + 1][j];
            int _7 = blocks[column + 1][j - 1];
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
            blocks[column][j] = x;
        }
    }
}

void generate_blocks(int (&blocks)[LVL_W][LVL_H], int& min, int& max, int& min_fl, int& max_fl, int& flattener, int& column_height, int& last_column_height) {
    for (int i = 0; i < LVL_W; i++) {
        generate_column(i, blocks, min, max, min_fl, max_fl, flattener, column_height, last_column_height);
    }
    for (int i = 0; i < LVL_W; i++) {
        texture_column(i, blocks);
    }
}

void jump(bool &jumping, double &jumpvelocity, int& jumpmax, int terrainheight, int jumpheight) {
    if (jumping) return;
    jumping = true;
    jumpvelocity = 1;
    jumpmax = WINDOW_HEIGHT-(terrainheight*BLOCKSIZE*SCALE)-jumpheight-(2*BLOCKSIZE*SCALE);
}

void fall(bool &jumping, double &jumpvelocity) {
    if (jumping) return;
    jumping = true;
    jumpvelocity = 0;
}

void runup(bool &run_up, double &speed_multiplier, bool &slow) {
    run_up = true;
    speed_multiplier /= 1.1;
    slow = true;
}

SDL_Texture* get_player_frame(SDL_Texture* runframes[8], SDL_Texture* jumpframes[4], SDL_Texture* glideframes[2], int &frameindex, double frametimer, double &_frametimer, double deltatime, bool jumping, double speed_multiplier, double jumpvelocity, Uint8 gliding, Uint8 &glideframe) {
    if (jumping){
        if (jumpvelocity < 0 && gliding){
            glideframe = !glideframe;
            return glideframes[glideframe];
        }
        if (jumpvelocity > 0.5) return jumpframes[0];
        else if (jumpvelocity > 0) return jumpframes[1];
        else if (jumpvelocity > -0.5) return jumpframes[2];
        else return jumpframes[3];
    }
    if (_frametimer < 0) {
        if (frameindex >= 7) frameindex = 0;
        else frameindex++;
        _frametimer = frametimer;
    }
    else _frametimer -= (deltatime/1000)*speed_multiplier*2;
    return runframes[frameindex];
}

int main(){
    // event handling
    bool quit = false;
    SDL_Event event;

    // init SDL
    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_PNG);
    TTF_Init();

    // hints
    SDL_SetHint(SDL_HINT_RENDER_VSYNC, "1");
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "2");

    // window and renderer
    SDL_Window * window = SDL_CreateWindow("Anima",SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    SDL_Renderer * renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_SetRenderDrawColor(renderer, BACKGROUND_COLOR);

    // fonts
    // TTF_Font * font = TTF_OpenFont("../HelvetiPixel.ttf", 50);
    // TTF_CloseFont(font);
    // aparently this breaks the entire game, i still need to figure out why

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
    SDL_Rect _playerblit;
    playersrc.x = 0;
    playersrc.y = 0;
    playersrc.w = BLOCKSIZE*SCALE;
    playersrc.h = 2*BLOCKSIZE*SCALE;
    playerdest.x = 2*BLOCKSIZE*SCALE;
    playerdest.y = 6*BLOCKSIZE*SCALE;
    playerdest.w = BLOCKSIZE*SCALE;
    playerdest.h = 2*BLOCKSIZE*SCALE;
    _playerblit.x = 0;
    _playerblit.y = -2*BLOCKSIZE;
    _playerblit.w = BLOCKSIZE;
    _playerblit.h = 2*BLOCKSIZE;
    SDL_Surface* playersurf;
    SDL_Texture* playertex;
    double frametimer = 0.1; // time between frame in seconds
    double _frametimer = frametimer;
    int frameindex = 0;
    SDL_Texture *runframes[8];
    SDL_Texture *jumpframes[4];
    SDL_Texture *glideframes[2];
    for (int i = 0; i < 8; i++) {
        _playerblit.x = -BLOCKSIZE * i;
        _playerblit.y = -2*BLOCKSIZE;
        playersurf = SDL_CreateRGBSurface(0,10,20,32, 0xff, 0xff00, 0xff0000, 0xff000000);
        SDL_BlitSurface(textures, nullptr, playersurf, &_playerblit);
        playersurf = rotozoomSurface(playersurf, 0.0, 5, 0);
        playertex = SDL_CreateTextureFromSurface(renderer, playersurf);
        runframes[i] = playertex;
    }
    for (int i = 0; i < 4; i++) {
        _playerblit.x = -BLOCKSIZE * i;
        _playerblit.y = -4*BLOCKSIZE;
        playersurf = SDL_CreateRGBSurface(0,10,20,32, 0xff, 0xff00, 0xff0000, 0xff000000);
        SDL_BlitSurface(textures, nullptr, playersurf, &_playerblit);
        playersurf = rotozoomSurface(playersurf, 0.0, 5, 0);
        playertex = SDL_CreateTextureFromSurface(renderer, playersurf);
        jumpframes[i] = playertex;
    }
    for (int i = 0; i < 2; i++) {
        _playerblit.x = (-BLOCKSIZE * i) -BLOCKSIZE*8;
        _playerblit.y = -2*BLOCKSIZE;
        playersurf = SDL_CreateRGBSurface(0,10,20,32, 0xff, 0xff00, 0xff0000, 0xff000000);
        SDL_BlitSurface(textures, nullptr, playersurf, &_playerblit);
        playersurf = rotozoomSurface(playersurf, 0.0, 5, 0);
        playertex = SDL_CreateTextureFromSurface(renderer, playersurf);
        glideframes[i] = playertex;
    }
 SDL_FreeSurface(playersurf);

    // blocks
    SDL_Rect SrcR;
    SDL_Rect DestR;
    SrcR.x = 0;
    SrcR.y = 0;
    SrcR.w = BLOCKSIZE*SCALE;
    SrcR.h = BLOCKSIZE*SCALE;
    DestR.x = 0;
    DestR.y = 0;
    DestR.w = BLOCKSIZE*SCALE;
    DestR.h = BLOCKSIZE*SCALE;
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
        blocksurf = rotozoomSurface(blocksurf, 0.0, SCALE, 0);
        blocktex = SDL_CreateTextureFromSurface(renderer, blocksurf);
        blocktextures[i] = blocktex;
    }
    SDL_FreeSurface(blocksurf);

    // mouse stuff
    int mouseX = 0;
    int mouseY = 0;
    Uint32 mouse_buttons;

    // level generation stuff
    int min;
    int max;
    int min_fl = 3;
    int max_fl = 5;
    int last_column_height = 0;
    int column_height = 0;
    int blocks[LVL_W][LVL_H] = { 0 };
    int flattener = randrange(min_fl, max_fl);
    srand(time(NULL));
    generate_blocks(blocks, min, max, min_fl, max_fl, flattener, column_height, last_column_height);

    // movement
    int playerX = 0;
    double playerspeed = 0.5;
    bool jumping = false;
    double jumpvelocity = 0;
    int jumpheight = 75; // pixels
    int terrainheight = 2; // blocks
    int terrainheight0 = 2;
    int terrainheight1 = 2;
    // jumpmax has no use at the moment, but I'll need it later
    int jumpmax = 0; // pixels
    bool run_up = false;
    double speed_multiplier = 1;
    Uint8 glideframe = 0;

    // slow overlay
    bool slow = false;
    double slowtimer = 0.4; // seconds
    double slowtimer_max = 0.4;
    Uint8 slowcolor = 128;
    Uint8 slowcolor_max = 128;

    // game loop
    while (!quit) {

        // clock
        SDL_PollEvent(&event);
        tick_delta(delta_time, NOW, LAST);

        // fps calculation
        fps_frames++;
        if (fps_lasttime < SDL_GetTicks() - FPS_INTERVAL*1000) {
            fps_lasttime = SDL_GetTicks();
            fps_current = fps_frames;
            fps_frames = 0;
        }

        // fps display
        std::stringstream ss;
        ss <<" FPS: " << fps_current << "    Speed Multiplier: " << speed_multiplier;
        SDL_SetWindowTitle(window, ss.str().c_str());

        // clear buffer
        SDL_RenderClear(renderer);

        // event handling
        switch (event.type) {
            case SDL_QUIT:
                quit = true;
                break;
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym)
                {
                    case SDLK_DOWN: // stomping
                        playerdest.y = WINDOW_HEIGHT-(terrainheight*BLOCKSIZE*SCALE)-(2*BLOCKSIZE*SCALE);
                        jumping = false;
                        break;
                }
                break;
        }

        playerX += playerspeed * delta_time * speed_multiplier;
        // key states
        if (state[SDL_SCANCODE_SPACE]) jump(jumping, jumpvelocity, jumpmax, terrainheight, jumpheight);
        if (state[SDL_SCANCODE_LEFT]) if (jumping && jumpvelocity < 0) jumpvelocity /= 1.5; // gliding
        if (state[SDL_SCANCODE_L]) speed_multiplier += 0.05; // we do a little cheating

        // mouse states & position
        mouse_buttons = SDL_GetMouseState(&mouseX, &mouseY);
        if (mouse_buttons & SDL_BUTTON_LMASK) ;// do something

        // player has passed a block --> create new column + set new terrainheight
        if (playerX >= BLOCKSIZE*SCALE) {
            speed_multiplier += 0.001;
            run_up = false;
            for (int i = 1; i < LVL_W; i++) {
                for (int j = 0; j < LVL_H; j++) {
                    blocks[i-1][j] = blocks[i][j];
                }
            }
            // terrainheight --> column behind the player
            terrainheight0 = terrainheight1; // column the player just reached
            terrainheight1 = 0; // column in front of the player
            for (int i = 0; blocks[3][i] > 0; i ++) terrainheight1++;
            if (terrainheight0 > terrainheight1) {
                terrainheight0 -= 1;
                fall(jumping, jumpvelocity);
            }
            else if (terrainheight0 < terrainheight1 && !jumping) {
                runup(run_up, speed_multiplier, slow);
            }
            terrainheight = terrainheight0;
            generate_column(LVL_W-1, blocks, min, max, min_fl, max_fl, flattener, column_height, last_column_height);
            texture_column(LVL_W-2, blocks);
            playerX = 0;
        }

        // render blocks
        for (int i = 0; i < LVL_W; i++) {
            DestR.x = (i*BLOCKSIZE*SCALE) - playerX;
            for (int j = LVL_H; j >= 0; j--) {
                if (blocks[i][j] > 0) {
                    DestR.y = WINDOW_HEIGHT-((j+1)*BLOCKSIZE*SCALE);
                    SDL_RenderCopy(renderer, blocktextures[blocks[i][j]], &SrcR, &DestR);
                }
            }
        }

        // player
        if (run_up) {
            playerdest.y = WINDOW_HEIGHT - ((terrainheight1 - 1) * BLOCKSIZE * SCALE) - (2 * BLOCKSIZE * SCALE) - playerX;
        }
        else if (jumping) {
            // make the player run against the ramp if he touches it while he's jumping
            if (terrainheight0 < terrainheight1 && playerdest.y > WINDOW_HEIGHT - ((terrainheight1 - 1) * BLOCKSIZE * SCALE) - (2 * BLOCKSIZE * SCALE) - playerX) {
                runup(run_up, speed_multiplier, slow);
            }
            // stop jumping as the player touches the ground
            else if (playerdest.y > WINDOW_HEIGHT-(terrainheight*BLOCKSIZE*SCALE)-(2*BLOCKSIZE*SCALE)) {
                playerdest.y = WINDOW_HEIGHT-(terrainheight*BLOCKSIZE*SCALE)-(2*BLOCKSIZE*SCALE);
                jumping = false;
            }
            // the player is still in the air
            else {
                // 0.5s air time per jump, regardless of jump high
                jumpvelocity -= 4*(delta_time/1000); // factor 2 would be 1s air time
                playerdest.y += 10*(-jumpvelocity); // factor 10 because I say so (stop questioning my code)
            }
        }
        SDL_RenderCopy(renderer, get_player_frame(runframes, jumpframes, glideframes, frameindex, frametimer, _frametimer, delta_time, jumping, speed_multiplier, jumpvelocity, state[SDL_SCANCODE_LEFT], glideframe), &playersrc, &playerdest);

        // slow overlay
        if (slow) {
            if (slowtimer < 0){
                slow = false;
                slowtimer = slowtimer_max;
                slowcolor = slowcolor_max;
                SDL_SetRenderDrawColor(renderer, BACKGROUND_COLOR);
            }
            else{
                slowtimer -= (delta_time/1000);
                slowcolor = 255 - ((slowtimer * (double) slowcolor_max)/slowtimer_max);
                if (slowcolor < 128) slowcolor = 255;
                SDL_SetRenderDrawColor(renderer, slowcolor, slowcolor, slowcolor, 255);
            }
        }
        SDL_RenderPresent(renderer);
    }

    // cleanup SDL
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    TTF_Quit();
    SDL_Quit();

    return 0;
}
