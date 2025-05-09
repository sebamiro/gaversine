u8 IsWhitespace(char c)
{
	return c == Token_Space ||
		c == Token_Tab ||
		c == Token_Newline ||
		c == Token_CarrigeReturn;
}

tokens Lex(char* file, u32 size_File)
{
	TimeFunction_Start;
	const char* literalsStr[3] = { "true", "null", "false" };
	const type_token literalsType[3] = { Token_True, Token_Null, Token_False };

	u32 iter_File, currentToken, sizeAlloc;
	iter_File = 0;
	currentToken = 0;
	sizeAlloc = 4096;

	type_token* type = malloc(sizeAlloc * sizeof(type_token));
	u32*		start = malloc(sizeAlloc * sizeof(u32));
	while(iter_File < size_File)
	{
		if (currentToken >= sizeAlloc)
		{
			u32 sizeRealloc = sizeAlloc * 2;
			type = realloc(type, sizeof(type_token) * sizeRealloc);
			start = realloc(start, sizeof(u32) * sizeRealloc);
			sizeAlloc = sizeRealloc;
		}

		while (file[iter_File] != 0 && IsWhitespace(file[iter_File]))
		{
			++iter_File;
		}
		if (file[iter_File] == 0)
		{
			break;
		}

		switch (file[iter_File])
		{
			case Token_BeginObject:
			case Token_EndObject:
			case Token_BeginArray:
			case Token_EndArray:
			case Token_NameSeparator:
			case Token_ValueSeparator:
			{
				start[currentToken] = iter_File;
				type[currentToken] = file[iter_File];
				++currentToken;
				++iter_File;
			} break;

			case '"':
			{
				++iter_File;
				start[currentToken] = iter_File;
				type[currentToken] = Token_String;
				++currentToken;
				while(file[iter_File] != 0 && file[iter_File] != '"')
				{
					++iter_File;
				}
				assert(file[iter_File] != 0);
				++iter_File;
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
				start[currentToken] = iter_File;
				type[currentToken] = Token_Number;
				++currentToken;
				if (file[iter_File] == '-')
				{
					iter_File++;
				}
				while(file[iter_File] != 0 && ((file[iter_File] > 0x2F && file[iter_File] < 0x3A) || file[iter_File] == '.'))
				{
					++iter_File;
				}
				assert(file[iter_File] != 0);
			} break;

			case 'f':
			case 't':
			case 'n':
			{
				for (u32 iterLit = 0; iterLit < ArrayCount(literalsStr); ++iterLit)
				{
					if (strncmp((char*)(file + iter_File), literalsStr[iterLit], strlen(literalsStr[iterLit]) == 0))
					{
						start[currentToken] = iter_File;
						type[currentToken] = literalsType[iterLit];
						iter_File += strlen(literalsStr[iterLit]);
						++currentToken;
						break;
					}
				}
			} break;

			default:
			{
				printf("[%d]=%c\n", iter_File, file[iter_File]);
				assert(0);
			} break;
		}
	}

	tokens tokens;
	tokens.Len = currentToken;
	tokens.Type = type;
	tokens.Start = start;

	TimeFunction_End;
	return tokens;
}
