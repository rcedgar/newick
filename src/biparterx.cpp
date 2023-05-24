#include "myutils.h"
#include "treex.h"
#include "biparterx.h"
#include "murmur.h"

/***
Enumerate bipartitions of rooted binary tree T.
Each edge E induces a bipartition (E1, E2).
For each edge E:
	The subtree under E is E1.
	E2 is (T - E1).

If the root is deleted, an identical set of bipartitions
is obtained. Thus, an unrooted tree can be handled by
placing an arbitrary root.

In a rooted tree, the two edges terminating on the root
node give identical bipartitions which should be counted
only once.

Root node induces bipartition (all, none) which cannot
be produced by cutting an edge.

Root->left and Root->right induce identical bipartitions.
Both are included in m_BipVec.
***/

void BiParterX::Init(const TreeX &InputTree)
	{
	asserta(InputTree.IsRootedBinary());

	m_Labels.clear();
	m_LabelToIndex.clear();
	m_NodeToLabelIndex.clear();
	m_LabelIndexToNode.clear();
	m_PartVec.clear();
	m_UniquePartNodes.clear();
	m_NodeToDupeNodes.clear();
	m_HashTable.clear();

	InputTree.CopyNormalized(m_T);
	uint NodeCount = m_T.GetNodeCount();
	uint LeafCount = m_T.GetLeafCount();

	m_T.GetLeafLabels(m_Labels, true);
	asserta(SIZE(m_Labels) == LeafCount);
	sort(m_Labels.begin(), m_Labels.end());
	m_LabelToIndex.clear();
	for (uint Index = 0; Index < LeafCount; ++Index)
		{
		const string &Label = m_Labels[Index];
		if (m_LabelToIndex.find(Label) != m_LabelToIndex.end())
			Die("BiParterX::Init(), dupe label >%s", Label.c_str());
		m_LabelToIndex[Label] = Index;
		}

	m_NodeToLabelIndex.clear();
	m_NodeToLabelIndex.resize(NodeCount, UINT_MAX);
	m_LabelIndexToNode.clear();
	m_LabelIndexToNode.resize(LeafCount, UINT_MAX);
	for (uint Node = 0; Node < NodeCount; ++Node)
		{
		if (m_T.IsLeaf(Node))
			{
			const string &Label = m_T.GetLabel(Node);
			map<string, uint>::const_iterator p = m_LabelToIndex.find(Label);
			asserta(p != m_LabelToIndex.end());
			uint Index = p->second;
			asserta(Index < LeafCount);
			m_NodeToLabelIndex[Node] = Index;
			asserta(m_LabelIndexToNode[Index] == UINT_MAX);
			m_LabelIndexToNode[Index] = Node;
			}
		}

	m_PartVec.resize(NodeCount);
	for (uint Node = 0; Node < NodeCount; ++Node)
		{
		if (m_T.IsRoot(Node) || m_T.IsLeaf(Node))
			continue;

		vector<uint> SubtreeLeafNodes;
		uint Parent = m_T.GetParent(Node);
		m_T.AppendLeafNodesInOrder(Parent, Node, SubtreeLeafNodes);
		vector<bool> &Row = m_PartVec[Node];
		Row.resize(LeafCount, false);
		const uint N = SIZE(SubtreeLeafNodes);
		for (uint i = 0; i < N; ++i)
			{
			uint SubtreeLeafNode = SubtreeLeafNodes[i];
			assert(m_T.IsLeaf(SubtreeLeafNode));
			uint Index = m_NodeToLabelIndex[SubtreeLeafNode];
			asserta(Index < NodeCount);
			Row[Index] = true;
			}
		if (Row[0])
			{
			for (uint i = 0; i < LeafCount; ++i)
				Row[i] = !Row[i];
			}
		}
	SetHashTable();
	}

