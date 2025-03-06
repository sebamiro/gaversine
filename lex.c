u8 is_whitespace(char c)
{
	return c == Token_Space ||
		c == Token_Tab ||
		c == Token_Newline ||
		c == Token_CarrigeReturn;
}

tokens lex(Arena* arena, char* file)
{
	u32 iter = 0;
	void* buf = Arena_alloc(arena, sizeof(type_token) * 100 + sizeof(u32) * 100);

	u32 current_token = 0;

	tokens tokens;
	tokens.len = 100;
	tokens.types = (type_token*)buf;
	tokens.starts = (u32*)(tokens.types + 100);

	while(file[iter] != 0)
	{
		while (file[iter] != 0 && is_whitespace(file[iter]))
		{
			++iter;
		}
		if (file[iter] == 0)
		{
			break;
		}
		if (file[iter] == '"')
		{
			++iter;
			tokens.starts[current_token] = iter;
			tokens.types[current_token] = Token_String;
			++current_token;
			while(file[iter] != 0 && file[iter] != '"')
			{
				++iter;
			}
			assert(file[iter] != 0);
			++iter;
		}
		else if (file[iter] > 0x2F && file[iter] < 0x3A)
		{
			tokens.starts[current_token] = iter;
			tokens.types[current_token] = Token_Number;
			++current_token;
			while(file[iter] != 0 && ((file[iter] > 0x2F && file[iter] < 0x3A) || file[iter] == '.'))
			{
				++iter;
			}
			assert(file[iter] != 0);
		}
		else if ((file[iter] > 0x60 && file[iter] < 0x7B) || (file[iter] > 0x40 && file[iter] < 0x5b))
		{
			if (strncmp((char*)(file + iter), "true", 4) == 0)
			{
				tokens.starts[current_token] = iter;
				tokens.types[current_token] = Token_True;
				iter += 4;
			}
			else if (strncmp((char*)(file + iter), "false", 5) == 0)
			{
				tokens.starts[current_token] = iter;
				tokens.types[current_token] = Token_False;
				iter += 5;
			}
			else if (strncmp((char*)(file + iter), "null", 4) == 0)
			{
				tokens.starts[current_token] = iter;
				tokens.types[current_token] = Token_Null;
				iter += 4;
			}
			else
			{
				assert(0);
			}
			++current_token;
		}
		else
		{
			switch (file[iter])
			{
				case Token_BeginObject:
				case Token_EndObject:
				case Token_BeginArray:
				case Token_EndArray:
				case Token_NameSeparator:
				case Token_ValueSeparator:
					tokens.starts[current_token] = iter;
					tokens.types[current_token] = file[iter];
					++current_token;
					break;
				default:
					printf("[%d]=%c\n", iter, file[iter]);
					assert(0);
					break;
			}
			++iter;
		}
	}
	return tokens;
}
