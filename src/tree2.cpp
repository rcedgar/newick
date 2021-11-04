#include "myutils.h"
#include "tree2.h"
#include "newickparser2.h"
#include "treen.h"

void RootByHalves(const Tree2 &UnrootedTree, Tree2 &RootedTree);

/***
Binary tree
    Nbr1    Nbr2    Nbr3
:-----------------------------------------------------------:
:	Nbr1	Nbr2	Nbr3	:	Non-leaf, unrooted			:
:	Parent	Left	Right	:	Internal, rooted			:
:	Parent	*		*		:	Leaf, rooted or unrooted	:
:	*		Left	Right	:	Root						:
:-----------------------------------------------------------:
***/

void Tree2::LogMe(FILE *f) const
	{
	if (f == 0)
		return;
	const uint NodeCount = GetNodeCount();
	const uint EdgeCount = GetEdgeCount();
	const uint LeafCount = GetLeafCount();
	bool Rooted = IsRooted();

	fprintf(f, "\n");
	fprintf(f, "%s, %u nodes, %u edges, %u leaves\n",
	  Rooted ? "Rooted" : "Unrooted", NodeCount, EdgeCount, LeafCount);
	ToJust(f);
	}

/***
N=Nodes, E=Edges, L=Leaves

Rooted:
	N is odd
	E = N - 1
	L = (N + 1)/2

Unrooted:
	N is even
	E = N - 1
	L = N/2 + 1
***/

uint Tree2::GetLeafCount() const
	{
	uint NodeCount = GetNodeCount();
	if (IsRooted())
		{
		asserta(NodeCount%2 == 1);
		return (NodeCount + 1)/2;
		}
	else
		{
		asserta(NodeCount%2 == 0);
		return NodeCount/2 + 1;
		}
	}

uint Tree2::GetEdgeCount() const
	{
	const uint NodeCount = GetNodeCount();
	asserta(NodeCount > 1);
	return NodeCount - 1;
	}

void Tree2::Rotate(uint Node)
	{
	uint Left = GetLeft(Node);
	uint Right = GetRight(Node);
	m_Nbrs2[Node] = Right;
	m_Nbrs3[Node] = Left;
	}

uint Tree2::GetNodeByAcc(const string &Acc, bool ErrorIfNotFound) const
	{
	const uint NodeCount = GetNodeCount();
	for (uint Node = 0; Node < NodeCount; ++Node)
		{
		const string &NodeLabel = GetLabel(Node);
		if (NodeLabel != "")
			{
			string NodeAcc;
			GetAccFromLabel(NodeLabel, NodeAcc);
			if (NodeAcc == Acc)
				return Node;
			}
		}
	if (ErrorIfNotFound)
		Die("Acc '%s' not found", Acc.c_str());
	return UINT_MAX;
	}

uint Tree2::GetNodeByLabel(const string &Label, bool ErrorIfNotFound) const
	{
	const uint NodeCount = GetNodeCount();
	for (uint Node = 0; Node < NodeCount; ++Node)
		{
		const string &NodeLabel = GetLabel(Node);
		if (NodeLabel != "")
			{
			if (NodeLabel == Label)
				return Node;
			}
		}
	if (ErrorIfNotFound)
		Die("Label '%s' not found", Label.c_str());
	return UINT_MAX;
	}

void Tree2::GetLabel(uint Node, string &Label) const
	{
	asserta(Node < SIZE(m_Labels));
	Label = m_Labels[Node];
	}

const string &Tree2::GetLabel(uint Node) const
	{
	asserta(Node < SIZE(m_Labels));
	const string &Label = m_Labels[Node];
	return Label;
	}

void Tree2::GetEdgePair(uint Node1, uint Node2, pair<uint, uint> &EdgePair,
  bool FailOnError) const
	{
	if (FailOnError)
		{
		asserta(Node1 != UINT_MAX);
		asserta(Node2 != UINT_MAX);
		}
	if (Node1 < Node2)
		EdgePair = make_pair(Node1, Node2);
	else
		EdgePair = make_pair(Node2, Node1);
	}

bool Tree2::IsEdgeInMap(uint Node1, uint Node2) const
	{
	pair<uint, uint> EdgePair;
	GetEdgePair(Node1, Node2, EdgePair);
	map<pair<uint, uint>, double>::const_iterator p = 
	  m_EdgeToLength.find(EdgePair);
	bool InMap = (p != m_EdgeToLength.end());
	return InMap;
	}

void Tree2::SetEdgeLength(uint Node, double Length)
	{
	asserta(IsRooted());
	uint Parent = GetParent(Node);
	asserta(IsEdge(Node, Parent));
	pair<uint, uint> EdgePair;
	GetEdgePair(Node, Parent, EdgePair);
	m_EdgeToLength[EdgePair] = Length;
	}

