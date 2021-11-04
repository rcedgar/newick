#include "myutils.h"
#include "newicktree.h"

void NewickTree::LogMe() const
	{
	const uint NodeCount = GetNodeCount();
	asserta(SIZE(m_Parents) == NodeCount);
	asserta(SIZE(m_Labels) == NodeCount);
	asserta(SIZE(m_Lengths) == NodeCount);

	Log("\n");
	Log("%u nodes\n", NodeCount);
	Log("      Node      Parent      Length  Type  Children  Label\n");
	//   1234567890  1234567890  1234567890  1234  12345678
	vector<vector<uint> > Edges;
	GetNonParentEdges(Edges);
	for (uint Node = 0; Node < NodeCount; ++Node)
		{
		uint Parent = m_Parents[Node];
		const string &Label = m_Labels[Node];
		double Length = m_Lengths[Node];
		bool IsLeaf = m_IsLeafs[Node];
		const vector<uint> &NodeEdges = Edges[Node];
		uint ChildCount = SIZE(NodeEdges);

		Log("%10u", Node);
		if (Parent == UINT_MAX)
			Log("  %10.10s", "*");
		else
			Log("  %10u", Parent);
		if (Length == MISSING_LENGTH)
			Log("  %10.10s", "*");
		else
			Log("  %10.4g", Length);
		if (Node == m_Root)
			Log("  %4.4s", "Root");
		else
			Log("  %4.4s", IsLeaf ? "Leaf" : "Int");
		if (!IsLeaf)
			Log("  %8u", ChildCount);
		else
			Log("  %8.8s", "");
		Log("  %s", Label.c_str());
		Log("\n");
		}
	}

uint NewickTree::GetLeafCount() const
	{
	const uint NodeCount = GetNodeCount();
	uint LeafCount = 0;
	for (uint Node = 0; Node < NodeCount; ++Node)
		{
		bool IsLeaf = m_IsLeafs[Node];
		if (IsLeaf)
			++LeafCount;
		}
	return LeafCount;
	}

void NewickTree::Validate() const
	{
	const uint NodeCount = GetNodeCount();
	asserta(SIZE(m_Parents) == NodeCount);
	asserta(SIZE(m_Labels) == NodeCount);
	asserta(SIZE(m_Lengths) == NodeCount);
	asserta(SIZE(m_Labels) == NodeCount);
	asserta(SIZE(m_IsLeafs) == NodeCount);
	asserta(m_Root < NodeCount);

// Exactly one root node
// (If actually unrooted, arbitrary choice)
	bool RootFound = false;

// Every node except leaves must have at least one child
	vector<bool> HasChild(NodeCount, false);
	for (uint Node = 0; Node < NodeCount; ++Node)
		{
		uint Parent = m_Parents[Node];
		if (Parent == UINT_MAX)
			{
			asserta(!RootFound);
			asserta(Node == m_Root);
			RootFound = true;
			}
		else
			{
			asserta(Parent < NodeCount);
			HasChild[Parent] = true;
			}
		}

	for (uint Node = 0; Node < NodeCount; ++Node)
		{
		bool IsLeaf = m_IsLeafs[Node];
		bool IsParent = HasChild[Node];
		asserta(IsLeaf || IsParent);
		asserta(!(IsLeaf && IsParent));
		}
	}

double NewickTree::GetLength(uint Node) const
	{
	asserta(Node < SIZE(m_Lengths));
	double Length = m_Lengths[Node];
	return Length;
	}

uint NewickTree::GetParent(uint Node) const
	{
	asserta(Node < SIZE(m_Parents));
	uint Parent = m_Parents[Node];
	return Parent;
	}

const string &NewickTree::GetLabel(uint Node) const
	{
	asserta(Node < SIZE(m_Labels));
	const string &Label = m_Labels[Node];
	return Label;
	}

bool NewickTree::HasBinaryRoot() const
	{
	vector<vector<uint> > Edges;
	GetNonParentEdges(Edges);
	const uint NodeCount = GetNodeCount();
	asserta(SIZE(Edges) == NodeCount);
	asserta(m_Root < NodeCount);
	const vector<uint> &RootNodeEdges = Edges[m_Root];
	uint RootChildCount = SIZE(RootNodeEdges);
	if (RootChildCount == 2)
		return true;
	return false;
	}

bool NewickTree::IsBinary() const
	{
	vector<vector<uint> > Edges;
	GetNonParentEdges(Edges);
	const uint NodeCount = GetNodeCount();
	asserta(SIZE(Edges) == NodeCount);
	asserta(m_Root < NodeCount);
	for (uint NodeIndex = 0; NodeIndex < NodeCount; ++NodeIndex)
		{
		const vector<uint> &NodeEdges = Edges[NodeIndex];
		uint ChildCount = SIZE(NodeEdges);
		if (ChildCount == 0)
			continue;
		if (NodeIndex == m_Root)
			{
			if (ChildCount != 2 && ChildCount != 3)
				return false;
			}
		else
			{
			if (ChildCount != 2)
				return false;
			}
		}
	return true;
	}

void NewickTree::GetNonParentEdges(vector<vector<uint> > &Edges) const
	{
	Edges.clear();
	const uint NodeCount = GetNodeCount();
	Edges.resize(NodeCount);

	bool RootFound = false;
	for (uint Node = 0; Node < NodeCount; ++Node)
		{
		uint Parent = m_Parents[Node];
		if (Parent == UINT_MAX)
			{
			asserta(!RootFound);
			RootFound = true;
			continue;
			}
		asserta(Parent < NodeCount);
		Edges[Parent].push_back(Node);
		}
	}