void BiParterX::SetHashTable()
	{
	uint NodeCount = m_T.GetNodeCount();
	uint SlotCount = NodeCount*5 + 1;
	m_HashTable.clear();
	m_HashTable.resize(SlotCount);
	const uint Root = m_T.m_Origin;
	m_UniquePartNodes.clear();
	m_NodeToDupeNodes.clear();
	m_NodeToDupeNodes.resize(NodeCount);
	for (uint Node = 0; Node < NodeCount; ++Node)
		{
		if (Node == Root)
			continue;
		if (m_T.IsLeaf(Node))
			continue;
		const string &Label = m_T.GetLabel(Node);
		const vector<bool> &Part = m_PartVec[Node];
		uint h = GetHash(Part);
		assert(h < SlotCount);
		vector<uint> &Nodes = m_HashTable[h];
		bool Dupe = false;
		for (uint i = 0; i < SIZE(Nodes); ++i)
			{
			uint Node2 = Nodes[i];
			const vector<bool> &Part2 = m_PartVec[Node2];
			if (Part2 == Part)
				{
				asserta(!m_T.IsLeaf(Node) && !m_T.IsLeaf(Node2));
				Dupe = true;
				m_NodeToDupeNodes[Node2].push_back(Node);
				break;
				}
			}
		if (!Dupe)
			{
			m_UniquePartNodes.push_back(Node);
			Nodes.push_back(Node);
			}
		}
	}

uint BiParterX::GetHash(const vector<bool> &Part) const
	{
	const uint SlotCount = SIZE(m_HashTable);
	const uint N = SIZE(Part);
	asserta(N == SIZE(m_Labels));
	uint x = 0xcc9e2d51;
	for (uint i = 0; i < N; ++i)
		if (Part[i])
			x ^= murmur_32_scramble(i ^ 0x1b873593);
	uint h = x%SlotCount;
	return h;
	}

uint BiParterX::GetHash(uint Node) const
	{
	const uint SlotCount = SIZE(m_HashTable);
	asserta(Node < SIZE(m_PartVec));
	const vector<bool> &Part = m_PartVec[Node];
	uint h = GetHash(Part);
	return h;
	}

uint BiParterX::GetLabelIndex(const string &Label) const
	{
	if (Label.empty())
		return UINT_MAX;
	map<string, uint>::const_iterator p = m_LabelToIndex.find(Label);
	if (p == m_LabelToIndex.end())
		return UINT_MAX;
	uint Index = p->second;
	return Index;
	}

uint BiParterX::Search(const vector<bool> &Part) const
	{
	if (Part.empty())
		return UINT_MAX;
	uint h = GetHash(Part);
	asserta(h < SIZE(m_HashTable));
	const vector<uint> &Nodes = m_HashTable[h];
	const uint N = SIZE(Nodes);
	for (uint i = 0; i < N; ++i)
		{
		uint Node = Nodes[i];
		asserta(Node < SIZE(m_PartVec));
		const vector<bool> &Part2 = m_PartVec[Node];
		if (Part2 == Part)
			return Node;
		}
	return UINT_MAX;
	}

uint BiParterX::GetOtherCount(const vector<bool> &Part,
  const vector<bool> &Query) const
	{
	uint MatchCountA = 0;
	uint MatchCountB = 0;
	uint OtherCountA = 0;
	uint OtherCountB = 0;

	const uint N = SIZE(Part);
	asserta(SIZE(Query) == SIZE(Part));
	uint NQ = 0;
	for (uint i = 0; i < N; ++i)
		{
		bool A = Part[i];
		bool Q = Query[i];
		if (A && Q)
			++MatchCountA;
		if (!A && Q)
			++MatchCountB;
		if (A && !Q)
			++OtherCountA;
		if (!A && !Q)
			++OtherCountB;
		if (Q)
			++NQ;
		}

	if (MatchCountA == NQ)
		{
		asserta(MatchCountB == 0);
		return OtherCountA;
		}
	else if (MatchCountB == NQ)
		{
		asserta(MatchCountA == 0);
		return OtherCountB;
		}
	return UINT_MAX;
	}

