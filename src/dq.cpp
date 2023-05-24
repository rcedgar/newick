#include "myutils.h"
#include "treex.h"
#include "featuretable.h"
#include "sort.h"
#include "quarts.h"

#if 0
void GetGroupLeafNodeSet(const TreeX &T, const FeatureTable &FT,
  uint ValueIndex, set<uint> &LeafNodeSet);
void GetConnComps(const vector<uint> &FromIndexes, const vector<uint> &ToIndexes,
  vector<vector<uint> > &CCs, bool ShowProgress);

static double GetNodeScore(const TreeX &T, uint Node)
	{
	if (!T.IsNode(Node))
		return -1;
	if (T.IsLeaf(Node))
		return -1;
	if (Node == T.m_Origin)
		return -1;
	uint n = T.GetSubtreeLeafCount_Rooted(Node);
	if (n < 10)
		return -1;
	double BF = T.GetBootFract(Node);
	double d = T.GetMedianLeafDist(Node);
	if (d < 1.5 || d > 2.5)
		return 0;
	if (BF < 0.75)
		return 0;
	double Score = BF*sqrt(n);
	return Score;
	}

static map<string, uint> g_LabelToIndex;
static vector<string> g_Labels;
static uint g_LabelCount;

static uint GetLabelIndex(const string &Label, bool Init)
	{
	if (Label == "")
		return UINT_MAX;
	uint Index = UINT_MAX;
	if (Init)
		{
		asserta(g_LabelToIndex.find(Label) == g_LabelToIndex.end());
		g_Labels.push_back(Label);
		Index = g_LabelCount++;
		g_LabelToIndex[Label] = Index;
		}
	else
		{
		map<string, uint>::const_iterator p = g_LabelToIndex.find(Label);
		asserta(p != g_LabelToIndex.end());
		Index = p->second;
		}
	return Index;
	}

static void AddTreeToIndex(const TreeX &T, uint TreeIndex)
	{
	const uint NodeCount = T.GetNodeIndexCount();
	for (uint Node = 0; Node < NodeCount; ++Node)
		{
		if (T.IsLeaf(Node))
			{
			const string &Label = T.GetLabel(Node);
			GetLabelIndex(Label, TreeIndex == 0);
			}
		}
	}

static map<pair<uint, uint>, uint> g_EdgeToCount;

static void AddEdges(const vector<string> &Labels)
	{
	vector<uint> LabelIndexes;
	const uint N = SIZE(Labels);
	for (uint i = 0; i < N; ++i)
		{
		uint LabelIndex = GetLabelIndex(Labels[i], false);
		if (LabelIndex != UINT_MAX)
			{
			asserta(LabelIndex < g_LabelCount);
			LabelIndexes.push_back(LabelIndex);
			}
		}
	
	const uint M = SIZE(LabelIndexes);
	for (uint i = 0; i < M; ++i)
		{
		uint ni = LabelIndexes[i];
		asserta(ni < g_LabelCount);
		for (uint j = i + 1; j < M; ++j)
			{
			uint nj = LabelIndexes[j];
			asserta(nj < g_LabelCount);

			uint nmin = min(ni, nj);
			uint nmax = max(ni, nj);
			pair<uint, uint> Edge(nmin, nmax);
			map<pair<uint, uint>, uint>::const_iterator p =
			  g_EdgeToCount.find(Edge);
			if (p == g_EdgeToCount.end())
				g_EdgeToCount[Edge] = 1;
			else
				g_EdgeToCount[Edge] += 1;
			}
		}
	ProgressLog("%s edges\n", IntToStr(g_EdgeToCount.size()));
	}