double Tree2::GetEdgeLength(uint Node1, uint Node2, bool FailOnError) const
	{
	if (FailOnError)
		asserta(IsEdge(Node1, Node2));
	pair<uint, uint> EdgePair;
	GetEdgePair(Node1, Node2, EdgePair, FailOnError);
	map<pair<uint, uint>, double>::const_iterator p = 
	  m_EdgeToLength.find(EdgePair);
	if (p == m_EdgeToLength.end())
		return MISSING_LENGTH;
	double Length = p->second;
	return Length;
	}

double Tree2::GetEdgeLengthToParent(uint Node, bool FailOnError) const
	{
	uint Parent = GetParent(Node);
	double Length = GetEdgeLength(Node, Parent, FailOnError);
	return Length;
	}

double Tree2::GetEdgeLengthToLeftChild(uint Node, bool FailOnError) const
	{
	uint Left = GetLeft(Node);
	double Length = GetEdgeLength(Node, Left, FailOnError);
	return Length;
	}

double Tree2::GetEdgeLengthToRightChild(uint Node, bool FailOnError) const
	{
	uint Right = GetRight(Node);
	double Length = GetEdgeLength(Node, Right, FailOnError);
	return Length;
	}

bool Tree2::IsLeaf(uint Node) const
	{
	if (Node == UINT_MAX)
		return false;
	asserta(Node < SIZE(m_Nbrs2));
	asserta(Node < SIZE(m_Nbrs3));
	uint Nbr2 = m_Nbrs2[Node];
	uint Nbr3 = m_Nbrs3[Node];
	if (Nbr2 == UINT_MAX && Nbr3 == UINT_MAX)
		return true;
	else if (Nbr2 != UINT_MAX && Nbr3 != UINT_MAX)
		return false;
	asserta(false);
	return false;
	}

bool Tree2::IsRoot(uint Node) const
	{
	if (Node == UINT_MAX)
		return false;
	asserta(Node < SIZE(m_Nbrs1));
	return Node == m_Root;
	}

void Tree2::ValidateEdge(uint Node, uint OtherNode) const
	{
	if (OtherNode == UINT_MAX)
		return;

	bool InMap = IsEdgeInMap(Node, OtherNode);
	if (!InMap)
		Die("ValidateEdge(%u, %u) length not found", Node, OtherNode);

	asserta(Node < SIZE(m_Nbrs1));
	asserta(OtherNode < SIZE(m_Nbrs1));
	bool Rev = (m_Nbrs1[OtherNode] == Node ||
	  m_Nbrs2[OtherNode] == Node ||
	  m_Nbrs3[OtherNode] == Node);
	if (!Rev)
		{
		Log("m_Nbrs1[Edge=%u]=%u\n", OtherNode, m_Nbrs1[OtherNode]);
		Log("m_Nbrs2[Edge=%u]=%u\n", OtherNode, m_Nbrs2[OtherNode]);
		Log("m_Nbrs3[Edge=%u]=%u\n", OtherNode, m_Nbrs3[OtherNode]);
		Die("ValidateEdge(uint Node=%u, uint OtherNode=%u) failed\n",
		  Node, OtherNode);
		}
	}

void Tree2::Validate() const
	{
	const uint NodeCount = SIZE(m_Nbrs1);
	asserta(SIZE(m_Nbrs2) == NodeCount);
	asserta(SIZE(m_Nbrs3) == NodeCount);
	asserta(SIZE(m_Labels) == NodeCount);

	bool Rooted = IsRooted();
	if (Rooted)
		asserta(m_Root < NodeCount);
	else
		asserta(m_Root == UINT_MAX);

	uint LeafCount = GetLeafCount();
	uint LeafCount2 = 0;
	for (uint Node = 0; Node < NodeCount; ++Node)
		{
		bool Leaf = IsLeaf(Node);
		if (Leaf)
			++LeafCount2;
		uint Node1 = m_Nbrs1[Node];
		uint Node2 = m_Nbrs2[Node];
		uint Node3 = m_Nbrs3[Node];
		asserta(Node1 != Node);
		asserta(Node2 != Node);
		asserta(Node3 != Node);
		ValidateEdge(Node, Node1);
		ValidateEdge(Node, Node2);
		ValidateEdge(Node, Node3);

		if (Leaf)
			{
			asserta(Node2 == UINT_MAX);
			asserta(Node3 == UINT_MAX);
			if (NodeCount == 1)
				asserta(Node1 == UINT_MAX);
			else
				asserta(Node1 != UINT_MAX);
			}

		if (Rooted)
			{
			bool Root = IsRoot(Node);
			if (Root)
				{
				asserta(Node1 == UINT_MAX);
				if (NodeCount == 1)
					{
					asserta(Node2 == UINT_MAX);
					asserta(Node3 == UINT_MAX);
					}
				else
					{
					asserta(Node2 != UINT_MAX);
					asserta(Node3 != UINT_MAX);
					}
				}
			else if (!Leaf)
				{
				asserta(Node2 < NodeCount);
				asserta(Node3 < NodeCount);
				asserta(m_Nbrs1[Node2] == Node);
				asserta(m_Nbrs1[Node3] == Node);
				}
			}
		}
	asserta(LeafCount2 == LeafCount);

	for (map<pair<uint, uint>, double>::const_iterator p = m_EdgeToLength.begin();
	  p != m_EdgeToLength.end(); ++p)
		{
		const pair<uint, uint> &EdgePair = p->first;
		uint Node = EdgePair.first;
		uint OtherNode = EdgePair.second;
		if (!IsEdge(Node, OtherNode))
			Die("Map edge not found %u, %u", Node, OtherNode);
		}
	}

