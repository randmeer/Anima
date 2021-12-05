#include <sstream>
#include <SDL.h>
#include <SDL_image.h>

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480

void set_pixel(SDL_Renderer *rend, int x, int y) {
    SDL_RenderDrawPoint(rend, x, y);
}

void draw_circle(SDL_Renderer *surface, int n_cx, int n_cy, int radius, Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    SDL_SetRenderDrawColor(surface, r,g,b,a);

    double error = (double)-radius;
    double x = (double)radius - 0.5;
    double y = (double)0.5;
    double cx = n_cx - 0.5;
    double cy = n_cy - 0.5;

    while (x >= y) {
        set_pixel(surface, (int)(cx + x), (int)(cy + y));
        set_pixel(surface, (int)(cx + y), (int)(cy + x));

        if (x != 0) {
            set_pixel(surface, (int)(cx - x), (int)(cy + y));
            set_pixel(surface, (int)(cx + y), (int)(cy - x));
        }

        if (y != 0) {
            set_pixel(surface, (int)(cx + x), (int)(cy - y));
            set_pixel(surface, (int)(cx - y), (int)(cy + x));
        }

        if (x != 0 && y != 0) {
            set_pixel(surface, (int)(cx - x), (int)(cy - y));
            set_pixel(surface, (int)(cx - y), (int)(cy - x));
        }

        error += y;
        ++y;
        error += y;

        if (error >= 0) {
            --x;
            error -= x;
            error -= x;
        }
    }
    SDL_SetRenderDrawColor(surface, 255,255,255,255);
}


void draw_filled_circle(SDL_Renderer *surface, int cx, int cy, int radius, Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    SDL_SetRenderDrawColor(surface, r,g,b,a);

    for (double dy = 1; dy <= radius; dy += 1.0) {
        double dx = floor(sqrt((2.0 * radius * dy) - (dy * dy)));
        SDL_RenderDrawLine(surface, cx - dx, cy + dy - radius, cx + dx, cy + dy - radius);
        SDL_RenderDrawLine(surface, cx - dx, cy - dy + radius, cx + dx, cy - dy + radius);
    }
    SDL_SetRenderDrawColor(surface, 255,255,255,255);
}

void draw_line(SDL_Renderer *surface, int x1, int y1, int x2, int y2, Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    SDL_SetRenderDrawColor(surface, r,g,b,a);
    SDL_RenderDrawLine(surface, x1, y1, x2, y2);
    SDL_SetRenderDrawColor(surface, 255,255,255,255);
}


void tick_delta(double& delta_time, Uint64& NOW, Uint64 LAST) {
    LAST = NOW;
    NOW = SDL_GetPerformanceCounter();
    delta_time = (double)((NOW - LAST)*1000 / (double)SDL_GetPerformanceFrequency());
}

void tick_fpslimit(double delta_time, int delay_std) {
    return;
    if (delta_time >= delay_std) SDL_Delay(0);
    else SDL_Delay(delay_std - delta_time);
}


int main(int argc, char ** argv)
{
    // variables

    bool quit = false;
    SDL_Event event;

    // init SDL

    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_JPG);

    // hints
    SDL_SetHint(SDL_HINT_RENDER_VSYNC, "1");
    SDL_SetHint (SDL_HINT_RENDER_SCALE_QUALITY, "2");

    // window and renderer
    SDL_Window * window = SDL_CreateWindow("Anima",SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    SDL_Renderer * renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);


    // TIME STUFF

    // fps
    #define FPS_INTERVAL 1.0
    Uint32 fps_lasttime = SDL_GetTicks();
    Uint32 fps_current;
    Uint32 fps_frames = 0;

    // fps limiter (currenly redundant because of vsync)
    //#define FPS 60
    //int delay_std = (1.0/FPS)*1000;
    //int delay_actual = delay_std;

    // delta time
    Uint64 NOW = SDL_GetPerformanceCounter();
    Uint64 LAST = 0;
    double delta_time = 0;

    // keystates
    const Uint8 *state = SDL_GetKeyboardState(NULL);

    SDL_Surface* image = IMG_Load("/Users/emil/coding/c++/Anima/amogus.jpg");
    if (image == NULL) {
        return 5;
    }

    #define SIZE_PLAYER 50
    SDL_Rect SrcR;
    SDL_Rect DestR;

    SrcR.x = 0;
    SrcR.y = 0;
    SrcR.w = image->w;
    SrcR.h = image->h;

    DestR.x = 640 / 2 - SIZE_PLAYER / 2;
    DestR.y = 480 / 2 - SIZE_PLAYER / 2;
    DestR.w = SIZE_PLAYER;
    DestR.h = SIZE_PLAYER;


    //SDL_FillRect(image, NULL, SDL_MapRGB(image->format, 255, 0, 0));

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, image);
    if (texture == NULL) {
        return 5;
    }
    SDL_FreeSurface(image);

    // mouse position
    int mouseX = 0;
    int mouseY = 0;


    // game loop
    while (!quit)
    {

        // clock
        //tick_fpslimit(delta_time, delay_std);
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


        // event handling
        switch (event.type)
        {
            case SDL_QUIT:
                quit = true;
                break;
            case SDL_KEYDOWN:
                break;
            case SDL_MOUSEBUTTONDOWN:
                switch (event.button.button)
                {
                    case SDL_BUTTON_LEFT:
                        SDL_ShowSimpleMessageBox(0, "Mouse", "Left button was pressed!", window);
                        break;
                    case SDL_BUTTON_RIGHT:
                        SDL_ShowSimpleMessageBox(0, "Mouse", "Right button was pressed!", window);
                        break;
                    default:
                        SDL_ShowSimpleMessageBox(0, "Mouse", "Some other button was pressed!", window);
                        break;
                }
                break;
            case SDL_MOUSEMOTION:
                //one way to get the mouse pos
                mouseX = event.motion.x;
                mouseY = event.motion.y;
                break;
        }

        // key states
        if (state[SDL_SCANCODE_A]) DestR.x -= 1;
        if (state[SDL_SCANCODE_D]) DestR.x += 1;
        if (state[SDL_SCANCODE_W]) DestR.y -= 1;
        if (state[SDL_SCANCODE_S]) DestR.y += 1;

        // another way to get the mouse pos
        SDL_GetMouseState(&mouseX, &mouseY);

        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, &SrcR, &DestR);

        // these lines have literally no purpose lol
        draw_line(renderer, 0, 0, mouseX, mouseY, 255, 0, 0, 0);
        draw_line(renderer, 0, WINDOW_HEIGHT, mouseX, mouseY, 255, 0, 0, 0);
        draw_line(renderer, WINDOW_WIDTH, 0, mouseX, mouseY, 255, 0, 0, 0);
        draw_line(renderer, WINDOW_WIDTH, WINDOW_HEIGHT, mouseX, mouseY, 255, 0, 0, 0);

        draw_circle(renderer, 100, 100, 50, 255, 0, 0, 255);
        draw_filled_circle(renderer, 200, 200, 20, 255, 0, 0, 255);
        SDL_RenderPresent(renderer);
    }

    // cleanup SDL

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();

    return 0;
}