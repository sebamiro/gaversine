typedef int64_t		i64;
typedef uint64_t	u64;
typedef int32_t		i32;
typedef uint32_t	u32;
typedef uint8_t		u8;
typedef double		f64;
typedef float		f32;

#define ArrayCount(x) ((sizeof(x)/sizeof(0[x])) / ((u64)(!(sizeof(x) % sizeof(0[x])))))

