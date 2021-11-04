#include "myutils.h"
#include "treen.h"
#include "tree2.h"
#include "newickparser2.h"
#include "sort.h"

void TreeN::LogMe() const
	{
	vector<uint> Nodes;
	GetNodes(Nodes);
	const uint NodeCount = GetNodeCount();
	asserta(SIZE(Nodes) == NodeCount);
	Log("TreeN %u nodes\n", NodeCount);
	for (uint k = 0; k < NodeCount; ++k)
		{
		uint Node = Nodes[k];
		uint Parent = GetParent(Node);
		const string &Label = GetLabel(Node);
		const vector<uint> &Children = GetChildren(Node);
		const uint ChildCount = SIZE(Children);
		double Length = GetLength(Node);

		Log("[%7u]", Node);
		if (Parent == UINT_MAX)
			Log("  %7.7s", "*");
		else
			Log("  %7u", Parent);
		if (Length == MISSING_LENGTH)
			Log("  %7.7s", ".");
		else
			Log("  %7.3g", Length);
		Log("  %s", Label.c_str());
		if (Node == m_Root)
			Log("  root");
		else if (ChildCount == 0)
			Log("  leaf");
		else
			Log("   int");
		if (ChildCount > 0)
			{
			Log("  %u<", ChildCount);
			for (uint i = 0; i < ChildCount; ++i)
				{
				if (i > 0)
					Log(", ");
				Log("%u", Children[i]);
				}
			Log(">");
			}
		Log("\n");
		}
	}

void TreeN::CopyNormalized(TreeN &T) const
	{
	T.m_NodeToParent = m_NodeToParent;
	T.m_NodeToLength = m_NodeToLength;
	T.m_NodeToLabel = m_NodeToLabel;
	T.SetDerived();
	T.Validate();
	T.Normalize();
	}

bool TreeN::IsNormalized() const
	{
	const uint NodeCount = GetNodeCount();
	for (uint i = 0; i < NodeCount; ++i)
		if (!IsNode(i))
			return false;
	return true;
	}

void TreeN::Normalize()
	{
	map<uint, uint> NewNodeToParent;
	map<uint, double> NewNodeToLength;
	map<uint, string> NewNodeToLabel;

	vector<uint> NewToOld;
	map<uint, uint> OldToNew;
	for (map<uint, uint>::const_iterator p = m_NodeToParent.begin();
	  p != m_NodeToParent.end(); ++p)
		{
		uint Node = p->first;
		uint NewNode = SIZE(NewToOld);
		NewToOld.push_back(Node);
		asserta(OldToNew.find(Node) == OldToNew.end());
		OldToNew[Node] = NewNode;
		}

	const uint NodeCount = SIZE(NewToOld);
	for (uint NewNode = 0; NewNode < NodeCount; ++NewNode)
		{
		uint OldNode = NewToOld[NewNode];
		uint OldParent = GetParent(OldNode);
		uint NewParent = UINT_MAX;
		if (OldParent != UINT_MAX)
			{
			asserta(OldToNew.find(OldParent) != OldToNew.end());
			NewParent = OldToNew[OldParent];
			}
		asserta(NewNodeToParent.find(NewNode) == NewNodeToParent.end());
		NewNodeToParent[NewNode] = NewParent;

		const string &Label = GetLabel(OldNode);
		double Length = GetLength(OldNode);

		asserta(NewNodeToLabel.find(NewNode) == NewNodeToLabel.end());
		asserta(NewNodeToLength.find(NewNode) == NewNodeToLength.end());

		NewNodeToLabel[NewNode] = Label;
		NewNodeToLength[NewNode] = Length;
		}

	m_NodeToParent = NewNodeToParent;
	m_NodeToLength = NewNodeToLength;
	m_NodeToLabel = NewNodeToLabel;
	SetDerived();
	Validate();
	asserta(IsNormalized());
	}

uint TreeN::GetPathToRootLength(uint Node) const
	{
	vector<uint> Path;
	GetPathToRoot(Node, Path);
	uint Length = SIZE(Path);
	return Length;
	}

void TreeN::GetPathToRoot(uint Node, vector<uint> &Path) const
	{
	Path.clear();
	const uint NodeCount = GetNodeCount();
	for (;;)
		{
		Path.push_back(Node);
		if (Node == m_Root)
			return;
		asserta(SIZE(Path) <= NodeCount);
		Node = GetParent(Node);
		}
	}