void Tree2::GetSubtrees(vector<vector<uint> > &LeafNodesVec) const
	{
	asserta(IsRooted());
	const uint NodeCount = GetNodeCount();
	LeafNodesVec.clear();
	LeafNodesVec.resize(NodeCount);
	for (uint Node = 0; Node < NodeCount; ++Node)
		GetSubtreeLeafNodes(Node, LeafNodesVec[Node]);
	}

void Tree2::GetSubtreeLeafNodes(uint Node, vector<uint> &LeafNodes) const
	{
	LeafNodes.clear();
	vector<uint> Leaves;
	AppendLeaves(Node, Leaves);
	for (uint i = 0; i < SIZE(Leaves); ++i)
		{
		uint LeafNode = Leaves[i];
		LeafNodes.push_back(LeafNode);
		}
	}

void Tree2::GetSubtreeLeafLabels(uint Node, vector<string> &Labels) const
	{
	Labels.clear();
	vector<uint> Leaves;
	AppendLeaves(Node, Leaves);
	for (uint i = 0; i < SIZE(Leaves); ++i)
		{
		uint Node = Leaves[i];
		const string &Label = GetLabel(Node);
		Labels.push_back(Label);
		}
	}

uint Tree2::GetSubtreeLeafCount(uint Node) const
	{
	vector<uint> Leaves;
	AppendLeaves(Node, Leaves);
	uint n = SIZE(Leaves);
	return n;
	}

void Tree2::AppendLeaves(uint Node, vector<uint> &Leaves) const
	{
	uint NodeCount = GetNodeCount();
	uint LeafCount = GetLeafCount();
	asserta(Node < NodeCount);
	asserta(SIZE(Leaves) < LeafCount);

	if (IsLeaf(Node))
		Leaves.push_back(Node);
	else
		{
		asserta(Node < SIZE(m_Nbrs2));
		asserta(Node < SIZE(m_Nbrs3));
		uint Nbr2 = m_Nbrs2[Node];
		uint Nbr3 = m_Nbrs3[Node];
		AppendLeaves(Nbr2, Leaves);
		AppendLeaves(Nbr3, Leaves);
		}
	}

uint Tree2::GetNodeCountToRoot(uint Node) const
	{
	if (!IsRooted())
		Die("GetDepth(), not rooted");
	const uint NodeCount = GetNodeCount();
	uint Depth = 1;
	for (;;)
		{
		asserta(Node < NodeCount);
		asserta(Depth <= NodeCount);
		if (IsRoot(Node))
			return Depth;
		Node = GetParent(Node);
		++Depth;
		}
	}

uint Tree2::GetNodeCountToFurthestLeaf(uint Node) const
	{
	if (IsLeaf(Node))
		return 1;
	uint Left = GetLeft(Node);
	uint Right = GetRight(Node);
	uint n = 1 + max(GetNodeCountToFurthestLeaf(Left),  
	  GetNodeCountToFurthestLeaf(Right));
	return n;
	}

uint Tree2::GetMaxNodeCountToRoot() const
	{
	const uint NodeCount = GetNodeCount();
	uint MaxDepth = 0;
	for (uint Node = 0; Node < NodeCount; ++Node)
		{
		uint Depth = GetNodeCountToRoot(Node);
		if (Depth > MaxDepth)
			MaxDepth = Depth;
		}
	return MaxDepth;
	}

double Tree2::GetMaxLeafDist(uint Node) const
	{
	asserta(IsRooted());
	if (IsLeaf(Node))
		return 0;

	uint Left = GetLeft(Node);
	uint Right = GetRight(Node);

	double LengthL = GetEdgeLength(Node, Left);
	double LengthR = GetEdgeLength(Node, Right);

	double DistL = GetMaxLeafDist(Left);
	double DistR = GetMaxLeafDist(Right);

	double SumL = LengthL + DistL;
	double SumR = LengthR + DistR;

	double MaxDist = max(SumL, SumR);
	return MaxDist;
	}

