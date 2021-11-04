#include "myutils.h"
#include "biparter.h"

double RofoPair(const TreeN &T1, const TreeN &T2)
	{
	uint LeafCount = T1.GetLeafCount();

	if (T2.GetLeafCount() != LeafCount)
		Die("Different leaf counts");

	BiParter BP1;
	BiParter BP2;

	BP1.Init(T1);
	BP2.Init(T2);

#if TRACE
	Log("_____________________1____________________\n");
	BP1.LogMe();

	Log("\n");
	Log("_____________________2____________________\n");
	BP2.LogMe();
#endif

	uint SameCount = 0;
	uint DiffCount = 0;
	uint UniquePartCount2 = SIZE(BP2.m_UniquePartNodes);
	for (uint k = 0; k < UniquePartCount2; ++k)
		{
		uint Node2 = BP2.m_UniquePartNodes[k];
		const vector<bool> &Part2 = BP2.m_PartVec[Node2];
		uint Node1 = BP1.Search(Part2);
		if (Node1 == UINT_MAX)
			{
			++DiffCount;
#if TRACE
			Log("Node2 %u (not found)\n", Node2, Node1);
#endif
			}
		else
			{
#if TRACE
			Log("Node2 %u == %u\n", Node2, Node1);
#endif
			++SameCount;
			}
		}
	double RF = double(DiffCount)/UniquePartCount2;
	ProgressLog("RF = %u / %u, %.4f\n", DiffCount, UniquePartCount2, RF);
	return RF;
	}

void cmd_rofo()
	{
	const string &FileName1 = opt(rofo);
	const string &FileName2 = opt(tree2);

	TreeN T1;
	TreeN T2;
	T1.FromNewickFile(FileName1);
	T2.FromNewickFile(FileName2);

	RofoPair(T1, T2);
	}
