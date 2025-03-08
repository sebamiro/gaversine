u8 is_whitespace(char c)
{
	return c == Token_Space ||
		c == Token_Tab ||
		c == Token_Newline ||
		c == Token_CarrigeReturn;
}

tokens lex(char* file)
{
	u32 iter = 0;

	u32 current_token = 0;

	tokens tokens;
	u32 alloc_size = 100;
	tokens.len = 100;
	tokens.types = malloc(sizeof(type_token) * alloc_size);
	tokens.starts = malloc(sizeof(u32) * alloc_size);

	while(file[iter] != 0)
	{
		if (current_token >= alloc_size)
		{
			alloc_size *= 2;
			tokens.types = realloc(tokens.types, sizeof(type_token) * alloc_size);
			tokens.starts = realloc(tokens.starts, sizeof(u32) * alloc_size);
		}
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
		else if (file[iter] == '-' || (file[iter] > 0x2F && file[iter] < 0x3A))
		{
			tokens.starts[current_token] = iter;
			tokens.types[current_token] = Token_Number;
			++current_token;
			if (file[iter] == '-')
			{
				iter++;
			}
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
	tokens.len = current_token;
	return tokens;
}