void Tree2::GetLeafRootDists(vector<double> &Heights) const
	{
	if (!IsRooted())
		Die("GetLeafHeights(), not rooted");

	Heights.clear();
	const uint NodeCount = GetNodeCount();
	for (uint Node = 0; Node < NodeCount; ++Node)
		{
		if (!IsLeaf(Node))
			continue;
		double Height = GetRootDist(Node);
		Heights.push_back(Height);
		}
	}

void Tree2::GetRootDists(vector<double> &Heights) const
	{
	if (!IsRooted())
		Die("GetHeights(), not rooted");

	Heights.clear();
	const uint NodeCount = GetNodeCount();
	for (uint Node = 0; Node < NodeCount; ++Node)
		{
		double Height = GetRootDist(Node);
		Heights.push_back(Height);
		}
	}

void Tree2::Inorder(uint Node, fn_OnNode OnNode) const
	{
	uint Left = GetLeft(Node);
	uint Right = GetRight(Node);
	if (Left != UINT_MAX)
		Inorder(Left, OnNode);
	OnNode(*this, Node);
	if (Right != UINT_MAX)
		Inorder(Left, OnNode);
	}

void Tree2::Preorder(uint Node, fn_OnNode OnNode) const
	{
	uint Left = GetLeft(Node);
	uint Right = GetRight(Node);
	OnNode(*this, Node);
	if (Left != UINT_MAX)
		Preorder(Left, OnNode);
	if (Right != UINT_MAX)
		Preorder(Right, OnNode);
	}

void Tree2::Postorder(uint Node, fn_OnNode OnNode) const
	{
	uint Left = GetLeft(Node);
	uint Right = GetRight(Node);
	if (Left != UINT_MAX)
		Postorder(Left, OnNode);
	if (Right != UINT_MAX)
		Postorder(Right, OnNode);
	OnNode(*this, Node);
	}

double Tree2::GetRootDist(uint Node) const
	{
	vector<uint> Path;
	GetPathToRoot(Node, Path);
	double Dist = 0;
	const uint n = SIZE(Path);
	for (uint i = 0; i < n; ++i)
		{
		uint Node = Path[i];
		if (Node == m_Root)
			{
			asserta(i+1 == n);
			break;
			}
		double Length = GetEdgeLengthToParent(Node);
		if (Length != MISSING_LENGTH)
			Dist += Length;
		}
	return Dist;
	}

void Tree2::GetPathToRoot(uint Node, vector<uint> &Path) const
	{
	if (!IsRooted())
		Die("GetPathToRoot(), not rooted");
	const uint NodeCount = GetNodeCount();
	Path.clear();
	for (;;)
		{
		asserta(Node < NodeCount);
		Path.push_back(Node);
		asserta(SIZE(Path) <= NodeCount);
		if (IsRoot(Node))
			return;
		Node = GetParent(Node);
		}
	}

void Tree2::GetLeafLabels(vector<string> &Labels, bool ErrorIfEmpty) const
	{
	Labels.clear();
	const uint NodeCount = GetNodeCount();
	for (uint Node = 0; Node < NodeCount; ++Node)
		{
		if (IsLeaf(Node))
			{
			const string &Label = GetLabel(Node);
			if (ErrorIfEmpty && Label.empty())
				Die("Empty label node %u", Node);
			Labels.push_back(Label);
			}
		}
	}

void Tree2::FromCStr(const char *CStr)
	{
	NewickParser2 NT;
	NT.FromCStr(CStr);
	FromNewickTree(NT);
	}

void Tree2::FromFile(const string &FileName)
	{
	FILE *f = OpenStdioFile(FileName);
	string Line;
	ReadLineStdioFile(f, Line);
	CloseStdioFile(f);
	if (Line.empty())
		Die("Empty line at start of '%s'", FileName.c_str());
	if (StartsWith(Line, "Node	Parent") || StartsWith(Line, "Node	Nbr1"))
		{
		FromTSVFile(FileName);
		return;
		}
	FromNewickFile(FileName);
	}

void Tree2::FromNewickFile(const string &FileName)
	{
	NewickParser2 NT;
	NT.FromFile(FileName);
	FromNewickTree(NT);
	}

void Tree2::FromTreeN(const TreeN &T)
	{
	string NewickStr;
	T.ToNewickStr(NewickStr, false);
	FromStr(NewickStr);
	}

