typedef struct Arena
{
	u8*	buf;
	u32 cur;
	u32 len;
} Arena;

Arena Arena_init(u32 len)
{
	Arena arena;
	arena.buf = malloc(sizeof(u8) * len);
	arena.len = len;
	arena.cur = 0;
	assert(arena.buf);
	return arena;
}

void* Arena_alloc(Arena* arena, u32 size)
{
	assert(size + arena->cur < arena->len);
	void* res = arena->buf + arena->cur;
	arena->cur += size;
	return res;
}

void Arena_deinit(Arena* arena)
{
	free(arena->buf);
}
