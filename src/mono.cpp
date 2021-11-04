#include "myutils.h"
#include "featuretable.h"
#include "treen.h"
#include "quarts.h"

void SetFeatureTable(const TreeN &T, FeatureTable &FT);
void GetGroupLeafNodeSet(const TreeN &T, const FeatureTable &FT,
  uint ValueIndex, set<uint> &LeafNodeSet);
void GetFractConfs(const TreeN &T, vector<double> &Confs);
void GetLCAs(const TreeN &T, const FeatureTable &FT, bool AllowInvert,
  vector<string> &Values, vector<uint> &LCAs, vector<uint> &GroupSizes,
  vector<uint> &SubtreeSizes, vector<uint> &FPs, vector<uint> &FNs,
  vector<bool> &Inverts, vector<double> &MonoFs);

static uint g_TreeIndex;
static FILE *g_fTsv;

void GetNodeToValueIndex(const TreeN &T, const FeatureTable &FT,
  vector<uint> &NodeToValueIndex)
	{
	NodeToValueIndex.clear();
	asserta(T.IsNormalized());
	const uint NodeCount = T.GetNodeCount();
	NodeToValueIndex.resize(NodeCount, UINT_MAX);
	for (uint Node = 0; Node < NodeCount; ++Node)
		{
		if (!T.IsLeaf(Node))
			continue;
		string Label = T.GetLabel(Node);
		if (opt(accs))
			GetAccFromLabel(Label, Label);

		uint ValueIndex = FT.GetValueIndex_ByLabel(Label);
		NodeToValueIndex[Node] = ValueIndex;
		}
	}

static void GetValueCounts(const TreeN &T, uint Node, uint ValueCount,
  const vector<uint> &NodeToValueIndex, vector<uint> &ValueCounts)
	{
	ValueCounts.clear();
	ValueCounts.resize(ValueCount, 0);

	const uint NodeCount = SIZE(NodeToValueIndex);
	vector<uint> LeafNodes;
	T.AppendSubtreeLeafNodes(Node, LeafNodes);
	const uint N = SIZE(LeafNodes);
	for (uint i = 0; i < N; ++i)
		{
		uint LeafNode = LeafNodes[i];
		asserta(LeafNode < NodeCount);
		uint ValueIndex = NodeToValueIndex[LeafNode];
		if (ValueIndex != UINT_MAX)
			{
			asserta(ValueIndex < ValueCount);
			++ValueCounts[ValueIndex];
			}
		}
	}

static uint GetBestFitSubtree(const TreeN &T, uint ValueCount,
  const vector<uint> &NodeToValueIndex, uint ValueIndex)
	{
	vector<uint> SubtreeLeafNodes;
	const uint NodeCount = T.GetNodeCount();
	asserta(SIZE(NodeToValueIndex) == NodeCount);

	uint InGroupSize = 0;
	for (uint Node = 0; Node < NodeCount; ++Node)
		if (NodeToValueIndex[Node] == ValueIndex)
			++InGroupSize;

	uint BestNode = UINT_MAX;
	uint BestErrs = UINT_MAX;
	for (uint Node = 0; Node < NodeCount; ++Node)
		{
		SubtreeLeafNodes.clear();
		T.AppendSubtreeLeafNodes(Node, SubtreeLeafNodes);
		const uint N = SIZE(SubtreeLeafNodes);
		uint InGroupCount = 0;
		uint OutGroupCount = 0;
		for (uint i = 0; i < N; ++i)
			{
			uint SubtreeLeafNode = SubtreeLeafNodes[i];
			uint ValueIndex2 = NodeToValueIndex[SubtreeLeafNode];
			if (ValueIndex2 != UINT_MAX)
				{
				if (ValueIndex2 == ValueIndex)
					++InGroupCount;
				else
					++OutGroupCount;
				}
			}
		asserta(InGroupSize >= InGroupCount);
		uint FN = InGroupSize - InGroupCount;
		if (FN*2 > InGroupSize)
			continue;

		uint FP = OutGroupCount;
		uint Errs = FN + FP;
		if (Errs < BestErrs)
			{
			BestNode = Node;
			BestErrs = Errs;
			}
		}
	return BestNode;
	}