void Tree2::FromTree(const Tree2 &T)
	{
	m_Root = T.m_Root;
	m_Nbrs1 = T.m_Nbrs1;
	m_Nbrs2 = T.m_Nbrs2;
	m_Nbrs3 = T.m_Nbrs3;
	m_Labels = T.m_Labels;
	m_EdgeToLength = T.m_EdgeToLength;
	}

void Tree2::FromData(const char *Data, uint Bytes)
	{
	NewickParser2 NP;
	NP.FromData(Data, Bytes);
	NP.FixFT();
	FromNewickTree(NP);
	}

void Tree2::FromNewickTree(const NewickTree &NT)
	{
	Clear();
	bool IsBinary = NT.IsBinary();
	if (!IsBinary)
		{
		NT.LogMe();
		Die("Tree2::FromNewickTree, not binary");
		}
	bool Rooted = NT.HasBinaryRoot();
	const uint NodeCountNT = NT.GetNodeCount();
	if (NodeCountNT < 3)
		Die("Tree2::FromNewickTree() Newick tree has %u nodes, not supported",
		  NodeCountNT);
	vector<vector<uint> > EdgesNT;
	NT.GetNonParentEdges(EdgesNT);
	uint RootNT = NT.m_Root;
	if (Rooted)
		{
		for (uint NodeNT = 0; NodeNT < NodeCountNT; ++NodeNT)
			{
			const vector<uint> &Edges = EdgesNT[NodeNT];
			uint EdgeCount = SIZE(Edges);
			uint Parent = NT.GetParent(NodeNT);
			string Label = NT.GetLabel(NodeNT);
			if (opt(accs) && SIZE(Edges) == 1)
				{
				string NewLabel;
				GetAccFromLabel(Label, NewLabel);
				Label = NewLabel;
				}
			double Length = NT.GetLength(NodeNT);

			if (NodeNT == RootNT)
				{
				asserta(Parent == UINT_MAX);
				m_Root = RootNT;
				}
			else
				{
				asserta(Parent < NodeCountNT);
				SetLength(NodeNT, Parent, Length);
				}

			m_Labels.push_back(Label);
			m_Nbrs1.push_back(Parent);
			if (EdgeCount == 2)
				{
				uint Left = Edges[0];
				uint Right = Edges[1];
				m_Nbrs2.push_back(Left);
				m_Nbrs3.push_back(Right);
				}
			else if (EdgeCount == 0)
				{
				m_Nbrs2.push_back(UINT_MAX);
				m_Nbrs3.push_back(UINT_MAX);
				}
			else
				Die("Rooted Newick tree is not binary, node %u has %u children",
				  NodeNT, EdgeCount);
			}
		}
	else
		{
		m_Root = UINT_MAX;
		for (uint NodeNT = 0; NodeNT < NodeCountNT; ++NodeNT)
			{
			string Label = NT.GetLabel(NodeNT);
			if (opt(accs))
				{
				string NewLabel;
				GetAccFromLabel(Label, NewLabel);
				Label = NewLabel;
				}
			double Length = NT.GetLength(NodeNT);
			uint Parent = NT.GetParent(NodeNT);
			const vector<uint> &Edges = EdgesNT[NodeNT];
			uint EdgeCount = SIZE(Edges);
			if (NodeNT == RootNT)
				{
				asserta(EdgeCount == 3);

				uint Nbr1 = Edges[0];
				uint Nbr2 = Edges[1];
				uint Nbr3 = Edges[2];

				double Length1 = NT.GetLength(Nbr1);
				double Length2 = NT.GetLength(Nbr2);
				double Length3 = NT.GetLength(Nbr3);

				m_Nbrs1.push_back(Nbr1);
				m_Nbrs2.push_back(Nbr2);
				m_Nbrs3.push_back(Nbr3);

				m_Labels.push_back(Label);
				continue;
				}

			SetLength(NodeNT, Parent, Length);
			m_Labels.push_back(Label);
			m_Nbrs1.push_back(Parent);
			if (EdgeCount == 2)
				{
				uint Left = Edges[0];
				uint Right = Edges[1];
				m_Nbrs2.push_back(Left);
				m_Nbrs3.push_back(Right);
				}
			else if (EdgeCount == 0)
				{
				m_Nbrs2.push_back(UINT_MAX);
				m_Nbrs3.push_back(UINT_MAX);
				}
			else
				Die("Rooted Newick tree is not binary, node %u has %u children",
				  NodeNT, EdgeCount);
			}
		}
	Validate();
	}