void TreeN::Validate() const
	{
	const uint NodeCount = GetNodeCount();
	asserta(SIZE(m_NodeToParent) == NodeCount);
	asserta(SIZE(m_NodeToLength) == NodeCount);
	asserta(SIZE(m_NodeToLabel) == NodeCount);

	set<uint> NodeSet;
	GetNodes(NodeSet);
	asserta(SIZE(NodeSet) == NodeCount);

	for (set<uint>::const_iterator p = NodeSet.begin();
	  p != NodeSet.end(); ++p)
		{
		uint Node = *p;

		asserta(m_NodeToLength.find(Node) != m_NodeToLength.end());
		asserta(m_NodeToLabel.find(Node) != m_NodeToLabel.end());
		map<uint, uint>::const_iterator q = m_NodeToParent.find(Node);
		asserta(q != m_NodeToParent.end());
		uint Parent = q->second;
		if (Parent == UINT_MAX)
			{
			asserta(Node == m_Root);
			continue;
			}
		asserta(NodeSet.find(Node) != NodeSet.end());
		asserta(NodeSet.find(Parent) != NodeSet.end());
		
		vector<uint> Path;
		GetPathToRoot(Node, Path);
		uint n = SIZE(Path);
		asserta(n > 1);
		asserta(Path[0] == Node);
		asserta(Path[1] == Parent);
		asserta(Path[n-1] == m_Root);
		}

	for (set<uint>::const_iterator p = NodeSet.begin();
	  p != NodeSet.end(); ++p)
		{
		uint Node = *p;
		if (Node == m_Root)
			continue;
		uint Parent = GetParent(Node);
		const vector<uint> &ParentChildren = GetChildren(Parent);
		const uint n = SIZE(ParentChildren);
		bool Found = false;
		for (uint i = 0; i < n; ++i)
			{
			if (ParentChildren[i] == Node)
				{
				Found = true;
				break;
				}
			}
		asserta(Found);
		}

	for (set<uint>::const_iterator p = NodeSet.begin();
	  p != NodeSet.end(); ++p)
		{
		uint Node = *p;
		const vector<uint> &Children = GetChildren(Node);
		const uint n = SIZE(Children);
		for (uint i = 0; i < n; ++i)
			{
			uint Child = Children[i];
			uint ChildParent = GetParent(Child);
			asserta(ChildParent == Node);
			}
		}

	asserta(SIZE(m_NodeToChildren) == NodeCount);
	for (map<uint, vector<uint> >::const_iterator p = m_NodeToChildren.begin();
	  p != m_NodeToChildren.end(); ++p)
		{
		uint Node = p->first;
		const vector<uint> &Children = p->second;
		const uint ChildCount = SIZE(Children);
		for (uint i = 0; i < ChildCount; ++i)
			{
			uint Child = Children[i];
			map<uint, uint>::const_iterator q = m_NodeToParent.find(Child);
			asserta(q != m_NodeToParent.end());
			uint ChildParent = q->second;
			asserta(ChildParent == Node);
			}
		}
	}

void TreeN::GetLeafNodes(set<uint> &Nodes) const
	{
	Nodes.clear();
	for (map<uint, uint>::const_iterator p = m_NodeToParent.begin();
	  p != m_NodeToParent.end(); ++p)
		{
		uint Node = p->first;
		if (IsLeaf(Node))
			Nodes.insert(Node);
		}
	}

void TreeN::GetLeafNodes(vector<uint> &Nodes) const
	{
	Nodes.clear();
	for (map<uint, uint>::const_iterator p = m_NodeToParent.begin();
	  p != m_NodeToParent.end(); ++p)
		{
		uint Node = p->first;
		if (IsLeaf(Node))
			Nodes.push_back(Node);
		}
	}

void TreeN::GetNodes(vector<uint> &Nodes) const
	{
	Nodes.clear();
	for (map<uint, uint>::const_iterator p = m_NodeToParent.begin();
	  p != m_NodeToParent.end(); ++p)
		{
		uint Node = p->first;
		Nodes.push_back(Node);
		}
	}

void TreeN::GetNodes(set<uint> &Nodes) const
	{
	Nodes.clear();
	for (map<uint, uint>::const_iterator p = m_NodeToParent.begin();
	  p != m_NodeToParent.end(); ++p)
		{
		uint Node = p->first;
		Nodes.insert(Node);
		}
	}

uint TreeN::GetLeft(uint Node) const
	{
	const vector<uint> &Children = GetChildren(Node);
	const uint ChildCount = SIZE(Children);
	if (ChildCount == 0)
		return UINT_MAX;
	asserta(ChildCount == 2);
	return Children[0];
	}

uint TreeN::GetRight(uint Node) const
	{
	const vector<uint> &Children = GetChildren(Node);
	const uint ChildCount = SIZE(Children);
	if (ChildCount == 0)
		return UINT_MAX;
	asserta(ChildCount == 2);
	return Children[1];
	}

uint TreeN::GetParent(uint Node) const
	{
	map<uint, uint>::const_iterator p = m_NodeToParent.find(Node);
	asserta(p != m_NodeToParent.end());
	return p->second;
	}

