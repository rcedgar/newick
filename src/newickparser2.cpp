#include "myutils.h"
#include "newickparser2.h"

void NewickParser2::LogTokens() const
	{
	const uint N = SIZE(m_Tokens);
	Log("\n");
	Log("%u tokens\n", N);
	for (uint i = 0; i < N; ++i)
		Log("[%5u]  '%s'\n", i, m_Tokens[i].c_str());
	}

void NewickParser2::GetNextToken(string &s) const
	{
	s.clear();
	if (m_TokenIndex + 1 < SIZE(m_Tokens))
		s = m_Tokens[m_TokenIndex + 1];
	}

// note labels can be bootstrap values (float/int)
bool NewickParser2::IsValidLabel(const string &s) const
	{
#define check(x)	if (!(x)) return false;
	check(SIZE(s) > 0);
	check(s != ";");
	check(s != ":");
	check(s != ",");
	check(s != "(");
	check(s != ")");
	char c = s[0];
	check(c != '.');
	check(!isspace(c));
#undef check
	return true;
	}

void NewickParser2::LogContext() const
	{
	const uint TokenCount = SIZE(m_Tokens);
	uint FromTokenIndex = 0;
	if (m_TokenIndex > 8)
		FromTokenIndex = m_TokenIndex - 4;
	uint HiTokenIndex = m_TokenIndex + 4;
	if (HiTokenIndex > TokenCount)
		HiTokenIndex = TokenCount;
	for (uint i = FromTokenIndex; i < HiTokenIndex; ++i)
		{
		if (i == m_TokenIndex)
			Log(" << ");
		Log("%s", m_Tokens[i].c_str());
		if (i == m_TokenIndex)
			Log(" >> ");
		}
	Log("\n");
	}

void NewickParser2::LogState() const
	{
	Log("\n");
	Log("_______________________________________\n");
	Log("Token %u", m_TokenIndex);
	if (m_TokenIndex < SIZE(m_Tokens))
		Log("='%s'", m_Tokens[m_TokenIndex].c_str());
	else
		Log("=[end-of-data]");
	Log("\n");
	LogContext();
	const uint Depth = SIZE(m_Stack);
	const uint NodeCount = SIZE(m_Parents);
	asserta(SIZE(m_Labels) == NodeCount);
	asserta(SIZE(m_Lengths) == NodeCount);

	Log("Nodes(%u):\n", NodeCount);
	for (uint i = 0; i < NodeCount; ++i)
		{
		Log("[%5u]  Label='%s'", i, m_Labels[i].c_str());
		double Length = m_Lengths[i];
		if (Length != MISSING_LENGTH)
			Log(" length=%.3g", Length);
		uint Parent = m_Parents[i];
		if (Parent == UINT_MAX)
			Log(" parent=*");
		else
			Log(" parent=%u", m_Parents[i]);
		Log("\n");
		}

	Log("  m_Stack[Depth=%u]", Depth);
	for (uint i = 0; i < Depth; ++i)
		Log(" %u", m_Stack[i]);
	Log("\n");
	}

const string &NewickParser2::GetNextToken() const
	{
	if (m_TokenIndex+1 >= SIZE(m_Tokens))
		{
		static const string EmptyString = "";
		return EmptyString;
		}
	return m_Tokens[m_TokenIndex+1];
	}

void NewickParser2::GetLength(double &Length)
	{
	Length = MISSING_LENGTH;
	const string &PendingToken = GetNextToken();
	if (PendingToken != ":")
		return;

	++m_TokenIndex;
	if (m_TokenIndex >= SIZE(m_Tokens))
		{
		LogState();
		Die("Newick ends with ':'");
		}

	const string &PendingToken2 = GetNextToken();
	if (!IsValidFloatStr(PendingToken2))
		{
		LogState();
		Die("Expected length after ':', got '%s'", PendingToken2);
		}
	Length = StrToFloat(PendingToken2);
	++m_TokenIndex;
	}

void NewickParser2::GetLabelAndLength(string &Label, double &Length)
	{
	Label.clear();
	Length = MISSING_LENGTH;
	if (m_TokenIndex >= SIZE(m_Tokens))
		return;

	const string &PendingToken = GetNextToken();
	if (PendingToken == ":")
		{
		GetLength(Length);
		return;
		}

	if (!IsValidLabel(PendingToken))
		return;
	Label = PendingToken;
	++m_TokenIndex;
	GetLength(Length);
	}

void NewickParser2::PopStack()
	{
	if (m_Stack.empty())
		{
		LogState();
		Die("Empty Newick stack");
		}
	m_Stack.pop_back();
	}

