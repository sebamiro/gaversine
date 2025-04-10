typedef struct chunk_buffer
{
	FILE*	in;
	u8*		Data;
	u64		size;
	u64		sizeChunk;
} chunk_buffer;

typedef struct buffer
{
	u8*	Data;
	u64	size;
} buffer;

b32 ReadChunk(chunk_buffer* source)
{
	b32 res = fread(source->Data, source->sizeChunk - 1, 1, source->in);
	source->Data[source->sizeChunk - 1] = 0;
	return res != 0;
}

static chunk_buffer AllocChunkBuffer(FILE* in, u64 sizeFile, u64 sizeChunk)
{
	chunk_buffer buffer;

	buffer.in = in;
	buffer.Data = mmap(NULL, sizeChunk, PROT_READ|PROT_WRITE,
			MAP_ANONYMOUS|MAP_PRIVATE|MAP_POPULATE, -1, 0);
	assert(buffer.Data != MAP_FAILED);
	buffer.size = sizeFile;
	buffer.sizeChunk = sizeChunk;
	ReadChunk(&buffer);
	return buffer;
}

static void DeallocChunkBuffer(chunk_buffer* buffer)
{
	fclose(buffer->in);
	munmap(buffer->Data, buffer->sizeChunk);
}

buffer GetChunkBuffer(chunk_buffer* source)
{
	buffer res = { .Data = source->Data, .size = source->sizeChunk };
	return res;
}


b32 IsInChunk(chunk_buffer* source, u64* at)
{
	b32 res = *at < source->sizeChunk - 1;
	if (!res)
	{
		TimeBandwidth_Start(Read, sizeFile);
		res = ReadChunk(source);
		TimeBandwidth_End(Read);
		*at = 0;
	}
	return res;
}

 b32 IsInBounds(buffer* source, u64 at)
{
	b32 res = at < source->size;
	return res;
}
