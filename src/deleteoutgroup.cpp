#include "myutils.h"
#include "featuretable.h"
#include "treen.h"

void StringsFromFile(const string &FileName, vector<string> &Strings);
void TreesFromFile(const string &FileName, vector<TreeN *> &Trees);
void TreesToFile(const vector<TreeN *> &Trees, const string &FileName);

void cmd_deleteoutgroup()
	{
	const string &InputFileName = opt(deleteoutgroup);

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
	else if (optset_outgroup)
		GroupName = opt(outgroup);
	else
		Die("Must specify -labels or -outgroup");

	vector<TreeN *> NewTrees;
	for (uint TreeIndex = 0; TreeIndex < TreeCount; ++TreeIndex)
		{
		TreeN &T = *Trees[TreeIndex];

		bool Rooted = false;
		bool Binary = T.IsBinary(Rooted);
		asserta(Rooted && Binary);

		vector<uint> LeafNodes;
		T.GetLeafNodes(LeafNodes);
		const uint LeafCount = SIZE(LeafNodes);

		set<uint> GroupLeafNodes;
		if (optset_labels)
			T.GetGroupLeafNodes(GroupLabels, GroupLeafNodes);
		else if (optset_outgroup)
			T.GetGroupLeafNodes(GroupName, GroupLeafNodes);
		else
			asserta(false);

		if (GroupLeafNodes.empty())
			Die("Group not found");

		uint Root = T.GetRoot();
		uint RootLeft = T.GetLeft(Root);
		uint RootRight = T.GetRight(Root);

		vector<uint> LeftLeaves;
		vector<uint> RightLeaves;
		T.AppendSubtreeLeafNodes(RootLeft, LeftLeaves);
		T.AppendSubtreeLeafNodes(RootRight, RightLeaves);

		uint LeftCount = 0;
		uint RightCount = 0;
		for (uint i = 0; i < SIZE(LeftLeaves); ++i)
			{
			uint LeafNode = LeftLeaves[i];
			if (GroupLeafNodes.find(LeafNode) != GroupLeafNodes.end())
				++LeftCount;
			}

		for (uint i = 0; i < SIZE(RightLeaves); ++i)
			{
			uint LeafNode = RightLeaves[i];
			if (GroupLeafNodes.find(LeafNode) != GroupLeafNodes.end())
				++RightCount;
			}
		asserta(LeftCount > 0 || RightCount > 0);

		TreeN &NewTree = *new TreeN;
		if (LeftCount > RightCount)
			NewTree.FromSubtree(T, RootRight);
		else
			NewTree.FromSubtree(T, RootLeft);

		if (optset_root_label)
			NewTree.UpdateLabel(NewTree.m_Root, opt(root_label));

		NewTrees.push_back(&NewTree);
		}

	TreesToFile(NewTrees, opt(output));
	}