void NewickParser2::FromTokens(const vector<string> &Tokens)
	{
	m_Tokens = Tokens;
	const uint TokenCount = SIZE(Tokens);

	if (Tokens.empty())
		Die("Empty Newick");

	m_Labels.clear();
	m_Parents.clear();
	m_Lengths.clear();
	m_Stack.clear();

	m_TokenIndex = 0;
	string Label;
	double Length;
	for (;;)
		{
		if (m_TokenIndex == TokenCount)
			{
			LogState();
			Die("Missing ';' in Newick");
			}

		asserta(m_TokenIndex < TokenCount);
		const string &Token = m_Tokens[m_TokenIndex];
		if (m_TraceParse)
			LogState();

		uint Depth = SIZE(m_Stack);
		if (Token == ";")
			{
			if (Depth != 0 || m_TokenIndex+1 != TokenCount)
				{
				LogState();
				Die("Unexpected ';' in Newick");
				}
			break;
			}
		else if (Token == "(")
			{
			uint Group = SIZE(m_Labels);

		// New group node
			uint Parent = (Depth == 0 ? UINT_MAX : m_Stack[Depth-1]);
			m_Parents.push_back(Parent);
			m_Labels.push_back("");
			m_Lengths.push_back(MISSING_LENGTH);

		// Push group
			m_Stack.push_back(Group);
			}
		else if (Token == ")")
			{
			if (Depth < 1)
				{
				LogState();
				Die("Unbalanced ')' in Newick");
				}
			uint Group = m_Stack[Depth-1];

			GetLabelAndLength(Label, Length);
			m_Labels[Group] = Label;
			m_Lengths[Group] = Length;

		// Pop group
			PopStack();
			}
		else if (Token == ",")
			;
		else if (Token == ":")
			{
			const string &Label = "";
			uint Child = SIZE(m_Labels);
			++m_TokenIndex;
			if (m_TokenIndex >= SIZE(m_Tokens))
				{
				LogState();
				Die("Newick ends with ':'");
				}
			const string &sLength = m_Tokens[m_TokenIndex];
			if (!IsValidFloatStr(sLength))
				{
				LogState();
				Die("Invalid length '%s'", sLength.c_str());
				}
			double Length = StrToFloat(sLength);
			uint Parent = (Depth == 0 ? UINT_MAX : m_Stack[Depth-1]);

			m_Parents.push_back(Parent);
			m_Labels.push_back(Label);
			m_Lengths.push_back(Length);
			}
		else if (IsValidLabel(Token))
			{
			if (m_TokenIndex > 0)
				{
				const string &PrevToken = m_Tokens[m_TokenIndex-1];
				if (PrevToken != "(" && PrevToken != ",")
					Die("Invalid Newick, '%s' followed by label '%s'",
					 PrevToken.c_str(), Token.c_str());
				}
		// New child node
			const string &Label = Token;
			uint Child = SIZE(m_Labels);
			GetLength(Length);
			uint Parent = (Depth == 0 ? UINT_MAX : m_Stack[Depth-1]);

			m_Parents.push_back(Parent);
			m_Labels.push_back(Label);
			m_Lengths.push_back(Length);
			}
		else
			{
			LogState();
			Die("Unexpected Newick token '%s'", Token.c_str());
			}
		++m_TokenIndex;
		}

	uint NodeCount = SIZE(m_Labels);
	asserta(SIZE(m_Parents) == NodeCount);
	asserta(SIZE(m_Lengths) == NodeCount);
	m_IsLeafs.clear();
	m_IsLeafs.resize(NodeCount, true);
	for (uint Node = 0; Node < NodeCount; ++Node)
		{
		uint Parent = m_Parents[Node];
		if (Parent != UINT_MAX)
			{
			asserta(Parent < NodeCount);
			m_IsLeafs[Parent] = false;
			}
		}
	m_Root = 0;

	Validate();
	}

void NewickParser2::FromFile(const string &FileName)
	{
	FILE *f = OpenStdioFile(FileName);
	FromFile(f);
	CloseStdioFile(f);
	}

void NewickParser2::FromFile(FILE *f)
	{
	uint DataBytes;
	const char *Data = (const char *) ReadAllStdioFile(f, DataBytes);
	FromData(Data, DataBytes);
	myfree((void *) Data);
	}

void NewickParser2::FromCStr(const char *CStr)
	{
	uint DataBytes = ustrlen(CStr);
	FromData(CStr, DataBytes);
	}

void NewickParser2::FromStr(const string &Str)
	{
	uint DataBytes = SIZE(Str);
	FromData(Str.c_str(), DataBytes);
	}

void NewickParser2::FromData(const char *Data, uint DataBytes)
	{
	m_Lexer.FromData(Data, DataBytes);
	FromTokens(m_Lexer.m_Tokens);
	}

