#include "myutils.h"
#include "featuretable.h"
#include "treen.h"
#include "quarts.h"
#include "sort.h"

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
static uint g_OutgroupValueIndex = UINT_MAX;

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

static void GetValueCounts(const TreeN &T, uint Node, 
  uint ValueCount, const vector<uint> &NodeToValueIndex,
  vector<uint> &ValueCounts)
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

static void GetErrs(const TreeN &T, uint Node, uint ValueIndex,
  const vector<uint> &NodeToValueIndex,
  vector<uint> &FPLeafNodeIndexes, vector<uint> &FNLeafNodeIndexes)
	{
	set<uint> SubtreeLeafNodes;
	T.AppendSubtreeLeafNodes(Node, SubtreeLeafNodes);

	for (set<uint>::const_iterator p = SubtreeLeafNodes.begin();
	  p != SubtreeLeafNodes.end(); ++p)
		{
		uint Node2 = *p;
		asserta(Node2 < SIZE(NodeToValueIndex));
		uint ValueIndex2 = NodeToValueIndex[Node2];
		if (ValueIndex2 == UINT_MAX)
			continue;
		if (ValueIndex2 != ValueIndex)
			FPLeafNodeIndexes.push_back(Node2);
		}

	set<uint> ValueLeafNodes;
	const uint N = SIZE(NodeToValueIndex);
	for (uint Node2 = 0; Node2 < N; ++Node2)
		{
		if (NodeToValueIndex[Node2] == ValueIndex)
			{
			ValueLeafNodes.insert(Node2);

			if (SubtreeLeafNodes.find(Node2) == 
			  SubtreeLeafNodes.end())
				FNLeafNodeIndexes.push_back(Node2);
			}
		}
	}

static uint GetBestFitSubtree(const TreeN &T, uint ValueCount,
  const vector<uint> &NodeToValueIndex, uint ValueIndex, uint &ErrCount,
  vector<uint> &FPLeafNodeIndexes, vector<uint> &FNLeafNodeIndexes)
	{
	ErrCount = UINT_MAX;
	FPLeafNodeIndexes.clear();
	FNLeafNodeIndexes.clear();

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
		vector<uint> FPVec;
		vector<uint> FNVec;
		GetErrs(T, Node, ValueIndex, NodeToValueIndex, FPVec, FNVec);
		uint FP = SIZE(FPVec);
		uint FN = SIZE(FNVec);
		uint Errs = FP + FN;
		if (Errs < BestErrs)
			{
			BestNode = Node;
			BestErrs = Errs;
			}
		}

	GetErrs(T, BestNode, ValueIndex, NodeToValueIndex,
	  FPLeafNodeIndexes, FNLeafNodeIndexes);

	uint FPSz = SIZE(FPLeafNodeIndexes);
	uint FNSz = SIZE(FNLeafNodeIndexes);
	uint Errs2 = FPSz + FNSz;
	asserta(BestErrs == Errs2);
	ErrCount = BestErrs;
	return BestNode;
	}

static void OutputFXVec(FILE *f, uint TreeCount, const vector<string> &FXs, bool IsFP)
	{
	if (f == 0)
		return;
	if (FXs.empty())
		return;
	map<string, uint> CountMap;
	VecToCountMap(FXs, CountMap);
	vector<string> Labels;
	vector<uint> Counts;
	CountMapToVecs(CountMap, Labels, Counts);
	const uint N = SIZE(Labels);
	for (uint i = 0; i < N; ++i)
		{
		const string &Label = Labels[i];
		uint n = Counts[i];
		fprintf(f, "\t%s:%s=%u/%u", IsFP ? "FP" : "FN",
		  Label.c_str(), n, TreeCount);
		}
	}

