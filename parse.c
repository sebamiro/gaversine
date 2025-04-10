json_number parseJsonNumber(chunk_buffer* buffer, u64* at)
{
	json_number number;
	number.Type = Number_int;

	f64 sign = 1;

	if (IsInChunk(buffer, at) && buffer->Data[*at] == '-')
	{
		sign = -1;
		++(*at);
	}

	f64 num = 0;
	while (IsInChunk(buffer, at))
	{
		u8 Char = buffer->Data[*at] - (u8)'0';
		if (Char < 10)
		{
			num = 10*num + Char;
			++(*at);
		}
		else
		{
			break;
		}
	}

	if (IsInChunk(buffer, at) && (buffer->Data[*at] == '.'))
	{
		++(*at);
		number.Type = Number_float;
		f64 c = 1.0 / 10.0;
		while (IsInChunk(buffer, at))
		{
			u8 Char = buffer->Data[*at] - (u8)'0';
			if (Char < 10)
			{
				num = num + Char*c;
				c *= 1.0 / 10.0;
				++(*at);
			}
			else
			{
				break;
			}
		}
	}

	num = sign * num;
	if (number.Type == Number_int)
	{
		number.Int = (i64)num;
	}
	else
	{
		number.Float = (f64)num;
	}
	return number;
}

json_string parseJsonString(arena* arena, chunk_buffer* buffer, u64* at)
{
	json_string string;

	if (buffer->Data[*at] != '"')
	{
		assert(0);
	}
	++(*at);

	u64 stringStart = *at;
	while (IsInChunk(buffer, at) && buffer->Data[*at] != '"')
	{
		++(*at);
	}
	assert(IsInChunk(buffer, at));

	string.Len = *at - stringStart;
	string.Str = (char*)Arena_Alloc(arena, sizeof(char) * string.Len);
	strncpy(string.Str, (const char*)buffer->Data + stringStart, string.Len);
	++(*at);
	return string;
}

json_token getJsonToken(chunk_buffer* buffer, u64* at);
handle_json_value parseJsonValue(
		arena* arena, json* json, chunk_buffer* buffer, u64* at);
json_value parseJsonList(
		arena* arena, json* json, chunk_buffer* buffer, u64* at, b32 hasLabel)
{
	(*at)++;

	json_value value = {0};
	value.Type = JSONValue_Array;

	u64 sizeList = 10;
	handle_json_value*	values = malloc(sizeList * sizeof(handle_json_value));
	value.Array.Values = values;

	handle_json_value*	keys;
	if (hasLabel)
	{
		keys = malloc(sizeList * sizeof(handle_json_value));

		value.Type = JSONValue_Object;
		value.Object.Keys = keys;
	}


	u32 currentMember = 0;
	json_token token = getJsonToken(buffer, at);
	while (token != Token_EndList)
	{
		if (currentMember >= sizeList)
		{
			sizeList *= 2;
			values = realloc(values, sizeList * sizeof(handle_json_value));
			if (hasLabel)
			{
				keys = realloc(keys, sizeList * sizeof(handle_json_value));
			}
		}

		if (hasLabel)
		{
			assert(token == Token_String);
			keys[currentMember] = parseJsonValue(arena, json, buffer, at);

			assert(getJsonToken(buffer, at) == Token_NameSeparator);
			(*at)++;
		}

		values[currentMember] = parseJsonValue(arena, json, buffer, at);

		token = getJsonToken(buffer, at);
		if (token == Token_ValueSeparator)
		{
			(*at)++;
			token = getJsonToken(buffer, at);
		}
		else
		{
			assert(token == Token_EndList);
			(*at)++;
		}
	}
	value.Array.Len = currentMember;


	return value;
}

inline u8 IsWhitespace(char c)
{
	return c == Token_Space ||
		c == Token_Tab ||
		c == Token_Newline ||
		c == Token_CarrigeReturn;
}

json_token getJsonToken(chunk_buffer* buffer, u64* at)
{
	json_token token = Token_None;


	while (IsInChunk(buffer, at) && IsWhitespace(buffer->Data[*at]))
	{
		++(*at);
	}

	switch (buffer->Data[*at])
	{
		case Token_BeginObject:
		case Token_BeginArray:
		{
			token = Token_BeginList;
		} break;


		case Token_EndObject:
		case Token_EndArray:
		{
			token = Token_EndList;
		} break;


		case Token_NameSeparator:
		case Token_ValueSeparator:
		{
			token = buffer->Data[*at];
		} break;

		case '"':
		{
			token = Token_String;
		} break;

		case '-':
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		{
			token = Token_Number;
		} break;

		case 'f':
		{
			token = Token_False;
		} break;
		case 't':
		{
			token = Token_True;
		} break;
		case 'n':
		{
			token = Token_Null;
		} break;

		default:
		{
			printf("[%ld], %s\n", *at, (char*)buffer->Data + *at);
			assert(0);
		} break;
	}
	return token;
}

inline enum json_value_type parseStringLiteral(
		chunk_buffer* buffer, u64* at, const char* strLit, enum json_value_type type)
{
	enum json_value_type res = JSONValue_None;
	u64 i = 0;
	while (IsInChunk(buffer, at) && strLit[i] == buffer->Data[*at])
	{
		i++; ++(*at);
	}
	if (!strLit[i])
	{
		res = type;
	}
	return res;
}

handle_json_value parseJsonValue(
		arena* arena, json* json, chunk_buffer* buffer, u64* at)
{
	json_value	val = {0};
	handle_json_value indexVal = (json->Len++);

	if (IsInChunk(buffer, at))
	{
		switch (getJsonToken(buffer, at))
		{
			case Token_BeginList:
			{
				val = parseJsonList(arena, json, buffer, at, buffer->Data[*at] == Token_BeginObject);
			} break;

			case Token_String:
			{
				json_string s = parseJsonString(arena, buffer, at);
				val.Type = JSONValue_String;
				val.String = s;
			} break;

			case Token_Number:
			{
				json_number n = parseJsonNumber(buffer, at);
				val.Type = JSONValue_Number;
				val.Number = n;
			} break;

			case Token_True:
			{
				val.Type = parseStringLiteral(buffer, at, "true", JSONValue_True);
			} break;
			case Token_False:
			{
				val.Type = parseStringLiteral(buffer, at, "false", JSONValue_False);
			} break;
			case Token_Null:
			{
				val.Type = parseStringLiteral(buffer, at, "null", JSONValue_Null);
			} break;
			default:
			{
			} break;
		}
	}
	if (json->Len >= json->Size)
	{
		json->Size *= 2;
		json->Values = realloc(json->Values, json->Size * sizeof(json_value));
	}
	json->Values[indexVal] = val;

	return indexVal;
}


typedef struct gaversine_pair
{
	f64 x0, y0;
	f64 x1, y1;
} gaversine_pair;

gaversine_pair* ParseGaversine(arena* arena, chunk_buffer* buffer)
{
	TimeFunction_Start;

	json json;
	json.Len = 0;
	json.Size = 4096;
	json.Values = malloc(json.Size * sizeof(json_value));

	u64 at = 0;
	(void)parseJsonValue(arena, &json, buffer, &at);

	gaversine_pair* pairs = NULL;
	TimeFunction_End;
	return pairs;
}
