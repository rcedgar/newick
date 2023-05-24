#include "myutils.h"
#include "treen.h"
#include "sort.h"

#if 0

/***
"Shrink" a tree by collapsing leaf nodes into subtrees
with maximum LCA-leaf distance approximately d, where d
is a parameter, ignoring outlier long branches which may
be artifacts.
***/

static double g_MaxUpperDist = 1.5;
static uint g_MinGroupSize = 5;
static uint g_MaxIters = 10;
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

static void DoTree(TreeN &T)
	{
//	const uint NodeCount = T.GetNodeCount();
	vector<double> RootDists;
	T.GetRootDists(RootDists);
	g_RootDists = &RootDists;
	vector<uint> SelectedNodes;
	set<uint> SelectedLeafNodes;
	vector<uint> LeafNodes;
	vector<uint> Nodes;
	for (uint Iter = 0; Iter < g_MaxIters; ++Iter)
		{
		uint BestNode = UINT_MAX;
		uint BestSize = UINT_MAX;
		T.GetNodes(Nodes);
		for (uint i = 0; i < SIZE(Nodes); ++i)
			{
			uint Node = Nodes[i];
			if (T.IsLeaf(Node))
				continue;
			if (T.IsRoot(Node))
				continue;
			uint Parent = T.GetParent(Node);
			if (T.IsRoot(Parent))
				continue;
			uint SubtreeLeafNodeCount = T.GetSubtreeLeafCount(Node);
			if (SubtreeLeafNodeCount < g_MinGroupSize)
				continue;
			double ud = GetUpperLeafDist(T, Node);
			T.GetSubtreeLeafNodes(Node, LeafNodes);
			uint Size = SIZE(LeafNodes);
			if (Size > g_MinGroupSize && 
			  ud <= g_MaxUpperDist)
				{
				bool Found = false;
				for (uint i = 0; i < Size; ++i)
					{
					uint LeafNode = LeafNodes[i];
					if (SelectedLeafNodes.find(LeafNode) !=
					  SelectedLeafNodes.end())
						{
						Found = true;
						break;
						}
					}
				if (Found)
					continue;

				if (BestNode == UINT_MAX || Size > BestSize)
					{
					BestNode = Node;
					BestSize = Size;
					}
				}
			}
		if (BestNode == UINT_MAX)
			break;
		SelectedNodes.push_back(BestNode);
		T.GetSubtreeLeafNodes(BestNode, LeafNodes);
		asserta(SIZE(LeafNodes) == BestSize);
		for (uint i = 0; i < BestSize; ++i)
			SelectedLeafNodes.insert(LeafNodes[i]);
		}
	for (uint i = 0; i < SIZE(SelectedNodes); ++i)
		{
		uint SelectedNode = SelectedNodes[i];
		string NewLabel;
		Ps(NewLabel, "SELECTED_%u", i);
		T.UpdateLabel(SelectedNode, NewLabel);

		const vector<uint> &ChildNodes = T.GetChildren(SelectedNode);
		for (uint i = 0; i < SIZE(ChildNodes); ++i)
			T.DeleteNode(ChildNodes[i], false);
		}
	T.SetDerived();
	T.CollapseUnary();

	T.GetNodes(Nodes);
	for (uint i = 0; i < SIZE(Nodes); ++i)
		{
		uint Node = Nodes[i];
		if (T.IsLeaf(Node))
			{
			string Label;
			T.GetLabel(Node, Label);
			if (Label == "")
				{
				Warning("Empty leaf label");
				Ps(Label, "_leaf_%u", Node);
				T.UpdateLabel(Node, Label);
				}
			continue;
			}

		string Label;
		T.GetLabel(Node, Label);
		if (Label != "")
			continue;
		string NewLabel;
		Ps(NewLabel, "_node_%u", Node);
		T.UpdateLabel(Node, NewLabel);
		}
	bool Rooted;
	asserta(T.IsBinary(Rooted));
	asserta(Rooted);
	}

void cmd_shrink()
	{
	const string &InputFileName = opt(shrink);
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
// Kind of a dead end
void cmd_shrink() {}
#endif
