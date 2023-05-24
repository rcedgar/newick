#include "myutils.h"
#include "consmakerx.h"

static const double MIN_TP_FRACT = 0.5;

// Disjoint subtrees which cover the given labels
// and nothing else.
void TreeX::GetPureSubtrees(const set<uint> &LeafNodeSet,
  vector<uint> &SubtreeNodes) const
	{
#define TRACE	0
#if TRACE
	{
	Log("GetPureSubtrees: ");
	for (set<uint>::const_iterator p = LeafNodeSet.begin();
	  p != LeafNodeSet.end(); ++p)
		{
		uint Node = *p;
		asserta(IsLeaf(Node));
		const string &Label = GetLabel(Node);
		Log(" %s", Label.c_str());
		}
	Log("\n");
	}
#endif
	SubtreeNodes.resize(0);

	assert(IsRootedBinary());
	assert(IsNormalized());

	const uint Root = m_Origin;
	const uint NodeCount = m_AssignedNodeCount;
	vector<uint> NodeToInCount(NodeCount, 0);
	set<uint> CandidateInternalNodes;

// Increment in-group counts
	vector<uint> Path;
	for (set<uint>::const_iterator p = LeafNodeSet.begin();
	  p != LeafNodeSet.end(); ++p)
		{
		uint LeafNode = *p;
		GetPathToOrigin2(LeafNode, Path);
		const uint n = SIZE(Path);
		for (uint i = 0; i < n; ++i)
			{
			uint Node2 = Path[i];
			asserta(Node2 < NodeCount);
			++NodeToInCount[Node2];
			CandidateInternalNodes.insert(Node2);
			}
		}

#if TRACE
	Log("%u candidate nodes\n", SIZE(CandidateInternalNodes));
#endif
	set<uint> PureNodes;
	for (set<uint>::const_iterator p = CandidateInternalNodes.begin();
	  p != CandidateInternalNodes.end(); ++p)
		{
		uint InternalNode = *p;
		uint InCount = NodeToInCount[InternalNode];
		uint Size = GetSubtreeLeafCount_Rooted(InternalNode);
#if TRACE
		Log("  IN %u size %u InCount %u pure %c\n",
		  InternalNode, Size, InCount, yon(Size == InCount));
#endif
		if (Size == InCount)
			PureNodes.insert(InternalNode);
		}

	for (set<uint>::const_iterator p = PureNodes.begin();
	  p != PureNodes.end(); ++p)
		{
		uint PureNode = *p;
#if TRACE
		Log(" Pure node %u, root %c\n", PureNode, yon(PureNode == Root));
#endif
		if (PureNode == Root)
			{
			SubtreeNodes.push_back(Root);
			return;
			}

		uint Parent = GetParent(PureNode);
		asserta(Parent != UINT_MAX);

		bool ParentIsNotPure = (PureNodes.find(Parent) == PureNodes.end());
#if TRACE
		Log("  parent %u, pure %c\n", Parent, yon(!ParentIsNotPure));
#endif
		if (ParentIsNotPure)
			{
			asserta(Parent == m_Origin || NodeToInCount[Parent] > 0);
			SubtreeNodes.push_back(PureNode);
			}
		}
	asserta(!SubtreeNodes.empty());

#if DEBUG
/////////////////////////////////////////////////
// Validate result
	{
	const uint SubtreeCount = SIZE(SubtreeNodes);
	set<uint> FoundSet;
	for (uint i = 0; i < SubtreeCount; ++i)
		{
		uint SubtreeNode = SubtreeNodes[i];
		vector<uint> LeafNodes;

	// Subtree IS pure
		GetSubtreeLeafNodes_Rooted(SubtreeNode, LeafNodes);
		for (uint j = 0; j < SIZE(LeafNodes); ++j)
			{
			uint LeafNode = LeafNodes[j];
			asserta(LeafNodeSet.find(LeafNode) != LeafNodeSet.end());
			asserta(FoundSet.find(LeafNode) == FoundSet.end());
			FoundSet.insert(LeafNode);
			}

	// Subtree parent is NOT pure
		uint SubtreeNodeParent = GetParent(SubtreeNode);
		if (SubtreeNodeParent != UINT_MAX)
			{
			GetSubtreeLeafNodes_Rooted(SubtreeNodeParent, LeafNodes);
			bool OutFound = false;
			for (uint j = 0; j < SIZE(LeafNodes); ++j)
				{
				uint LeafNode = LeafNodes[j];
				if (LeafNodeSet.find(LeafNode) == LeafNodeSet.end())
					{
					OutFound = true;
					break;
					}
				}
			asserta(OutFound);
			}
		}

// Accounted for ALL labels
	asserta(SIZE(FoundSet) == SIZE(LeafNodeSet));
	}
// End validate result
/////////////////////////////////////////////////
#endif // DEBUG
#undef TRACE
	}

