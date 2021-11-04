#include "myutils.h"
#include "biparter.h"
#include "quarts.h"

double RofoPair(const TreeN &T1, const TreeN &T2);
void TreesFromFile(const string &FileName, vector<TreeN *> &Trees);

void cmd_rofos()
	{
	const string &FileName = opt(rofos);

	vector<TreeN *> Trees;
	TreesFromFile(FileName, Trees);

	const uint TreeCount = SIZE(Trees);
	ProgressLog("%u trees\n", TreeCount);
	uint PairCount = (TreeCount*(TreeCount - 1))/2;
	uint PairIndex = 0;
	vector<double> RFs;
	for (uint i = 0; i < TreeCount; ++i)
		{
		const TreeN &Ti = *Trees[i];
		for (uint j = i + 1; j < TreeCount; ++j)
			{
			ProgressStep(PairIndex++, PairCount, "Comparing");
			const TreeN &Tj = *Trees[j];
			double RF = RofoPair(Ti, Tj);
			RFs.push_back(RF);
			}
		}

	QuartsDouble Q;
	GetQuarts(RFs, Q);
	ProgressLog("Median RF %.3g\n", Q.Med);
	}
