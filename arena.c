typedef struct arena
{
	u8*	buf;
	u64 cur;
	u64 len;
} arena;

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