static void DoMono(FILE *f, const TreeN &T, const FeatureTable &FT,
  vector<uint> &SumTPs, vector<uint> &SumFPs, vector<uint> &SumFNs,
  vector<uint> &ValueIndexToSumSize,
  vector<vector<string> > &TotalValueIndexToFPs,
  vector<vector<string> > &TotalValueIndexToFNs)
	{
	const uint NodeCount = T.GetNodeCount();
	const uint ValueCount = FT.GetValueCount();

	vector<uint> NodeToValueIndex;
	GetNodeToValueIndex(T, FT, NodeToValueIndex);
	asserta(SIZE(NodeToValueIndex) == NodeCount);

	vector<uint> ValueTotals(ValueCount, 0);
	for (uint Node = 0; Node < NodeCount; ++Node)
		{
		uint ValueIndex = NodeToValueIndex[Node];
		if (ValueIndex != UINT_MAX && ValueIndex != g_OutgroupValueIndex)
			{
			asserta(ValueIndex < ValueCount);
			++ValueTotals[ValueIndex];
			}
		}

	vector<vector<string> > ValueIndexToFPs(ValueCount);
	vector<vector<string> > ValueIndexToFNs(ValueCount);
	for (uint ValueIndex = 0; ValueIndex < ValueCount; ++ValueIndex)
		{
		if (ValueIndex == g_OutgroupValueIndex)
			continue;

		const string &Value = FT.GetValue(ValueIndex);
		
		vector<uint> FPLeafNodeIndexes;
		vector<uint> FNLeafNodeIndexes;
		uint ErrCount = UINT_MAX;
		uint BFS = GetBestFitSubtree(T, ValueCount, NodeToValueIndex, ValueIndex,
		  ErrCount, FPLeafNodeIndexes, FNLeafNodeIndexes);
		asserta(BFS != UINT_MAX);
		const uint FP = SIZE(FPLeafNodeIndexes);
		const uint FN = SIZE(FNLeafNodeIndexes);
		asserta(FP + FN == ErrCount);
		for (uint i = 0; i < FP; ++i)
			{
			uint LeafNode = FPLeafNodeIndexes[i];
			const string &Label = T.GetLabel(LeafNode);
			ValueIndexToFPs[ValueIndex].push_back(Label);
			TotalValueIndexToFPs[ValueIndex].push_back(Label);
			}
		for (uint i = 0; i < FN; ++i)
			{
			uint LeafNode = FNLeafNodeIndexes[i];
			const string &Label = T.GetLabel(LeafNode);
			ValueIndexToFNs[ValueIndex].push_back(Label);
			TotalValueIndexToFNs[ValueIndex].push_back(Label);
			}

		vector<uint> ValueCounts;
		GetValueCounts(T, BFS, ValueCount, NodeToValueIndex, ValueCounts);

		asserta(SIZE(ValueCounts) == ValueCount);
		uint SubtreeSize = 0;
		for (uint ValueIndex2 = 0; ValueIndex2 < ValueCount; ++ValueIndex2)
			{
			if (ValueIndex2 == g_OutgroupValueIndex)
				continue;
			SubtreeSize += ValueCounts[ValueIndex2];
			}

		uint ValueSize = ValueTotals[ValueIndex];
		if (ValueSize == 0)
			continue;
		ValueIndexToSumSize[ValueIndex] += ValueSize;

		uint TP =  ValueCounts[ValueIndex];
		asserta(TP <= ValueSize);
		asserta(TP <= SubtreeSize);
		uint FN2 = ValueSize - TP;
		uint FP2 = SubtreeSize - TP;
		asserta(FN2 == FN);
		asserta(FP2 == FP);

		SumTPs[ValueIndex] += TP;
		SumFPs[ValueIndex] += FP;
		SumFNs[ValueIndex] += FN;

		Pf(f, "tree=%u", g_TreeIndex);
		Pf(f, "\tvalue=%s", Value.c_str());
		Pf(f, "\tSz=%u", ValueSize);
		Pf(f, "\tTP=%u", TP);
		Pf(f, "\tFN=%u", FN);
		Pf(f, "\tFP=%u", FP);
		OutputFXVec(f, 1, ValueIndexToFPs[ValueIndex], true);
		OutputFXVec(f, 1, ValueIndexToFNs[ValueIndex], false);
		Pf(f, "\n");
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

	if (optset_outgroup)
		{
		const string &OutgroupLabel = opt(outgroup);
		g_OutgroupValueIndex = FT.GetValueIndex(OutgroupLabel);
		}

	g_fTsv = CreateStdioFile(opt(output));

	const uint TreeCount = SIZE(Trees);

	vector<uint> SumTPs(ValueCount);
	vector<uint> SumFPs(ValueCount);
	vector<uint> SumFNs(ValueCount);
	vector<vector<string> > ValueIndexToFPs(ValueCount);
	vector<vector<string> > ValueIndexToFNs(ValueCount);
	vector<uint> ValueIndexToSumSize(ValueCount);

	for (g_TreeIndex = 0; g_TreeIndex < TreeCount; ++g_TreeIndex)
		{
		TreeN &T = *Trees[g_TreeIndex];
		asserta(T.IsNormalized());
		const uint NodeCount = T.GetNodeCount();

		DoMono(g_fTsv, T, FT, SumTPs, SumFPs, SumFNs,
		  ValueIndexToSumSize, ValueIndexToFPs, ValueIndexToFNs);
		}


	Pf(g_fTsv, "Sz\tTP\tFP\tFN\tValue\n");

	double TotalTPPct = 0;
	double TotalErrPct = 0;
	uint TotalN = 0;
	for (uint ValueIndex = 0; ValueIndex < ValueCount; ++ValueIndex)
		{
		if (ValueIndex == g_OutgroupValueIndex)
			continue;
		const string &Value = FT.GetValue(ValueIndex);

		double TP = SumTPs[ValueIndex];
		double FP = SumFPs[ValueIndex];
		double FN = SumFNs[ValueIndex];
		double Sum = TP + FP + FN;
		if (Sum == 0)
			continue;

		double PctTP = TP*100.0/Sum;
		double PctFP = FP*100.0/Sum;
		double PctFN = FN*100.0/Sum;


		double AvgSize = double(ValueIndexToSumSize[ValueIndex])/TreeCount;
		if (AvgSize >= 2)
			{
			TotalTPPct += PctTP;
			TotalErrPct += PctFP + PctFN;
			++TotalN;
			}

		asserta(feq(PctTP + PctFP + PctFN, 100.0));

		Pf(g_fTsv, "%.1f", AvgSize);
		Pf(g_fTsv, "\t%.1f", PctTP);
		Pf(g_fTsv, "\t%.1f", PctFP);
		Pf(g_fTsv, "\t%.1f", PctFN);
		Pf(g_fTsv, "\t%s\n", Value.c_str());
		}

	for (uint ValueIndex = 0; ValueIndex < ValueCount; ++ValueIndex)
		{
		if (ValueIndex == g_OutgroupValueIndex)
			continue;
		const string &Value = FT.GetValue(ValueIndex);

		double TP = SumTPs[ValueIndex];
		double FP = SumFPs[ValueIndex];
		double FN = SumFNs[ValueIndex];
		double Sum = TP + FP + FN;
		if (Sum == 0)
			continue;

		double PctFP = FP*100.0/Sum;
		double PctFN = FN*100.0/Sum;

		Pf(g_fTsv, "valuefp=%s", Value.c_str());
		Pf(g_fTsv, "\tFP=%.1f", PctFP);
		OutputFXVec(g_fTsv, TreeCount, ValueIndexToFPs[ValueIndex], true);
		Pf(g_fTsv, "\n");

		Pf(g_fTsv, "valuefn=%s", Value.c_str());
		Pf(g_fTsv, "\tFN=%.1f", PctFN);
		OutputFXVec(g_fTsv, TreeCount, ValueIndexToFNs[ValueIndex], false);
		Pf(g_fTsv, "\n");
		}

	CloseStdioFile(g_fTsv);

	ProgressLog("Total TP %.1f%%, Err %.1f%%\n",
	  TotalTPPct/TotalN, TotalErrPct/TotalN);
	}
