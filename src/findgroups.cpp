#include "myutils.h"
#include "treen.h"
#include "sort.h"

#if 0

static double g_IdealUpperDist = 2.0;
static uint g_MinGroupSize = 10;
static const vector<double> *g_RootDists;

static double GetUpperLeafDist(TreeN &T, uint Node)
	{
	const vector<double> &RootDists = *g_RootDists;
	vector<uint> LeafNodes;
	T.GetSubtreeLeafNodes(Node, LeafNodes);
	asserta(Node < SIZE(RootDists));
	double ThisRootDist = RootDists[Node];
	vector<double> LeafDists;
	const uint N = SIZE(LeafNodes);
	for (uint i = 0; i < N; ++i)
		{
		uint Leaf = LeafNodes[i];
		asserta(Leaf < SIZE(RootDists));
		double d = RootDists[Leaf];
		LeafDists.push_back(d - ThisRootDist);
		}
	vector<uint> Order;
	QuickSortInPlace(LeafDists.data(), N);
	double UpperDist = LeafDists[3*N/4];
	return UpperDist;
	}

static double GetScore_Size(uint N)
	{
	double Score = double(N)/(N + 10);
	return Score;
	}

static double GetScore_UpperDist(double d)
	{
	const double ID = g_IdealUpperDist;
	const double ID4 = ID/4;

	double Err = 0;
	if (d < ID - ID4)
		Err = ID - ID4 - d;
	else if (d > ID - ID4)
		Err = d - (ID + ID4);
	double Score = 1.0 - Err/ID;
	if (Score < 0)
		Score = 0;
	return Score;
	}

static double GetScore_Length(double Length)
	{
	double Score = Length/(Length + 0.5);
	return Score;
	}

static double GetScore_Balance(uint NL, uint NR)
	{
	double MaxN = (double) max(NL, NR);
	double MinN = (double) min(NL, NR);

	double Score = (MaxN + 5.0)/(MinN + 5.0);
	return Score;
	}

static double GetScore(TreeN &T, uint Node)
	{
	double UpperDist = GetUpperLeafDist(T, Node);
	double Length = T.GetLength(Node);
	uint Left = T.GetLeft(Node);
	uint Right = T.GetRight(Node);
	uint NL = T.GetSubtreeLeafCount(Left);
	uint NR = T.GetSubtreeLeafCount(Left);
	uint N = NL + NR;
	asserta(N >= g_MinGroupSize);

	double Score_Size = GetScore_Size(N);
	double Score_UpperDist = GetScore_UpperDist(UpperDist);
	double Score_Length = GetScore_Length(Length);
	double Score_Balance = GetScore_Balance(NL, NR);

	double Score1 = (Score_Size + Score_Length + Score_Balance)/3;
	double Score = Score1*Score_UpperDist;

	return Score;
	}

static void DoTree(TreeN &T)
	{
	const uint NodeCount = T.GetNodeCount();
	vector<double> RootDists;
	T.GetRootDists(RootDists);
	g_RootDists = &RootDists;
	for (uint Node= 0; Node < NodeCount; ++Node)
		{
		if (T.IsLeaf(Node))
			continue;
		uint SubtreeLeafNodeCount = T.GetSubtreeLeafCount(Node);
		if (SubtreeLeafNodeCount < g_MinGroupSize)
			continue;
		double Score = GetScore(T, Node);

		string sScore;
		Ps(sScore, "%u=%.3g", Node, Score);
		T.UpdateLabel(Node, sScore);
		}
	}

void cmd_findgroups()
	{
	const string &InputFileName = opt(findgroups);
	const string &OutputFileName = opt(output);
	FILE *fOut = CreateStdioFile(OutputFileName);

	vector<TreeN *> Trees;
	TreesFromFile(InputFileName, Trees);
	const uint TreeCount = SIZE(Trees);
	for (uint TreeIndex = 0; TreeIndex < TreeCount; ++TreeIndex)
		{
		TreeN &T = *Trees[TreeIndex];
		DoTree(T);
		T.ToNewickFile(fOut);
		}
	CloseStdioFile(fOut);
	}
#else
// kind of a dead end
void cmd_findgroups() {}
#endif
