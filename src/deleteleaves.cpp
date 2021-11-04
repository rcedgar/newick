#include "myutils.h"
#include "treen.h"
#include "tree2.h"

void StringsFromFile(const string &FileName, vector<string> &Strings);

void cmd_deleteleaves()
	{
	const string &InputFileName = opt(deleteleaves);

	vector<string> Labels;
	if (optset_labels)
		{
		asserta(!optset_label);
		StringsFromFile(opt(labels), Labels);
		}
	else if (optset_label)
		{
		asserta(!optset_labels);
		Labels.push_back(opt(label));
		}
	else
		Die("Must set -label or -labels");
	const uint LabelCount = SIZE(Labels);
	asserta(LabelCount > 0);

	FILE *fOut = CreateStdioFile(opt(output));

	vector<TreeN *> Trees;
	TreesFromFile(InputFileName, Trees);
	const uint TreeCount = SIZE(Trees);
	for (uint TreeIndex = 0; TreeIndex < TreeCount; ++TreeIndex)
		{
		TreeN T = *Trees[TreeIndex];
		for (uint i = 0; i < SIZE(Labels); ++i)
			{
			const string &Label = Labels[i];
			uint Node = (opt(accs) ? T.GetNodeByAcc(Label, false)
			  : T.GetNodeByLabel(Label, false));
			if (Node == UINT_MAX)
				{
//				Warning("Label not found >%s", Label.c_str());
				continue;
				}
			if (!T.IsLeaf(Node))
				{
				Warning("Not leaf >%s", Label.c_str());
				continue;
				}
			T.DeleteLeaf(Node);
			}

		T.CollapseUnary();
		T.SetDerived();
		//uint ChildCountRoot = T.GetChildCount(T.m_Root);
		//if (ChildCountRoot == 1)
		//	{
		//	uint OldRoot = T.m_Root;
		//	uint NewRoot = T.GetChild(OldRoot, 0);
		//	//string OldRootLabel = T.GetLabel(OldRoot);
		//	T.m_Root = NewRoot;
		//	//T.UpdateLabel(NewRoot, OldRootLabel);
		//	T.EraseNode(OldRoot);
		//	T.m_NodeToParent[NewRoot] = UINT_MAX;
		//	T.SetDerived();
		//	}
		T.ToNewickFile(fOut, false);
		}
	CloseStdioFile(fOut);
	}
