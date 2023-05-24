#include "myutils.h"
#include "featuretable.h"
#include "treen.h"

void StringsFromFile(const string &FileName, vector<string> &Strings);
void TreesFromFile(const string &FileName, vector<TreeN *> &Trees);
void TreesToFile(const vector<TreeN *> &Trees, const string &FileName);

void cmd_deletegroup()
	{
	const string &InputFileName = opt(deletegroup);

	vector<TreeN *> Trees;
	TreesFromFile(InputFileName, Trees);
	const uint TreeCount = SIZE(Trees);

	vector<string> GroupLabelsVec;
	set<string> GroupLabels;
	string GroupName;
	string LabelSubstr;

	if (optset_labels)
		{
		StringsFromFile(opt(labels), GroupLabelsVec);
		for (uint i = 0; i < SIZE(GroupLabelsVec); ++i)
			GroupLabels.insert(GroupLabelsVec[i]);
		}
	else if (optset_label)
		GroupName = opt(label);
	else
		Die("Must specify -labels, -label or -label_substr");

	for (uint TreeIndex = 0; TreeIndex < TreeCount; ++TreeIndex)
		{
		TreeN &T = *Trees[TreeIndex];

		vector<uint> LeafNodes;
		T.GetLeafNodes(LeafNodes);
		const uint LeafCount = SIZE(LeafNodes);

		set<uint> GroupLeafNodes;
		if (optset_labels)
			T.GetGroupLeafNodes(GroupLabels, GroupLeafNodes);
		else if (optset_label)
			T.GetGroupLeafNodes(GroupName, GroupLeafNodes);
		else
			asserta(false);

		if (GroupLeafNodes.empty())
			Die("Group not found");

		uint FPs;
		uint FNs;
		bool Invert;
		uint LCA = T.GetBestFitSubtree1(GroupLeafNodes, false, FPs, FNs, Invert);
		asserta(LCA != UINT_MAX);

		T.DeleteSubtree(LCA);
		T.CollapseUnary();
		T.Ladderize(opt(right));
		}

	TreesToFile(Trees, opt(output));
	}