void Tree2::FromVectors(const vector<string> &Labels, 
  const vector<uint> &Parents, const vector<double> &Lengths)
	{
	Clear();
	const uint NodeCount = SIZE(Labels);
	asserta(SIZE(Parents) == NodeCount);
	asserta(SIZE(Lengths) == NodeCount);

	vector<uint> Lefts(NodeCount, UINT_MAX);
	vector<uint> Rights(NodeCount, UINT_MAX);
	m_Root = UINT_MAX;
	for (uint Node = 0; Node < NodeCount; ++Node)
		{
		uint Parent = Parents[Node];
		if (Parent == UINT_MAX)
			{
			asserta(m_Root == UINT_MAX);
			m_Root = Node;
			continue;
			}
		asserta(Parent < NodeCount);
		if (Lefts[Parent] == UINT_MAX)
			Lefts[Parent] = Node;
		else if (Rights[Parent] == UINT_MAX)
			Rights[Parent] = Node;
		else
			Die("Invalid vector topology");
		}
	asserta(m_Root != UINT_MAX);

	bool RootFound = false;
	for (uint Node = 0; Node < NodeCount; ++Node)
		{
		const string &Label = Labels[Node];
		double Length = Lengths[Node];
		uint Parent = Parents[Node];
		uint Left = Lefts[Node];
		uint Right = Rights[Node];
		bool Leaf = (Left == UINT_MAX && Right == UINT_MAX);
		bool Int =  (Left != UINT_MAX && Right != UINT_MAX);
		asserta(Leaf || Int);

		m_Labels.push_back(Label);
		m_Nbrs1.push_back(Parent);
		m_Nbrs2.push_back(Left);
		m_Nbrs3.push_back(Right);
		//m_Lengths.push_back(Length);
		if (Parent == UINT_MAX)
			{
			asserta(!RootFound);
			RootFound = true;
			continue;
			}
		SetLength(Node, Parent, Length);
		}
	Validate();
	}

void Tree2::SetLength(uint Node1, uint Node2, double Length)
	{
	asserta(Node1 != UINT_MAX);
	asserta(Node2 != UINT_MAX);
	pair<uint, uint> EdgePair;
	GetEdgePair(Node1, Node2, EdgePair);
	m_EdgeToLength[EdgePair] = Length;
	}

// AncNode must be on path from Node to root
double Tree2::GetDistance(uint Node, uint AncNode) const
	{
	if (Node == m_Root && AncNode == UINT_MAX)
		return 0;

	vector<uint> Path;
	GetPathToRoot(Node, Path);
	double Distance = 0;
	const uint n = SIZE(Path);
	asserta(Path[0] == Node);
	for (uint i = 0; i < n; ++i)
		{
		uint Node2 = Path[i];
		if (Node2 == AncNode)
			return Distance;
		double Length = GetEdgeLengthToParent(Node2);
		if (Length != MISSING_LENGTH)
			Distance += Length;
		}
	Die("GetDistance, not ancestor");
	return 0;
	}

uint Tree2::GetParent(uint Node) const
	{
	asserta(IsRooted());
	asserta(Node < SIZE(m_Nbrs1));
	uint Parent = m_Nbrs1[Node];
	return Parent;
	}

uint Tree2::GetLeft(uint Node) const
	{
	asserta(IsRooted());
	asserta(Node < SIZE(m_Nbrs2));
	uint Left = m_Nbrs2[Node];
	return Left;
	}

uint Tree2::GetRight(uint Node) const
	{
	asserta(IsRooted());
	asserta(Node < SIZE(m_Nbrs3));
	uint Right = m_Nbrs3[Node];
	return Right;
	}

uint Tree2::GetEdge1(uint Node) const
	{
	asserta(Node < SIZE(m_Nbrs1));
	uint Nbr1 = m_Nbrs1[Node];
	return Nbr1;
	}

uint Tree2::GetEdge2(uint Node) const
	{
	asserta(Node < SIZE(m_Nbrs2));
	uint Nbr2 = m_Nbrs2[Node];
	return Nbr2;
	}

uint Tree2::GetEdge3(uint Node) const
	{
	asserta(Node < SIZE(m_Nbrs3));
	uint Nbr3 = m_Nbrs3[Node];
	return Nbr3;
	}

void Tree2::SetEdge(uint FromNode, uint i, uint ToNode)
	{
	switch (i)
		{
	case 1:	m_Nbrs1[FromNode] = ToNode; return;
	case 2:	m_Nbrs2[FromNode] = ToNode; return;
	case 3:	m_Nbrs3[FromNode] = ToNode; return;
		}
	asserta(false);
	}

