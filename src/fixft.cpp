#include "myutils.h"
#include "newickparser2.h"
#include "tree2.h"

uint NewickParser2::FixFT()
	{
	if (IsBinary())
		return 0;

	vector<vector<uint> > Edges;
	GetNonParentEdges(Edges);
	const uint NodeCount = GetNodeCount();
	asserta(SIZE(Edges) == NodeCount);
	asserta(m_Root < NodeCount);
	uint DupeCount = 0;

	vector<uint> NewParents = m_Parents;
	vector<string> NewLabels = m_Labels;
	vector<bool> NewIsLeafs = m_IsLeafs;
	vector<double> NewLengths = m_Lengths;

	for (uint Node = 0; Node < NodeCount; ++Node)
		{
		const vector<uint> &NodeEdges = Edges[Node];
		uint ChildCount = SIZE(NodeEdges);
		if (ChildCount == 0)
			continue;
		if (ChildCount == 1)
			Die("Unary node");
		if (Node == m_Root)
			{
			if (ChildCount != 2 && ChildCount != 3)
				Die("Root is non-binary");
			}
		else
			{
			if (ChildCount > 2)
				{
				DupeCount += ChildCount - 1;
				for (uint i = 0; i < ChildCount; ++i)
					{
					uint Edge = NodeEdges[i];
					bool IsLeaf = m_IsLeafs[Edge];
					double Length = m_Lengths[Edge];
					if (!IsLeaf)
						Die("%u-ary node has non-leaf child", ChildCount);
					if (Length != 0)
						Die("%u-ary node length %.3g", ChildCount, Length);
					}

				uint FirstNewNode = SIZE(NewLabels);
				for (uint i = 0; i + 2 < ChildCount; ++i)
					{
					uint ChildNode = NodeEdges[i];
					uint NewNode = FirstNewNode + i;

					uint NewLeftChild = (i == 0 ? NodeEdges[0] : NewNode - 1);
					uint NewRightChild = NodeEdges[i+1];
					uint NewParent = (i + 3 == ChildCount ? Node : NewNode + 1);

					if (i == 0)
						assert(m_Parents[NewLeftChild] == Node);
					assert(m_Parents[NewRightChild] == Node);

					NewParents[NewLeftChild] = NewNode;
					NewParents[NewRightChild] = NewNode;

					NewLabels.push_back("");
					NewParents.push_back(NewParent);
					NewLengths.push_back(0);
					NewIsLeafs.push_back(false);

					m_Labels = NewLabels;
					m_IsLeafs = NewIsLeafs;
					m_Parents = NewParents;
					m_Lengths = NewLengths;
					}
//				Validate();
				}
			}
		}
	Validate();
	return DupeCount;
	}

void cmd_fixft()
	{
	const string &InputFileName = opt(fixft);
	const string &OutputFileName = opt(output);

	NewickParser2 NP;
	NP.FromFile(InputFileName);
#if 0
	vector<vector<uint> > Edges;
	NP.GetNonParentEdges(Edges);
	const uint NodeCount = NP.GetNodeCount();
	asserta(SIZE(Edges) == NodeCount);
	asserta(NP.m_Root < NodeCount);
	uint DupeCount = 0;

	vector<uint> NewParents = NP.m_Parents;
	vector<string> NewLabels = NP.m_Labels;
	vector<bool> NewIsLeafs = NP.m_IsLeafs;
	vector<double> NewLengths = NP.m_Lengths;

	for (uint Node = 0; Node < NodeCount; ++Node)
		{
		const vector<uint> &NodeEdges = Edges[Node];
		uint ChildCount = SIZE(NodeEdges);
		if (ChildCount == 0)
			continue;
		if (ChildCount == 1)
			Die("Unary node");
		if (Node == NP.m_Root)
			{
			if (ChildCount != 2 && ChildCount != 3)
				Die("Root is non-binary");
			}
		else
			{
			if (ChildCount > 2)
				{
				DupeCount += ChildCount - 1;
				for (uint i = 0; i < ChildCount; ++i)
					{
					uint Edge = NodeEdges[i];
					bool IsLeaf = NP.m_IsLeafs[Edge];
					double Length = NP.m_Lengths[Edge];
					if (!IsLeaf)
						Die("%u-ary node has non-leaf child", ChildCount);
					if (Length != 0)
						Die("%u-ary node length %.3g", ChildCount, Length);
					}

				uint FirstNewNode = SIZE(NewLabels);
				for (uint i = 0; i + 2 < ChildCount; ++i)
					{
					uint ChildNode = NodeEdges[i];
					uint NewNode = FirstNewNode + i;

					uint NewLeftChild = (i == 0 ? NodeEdges[0] : NewNode - 1);
					uint NewRightChild = NodeEdges[i+1];
					uint NewParent = (i + 3 == ChildCount ? Node : NewNode + 1);

					if (i == 0)
						assert(NP.m_Parents[NewLeftChild] == Node);
					assert(NP.m_Parents[NewRightChild] == Node);

					NewParents[NewLeftChild] = NewNode;
					NewParents[NewRightChild] = NewNode;

					NewLabels.push_back("");
					NewParents.push_back(NewParent);
					NewLengths.push_back(0);
					NewIsLeafs.push_back(false);

					NP.m_Labels = NewLabels;
					NP.m_IsLeafs = NewIsLeafs;
					NP.m_Parents = NewParents;
					NP.m_Lengths = NewLengths;
					}
				NP.Validate();
				}
			}
		}
	NP.Validate();
#endif // 0

	uint DupeCount = NP.FixFT();
	ProgressLog("%u dupes resolved\n", DupeCount);

	Tree2 T;
	T.FromNewickTree(NP);
	T.ToNewickFile(OutputFileName);
	}
