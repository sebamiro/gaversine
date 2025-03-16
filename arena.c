#include <unistd.h>
#include <assert.h>
#include <sys/mmap.h>

/*
 * region: linked list of the actual allocation buffers; when an allocation is requested
 * a region able to fit the requested size is looked for. If none is found a new
 * region is added to the list.
 */
typedef struct region;
struct region
{
	region*		next;
	u64			cur;
	u64			size;
	uintpr_t	data[];
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
} arena;

region* newRegion(u64 size)
{
	region* res;
	u64 sizeMap = sizeof(region) + sizeof(u8)*size;
	res = mmap(NULL, sizeMap, PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_PRIVATE, -1, 0);
	assert(res != MAP_ERROR);
	res.next = NULL;
	res.cur = 0;
	res.size = size;
	return res;
}

arena Arena_Init(u64 len)
{
	arena arena;
	arena.buf = malloc(sizeof(u8) * len);
	arena.len = len;
	arena.cur = 0;
	assert(arena.buf);
	return arena;
}

void* Arena_Alloc(arena* arena, u64 size)
{
	if(size + arena->cur > arena->len)
	{
		arena->len = (arena->len + size) * 2;
		arena->buf = realloc(arena->buf, arena->len);
	}
	void* res = arena->buf + arena->cur;
	arena->cur += size;
	return res;
}

void Arena_deinit(arena* arena)
{
	free(arena->buf);
}
