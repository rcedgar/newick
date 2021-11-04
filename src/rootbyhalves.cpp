#include "myutils.h"
#include "tree2.h"
#include "featuretable.h"

void RootByHalves(const Tree2 &UnrootedTree, Tree2 &RootedTree)
	{
	asserta(!UnrootedTree.IsRooted());
	RootedTree.FromTree(UnrootedTree);

	const uint LeafCount = RootedTree.GetLeafCount();
	const uint NodeCount = RootedTree.GetNodeCount();
	asserta(LeafCount >= 2);

	const uint TargetLeafCount = LeafCount/2;
	asserta(TargetLeafCount > 0);

	uint BestNode = UINT_MAX;
	uint BestDiff = UINT_MAX;
	uint BestLeafCount = 0;
	for (uint Node = 0; Node < NodeCount; ++Node)
		{
		uint SubtreeLeafCount = RootedTree.GetSubtreeLeafCount(Node);
		uint Diff = (SubtreeLeafCount > TargetLeafCount ?
		  SubtreeLeafCount - TargetLeafCount  :
		  TargetLeafCount  - SubtreeLeafCount);
		if (BestNode == UINT_MAX || Diff < BestDiff)
			{
			BestNode = Node;
			BestDiff = Diff;
			BestLeafCount = SubtreeLeafCount;
			}
		}

	asserta(BestNode < NodeCount);
	uint NbrNode = RootedTree.m_Nbrs1[BestNode];
	RootedTree.SetRoot(BestNode, NbrNode);
	}

void cmd_rootbyhalves()
	{
	const string &InputFileName = opt(rootbyhalves);


	Tree2 T;
	T.FromNewickFile(InputFileName);

	Tree2 RootedTree;
	RootByHalves(T, RootedTree);
	RootedTree.Ladderize(opt(right));
	RootedTree.ToNewickFile(opt(output));
	}
