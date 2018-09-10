#include <iostream>

#ifdef _WIN32
#include <SDL.h>
#else
#include <SDL2/SDL.h>
#endif

#include "common.h"
#include "math.h"

// NOTE(hugo): This should probably be the last
// include.
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#define MAX_S32 0x7FFFFFFF

#define STBTT_CHECK(Call) {Assert(Call != 0);}
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

// TODO(hugo):  This should not be in here
u32 GetFileSizeInBytes(FILE* f)
{
	// NOTE(hugo) : This resets the cursor at the beginning
	Assert(f);
	fseek(f, 0, SEEK_SET);
	fseek(f, 0, SEEK_END);

	// TODO(hugo) : Achtung ! Internet says this is not cross-platform
	// and should only be used with binary files. Why does it
	// work for me here ?
	u32 FileSize = ftell(f);

	fseek(f, 0, SEEK_SET);

	return(FileSize);
}

struct bitmap
{
	s32 Width;
	s32 Height;
	s32 Depth;
	s32 Pitch;
	u8* Data;
};

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

	FILE* FontFileHandle = fopen("../data/NotoMono-Regular.ttf", "rb");
	Assert(FontFileHandle);
	u32 FontFileSize = GetFileSizeInBytes(FontFileHandle);
	u8* FontDataBuffer = (u8*)malloc(FontFileSize + 1);
	fread(FontDataBuffer, sizeof(u8), FontFileSize, FontFileHandle);
	FontDataBuffer[FontFileSize] = 0;
	fclose(FontFileHandle);

	u32 CharCount = 256;
	stbtt_bakedchar* CharacterData = (stbtt_bakedchar *)malloc(CharCount * sizeof(stbtt_bakedchar));
	Assert(CharacterData);

	bitmap Bitmap = {};
	Bitmap.Width = 16 * TileSize;
	Bitmap.Height = 16 * TileSize;
	Bitmap.Depth = 8;
	Bitmap.Pitch = sizeof(u8) * Bitmap.Width;
	Bitmap.Data = (u8 *)malloc(sizeof(u8) * Bitmap.Width * Bitmap.Height);

	stbtt_BakeFontBitmap(FontDataBuffer, 0, float(TileSize), Bitmap.Data, 
			Bitmap.Width, Bitmap.Height, 0, CharCount, CharacterData);
	free(FontDataBuffer);

#if 0
	// NOTE(hugo): Debug: dumping all character data
	{
		for(u32 CharIndex = 0; CharIndex < 256; ++CharIndex)
		{
			stbtt_bakedchar* Data = CharacterData + CharIndex;
			printf("Char %i\nx0 = %u, y0 = %u\nx1 = %u, y1 = %u\nxoff = %f, yoff = %f, xadvance = %f\n",
					CharIndex, Data->x0, Data->y0, Data->x1, Data->y1, Data->xoff, Data->yoff, Data->xadvance);
		}
	}
#endif


	bitmap DisplayedBitmap = {};
	DisplayedBitmap.Width = Bitmap.Width;
	DisplayedBitmap.Height = Bitmap.Height;
	DisplayedBitmap.Depth = 32;
	DisplayedBitmap.Pitch = sizeof(u32) * DisplayedBitmap.Width;
	DisplayedBitmap.Data = (u8 *)malloc(sizeof(u32) * DisplayedBitmap.Width * DisplayedBitmap.Height);
	Assert(DisplayedBitmap.Data);

	u32* OnePastLastPixel = ((u32* )DisplayedBitmap.Data + (DisplayedBitmap.Width * DisplayedBitmap.Height));
	u32 PixelIndex = 0;
	for(u32* Pixel = (u32 *)DisplayedBitmap.Data; Pixel != OnePastLastPixel; ++Pixel, ++PixelIndex)
	{
		u8 R = 0x00;
		u8 G = 0x00;
		u8 B = 0x00;
		u8 A = Bitmap.Data[PixelIndex];
		*Pixel = (R << 0) | (G << 8) | (B << 16) | (A << 24);
	}
	free(Bitmap.Data);

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	SDL_Surface* Surface = SDL_CreateRGBSurfaceFrom(DisplayedBitmap.Data, DisplayedBitmap.Width, 
			DisplayedBitmap.Height, DisplayedBitmap.Depth, DisplayedBitmap.Pitch, 
			0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);
#else
	SDL_Surface* Surface = SDL_CreateRGBSurfaceFrom(DisplayedBitmap.Data, DisplayedBitmap.Width, 
			DisplayedBitmap.Height, DisplayedBitmap.Depth, DisplayedBitmap.Pitch, 
			0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
#endif
	Assert(Surface);

	SDL_Texture* Texture = SDL_CreateTextureFromSurface(Renderer, Surface);
	Assert(Texture);
	SDL_FreeSurface(Surface);
	free(DisplayedBitmap.Data);

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

		SDL_Rect DestRect = {};
		DestRect.x = 0;
		DestRect.y = 0;
		DestRect.w = DisplayedBitmap.Width;
		DestRect.h = DisplayedBitmap.Height;
		SDL_RenderCopy(Renderer, Texture, 0, &DestRect);

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