void BiParterX::SearchBestMatch(const vector<string> &Labels,
  uint &BestNode, vector<string> &MissingLabels, uint &OtherCount) const
	{
	BestNode = UINT_MAX;
	MissingLabels.clear();
	OtherCount = UINT_MAX;
	const uint InputLabelCount = SIZE(Labels);
	if (InputLabelCount == 0)
		return;

	const uint TreeLabelCount = SIZE(m_LabelIndexToNode);

	vector<bool> Query(TreeLabelCount, false);
	for (uint i = 0; i < InputLabelCount; ++i)
		{
		const string &Label = Labels[i];
		uint Index = GetLabelIndex(Label);
		if (Index == UINT_MAX)
			MissingLabels.push_back(Label);
		else
			{
			asserta(Index < TreeLabelCount);
			Query[Index] = true;
			}
		}
	
	const uint NodeCount = m_T.GetNodeCount();
	for (uint Node = 0; Node < NodeCount; ++Node)
		{
		if (m_T.IsLeaf(Node) || m_T.IsRoot(Node))
			continue;
		const vector<bool> &Part = m_PartVec[Node];
		uint QOtherCount = GetOtherCount(Part, Query);
		if (QOtherCount < OtherCount)
			{
			OtherCount = QOtherCount;
			BestNode = Node;
			}
		}
	}

void BiParterX::ToTSV(const string &Name, FILE *f) const
	{
	if (f == 0)
		return;
	uint LabelCount = SIZE(m_Labels);
	const uint NodeCount = m_T.GetNodeCount();
	uint Root = m_T.m_Origin;
	for (uint Node = 0; Node < NodeCount; ++Node)
		{
		const vector<bool> &Part = m_PartVec[Node];
		if (Node == Root || m_T.IsLeaf(Node))
			{
			asserta(Part.empty());
			continue;
			}
		asserta(SIZE(Part) == LabelCount);
		vector<string> LabelsA;
		vector<string> LabelsB;
		for (uint i = 0; i < LabelCount; ++i)
			{
			if (Part[i])
				LabelsA.push_back(m_Labels[i]);
			else
				LabelsB.push_back(m_Labels[i]);
			}

		sort(LabelsA.begin(), LabelsA.end());
		sort(LabelsB.begin(), LabelsB.end());

		uint NA = SIZE(LabelsA);
		uint NB = SIZE(LabelsB);

		string Label;
		m_T.GetLabel(Node, Label);

		if (!Name.empty())
			fprintf(f, "%s	", Name.c_str());

		fprintf(f, "%u	A(%u)", Node, NA);
		fprintf(f, "	label=%s", Label.c_str());
		for (uint i = 0; i < NA; ++i)
			fprintf(f, "	%s", LabelsA[i].c_str());
		fprintf(f, "\n");

		if (!Name.empty())
			fprintf(f, "%s	", Name.c_str());
		fprintf(f, "%u	B(%u)", Node, NB);
		fprintf(f, "	label=%s", Label.c_str());
		for (uint i = 0; i < NB; ++i)
			fprintf(f, "	%s", LabelsB[i].c_str());
		fprintf(f, "\n");
		}
	}

void BiParterX::LogPatterns() const
	{
	Log("\n");
	const uint NodeCount = m_T.GetNodeCount();
	uint Root = m_T.m_Origin;
	vector<string> Labels;
	uint LabelCount = SIZE(m_Labels);
	uint L = 0;
	for (uint i = 0; i < LabelCount; ++i)
		{
		string Label = m_Labels[i];
		if (StartsWith(Label, "Leaf"))
			Label = Label.substr(4, string::npos);
		Labels.push_back(Label);
		uint n = SIZE(Label);
		L = max(n, L);
		}
	for (uint i = 0; i < L; ++i)
		{
		for (uint j = 0; j < LabelCount; ++j)
			{
			const string &Label = Labels[j];
			uint n = SIZE(Label);
			if (i >= L - n)
				Log("%c", Label[i-(L-n)]);
			else
				Log(" ");
			}
		Log("\n");
		}
	for (uint Node = 0; Node < NodeCount; ++Node)
		{
		const vector<bool> &Part = m_PartVec[Node];
		if (Node == Root || m_T.IsLeaf(Node))
			{
			asserta(Part.empty());
			continue;
			}
		asserta(SIZE(Part) == LabelCount);
		string s = "";
		for (uint i = 0; i < LabelCount; ++i)
			{
			if (Part[i])
				s += '|';
			else
				s += '_';
			}
		Log("%s", s.c_str());
		Log("  [%u", Node);
		const vector<uint> &DupeNodes = m_NodeToDupeNodes[Node];
		for (uint i = 0; i < SIZE(DupeNodes); ++i)
			Log("+%u", DupeNodes[i]);
		Log("]");
		Log("  {'%s'", m_T.GetLabel(Node));
		for (uint i = 0; i < SIZE(DupeNodes); ++i)
			Log("+'%s'", m_T.GetLabel(DupeNodes[i]));
		Log("}");
		Log("\n");
		}
	}

