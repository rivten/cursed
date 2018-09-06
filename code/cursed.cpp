#include <iostream>

#ifdef _WIN32
#include <SDL.h>
#else
#include <SDL2/SDL.h>
#endif

#include "common.h"

#define SDL_CHECK(Call) {Assert(Call == 0);}

int main(int ArgumentCount, char** Arguments)
{
	SDL_CHECK(SDL_Init(SDL_INIT_VIDEO));

	u32 WindowWidth = 200;
	u32 WindowHeight = 200;
	u32 WindowFlags = SDL_WINDOW_SHOWN;

	SDL_Window* Window =
		SDL_CreateWindow("cursed", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 
			WindowWidth, WindowHeight, WindowFlags);
	Assert(Window);

	u32 RendererFlags = SDL_RENDERER_ACCELERATED;
	SDL_Renderer* Renderer =
		SDL_CreateRenderer(Window, -1, RendererFlags);
	Assert(Renderer);

	SDL_SetRenderDrawColor(Renderer, 112, 128, 144, 255);
	SDL_RenderClear(Renderer);
	SDL_RenderPresent(Renderer);
	SDL_Delay(3000);

	SDL_Quit();

	return(0);
}
