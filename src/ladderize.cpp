#include "myutils.h"
#include "treen.h"

void cmd_ladderize()
	{
	vector<TreeN *> Trees;
	TreesFromFile(opt(ladderize), Trees);
	const uint TreeCount = SIZE(Trees);

	for (uint TreeIndex = 0; TreeIndex < TreeCount; ++TreeIndex)
		{
		TreeN &T = *Trees[TreeIndex];
		T.Ladderize(opt(right));
		}

	TreesToFile(Trees, opt(output));
	}
