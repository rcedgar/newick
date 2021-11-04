#include "myutils.h"
#include "biparter.h"

static FILE *g_fTab;

static void BootQPair(const TreeN &T1, const TreeN &T2)
	{
	uint LeafCount = T1.GetLeafCount();

	if (T2.GetLeafCount() != LeafCount)
		Die("Different leaf counts");

	BiParter BP1;
	BiParter BP2;

	BP1.Init(T1);
	BP2.Init(T2);
	const uint NodeCount1 = T1.GetNodeCount();

#if TRACE
	Log("_____________________1____________________\n");
	BP1.LogMe();

	Log("\n");
	Log("_____________________2____________________\n");
	BP2.LogMe();
#endif

	uint SameCount = 0;
	uint DiffCount = 0;
	const vector<vector<bool> > &PartVec1 = BP1.m_PartVec;
	for (uint Node1 = 0; Node1 < NodeCount1; ++Node1)
		{
		if (T1.IsLeaf(Node1) || T1.IsRoot(Node1))
			continue;

		asserta(Node1 < SIZE(PartVec1));
		const vector<bool> &Part1 = PartVec1[Node1];

		vector<string> Labels;
		BP1.GetPartInternalNodeLabels(Node1, Labels);
		const uint LabelCount = SIZE(Labels);

		uint Node2 = BP2.Search(Part1);
		bool Found = (Node2 != UINT_MAX);
		if (Found)
			++SameCount;
		else
			++DiffCount;

		Pf(g_fTab, "%u", Node1);
		Pf(g_fTab, "\t%c", tof(Found));
		bool First = true;
		for (uint LabelIndex = 0; LabelIndex < LabelCount; ++LabelIndex)
			{
			const string &Label = Labels[LabelIndex];
			if (Label == "")
				continue;
			if (First)
				Pf(g_fTab, "\t");
			else
				Pf(g_fTab, ",");
			Pf(g_fTab, "%s", Label.c_str());
			}
		Pf(g_fTab, "\n");
#if TRACE
		Log("Node1=%u Found=%c\n", Node1, tof(Found));
#endif
		}

	const uint UniquePartCount1 = SIZE(BP1.m_UniquePartNodes);
	double RF = double(DiffCount)/UniquePartCount1;
	double Q = double(SameCount)/UniquePartCount1;
	ProgressLog("RF = %u / %u, %.4f Q = %.4f\n", DiffCount, UniquePartCount1, RF, Q);
	}

// "Q"-score of tree compared to correct reference tree
//   plus bootstrap assessment.
void cmd_bootq()
	{
	const string &FileName1 = opt(bootq);
	const string &FileName2 = opt(ref);
	const string &OutputFileName = opt(output);
	g_fTab = CreateStdioFile(OutputFileName);

	TreeN T1;
	TreeN T2;
	T1.FromNewickFile(FileName1);
	T2.FromNewickFile(FileName2);

	BootQPair(T1, T2);

	CloseStdioFile(g_fTab);
	}
