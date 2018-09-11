#ifdef __unix__
#include <sys/types.h>
#endif

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef bool b32;
typedef float r32;
typedef double r64;

typedef size_t memory_index;

#define ArrayCount(x) (sizeof((x))/(sizeof((x)[0])))
#define Assert(x) do{if(!(x)){ printf("******\nASSERT FAIL:: FILE: %s, LINE: %i, FUNCTION: %s.\n******\n", __FILE__, __LINE__, __FUNCTION__); *(int*)0=0;}}while(0)

#define global_variable static
#define internal static
#define local_persist static

#define InvalidCodePath Assert(!"InvalidCodePath");
#define InvalidDefaultCase default: {InvalidCodePath;} break

#define Kilobytes(Value) ((Value)*1024LL)
#define Megabytes(Value) (Kilobytes(Value)*1024LL)
#define Gigabytes(Value) (Megabytes(Value)*1024LL)
#define Terabytes(Value) (Gigabytes(Value)*1024LL)

#define ZeroBuffer(Instance, Count) ZeroSize(sizeof(Instance[0]) * Count, Instance)
#define ZeroStruct(Instance) ZeroSize(sizeof(Instance), &(Instance))
void ZeroSize(memory_index Size, void* Ptr)
{
	u8* Byte = (u8 *)Ptr;
	while(Size > 0)
	{
		*Byte = 0;
		++Byte;
		--Size;
	}
}

