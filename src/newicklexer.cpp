#include "myutils.h"
#include "newicklexer.h"
#include "newick_token.h"

char NewickLexer::GetCharFailOnEof()
	{
	int c = GetChar();
	if (c == EOF)
		Die("Parsing tree, unexpected end-of-file");
	return (char) c;
	}

int NewickLexer::GetChar()
	{
	if (m_DataPos >= m_DataBytes)
		return EOF;
	char Ch = m_Data[m_DataPos++];
	return Ch;
	}

void NewickLexer::SkipWhite()
	{
	for (;;)
		{
		int c = GetChar();
		if (c == EOF)
			return;
		if (!isspace(c))
			{
			asserta(m_DataPos > 0);
			--m_DataPos;
			return;
			}
		}
	}

bool NewickLexer::GetToken(string &Token)
	{
	Token.clear();
	if (m_DataPos >= m_DataBytes)
		return false;

// Skip leading white space
	SkipWhite();
	if (m_DataPos >= m_DataBytes)
		return false;

	char c = GetCharFailOnEof();

// In case a single-character token
	Token = c;

	uint uBytesCopied = 0;
	NEWICK_TOKEN_TYPE TT;
	switch (c)
		{
	case '(':
		Token = "(";
		return true;

	case ')':
		Token = ")";
		return true;

	case ':':
		Token = ":";
		return true;

	case ';':
		Token = ";";
		return true;

	case ',':
		Token = ",";
		return true;

	case '\'':
		TT = NTT_SingleQuotedString;
		c = GetCharFailOnEof();
		break;

	case '"':
		TT = NTT_DoubleQuotedString;
		c = GetCharFailOnEof();
		break;

	case '[':
		TT = NTT_Comment;
		break;

	default:
		TT = NTT_String;
		break;
		}

// Discard char already added
	Token.clear();
	for (;;)
		{
		if (TT != NTT_Comment)
			Token += c;
		int ic = GetChar();
		if (ic == EOF)
			Die("Unexpected end of Newick file in %s", NTTToStr(TT));
		c = (char) ic;

		switch (TT)
			{
		case NTT_String:
			if (0 != strchr("():;,", c))
				{
				asserta(m_DataPos > 0);
				--m_DataPos;
				return true;
				}
			if (isspace(c))
				return true;
			break;

		case NTT_SingleQuotedString:
			if ('\'' == c)
				return true;
			break;

		case NTT_DoubleQuotedString:
			if ('"' == c)
				return true;
			break;

		case NTT_Comment:
			if (']' == c)
				{
				bool Ok = GetToken(Token);
				return Ok;
				}
			break;

		default:
			Die("NewickParser::GetToken, invalid TT=%u", TT);
			}
		}
	}

void NewickLexer::LogTokens() const
	{
	const uint N = SIZE(m_Tokens);
	Log("\n");
	Log("%u tokens\n", N);
	for (uint i = 0; i < N; ++i)
		Log("[%5u]  '%s'\n", i, m_Tokens[i].c_str());
	}

void NewickLexer::FromData(const char *Data, uint DataBytes)
	{
	Clear();

	m_Data = Data;
	m_DataBytes = DataBytes;
	m_DataPos = 0;

	for (;;)
		{
		string Token;
		bool Ok = GetToken(Token);
		if (!Ok)
			break;
		m_Tokens.push_back(Token);
		}
	}

void NewickLexer::FromStr(const string &Str)
	{
	uint DataBytes = SIZE(Str);
	FromData(Str.c_str(), DataBytes);
	}

void NewickLexer::SplitTokens(vector<vector<string> > &TokenVec) const
	{
	TokenVec.clear();
	const uint N = SIZE(m_Tokens);
	vector<string> Split;
	for (uint i = 0; i < N; ++i)
		{
		const string &Token = m_Tokens[i];
		Split.push_back(Token);
		if (Token == ";")
			{
			TokenVec.push_back(Split);
			Split.clear();
			}
		}
	}
