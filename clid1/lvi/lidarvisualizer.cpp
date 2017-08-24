#include<iostream>
#include<string>
#include<sstream>
#include<vector>
#include<math.h>

#include<SDL2/SDL.h>
#include"rs232.h"

using namespace std;

//Screen dimension constants
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 640;

const int GRID_SIZE = 320;
const int TPMM = 1.0f; //tiles per mm

struct ivec2
{
    int x, y;
    ivec2()
    {

    }

    ivec2(int _x)
    {
        x = _x, y = _x;
    }
    ivec2(int _x, int _y)
    {
        x = _x, y = _y;
    }
    ivec2 operator*(int b)
    {
        return ivec2(x*b, y*b);
    }
    ivec2 operator*(float b)
    {
        return ivec2(x*b, y*b);
    }

    ivec2 operator+(ivec2 b)
    {
        return ivec2(x+b.x, y+b.y);
    }

    ivec2 operator+(int b)
    {
        return ivec2(x+b, y+b);
    }

    ivec2 operator+=(int b)
    {
        x+=b;
        y+=b;
        return ivec2(x, y);
    }
};

void init_SDL(SDL_Window* &window, SDL_Surface* &screenSurface, SDL_Renderer* &renderer)
{
	// Initialize SDL.
        if (SDL_Init(SDL_INIT_VIDEO) < 0)
        {
            cout << "error: "<< SDL_GetError() << endl;
            return;
        }

    // Creates the window where we will draw.
    window = SDL_CreateWindow("SDL_RenderClear",
                    SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                    SCREEN_WIDTH, SCREEN_HEIGHT,
                    0);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_SetRenderDrawBlendMode(renderer,SDL_BLENDMODE_BLEND);
}

void DrawTile(ivec2 pos, ivec2 grid_size, SDL_Renderer* renderer)
{
    /*if(pos.x < 0 || pos.x > grid_size.x || pos.y < 0 || pos.x > grid_size.y)
        return;*/


    ivec2 tile_middle = ivec2(
                              pos.x * (SCREEN_WIDTH / grid_size.x),
                              pos.y * (SCREEN_HEIGHT / grid_size.y)
                              );

    for(int x = 0; x < (SCREEN_WIDTH / grid_size.x); x++)
    {
        for(int y = 0; y < (SCREEN_HEIGHT / grid_size.y); y++)
        {
            SDL_RenderDrawPoint(renderer, x + tile_middle.x, y + tile_middle.y);
        }
    }
    SDL_RenderPresent(renderer);
}

void quit(SDL_Window* window)
{
    //Destroy window
	SDL_DestroyWindow( window );

	//Quit SDL subsystems
	SDL_Quit();
}

vector<ivec2> get_data(int comport_number)
{
    vector<ivec2> temp;
    unsigned char buf[4096];
    int read = RS232_PollComport(comport_number, buf, 4095);
    if(read>0)
    {
        buf[read] = 0;  //ending character
        stringstream datass(string((char*)buf));
        while(datass)
        {
            char d1, d2;
            int angle, distance;
            datass >> angle >> d1 >> distance >> d2;
            if(d1 != ',' || d2 != '.') //if data comes out of order
            {
                datass >> angle >> d1; //makes so it will be in the correct order the next time
                continue;
            }
            temp.push_back(ivec2(angle, distance));

        }

    }
    return temp;
}

int main(int argv, char** args)
{
    int comport_number;
    while(true)
    {
        int baudrate; string mode, comport_name;
        cout << "enter port number, baudrate and mode (\"8N1\", \"7E2\") separated by spaces" << endl;
        cin >> comport_name >> baudrate >> mode;
        comport_number = RS232_GetPortnr(comport_name.c_str());
        if(RS232_OpenComport(comport_number, baudrate, mode.c_str()) != 1)
            break;
        else
            cout << "unable to begin serial connectio, try again" << endl;
    }

    SDL_Window* window;
    SDL_Surface* screenSurface;
    SDL_Renderer* renderer;

    init_SDL(window, screenSurface, renderer);
    SDL_Texture *display = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, SCREEN_WIDTH, SCREEN_HEIGHT);

    SDL_SetRenderTarget(renderer, display);

    SDL_SetRenderDrawColor(renderer, 10, 13, 10, 255);
    SDL_RenderClear(renderer);

    SDL_Event Events;
    bool run = true;
    while(run)
    {
        while (SDL_PollEvent(&Events))
        {
            if (Events.type == SDL_QUIT)
                run = false;
        }

        vector<ivec2> coords = get_data(comport_number);
        for(auto coord : coords)
        {
            if(abs(coord.y - 360) < 5)
            {
                SDL_SetRenderTarget(renderer, display);
                SDL_SetRenderDrawColor(renderer, 10, 13, 10, 255);
                SDL_RenderClear(renderer);
            }
            //cout << coord.x << " " << coord.y << endl;
            ivec2 xycoord = ivec2(coord.x*sin(coord.y*M_PI/180.0f), coord.x*cos(coord.y*M_PI/180.0f));
            xycoord = xycoord*0.5f;
            xycoord += GRID_SIZE/2;
            //cout << xycoord.x << " " << xycoord.y << endl;
            SDL_SetRenderTarget(renderer, display);
            SDL_SetRenderDrawColor(renderer, 0, 200, 10, 255);
            DrawTile(xycoord, ivec2(GRID_SIZE), renderer);
        }
        SDL_SetRenderTarget(renderer, NULL);
        SDL_RenderCopy(renderer, display, NULL, NULL);
        SDL_RenderPresent(renderer);
        SDL_Delay(100);
    }

    quit(window);
}
