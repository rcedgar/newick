#include "myutils.h"
#include "treex.h"
#include "murmur.h"
#include "sort.h"
#include <map>
#include <list>
#include <set>

/***
The tree is represented as a directed graph.

Edges are directed towards a leaf.

If tree is rooted, edge is oriented in root-to-leaf direction.

If tree is unrooted, an internal node must be chosen to
define a direction, this is the "origin". The choice is
abitrary; choosing a different origin flips the orientation
of some edges leaving the undirected graph of the tree
unchanged.

Incoming = "node-to-parent"
Outgoing = "node-to-children"

Binary3 = Newick-type unrooted binary tree with one 3-ary node.
Binary2 = Alternative unrooted binary tree with 2-ary origin.

   Ary   Rooted    Node   Incoming   Outgoing
Binary      Yes    Root          0          2
Binary      Yes     Int          1          2
Binary      Yes    Leaf          1          0

Binary3      No  Origin          0          3
Binary3      No     Int          1          2
Binary3      No    Leaf          1          0

Binary2      No  Origin          0          2
Binary2      No     Int          1          2
Binary2      No    Leaf          1          0

N-ary        Yes    Root         0        >=1
N-ary        Yes     Int         1        >=1
N-ary        Yes    Leaf         1          0

N-ary         No  Origin         0        >=1
N-ary         No     Int         1        >=1
N-ary         No    Leaf         1          0
***/

void TreeX::FromNewickFile(const string &FileName)
	{
	NewickParser2 NP;
	NP.FromFile(FileName);
	FromNewickParser(NP);
	}

void TreeX::CopyNormalized(TreeX &T) const
	{
	Copy(T);
	T.Normalize();
	}

