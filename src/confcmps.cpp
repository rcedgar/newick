#include "myutils.h"
#include "biparter.h"

void GetFractConfs(const TreeN &T, vector<double> &Confs);
void GetSubtreeVec(const TreeN &T, const vector<double> &FractConfs,
  const map<string, uint> &LabelToIndex, double MinConf,
  vector<uint> &HiConfNodes, vector<set<uint> > &SubtreeVec);
void TreesFromFile(const string &FileName, vector<TreeN *> &Trees);

static FILE *g_fTab;

static void CheckSubtreeConflict(const set<uint> &Subtree1,
  const set<uint> &Subtree2, uint &Found12, uint &Found21, 
  bool &Overlap, bool &Conflict)
	{
	Overlap = false;
	Conflict = false;

	const uint N1 = SIZE(Subtree1);
	const uint N2 = SIZE(Subtree2);
	Found12 = 0;
	Found21 = 0;
	for (set<uint>::const_iterator p = Subtree1.begin();
	  p != Subtree1.end(); ++p)
		{
		uint Node = *p;
		if (Subtree2.find(Node) != Subtree2.end())
			++Found12;
		}
	for (set<uint>::const_iterator p = Subtree2.begin();
	  p != Subtree2.end(); ++p)
		{
		uint Node = *p;
		if (Subtree1.find(Node) != Subtree1.end())
			++Found21;
		}

	Overlap = (Found12 > 0 || Found21 > 0);
	if (!Overlap)
		return;

	if (N1 <= N2)
		{
		if (Found12 < N1)
			Conflict = true;
		}
	else if (N2 < N1)
		{
		if (Found21 < N2)
			Conflict = true;
		}
	}

static void Cmp1(
  const vector<TreeN *> &Trees,
  const vector<string> &Labels,
  const vector<vector<double> > &FractConfsVec,
  const vector<vector<uint> > &HiConfNodesVec,
  const vector<vector<set<uint> > > &SubtreeVecVec,
  uint TreeIndex1, uint NodeIndex1)
	{
	const uint TreeCount = SIZE(Trees);
	const uint LabelCount = SIZE(Labels);

	asserta(TreeIndex1 < TreeCount);
	asserta(TreeIndex1 < SIZE(FractConfsVec));
	asserta(TreeIndex1 < SIZE(HiConfNodesVec));
	asserta(TreeIndex1 < SIZE(SubtreeVecVec));

	const TreeN &T1 = *Trees[TreeIndex1];

	const vector<double> &FractConfs1 = FractConfsVec[TreeIndex1];
	const vector<uint> &HiConfNodes1 = HiConfNodesVec[TreeIndex1];
	const vector<set<uint> > &SubtreeVec1 = SubtreeVecVec[TreeIndex1];

	uint Index1 = UINT_MAX;
	for (uint i = 0; i < SIZE(HiConfNodes1); ++i)
		{
		if (HiConfNodes1[i] == NodeIndex1)
			{
			Index1 = i;
			break;
			}
		}
	asserta(Index1 != UINT_MAX);

	const uint HiConfNodeCount1 = SIZE(HiConfNodes1);
	asserta(SIZE(SubtreeVec1) == HiConfNodeCount1);
	const set<uint> &Subtree1 = SubtreeVec1[Index1];
	Log("Subtree1(%u,%u) label %s\n",
	  NodeIndex1, Index1, T1.GetLabel(NodeIndex1).c_str());
	for (set<uint>::const_iterator p = Subtree1.begin(); p != Subtree1.end(); ++p)
		{
		uint LabelIndex = *p;
		asserta(LabelIndex < LabelCount);
		Log("  [%5u]  %s\n", LabelIndex, Labels[LabelIndex].c_str());
		}

	uint OverlapCount = 0;
	uint ConflictCount = 0;
	for (uint TreeIndex2 = 0; TreeIndex2 < TreeCount; ++TreeIndex2)
		{
		ProgressStep(TreeIndex2, TreeCount, "Processing");

		if (TreeIndex2 == TreeIndex1)
			continue;

		const TreeN &T2 = *Trees[TreeIndex2];
		const vector<double> &FractConfs2 = FractConfsVec[TreeIndex2];
		const vector<uint> &HiConfNodes2 = HiConfNodesVec[TreeIndex2];
		const vector<set<uint> > &SubtreeVec2 = SubtreeVecVec[TreeIndex2];

		const uint HiConfNodeCount2 = SIZE(HiConfNodes2);
		asserta(SIZE(SubtreeVec2) == HiConfNodeCount2);
		for (uint Index2 = 0; Index2 < HiConfNodeCount2;
			++Index2)
			{
			uint NodeIndex2 = HiConfNodes2[Index2];
			const set<uint> &Subtree2 = SubtreeVec2[Index2];

			bool Overlap;
			bool Conflict;
			uint Found12 = 0;
			uint Found21 = 0;
			CheckSubtreeConflict(Subtree1, Subtree2,
			  Found12, Found21, Overlap, Conflict);
			if (Overlap)
				++OverlapCount;
			if (Conflict)
				++ConflictCount;
			if (!Overlap)
				continue;

			Log("\n");
			Log("Tree %u node %u,%u label %s size %u,%u found %u,%u conflict %c\n",
			  TreeIndex2, NodeIndex2, Index2,
			  T2.GetLabel(NodeIndex2).c_str(), 
			  SIZE(Subtree1), SIZE(Subtree2),
			  Found12, Found21,
			  yon(Conflict));
			for (set<uint>::const_iterator p = Subtree2.begin();
			  p != Subtree2.end(); ++p)
				{
				uint LabelIndex = *p;
				asserta(LabelIndex < LabelCount);
				Log("  [%5u]  %s\n", LabelIndex, Labels[LabelIndex].c_str());
				}
			}
		}

	double Fract = double(ConflictCount)/OverlapCount;
	Log("CMP1 Tree %u node %u overlaps %u conflicts %u fract %.4f\n",
		TreeIndex1, NodeIndex1, OverlapCount, ConflictCount, Fract);
	}

