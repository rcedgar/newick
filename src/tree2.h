#pragma once

#include <map>
#include "newicktree.h"

class Tree2;
class TreeN;
typedef void fn_OnNode(const Tree2 &T, uint Node);

class Tree2
	{
public:
	uint m_Root;	// UINT_MAX if not rooted
	vector<uint> m_Nbrs1;
	vector<uint> m_Nbrs2;
	vector<uint> m_Nbrs3;
	vector<string> m_Labels;
	map<pair<uint, uint>, double> m_EdgeToLength;

public:
	Tree2() { Clear(); }
	void Clear()
		{
		m_Root = UINT_MAX;
		m_Nbrs1.clear();
		m_Nbrs2.clear();
		m_Nbrs3.clear();
		m_Labels.clear();
		m_EdgeToLength.clear();
		}

public:
	void FromData(const char *Data, uint Bytes);
	void FromFile(const string &FileName);
	void FromNewickTree(const NewickTree &T);
	void FromNewickFile(const string &FileName);
	void FromCStr(const char *CStr);
	void FromStr(const string &Str) { FromCStr(Str.c_str()); }
	void FromVectors(const vector<string> &Labels, 
	  const vector<uint> &Parents, const vector<double> &Lengths);
	void FromTree(const Tree2 &T);
	void FromTreeN(const TreeN &T);

	void ToNewickFile(const string &FileName) const;
	void ToNewickFile(FILE *f) const;
	void ToNewickStr(string &Str, bool WithLineBreaks = false) const;

	void GetLabelToLeafNodeIndexMap(map<string, uint> &LabelToLeafNodeIndex) const;

	void LogMe(FILE *f = g_fLog) const;
	void ToTSV(FILE *f) const;
	void ToJust(FILE *f) const;
	void ToTSV(const string &FileName) const;
	void ToTSVStrings(vector<string> &Lines) const;
	void ToJustifiedStrings(vector<string> &Lines) const;
	void FromTSVStrings(const vector<string> &Lines);
	void FromTSVFile(const string &FileName);

	void Unroot();
	void SetRoot(uint Node1, uint Node2);
	void Rotate(uint Node);
	uint Ladderize(bool MoreRight);

	uint GetRoot() const { return m_Root; }
	bool IsRooted() const { return m_Root != UINT_MAX; }
	uint GetNodeCount() const { return SIZE(m_Nbrs1); }
	uint GetEdgeCount() const;
	uint GetLeafCount() const;
	void Validate() const;
	void ValidateEdge(uint Node, uint Edge) const;
	bool IsLeaf(uint Node) const;
	bool IsRoot(uint Node) const;
	uint GetNodeByLabel(const string &Label, bool ErrorIfNotFound) const;
	uint GetNodeByAcc(const string &Acc, bool ErrorIfNotFound) const;
	const string &GetLabel(uint Node) const;
	void GetLabel(uint Node, string &Label) const;
	void SetEdgeLength(uint Node, double Length);
	double GetEdgeLength(uint Node1, uint Node2, bool FailOnError = true) const;
	double GetEdgeLengthToParent(uint Node, bool FailOnError = true) const;
	double GetEdgeLengthToLeftChild(uint Node, bool FailOnError = true) const;
	double GetEdgeLengthToRightChild(uint Node, bool FailOnError = true) const;
	void AppendLeaves(uint Node, vector<uint> &Leaves) const;
	uint GetSubtreeLeafCount(uint Node) const;
	void GetSubtreeLeafLabels(uint Node, vector<string> &Labels) const;
	void GetSubtreeLeafNodes(uint Node, vector<uint> &LeafNodes) const;
	void GetSubtrees(vector<vector<uint> > &LeafNodesVec) const;
	void GetLeafLabels(vector<string> &Labels, bool ErrorIfEmpty) const;
	void GetPathToRoot(uint Node, vector<uint> &Path) const;
	double GetDistance(uint Node, uint AncNode) const;
	void FindNCA(const vector<uint> &Nodes, uint &Node1, uint &Node2) const;

	uint GetLeft(uint Node) const;
	uint GetRight(uint Node) const;
	uint GetParent(uint Node) const;

	uint GetEdge1(uint Node) const;
	uint GetEdge2(uint Node) const;
	uint GetEdge3(uint Node) const;
	uint GetEdge(uint Node, uint i) const;
	bool IsEdge(uint Node1, uint Ndoe2) const;

	double GetRootDist(uint Node) const;
	void GetRootDists(vector<double> &Dists) const;
	void GetLeafRootDists(vector<double> &Dists) const;

	double GetMaxLeafDist(uint Node) const;
	uint GetNodeCountToRoot(uint Node) const;
	uint GetMaxNodeCountToRoot() const;
	uint GetNodeCountToFurthestLeaf(uint Node) const;

	void Inorder(uint Node, fn_OnNode OnNode) const;
	void Preorder(uint Node, fn_OnNode OnNode) const;
	void Postorder(uint Node, fn_OnNode OnNode) const;

	void SetLength(uint Node1, uint Node2, double Length);
	bool IsEdgeInMap(uint Node1, uint Node2) const;

private:
	void SetEdge(uint FromNode, uint i, uint ToNode);
	void OrientNode(uint Node, uint Parent);
	void GetEdgePair(uint Node1, uint Node2,
	  pair<uint, uint> &EdgePair, bool FailOnError = true) const;

private:
	void AppendNodeToNewickStr(string &Str, uint Node, bool WithLineBreaks) const;
	};

void GenerateRandomTree(uint LeafCount, bool Rooted, 
  double MinLength, double MaxLength, Tree2 &T);

void MakeSubsetNodes(const Tree2 &InputTree,
  const vector<uint> &SubsetNodes, 
  const vector<string> &SubsetLabels,
  Tree2 &SubsetTree);
