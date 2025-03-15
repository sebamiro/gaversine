typedef struct scanner
{
	u32		cur;
	tokens	tokens;
	char*	buf;
} scanner;

static void Scanner_ExpectType(scanner* scanner, type_token expected)
{
	assert(expected == scanner->tokens.Type[scanner->cur]);
}

type_token Scanner_PeekType(scanner* scanner)
{
	return scanner->tokens.Type[scanner->cur];
}

u32 Scanner_GetStringLen(scanner* scanner)
{
	Scanner_ExpectType(scanner, Token_String);
	u32 len = scanner->tokens.Start[scanner->cur];

	while (scanner->buf[len] != 0 && scanner->buf[len] != '"')
	{
		++len;
	}
	assert(scanner->buf[len] != 0);
	return len - scanner->tokens.Start[scanner->cur];
}

u32 Scanner_GetObjectLen(scanner* scanner)
{
	u32 i = scanner->cur;
	u32 Len = 0;
	u32 inObject = 0;
	u32 inArray = 0;

	while (i < scanner->tokens.Len)
	{
		switch (scanner->tokens.Type[i])
		{
			case Token_BeginObject:
				++inObject;
				break;
			case Token_EndObject:
				if (inObject == 0 && inArray == 0)
				{
					return i != scanner->cur ? Len + 1 : Len;
				}
				--inObject;
				break;
			case Token_BeginArray:
				++inArray;
				break;
			case Token_EndArray:
				--inArray;
				break;
			case Token_ValueSeparator:
				if (inObject == 0 && inArray == 0)
				{
					++Len;
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

u32 Scanner_array_len(scanner* scanner)
{
	u32 i = scanner->cur;
	u32 Len = 0;
	u32 inObject = 0;
	u32 inArray = 0;

	while (i < scanner->tokens.Len)
	{
		switch (scanner->tokens.Type[i])
		{
			case Token_BeginObject:
				++inObject;
				break;
			case Token_EndObject:
				--inObject;
				break;
			case Token_BeginArray:
				++inArray;
				break;
			case Token_EndArray:
				if (inObject == 0 && inArray == 0)
				{
					return Len != 0 ? Len + 1 : Len;
				}
				--inArray;
				break;
			case Token_ValueSeparator:
				if (inObject == 0 && inArray == 0)
				{
					++Len;
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

json_number ParseJsonNumber(scanner* scanner)
{
	Scanner_ExpectType(scanner, Token_Number);
	json_number number;

	number.Type = Number_float;
	number.Float = strtod(scanner->buf + scanner->tokens.Start[scanner->cur], NULL);
	scanner->cur++;
	return number;
}

json_string ParseJsonString(arena* arena, scanner* scanner)
{
	json_string string;

	string.Len = Scanner_GetStringLen(scanner);
	string.Str = (char*)Arena_Alloc(arena, sizeof(char) * string.Len);
	strncpy(string.Str, scanner->buf + scanner->tokens.Start[scanner->cur], string.Len);
	scanner->cur++;
	return string;
}

handle_json_value ParseJsonValue(arena* arena, json* json, scanner* scanner);
json_array	ParseJsonArray(arena* arena, json* json, scanner* scanner)
{
	Scanner_ExpectType(scanner, Token_BeginArray);
	scanner->cur++;

	u32 currentElement = 0;
	json_array	array;
	array.Len = Scanner_array_len(scanner);
	if (array.Len > 0)
	{
		array.Values = (handle_json_value*)malloc(sizeof(handle_json_value) * array.Len);
	}

	while (Scanner_PeekType(scanner) != Token_EndArray)
	{
		handle_json_value val = ParseJsonValue(arena, json, scanner);
		array.Values[currentElement] = val;
		++currentElement;
		if (Scanner_PeekType(scanner) == Token_ValueSeparator)
		{
			scanner->cur++;
		}
		else
		{
			Scanner_ExpectType(scanner, Token_EndArray);
		}
	}
	scanner->cur++;
	return array;
}


json_object ParseJsonObject(arena* arena, json* json, scanner* scanner)
{
	Scanner_ExpectType(scanner, Token_BeginObject);
	scanner->cur++;

	u32 currentMember = 0;
	json_object object = {0};
	object.Len = Scanner_GetObjectLen(scanner);
	if (object.Len > 0)
	{
		void* buf = malloc(sizeof(handle_json_value) * object.Len + sizeof(handle_json_value) * object.Len);
		object.Keys = (handle_json_value*)buf;
		object.Values = (handle_json_value*)(object.Keys + object.Len);
	}

	while (Scanner_PeekType(scanner) != Token_EndObject)
	{
		Scanner_ExpectType(scanner, Token_String);
		object.Keys[currentMember] = ParseJsonValue(arena, json, scanner);
		Scanner_ExpectType(scanner, Token_NameSeparator);
		scanner->cur++;
		object.Values[currentMember] = ParseJsonValue(arena, json, scanner);
		++currentMember;
		if (Scanner_PeekType(scanner) == Token_ValueSeparator)
		{
			scanner->cur++;
		}
		else
		{
			Scanner_ExpectType(scanner, Token_EndObject);
		}
	}
	scanner->cur++;
	return object;
}

handle_json_value ParseJsonValue(arena* arena, json* json, scanner* scanner)
{
	json_value val;
	u32 index_Val = json->Len++;
	if (json->Len >= json->Size)
	{
		json->Size *= 2;
		json->Values = realloc(json->Values, sizeof(json_value) * json->Size);
	}
	switch (Scanner_PeekType(scanner))
	{
		case Token_BeginObject:
		{
			json_object o = ParseJsonObject(arena, json, scanner);
			val.Type = JSONValue_Object;
			val.Object = o;
			break;
		}
		case Token_BeginArray:
		{
			json_array a = ParseJsonArray(arena, json, scanner);
			val.Type = JSONValue_Array;
			val.Array = a;
			break;
		}
		case Token_String:
		{
			json_string s = ParseJsonString(arena, scanner);
			val.Type = JSONValue_String;
			val.String = s;
			break;
		}
		case Token_Number:
		{
			json_number n = ParseJsonNumber(scanner);
			val.Type = JSONValue_Number;
			val.Number = n;
			break;
		}
		case Token_True:
			val.Type = JSONValue_True;
			[[fallthrough]];
		case Token_False:
			val.Type = JSONValue_False;
			[[fallthrough]];
		case Token_Null:
			val.Type = JSONValue_Null;
			scanner->cur++;
			break;
		default:
			assert(0);
	}
	json->Values[index_Val] = val;

	return (handle_json_value)(index_Val);
}


json Parse(arena* arena, char* buf, tokens tokens)
{
	TimeFunction_Start;

	scanner scanner = {0};
	scanner.buf = buf;
	scanner.tokens = tokens;
	json json;
	json.Len = 0;
	json.Size = 100;
	json.Values = malloc(json.Size * sizeof(json_value));

	(void)ParseJsonValue(arena, &json, &scanner);

	TimeFunction_End;
	return json;
}