void cmd_confcmps()
	{
	const string &TreesFileName = opt(confcmps);
	const string &OutputFileName = opt(output);
	g_fTab = CreateStdioFile(OutputFileName);
	double MinConf = 0.9;
	if (optset_minconf)
		MinConf = opt(minconf);
	asserta(MinConf >= 0 && MinConf <= 1);

	vector<TreeN *> Trees;
	TreesFromFile(TreesFileName, Trees);
	const uint TreeCount = SIZE(Trees);
	asserta(TreeCount > 0);

	const TreeN &T0 = *Trees[0];
	vector<string> Labels;
	map<string, uint> LabelToIndex;
	T0.GetSortedLeafLabels(Labels);
	T0.GetSortedLeafLabelToIndex(LabelToIndex);

	vector<vector<set<uint> > > SubtreeVecVec(TreeCount);
	vector<vector<double> > FractConfsVec(TreeCount);
	vector<vector<uint> > HiConfNodesVec(TreeCount);

	for (uint TreeIndex = 0; TreeIndex < TreeCount; ++TreeIndex)
		{
		ProgressStep(TreeIndex, TreeCount, "Processing");
		const TreeN &T = *Trees[TreeIndex];

		vector<double> &FractConfs = FractConfsVec[TreeIndex];
		vector<uint> &HiConfNodes = HiConfNodesVec[TreeIndex];
		vector<set<uint> > &SubtreeVec = SubtreeVecVec[TreeIndex];

		GetFractConfs(T, FractConfs);
		GetSubtreeVec(T, FractConfs, LabelToIndex, MinConf,
		  HiConfNodes, SubtreeVec);
		}

	if (optset_treeix || optset_nodeix)
		{
		asserta(optset_treeix && optset_nodeix);
		uint TreeIx = opt(treeix);
		uint NodeIx = opt(nodeix);
		Cmp1(Trees, Labels, FractConfsVec, HiConfNodesVec,
		  SubtreeVecVec, TreeIx, NodeIx);
		return;
		}

	double SumFract = 0;
	uint PairCount = 0;
	for (uint TreeIndex1 = 0; TreeIndex1 < TreeCount; ++TreeIndex1)
		{
		const TreeN &T1 = *Trees[TreeIndex1];
		vector<double> &FractConfs1 = FractConfsVec[TreeIndex1];
		vector<uint> &HiConfNodes1 = HiConfNodesVec[TreeIndex1];
		vector<set<uint> > &SubtreeVec1 = SubtreeVecVec[TreeIndex1];

		const uint HiConfNodeCount1 = SIZE(HiConfNodes1);
		asserta(SIZE(SubtreeVec1) == HiConfNodeCount1);

		for (uint Index1 = 0; Index1 < HiConfNodeCount1;
		  ++Index1)
			{
			uint NodeIndex1 = HiConfNodes1[Index1];
			const set<uint> &Subtree1 = SubtreeVec1[Index1];

			uint OverlapCount = 0;
			uint ConflictCount = 0;
			for (uint TreeIndex2 = 0; TreeIndex2 < TreeCount; ++TreeIndex2)
				{
				if (TreeIndex2 == TreeIndex1)
					continue;

				const TreeN &T2 = *Trees[TreeIndex2];
				vector<double> &FractConfs2 = FractConfsVec[TreeIndex2];
				vector<uint> &HiConfNodes2 = HiConfNodesVec[TreeIndex2];
				vector<set<uint> > &SubtreeVec2 = SubtreeVecVec[TreeIndex2];

				const uint HiConfNodeCount2 = SIZE(HiConfNodes2);
				asserta(SIZE(SubtreeVec2) == HiConfNodeCount2);
				for (uint Index2 = 0; Index2 < HiConfNodeCount2;
				  ++Index2)
					{
					uint NodeIndex2 = HiConfNodes2[Index2];
					const set<uint> &Subtree2 = SubtreeVec2[Index2];

					bool Overlap;
					bool Conflict;
					uint Found12 = 0;
					uint Found21 = 0;
					CheckSubtreeConflict(Subtree1, Subtree2, 
					  Found12, Found21, Overlap, Conflict);
					if (Overlap)
						++OverlapCount;
					if (Conflict)
						++ConflictCount;
					}
				}

			asserta(ConflictCount <= OverlapCount);
			double Fract = 0;
			if (ConflictCount > 0)
				Fract = double(ConflictCount)/OverlapCount;
			++PairCount;
			SumFract += Fract;
			uint PathLength = T1.GetPathToRootLength(NodeIndex1);
			if (g_fTab != 0)
				{
				FILE *f = g_fTab;
				uint Size = SIZE(Subtree1);

				Pf(f, "tree=%u", TreeIndex1);
				Pf(f, "\tnode=%u", NodeIndex1);
				Pf(f, "\tsize=%u", Size);
				Pf(f, "\tPL=%u", PathLength);
				Pf(f, "\toverlaps=%u", OverlapCount);
				Pf(f, "\tconflicts=%u", ConflictCount);
				Pf(f, "\tfract=%.4f", Fract);
				Pf(f, "\n");
				}
			//Log("Tree %u node %u size %u PL %u overlaps %u conflicts %u fract %.4f\n",
			//  TreeIndex1, NodeIndex1, SIZE(Subtree1), PathLength, OverlapCount, ConflictCount, Fract);
			}
		}

	ProgressLog("Mean fract %.4f\n", SumFract/PairCount);
	CloseStdioFile(g_fTab);
	}
