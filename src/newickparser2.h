#pragma once

#include "newicklexer.h"
#include "newicktree.h"

class NewickParser2 : public NewickTree
	{
public:
	NewickLexer m_Lexer;
	vector<string> m_Tokens;
	vector<uint> m_Stack;
	uint m_TokenIndex = 0;
	bool m_TraceParse = false;

public:
	NewickParser2()
		{
		m_TokenIndex = 0;
		m_TraceParse = opt(trace_parse);
		}

	void Clear()
		{
		m_Lexer.Clear();
		m_Tokens.clear();

		m_Labels.clear();
		m_Parents.clear();
		m_Lengths.clear();
		m_IsLeafs.clear();

		m_TokenIndex = 0;
		m_TraceParse = false;
		}

	void FromFile(FILE *f);
	void FromFile(const string &FileName);
	void FromCStr(const char *CStr);
	void FromData(const char *Data, uint DataBytes);
	void FromStr(const string &Str);
	void FromTokens(const vector<string> &Tokens);
	void LogTokens() const;
	void GetNextToken(string &s) const;
	bool IsValidLabel(const string &s) const;
	void LogContext() const;
	void LogState() const;
	void GetLabelAndLength(string &Label, double &Length);
	void GetLength(double &Length);
	void PopStack();
	const string &GetNextToken() const;
	uint FixFT();
	};
