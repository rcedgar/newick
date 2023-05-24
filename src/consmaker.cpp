#include "myutils.h"
#include "consmaker.h"

// Disjoint subtrees which cover the given labels
// and nothing else.
void TreeN::GetPureSubtrees(const set<string> &LabelSet,
  vector<uint> &SubtreeNodes) const
	{
	SubtreeNodes.clear();

	bool Rooted;
	bool Binary = IsBinary(Rooted);
	asserta(Binary && Rooted);
	asserta(IsNormalized());
	uint Root = GetRoot();

	const uint NodeCount = GetNodeCount();
	vector<uint> NodeToInCount(NodeCount, 0);
	vector<uint> NodeToOutCount(NodeCount, 0);

// Increment in-group counts
	for (set<string>::const_iterator p = LabelSet.begin();
	  p != LabelSet.end(); ++p)
		{
		const string &Label = *p;
		vector<uint> Path;
		uint Node = GetNodeByLabel(Label, true);
		GetPathToRoot(Node, Path);
		const uint n = SIZE(Path);
		for (uint i = 0; i < n; ++i)
			{
			uint Node2 = Path[i];
			asserta(Node2 < NodeCount);
			++NodeToInCount[Node2];
			}
		}

// Increment out-group counts
	for (map<string, uint>::const_iterator p = m_LabelToNode.begin();
	  p != m_LabelToNode.end(); ++p)
		{
		const string &Label = p->first;
		uint Node = p->second;
		if (LabelSet.find(Label) != LabelSet.end())
			continue;

		vector<uint> Path;
		GetPathToRoot(Node, Path);
		const uint n = SIZE(Path);
		for (uint i = 0; i < n; ++i)
			{
			uint Node2 = Path[i];
			asserta(Node2 < NodeCount);
			++NodeToOutCount[Node2];
			}
		}

	for (uint Node = 0; Node < NodeCount; ++Node)
		{
		uint Parent = GetParent(Node);
		if (Parent == UINT_MAX)
			{
			asserta(IsRoot(Node));
			continue;
			}

		bool IsPure = (NodeToInCount[Node] > 0 && NodeToOutCount[Node] == 0);
		bool ParentIsNotPure = (NodeToOutCount[Parent] != 0);
		if (IsPure && ParentIsNotPure)
			{
			asserta(NodeToInCount[Parent] > 0);
			SubtreeNodes.push_back(Node);
			}
		}
	if (SubtreeNodes.empty())
		{
		asserta(m_Root != UINT_MAX);
		asserta(NodeToInCount[m_Root] > 0);
		asserta(NodeToOutCount[m_Root] == 0);
		SubtreeNodes.push_back(m_Root);
		}

/////////////////////////////////////////////////
// Validate result
	{
	const uint SubtreeCount = SIZE(SubtreeNodes);
	set<string> FoundSet;
	for (uint i = 0; i < SubtreeCount; ++i)
		{
		uint SubtreeNode = SubtreeNodes[i];
		vector<uint> LeafNodes;

	// Subtree IS pure
		GetSubtreeLeafNodes(SubtreeNode, LeafNodes);
		for (uint j = 0; j < SIZE(LeafNodes); ++j)
			{
			uint LeafNode = LeafNodes[j];
			const string &LeafLabel = GetLabel(LeafNode);
			asserta(LabelSet.find(LeafLabel) != LabelSet.end());
			asserta(FoundSet.find(LeafLabel) == FoundSet.end());
			FoundSet.insert(LeafLabel);
			}

	// Subtree parent is NOT pure
		uint SubtreeNodeParent = GetParent(SubtreeNode);
		if (SubtreeNodeParent != UINT_MAX)
			{
			GetSubtreeLeafNodes(SubtreeNodeParent, LeafNodes);
			bool OutFound = false;
			for (uint j = 0; j < SIZE(LeafNodes); ++j)
				{
				uint LeafNode = LeafNodes[j];
				const string &LeafLabel = GetLabel(LeafNode);
				if (LabelSet.find(LeafLabel) == LabelSet.end())
					{
					OutFound = true;
					break;
					}
				}
			asserta(OutFound);
			}
		}

// Accounted for ALL labels
	asserta(SIZE(FoundSet) == SIZE(LabelSet));
	}
// End validate result
/////////////////////////////////////////////////
	}

