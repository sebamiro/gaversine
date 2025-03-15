enum json_value_type
{
	JSONValue_False,
	JSONValue_Null,
	JSONValue_True,
	JSONValue_Object,
	JSONValue_Array,
	JSONValue_Number,
	JSONValue_String,
};

typedef struct json_string
{
	u32		Len;
	char*	Str;
} json_string;

enum type_number
{
	Number_int,
	Number_float,
};
typedef struct json_number
{
	enum type_number	Type;

	union
	{
		i64	Int;
		f64 Float;
	};
} json_number;

typedef u32 handle_json_value;
typedef struct json_value json_value;
typedef struct json_array
{
	u32					Len;
	handle_json_value*	Values;
} json_array;

typedef struct json_object
{
	u32					Len;
	handle_json_value*	Keys;
	handle_json_value*	Values;
} json_object;

struct json_value
{
	enum json_value_type	Type;
	union
	{
		json_string	String;
		json_number	Number;
		json_array	Array;
		json_object	Object;
	};
};

typedef struct json
{
	json_value* Values;
	u32			Len;
	u32			Size;
} json;
