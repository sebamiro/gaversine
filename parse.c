typedef struct Scanner
{
	u32		cur;
	tokens	tokens;
	char*	buf;
} Scanner;

void Scanner_expect_type(Scanner* scanner, type_token expected)
{
	assert(expected == scanner->tokens.types[scanner->cur]);
}

type_token Scanner_peek_type(Scanner* scanner)
{
	return scanner->tokens.types[scanner->cur];
}

u32 Scanner_string_len(Scanner* scanner)
{
	assert(scanner->tokens.types[scanner->cur] == Token_String);
	u32 len = scanner->tokens.starts[scanner->cur];

	while (scanner->buf[len] != 0 && scanner->buf[len] != '"')
	{
		++len;
	}
	assert(scanner->buf[len] != 0);
	return len - scanner->tokens.starts[scanner->cur];
}

u32 Scanner_object_len(Scanner* scanner)
{
	u32 i = scanner->cur;
	u32 len = 0;
	u32 in_object = 0;
	u32 in_array = 0;

	while (i < scanner->tokens.len)
	{
		switch (scanner->tokens.types[i])
		{
			case Token_BeginObject:
				++in_object;
				break;
			case Token_EndObject:
				if (in_object == 0 && in_array == 0)
				{
					return i != scanner->cur ? len + 1 : len;
				}
				--in_object;
				break;
			case Token_BeginArray:
				++in_array;
				break;
			case Token_EndArray:
				--in_array;
				break;
			case Token_ValueSeparator:
				if (in_object == 0 && in_array == 0)
				{
					++len;
				}
				break;
			default:
				break;
		}
		++i;
	}
	assert(0);
	return 0;
}

u32 Scanner_array_len(Scanner* scanner)
{
	u32 i = scanner->cur;
	u32 len = 0;
	u32 in_object = 0;
	u32 in_array = 0;

	while (i < scanner->tokens.len)
	{
		switch (scanner->tokens.types[i])
		{
			case Token_BeginObject:
				++in_object;
				break;
			case Token_EndObject:
				--in_object;
				break;
			case Token_BeginArray:
				++in_array;
				break;
			case Token_EndArray:
				if (in_object == 0 && in_array == 0)
				{
					return len != 0 ? len + 1 : len;
				}
				--in_array;
				break;
			case Token_ValueSeparator:
				if (in_object == 0 && in_array == 0)
				{
					++len;
				}
				break;
			default:
				break;
		}
		++i;
	}
	assert(0);
	return 0;
}

JSON_number parse_JSON_number(Scanner* scanner)
{
	Scanner_expect_type(scanner, Token_Number);
	JSON_number number;

	number.typ = Number_float;
	number.num_float = strtod(scanner->buf + scanner->tokens.starts[scanner->cur], NULL);
	scanner->cur++;
	return number;
}

JSON_string parse_JSON_string(Arena* arena, Scanner* scanner)
{
	JSON_string string;

	string.len = Scanner_string_len(scanner);
	string.str = (char*)Arena_alloc(arena, sizeof(char) * string.len);
	strncpy(string.str, scanner->buf + scanner->tokens.starts[scanner->cur], string.len);
	scanner->cur++;
	return string;
}

Handle_JSONValue parse_JSON_value(Arena* arena, JSON* json, Scanner* scanner);
JSON_array	parse_JSON_array(Arena* arena, JSON* json, Scanner* scanner)
{
	Scanner_expect_type(scanner, Token_BeginArray);
	scanner->cur++;

	u32 current_element = 0;
	JSON_array	array;
	array.len = Scanner_array_len(scanner);
	if (array.len > 0)
	{
		array.values = (Handle_JSONValue*)malloc(sizeof(Handle_JSONValue) * array.len);
	}

	while (Scanner_peek_type(scanner) != Token_EndArray)
	{
		Handle_JSONValue val = parse_JSON_value(arena, json, scanner);
		array.values[current_element] = val;
		++current_element;
		if (Scanner_peek_type(scanner) == Token_ValueSeparator)
		{
			scanner->cur++;
		}
		else
		{
			Scanner_expect_type(scanner, Token_EndArray);
		}
	}
	scanner->cur++;
	return array;
}


JSON_object parse_JSON_object(Arena* arena, JSON* json, Scanner* scanner);
Handle_JSONValue parse_JSON_value(Arena* arena, JSON* json, Scanner* scanner)
{
	JSON_value val;
	u32 index_val = json->len++;
	if (json->len >= json->size)
	{
		json->size *= 2;
		json->values = realloc(json->values, sizeof(JSON_value) * json->size);
	}
	switch (Scanner_peek_type(scanner))
	{
		case Token_BeginObject:
			JSON_object o = parse_JSON_object(arena, json, scanner);
			val.typ = JSONValue_Object;
			val.object = o;
			break;
		case Token_BeginArray:
			JSON_array a = parse_JSON_array(arena, json, scanner);
			val.typ = JSONValue_Array;
			val.array = a;
			break;
		case Token_String:
			JSON_string s = parse_JSON_string(arena, scanner);
			val.typ = JSONValue_String;
			val.string = s;
			break;
		case Token_Number:
			JSON_number n = parse_JSON_number(scanner);
			val.typ = JSONValue_Number;
			val.number = n;
			break;
		case Token_True:
			val.typ = JSONValue_True;
			[[fallthrough]];
		case Token_False:
			val.typ = JSONValue_False;
			[[fallthrough]];
		case Token_Null:
			val.typ = JSONValue_Null;
			scanner->cur++;
			break;
		default:
			assert(0);
	}
	json->values[index_val] = val;
	return (Handle_JSONValue)(index_val);
}

JSON_object parse_JSON_object(Arena* arena, JSON* json, Scanner* scanner)
{
	Scanner_expect_type(scanner, Token_BeginObject);
	scanner->cur++;

	u32 current_member = 0;
	JSON_object object = {0};
	object.len = Scanner_object_len(scanner);
	if (object.len > 0)
	{
		void* buf = malloc(sizeof(Handle_JSONValue) * object.len + sizeof(Handle_JSONValue) * object.len);
		object.keys = (Handle_JSONValue*)buf;
		object.values = (Handle_JSONValue*)(object.keys + object.len);
	}

	while (Scanner_peek_type(scanner) != Token_EndObject)
	{
		Scanner_expect_type(scanner, Token_String);
		object.keys[current_member] = parse_JSON_value(arena, json, scanner);
		Scanner_expect_type(scanner, Token_NameSeparator);
		scanner->cur++;
		object.values[current_member] = parse_JSON_value(arena, json, scanner);
		++current_member;
		if (Scanner_peek_type(scanner) == Token_ValueSeparator)
		{
			scanner->cur++;
		}
		else
		{
			Scanner_expect_type(scanner, Token_EndObject);
		}
	}
	scanner->cur++;
	return object;
}

JSON parse(Arena* arena, char* buf, tokens tokens)
{
	Scanner scanner = {0};
	scanner.buf = buf;
	scanner.tokens = tokens;
	JSON json;
	json.size = 100;
	json.len = 0;
	json.values = malloc(json.size * sizeof(JSON_value));

	(void)parse_JSON_value(arena, &json, &scanner);
	return json;
}
