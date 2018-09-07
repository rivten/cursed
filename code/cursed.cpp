#include <iostream>

#ifdef _WIN32
#include <SDL.h>
#else
#include <SDL2/SDL.h>
#endif

#include "common.h"
#include "math.h"

#define MAX_S32 0x7FFFFFFF

#define SDL_CHECK(Call) {Assert(Call == 0);}

global_variable bool GlobalRunning = true;

struct tile
{
	v3 BackgroundColor;
	char C;
};

struct cursed_state
{
	tile* Buffer;
	u32 TileSize;
	v2i TileCount;
};

internal cursed_state
CursedInit(v2i TileCount, u32 TileSize)
{
	cursed_state Result = {};
	Result.TileSize = TileSize;
	Result.TileCount = TileCount;

	memory_index BufferSize = sizeof(tile) * TileCount.x * TileCount.y;
	Result.Buffer = (tile *)malloc(BufferSize);
	Assert(Result.Buffer);
	ZeroSize(BufferSize, Result.Buffer);

	return(Result);
}

internal void
CursedRender(cursed_state* State, SDL_Renderer* Renderer)
{
	u32 TileCount = State->TileCount.x * State->TileCount.y;
	for(u32 TileIndex = 0; TileIndex < TileCount; ++TileIndex)
	{
		tile* Tile = State->Buffer + TileIndex;
		if(Tile->C == 'a')
		{
			SDL_SetRenderDrawColor(Renderer, 255, 0, 0, 255);
			SDL_Rect Rect = {};
			Rect.x = State->TileSize * (TileIndex % State->TileCount.x);
			Rect.y = State->TileSize * (TileIndex / State->TileCount.x);
			Rect.w = State->TileSize;
			Rect.h = State->TileSize;

			SDL_RenderFillRect(Renderer, &Rect);
		}
	}
}

int main(int ArgumentCount, char** Arguments)
{
	SDL_CHECK(SDL_Init(SDL_INIT_VIDEO));

	u32 TileSize = 16;
	v2i TileCount = V2i(80, 50);

	v2i WindowSize = TileSize * TileCount;
	u32 WindowFlags = SDL_WINDOW_SHOWN;

	SDL_Window* Window =
		SDL_CreateWindow("cursed", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 
			WindowSize.x, WindowSize.y, WindowFlags);
	Assert(Window);

	u32 RendererFlags = SDL_RENDERER_ACCELERATED;
	SDL_Renderer* Renderer =
		SDL_CreateRenderer(Window, -1, RendererFlags);
	Assert(Renderer);

	s32 TestIndex = 0;

	cursed_state Cursed = CursedInit(TileCount, TileSize);
	Cursed.Buffer[TestIndex].C = 'a';

	u32 LastCounter = SDL_GetTicks();
	float TargetSecondsPerFrame = 1.0f / 60.0f;
	while(GlobalRunning)
	{
		SDL_Event Event = {};
		while(SDL_PollEvent(&Event))
		{
			switch(Event.type)
			{
				case SDL_QUIT:
					{
						GlobalRunning = false;
					} break;
				default:
					{
					} break;

			}
		}

		Cursed.Buffer[TestIndex].C = 0;
		++TestIndex;
		if(TestIndex >= Cursed.TileCount.x * Cursed.TileCount.y)
		{
			TestIndex = 0;
		}
		Cursed.Buffer[TestIndex].C = 'a';

		SDL_SetRenderDrawColor(Renderer, 112, 128, 144, 255);
		SDL_RenderClear(Renderer);

		CursedRender(&Cursed, Renderer);

		SDL_RenderPresent(Renderer);

		float WorkCounter = SDL_GetTicks();
		float WorkMSElapsed = (float)(WorkCounter - LastCounter);

		{
			char WindowNewTitleBuffer[128];
			s32 FPS = s32(WorkMSElapsed) == 0 ? MAX_S32 : s32(1000.0f / WorkMSElapsed);
			snprintf(WindowNewTitleBuffer, sizeof(WindowNewTitleBuffer),
					"cursed demo @ rivten - %dms - %i FPS",
					(int)(WorkMSElapsed), FPS);
			SDL_SetWindowTitle(Window, WindowNewTitleBuffer);
		}

		float SecondsElapsedForFrame = WorkMSElapsed / 1000.0f;
		if(SecondsElapsedForFrame < TargetSecondsPerFrame)
		{
			u32 SleepMS = (u32)(1000.0f * (TargetSecondsPerFrame - SecondsElapsedForFrame));
			if(SleepMS > 0)
			{
				SDL_Delay(SleepMS);
			}
		}
		float EndWorkCounter = SDL_GetTicks();
		LastCounter = EndWorkCounter;
	}

	SDL_Quit();

	return(0);
}
