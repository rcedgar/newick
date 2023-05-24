#include "myutils.h"
#include "featuretable.h"
#include "treex.h"

void TreesFromFile(const string &FileName, vector<TreeX *> &Trees);
void SetFeatureTable(const TreeX &T, FeatureTable &FT);

static FILE *g_fFev;
static FILE *g_fTsv;
static FILE *g_fOut;
static uint g_KFold;

static void DoTree(uint TreeIndex, TreeX &T)
	{
	const double MinTPFract = 0.5;
	const uint NodeIndexCount = T.GetNodeIndexCount();

	FeatureTable FT;
	SetFeatureTable(T, FT);
	const uint ValueCount = FT.GetValueCount();

	set<string> LabelSet;
	set<uint> LeafNodeSet;
	vector<vector<uint> > NodeToValueCountVec(NodeIndexCount);
	for (uint Node = 0; Node < NodeIndexCount; ++Node)
		NodeToValueCountVec[Node].resize(ValueCount, 0);

	set<uint> HoldOutSet;
	if (g_KFold != UINT_MAX)
		{
		for (uint ValueIndex = 0; ValueIndex < ValueCount; ++ValueIndex)
			{
			FT.GetLabels_ByValueIndex(ValueIndex, LabelSet);
			T.LabelSetToLeafNodeSet(LabelSet, LeafNodeSet);
			for (set<uint>::const_iterator p = LeafNodeSet.begin();
			  p != LeafNodeSet.end(); ++p)
				{
				uint r = randu32();
				if (r%g_KFold == 0)
					{
					uint LeafNode = *p;
					HoldOutSet.insert(LeafNode);
					}
				}
			}
		}

	vector<uint> Path;
	for (uint ValueIndex = 0; ValueIndex < ValueCount; ++ValueIndex)
		{
		FT.GetLabels_ByValueIndex(ValueIndex, LabelSet);
		T.LabelSetToLeafNodeSet(LabelSet, LeafNodeSet);

		for (set<uint>::const_iterator p = LeafNodeSet.begin();
		  p != LeafNodeSet.end(); ++p)
			{
			uint LeafNode = *p;
			if (g_KFold != UINT_MAX &&
			  HoldOutSet.find(LeafNode) != HoldOutSet.end())
				continue;

			T.GetPathToOrigin2(LeafNode, Path);
			for (uint i = 0; i < SIZE(Path); ++i)
				{
				uint Node = Path[i];
				NodeToValueCountVec[Node][ValueIndex] += 1;
				}
			}
		}

	vector<uint> ValueIndexToBestNode;
	map<uint, uint> BestNodeToValueIndex;
	for (uint ValueIndex = 0; ValueIndex < ValueCount; ++ValueIndex)
		{
		uint N = FT.GetLabelCount_ByValueIndex(ValueIndex);
		uint BestNode = UINT_MAX;
		uint BestErrs = UINT_MAX;
		uint TP = UINT_MAX;
		uint FP = UINT_MAX;
		uint FN = UINT_MAX;

		uint MinTP = uint(N*MinTPFract);
		if (MinTP == 0)
			MinTP = 1;
		const uint NodeIndexCount = SIZE(NodeToValueCountVec);
		for (uint Node = 0; Node < NodeIndexCount; ++Node)
			{
			if (!T.IsNode(Node))
				continue;
			const vector<uint> &ValueToCount = NodeToValueCountVec[Node];
			asserta(SIZE(ValueToCount) == ValueCount);
			uint TP2 = 0;
			uint FP2 = 0;
			for (uint ValueIndex2 = 0; ValueIndex2 < ValueCount;
			  ++ValueIndex2)
				{
				uint n = ValueToCount[ValueIndex2];
				if (ValueIndex2 == ValueIndex)
					TP2 = n;
				else
					FP2 += n;
				}
			if (TP2 < MinTP)
				continue;
			asserta(TP2 <= N);
			uint FN2 = N - TP2;
			uint Errs2 = FP2 + 2*FN2;
			if (BestNode == UINT_MAX || Errs2 < BestErrs)
				{
				BestNode = Node;
				BestErrs = Errs2;
				TP = TP2;
				FP = FP2;
				FN = FN2;
				}
			}

		double Acc = double(TP)/(N + FP);
		const string &ValueName = FT.GetValue(ValueIndex);
		T.UpdateLabel(BestNode, ValueName);
		ValueIndexToBestNode.push_back(BestNode);
		if (BestNode != UINT_MAX)
			BestNodeToValueIndex[BestNode] = ValueIndex;

		if (g_fFev != 0)
			{
			FILE *f = g_fFev;
			fprintf(f, "tree=%u", TreeIndex);
			fprintf(f, "\tname=%s", ValueName.c_str());
			fprintf(f, "\tacc=%.4f", Acc);
			fprintf(f, "\tN=%u", N);
			fprintf(f, "\tTP=%u", TP);
			fprintf(f, "\tFP=%u", FP);
			fprintf(f, "\tFN=%u", FN);
			fprintf(f, "\n");
			vector<uint> LeafNodes;
			T.GetSubtreeLeafNodes_Rooted(BestNode, LeafNodes);

			for (uint i = 0; i < SIZE(LeafNodes); ++i)
				{
				uint LeafNode = LeafNodes[i];

				const string &Label = T.GetLabel(LeafNode);
				string LeafValue;
				FT.GetValue_ByLabel(Label, LeafValue, false);
				string sResult = "?";
				if (LeafValue.empty())
					{
					LeafValue = ".";
					sResult = "-";
					}
				else
					{
					if (LeafValue == ValueName)
						sResult = "TP";
					else
						sResult = "FP";
					}

				fprintf(f, "tree_subtree=%u", TreeIndex);
				fprintf(f, "\tsubtree_value=%s", ValueName.c_str());
				fprintf(f, "\tsubtree_leaf_value=%s", LeafValue.c_str());
				fprintf(f, "\tsubtree_leaf_label=%s", Label.c_str());
				fprintf(f, "\tresult=%s", sResult.c_str());
				if (g_KFold != UINT_MAX)
					{
					if (HoldOutSet.find(LeafNode) != HoldOutSet.end())
						fprintf(f, "\thold=out");
					else
						fprintf(f, "\thold=in");
					}
				fprintf(f, "\n");
				}
			}
		}
	asserta(SIZE(ValueIndexToBestNode) == ValueCount);

	for (uint Node = 0; Node < NodeIndexCount; ++Node)
		{
		if (!T.IsLeaf(Node))
			continue;

		T.GetPathToOrigin2(Node, Path);
		uint ValueIndex = UINT_MAX;
		for (uint i = 0; i < SIZE(Path); ++i)
			{
			uint Node2 = Path[i];
			if (BestNodeToValueIndex.find(Node2) !=
			  BestNodeToValueIndex.end())
				{
				ValueIndex = BestNodeToValueIndex[Node2];
				break;
				}
			}

		if (g_fTsv != 0)
			{
			FILE *f = g_fTsv;
			const string &Label = T.GetLabel(Node);
			string ValueName;

			fprintf(f, "%u", TreeIndex);
			fprintf(f, "\t%s", Label.c_str());
			if (ValueIndex == UINT_MAX)
				fprintf(f, "\t.");
			else
				fprintf(f, "\t%s", FT.GetValue(ValueIndex).c_str());
			fprintf(f, "\n");
			}

		if (g_fFev != 0)
			{
			FILE *f = g_fFev;
			const string &Label = T.GetLabel(Node);
			uint TrueValueIndex = FT.GetValueIndex_ByLabel(Label);
			string TrueValue = ".";
			if (TrueValueIndex != UINT_MAX)
				TrueValue = FT.GetValue(TrueValueIndex);

			string Result = "?";
			fprintf(f, "tree_pred=%u", TreeIndex);
			fprintf(f, "\tlabel=%s", Label.c_str());
			if (ValueIndex == UINT_MAX)
				{
				fprintf(f, "\tpred_value=.");
				if (TrueValueIndex == UINT_MAX)
					Result = "UN";
				else
					Result = "FN";
				}
			else
				{
				const string &ValueName = FT.GetValue(ValueIndex);
				fprintf(f, "\tpred_value=%s", ValueName.c_str());
				if (TrueValueIndex == UINT_MAX)
					Result = "UN";
				else
					{
					if (ValueName == TrueValue)
						Result = "TP";
					else
						Result = "FP";
					}
				fprintf(f, "\ttrue_value=%s", ValueName.c_str());
				fprintf(f, "\tpred_result=%s", Result.c_str());
				}
			fprintf(f, "\n");
			}
		}

	T.ToNewickFile(g_fOut);
	}

void cmd_taxqx()
	{
	const string &InputFileName = opt(taxqx);
	g_fTsv = CreateStdioFile(opt(tsvout));
	g_fFev = CreateStdioFile(opt(fevout));
	g_fOut = CreateStdioFile(opt(output));
	g_KFold = UINT_MAX;
	if (optset_kfold)
		{
		g_KFold = opt(kfold);
		asserta(g_KFold > 1);
		}

	vector<TreeX *> Trees;
	TreesFromFile(InputFileName, Trees);
	const uint TreeCount = SIZE(Trees);

	FeatureTable FT;
	for (uint TreeIndex = 0; TreeIndex < TreeCount; ++TreeIndex)
		{
		TreeX &T = *Trees[TreeIndex];
		DoTree(TreeIndex, T);
		}

	CloseStdioFile(g_fTsv);
	CloseStdioFile(g_fFev);
	CloseStdioFile(g_fOut);
	}
