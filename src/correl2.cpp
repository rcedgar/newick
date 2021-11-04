#include "myutils.h"
#include "treen.h"
#include "biparter.h"

// Output tsv with confidences from equivalent bipartitions 
// in two trees with some/all leaves in common.
void cmd_correl2()
	{
	const string &TreeFN1 = opt(correl2);
	const string &TreeFN2 = opt(tree2);

	FILE *fOut = CreateStdioFile(opt(output));

	TreeN T1;
	TreeN T2;

	T1.FromNewickFile(TreeFN1);
	T2.FromNewickFile(TreeFN2);

	BiParter BP1;
	BiParter BP2;

	BP1.Init(T1);
	BP2.Init(T2);

	uint UniquePartCount2 = SIZE(BP2.m_UniquePartNodes);
	for (uint k = 0; k < UniquePartCount2; ++k)
		{
		uint Node2 = BP2.m_UniquePartNodes[k];
		const vector<bool> &Part2 = BP2.m_PartVec[Node2];
		uint Node1 = BP1.Search(Part2);
		if (Node1 != UINT_MAX)
			{
			const string &Label1 = T1.GetLabel(Node1);
			const string &Label2 = T2.GetLabel(Node2);
			if (Label1.empty() || Label2.empty())
				continue;
			if (!IsValidFloatStr(Label1) || !IsValidFloatStr(Label2))
				continue;
			Pf(fOut, "%s	%s\n", Label1.c_str(), Label2.c_str());
			}
		}

	CloseStdioFile(fOut);
	}
