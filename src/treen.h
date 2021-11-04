#pragma once

#include <map>
#include <set>

class Tree2;
#include "newicktree.h"

/***
N-ary tree.
Always a root (or pseudo-root if representing unrooted tree).
Nodes are integers, may not be consecutive.
***/
class TreeN
	{
public:
// Primary data
	map<uint, uint> m_NodeToParent;
	map<uint, double> m_NodeToLength;
	map<uint, string> m_NodeToLabel;

// Derived data
	uint m_Root;
	map<uint, vector<uint> > m_NodeToChildren;
	map<string, uint> m_LabelToNode;

public:
	TreeN()
		{
		m_Root = UINT_MAX;
		}

	void Clear()
		{
		m_Root = UINT_MAX;
		m_NodeToParent.clear();
		m_NodeToLength.clear();
		m_NodeToLabel.clear();
		}

public:
	void FromData(const char *Data, uint Bytes);
	void FromTokens(const vector<string> &Tokens);
	void FromNewickFile(const string &FileName);
	void FromNewickTree(const NewickTree &NP);
	void FromTree2(const Tree2 &T2);
	void CopyNormalized(TreeN &T) const;

	void ToNewickStr(string &s, bool WithLineBreaks) const;
	void ToTree2(Tree2 &T2) const;
	void ToNewickFile(const string &FileName, bool WithLineBreaks = true) const;
	void ToNewickFile(FILE *f, bool WithLineBreaks = true) const;
	void ToTSV(FILE *f) const;

	uint GetLeafCount() const;
	void GetLeafLabels(vector<string> &Labels, bool ErrorIfEmpty) const;
	bool IsRoot(uint Node) const { return Node == m_Root; }
	uint GetRoot() const { return m_Root; }
	bool IsBinary(bool &Rooted) const;
	void LogMe() const;
	uint GetNodeCount() const { return SIZE(m_NodeToParent); }
	void GetNodes(set<uint> &Nodes) const;
	void GetNodes(vector<uint> &Nodes) const;
	void GetLeafNodes(vector<uint> &Nodes) const;
	void GetLeafNodes(set<uint> &Nodes) const;
	uint GetNodeByLabel(const string &Label, bool FailIfNotFound) const;
	uint GetNodeByAcc(const string &Label, bool FailIfNotFound) const;
	void Validate() const;
	void GetPathToRoot(uint Node, vector<uint> &Path) const;
	uint GetParent(uint Node) const;
	uint GetLeft(uint Node) const;
	uint GetRight(uint Node) const;
	double GetLength(uint Node) const;
	const string &GetLabel(uint Node) const;
	void GetNewickLabel(uint Node, string &Label) const;
	void GetLabel(uint Node, string &Label) const { Label = GetLabel(Node); }
	bool IsLeaf(uint Node) const;
	bool IsInternalNode(uint Node) const { return !IsLeaf(Node); }
	bool IsNonRootInternalNode(uint Node) const { return !IsLeaf(Node) && !IsRoot(Node); }
	const vector<uint> &GetChildren(uint Node) const;
	vector<uint> &GetChildren(uint Node);
	void DeleteLeaf(uint Node);
	void CollapseConfidenceUnary();
	void CollapseUnary();
	void CollapseNode(uint Node);
	uint GetChildCount(uint Node) const;
	uint GetChild(uint Node, uint ChildIndex) const;
	void UpdateParent(uint Node, uint Parent);
	void UpdateLength(uint Node, double Length);
	void UpdateLabel(uint Node, const string &Label);
	void AppendSubtreeLeafNodes(uint Node, vector<uint> &LeafNodes) const;
	void AppendSubtreeLeafNodes(uint Node, set<uint> &LeafNodes) const;
	void GetConcatLeafLabel(uint Node, string &ConcatLabel) const;
	uint GetSubtreeLeafCount(uint Node) const;
	void Reroot(uint Node);
	void InsertRootAbove(uint InsertNode, const string &OldRootLabel,
	  const string &NewRootLabel);
	uint IsNode(uint Node) const;
	uint GetNewNode() const;
	void GetLeafNodeSet(set<uint> &LeafNodeSet) const;
	uint GetBestFitSubtree1(const set<uint> &InGroupLeafNodes,
	  bool AllowInvert, uint &FPs, uint &FNs, bool &Invert) const;
	uint GetBestFitSubtree2(const set<uint> &AllGroupLeafNodes, 
	  const set<uint> &InGroupLeafNodes, bool AllowInvert,
	  uint &FPs, uint &FNs, bool &Invert) const;
	void GetGroupLeafNodes(const set<string> &GroupLabels,
	  set<uint> &GroupLeafNodes) const;
	void GetGroupLeafNodes(const string &GroupName,
	  set<uint> &GroupLeafNodes) const;
	void Subset(const set<uint> &SubsetNodes);
	void Ladderize(bool MoreRight);
	void SortNodesBySubtreeSize(vector<uint> &Nodes, bool Increasing) const;
	void Normalize();
	bool IsNormalized() const;
	void GetSortedLeafLabels(vector<string> &Labels) const;
	void GetSubtreeSortedLeafLabels(uint Node, vector<string> &Labels) const;
	void GetSortedLeafLabelToIndex(map<string, uint> &Map) const;
	uint GetPathToRootLength(uint Node) const;
	double GetRootDist(uint Node) const;
	void GetRootDists(vector<double> &Dists) const;
	void GetLeafRootDists(vector<double> &Dists) const;
	uint GetNodeCountToFurthestLeaf(uint Node) const;
	void SetDerived();
	void EraseNode(uint Node);

private:
	void AppendNodeToNewickStr(string &Str, uint Node, bool WithLineBreaks) const;

public:
	static double SumLengths(double Length1, double Length2);
	static bool IsValidNewickLabel(const string &Label);
	};

void TreesFromFile(const string &FileName, vector<TreeN *> &Trees);
void TreesToFile(const vector<TreeN *> &Trees, const string &FileName);
