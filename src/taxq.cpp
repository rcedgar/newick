#include "myutils.h"
#include "featuretable.h"
#include "biparter.h"

void GetFractConfs(const TreeN &T, vector<double> &Confs);
void ReadTaxTable(const string &FileName, const vector<string> &Ranks,
  vector<FeatureTable *> &FTs);

void SetFeatureTable(const TreeN &T, FeatureTable &FT)
	{
	if (optset_ff)
		{
		const string &FF = opt(ff);
		if (SIZE(FF) != 2 || !isdigit(FF[1]))
			Die("Invalid ff");

		char Sep = FF[0];
		char Digit = FF[1];
		if (Digit == '0')
			Die("Invalid ff (field must be >0)");
		uint FieldIndex = uint(Digit - '0') - 1;
		FT.FromTree(T, Sep, FieldIndex);
		}
	else if (optset_features)
		{
		FT.FromFile(opt(features));
		FT.SetLeafNodeSet(T);
		}
	else
		Die("Must set -ff or -features");
	}

static bool IsConflict(uint GroupInSubtree, uint GroupSize, uint SubtreeSize)
	{
	if (GroupInSubtree == 0)
		return false;
	if (GroupInSubtree == SubtreeSize)
		return false;
	if (GroupInSubtree == GroupSize)
		return false;
	return true;
	}

double GetNodeToAnyConflict(FILE *fTsv, const TreeN &T,
  const vector<double> &FractConfs,
  const FeatureTable &FT, vector<bool> &NodeToAnyConflict)
	{
	NodeToAnyConflict.clear();
	const uint NodeCount = T.GetNodeCount();
	const uint ValueCount = FT.GetValueCount();
	NodeToAnyConflict.resize(NodeCount, false);

	vector<uint> ValueIndexToLeafCount;
	for (uint ValueIndex = 0; ValueIndex < ValueCount; ++ValueIndex)
		{
		uint Size = FT.GetLabelCount_ByValueIndex(ValueIndex);
		ValueIndexToLeafCount.push_back(Size);
		}

	vector<uint> NodeToValueIndex(NodeCount, UINT_MAX);
	for (uint Node = 0; Node < NodeCount; ++Node)
		{
		if (!T.IsLeaf(Node))
			continue;
		const string &Label = T.GetLabel(Node);
		if (Label.empty())
			continue;
		string Acc;
		GetAccFromLabel(Label, Acc);
		uint LabelIndex = FT.GetLabelIndex_FailOnError(Acc);
		uint ValueIndex = FT.GetValueIndex_ByLabelIndex(LabelIndex);
		NodeToValueIndex[Node] = ValueIndex;
		}

	uint InternalNodeCount = 0;
	uint ConflictCount = 0;
	for (uint Node = 0; Node < NodeCount; ++Node)
		{
		if (T.IsLeaf(Node) || T.IsRoot(Node))
			continue;

		vector<uint> SubtreeLeafNodes;
		T.AppendSubtreeLeafNodes(Node, SubtreeLeafNodes);

		vector<uint> ValueIndexToSubtreeLeafCount(ValueCount, 0);
		const uint SubtreeSize = SIZE(SubtreeLeafNodes);
		uint SubtreeTotal = 0;
		for (uint i = 0; i < SubtreeSize; ++i)
			{
			uint LeafNode = SubtreeLeafNodes[i];
			uint ValueIndex = NodeToValueIndex[LeafNode];
			if (ValueIndex != UINT_MAX)
				{
				asserta(ValueIndex < SIZE(ValueIndexToSubtreeLeafCount));
				++ValueIndexToSubtreeLeafCount[ValueIndex];
				++SubtreeTotal;
				}
			}

		const string &Label = T.GetLabel(Node);
		bool AnyConflict = false;
		for (uint ValueIndex = 0; ValueIndex < ValueCount; ++ValueIndex)
			{
			uint GroupSize = ValueIndexToLeafCount[ValueIndex];
			uint GroupInSubtree = ValueIndexToSubtreeLeafCount[ValueIndex];
			if (GroupInSubtree == 0)
				continue;
			bool Conflict = IsConflict(GroupInSubtree, GroupSize, SubtreeTotal);
			if (Conflict)
				{
				AnyConflict = true;
				if (fTsv != 0)
					{
					const string &Value = FT.GetValue(ValueIndex);
					Pf(fTsv, "conflict");
					Pf(fTsv, "\tnode=%u", Node);
					Pf(fTsv, "\tlabel=%s", Label.c_str());
					Pf(fTsv, "\tvalue=%s", Value.c_str());
					Pf(fTsv, "\tgroup_size=%u", GroupSize);
					Pf(fTsv, "\tgroup_in_subtree=%u", GroupInSubtree);
					Pf(fTsv, "\tsubtree_size=%u", SubtreeTotal);
					Pf(fTsv, "\n");
					}
				break;
				}
			}
		++InternalNodeCount;
		if (AnyConflict)
			{
			++ConflictCount;
			NodeToAnyConflict[Node] = true;
			}
		}
	double Q = double(InternalNodeCount - ConflictCount)/InternalNodeCount;
	return Q;
	}

void cmd_taxq()
	{
	const string &TreeFileName = opt(taxq);
	const string &OutputFileName = opt(output);
	FILE *fTsv = CreateStdioFile(OutputFileName);
	const string &TaxTableFileName = opt(taxtable);

	vector<string> Ranks;
	Ranks.push_back("phylum");
	Ranks.push_back("order");
	Ranks.push_back("class");
	Ranks.push_back("family");
	Ranks.push_back("genus");
	const uint RankCount = SIZE(Ranks);

	vector<FeatureTable *> FTs;
	ReadTaxTable(TaxTableFileName, Ranks, FTs);

	TreeN T;
	T.FromNewickFile(TreeFileName);
	asserta(T.IsNormalized());
	const uint NodeCount = T.GetNodeCount();
	asserta(T.IsNormalized());

	vector<double> FractConfs;
	GetFractConfs(T, FractConfs);
	asserta(SIZE(FractConfs) == NodeCount);

	vector<bool> AccumNodeToAnyConflict(NodeCount, false);
	for (uint i = 0; i < RankCount; ++i)
		{
		const FeatureTable &FT = *FTs[i];
		vector<bool> NodeToAnyConflict;
		double Q = GetNodeToAnyConflict(fTsv, T, FractConfs, 
		  FT, NodeToAnyConflict);
		asserta(SIZE(NodeToAnyConflict) == NodeCount);
		for (uint j = 0; j < NodeCount; ++j)
			AccumNodeToAnyConflict[j] =
			  (AccumNodeToAnyConflict[j] || NodeToAnyConflict[j]);
		}

	for (uint Node = 0; Node < NodeCount; ++Node)
		{
		if (T.IsLeaf(Node) || T.IsRoot(Node))
			continue;

		double FractConf = FractConfs[Node];
		bool AnyConflict = AccumNodeToAnyConflict[Node];
		Pf(fTsv, "eval");
		Pf(fTsv, "\tnode=%u", Node);
		if (FractConf < 0)
			Pf(fTsv, "\tconf=.", FractConf);
		else
			Pf(fTsv, "\tconf=%.3f", FractConf);
		Pf(fTsv, "\tconflict=%c", yon(AnyConflict));
		Pf(fTsv, "\n");
		}

	CloseStdioFile(fTsv);
	}