double TreeN::GetLength(uint Node) const
	{
	map<uint, double>::const_iterator p = m_NodeToLength.find(Node);
	asserta(p != m_NodeToLength.end());
	return p->second;
	}

bool TreeN::IsValidNewickLabel(const string &Label)
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

void TreeN::GetNewickLabel(uint Node, string &Label) const
	{
	Label = GetLabel(Node);
	bool Ok = IsValidNewickLabel(Label);
	if (!Ok)
		Label = "'" + Label + "'";
	}

const string &TreeN::GetLabel(uint Node) const
	{
	map<uint, string>::const_iterator p = m_NodeToLabel.find(Node);
	asserta(p != m_NodeToLabel.end());
	return p->second;
	}

void TreeN::FromNewickTree(const NewickTree &NP)
	{
	Clear();
	const uint NodeCount = NP.GetNodeCount();
	for (uint Node = 0; Node < NodeCount; ++Node)
		{
		uint Parent = NP.m_Parents[Node];
		double Length = NP.m_Lengths[Node];
		const string &Label = NP.m_Labels[Node];
		if (Parent == UINT_MAX)
			{
			asserta(m_Root == UINT_MAX);
			m_Root = Node;
			}

		m_NodeToParent[Node] = Parent;
		m_NodeToLength[Node] = Length;
		m_NodeToLabel[Node] = Label;
		}
	SetDerived();
	asserta(IsNormalized());
	}

void TreeN::FromTree2(const Tree2 &T)
	{
	asserta(T.IsRooted());
	m_Root = T.m_Root;
	const uint NodeCount = T.GetNodeCount();
	for (uint Node = 0; Node < NodeCount; ++Node)
		{
		uint Parent = T.GetParent(Node);
		double Length = T.GetEdgeLength(Node, Parent);
		const string &Label = T.GetLabel(Node);

		m_NodeToParent[Node] = Parent;
		m_NodeToLength[Node] = Length;
		m_NodeToLabel[Node] = Label;
		}
	SetDerived();
	}

uint TreeN::GetChildCount(uint Node) const
	{
	const vector<uint> &Children = GetChildren(Node);
	uint ChildCount = SIZE(Children);
	return ChildCount;
	}

bool TreeN::IsLeaf(uint Node) const
	{
	uint ChildCount = GetChildCount(Node);
	return ChildCount == 0;
	}

void TreeN::GetConcatLeafLabel(uint Node, string &ConcatLabel) const
	{
	ConcatLabel.clear();
	vector<string> LeafLabels;
	GetSubtreeSortedLeafLabels(Node, LeafLabels);
	for (uint i = 0; i < SIZE(LeafLabels); ++i)
		{
		const string &Label = LeafLabels[i];
		if (i > 0)
			ConcatLabel += "+";
		ConcatLabel += Label;
		}
	}

void TreeN::AppendSubtreeLeafNodes(uint Node, set<uint> &LeafNodes) const
	{
	if (IsLeaf(Node))
		{
		LeafNodes.insert(Node);
		return;
		}
	const vector<uint> &Children = GetChildren(Node);
	const uint ChildCount = SIZE(Children);
	asserta(ChildCount > 0);
	for (uint i = 0; i < ChildCount; ++i)
		{
		uint Child = Children[i];
		AppendSubtreeLeafNodes(Child, LeafNodes);
		}
	}

uint TreeN::GetSubtreeLeafCount(uint Node) const
	{
	vector<uint> LeafNodes;
	AppendSubtreeLeafNodes(Node, LeafNodes);
	uint n = SIZE(LeafNodes);
	return n;
	}

void TreeN::AppendSubtreeLeafNodes(uint Node, vector<uint> &LeafNodes) const
	{
	if (IsLeaf(Node))
		{
		LeafNodes.push_back(Node);
		return;
		}
	const vector<uint> &Children = GetChildren(Node);
	const uint ChildCount = SIZE(Children);
	asserta(ChildCount > 0);
	for (uint i = 0; i < ChildCount; ++i)
		{
		uint Child = Children[i];
		AppendSubtreeLeafNodes(Child, LeafNodes);
		}
	}

vector<uint> &TreeN::GetChildren(uint Node)
	{
	map<uint, vector<uint> >::iterator p = m_NodeToChildren.find(Node);
	asserta(p != m_NodeToChildren.end());
	return p->second;
	}

const vector<uint> &TreeN::GetChildren(uint Node) const
	{
	map<uint, vector<uint> >::const_iterator p = m_NodeToChildren.find(Node);
	asserta(p != m_NodeToChildren.end());
	return p->second;
	}