void ConsMaker::IntersectLabels(const TreeN &Tree1, const TreeN &T2,
  set<string> &LabelsBoth, set<string> &Labels1Only, set<string> &Labels2Only,
  bool ErrorIfEmpty, bool ErrorIfDupe)
	{
	LabelsBoth.clear();
	Labels1Only.clear();
	Labels2Only.clear();

	set<string> Labels1;
	set<string> Labels2;

	Tree1.GetLeafLabelSet(Labels1, ErrorIfEmpty, ErrorIfDupe);
	T2.GetLeafLabelSet(Labels2, ErrorIfEmpty, ErrorIfDupe);

	for (set<string>::const_iterator p = Labels1.begin();
	  p != Labels1.end(); ++p)
		{
		const string &Label = *p;
		if (Labels2.find(Label) != Labels2.end())
			LabelsBoth.insert(Label);
		else
			Labels1Only.insert(Label);
		}

	for (set<string>::const_iterator p = Labels2.begin();
	  p != Labels2.end(); ++p)
		{
		const string &Label = *p;
		if (LabelsBoth.find(Label) == LabelsBoth.end() &&
		  Labels1Only.find(Label) == Labels1Only.end())
			Labels2Only.insert(Label);
		}
	}

// Special case for disjoint trees
void ConsMaker::MakeConsensus_Disjoint()
	{
	m_ConsTree.CreateSingleton();
	InsertSubtreeNode(*m_Tree1, m_Tree1->m_Root, 0);
	InsertSubtreeNode(*m_Tree2, m_Tree2->m_Root, 0);
	}

void ConsMaker::MakeConsensus(const TreeN &T1, const TreeN &T2)
	{
	asserta(m_Tree1 == 0 && m_Tree2 == 0);

	bool Rooted1;
	bool Rooted2;
	bool Binary1 = T1.IsBinary(Rooted1);
	bool Binary2 = T2.IsBinary(Rooted2);
	asserta(Binary1);
	asserta(Binary2);
	asserta(Rooted1);
	asserta(Rooted2);

	m_Tree1 = &T1;
	m_Tree2 = &T2;

	IntersectLabels(*m_Tree1, *m_Tree2,
	  m_LabelsBoth, m_Labels1Only, m_Labels2Only, true, true);

	const uint N12 = SIZE(m_LabelsBoth);
	const uint N1 = SIZE(m_Labels1Only);
	const uint N2 = SIZE(m_Labels2Only);

	ProgressLog("%u both, %u T1only, %u T2only\n", N12, N1, N2);

	if (N12 == 0)
		MakeConsensus_Disjoint();
	else if (N1 == 0)
		m_Tree2->CopyNormalized(m_ConsTree);
	else if (N2 == 0)
		m_Tree1->CopyNormalized(m_ConsTree);
	else
		{
	// Start from T1 -- TODO choose better tree
		m_Tree1->CopyNormalized(m_ConsTree);

	// Add from T2
		m_Tree2->GetPureSubtrees(m_Labels2Only, m_Pures2);
		const uint SubtreeCount = SIZE(m_Pures2);

		for (uint i = 0; i < SubtreeCount; ++i)
			{
			ProgressStep(i, SubtreeCount, "Subtrees");
			uint SubtreeNode2 = m_Pures2[i];
			AddSubtree(SubtreeNode2);
			}
		}
	ValidateConsensusLabels();

	bool RootedC = false;
	bool BinaryC = m_ConsTree.IsBinary(RootedC);
	asserta(BinaryC && RootedC);
	}

void ConsMaker::ValidateConsensusLabels()
	{
	set<string> LabelSet1;
	set<string> LabelSet2;
	m_Tree1->GetLeafLabelSet(LabelSet1, true, true);
	m_Tree2->GetLeafLabelSet(LabelSet2, true, true);

	for (set<string>::const_iterator p = LabelSet1.begin();
	  p != LabelSet1.end(); ++p)
		{
		const string &Label = *p;
		m_ConsTree.GetNodeByLabel(Label, true);
		}

	for (set<string>::const_iterator p = LabelSet2.begin();
	  p != LabelSet2.end(); ++p)
		{
		const string &Label = *p;
		m_ConsTree.GetNodeByLabel(Label, true);
		}
	}
 
// SubtreeNode2 has no labels from Tree1
// Sibling of SubtreeNode2 has at least one label from Tree1
// Find best subtree match to the Tree1 labels in Tree1
void ConsMaker::AddSubtree(uint SubtreeNode2)
	{
	asserta(!m_Tree2->IsRoot(SubtreeNode2));
	uint Sibling = m_Tree2->GetSibling(SubtreeNode2);

	vector<uint> SiblingNodes2;
	m_Tree2->AppendSubtreeLeafNodes(Sibling, SiblingNodes2);

	set<string> SiblingLabels2;
	set<uint> SiblingNodesC;
	for (uint i = 0; i < SIZE(SiblingNodes2); ++i)
		{
		uint Node2 = SiblingNodes2[i];
		const string &SiblingLabel = m_Tree2->GetLabel(Node2);
		uint NodeC = m_ConsTree.GetNodeByLabel(SiblingLabel, false);
		if (NodeC != UINT_MAX)
			SiblingNodesC.insert(NodeC);
		}
	asserta(!SiblingNodesC.empty());

	uint FPs = UINT_MAX;
	uint FNs = UINT_MAX;
	bool Invert;
	uint NodeC = m_ConsTree.GetBestFitSubtree1(SiblingNodesC,
	  false, FPs, FNs, Invert);
	if (NodeC == UINT_MAX)
		NodeC = m_ConsTree.m_Root;

	InsertSubtree(SubtreeNode2, NodeC);
	}

