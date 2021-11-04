#include "myutils.h"
#include "treen.h"

// delete lengths & internal node labels leaving
// "topology-only" tree.
void cmd_topo()
	{
	const string &InputFileName = opt(topo);
	const string &OutputFileName = opt(output);
	FILE *fTsv = CreateStdioFile(opt(tsvout));

	vector<TreeN *> Trees;
	TreesFromFile(InputFileName, Trees);
	uint TreeCount = SIZE(Trees);

	for (uint TreeIndex = 0; TreeIndex < TreeCount; ++TreeIndex)
		{
		ProgressStep(TreeIndex, TreeCount, "Processing");
		TreeN &T = *Trees[TreeIndex];

		string BeforeStr;
		T.ToNewickStr(BeforeStr, false);
		BeforeStr[SIZE(BeforeStr)-1] = 0;

		T.CollapseUnary();
		T.Normalize();
		const uint NodeCount = T.GetNodeCount();
		for (uint Node = 0; Node < NodeCount; ++Node)
			{
			if (!T.IsLeaf(Node))
				T.UpdateLabel(Node, "");
			T.UpdateLength(Node, MISSING_LENGTH);
			}
		T.Ladderize(opt(right));

		string AfterStr;
		T.ToNewickStr(AfterStr, false);
		AfterStr[SIZE(AfterStr)-1] = 0;
		if (fTsv != 0)
			fprintf(fTsv, "%s\t%s\n", AfterStr.c_str(), BeforeStr.c_str());
		}

	TreesToFile(Trees, OutputFileName);
	CloseStdioFile(fTsv);
	}