void TreeN::SetDerived()
	{
	m_NodeToChildren.clear();
	m_LabelToNode.clear();
	m_Root = UINT_MAX;

	vector<uint> Nodes;
	GetNodes(Nodes);
	const uint NodeCount = SIZE(Nodes);
	const vector<uint> Empty;
	for (uint k = 0; k < NodeCount; ++k)
		{
		uint Node = Nodes[k];
		m_NodeToChildren[Node] = Empty;
		}

	for (uint k = 0; k < NodeCount; ++k)
		{
		uint Node = Nodes[k];
		uint Parent = GetParent(Node);
		if (Parent == UINT_MAX)
			{
			asserta(m_Root == UINT_MAX);
			m_Root = Node;
			}
		else
			{
			map<uint, vector<uint> >::const_iterator p = m_NodeToChildren.find(Parent);
			asserta(p != m_NodeToChildren.end());
			m_NodeToChildren[Parent].push_back(Node);
			}
		}
	asserta(m_Root != UINT_MAX);

	for (uint k = 0; k < NodeCount; ++k)
		{
		uint Node = Nodes[k];
		const string &Label = GetLabel(Node);

	// Do not check for dupes, leaves get duplicated boostrap
	// labels when collapsing.
		if (!Label.empty() && IsLeaf(Node))
			m_LabelToNode[Label] = Node;
		}
	}

uint TreeN::GetChild(uint Node, uint ChildIndex) const
	{
	const vector<uint> &Children = GetChildren(Node);
	asserta(ChildIndex < SIZE(Children));
	uint Child = Children[ChildIndex];
	return Child;
	}

void TreeN::UpdateParent(uint Node, uint Parent)
	{
	map<uint, uint>::iterator p = m_NodeToParent.find(Node);
	asserta(p != m_NodeToParent.end());
	p->second = Parent;
	}

void TreeN::UpdateLength(uint Node, double Length)
	{
	map<uint, double>::iterator p = m_NodeToLength.find(Node);
	asserta(p != m_NodeToLength.end());
	p->second = Length;
	}

void TreeN::UpdateLabel(uint Node, const string &Label)
	{
	map<uint, string>::iterator p = m_NodeToLabel.find(Node);
	asserta(p != m_NodeToLabel.end());
	p->second = Label;
	}

double TreeN::SumLengths(double Length1, double Length2)
	{
	if (Length1 == MISSING_LENGTH && Length2 == MISSING_LENGTH)
		return MISSING_LENGTH;
	double Sum = 0;
	if (Length1 != MISSING_LENGTH)
		Sum += Length1;
	if (Length2 != MISSING_LENGTH)
		Sum += Length2;
	return Sum;
	}

// Merge Node with its parent.
// Node is deleted.
// Node's children become parent's children.
// Special case: if root, delete root, child becomes
// new root.
void TreeN::CollapseNode(uint Node)
	{
	if (Node == m_Root)
		{
		const vector<uint> &Children = GetChildren(Node);
		const uint ChildCount = SIZE(Children);
		asserta(ChildCount == 1);
		uint NewRoot = Children[0];
		EraseNode(m_Root);
		m_Root = NewRoot;
		return;
		}

	uint Parent = GetParent(Node);
	asserta(Parent != UINT_MAX);

	double NodeLength = GetLength(Node);

	const vector<uint> &Children = GetChildren(Node);
	const uint ChildCount = SIZE(Children);
	vector<uint> NewParentChildren;
	for (uint i = 0; i < ChildCount; ++i)
		{
		uint Child = Children[i];
		double ChildLength = GetLength(Child);
		double NewLength = SumLengths(ChildLength,  NodeLength);
		UpdateLength(Child, NewLength);
		UpdateParent(Child, Parent);
		NewParentChildren.push_back(Child);
		}

	const vector<uint> &ParentChildren = GetChildren(Parent);
	bool Found = false;
	for (uint i = 0; i < SIZE(ParentChildren); ++i)
		{
		uint Child = ParentChildren[i];
		if (Child == Node)
			Found = true;
		else
			NewParentChildren.push_back(Child);
		}
	asserta(Found);
	m_NodeToChildren[Parent] = NewParentChildren;

	EraseNode(Node);
//	SetDerived();
#if DEBUG
	Validate();
#endif
	}