static void Test1(NewickParser2 &P,
  const string &Str,
  const string &StrLabels,
  const string &StrLengths)
	{
	Log("______________________________________________________________\n");
	ProgressLog("\n");
	ProgressLog("  %s\n", Str.c_str());
	P.FromStr(Str);
	P.LogMe();
	}

static void cmd_test()
	{
	opt(test);
	NewickParser2 P;
//BUG
	Test1(P, "(A,B,(C,D));",				"A,B,C,D",		"");
	Test1(P, "(,,(,));",					"",				"");
	return;//@@@@@@

// My examples
	Test1(P, "(A, B, C);",					"A,B,C",		"");
	Test1(P, "(A, (B, C));",				"A,B,C",		"");
	Test1(P, "((A:2,B:3):4,C:5);",			"A,B,C",		"2,3,4,5");
	Test1(P, "((A:1,B:2):5,(C:3,D:4));",	"A,B,C,D",		"1,2,5,3,4");
	Test1(P, "((A:1,B:2):5,(C:3,D:4));",	"A,B,C,D",		"1,2,5,3,4");
	Test1(P, "((A,B),(C,D));",				"A,B,C,D",		"");
	Test1(P, "(A, B, C, D, E);",			"A,B,C,D,E",	"");
	Test1(P, "((:1,:2):5,(:3,:4));",		"",				"1,2,5,3,4");
	Test1(P, "(,(,));",						"",				"");
	Test1(P, "((,),(,));",					"",				"");
	Test1(P, "(,,);",						"",				"");
	Test1(P, "(A:1,B:2);",		"A,B",				"1,2");
	Test1(P, "(:1,B:2);",		"B",				"1,2");
	Test1(P, "(:1,:2);",		"",				"1,2");
	Test1(P, "(A:1,:2);",		"A",				"1,2");

// Wikipedia examples
	Test1(P, "(,,(,));",					"",				"");
	Test1(P, "(A,B,(C,D));",				"A,B,C,D",		"");
	Test1(P, "(A,B,(C,D)E)F;",				"A,B,C,D,E,F",	"");
	Test1(P, "(:0.1,:0.2,(:0.3,:0.4):0.5);",		"",		"0.1,0.2,0.3,0.4,0.5");
	Test1(P, "(:0.1,:0.2,(:0.3,:0.4):0.5):0.0;",	"",		"0.1,0.2,0.3,0.4,0.5,0.0");
	Test1(P, "(A:0.1,B:0.2,(C:0.3,D:0.4):0.5);",	"",		"0.1,0.2,0.3,0.4,0.5");
	Test1(P, "(A:0.1,B:0.2,(C:0.3,D:0.4)E:0.5)F;",	"",		"0.1,0.2,0.3,0.4,0.5");
	Test1(P, "((B:0.2,(C:0.3,D:0.4)E:0.5)F:0.1)A;",	"",		"0.1,0.2,0.3,0.4,0.5");

// Examples from https://evolution.genetics.washington.edu/phylip/newicktree.html (2021-03-28)
	Test1(P, "((raccoon:19.19959,bear:6.80041):0.84600,((sea_lion:11.99700, seal:12.00300):7.52973,((monkey:100.85930,cat:47.14069):20.59201, weasel:18.87953):2.09460):3.87382,dog:25.46154);",
	  "raccoon,bear,sea_lion,seal,monkey,cat,weasel,dog",
	  "19.19959,6.80041,0.84600,11.99700,12.00300,7.52973,100.85930,47.14069,20.59201, 18.87953,2.09460,3.87382,25.46154");

	Test1(P, "(Bovine:0.69395,(Gibbon:0.36079,(Orang:0.33636,(Gorilla:0.17147,(Chimp:0.19268, Human:0.11927):0.08386):0.06124):0.15057):0.54939,Mouse:1.21460):0.10;",
	  "Bovine,Gibbon,Orang,Gorilla,Chimp,Human,Mouse",
	  "0.69395,0.36079,0.33636,0.17147,0.19268,0.11927,0.08386,0.06124,0.15057,0.54939,1.21460,0.10");
 
	Test1(P, "(Bovine:0.69395,(Hylobates:0.36079,(Pongo:0.33636,(G._Gorilla:0.17147, (P._paniscus:0.19268,H._sapiens:0.11927):0.08386):0.06124):0.15057):0.54939, Rodent:1.21460);",
	  "Bovine,Hylobates,Pongo,G._Gorilla,P._paniscus,H._sapiens,Rodent",
	  "0.69395,0.36079,0.33636,0.17147,0.19268,0.11927,0.08386,0.06124,0.15057,0.54939,1.21460");

	Test1(P, "((A,B),(C,D));", "A,B,C,D", "");

	Test1(P, "(Alpha,Beta,Gamma,Delta,,Epsilon,,,);", "Alpha,Beta,Gamma,Delta,Epsilon", "");
	}