void Tree2::OrientNode(uint Node, uint Parent)
	{
	if (Node == UINT_MAX)
		return;

	uint Nbr1 = m_Nbrs1[Node];
	uint Nbr2 = m_Nbrs2[Node];
	uint Nbr3 = m_Nbrs3[Node];

// If leaf, nothing to do
	if (Nbr1 == UINT_MAX || Nbr2 == UINT_MAX || Nbr3 == UINT_MAX)
		{
		asserta(Nbr1 != UINT_MAX && Nbr2 == UINT_MAX && Nbr3 == UINT_MAX);
		return;
		}

	uint Left = UINT_MAX;
	uint Right = UINT_MAX;
	if (Nbr1 == Parent)
		{
		Left = Nbr2;
		Right = Nbr3;
		}
	else if (Nbr2 == Parent)
		{
		Left = Nbr1;
		Right = Nbr3;
		}
	else if (Nbr3 == Parent)
		{
		Left = Nbr1;
		Right = Nbr2;
		}
	else
		Die("OrientNode(%u, %u) not edge", Node, Parent);

	m_Nbrs1[Node] = Parent;
	m_Nbrs2[Node] = Left;
	m_Nbrs3[Node] = Right;

	OrientNode(Left, Node);
	OrientNode(Right, Node);
	}

bool Tree2::IsEdge(uint Node, uint OtherNode) const
	{
	if (Node == UINT_MAX || OtherNode == UINT_MAX)
		return false;
	const uint NodeCount = GetNodeCount();
	asserta(Node < NodeCount);
	asserta(OtherNode < NodeCount);
	if (m_Nbrs1[Node] == OtherNode)
		return true;
	if (m_Nbrs2[Node] == OtherNode)
		return true;
	if (m_Nbrs3[Node] == OtherNode)
		return true;
	return false;
	}

uint Tree2::GetEdge(uint Node, uint i) const
	{
	switch (i)
		{
	case 1:	return GetEdge1(Node);
	case 2:	return GetEdge2(Node);
	case 3:	return GetEdge3(Node);
		}
	asserta(false);
	return UINT_MAX;
	}

void Tree2::AppendNodeToNewickStr(string &Str, uint Node, bool WithLineBreaks) const
	{
	asserta(IsRooted());
	uint Left = GetLeft(Node);
	uint Right = GetRight(Node);
	uint Parent = GetParent(Node);
	double Length = MISSING_LENGTH;
	if (Parent != UINT_MAX)
		Length = GetEdgeLength(Node, Parent);
	const string &Label = GetLabel(Node);
	bool Leaf = IsLeaf(Node);

	if (Leaf)
		asserta(Left == UINT_MAX && Right == UINT_MAX);
	else
		{
		asserta(Left != UINT_MAX && Right != UINT_MAX);
		Str += "(";
		AppendNodeToNewickStr(Str, Left, WithLineBreaks);
		Str += ",";
		AppendNodeToNewickStr(Str, Right, WithLineBreaks);
		Str += ")";
		}

	Str += Label;
	if (Length != MISSING_LENGTH)
		Psa(Str, ":%.4g", Length);
	if (WithLineBreaks)
		Str += "\n";
	}

void Tree2::ToNewickFile(const string  &FileName) const
	{
	if (FileName.empty())
		return;

	FILE *f = CreateStdioFile(FileName);
	ToNewickFile(f);
	CloseStdioFile(f);
	}

void Tree2::ToNewickStr(string &Str, bool WithLineBreaks) const
	{
	Str.clear();
	if (IsRooted())
		{
		AppendNodeToNewickStr(Str, m_Root, WithLineBreaks);
		Str += ";";
		if (WithLineBreaks)
			Str += "\n";
		}
	else
		{
		Tree2 RootedTree;
		RootByHalves(*this, RootedTree);

		uint Root = RootedTree.GetRoot();
		uint RootLeft = RootedTree.GetLeft(Root);
		uint RootRight = RootedTree.GetRight(Root);
		bool RootLeftIsLeaf = RootedTree.IsLeaf(RootLeft);
		bool RootRightIsLeaf = RootedTree.IsLeaf(RootRight);
		if (RootLeftIsLeaf && RootRightIsLeaf)
			Die("Unrooted tree has 2 leaves");

		double RootLeftLength = RootedTree.GetEdgeLength(Root, RootLeft);
		double RootRightLength = RootedTree.GetEdgeLength(Root, RootRight);
		double RootEdgeLength = RootLeftLength + RootRightLength;

		if (!RootRightIsLeaf)
			{
			uint RootRightLeft = RootedTree.GetLeft(RootRight);
			uint RootRightRight = RootedTree.GetRight(RootRight);
			asserta(RootRightLeft != UINT_MAX);
			asserta(RootRightRight != UINT_MAX);
			RootedTree.SetEdgeLength(RootLeft, RootEdgeLength);
			RootedTree.SetEdgeLength(RootRight, MISSING_LENGTH);

			Str += "(";
			RootedTree.AppendNodeToNewickStr(Str, RootLeft, WithLineBreaks);
			Str += ",";
			RootedTree.AppendNodeToNewickStr(Str, RootRightLeft, WithLineBreaks);
			Str += ",";
			RootedTree.AppendNodeToNewickStr(Str, RootRightRight, WithLineBreaks);
			Str += ");";
			if (WithLineBreaks)
				Str += "\n";
			}
		else
			{
			asserta(!RootLeftIsLeaf);
			uint RootLeftLeft = RootedTree.GetLeft(RootLeft);
			uint RootLeftRight = RootedTree.GetRight(RootLeft);
			asserta(RootLeftLeft != UINT_MAX);
			asserta(RootLeftRight != UINT_MAX);
			RootedTree.SetEdgeLength(RootRight, RootEdgeLength);
			RootedTree.SetEdgeLength(RootLeft, MISSING_LENGTH);

			Str += "(";
			RootedTree.AppendNodeToNewickStr(Str, RootRight, WithLineBreaks);
			Str += ",";
			RootedTree.AppendNodeToNewickStr(Str, RootLeftLeft, WithLineBreaks);
			Str += ",";
			RootedTree.AppendNodeToNewickStr(Str, RootLeftRight, WithLineBreaks);
			Str += ");";
			if (WithLineBreaks)
				Str += "\n";
			}
		}
	}

