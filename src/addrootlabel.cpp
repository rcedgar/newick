#include "myutils.h"
#include "treen.h"

void MakeOutputFileName(const string &Pattern, string &FN, uint n);

void cmd_addrootlabel()
	{
	const string &FileName = opt(addrootlabel);

	vector<TreeN *> Trees;
	TreesFromFile(FileName, Trees);
	const uint TreeCount = SIZE(Trees);

	string Pattern = "Tree.@";
	if (optset_pattern)
		Pattern = opt(pattern);

	FILE *f = CreateStdioFile(opt(output));
	uint UnrootedCount = 0;
	for (uint TreeIndex = 0; TreeIndex < TreeCount; ++TreeIndex)
		{
		TreeN &T = *Trees[TreeIndex];
		bool Rooted = false;
		bool Binary = T.IsBinary(Rooted);
		if (Rooted)
			++UnrootedCount;
		else
			{
			string RootLabel;
			if (optset_label)
				RootLabel = opt(label);
			else
				MakeOutputFileName(opt(pattern), RootLabel, TreeIndex+1);
			T.UpdateLabel(T.m_Root, RootLabel);
			}
		T.ToNewickFile(f);
		}
	if (UnrootedCount > 0)
		Warning("%u unrooted, not changed", UnrootedCount);
	CloseStdioFile(f);
	}
