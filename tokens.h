typedef enum type_token
{
	Token_None = 0x0,

	// structural
	Token_BeginObject = 0x7B, // { begin-object
	Token_EndObject = 0x7D, // } end-object
	Token_BeginArray = 0x5B, // [ begin-array
	Token_EndArray = 0x5D, // ] end-array
	Token_NameSeparator = 0x3A, // : name-separator
	Token_ValueSeparator = 0x2C, // , value-separator

	// whitespace
	Token_Space = 0x20,
	Token_Tab = 0x09,
	Token_Newline = 0x0A,
	Token_CarrigeReturn = 0x0D,

	// Values
	Token_True,
	Token_False,
	Token_Null,
	Token_String,
	Token_Number,
} type_token;

typedef struct tokens
{
	type_token*	Type;
	u32*		Start;
	u32			Len;
} tokens;