void TreeX::Copy(TreeX &T) const
	{
	T.m_Origin = m_Origin;
	T.m_Rooted = m_Rooted;
	T.m_AssignedNodeCount = 0;
	T.m_Size = 0;
	T.m_LabelToNode = m_LabelToNode;
	T.m_Labels = m_Labels;
	T.m_LeafNodeSet = 0;
	T.m_FoundLeafNodeSet.clear();
	T.m_VisitedNodes.clear();
	T.m_PendingNodes.clear();
	T.m_TX = 0;

	asserta(m_Size >= m_AssignedNodeCount);
	T.Alloc(m_Size);
	T.m_Size = m_Size;
	T.m_AssignedNodeCount = m_AssignedNodeCount;

#define c(t, x)	\
	memcpy(T.m_##x, m_##x, m_Size*sizeof(t));

	c(double, NodeToLength);
	c(uint32, NodeToChildSize);
	c(uint32, NodeToChildCount);
	c(uint32, NodeToParent);

#undef c
	
	for (uint Node = 0; Node < m_AssignedNodeCount; ++Node)
		{
		T.m_NodeToChildren[Node] = 0;
		if (!IsNode(Node))
			continue;
		uint ChildCount = GetChildCount(Node);
		const uint *Children = GetChildren(Node);
		uint *TChildren = myalloc(uint, ChildCount);
		for (uint i = 0; i < ChildCount; ++i)
			TChildren[i] = Children[i];
		T.m_NodeToChildren[Node] = TChildren;
		}
	T.Validate();
	}

void TreeX::LogMe() const
	{
	Log("\n");
	Log("TreeX %u assigned nodes, origin %u, rooted %c\n",
	  m_AssignedNodeCount, m_Origin, yon(m_Rooted));
	uint DeletedCount = 0;
	uint ActiveCount = 0;
	for (uint Node = 0; Node < m_AssignedNodeCount; ++Node)
		{
		if (!IsNode(Node))
			{
			++DeletedCount;
			continue;
			}
		++ActiveCount;
		Log("[%5u]", Node);
		uint Parent = GetParent(Node);
		Log("  P: ");
		if (Parent == UINT_MAX)
			Log("%5.5s", "*");
		else
			Log("%5u", Parent);
		uint ChildCount = GetChildCount(Node);
		if (ChildCount == 0)
			Log(" Leaf");
		else
			{
			Log(" (%u children: ", ChildCount);
			for (uint i = 0; i < ChildCount; ++i)
				{
				uint Child = GetChild(Node, i);
				Log(" %u", Child);
				}
			Log(")");
			}
		double Length = GetLength(Node);
		if (Length != DBL_MAX)
			Log("  len=%.3g", Length);
		const string &Label = GetLabel(Node);
		if (Label != "")
			{
			if (ChildCount == 0)
				Log("  >%s", Label.c_str());
			else
				Log("  [%s]", Label.c_str());
			}
		Log("\n");
		}
	Log("%u active nodes, %u deleted\n",
	  ActiveCount, DeletedCount);
	}

void TreeX::Validate() const
	{
	set<uint> ChildrenFound;
	uint NoParentCount = 0;
	bool OriginFound = false;
	for (uint Node = 0; Node < m_AssignedNodeCount; ++Node)
		{
		if (m_NodeToChildCount[Node] == UINT_MAX)
			continue;

		const uint *Children = GetChildren(Node);
		uint ChildCount = GetChildCount(Node);
		for (uint i = 0; i < ChildCount; ++i)
			{
			uint Child = Children[i];
			if (ChildrenFound.find(Child) != ChildrenFound.end())
				Die("Child %u found twice", Child);
			ChildrenFound.insert(Child);
			}

		ValidateNode(Node);

		if (Node == m_Origin)
			{
			asserta(!OriginFound);
			OriginFound = true;
			}

		uint Parent = GetParent(Node);
		if (Parent == UINT_MAX)
			{
			asserta(Node == m_Origin);
			++NoParentCount;
			}

		const uint ECTO = GetEdgeCountToOrigin(Node);
		if (Node == m_Origin)
			asserta(ECTO == 0);
		else
			asserta(ECTO > 0);
		}

	for (unordered_map<string, uint>::const_iterator p = m_LabelToNode.begin();
	  p != m_LabelToNode.end(); ++p)
		{
		const string &Label1 = p->first;
		uint Node = p->second;
		asserta(Node < m_AssignedNodeCount);
		const string &Label2 = m_Labels[Node];
		asserta(Label1 == Label2);
		}

	asserta(NoParentCount == 1);
	asserta(OriginFound);
	}

void TreeX::ValidateNode(uint Node) const
	{
	asserta(Node < m_Size);
	uint ChildCount = m_NodeToChildCount[Node];
	if (ChildCount == UINT_MAX)
		return;

	asserta(Node < SIZE(m_Labels));

	const string &Label = m_Labels[Node];
	if (ChildCount == 0 && Label != "")
		{
		uint Node2 = GetNodeByLabel(Label);
		asserta(Node2 == Node);
		}

	uint Parent = m_NodeToParent[Node];
	if (Parent != UINT_MAX)
		{
		const uint *Children = GetChildren(Node);
		const uint ChildCount = GetChildCount(Node);
		for (uint i = 0; i < ChildCount; ++i)
			{
			uint Child = Children[i];
			asserta(Child < m_AssignedNodeCount);
			if (m_NodeToChildCount[Child] == UINT_MAX)
				{
				Log("\n");
				Log("================\n");
				LogMe();
				Log("\n");
				Log("================\n");
				Log("Node = %u, ChildCount = %u\n", Node, ChildCount);
				Log("m_NodeToChildCount[%u] = UINT_MAX\n", Child);
				}
			uint ChildParent = GetParent(Child);
			if (ChildParent != Node)
				{
				Log("\n");
				Log("================\n");
				LogMe();
				Log("\n");
				Log("================\n");
				Log("\n\nNode %u, %u'th child %u, ChildParent %u\n",
				  Node, i, Child, ChildParent);
				Die("ChildParent != Node");
				}
			}
		}
	}

void TreeX::FromNewickStr(const string &Str)
	{
	NewickParser2 NP;
	NP.FromStr(Str);
	FromNewickParser(NP);
	}

void TreeX::FromNewickParser(const NewickParser2 &NP)
	{
	m_Rooted = false;
	const uint NodeCount = NP.GetNodeCount();
	vector<uint> NewickNodeToXNode;
	m_Labels.clear();
	m_Labels.resize(NodeCount);
	Alloc(NodeCount);
	m_Origin = UINT_MAX;
	m_LabelToNode.clear();

	for (uint Node = 0; Node < NodeCount; ++Node)
		{
		m_NodeToChildCount[Node] = 0;
		m_NodeToChildSize[Node] = 0;
		}

	for (uint Node = 0; Node < NodeCount; ++Node)
		{
		uint Parent = NP.m_Parents[Node];
		double Length = NP.m_Lengths[Node];
		const string &Label = NP.m_Labels[Node];
		m_Labels[Node] = Label;
		if (Parent == UINT_MAX)
			{
			asserta(m_Origin == UINT_MAX);
			m_Origin = Node;
			}

		m_Labels.push_back(Label);
		m_NodeToParent[Node] = Parent;
		m_NodeToLength[Node] = Length;
		if (Parent != UINT_MAX)
			AddChildToBuffer(Parent, Node);
		}
	m_AssignedNodeCount = NodeCount;
	for (uint Node = 0; Node < NodeCount; ++Node)
		{
		if (GetChildCount(Node) == 0)
			{
			const string &Label = m_Labels[Node];
			if (Label != "")
				m_LabelToNode[Label] = Node;
			}
		}
	Validate();
	m_Rooted = NP.HasBinaryRoot();
	}

const uint *TreeX::GetChildren(uint Node) const
	{
	assert(IsNode(Node));
	return m_NodeToChildren[Node];
	}

uint *TreeX::GetChildren(uint Node)
	{
	assert(IsNode(Node));
	return m_NodeToChildren[Node];
	}

uint TreeX::GetParent(uint Node) const
	{
	assert(IsNode(Node));
	uint Parent = m_NodeToParent[Node];
	return Parent;
	}

uint TreeX::GetChildIndex(uint Parent, uint Child) const
	{
	assert(IsNode(Parent));
	assert(IsNode(Child));
	const uint *Children = m_NodeToChildren[Parent];
	const uint ChildCount = m_NodeToChildCount[Parent];
	for (uint i = 0; i < ChildCount; ++i)
		{
		if (Children[i] == Child)
			return i;
		}
	asserta(false);
	return UINT_MAX;
	}

void TreeX::Alloc(uint N)
	{
	if (N <= m_Size)
		return;

	uint OldSize = m_Size;
	uint NewSize = (1 + N/GOBUFF_SIZE)*GOBUFF_SIZE;
	asserta(NewSize >= N);
	asserta(NewSize > m_Size);
#if DEBUG
	const bool InitMem = true;
#else
	const bool InitMem = false;
#endif

#define	a(t, x, z)	\
	{ \
	t *NewBuff = myalloc(t, NewSize); \
	if (m_Size > 0) \
		memcpy(NewBuff, m_##x, m_Size*sizeof(t)); \
	myfree(m_##x); \
	if (InitMem) \
		{ \
		for (uint i = OldSize; i < NewSize; ++i) \
			NewBuff[i] = z; \
		} \
	m_##x = NewBuff; \
	}

	m_Labels.resize(NewSize);

	a(double, NodeToLength, 0);
	a(uint, NodeToParent, UINT_MAX-1);
	a(uint, NodeToChildCount, UINT_MAX-1);
	a(uint, NodeToChildSize, UINT_MAX-1);
	a(uint, NodeToPrev, UINT_MAX-1);
	a(uint *, NodeToChildren, 0);

	for (uint Node = OldSize; Node < NewSize; ++Node)
		{
		m_NodeToChildSize[Node] = 0;
		m_NodeToChildCount[Node] = UINT_MAX;
		}

	m_Size = NewSize;
	}

uint TreeX::GetNodeByLabel(const string &Label, bool FailIfNotFound) const
	{
	unordered_map<string, uint>::const_iterator p =
	  m_LabelToNode.find(Label);
	if (p == m_LabelToNode.end())
		{
		if (FailIfNotFound)
			Die("Label not found >%s", Label.c_str());
		return UINT_MAX;
		}
	uint Node = p->second;
	return Node;
	}

void TreeX::Init()
	{
	Alloc(GOBUFF_SIZE);

	m_Rooted = false;
	m_AssignedNodeCount = 1;
	m_Origin = 0;
	m_NodeToChildCount[0] = 0;
	m_NodeToParent[0] = UINT_MAX;
	m_NodeToLength[0] = DBL_MAX;
	}

void TreeX::EraseNode(uint Node)
	{
	assert(m_NodeToChildCount[Node] != UINT_MAX);
	m_NodeToChildCount[Node] = UINT_MAX;
	asserta(Node < SIZE(m_Labels));
	const string &Label = m_Labels[Node];
	if (Label != "")
		m_LabelToNode.erase(Label);
	m_NodeToParent[Node] = UINT_MAX;
	}

// Insert new node without assigning a parent
uint TreeX::InsertOrphanNode(double Length, const string &Label)
	{
	uint NewNode = InsertNode(UINT_MAX, Length, Label);
	return NewNode;
	}

uint TreeX::InsertNode(uint Parent, double Length, const string &Label)
	{
	uint NewNode = m_AssignedNodeCount;
	++m_AssignedNodeCount;
	Alloc(m_AssignedNodeCount);
	if (Parent != UINT_MAX)
		{
		assert(IsNode(Parent));
		asserta(Parent < m_AssignedNodeCount);
		AddChildToBuffer(Parent, NewNode);
		}

	m_NodeToChildCount[NewNode] = 0;
	m_NodeToParent[NewNode] = Parent;
	m_NodeToLength[NewNode] = Length;
	m_Labels[NewNode] = Label;

	if (Label != "")
		m_LabelToNode[Label] = NewNode;
	return NewNode;
	}

// Delete Node and all nodes in its subtree.
void TreeX::DeleteSubtree(uint Node, const string &NewLabel,
  bool DeleteSelf)
	{
	AssertNode(Node);

	uint ChildCount = m_NodeToChildCount[Node];
	asserta(ChildCount != UINT_MAX);

	const uint *Children = m_NodeToChildren[Node];
	for (uint i = 0; i < ChildCount; ++i)
		{
		uint Child = Children[i];
		DeleteSubtree(Child, "", true);
		}

	if (DeleteSelf)
		EraseNode(Node);
	else
		{
		m_NodeToChildCount[Node] = 0;
		UpdateLabel(Node, NewLabel);
		}
	}

void TreeX::SetChildCount(uint Parent, uint ChildCount)
	{
	uint ChildSize = m_NodeToChildSize[Parent];
	if (ChildCount < ChildSize)
		return;

	uint NewChildSize = ChildSize + 8;
	uint *NewChildBuffer = myalloc(uint, NewChildSize);
	if (ChildSize > 0)
		{
		uint32 *OldChildBuffer = m_NodeToChildren[Parent];
		memcpy(NewChildBuffer, OldChildBuffer, ChildSize*sizeof(uint32));
		myfree(OldChildBuffer);
		}
	m_NodeToChildSize[Parent] = NewChildSize;
	m_NodeToChildren[Parent] = NewChildBuffer;
	}

void TreeX::ReplaceChildInBuffer(uint Parent,
  uint OldChild, int NewChild)
	{
	uint ChildCount = GetChildCount(Parent);
	uint *ChildBuffer = m_NodeToChildren[Parent];
	for (uint i = 0; i < ChildCount; ++i)
		{
		if (ChildBuffer[i] == OldChild)
			{
			ChildBuffer[i] = NewChild;
			return;
			}
		}
	Die("ReplaceChildInBuffer(%u, %u, %u)\n",
	  Parent, OldChild, NewChild);
	}

void TreeX::AddChildToBuffer(uint Parent, uint Child)
	{
	uint ChildCount = m_NodeToChildCount[Parent];
	asserta(ChildCount < 3);//@@
	SetChildCount(Parent, ChildCount + 1);
	uint32 *Children = m_NodeToChildren[Parent];
	Children[ChildCount] = Child;
	m_NodeToChildCount[Parent] = ChildCount + 1;
	}

void TreeX::ReplaceChildInBuffer(uint Parent,
  uint OldChild, uint NewChild)
	{
	asserta(OldChild != NewChild);

	uint ChildCount = m_NodeToChildCount[Parent];
	asserta(ChildCount > 0);
	uint32 *Children = m_NodeToChildren[Parent];

	for (uint i = 0; i < ChildCount; ++i)
		{
		if (Children[i] == OldChild)
			{
			Children[i] = NewChild;
			return;
			}
		}
	asserta(false);
	}

void TreeX::DeleteChildFromBuffer(uint Parent, uint Child)
	{
	uint ChildCount = m_NodeToChildCount[Parent];
	asserta(ChildCount > 0);
	uint32 *Children = m_NodeToChildren[Parent];

	bool Found = false;
	for (uint i = 0; i < ChildCount; ++i)
		{
		if (Children[i] == Child)
			{
			asserta(!Found);
			Found = true;
			}
		if (Found)
			Children[i] = Children[i+1];
		}
	asserta(Found);
	m_NodeToChildCount[Parent] = ChildCount - 1;
	}

// Delete leaf, reducing number of outgoing
// edges from the parent by 1.
void TreeX::DeleteLeaf(uint Node)
	{
	AssertNode(Node);

	asserta(Node != m_Origin);
	uint ChildCount = m_NodeToChildCount[Node];
	asserta(ChildCount == 0);

	uint Parent = m_NodeToParent[Node];
	DeleteChildFromBuffer(Parent, Node);

	EraseNode(Node);
	}

bool TreeX::IsNode(uint Node) const
	{
	if (Node >= m_Size)
		return false;
	if (m_NodeToChildCount[Node] == UINT_MAX)
		return false;
	return true;
	}

uint TreeX::GetChild(uint Node, uint ChildIndex) const
	{
	assert(IsNode(Node));
	asserta(ChildIndex < m_NodeToChildCount[Node]);
	const uint *Children = m_NodeToChildren[Node];
	uint Child = Children[ChildIndex];
	return Child;
	}

void TreeX::UpdateLabel(uint Node, const string &NewLabel)
	{
	asserta(IsNode(Node));
	bool Leaf = IsLeaf(Node);
	if (Leaf)
		{
		const string &Label = m_Labels[Node];
		if (Label != "")
			m_LabelToNode.erase(Label);
		}
	m_Labels[Node] = NewLabel;
	if (NewLabel != "")
		m_LabelToNode[NewLabel] = Node;
	}

void TreeX::SetLength(uint Node, double Length)
	{
	assert(IsNode(Node));
	m_NodeToLength[Node] = Length;
	}

void TreeX::SetParent(uint Node, uint Parent)
	{
	assert(IsNode(Node));
	m_NodeToParent[Node] = Parent;
	}

void TreeX::GetLabel(uint Node, string &Label) const
	{
	Label = GetLabel(Node);
	}

const string &TreeX::GetLabel(uint Node) const
	{
	asserta(Node < SIZE(m_Labels));
	const string &Label = m_Labels[Node];
	return Label;
	}

uint TreeX::GetLeft(uint Node) const
	{
	AssertNode(Node);
	asserta(m_NodeToChildCount[Node] == 2);
	const uint *Children = m_NodeToChildren[Node];
	uint Left = Children[0];
	return Left;
	}

uint TreeX::GetRight(uint Node) const
	{
	AssertNode(Node);
	asserta(m_NodeToChildCount[Node] == 2);
	const uint *Children = m_NodeToChildren[Node];
	uint Right = Children[1];
	return Right;
	}

void TreeX::Unroot()
	{
	asserta(m_Rooted);
	uint ChildCount = GetChildCount(m_Origin);
	asserta(ChildCount == 2);

// Right will be new origin
// New node-to-parent edge Left -> Right
	uint Left = GetLeft(m_Origin);
	uint Right = GetRight(m_Origin);

// New edge Left->Right, length is sum
// of old edges Root->Left + Root->Right
	double LeftLength = GetLength(Left);
	double RightLength = GetLength(Right);
	m_NodeToLength[Left] = AddLengths(LeftLength, RightLength);

// Right is now parent of Left
	m_NodeToParent[Left] = Right;
	AddChildToBuffer(Right, Left);

	EraseNode(m_Origin);
	m_Origin = Right;

	Validate();
	}

uint TreeX::GetChildCount(uint Node) const
	{
	AssertNode(Node);
	uint ChildCount = m_NodeToChildCount[Node];
	return ChildCount;
	}

double TreeX::GetLength(uint Node) const
	{
	AssertNode(Node);
	double Length = m_NodeToLength[Node];
	return Length;
	}

bool TreeX::IsLeaf(uint Node) const
	{
	if (!IsNode(Node))
		return false;
	uint ChildCount = GetChildCount(Node);
	return ChildCount == 0;
	}

// Replace current origin with new node inserted
// between Node and its Parent.
void TreeX::InsertRootAbove(uint Node)
	{
	AssertNode(Node);
	asserta(!m_Rooted);

#if DEBUG
	set<string> LabelSetBefore;
	GetLeafLabelSet(LabelSetBefore);
	double SumLengthsBefore = GetSumLengths();
#endif

	uint Parent = m_NodeToParent[Node];
	asserta(Parent != UINT_MAX);

	double Length = GetLength(Node);
	if (Length != DBL_MAX)
		Length /= 2;
	//m_NodeToLength[Node] = Length;

	m_NodeBelowRoot = Node;
	m_NodeAboveRoot = Parent;

	m_TX = new TreeX;
	m_TX->Init();
	m_TX->Alloc(m_AssignedNodeCount);
	m_TX->m_AssignedNodeCount = m_AssignedNodeCount;
	m_TX->m_Labels = m_Labels;
	uint Root = m_TX->InsertNode(UINT_MAX, Length, "Root");
	m_TX->AddChildToBuffer(Root, Node);
	m_TX->AddChildToBuffer(Root, Parent);

	m_TX->SetParent(Node, Root);
	m_TX->SetParent(Parent, Root);

	m_TX->m_Origin = Root;

	Traverse_Preorder(Node, Parent, this, StaticOnNode_InsertRootAbove);
	Traverse_Preorder(Parent, Node, this, StaticOnNode_InsertRootAbove);

	m_TX->SetLength(Node, Length);
	m_TX->SetLength(Parent, Length);
	m_TX->SetLength(Root, DBL_MAX);

	m_TX->LogMe();
	m_TX->DrawText();
	m_TX->m_Rooted = true;
//	m_TX->Validate();

#if DEBUG
	set<string> LabelSetAfter;
	m_TX->GetLeafLabelSet(LabelSetAfter);
	double SumLengthsAfter = m_TX->GetSumLengths();
	asserta(LabelSetBefore == LabelSetAfter);
	asserta(feq(SumLengthsBefore, SumLengthsAfter));
#endif

	m_TX->Copy(*this);
	Validate();
	}

uint TreeX::GetEdgeCountToOrigin(uint Node) const
	{
	uint Count = 0;
	for (uint i = 0; i < m_AssignedNodeCount; ++i)
		{
		if (Node == m_Origin)
			return Count;
		Node = GetParent(Node);
		++Count;
		}
	Die("Cycle in tree");
	return UINT_MAX;
	}

uint TreeX::GetLCANode_Rooted(uint Node1, uint Node2) const
	{
	vector<uint> Path1;
	vector<uint> Path2;
	GetPathToOrigin2(Node1, Path1);
	GetPathToOrigin2(Node2, Path2);

	set<uint> PathSet1;
	for (uint i = 0; i < SIZE(Path1); ++i)
		PathSet1.insert(Path1[i]);

	for (uint i = 0; i < SIZE(Path2); ++i)
		{
		uint Node = Path2[i];
		if (PathSet1.find(Node) != PathSet1.end())
			return Node;
		}
	asserta(false);
	return UINT_MAX;
	}

// DOES include Node or Origin
void TreeX::GetPathToOrigin2(uint Node, vector<uint> &Path) const
	{
	Path.resize(0);
	for (;;)
		{
		Path.push_back(Node);
		if (Node == m_Origin || Node == UINT_MAX)
			return;
		Node = GetParent(Node);
		if (SIZE(Path) > m_AssignedNodeCount)
			Die("Cycle in tree");
		}
	}

// Does NOT include Node or Origin
void TreeX::GetPathToOrigin(uint Node, vector<uint> &Path) const
	{
	Path.resize(0);
	for (;;)
		{
		if (Node == m_Origin || Node == UINT_MAX)
			return;
		Node = GetParent(Node);
		if (Node == m_Origin || Node == UINT_MAX)
			return;
		Path.push_back(Node);
		if (SIZE(Path) > m_AssignedNodeCount)
			Die("Cycle in tree");
		}
	}

bool TreeX::IsEdge(uint Node1, uint Node2) const
	{
	vector<uint> Neighbors;
	GetNeighbors(Node1, Neighbors);
	for (uint i = 0; i < SIZE(Neighbors); ++i)
		if (Neighbors[i] == Node2)
			return true;
	return false;
	}

void TreeX::GetNeighbors(uint Node, vector<uint> &Neighbors) const
	{
	Neighbors.resize(0);

	bool Found = false;
	uint Parent = GetParent(Node);
	if (Parent != UINT_MAX)
		Neighbors.push_back(Parent);

	const uint ChildCount = GetChildCount(Node);
	const uint *Children = GetChildren(Node);
	for (uint i = 0; i < ChildCount; ++i)
		{
		uint Child = Children[i];
		Neighbors.push_back(Child);
		}
	}

void TreeX::Traverse_Preorder(uint PrevNode, uint Node, void *UserData, ptrOnNodeFn OnNode)
	{
	m_NodeToPrev[Node] = PrevNode;

	OnNode(PrevNode, Node, UserData);

	vector<uint> Neighbors;
	GetNeighbors(Node, Neighbors);
	bool Found = false;
	for (uint i = 0; i < SIZE(Neighbors); ++i)
		{
		uint Neighbor = Neighbors[i];
		if (Neighbor == PrevNode)
			{
			asserta(!Found);
			Found = true;
			}
		else
			Traverse_Preorder(Node, Neighbor, UserData, OnNode);
		}
	asserta(Found);

	}

void TreeX::Traverse_Inorder_Binary(uint Node, void *UserData, ptrOnNodeFn OnNode)
	{
	vector<uint> Neighbors;
	GetNeighbors(Node, Neighbors);
	uint n = SIZE(Neighbors);
	if (n == 0)
		Traverse_Inorder_Binary(Node, UserData, OnNode);
	else if (n == 2)
		{
		Traverse_Inorder_Binary(Neighbors[0], UserData, OnNode);
		Traverse_Inorder_Binary(Node, UserData, OnNode);
		Traverse_Inorder_Binary(Neighbors[1], UserData, OnNode);
		}
	else
		Die("TraverseInorder, Node %u is %u-ary", Node, n);
	}

void TreeX::Traverse_Postorder(uint PrevNode, uint Node, void *UserData, ptrOnNodeFn OnNode)
	{
	vector<uint> Neighbors;
	GetNeighbors(Node, Neighbors);
	for (uint i = 0; i < SIZE(Neighbors); ++i)
		Traverse_Postorder(Node, Neighbors[i], UserData, OnNode);
	OnNode(PrevNode, Node, UserData);
	}

/***
https://en.wikipedia.org/wiki/Breadth-first_search

 1  procedure BFS(G, root) is
 2      let Q be a queue
 3      label root as explored
 4      Q.enqueue(root)
 5      while Q is not empty do
 6          v := Q.dequeue()
 7          if v is the goal then
 8              return v
 9          for all edges from v to w in G.adjacentEdges(v) do
10              if w is not labeled as explored then
11                  label w as explored
12                  w.parent := v
13                  Q.enqueue(w)

***/

void TreeX::Traverse_BreadthFirst(uint StartNode, void *UserData,
  ptrOnNodeFn OnNode)
	{
	m_PendingNodes.clear();
	m_VisitedNodes.clear();

	m_NodeToPrev[StartNode] = UINT_MAX;
	m_PendingNodes.push_back(StartNode);
	vector<uint> Neighbors;
	while (!m_PendingNodes.empty())
		{
		const uint Node = m_PendingNodes.back();
		m_PendingNodes.pop_back();
		asserta(m_VisitedNodes.find(Node) == m_VisitedNodes.end());
		uint PrevNode = m_NodeToPrev[Node];
		bool Ok = OnNode(PrevNode, Node, UserData);
		if (!Ok)
			return;
		m_VisitedNodes.insert(Node);
		GetNeighbors(Node, Neighbors);
		const uint n = SIZE(Neighbors);
		for (uint i = 0; i < n; ++i)
			{
			uint NextNode = Neighbors[i];
			if (m_VisitedNodes.find(NextNode) == m_VisitedNodes.end())
				{
				m_NodeToPrev[NextNode] = Node;
				m_PendingNodes.push_back(NextNode);
				}
			}
		}
	}

bool TreeX::StaticOnNode_LCA(uint PrevNode, uint Node, void *UserData)
	{
	TreeX *TX = (TreeX *) UserData;
	bool Ok = TX->OnNode_LCA(PrevNode, Node);
	return Ok;
	}

bool TreeX::StaticOnNode_InsertRootAbove(uint PrevNode, uint Node, void *UserData)
	{
	TreeX *TX = (TreeX *) UserData;
	bool Ok = TX->OnNode_InsertRootAbove(PrevNode, Node);
	return Ok;
	}

bool TreeX::OnNode_InsertRootAbove(uint PrevNode, uint Node)
	{
	double Length = DBL_MAX;
	uint NodeParent = GetParent(Node);
	uint PrevNodeParent = GetParent(PrevNode);
	if (PrevNode == NodeParent)
		Length = GetLength(Node);
	else if (Node == PrevNodeParent)
		Length = GetLength(PrevNode);
	else
		asserta(false);

	m_TX->SetLength(Node, Length);
	m_TX->m_NodeToChildCount[Node] = 0;
	if (PrevNode != UINT_MAX && Node != m_NodeBelowRoot)
		{
		if (m_TX->m_NodeToChildCount[PrevNode] == UINT_MAX)
			m_TX->m_NodeToChildCount[PrevNode] = 0;
		m_TX->AddChildToBuffer(PrevNode, Node);
		}
	if (Node != m_NodeAboveRoot && Node != m_NodeBelowRoot)
		m_TX->SetParent(Node, PrevNode);

	return true;
	}

bool TreeX::OnNode_LCA(uint PrevNode, uint Node)
	{
	if (IsLeaf(Node))
		{
		if (m_LeafNodeSet->find(Node) == m_LeafNodeSet->end())
			m_OtherLeafNodeSet.insert(Node);
		else
			m_FoundLeafNodeSet.insert(Node);
		}
	if (SIZE(m_FoundLeafNodeSet) == SIZE(*m_LeafNodeSet))
		return false;
	return true;
	}

double TreeX::GetSumLengths() const
	{
	double Sum = 0;
	for (uint Node = 0; Node < m_AssignedNodeCount; ++Node)
		if (IsNode(Node) && Node != m_Origin)
			{
			double Len = GetLength(Node);
			if (Len != DBL_MAX)
				Sum += Len;
			}
	return Sum;
	}

void TreeX::GetLeafLabelSet(set<string> &LabelSet, bool ErrorIfEmpty,
  bool ErrorIfDupe) const
	{
	LabelSet.clear();
	for (uint Node = 0; Node < m_AssignedNodeCount; ++Node)
		{
		if (IsLeaf(Node))
			{
			const string &Label = GetLabel(Node);
			if (Label.empty())
				{
				if (ErrorIfEmpty)
					Die("Empty leaf label");
				}
			else
				{
				if (LabelSet.find(Label) != LabelSet.end())
					{
					if (ErrorIfDupe)
						Die("Dupe label >%s", Label.c_str());
					}
				LabelSet.insert(Label);
				}
			}
		}
	}

void TreeX::GetLeafLabels(vector<string> &Labels, bool ErrorIfEmpty) const
	{
	Labels.clear();
	for (uint Node = 0; Node < m_AssignedNodeCount; ++Node)
		{
		if (IsLeaf(Node))
			{
			const string &Label = GetLabel(Node);
			if (Label.empty())
				{
				if (ErrorIfEmpty)
					Die("Empty leaf label");
				}
			else
				Labels.push_back(Label);
			}
		}
	}

void TreeX::GetSubtreeLeafNodeSet_Rooted(uint Node, set<uint> &LeafNodeSet) const
	{
	LeafNodeSet.clear();
	vector<uint> LeafNodes;
	GetSubtreeLeafNodes_Rooted(Node, LeafNodes);
	for (uint i = 0; i < SIZE(LeafNodes); ++i)
		{
		uint LeafNode = LeafNodes[i];
		LeafNodeSet.insert(LeafNode);
		}
	}

void TreeX::AppendSubtreeLeafLabels_Rooted(uint Node, vector<string> &Labels) const
	{
	if (IsLeaf(Node))
		{
		const string &Label = GetLabel(Node);
		if (Label != "")
			Labels.push_back(Label);
		return;
		}
	const uint *Children = GetChildren(Node);
	uint ChildCount = GetChildCount(Node);
	for (uint i = 0; i < ChildCount; ++i)
		AppendSubtreeLeafLabels_Rooted(Children[i], Labels);
	}

void TreeX::GetSubtreeLeafLabels_Rooted(uint Node, vector<string> &Labels) const
	{
	Labels.clear();
	AppendSubtreeLeafLabels_Rooted(Node, Labels);
	}

void TreeX::GetSubtreeLeafNodes_Rooted(uint Node, vector<uint> &LeafNodes) const
	{
	asserta(m_Rooted);
	LeafNodes.resize(0);
	AppendSubtreeLeafNodes_Rooted(Node, LeafNodes);
	}

void TreeX::AppendSubtreeLeafNodes_Rooted(uint Node, vector<uint> &LeafNodes) const
	{
	if (IsLeaf(Node))
		{
		LeafNodes.push_back(Node);
		return;
		}
	const uint *Children = GetChildren(Node);
	uint ChildCount = GetChildCount(Node);
	for (uint i = 0; i < ChildCount; ++i)
		AppendSubtreeLeafNodes_Rooted(Children[i], LeafNodes);
	}

void TreeX::GetSubtreeLeafNodes(uint FromNode, uint ToNode, vector<uint> &LeafNodes) const
	{
	LeafNodes.resize(0);
	AppendLeafNodesInOrder(FromNode, ToNode, LeafNodes);
	}

void TreeX::GetSubtreeLeafNodes(uint FromNode, uint ToNode, set<uint> &LeafNodes) const
	{
	LeafNodes.clear();
	AppendLeafNodesInOrder(FromNode, ToNode, LeafNodes);
	}

void TreeX::AppendLeafNodesInOrder(uint FromNode, uint ToNode, vector<uint> &LeafNodes) const
	{
	if (IsLeaf(ToNode))
		{
		LeafNodes.push_back(ToNode);
		return;
		}

	vector<uint> Neighbors;
	GetNeighbors(ToNode, Neighbors);
	const uint n = SIZE(Neighbors);
	bool Found = false;
	for (uint i = 0; i < n; ++i)
		{
		uint Neighbor = Neighbors[i];
		if (Neighbor == FromNode)
			Found = true;
		else
			AppendLeafNodesInOrder(ToNode, Neighbor, LeafNodes);
		}
	asserta(Found || FromNode == UINT_MAX);
	}

void TreeX::AppendLeafNodesInOrder(uint FromNode, uint ToNode, set<uint> &LeafNodes) const
	{
	if (IsLeaf(ToNode))
		{
		LeafNodes.insert(ToNode);
		return;
		}

	vector<uint> Neighbors;
	GetNeighbors(ToNode, Neighbors);
	const uint n = SIZE(Neighbors);
	bool Found = false;
	for (uint i = 0; i < n; ++i)
		{
		uint Neighbor = Neighbors[i];
		if (Neighbor == FromNode)
			Found = true;
		else
			AppendLeafNodesInOrder(ToNode, Neighbor, LeafNodes);
		}
	asserta(Found || FromNode == UINT_MAX);
	}

void TreeX::GetLeafNodesInOrder(vector<uint> &LeafNodes) const
	{
	LeafNodes.resize(0);
	AppendLeafNodesInOrder(UINT_MAX, m_Origin, LeafNodes);
	}

void TreeX::LogBreadthFirstState(const string &Msg) const
	{
	Log("\n");
	Log("BreadthFirstState(%s)\n", Msg.c_str());
	Log("Pending(%u) : ", SIZE(m_PendingNodes));
	for (list<uint>::const_iterator p = m_PendingNodes.begin();
	  p != m_PendingNodes.end(); ++p)
		{
		uint Node = *p;
		uint Prev = m_NodeToPrev[Node];
		if (Prev == UINT_MAX)
			Log(" START->%u", *p);
		else
			Log(" %u->%u", Prev, Node);
		}
	Log("\n");
	Log("Visited(%u) ", SIZE(m_VisitedNodes));
	for (set<uint>::const_iterator p = m_VisitedNodes.begin();
	  p != m_VisitedNodes.end(); ++p)
		Log(" %u", *p);
	Log("\n");
	}

void TreeX::GetLCAEdge(const set<uint> &LeafNodes,
  uint &FromNode, uint &ToNode)
	{
	asserta(!LeafNodes.empty());

	uint InputLeafCount = SIZE(LeafNodes);
	m_LeafNodeSet = &LeafNodes;
	m_FoundLeafNodeSet.clear();
	m_OtherLeafNodeSet.clear();

	for (set<uint>::const_iterator p = LeafNodes.begin();
	  p != LeafNodes.end(); ++p)
		asserta(IsLeaf(*p));

	uint LeafNode = *LeafNodes.begin();
	uint Parent = GetParent(LeafNode);
	Traverse_BreadthFirst(LeafNode, this, StaticOnNode_LCA);
	//LogBreadthFirstState();
	uint n = SIZE(m_PendingNodes);
	asserta(n > 0);
	if (n == 1)
		{
		ToNode = *m_PendingNodes.begin();
		FromNode = m_NodeToPrev[ToNode];
		}
	else
		{
		uint BestCount = UINT_MAX;
		for (list<uint>::const_iterator p = m_PendingNodes.begin();
		  p != m_PendingNodes.end(); ++p)
			{
			uint FromNode2 = *p;
			uint ToNode2 = m_NodeToPrev[FromNode2];
			uint LeafCount = GetSubtreeLeafCount(FromNode2, ToNode2);
			asserta(LeafCount >= InputLeafCount);
			if (LeafCount == InputLeafCount)
				{
				FromNode = FromNode2;
				ToNode = ToNode2;
				return;
				}
			else if (BestCount == UINT_MAX || LeafCount < BestCount)
				{
				FromNode = FromNode2;
				ToNode = ToNode2;
				BestCount = LeafCount;
				}
			}
		}
	}

uint TreeX::GetSubtreeLeafCount_Rooted(uint Node) const
	{
	if (IsLeaf(Node))
		return 1;

	uint Sum = 0;
	const uint *Children = GetChildren(Node);
	uint ChildCount = GetChildCount(Node);
	for (uint i = 0; i < ChildCount; ++i)
		Sum += GetSubtreeLeafCount_Rooted(Children[i]);
	return Sum;
	}

uint TreeX::GetSubtreeLeafCount(uint FromNode, uint ToNode) const
	{
	asserta(IsEdge(FromNode, ToNode));

	set<pair<uint, uint> > VisitedEdges;
	list<pair<uint, uint> > PendingEdges;

	uint LeafCount = 0;
	vector<uint> Neighbors;
	PendingEdges.push_back(pair<uint, uint>(FromNode, ToNode));
	while (!PendingEdges.empty())
		{
		pair<uint, uint> Edge = PendingEdges.back();
		PendingEdges.pop_back();
		if (IsLeaf(Edge.first))
			++LeafCount;
		if (IsLeaf(Edge.second))
			++LeafCount;
		VisitedEdges.insert(Edge);
		GetNeighbors(Edge.second, Neighbors);
		uint ToNode = Edge.first;
		const uint n = SIZE(Neighbors);
		bool Found = false;
		pair<uint, uint> NextEdge(Edge.second, UINT_MAX);
		for (uint i = 0; i < n; ++i)
			{
			if (Neighbors[i] == ToNode)
				Found = true;
			else
				{
				NextEdge.second = Neighbors[i];
				assert(IsEdge(NextEdge.first, NextEdge.second));
				pair<uint, uint> ReverseNextEdge(NextEdge.second, NextEdge.first);
				if (VisitedEdges.find(NextEdge) == VisitedEdges.end() &&
				  VisitedEdges.find(ReverseNextEdge) == VisitedEdges.end())
					PendingEdges.push_back(NextEdge);
				}
			}
		asserta(Found);
		}
	return LeafCount;
	}

void TreeX::ToNewickStr(string &Str, bool WithLineBreaks) const
	{
	const uint OriginChildCount = GetChildCount(m_Origin);
	Str = "(";
	for (uint i = 0; i < OriginChildCount; ++i)
		{
		if (i > 0)
			{
			Str += ",";
			if (WithLineBreaks)
				Str += "\n";
			}
		uint Child = GetChild(m_Origin, i);
		AppendNodeToNewickStr(Str, Child, WithLineBreaks);
		}
	Str += ")";
	string OriginLabel;
	GetNewickLabel(m_Origin, OriginLabel);
	if (OriginLabel != "")
		Str += OriginLabel;
	double OriginLength = GetLength(m_Origin);
	if (OriginLength != DBL_MAX)
		Psa(Str, ":%.4g", OriginLength);
	Str += ";\n";
	}

void TreeX::ToNewickFile(const string &FileName, bool WithLineBreaks) const
	{
	FILE *f = CreateStdioFile(FileName);
	ToNewickFile(f, WithLineBreaks);
	CloseStdioFile(f);
	}

void TreeX::ToNewickFile(FILE *f, bool WithLineBreaks) const
	{
	if (f == 0)
		return;
	string Str;
	ToNewickStr(Str, WithLineBreaks);
	fputs(Str.c_str(), f);
	}

void TreeX::AppendNodeToNewickStr(string &Str, uint Node, bool WithLineBreaks) const
	{
	if (!IsLeaf(Node))
		{
		const uint *Children = GetChildren(Node);
		const uint ChildCount = GetChildCount(Node);
		Str += "(";
		for (uint i = 0; i < ChildCount; ++i)
			{
			if (i > 0)
				Str += ",";
			uint Child = Children[i];
			AppendNodeToNewickStr(Str, Child, WithLineBreaks);
			}
		Str += ")";
		}

	double Length = GetLength(Node);
	string Label;
	GetNewickLabel(Node, Label);
	Str += Label;
	if (Length != DBL_MAX)
		Psa(Str, ":%.4g", Length);
	if (WithLineBreaks)
		Str += "\n";
	}

static bool IsValidNewickLabel(const string &Label)
	{
	const uint n = SIZE(Label);
	for (uint i = 0; i < n; ++i)
		{
		char c = Label[i];
		if (isspace(c))
			return false;
		if (c == '(' || c == ')' || c == ':' || c == ';' || c == '[' || c == ']')
			return false;
		}
	return true;
	}

void TreeX::GetNewickLabel(uint Node, string &Label) const
	{
	Label = GetLabel(Node);
	if (!opt(allow_blank_labels) && Label.empty() && IsLeaf(Node))
		Label = "_";
	bool Ok = IsValidNewickLabel(Label);
	if (!Ok)
		Label = "'" + Label + "'";
	}

void TreeX::DrawText(FILE *f, bool WithLengths) const
	{
	if (f == 0)
		return;

	const uint XM = WithLengths ? 12 : 6;
	const uint YM = 2;

	m_DrawLines.clear();
	m_DrawXs.clear();
	m_DrawYs.clear();
	m_DrawSumYs.clear();
	m_DrawNs.clear();

	m_DrawXs.resize(m_AssignedNodeCount);
	m_DrawYs.resize(m_AssignedNodeCount);
	m_DrawSumYs.resize(m_AssignedNodeCount, 0);
	m_DrawNs.resize(m_AssignedNodeCount, 0);

	m_DrawMaxY = 0;
	vector<uint> LeafNodes;
	GetLeafNodesInOrder(LeafNodes);
	uint LeafNodeCount = SIZE(LeafNodes);
	for (uint Y = 0; Y < LeafNodeCount; ++Y)
		{
		uint Node = LeafNodes[Y];
		asserta(Node < m_AssignedNodeCount);
		const string &Label = GetLabel(Node);
		m_DrawYs[Node] = Y*YM;
		}
	m_DrawMaxY = YM*LeafNodeCount + 2;

	vector<uint> Path;
	for (uint Y = 0; Y < LeafNodeCount; ++Y)
		{
		uint Node = LeafNodes[Y];
		GetPathToOrigin(Node, Path);
		for (uint i = 0; i < SIZE(Path); ++i)
			{
			uint Node2 = Path[i];
			asserta(!IsLeaf(Node2));
			m_DrawYs[Node2] += Y*YM;
			m_DrawNs[Node2] += 1;
			}
		}

	for (uint Node = 0; Node < m_AssignedNodeCount; ++Node)
		{
		if (!IsNode(Node))
			continue;
		if (IsLeaf(Node))
			continue;
		if (Node == m_Origin)
			m_DrawYs[m_Origin] = YM*LeafNodeCount/2;
		else
			{
			uint SumY = m_DrawYs[Node];
			uint N = m_DrawNs[Node];
			if (N == 0)
				Die("m_DrawNs[%u] = 0", Node);
			uint AvgY = SumY/N;
			asserta(AvgY <= m_DrawMaxY);
			m_DrawYs[Node] = AvgY;
			}
		}

	m_DrawMaxX = 0;
	for (uint Node = 0; Node < m_AssignedNodeCount; ++Node)
		{
		if (!IsNode(Node))
			continue;
		uint X = 1 + GetEdgeCountToOrigin(Node)*XM;
		m_DrawXs[Node] = X;
		m_DrawMaxX = max(X, m_DrawMaxX);
		}

	for (uint Node = 0; Node < m_AssignedNodeCount; ++Node)
		{
		if (!IsNode(Node))
			continue;
		uint X = m_DrawXs[Node];
		uint Y = m_DrawYs[Node];
		uint Parent = GetParent(Node);
		if (Parent == UINT_MAX)
			{
			DrawPt(X, Y, '*');
			continue;
			}
		uint XP = m_DrawXs[Parent];
		uint YP = m_DrawYs[Parent];
		asserta(XP < X);
		for (uint x = XP + 1; x + 1 < X; ++x)
			DrawPt(x, Y, '-');
		DrawPt(X, Y, '|');
		if (WithLengths)
			{
			double Length = GetLength(Node);
			if (Length != DBL_MAX)
				{
				string s;
				Ps(s, "<%.3g>", Length);
				DrawStr(XP+2, Y, s);
				}
			}

		uint y1 = min(YP, Y);
		uint y2 = max(YP, Y);
		for (uint y = y1 + 1; y + 1 <= y2; ++y)
			DrawPt(XP, y, '|');
		}

	for (uint Node = 0; Node < m_AssignedNodeCount; ++Node)
		{
		if (!IsNode(Node))
			continue;
		uint X = m_DrawXs[Node];
		uint Y = m_DrawYs[Node];
		if (IsLeaf(Node))
			{
			const string &Label = GetLabel(Node);
			string s = Label;
			Psa(s, "(%u)", Node);
			DrawStr(X, Y, s);
			}
		else
			{
			string sP;
			Ps(sP, "%u", Node);
			DrawStr(X+1, Y, sP);
			}
		}

	asserta(m_DrawMaxX < 1000);
	asserta(m_DrawMaxY < 1000);
	fputc('\n', f);
	for (uint i = 0; i < SIZE(m_DrawLines); ++i)
		fprintf(f, "%s\n", m_DrawLines[i].c_str());
	fputc('\n', f);
	}

void TreeX::DrawPt(uint x, uint y, char c) const
	{
	asserta(x <= m_DrawMaxX + 100);
	asserta(y <= m_DrawMaxY + 100);
	if (y >= SIZE(m_DrawLines))
		m_DrawLines.resize(y+1);
	if (x >= SIZE(m_DrawLines[y]))
		m_DrawLines[y].resize(x+1, ' ');
	m_DrawLines[y][x] = c;
	}

void TreeX::DrawStr(uint x, uint y, const string &s) const
	{
	for (uint i = 0; i < SIZE(s); ++i)
		DrawPt(x+i, y, s[i]);
	}

uint TreeX::GetNodeCount() const
	{
	uint n = 0;
	for (uint i = 0; i < m_AssignedNodeCount; ++i)
		if (IsNode(i))
			++n;
	return n;
	}

uint TreeX::GetLeafCount() const
	{
	uint n = 0;
	for (uint i = 0; i < m_AssignedNodeCount; ++i)
		if (IsLeaf(i))
			++n;
	return n;
	}

bool TreeX::IsRoot(uint Node) const
	{
	if (!m_Rooted)
		return false;
	return Node == m_Origin;
	}

bool TreeX::IsRootedBinary() const
	{
	if (!m_Rooted)
		return false;
	return IsBinarySubtree(m_Origin);
	}

bool TreeX::IsBinarySubtree(uint Node) const
	{
	uint ChildCount = GetChildCount(Node);
	if (ChildCount == 0)
		return true;
	else if (ChildCount == 2)
		return
		  IsBinarySubtree(GetLeft(Node)) &&
		  IsBinarySubtree(GetRight(Node));
	else
		return false;
	}

bool TreeX::IsNormalized() const
	{
	for (uint i = 0; i < m_AssignedNodeCount; ++i)
		if (!IsNode(i))
			return false;
	return true;
	}

void TreeX::Normalize()
	{
	if (IsNormalized())
		return;

#if DEBUG
	double SumLengthsBefore = GetSumLengths();
	set<string> LabelSetBefore;
	GetLeafLabelSet(LabelSetBefore, false, true);
#endif

	vector<uint> OldNodeToNewNode(m_AssignedNodeCount, UINT_MAX);
	vector<uint> NewNodeToOldNode(m_AssignedNodeCount, UINT_MAX);

	uint NewNode = 0;
	vector<string> NewLabels;
	vector<double> NewLengths;
	vector<uint> NewChildCounts;
	vector<vector<uint> > NewNodeToChildren;
	for (uint OldNode = 0; OldNode < m_AssignedNodeCount; ++OldNode)
		{
		if (!IsNode(OldNode))
			continue;

		OldNodeToNewNode[OldNode] = NewNode;
		NewNodeToOldNode[NewNode] = OldNode;

		const string &Label = GetLabel(OldNode);
		double Length = GetLength(OldNode);
		uint ChildCount = GetChildCount(OldNode);

		vector<uint> Children;
		for (uint i = 0; i < ChildCount; ++i)
			Children.push_back(GetChild(OldNode, i));
		NewNodeToChildren.push_back(Children);

		NewLabels.push_back(Label);
		NewLengths.push_back(Length);
		NewChildCounts.push_back(ChildCount);

		++NewNode;
		}
	asserta(NewNode <= m_AssignedNodeCount);

	const uint NewNodeCount = NewNode;

	vector<uint> NewParents;
	for (uint NewNode = 0; NewNode < NewNodeCount; ++NewNode)
		{
		uint OldNode = NewNodeToOldNode[NewNode];
		uint OldParent = GetParent(OldNode);
		uint NewParent = UINT_MAX;
		if (OldParent != UINT_MAX)
			NewParent = OldNodeToNewNode[OldParent];
		NewParents.push_back(NewParent);
		}

	asserta(SIZE(NewLengths) == NewNodeCount);
	asserta(SIZE(NewParents) == NewNodeCount);
	asserta(SIZE(NewChildCounts) == NewNodeCount);
	for (uint NewNode = 0; NewNode < NewNodeCount; ++NewNode)
		{
		double NewLength = NewLengths[NewNode];
		uint NewParent = NewParents[NewNode];
		uint NewChildCount = NewChildCounts[NewNode];
		const vector<uint> &Children = NewNodeToChildren[NewNode];
		asserta(SIZE(Children) == NewChildCount);

		m_NodeToLength[NewNode] = NewLength;
		m_NodeToParent[NewNode] = NewParent;
		SetChildCount(NewNode, NewChildCount);

		uint *ChildrenBuffer = m_NodeToChildren[NewNode];
		for (uint i = 0; i < NewChildCount; ++i)
			ChildrenBuffer[i] = Children[i];
		}

// Hash table and labels
	m_Labels = NewLabels;
	m_LabelToNode.clear();
	for (uint NewNode = 0; NewNode < NewNodeCount; ++NewNode)
		{
		if (NewChildCounts[NewNode] == 0)
			{
			const string &NewLabel = NewLabels[NewNode];
			if (NewLabel != "")
				m_LabelToNode[NewLabel] = NewNode;
			}
		}

	m_AssignedNodeCount = NewNodeCount;

	Validate();

#if DEBUG
	double SumLengthsAfter = GetSumLengths();
	set<string> LabelSetAfter;
	GetLeafLabelSet(LabelSetAfter, false, true);
	asserta(LabelSetBefore == LabelSetAfter);
	asserta(feq(SumLengthsBefore, SumLengthsAfter));
#endif
	}

uint TreeX::GetBestFitSubtree(const set<uint> &GroupLeafNodeSet,
  double MinTPFract, uint &TP, uint &FP, uint &FN)
	{
	asserta(MinTPFract > 0 && MinTPFract <= 1);
	asserta(m_Rooted);
	const uint GroupSize = SIZE(GroupLeafNodeSet);
	vector<uint> Path;
	map<uint, uint> NodeToGroupCount;
	for (set<uint>::const_iterator p = GroupLeafNodeSet.begin();
	  p != GroupLeafNodeSet.end(); ++p)
		{
		uint LeafNode = *p;
		NodeToGroupCount[LeafNode] = 1;

		GetPathToOrigin(LeafNode, Path);
		const uint PathLength = SIZE(Path);
		for (uint j = 0; j < PathLength; ++j)
			{
			uint InternalNode = Path[j];
			asserta(InternalNode != m_Origin);
			if (NodeToGroupCount.find(InternalNode) ==
			  NodeToGroupCount.end())
				NodeToGroupCount[InternalNode] = 1;
			else
				{
				uint n = NodeToGroupCount[InternalNode] + 1;
				NodeToGroupCount[InternalNode] = n;
				if (n == GroupSize)
					break;
				}
			}
		}

	uint MinTP = uint(MinTPFract*GroupSize + 0.5);
	if (MinTP == 0)
		MinTP = 1;
	else if (MinTP > GroupSize)
		MinTP = GroupSize;

	uint BestNode = UINT_MAX;
	uint BestErrs = UINT_MAX;
	for (map<uint, uint>::const_iterator p = NodeToGroupCount.begin();
	  p != NodeToGroupCount.end(); ++p)
		{
		uint Node = p->first;
		uint SubtreeTP = p->second;
		if (SubtreeTP < MinTP)
			continue;

		uint SubtreeSize = GetSubtreeLeafCount_Rooted(Node);
		asserta(SubtreeSize >= SubtreeTP);
		uint SubtreeFP = SubtreeSize - SubtreeTP;
		uint SubtreeFN = GroupSize - SubtreeTP;
		uint Errs = SubtreeFP + 2*SubtreeFN;
		if (BestNode == UINT_MAX || Errs < BestErrs
		  || Errs == BestErrs && SubtreeTP > TP)
			{
			BestNode = Node;
			TP = SubtreeTP;
			FP = SubtreeFP;
			FN = SubtreeFN;
			BestErrs = Errs;
			}
		}
	return BestNode;
	}

/***
Insert subtree below ThisNode, adding a new node.
NodeT can be a leaf.

             ThisNode
         --------|--------
						  |
                   NewNode/NodeT   << new child in this tree
                         /|\       << existing subtree in T

***/
void TreeX::InsertSubtreeBelow(uint ThisNode, double Length,
  const TreeX &T, uint NodeT)
	{
	const string &Label = T.GetLabel(NodeT);
	uint NewNode = InsertNode(ThisNode, Length, Label);
	const uint *ChildrenT = T.GetChildren(NodeT);
	const uint ChildCountT = T.GetChildCount(NodeT);
	for (uint i = 0; i < ChildCountT; ++i)
		{
		uint ChildT = ChildrenT[i];
		double Length = T.GetLength(ChildT);
		InsertSubtreeBelow(NewNode, Length, T, ChildT);
//		Validate();//@@
		}
	}

/***
              Parent
                |
          ------------
         |            | Length/2
                   NewNode
                      | Length/2
                    Node
***/
uint TreeX::InsertNodeBetweenNodeAndParent(uint Node,
  const string &Label)
	{
	uint Parent = m_NodeToParent[Node];
	asserta(Parent != UINT_MAX);

	double Length = GetLength(Node);
	if (Length != DBL_MAX)
		Length /= 2;

	SetLength(Node, Length);
	uint NewNode = InsertOrphanNode(Length, Label);
	AddChildToBuffer(NewNode, Node);
	SetParent(NewNode, Parent);
	SetParent(Node, NewNode);
	ReplaceChildInBuffer(Parent, Node, NewNode);

	return NewNode;
	}

void TreeX::LabelSetToLeafNodeSet(const set<string> &LabelSet,
  set<uint> &LeafNodeSet) const
	{
	LeafNodeSet.clear();
	for (set<string>::const_iterator p = LabelSet.begin();
	  p != LabelSet.end(); ++p)
		{
		const string &Label = *p;
		uint Node = GetNodeByLabel(Label);
		LeafNodeSet.insert(Node);
		}
	}

uint TreeX::GetSibling(uint Node, bool FailIfNone) const
	{
	uint Parent = GetParent(Node);
	if (Parent == UINT_MAX)
		{
		if (FailIfNone)
			Die("GetSibling(%u), no parent", Node);
		return UINT_MAX;
		}

	uint ParentChildCount = GetChildCount(Parent);
	if (ParentChildCount != 2)
		{
		if (FailIfNone)
			Die("GetSibling(%u), parent %u has %u children",
			  Node, Parent, ParentChildCount);
		return UINT_MAX;
		}

	const uint *ParentChildren = GetChildren(Parent);
	if (ParentChildren[0] == Node)
		return ParentChildren[1];
	else if (ParentChildren[1] == Node)
		return ParentChildren[0];

	Die("GetSibling(%u), not found (parent %u)", Node, Parent);
	return UINT_MAX;
	}

double TreeX::GetNodePairDist_Rooted(uint Node1, uint Node2) const
	{
	uint LCA = GetLCANode_Rooted(Node1, Node2);
	vector<uint> Path1;
	vector<uint> Path2;
	GetPathToOrigin2(Node1, Path1);
	GetPathToOrigin2(Node2, Path2);
	double Sum = 0;
	bool Found1 = false;
	for (uint i = 0; i < SIZE(Path1); ++i)
		{
		uint Node = Path1[i];
		if (Node == LCA)
			{
			Found1 = true;
			break;
			}
		double Dist = GetLength(Node);
		if (Dist != DBL_MAX)
			Sum += Dist;
		}
	asserta(Found1);

	bool Found2 = false;
	for (uint i = 0; i < SIZE(Path2); ++i)
		{
		uint Node = Path2[i];
		if (Node == LCA)
			{
			Found2 = true;
			break;
			}
		double Dist = GetLength(Node);
		if (Dist != DBL_MAX)
			Sum += Dist;
		}
	asserta(Found2);
	return Sum;
	}

double TreeX::GetMedianLeafDist(uint Node) const
	{
	vector<double> Dists;
	GetSubtreeLeafDistsSorted_Rooted(Node, Dists);
	uint N = SIZE(Dists);
	if (N == 0)
		return 0;
	double Med = Dists[N/2];
	return Med;
	}

void TreeX::GetSubtreeLeafDistsSorted_Rooted(uint Node, 
  vector<double> &Dists) const
	{
	Dists.resize(0);

	vector<uint> LeafNodes;
	GetSubtreeLeafNodes_Rooted(Node, LeafNodes);

	const uint N = SIZE(LeafNodes);
	for (uint i = 0; i < N; ++i)
		{
		uint LeafNode = LeafNodes[i];
		double Dist = GetNodePairDist_Rooted(LeafNode, Node);
		Dists.push_back(Dist);
		}
	QuickSortInPlace(Dists.data(), N);
	}

double TreeX::GetBootFract(uint Node) const
	{
	if (!IsNode(Node))
		return -1;
	if (IsLeaf(Node))
		return 1;
	const string &Label = GetLabel(Node);
	if (!IsValidFloatStr(Label))
		return -2;
	double b = StrToFloat(Label);
	if (b > 1 && b <= 100)
		b /= 100;
	return b;
	}

void TreeX::LadderizeNode(uint Node)
	{
	uint ChildCount = GetChildCount(Node);
	if (ChildCount <= 1)
		return;
	uint *Children = GetChildren(Node);

	vector<uint> Sizes;
	for (uint i = 0; i < ChildCount; ++i)
		{
		uint Child = Children[i];
		uint Size = GetSubtreeLeafCount_Rooted(Child);
		Sizes.push_back(Size);
		}

	vector<uint> NewChildren;
	vector<uint> Order(ChildCount);
	QuickSortOrder(Sizes.data(), ChildCount, Order.data());
	for (uint k = 0; k < ChildCount; ++k)
		{
		uint i = Order[k];
		uint Child = Children[i];
		NewChildren.push_back(Child);
		}
	memcpy(Children, NewChildren.data(),
	  ChildCount*sizeof(uint));
	}

void TreeX::Ladderize()
	{
	LadderizeNode(m_Origin);
	}

bool TreeX::IsInSubtree(uint SubtreeNode, uint Node) const
	{
	vector<uint> Path;
	GetPathToOrigin2(Node, Path);
	const uint N = SIZE(Path);
	for (uint i = 0; i < N; ++i)
		if (Path[i] == SubtreeNode)
			return true;
	return false;
	}

double TreeX::AddLengths(double Length1, double Length2)
	{
	if (Length1 != DBL_MAX && Length2 != DBL_MAX)
		return Length1 + Length2;
	else if (Length1 == DBL_MAX && Length2 == DBL_MAX)
		return DBL_MAX;
	else if (Length1 != DBL_MAX && Length2 == DBL_MAX)
		return Length1;
	else if (Length1 == DBL_MAX && Length2 != DBL_MAX)
		return Length2;
	else
		asserta(false);
	return DBL_MAX;
	}

void TreeX::CollapseUnary()
	{
	uint NodeCount = GetNodeIndexCount();
	for (;;)
		{
		bool Any = false;
		for (uint Node = 0; Node < NodeCount; ++Node)
			{
			if (!IsNode(Node))
				continue;
			if (Node == m_Origin)
				continue;
			uint ChildCount = GetChildCount(Node);
			if (ChildCount == 1)
				{
				Any = true;
				CollapseUnaryNode(Node);
				}
			}
		if (!Any)
			break;
		}
	}

// Merge Node with its parent.
// Node is deleted.
// Node's children become parent's children.
void TreeX::CollapseUnaryNode(uint Node)
	{
	asserta(Node != m_Origin);
	uint ChildCount = GetChildCount(Node);
	asserta(ChildCount == 1);
	uint Parent = GetParent(Node);
	asserta(Parent != UINT_MAX);
	const string &NodeLabel = GetLabel(Node);
	double NodeLength = GetLength(Node);
	double ParentLength = GetLength(Parent);
	double Length = AddLengths(NodeLength, ParentLength);
	m_NodeToLength[Parent] = Length;
	uint Child = GetChild(Node, 0);
	ReplaceChildInBuffer(Parent, Node, Child);
	SetParent(Child, Parent);
	UpdateLabel(Node, "");
	m_NodeToChildCount[Node] = UINT_MAX;
	UpdateLabel(Parent, NodeLabel);
#if DEBUG
	Validate();
#endif
	}
