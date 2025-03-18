#include <unistd.h>
#include <assert.h>
#include <sys/mman.h>

#ifndef Size_DefaultRegion
# define Size_DefaultRegion 8*1024
#endif

/*
 * region: linked list of the actual allocation buffers; when an allocation is requested
 * a region able to fit the requested size is looked for. If none is found a new
 * region is added to the list.
 */
typedef struct region region;
struct region
{
	region*		next;
	u64			cur;
	u64			size;
	uintptr_t	data[];
};

typedef struct arena
{
	region* begin;
	region* end;
} arena;

typedef struct arena_mark
{
	region* region;
	u64		cur;
} arena_mark;

region* newRegion(u64 size)
{
	region* res;
	u64 sizeMap = sizeof(region) + sizeof(uintptr_t)*size;
	res = mmap(NULL, sizeMap, PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_PRIVATE, -1, 0);
	assert(res != MAP_FAILED);
	res->next = NULL;
	res->cur = 0;
	res->size = size;
	return res;
}

void freeRegion(region* region)
{
	u64 sizeMap = sizeof(region) + sizeof(u8)*region->size;
	munmap(region, sizeMap);
}

void* Arena_Alloc(arena* arena, u64 sizeBytes)
{
	u64 sizeAlloc = (sizeBytes + sizeof(uintptr_t) - 1) / sizeof(uintptr_t);

	while(arena->end->cur + sizeAlloc > arena->end->size && arena->end->next != NULL)
	{
		arena->end = arena->end->next;
	}

	if (arena->end->cur + sizeAlloc > arena->end->size)
	{
		u64	sizeRegion = Size_DefaultRegion;
		if (sizeAlloc > sizeRegion)
		{
			sizeRegion = sizeAlloc;
		}
		arena->end->next = newRegion(sizeRegion);
		arena->end = arena->end->next;
	}

	void* res = &arena->end->data[arena->end->cur];
	arena->end->cur += sizeAlloc;
	return res;
}

void*	Arena_Realloc(arena* arena, void* oldPtr, u64 oldSize, u64 newSize)
{
	if (newSize <= oldSize)
	{
		return oldPtr;
	}
	void* newPtr = Arena_Alloc(arena, newSize);
	for (u64 i = 0; i < oldSize; ++i)
	{
		((u8*)newPtr)[i] = ((u8*)oldPtr)[i];
	}
	return newPtr;
}

arena Arena_Init(u64 len)
{
	arena arena;

	arena.begin = newRegion(len);
	arena.end = arena.begin;
	return arena;
}

void Arena_deinit(arena* arena)
{
	while(arena->begin)
	{
		region* t = arena->begin;
		arena->begin = t->next;
		freeRegion(t);
	}
}
