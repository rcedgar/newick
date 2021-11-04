#include "myutils.h"
#include "biparter.h"

void GetFractConfs(const TreeN &T, vector<double> &Confs);

static FILE *g_fTab;

static bool IsSubtreeConflict(const set<uint> &Subtree1, const set<uint> &Subtree2)
	{
	const uint N1 = SIZE(Subtree1);
	uint FoundIn2Count = 0;
	for (set<uint>::const_iterator p = Subtree1.begin();
	  p != Subtree1.end(); ++p)
		{
		uint Node = *p;
		if (Subtree2.find(Node) != Subtree2.end())
			++FoundIn2Count;
		}
	bool Conflict = (FoundIn2Count > 0 && FoundIn2Count < N1);
	return Conflict;
	}

void GetSubtreeVec(const TreeN &T, const vector<double> &FractConfs,
  const map<string, uint> &LabelToIndex, double MinConf,
  vector<uint> &HiConfNodes, vector<set<uint> > &SubtreeVec)
	{
	asserta(T.IsNormalized());

	SubtreeVec.clear();
	HiConfNodes.clear();

	const uint NodeCount = T.GetNodeCount();
	asserta(SIZE(FractConfs) == NodeCount);

	for (uint Node = 0; Node < NodeCount; ++Node)
		{
		double Conf = FractConfs[Node];
		if (Conf < MinConf)
			continue;
		if (T.IsLeaf(Node) || T.IsRoot(Node))
			continue;
		
		set<uint> LeafNodes;
		T.AppendSubtreeLeafNodes(Node, LeafNodes);

		set<uint> LabelIndexes;
		for (set<uint>::const_iterator p = LeafNodes.begin();
		  p != LeafNodes.end(); ++p)
			{
			uint LeafNodeIndex = *p;
			asserta(T.IsLeaf(LeafNodeIndex));
			const string &Label = T.GetLabel(LeafNodeIndex);
			if (!Label.empty())
				{
				map<string, uint>::const_iterator q =
				  LabelToIndex.find(Label);
				asserta(q != LabelToIndex.end());
				uint LabelIndex = q->second;
				LabelIndexes.insert(LabelIndex);
				}
			}

		HiConfNodes.push_back(Node);
		SubtreeVec.push_back(LabelIndexes);
		}
	}

// Search for conflicting edges with conf above threshold
void cmd_conf2()
	{
	const string &FileName1 = opt(conf2);
	const string &FileName2 = opt(tree2);
	const string &OutputFileName = opt(output);
	g_fTab = CreateStdioFile(OutputFileName);
	double MinConf = 0.9;
	if (optset_minconf)
		MinConf = opt(minconf);
	asserta(MinConf >= 0 && MinConf <= 1);

	TreeN T1;
	TreeN T2;
	T1.FromNewickFile(FileName1);
	T2.FromNewickFile(FileName2);

	asserta(T1.IsNormalized());
	asserta(T2.IsNormalized());

	map<string, uint> LabelToIndex;
	T1.GetSortedLeafLabelToIndex(LabelToIndex);

	for (map<string, uint>::const_iterator p = LabelToIndex.begin();
	  p != LabelToIndex.end(); ++p)
		{
		const string &Label = p->first;
		uint Node = T2.GetNodeByLabel(Label, false);
		if (Node == UINT_MAX)
			Die("Not found in tree2 >%s", Label.c_str());
		}

	vector<double> FractConfs1;
	vector<double> FractConfs2;
	GetFractConfs(T1, FractConfs1);
	GetFractConfs(T2, FractConfs2);

	vector<uint> HiConfNodes1;
	vector<uint> HiConfNodes2;

	vector<set<uint> > SubtreeVec1;
	vector<set<uint> > SubtreeVec2;

	GetSubtreeVec(T1, FractConfs1, LabelToIndex, MinConf,
	  HiConfNodes1, SubtreeVec1);
	GetSubtreeVec(T2, FractConfs2, LabelToIndex, MinConf,
	  HiConfNodes2, SubtreeVec2);

	const uint N1 = SIZE(HiConfNodes1);
	const uint N2 = SIZE(HiConfNodes2);
	asserta(SIZE(SubtreeVec1) == N1);
	asserta(SIZE(SubtreeVec2) == N2);

	for (uint i1 = 0; i1 < N1; ++i1)
		{
		uint Node1 = HiConfNodes1[i1];
		const set<uint> Subtree1 = SubtreeVec1[i1];
		for (uint i2 = 0; i2 < N2; ++i2)
			{
			uint Node2 = HiConfNodes2[i2];
			const set<uint> Subtree2 = SubtreeVec2[i2];

			bool Conflict12 = IsSubtreeConflict(Subtree1, Subtree2);
			bool Conflict21 = IsSubtreeConflict(Subtree2, Subtree1);
			
			if (Conflict12 || Conflict21)
				{
				uint Size1 = SIZE(Subtree1);
				uint Size2 = SIZE(Subtree2);

				double Conf1 = FractConfs1[Node1];
				double Conf2 = FractConfs2[Node2];

				Pf(g_fTab, "conflict");
				Pf(g_fTab, "\tnode1=%u", Node1);
				Pf(g_fTab, "\tsize1=%u", Size1);
				Pf(g_fTab, "\tconf1=%.3f", Conf1);
				Pf(g_fTab, "\tnode2=%u", Node2);
				Pf(g_fTab, "\tsize2=%u", Size2);
				Pf(g_fTab, "\tconf2=%.3f", Conf2);
				Pf(g_fTab, "\tconflict12=%c", yon(Conflict12));
				Pf(g_fTab, "\tconflict21=%c", yon(Conflict21));
				Pf(g_fTab, "\n");
				}
			}
		}

	CloseStdioFile(g_fTab);
	}
