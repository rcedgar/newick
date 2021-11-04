#include "myutils.h"
#include "treen.h"

void StringsFromFile(const string &FileName, vector<string> &Strings);

static void Subset(const string &InputFileName, bool UseAccs)
	{
	const string &LabelsFileName = opt(labels);

	vector<string> Labels;
	StringsFromFile(LabelsFileName, Labels);
	
	TreeN T;
	T.FromNewickFile(InputFileName);

	set<uint> LeafNodeIndexes;
	const uint LabelCount = SIZE(Labels);
	vector<string> NotLeaves;
	for (uint i = 0; i < LabelCount; ++i)
		{
		const string &Label = Labels[i];
		uint NodeIndex = UINT_MAX;
		if (UseAccs)
			{
			string Acc;
			GetAccFromLabel(Label, Acc);
			NodeIndex = T.GetNodeByAcc(Acc, false);
			if (NodeIndex == UINT_MAX)
				Log("Not found >%s (acc=%s)\n",
				  Label.c_str(), Acc.c_str());
			}
		else
			NodeIndex = T.GetNodeByLabel(Label, false);
		if (NodeIndex == UINT_MAX || !T.IsLeaf(NodeIndex))
			{
			NotLeaves.push_back(Label);
			continue;
			}
		LeafNodeIndexes.insert(NodeIndex);
		}

	if (!NotLeaves.empty())
		Warning("%u / %u labels not found",
		  SIZE(NotLeaves), SIZE(Labels));

	T.Subset(LeafNodeIndexes);
	T.CollapseConfidenceUnary();
	T.CollapseUnary();
	T.ToNewickFile(opt(output));
	}

void cmd_subset()
	{
	Subset(opt(subset), false);
	}

void cmd_subsetacc()
	{
	Subset(opt(subsetacc), true);
	}