static void GetMaxString(const vector<string> &v, string &MaxStr)
	{
	asserta(!v.empty());
	MaxStr = v[0];
	for (uint i = 1; i < SIZE(v); ++i)
		MaxStr = max(MaxStr, v[i]);
	}

static bool CompareLabels(const vector<string> &Labels1,
  const vector<string> &Labels2)
	{
	string Max1;
	string Max2;
	GetMaxString(Labels1, Max1);
	GetMaxString(Labels2, Max2);
	bool Gt = (Max1 > Max2);
	return Gt;
	}

uint Tree2::Ladderize(bool MoreRight)
	{
	const uint NodeCount = GetNodeCount();
	uint RotatedCount = 0;
	for (uint Node = 0; Node < NodeCount; ++Node)
		{
		if (IsLeaf(Node))
			continue;

		uint Left = GetLeft(Node);
		uint Right = GetRight(Node);
		uint NLeft = GetSubtreeLeafCount(Left);
		uint NRight = GetSubtreeLeafCount(Right);

		bool DoRotate = (MoreRight ? NRight < NLeft : NLeft < NRight);
		if (NLeft == NRight)
			{
			vector<string> LeftLabels;
			vector<string> RightLabels;
			GetSubtreeLeafLabels(Left, LeftLabels);
			GetSubtreeLeafLabels(Right, RightLabels);
			DoRotate = CompareLabels(LeftLabels, RightLabels);
			}
		if (DoRotate)
			{
			++RotatedCount;
			Rotate(Node);
			}
		}
	return RotatedCount;
	}

void Tree2::ToNewickFile(FILE *f) const
	{
	if (f == 0)
		return;
	string Str;
	ToNewickStr(Str, true);
	fputs(Str.c_str(), f);
	}

static void Test1(NewickParser2 &NP, Tree2 &T2, const string &NewickStr)
	{
	ProgressLog("\n");
	ProgressLog("_______________________________________________________\n");
	ProgressLog("  %s\n", NewickStr.c_str());

	NP.FromStr(NewickStr);
	T2.FromNewickTree(NP);
	T2.Validate();

	ProgressLog("    %s, validate ok.\n", T2.IsRooted() ? "rooted" : "unrooted");

	string NewickStr2;
	T2.ToNewickStr(NewickStr2);

	Log("InputStr %s\n", NewickStr.c_str());
	Log("Str2     %s\n", NewickStr2.c_str());

	Tree2 SameT2;
	SameT2.FromStr(NewickStr2);
	asserta(SameT2.GetNodeCount() == T2.GetNodeCount());
	}

static void cmd_test()
	{
	opt(test);
	NewickParser2 NP;
	Tree2 T2;

	Test1(NP, T2, "(A:0.1,B:0.2,C:0.3);");
	Test1(NP, T2, "(A,B,(C,D));");
	Test1(NP, T2, "(A,B,C);");
	Test1(NP, T2, "(A,(B,C));");
	Test1(NP, T2, "(A,B,(C,D));");
	Test1(NP, T2, "(A,B,(C,D)E)F;");
	Test1(NP, T2, "(A:0.1,B:0.2,(C:0.3,D:0.4):0.5);");
	Test1(NP, T2, "(A:0.1,B:0.2,(C:0.3,D:0.4)E:0.5)F;");
	//Test1(NP, T2, "(,,(,));");
	//Test1(NP, T2, "(:0.1,:0.2,(:0.3,:0.4):0.5);");
	//Test1(NP, T2, "(:0.1,:0.2,(:0.3,:0.4):0.5):0.0;");
	}