void cmd_dq()
	{
	const string &InputFileName = opt(dq);
	FILE *fTsv = CreateStdioFile(opt(tsvout));

	double MinTPFract = 0.5;
	if (optset_mintpfract)
		MinTPFract = opt(mintpfract);

	vector<TreeX *> Trees;
	TreesFromFile(InputFileName, Trees);
	const uint TreeCount = SIZE(Trees);

	FeatureTable FT;
	set<uint> LeafNodeSet;
	vector<double> Dists;
	for (uint TreeIndex = 0; TreeIndex < TreeCount; ++TreeIndex)
		{
		TreeX &T = *Trees[TreeIndex];
		AddTreeToIndex(T, TreeIndex);
		SetFeatureTable(T, FT);
		const uint ValueCount = FT.GetValueCount();
		T.GetSubtreeLeafDistsSorted_Rooted(T.m_Origin, Dists);
		QuartsDouble QD;
		GetQuarts(Dists, QD);
		Log("\nRoot median=%.3g mean=%.3g\n", QD.Med, QD.Avg);

		for (uint ValueIndex = 0; ValueIndex < ValueCount; ++ValueIndex)
			{
			GetGroupLeafNodeSet(T, FT, ValueIndex, LeafNodeSet);
			uint N = SIZE(LeafNodeSet);
			if (N < 2)
				continue;
			uint TP, FP, FN;
			uint BestFitNode = 
			  T.GetBestFitSubtree(LeafNodeSet, MinTPFract, TP, FP, FN);
			T.GetSubtreeLeafDistsSorted_Rooted(BestFitNode, Dists);
			QuartsDouble QD;
			GetQuarts(Dists, QD);

			const string &ValueName = FT.GetValue(ValueIndex);
			Log("\n%s node=%u N=%u TP=%u",
			  ValueName.c_str(), BestFitNode, N, TP);
			Log(" median=%.3g mean=%.3g\n", QD.Med, QD.Avg);
			}

		uint LeafCount = T.GetLeafCount();
		uint AssignedCount = 0;
		for (uint Iter = 0; Iter < 50; ++Iter)
			{
			uint TopNode = UINT_MAX;
			double TopScore = 1;
			for (uint Node = 0; Node < T.GetNodeIndexCount(); ++Node)
				{
				if (!T.IsNode(Node))
					continue;
				double Score = GetNodeScore(T, Node);
				if (Score > TopScore)
					{
					TopScore = Score;
					TopNode = Node;
					}
				}
			if (TopNode == UINT_MAX)
				break;

			vector<uint> LeafNodes;
			T.GetSubtreeLeafNodes_Rooted(TopNode, LeafNodes);
			uint Size = SIZE(LeafNodes);

			vector<string> SubtreeLeafLabels;
			map<string, uint> ValueToCount;
			bool AnyValues = false;
			for (uint i = 0; i < Size; ++i)
				{
				uint LeafNode = LeafNodes[i];
				const string &Label = T.GetLabel(LeafNode);
				SubtreeLeafLabels.push_back(Label);
				string Value;
				FT.GetValue_ByLabel(Label, Value, false);
				if (Value.empty())
					continue;
				AnyValues = true;
				if (ValueToCount.find(Value) == ValueToCount.end())
					ValueToCount[Value] = 1;
				else
					ValueToCount[Value] += 1;

				if (fTsv != 0)
					{
					FILE *f = fTsv;
					fprintf(f, "%u", TreeIndex);
					fprintf(f, "\t%u", Iter);
					fprintf(f, "\t%s", Label.c_str());
					fprintf(f, "\n");
					}
				}

			AddEdges(SubtreeLeafLabels);

			if (AnyValues)
				{
				vector<string> SubtreeValues;
				vector<uint> SubtreeValueCounts;
				CountMapToVecs(ValueToCount, SubtreeValues, SubtreeValueCounts);
				for (uint j = 0; j < SIZE(SubtreeValues); ++j)
					Log(" iter %u value %s count %u\n",
					  Iter, SubtreeValues[j], SubtreeValueCounts[j]);
				}
			else
				Log(" iter %u NOVALUES\n", Iter);

			AssignedCount += Size;
			double Pct = GetPct(AssignedCount, LeafCount);
			ProgressLog("Iter %u DQ%u(%.3g) size=%u total=%u / %u (%.1f%%)\n",
			  Iter, TopNode, TopScore, Size, AssignedCount, LeafCount, Pct);
			T.DeleteSubtree(TopNode, "", false);
			T.Validate();
			asserta(T.IsRootedBinary());
			if (Pct > 95)
				break;
			}
		Log("\n");
		}

	const uint EdgeCount = SIZE(g_EdgeToCount);
	const uint MinCount = TreeCount/2 + 1;
	vector<uint> Froms;
	vector<uint> Tos;
	for (map<pair<uint, uint>, uint>::const_iterator p =
	  g_EdgeToCount.begin(); p != g_EdgeToCount.end(); ++p)
		{
		uint Count = p->second;
		if (Count >= MinCount)
			{
			const pair<uint, uint> &Edge = p->first;
			uint From = Edge.first;
			uint To = Edge.second;
			asserta(From < g_LabelCount);
			asserta(To < g_LabelCount);
			Froms.push_back(From);
			Tos.push_back(To);
			}
		}

	uint SelectedEdgeCount = SIZE(Froms);
	ProgressLog("%u / %u edges selected\n",
	  SelectedEdgeCount, EdgeCount);

	vector<vector<uint> > CCs;
	GetConnComps(Froms, Tos, CCs, true);
	const uint CCCount = SIZE(CCs);

	map<uint, uint> CCToSize;
	for (uint i = 0; i < CCCount; ++i)
		{
		uint Size = SIZE(CCs[i]);
		CCToSize[i] = Size;
		}

	vector<uint> CCIndexes;
	vector<uint> CCSizes;
	CountMapToVecs(CCToSize, CCIndexes, CCSizes);

	for (uint k = 0; k < CCCount; ++k)
		{
		uint CCIndex = CCIndexes[k];
		uint Size = CCSizes[k];
		const vector<uint> &LabelIndexes = CCs[CCIndex];

		Log("\n==== CC %u (size %u)\n", k, Size);
		for (uint j = 0; j < Size; ++j)
			{
			uint LabelIndex = LabelIndexes[j];
			asserta(LabelIndex < SIZE(g_Labels));
			const string &Label = g_Labels[LabelIndex];
			Log("  >%s\n", Label.c_str());
			}
		}

	CloseStdioFile(fTsv);
	}

#else
void cmd_dq() { Die("dq"); }
#endif
