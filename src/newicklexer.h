#pragma once

class NewickLexer
	{
public:
	const char *m_Data;
	uint m_DataPos;
	uint m_DataBytes;

	vector<string> m_Tokens;

public:
	void Clear()
		{
		m_Data = 0;
		m_DataPos = 0;
		m_DataBytes = 0;
		m_Tokens.clear();
		}

	void FromStr(const string &Str);
	void FromData(const char *Data, uint DataBytes);
	char GetCharFailOnEof();
	int GetChar();
	void SkipWhite();
	bool GetToken(string &Token);
	void LogTokens() const;
	void SplitTokens(vector<vector<string> > &TokensVec) const;
	};