/***
Insert subtree from T2 above existing NodeC in ConsTree.
Maintain normalization of ConsTree (consecutive nodes 0, 1 ... N-1).


              ParentC
         --------|--------
						  | Length/2
						  |
                      NewNodeC
                   -------|-----
                  |             | Length/2
              Copy of           |
             subtree here      NodeC
                  
***/
void ConsMaker::InsertSubtree(uint SubtreeNode2, uint NodeC)
	{
#if DEBUG
	m_ConsTree.Validate();
#endif

	assert(m_ConsTree.IsNormalized());
	uint ChildCount = m_ConsTree.GetChildCount(NodeC);
	asserta(ChildCount == 0 || ChildCount == 2);

	string NewLabelC;
	Ps(NewLabelC, "Sub%u", m_NewSubtreeCount);
	++m_NewSubtreeCount;

// Split parent edge into equal-length halves
	double Length = m_ConsTree.GetLength(NodeC);
	double HalfLength = DBL_MAX;
	if (Length != DBL_MAX)
		HalfLength = Length/2;

	uint ParentC = m_ConsTree.GetParent(NodeC);
	uint NewNodeC = m_ConsTree.GetNodeCount();

	m_ConsTree.m_NodeToLabel[NewNodeC] = NewLabelC;
	m_ConsTree.m_LabelToNode[NewLabelC] = NewNodeC;

	m_ConsTree.m_NodeToParent[NodeC] = NewNodeC;
	m_ConsTree.m_NodeToParent[NewNodeC] = ParentC;

	m_ConsTree.m_NodeToLength[NodeC] = HalfLength;
	m_ConsTree.m_NodeToLength[NewNodeC] = HalfLength;

// Initially NewNodeC is unary with NodeC as its only child.
	vector<uint> NewChildren;
	NewChildren.push_back(NodeC);
	m_ConsTree.m_NodeToChildren[NewNodeC] = NewChildren;

// Update children of ParentC, replace NodeC by NewNodeC
	if (ParentC == UINT_MAX)
		asserta(NodeC == m_ConsTree.m_Root);
	else
		{
		map<uint, vector<uint> >::iterator p =
		  m_ConsTree.m_NodeToChildren.find(ParentC);
		asserta(p != m_ConsTree.m_NodeToChildren.end());
		vector<uint> &Children = p->second;
		bool Found = false;
		for (uint i = 0; i < SIZE(Children); ++i)
			{
			if (Children[i] == NodeC)
				{
				Found = true;
				Children[i] = NewNodeC;
				break;
				}
			}
		asserta(Found);
		}

#if DEBUG
	m_ConsTree.Validate();
#endif

// Add copy of subtree as second branch to NewNodeC
	InsertSubtreeNode(*m_Tree2, SubtreeNode2, NewNodeC);

	assert(m_ConsTree.IsNormalized());
	}

// ParentNodeC already exists
// Insert Node2 into ConsTree under ParentNodeC
void ConsMaker::InsertSubtreeNode(const TreeN &T, uint NodeT, uint ParentNodeC)
	{
#if DEBUG
	m_ConsTree.Validate();
#endif

	if (T.IsLeaf(NodeT))
		{
		const string &Label = T.GetLabel(NodeT);
		double Length = T.GetLength(NodeT);

		m_ConsTree.InsertNode(ParentNodeC, Label, Length);

#if DEBUG
		m_ConsTree.Validate();
#endif
		}
	else
		{
		uint LeftT = T.GetLeft(NodeT);
		uint RightT = T.GetRight(NodeT);
		double Length = T.GetLength(NodeT);

		string NewLabelC;
		Ps(NewLabelC, "n%u", m_NewNodeCount++);

		uint NewNodeC = m_ConsTree.InsertNode(ParentNodeC, NewLabelC, Length);

		InsertSubtreeNode(T, LeftT, NewNodeC);
		InsertSubtreeNode(T, RightT, NewNodeC);

#if DEBUG
		m_ConsTree.Validate();
#endif
		}
	}
