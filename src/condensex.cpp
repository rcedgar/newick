#include "myutils.h"
#include "featuretable.h"
#include "treex.h"

static void Condense(TreeX &T, double MinTPFract)
	{
	FeatureTable FT;
	SetFeatureTable(T, FT);

	vector<uint> ValueToBestFitNode;
	const uint ValueCount = FT.GetValueCount();
	for (uint ValueIndex = 0; ValueIndex < ValueCount; ++ValueIndex)
		{
		set<string> LabelSet;
		FT.GetLabels_ByValueIndex(ValueIndex, LabelSet);

		set<uint> LeafNodeSet;
		T.LabelSetToLeafNodeSet(LabelSet, LeafNodeSet);

		uint TP, FP, FN;
		uint BestFitNode = 
		  T.GetBestFitSubtree(LeafNodeSet, MinTPFract, TP, FP, FN);

		ValueToBestFitNode.push_back(BestFitNode);
		}

	for (uint i = 0; i < ValueCount; ++i)
		{
		uint Nodei = ValueToBestFitNode[i];
		if (Nodei == UINT_MAX)
			continue;
		for (uint j = 0; j < ValueCount; ++j)
			{
			if (i == j)
				continue;
			uint Nodej = ValueToBestFitNode[j];
			if (T.IsInSubtree(Nodei, Nodej))
				{
				const string &Valuei = FT.GetValue(i);
				const string &Valuej = FT.GetValue(j);
				Warning("%s > %s\n", Valuei.c_str(), Valuej.c_str());

				ValueToBestFitNode[j] = UINT_MAX;
				}
			}
		}

	for (uint i = 0; i < ValueCount; ++i)
		{
		uint Node = ValueToBestFitNode[i];
		if (Node == UINT_MAX)
			continue;
		const string &Value = FT.GetValue(i);
		string NewLabel = "cond_" + Value;
		T.DeleteSubtree(Node, NewLabel, false);
#if DEBUG
		T.Validate();
#endif
		}

	uint DeleteCount = 0;
	for (;;)
		{
		bool AnyDel = false;
		const uint NodeCount = T.GetNodeIndexCount();
		for (uint Node = 0; Node < NodeCount; ++Node)
			{
			if (!T.IsNode(Node))
				continue;
			const string &Label = T.GetLabel(Node);
			vector<string> SubtreeLabels;
			T.GetSubtreeLeafLabels_Rooted(Node, SubtreeLabels);
			if (SIZE(SubtreeLabels) == 1 &&
			  SubtreeLabels[0].substr(0, 3) == "DEL")
				continue;
			bool Found = false;
			for (uint i = 0; i < SIZE(SubtreeLabels); ++i)
				{
				const string &Label = SubtreeLabels[i];
				if (Label.substr(0, 5) == "cond_")
					{
					Found = true;
					break;
					}
				}
			if (!Found)
				{
				AnyDel = true;
				string NewLabel;
				Ps(NewLabel, "DEL%u", ++DeleteCount);
				T.DeleteSubtree(Node, NewLabel, false);
#if DEBUG
				T.Validate();
#endif
				}
			}
		if (!AnyDel)
			break;
		}

	const uint NodeCount = T.GetNodeIndexCount();
	uint DelCount = 0;
	for (uint Node = 0; Node < NodeCount; ++Node)
		{
		if (!T.IsLeaf(Node))
			continue;
		const string &Label = T.GetLabel(Node);
		if (Label.substr(0, 3) == "DEL")
			{
#if DEBUG
			T.Validate();
#endif
			T.DeleteLeaf(Node);
#if DEBUG
			T.Validate();
#endif
			++DelCount;
			}
		}
	//T.Normalize();
#if DEBUG
		T.Validate();
#endif
	}

void cmd_condensex()
	{
	vector<TreeX *> Trees;
	TreesFromFile(opt(condensex), Trees);
	FILE *fTsv = CreateStdioFile(opt(tsvout));

	double MinTPFract = 0.5;
	if (optset_mintpfract)
		MinTPFract = opt(mintpfract);

	const uint TreeCount = SIZE(Trees);
	for (uint TreeIndex = 0; TreeIndex < TreeCount; ++TreeIndex)
		{
		TreeX &T = *Trees[TreeIndex];
		Condense(T, MinTPFract);
		T.CollapseUnary();
		T.Ladderize();
		T.Validate();
		}

	TreesToFile(Trees, opt(output));

	CloseStdioFile(fTsv);
	}