void TreeN::GetLeafLabels(vector<string> &Labels, bool ErrorIfEmpty) const
	{
	Labels.clear();
	for (map<uint, vector<uint> >::const_iterator p = m_NodeToChildren.begin();
	  p != m_NodeToChildren.end(); ++p)
		{
		const vector<uint> &Children = p->second;
		if (SIZE(Children) == 0)
			{
			uint Node = p->first;
			map<uint, string>::const_iterator q = m_NodeToLabel.find(Node);
			asserta(q != m_NodeToLabel.end());
			const string &Label = q->second;
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

uint TreeN::GetLeafCount() const
	{
	uint LeafCount = 0;
	for (map<uint, vector<uint> >::const_iterator p = m_NodeToChildren.begin();
	  p != m_NodeToChildren.end(); ++p)
		{
		const vector<uint> &Children = p->second;
		if (SIZE(Children) == 0)
			++LeafCount;
		}
	return LeafCount;
	}

bool TreeN::IsBinary(bool &Rooted) const
	{
	vector<uint> Nodes;
	GetNodes(Nodes);
	const uint NodeCount = SIZE(Nodes);
	for (uint k = 0; k < NodeCount; ++k)
		{
		uint Node = Nodes[k];
		uint ChildCount = GetChildCount(Node);
		if (Node == m_Root)
			{
			if (ChildCount == 2)
				Rooted = true;
			else if (ChildCount == 3)
				Rooted = false;
			else
				return false;
			}
		else if (ChildCount != 0 && ChildCount != 2)
			return false;
		}
	return true;
	}

void TreeN::CollapseConfidenceUnary()
	{
	for (;;)
		{
		bool Any = false;
		vector<uint> Nodes;
		GetNodes(Nodes);
		const uint NodeCount = SIZE(Nodes);
		for (uint k = 0; k < NodeCount; ++k)
			{
			uint Node = Nodes[k];
			if (Node == m_Root)
				continue;
			uint ChildCount = GetChildCount(Node);
			if (ChildCount == 1)
				{
				uint Child = GetChild(Node, 0);
				if (IsNonRootInternalNode(Node) && IsNonRootInternalNode(Child))
					{
					const string &Label = GetLabel(Node).c_str();
					const string &ChildLabel = GetLabel(Child).c_str();
					if (IsValidFloatStr(Label) && IsValidFloatStr(ChildLabel))
						{
						double Conf = StrToFloat(Label);
						double ChildConf = StrToFloat(ChildLabel);
						if (Conf > ChildConf)
							{
							Any = true;
							UpdateLabel(Child, Label);
							}
						else if (ChildConf > Conf)
							{
							Any = true;
							UpdateLabel(Node, ChildLabel);
							}
						}
					}
				}
			}
		if (!Any)
			return;
		}
	}

void TreeN::CollapseUnary()
	{
	for (;;)
		{
		bool AnyUnary = false;
		vector<uint> Nodes;
		GetNodes(Nodes);
		const uint NodeCount = SIZE(Nodes);
		for (uint k = 0; k < NodeCount; ++k)
			{
			uint Node = Nodes[k];
			//if (Node == m_Root)
			//	continue;
			uint ChildCount = GetChildCount(Node);
			if (ChildCount == 1)
				{
				AnyUnary = true;
				CollapseNode(Node);
				}
			}
		if (!AnyUnary)
			return;
		}
	}

void TreeN::EraseNode(uint Node)
	{
	asserta(Node != m_Root);

	const string &Label = GetLabel(Node);
	if (Label != "")
		m_LabelToNode.erase(Label);

	size_t n = m_NodeToParent.erase(Node);
	assert(n == 1);

	n = m_NodeToLabel.erase(Node);
	assert(n == 1);

	n = m_NodeToLength.erase(Node);
	assert(n == 1);

	n = m_NodeToChildren.erase(Node);
	assert(n == 1);
	}

void TreeN::DeleteLeaf(uint Node)
	{
	asserta(IsLeaf(Node));
	EraseNode(Node);
	SetDerived();
	}

uint TreeN::GetNodeByAcc(const string &Acc, bool ErrorIfNotFound) const
	{
	vector<uint> Nodes;
	GetNodes(Nodes);
	const uint N = SIZE(Nodes);
	for (uint k = 0; k < N; ++k)
		{
		uint Node = Nodes[k];
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

uint TreeN::GetNodeByLabel(const string &Label, bool FailIfNotFound) const
	{
	map<string, uint>::const_iterator p = m_LabelToNode.find(Label);
	if (p == m_LabelToNode.end())
		{
		if (FailIfNotFound)
			Die("Label not found >%s", Label.c_str());
		return UINT_MAX;
		}
	return p->second;
	}

void TreeN::AppendNodeToNewickStr(string &Str, uint Node, bool WithLineBreaks) const
	{
	if (!IsLeaf(Node))
		{
		const vector<uint> &Children = GetChildren(Node);
		const uint ChildCount = SIZE(Children);
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
	if (Length != MISSING_LENGTH)
		Psa(Str, ":%.4g", Length);
	if (WithLineBreaks)
		Str += "\n";
	}

void TreeN::ToNewickStr(string &Str, bool WithLineBreaks) const
	{
	const uint RootChildCount = GetChildCount(m_Root);
	Str = "(";
	for (uint i = 0; i < RootChildCount; ++i)
		{
		if (i > 0)
			{
			Str += ",";
			if (WithLineBreaks)
				Str += "\n";
			}
		uint Child = GetChild(m_Root, i);
		AppendNodeToNewickStr(Str, Child, WithLineBreaks);
		}
	Str += ")";
	string RootLabel;
	GetNewickLabel(m_Root, RootLabel);
	if (RootLabel != "")
		Str += RootLabel;
	double RootLength = GetLength(m_Root);
	if (RootLength != MISSING_LENGTH)
		Psa(Str, ":%.4g", RootLength);
	Str += ";\n";
	}

void TreeN::ToNewickFile(const string &FileName, bool WithLineBreaks) const
	{
	FILE *f = CreateStdioFile(FileName);
	ToNewickFile(f, WithLineBreaks);
	CloseStdioFile(f);
	}

void TreeN::ToNewickFile(FILE *f, bool WithLineBreaks) const
	{
	if (f == 0)
		return;
	string Str;
	ToNewickStr(Str, WithLineBreaks);
	fputs(Str.c_str(), f);
	}

void TreeN::FromNewickFile(const string &FileName)
	{
	NewickParser2 NT;
	NT.FromFile(FileName);
	FromNewickTree(NT);
	}

void TreeN::FromData(const char *Data, uint Bytes)
	{
	NewickParser2 NP;
	NP.FromData(Data, Bytes);
	FromNewickTree(NP);
	}

void TreeN::FromTokens(const vector<string> &Tokens)
	{
	const uint N = SIZE(Tokens);
	if (N == 0)
		Die("TreeN::FromTokens(), no data");
	if (Tokens[N-1] != ";")
		Die("TreeN::FromTokens(), missing ';'");

	NewickParser2 NP;
	NP.FromTokens(Tokens);
	FromNewickTree(NP);
	}

void TreeN::ToTree2(Tree2 &T2) const
	{
	string NewickStr;
	ToNewickStr(NewickStr, false);
	T2.FromStr(NewickStr);
	}

void TreeN::ToTSV(FILE *f) const
	{
	if (f == 0)
		return;

	const uint NodeCount = GetNodeCount();
	fprintf(f, "nodes	%u\n", NodeCount);
	for (map<uint, uint>::const_iterator p = m_NodeToParent.begin();
	  p != m_NodeToParent.end(); ++p)
		{
		uint Node = p->first;
		uint Parent = p->second;
		const string &Label = GetLabel(Node);
		double Length = GetLength(Node);
		fprintf(f, "%u", Node);
		if (Parent == UINT_MAX)
			fprintf(f, "\t*");
		else
			fprintf(f, "\t%u", Parent);
		if (Length == MISSING_LENGTH)
			fprintf(f, "\t*");
		else
			fprintf(f, "\t%.4g", Length);
		fprintf(f, "\t%s", Label.c_str());
		fprintf(f, "\n");
		}
	}

// Re-root on existing node.
// Do not insert new node for new root, 
// or fix up old root.
void TreeN::Reroot(uint NewRootNode)
	{
	if (NewRootNode == m_Root)
		return;

	vector<uint> Path;
	GetPathToRoot(NewRootNode, Path);
	const uint n = SIZE(Path);

	vector<double> Lengths;
// Path excluding root
	for (uint i = 0; i + 1 < n; ++i)
		{
		uint Node = Path[i];
		double Length = GetLength(Node);
		Lengths.push_back(Length);
		}

// Path excluding root
	for (uint i = 0; i + 1 < n; ++i)
		{
		uint Node = Path[i];
		uint Parent = Path[i+1];
		double Length = Lengths[i];
		m_NodeToParent[Parent] = Node;
		m_NodeToLength[Parent] = Length;
		}
	m_NodeToParent[NewRootNode] = UINT_MAX;
	m_NodeToLength[NewRootNode] = MISSING_LENGTH;

	SetDerived();
	Validate();
	}

void TreeN::SortNodesBySubtreeSize(vector<uint> &Nodes, bool Increasing) const
	{
	const uint n = SIZE(Nodes);
	if (n == 0)
		return;
	vector<uint> Sizes;
	for (uint i = 0; i < n; ++i)
		{
		uint Node = Nodes[i];
		uint Size = GetSubtreeLeafCount(Node);
		Sizes.push_back(Size);
		}


	vector<uint> Order(n);
	if (Increasing)
		QuickSortOrder<uint>(Sizes.data(), n, Order.data());
	else
		QuickSortOrderDesc<uint>(Sizes.data(), n, Order.data());

	for (uint k = 1; k < n; ++k)
		{
		uint PrevIndex = Order[k-1];
		uint Index = Order[k];
		if (Sizes[PrevIndex] == Sizes[Index])
			{
			uint PrevNode = Nodes[PrevIndex];
			uint Node = Nodes[Index];

			string PrevConcatLabel;
			string ConcatLabel;
			GetConcatLeafLabel(PrevNode, PrevConcatLabel);
			GetConcatLeafLabel(Node, ConcatLabel);
			if (PrevConcatLabel > ConcatLabel)
				{
				Order[k-1] = Index;
				Order[k] = PrevIndex;
				}
			}
		}

	vector<uint> NewNodes;
	for (uint i = 0; i < n; ++i)
		{
		uint k = Order[i];
		uint Node = Nodes[k];
		NewNodes.push_back(Node);
		}
	Nodes = NewNodes;
	}

void TreeN::Ladderize(bool MoreRight)
	{
	vector<uint> Nodes;
	GetNodes(Nodes);
	const uint NodeCount = SIZE(Nodes);
	for (uint k = 0; k < NodeCount; ++k)
		{
		uint Node = Nodes[k];
		if (IsLeaf(Node))
			continue;

		vector<uint> &Children = GetChildren(Node);
		const uint ChildCount = SIZE(Children);
		asserta(ChildCount > 0);
		SortNodesBySubtreeSize(Children, MoreRight);
		m_NodeToChildren[Node] = Children;
		}
	}

void TreeN::Subset(const set<uint> &SubsetNodes)
	{
	map<uint, uint> NodeToCount;

	for (set<uint>::const_iterator p = SubsetNodes.begin();
	  p != SubsetNodes.end(); ++p)
		{
		uint Node = *p;
		asserta(IsNode(Node));

		vector<uint> Path;
		GetPathToRoot(Node, Path);

		const uint n = SIZE(Path);
		for (uint i = 0; i < n; ++i)
			{
			uint PathNode = Path[i];
			if (NodeToCount.find(PathNode) == NodeToCount.end())
				NodeToCount[PathNode] = 1;
			else
				NodeToCount[PathNode] += 1;
			}
		}

	set<uint> KeepNodes = SubsetNodes;
	for (map<uint, uint>::const_iterator p = NodeToCount.begin();
	  p != NodeToCount.end(); ++p)
		{
		uint Count = p->second;
		if (Count > 0)
			KeepNodes.insert(p->first);
		}

	set<uint> Nodes;
	GetNodes(Nodes);
	const uint Count = SIZE(Nodes);
	uint Counter = 0;
	for (set<uint>::const_iterator p = Nodes.begin();
	  p != Nodes.end(); ++p)
		{
		ProgressStep(Counter++, Count, "Collapsing");
		uint Node = *p;
		if (IsRoot(Node))
			continue;
		set<uint>::const_iterator q = KeepNodes.find(Node);
		if (q == KeepNodes.end())
			CollapseNode(Node);
		}
	SetDerived();
	Validate();
	}

uint TreeN::IsNode(uint Node) const
	{
	map<uint, uint>::const_iterator p = m_NodeToParent.find(Node);
	bool Found = (p != m_NodeToParent.end());
	return Found;
	}

void TreeN::GetGroupLeafNodes(const set<string> &GroupLabels,
  set<uint> &GroupLeafNodes) const
	{
	GroupLeafNodes.clear();

	vector<uint> LeafNodes;
	GetLeafNodes(LeafNodes);
	const uint LeafCount = SIZE(LeafNodes);

	for (uint k = 0; k < LeafCount; ++k)
		{
		uint Node = LeafNodes[k];
		const string &Label = GetLabel(Node);
		if (GroupLabels.find(Label) != GroupLabels.end())
			GroupLeafNodes.insert(Node);
		}
	}

void TreeN::GetGroupLeafNodes(const string &GroupName,
  set<uint> &GroupLeafNodes) const
	{
	GroupLeafNodes.clear();

	vector<uint> LeafNodes;
	GetLeafNodes(LeafNodes);
	const uint LeafCount = SIZE(LeafNodes);

	for (uint k = 0; k < LeafCount; ++k)
		{
		uint Node = LeafNodes[k];
		const string &Label = GetLabel(Node);
		if (Label.find(GroupName) != string::npos)
			GroupLeafNodes.insert(Node);
		}
	}

uint TreeN::GetNewNode() const
	{
	uint NodeCount = GetNodeCount();
	for (uint Node = 0; Node <= NodeCount; ++Node)
		{
		if (!IsNode(Node))
			return Node;
		}
	asserta(false);
	return UINT_MAX;
	}

// Insert new node for root between AboveNode and its parent.
// If tree was binary, old root becomes unary node
void TreeN::InsertRootAbove(uint InsertNode, const string &OldRootLabel,
  const string &NewRootLabel)
	{
	uint OldRootNode = GetRoot();
	if (OldRootLabel != "-")
		UpdateLabel(OldRootNode, OldRootLabel);

	uint Parent = GetParent(InsertNode);
	uint NewRootNode = GetNewNode();

	double Length = GetLength(InsertNode);
	double Length2 = (Length == MISSING_LENGTH ? MISSING_LENGTH : Length/2);

	UpdateParent(InsertNode, NewRootNode);
	UpdateLength(InsertNode, Length2);

	m_NodeToParent[NewRootNode] = Parent;
	m_NodeToLabel[NewRootNode] = NewRootLabel;
	m_NodeToLength[NewRootNode] = Length2;

	SetDerived();
	Validate();
	Reroot(NewRootNode);
	uint n = GetChildCount(OldRootNode);
	if (n == 1)
		CollapseNode(OldRootNode);
	}

void TreeN::GetSubtreeSortedLeafLabels(uint Node, vector<string> &Labels) const
	{
	vector<uint> LeafNodes;
	AppendSubtreeLeafNodes(Node, LeafNodes);

	for (uint i = 0; i < SIZE(LeafNodes); ++i)
		{
		uint LeafNode = LeafNodes[i];
		const string &Label = GetLabel(LeafNode);
		if (!Label.empty())
			Labels.push_back(Label);
		}
	sort(Labels.begin(), Labels.end());
	}

void TreeN::GetSortedLeafLabels(vector<string> &Labels) const
	{
	Labels.clear();
	for (map<uint, string>::const_iterator p = m_NodeToLabel.begin();
	  p != m_NodeToLabel.end(); ++p)
		{
		uint Node = p->first;
		if (!IsLeaf(Node))
			continue;
		const string &Label = p->second;
		if (!Label.empty())
			Labels.push_back(Label);
		}
	sort(Labels.begin(), Labels.end());
	}

void TreeN::GetSortedLeafLabelToIndex(map<string, uint> &Map) const
	{
	Map.clear();
	vector<string> Labels;
	GetSortedLeafLabels(Labels);
	const uint N = SIZE(Labels);
	for (uint i = 0; i < N; ++i)
		{
		const string &Label = Labels[i];
		asserta(Map.find(Label) == Map.end());
		Map[Label] = i;
		}
	}

void TreeN::GetRootDists(vector<double> &Heights) const
	{
	Heights.clear();
	const uint NodeCount = GetNodeCount();
	for (uint Node = 0; Node < NodeCount; ++Node)
		{
		double Height = GetRootDist(Node);
		Heights.push_back(Height);
		}
	}

void TreeN::GetLeafRootDists(vector<double> &Heights) const
	{
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

double TreeN::GetRootDist(uint Node) const
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
		double Length = GetLength(Node);
		if (Length != MISSING_LENGTH)
			Dist += Length;
		}
	return Dist;
	}

static void Test1(NewickParser2 &NP, TreeN &TN, const string &NewickStr)
	{
	ProgressLog("\n");
	ProgressLog("_______________________________________________________\n");
	ProgressLog("  %s\n", NewickStr.c_str());

	NP.FromStr(NewickStr);
	TN.FromNewickTree(NP);
	TN.LogMe();
	TN.Validate();
	ProgressLog("  validate ok.\n");

	string s;
	TN.ToNewickStr(s, false);
	Log("      InStr = %s\n", NewickStr.c_str());
	Log("ToNewickStr = %s\n", s.c_str());

	uint NodeA = TN.GetNodeByLabel("A", true);
	TN.DeleteLeaf(NodeA);
	TN.Validate();
	TN.LogMe();

	ProgressLog("  validate after delete ok.\n");
	}

void cmd_test()
	{
	opt(test);
	NewickParser2 NP;
	TreeN TN;

	Test1(NP, TN, "(A:0.1,B:0.2,C:0.3);");
	Test1(NP, TN, "(A,B,(C,D));");
	Test1(NP, TN, "(A,B,C);");
	Test1(NP, TN, "(A,(B,C));");
	Test1(NP, TN, "(A,B,(C,D));");
	Test1(NP, TN, "(A,B,(C,D)E)F;");
	Test1(NP, TN, "(A:0.1,B:0.2,(C:0.3,D:0.4):0.5);");
	Test1(NP, TN, "(A:0.1,B:0.2,(C:0.3,D:0.4)E:0.5)F;");
	//Test1(NP, TN, "(,,(,));");
	//Test1(NP, TN, "(:0.1,:0.2,(:0.3,:0.4):0.5);");
	//Test1(NP, TN, "(:0.1,:0.2,(:0.3,:0.4):0.5):0.0;");
	}

void cmd_newick2tsv()
	{
	vector<TreeN *> Trees;
	TreesFromFile(opt(newick2tsv), Trees);

	asserta(optset_output);
	FILE *f = CreateStdioFile(opt(output));
	const uint TreeCount = SIZE(Trees);
	fprintf(f, "trees	%u\n", TreeCount);
	for (uint TreeIndex = 0; TreeIndex < TreeCount; ++TreeIndex)
		{
		const TreeN &T = *Trees[TreeIndex];
		T.ToTSV(f);
		}
	CloseStdioFile(f);
	}