void BiParterX::LogMe() const
	{
	uint Root = m_T.m_Origin;
	uint RootLeft = m_T.GetLeft(Root);
	uint RootRight = m_T.GetRight(Root);

	const uint NodeCount = m_T.GetNodeCount();
	const uint LeafCount = m_T.GetLeafCount();
	asserta(SIZE(m_PartVec) == NodeCount);
	asserta(SIZE(m_Labels) == LeafCount);

	Log("\n");
	Log("%u nodes, %u leaves\n", NodeCount, LeafCount);
	for (uint i = 0; i < LeafCount; ++i)
		Log("Label[%5u]  %s\n", i, m_Labels[i].c_str());
	Log("\n");
	Log("Root %u, left %u, right %u\n", Root, RootLeft, RootRight);
	Log("\n");
	uint CollisionCount = 0;
	for (uint Node = 0; Node < NodeCount; ++Node)
		{
		if (Node == Root)
			continue;
		if (m_T.IsLeaf(Node))
			continue;
		const vector<bool> &Part = m_PartVec[Node];
		Log("Node %5u | ", Node);
		uint h = GetHash(Node);
		uint Size = SIZE(m_HashTable[h]);
		asserta(Size > 0);
		CollisionCount += (Size - 1);
		Log(" %08x (size=%u) |", h, Size);
		Log("   ");
		for (uint i = 0; i < SIZE(Part); ++i)
			if (Part[i])
				Log(" %s", m_Labels[i].c_str());
		Log("  |  ");
		for (uint i = 0; i < SIZE(Part); ++i)
			if (!Part[i])
				Log(" %s", m_Labels[i].c_str());
		if (Node == RootLeft)
			Log("   ...ROOTL");
		else if (Node == RootRight)
			Log("   ...ROOTR");
		Log("\n");
		}
	Log("%u collisions\n", CollisionCount);
	}

void BiParterX::GetPartInternalNodes(uint PartIndex, vector<uint> &Nodes) const
	{
	Nodes.clear();
	asserta(PartIndex < SIZE(m_PartVec));
	const vector<bool> &Part = m_PartVec[PartIndex];
	if (m_T.IsRoot(PartIndex) || m_T.IsLeaf(PartIndex))
		{
		asserta(Part.empty());
		return;
		}
	Nodes.push_back(PartIndex);
	asserta(PartIndex < SIZE(m_NodeToDupeNodes));
	const vector<uint> &DupeNodes = m_NodeToDupeNodes[PartIndex];
	for (uint i = 0; i < SIZE(DupeNodes); ++i)
		Nodes.push_back(DupeNodes[i]);
	}

void BiParterX::GetPartInternalNodeLabels(uint PartIndex, vector<string> &Labels) const
	{
	Labels.clear();
	if (m_T.IsRoot(PartIndex) || m_T.IsLeaf(PartIndex))
		return;
	const uint NodeCount = m_T.GetNodeCount();
	string Label;
	m_T.GetLabel(PartIndex, Label);
	Labels.push_back(Label);

	asserta(PartIndex < SIZE(m_NodeToDupeNodes));
	const vector<uint> &DupeNodes = m_NodeToDupeNodes[PartIndex];
	for (uint i = 0; i < SIZE(DupeNodes); ++i)
		{
		uint DupeNode = DupeNodes[i];
		asserta(!m_T.IsLeaf(DupeNode));
		m_T.GetLabel(DupeNode, Label);
		Labels.push_back(Label);
		}
	}
