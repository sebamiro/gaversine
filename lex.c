u8 IsWhitespace(char c)
{
	return c == Token_Space ||
		c == Token_Tab ||
		c == Token_Newline ||
		c == Token_CarrigeReturn;
}

tokens Lex(char* file, u64 size_File)
{
	TimeFunction_Start;
	const char* literalsStr[3] = { "true", "null", "false" };
	const type_token literalsType[3] = { Token_True, Token_Null, Token_False };

	tokens tokens;
	u64 iter_File, currentToken, allocSize;
	iter_File = 0;
	currentToken = 0;
	allocSize = 100;

	tokens.Len = 100;
	tokens.Type = malloc(sizeof(type_token) * allocSize);
	tokens.Start = malloc(sizeof(u64) * allocSize);

	while(iter_File < size_File)
	{
		if (currentToken >= allocSize)
		{
			allocSize *= 2;
			tokens.Type = realloc(tokens.Type, sizeof(type_token) * allocSize);
			tokens.Start = realloc(tokens.Start, sizeof(u64) * allocSize);
		}
		while (file[iter_File] != 0 && IsWhitespace(file[iter_File]))
		{
			++iter_File;
		}
		if (file[iter_File] == 0)
		{
			break;
		}
		if (file[iter_File] == '"')
		{
			++iter_File;
			tokens.Start[currentToken] = iter_File;
			tokens.Type[currentToken] = Token_String;
			++currentToken;
			while(file[iter_File] != 0 && file[iter_File] != '"')
			{
				++iter_File;
			}
			assert(file[iter_File] != 0);
			++iter_File;
		}
		else if (file[iter_File] == '-' || (file[iter_File] > 0x2F && file[iter_File] < 0x3A))
		{
			tokens.Start[currentToken] = iter_File;
			tokens.Type[currentToken] = Token_Number;
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
		}
		else if ((file[iter_File] > 0x60 && file[iter_File] < 0x7B) || (file[iter_File] > 0x40 && file[iter_File] < 0x5b))
		{
			for (u64 iterLit = 0; iterLit < ArrayCount(literalsStr); ++iterLit)
			{
				if (strncmp((char*)(file + iter_File), literalsStr[iterLit], strlen(literalsStr[iterLit]) == 0))
				{
					tokens.Start[currentToken] = iter_File;
					tokens.Type[currentToken] = literalsType[iterLit];
					iter_File += strlen(literalsStr[iterLit]);
					++currentToken;
					break;
				}
			}
		}
		else
		{
			switch (file[iter_File])
			{
				case Token_BeginObject:
				case Token_EndObject:
				case Token_BeginArray:
				case Token_EndArray:
				case Token_NameSeparator:
				case Token_ValueSeparator:
					tokens.Start[currentToken] = iter_File;
					tokens.Type[currentToken] = file[iter_File];
					++currentToken;
					break;
				default:
					printf("[%ld]=%c\n", iter_File, file[iter_File]);
					assert(0);
					break;
			}
			++iter_File;
		}
	}
	tokens.Len = currentToken;

	TimeFunction_End;
	return tokens;
}