void ConsMakerX::IntersectLabels(const TreeX &Tree1, const TreeX &T2,
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
void ConsMakerX::MakeConsensus_Disjoint()
	{
	m_ConsTree.Init();

	m_ConsTree.InsertSubtreeBelow(m_ConsTree.m_Origin,
	  0, *m_Tree1, m_Tree1->m_Origin);

	m_ConsTree.InsertSubtreeBelow(m_ConsTree.m_Origin,
	  0, *m_Tree2, m_Tree2->m_Origin);

	m_ConsTree.m_Rooted = true;
	}

void ConsMakerX::MakeConsensus(const TreeX &T1, const TreeX &T2)
	{
	asserta(m_Tree1 == 0 && m_Tree2 == 0);

	assert(T1.IsRootedBinary());
	assert(T2.IsRootedBinary());

	m_Tree1 = &T1;
	m_Tree2 = &T2;

	IntersectLabels(*m_Tree1, *m_Tree2,
	  m_LabelsBoth, m_Labels1Only, m_Labels2Only, true, true);

	const uint N12 = SIZE(m_LabelsBoth);
	const uint N1 = SIZE(m_Labels1Only);
	const uint N2 = SIZE(m_Labels2Only);
	bool Big = (N12 > 1000 || N1 > 1000 || N2 > 1000);

	//ProgressLog("%u both, %u T1only, %u T2only\n", N12, N1, N2);

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
		m_Tree2->LabelSetToLeafNodeSet(m_Labels2Only, m_LeafNodes2Only);
		m_Tree2->GetPureSubtrees(m_LeafNodes2Only, m_Pures2);
		const uint SubtreeCount = SIZE(m_Pures2);

		for (uint i = 0; i < SubtreeCount; ++i)
			{
			if (Big)
				ProgressStep(i, SubtreeCount, "Subtrees");
			uint SubtreeNode2 = m_Pures2[i];
			AddSubtree(SubtreeNode2);
			}
		}
	ValidateConsensusLabels();
	assert(m_ConsTree.IsRootedBinary());
	}

void ConsMakerX::ValidateConsensusLabels()
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
void ConsMakerX::AddSubtree(uint SubtreeNode2)
	{
	asserta(!m_Tree2->IsRoot(SubtreeNode2));
	uint Sibling = m_Tree2->GetSibling(SubtreeNode2);

	vector<uint> SiblingNodes2;
	m_Tree2->GetSubtreeLeafNodes_Rooted(Sibling, SiblingNodes2);

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

	uint TPs = UINT_MAX;
	uint FPs = UINT_MAX;
	uint FNs = UINT_MAX;
	uint NodeC = m_ConsTree.GetBestFitSubtree(SiblingNodesC,
	  MIN_TP_FRACT, TPs, FPs, FNs);
	if (NodeC == UINT_MAX)
		NodeC = m_ConsTree.m_Origin;

	InsertSubtree(SubtreeNode2, NodeC);
	}

/***
Insert subtree from T2 above existing NodeC in ConsTree.

              ParentC
         --------|--------
						  | Length/2
                      NewNodeC
          (SubTreeNode2)--|-----
                 /|\            | Length/2
              Copy of         NodeC
            SubtreeNode2
           and its subtree
***/
void ConsMakerX::InsertSubtree(uint SubtreeNode2, uint NodeC)
	{
#if DEBUG
	m_ConsTree.Validate();
#endif
	++m_NewSubtreeCount;

	double Length = m_ConsTree.GetLength(NodeC);
	if (Length != DBL_MAX)
		Length /= 2;

// Don't do this, results in duplicate internal labels
// when trees are combined.
	//string NewLabel1;
	//Ps(NewLabel1, "Sxb%u", m_NewSubtreeCount);

	uint NewNodeC = 
	  m_ConsTree.InsertNodeBetweenNodeAndParent(NodeC, "");

	m_ConsTree.InsertSubtreeBelow(NewNodeC, Length, *m_Tree2, SubtreeNode2);

#if DEBUG
	m_ConsTree.Validate();
	assert(m_ConsTree.IsNormalized());
	assert(m_ConsTree.IsRootedBinary());
#endif
	}