static void DoMono(FILE *f, const TreeN &T, const FeatureTable &FT,
  vector<double> &Fracts)
	{
	if (f == 0)
		return;

	Fracts.clear();

	const uint NodeCount = T.GetNodeCount();
	const uint ValueCount = FT.GetValueCount();

	vector<uint> NodeToValueIndex;
	GetNodeToValueIndex(T, FT, NodeToValueIndex);
	asserta(SIZE(NodeToValueIndex) == NodeCount);

	vector<uint> ValueTotals(ValueCount, 0);
	for (uint Node = 0; Node < NodeCount; ++Node)
		{
		uint ValueIndex = NodeToValueIndex[Node];
		if (ValueIndex != UINT_MAX)
			{
			asserta(ValueIndex < ValueCount);
			++ValueTotals[ValueIndex];
			}
		}

	for (uint ValueIndex = 0; ValueIndex < ValueCount; ++ValueIndex)
		{
		const string &Value = FT.GetValue(ValueIndex);
		
		uint BFS = GetBestFitSubtree(T, ValueCount, NodeToValueIndex, ValueIndex);
		asserta(BFS != UINT_MAX);

		vector<uint> ValueCounts;
		GetValueCounts(T, BFS, ValueCount, NodeToValueIndex, ValueCounts);

		uint SubtreeSize = 0;
		for (uint ValueIndex2 = 0; ValueIndex2 < ValueCount; ++ValueIndex2)
			SubtreeSize += ValueCounts[ValueIndex2];

		const uint ValueTotal = ValueTotals[ValueIndex];
		asserta(ValueTotal > 0);

		uint Count = ValueCounts[ValueIndex];
		double ValueFract = double(Count)/ValueTotal;
		Fracts.push_back(ValueFract);

		fprintf(f, "tree=%u", g_TreeIndex);
		fprintf(f, "\tvalue=%s", Value.c_str());
		fprintf(f, "\tfract=%.4f", ValueFract);
		fprintf(f, "\tbfs=%u", BFS);
		fprintf(f, "\tgroup_size=%u", ValueTotal);
		fprintf(f, "\tsubtree_size=%u", SubtreeSize);
		for (uint ValueIndex2 = 0; ValueIndex2 < ValueCount; ++ValueIndex2)
			{
			const string &Value2 = FT.GetValue(ValueIndex2);

			uint Total2 = ValueTotals[ValueIndex2];
			uint Count2 = ValueCounts[ValueIndex2];
			asserta(Total2 > 0);

			double ValueFract = double(Count2)/Total2;
			double SubtreeFract = double(Count2)/SubtreeSize;
			fprintf(f, "\tvalue2=%s:%u:%u:%.4f:%.4f",
			  Value2.c_str(), Count2, Total2, ValueFract, SubtreeFract);
			}
		fprintf(f, "\n");
		}
	}

void cmd_mono()
	{
	vector<TreeN *> Trees;
	TreesFromFile(opt(mono), Trees);

	asserta(optset_features);
	FeatureTable FT;
	FT.FromFile(opt(features));
	const uint ValueCount = FT.GetValueCount();

	g_fTsv = CreateStdioFile(opt(output));

	const uint TreeCount = SIZE(Trees);
	vector<vector<double> > Mx(ValueCount);
	for (uint ValueIndex = 0; ValueIndex < ValueCount; ++ValueIndex)
		Mx[ValueIndex].resize(ValueCount);

	vector<vector<double> > ValueToFracts(ValueCount);
	for (g_TreeIndex = 0; g_TreeIndex < TreeCount; ++g_TreeIndex)
		{
		TreeN &T = *Trees[g_TreeIndex];
		asserta(T.IsNormalized());
		const uint NodeCount = T.GetNodeCount();

		bool Rooted = false;
		bool Binary = T.IsBinary(Rooted);
		if (!Binary || !Rooted)
			Warning("non-binary / non-rooted tree");

		vector<double> Fracts;
		DoMono(g_fTsv, T, FT, Fracts);
		asserta(SIZE(Fracts) == ValueCount);

		for (uint ValueIndex = 0; ValueIndex < ValueCount; ++ValueIndex)
			ValueToFracts[ValueIndex].push_back(Fracts[ValueIndex]);
		}

	for (uint ValueIndex = 0; ValueIndex < ValueCount; ++ValueIndex)
		{
		const string &Value = FT.GetValue(ValueIndex);
		const vector<double> &Fracts = ValueToFracts[ValueIndex];

		QuartsDouble Q;
		GetQuarts(Fracts, Q);

		Pf(g_fTsv, "mono=%s", Value.c_str());
		Pf(g_fTsv, "\tloq=%.4f", Q.LoQ);
		Pf(g_fTsv, "\tmeam=%.4f", Q.Avg);
		Pf(g_fTsv, "\tmedian=%.4f", Q.Med);
		Pf(g_fTsv, "\thiq=%.4f", Q.HiQ);
		Pf(g_fTsv, "\n");
		}

	CloseStdioFile(g_fTsv);
	}
