#include "myutils.h"
#include "newicklexer.h"
#include "treen.h"
#include "tree2.h"

void TreesFromData(const char *Data, uint DataBytes,
  vector<TreeN *> &Trees)
	{
	Trees.clear();

	NewickLexer NL;
	NL.FromData(Data, DataBytes);
	vector<vector<string> > TokensVec;
	NL.SplitTokens(TokensVec);
	const uint TreeCount = SIZE(TokensVec);
	for (uint i = 0; i < TreeCount; ++i)
		{
		const vector<string> &Tokens = TokensVec[i];
		TreeN *T = new TreeN;
		T->FromTokens(Tokens);
		Trees.push_back(T);
		}
	}

void TreesFromFile(const string &FileName, vector<TreeN *> &Trees)
	{
	FILE *f = OpenStdioFile(FileName);
	uint32 FileSize;
	const byte *Data = ReadAllStdioFile(f, FileSize);
	CloseStdioFile(f);
	TreesFromData((const char *) Data, FileSize, Trees);
	}

void TreesFromFile2(const string &FileName, vector<Tree2 *> &Trees)
	{
	vector<TreeN *> TreesN;
	TreesFromFile(FileName, TreesN);
	const uint N = SIZE(TreesN);
	for (uint i = 0; i < N; ++i)
		{
		const TreeN *TN = TreesN[i];
		Tree2 *T2 = new Tree2;
		TN->ToTree2(*T2);
		Trees.push_back(T2);
		}
	}

void TreesToFile(const vector<TreeN *> &Trees, const string &FileName)
	{
	FILE *f = CreateStdioFile(FileName);
	const uint TreeCount = SIZE(Trees);
	for (uint TreeIndex = 0; TreeIndex < TreeCount; ++TreeIndex)
		{
		const TreeN &T = *Trees[TreeIndex];
		T.ToNewickFile(f, false);
		}
	CloseStdioFile(f);
	}
