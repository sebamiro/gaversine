typedef struct Arena
{
	u8*	buf;
	u64 cur;
	u64 len;
} Arena;

Arena Arena_init(u64 len)
{
	Arena arena;
	arena.buf = malloc(sizeof(u8) * len);
	arena.len = len;
	arena.cur = 0;
	assert(arena.buf);
	return arena;
}

void* Arena_alloc(Arena* arena, u64 size)
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

void Arena_deinit(Arena* arena)
{
	free(arena->buf);
}
