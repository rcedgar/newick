#include "myutils.h"
#include "featuretable.h"
#include "treex.h"

void StringsFromFile(const string &FileName, vector<string> &Strings);
void TreesFromFile(const string &FileName, vector<TreeX *> &Trees);
void TreesToFile(const vector<TreeX *> &Trees, const string &FileName);

void TreeX::SetRootByGroupLabels(const vector<string> &GroupLabels)
	{
	set<uint> LeafNodes;
	for (uint i = 0; i < SIZE(GroupLabels); ++i)
		{
		uint Node = GetNodeByLabel(GroupLabels[i], false);
		if (Node != UINT_MAX)
			LeafNodes.insert(Node);
		}
	asserta(LeafNodes.size() > 0);

	uint FromNode, ToNode;
	GetLCAEdge(LeafNodes, FromNode, ToNode);
	if (GetParent(ToNode) == FromNode)
		{
		Log("Root edge %u --> %u\n", FromNode, ToNode);
		InsertRootAbove(ToNode);
		}
	else if (GetParent(FromNode) == ToNode)
		{
		Log("Root edge %u <-- %u\n", ToNode, FromNode);
		InsertRootAbove(FromNode);
		}
	else
		asserta(false);

	Validate();
	}

void cmd_rootbyoutgroupx()
	{
	const string &InputFileName = opt(rootbyoutgroup);
	const string &TsvFileName = opt(tsvout);
	FILE *fTsv = CreateStdioFile(TsvFileName);

	vector<TreeX *> Trees;
	TreesFromFile(InputFileName, Trees);
	const uint TreeCount = SIZE(Trees);

	const string NewRootLabel = optset_root_label ? opt(root_label) : "";
	const string OldRootLabel = optset_old_root_label ? opt(old_root_label) : "-";

	vector<string> GroupLabels;
	string GroupName;

	asserta(optset_labels);
	StringsFromFile(opt(labels), GroupLabels);

	for (uint TreeIndex = 0; TreeIndex < TreeCount; ++TreeIndex)
		{
		TreeX &T = *Trees[TreeIndex];
		T.SetRootByGroupLabels(GroupLabels);
		}

	TreesToFile(Trees, opt(output));
	CloseStdioFile(fTsv);
	}
