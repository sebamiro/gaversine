typedef enum JSON_type
{
	JSONValue_False,
	JSONValue_Null,
	JSONValue_True,
	JSONValue_Object,
	JSONValue_Array,
	JSONValue_Number,
	JSONValue_String,
} JSON_type;

typedef struct JSON_string
{
	u32		len;
	char*	str;
} JSON_string;

typedef enum type_number
{
	Number_int,
	Number_float,
} type_number;
typedef struct JSON_number
{
	type_number	typ;
	union
	{
		i64 num_int;
		f64 num_float;
	};
} JSON_number;

typedef u32 Handle_JSONValue;
typedef struct JSON_value JSON_value;
typedef struct JSON_array
{
	u32					len;
	Handle_JSONValue*	values;
} JSON_array;

typedef struct JSON_object
{
	u32					len;
	Handle_JSONValue*	keys;
	Handle_JSONValue*	values;
} JSON_object;

struct JSON_value
{
	JSON_type	typ;
	union
	{
		JSON_string	string;
		JSON_number	number;
		JSON_array	array;
		JSON_object	object;
	};
};

typedef struct JSON
{
	JSON_value* values;
	u32			len;
	u32			size;
} JSON;
