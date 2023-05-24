#pragma once

#include "gobuff.h"
#include "newickparser2.h"
#include <set>
#include <list>
#include <unordered_map>

#define GOBUFF_SIZE 1024

//typedef bool (*ptrOnEdgeFn)(uint Node1, uint Node2, void *UserData);
typedef bool (*ptrOnNodeFn)(uint PrevNode, uint Node, void *UserData);

class TreeX
	{
public:
	uint m_Origin = UINT_MAX;
	bool m_Rooted = false;

	uint m_AssignedNodeCount = 0;
	uint m_Size = 0;
	vector<string> m_Labels;

// m_NodeToChildCount[Node] is UINT_MAX if node deleted
	double *m_NodeToLength = 0;
	uint *m_NodeToChildSize = 0;
	uint *m_NodeToChildCount = 0;
	uint **m_NodeToChildren = 0;
	uint *m_NodeToParent = 0;
	unordered_map<string, uint> m_LabelToNode;

// For GetLCAEdge() callback
	const set<uint> *m_LeafNodeSet = 0;
	set<uint> m_FoundLeafNodeSet;
	set<uint> m_OtherLeafNodeSet;

// For breadth-first traversal
	set<uint> m_VisitedNodes;
	list<uint> m_PendingNodes;
	uint *m_NodeToPrev = 0;

// For OnNode_InsertRootAbove
	TreeX *m_TX = 0;
	uint m_NodeBelowRoot = UINT_MAX;
	uint m_NodeAboveRoot = UINT_MAX;

// For DrawText
	mutable vector<string> m_DrawLines;
	mutable vector<uint> m_DrawXs;
	mutable vector<uint> m_DrawYs;
	mutable vector<uint> m_DrawSumYs;
	mutable vector<uint> m_DrawNs;
	mutable uint m_DrawMaxX = 0;
	mutable uint m_DrawMaxY = 0;

public:
// Not node count, some indexes are not nodes
	uint GetNodeIndexCount() const { return m_AssignedNodeCount; }
	bool IsRootedBinary() const;
	bool IsBinarySubtree(uint Node) const;
	bool IsNormalized() const;
	void Normalize();
	void CopyNormalized(TreeX &T) const;
	uint GetNodeCount() const;
	uint GetLeafCount() const;
	bool IsRoot(uint Node) const;
	void Copy(TreeX &T) const;
	void LogMe() const;
	void Init();
	void Ladderize();
	void LadderizeNode(uint Node);
	uint GetNodeByLabel(const string &Label,
	  bool FailIfNotFound = true) const;
	uint InsertOrphanNode(double Length, const string &Label);
	void Alloc(uint NodeCount);
	void FromNewickFile(const string &FileName);
	void FromNewickParser(const NewickParser2 &NP);
	void FromNewickStr(const string &Str);
	void ToNewickFile(const string &FileName, bool WithLineBreaks = true) const;
	void ToNewickFile(FILE *f, bool WithLineBreaks = true) const;
	void ToNewickStr(string &Str, bool WithLineBreaks = true) const;
	uint InsertNode(uint Parent, double Length, const string &Label);
	void DeleteSubtree(uint Node, const string &NewLabel,
	  bool DeleteSelf);
	void DeleteLeaf(uint Node);
	void InsertRootAbove(uint Node);
	void SetRootByGroupLabels(const vector<string> &Labels);
	bool IsNode(uint Node) const;
	void Unroot();
	void AssertNode(uint Node) const { asserta(IsNode(Node)); }
	void DeleteChildFromBuffer(uint Parent, uint Child);
	void ReplaceChildInBuffer(uint Parent, uint OldChild, uint NewChild);
	void SetChildCount(uint Parent, uint ChildCount);
	void AddChildToBuffer(uint Parent, uint Child);
	void ReplaceChildInBuffer(uint Parent,
	  uint OldChild, int NewChild);
	bool IsLeaf(uint Node) const;
	uint GetChildCount(uint Node) const;
	uint GetChild(uint Node, uint ChildIndex) const;
	uint GetLeft(uint Node) const;
	uint GetRight(uint Node) const;
	double GetLength(uint Node) const;
	void EraseNode(uint Node);
	void Validate() const;
	void ValidateNode(uint Node) const;
	uint GetChildIndex(uint Parent, uint Child) const;
	uint GetParent(uint Node) const;
	const uint *GetChildren(uint Node) const;
	uint *GetChildren(uint Node);
	const string &GetLabel(uint Node) const;
	void GetLabel(uint Node, string &Label) const;
	uint GetBestFitSubtree(const set<uint> &Nodes,
	  double MinTPFract, uint &TP, uint &FP, uint &FN);
	void GetPathToOrigin(uint Node, vector<uint> &Path) const;
	void GetPathToOrigin2(uint Node, vector<uint> &Path) const;
	uint GetEdgeCountToOrigin(uint Node) const;
	void Traverse_BreadthFirst(uint StartNode, void *UserData, ptrOnNodeFn OnNode);
	void Traverse_Inorder_Binary(uint Node, void *UserData, ptrOnNodeFn OnNode);
	void Traverse_Preorder(uint PrevNode, uint Node, void *UserData, ptrOnNodeFn OnNode);
	void Traverse_Postorder(uint PrevNode, uint Node, void *UserData, ptrOnNodeFn OnNode);
	void GetNeighbors(uint Node, vector<uint> &Neighbors) const;
	bool IsEdge(uint Node1, uint Node2) const;
	void GetLCAEdge(const set<uint> &LeafNodes, uint &FromNode, uint &ToNode);
	uint GetSubtreeLeafCount_Rooted(uint ToNode) const;
	uint GetSubtreeLeafCount(uint FromNode, uint ToNode) const;
	void LabelSetToLeafNodeSet(const set<string> &LabelSet,
	  set<uint> &LeafNodeSet) const;
	uint GetSibling(uint Node, bool FailIfNone = true) const;
	void GetSubtreeLeafNodes_Rooted(uint Node, vector<uint> &LeafNodes) const;
	void GetSubtreeLeafNodeSet_Rooted(uint Node, set<uint> &LeafNodes) const;
	void AppendSubtreeLeafNodes_Rooted(uint Node, vector<uint> &LeafNodes) const;
	void AppendSubtreeLeafLabels_Rooted(uint Node, vector<string> &Labels) const;
	void GetSubtreeLeafLabels_Rooted(uint Node, vector<string> &Labels) const;
	void GetSubtreeLeafNodes(uint FromNode, uint ToNode, vector<uint> &LeafNodes) const;
	void GetSubtreeLeafNodes(uint FromNode, uint ToNode, set<uint> &LeafNodes) const;
	void AppendNodeToNewickStr(string &Str, uint Node, bool WithLineBreaks) const;
	void GetNewickLabel(uint Node, string &Label) const;
	void GetLeafNodesInOrder(vector<uint> &LeafNodes) const;
	void AppendLeafNodesInOrder(uint FromNode, uint ToNode, vector<uint> &LeafNodes) const;
	void AppendLeafNodesInOrder(uint FromNode, uint ToNode, set<uint> &LeafNodes) const;
	void GetLeafLabels(vector<string> &Labels, bool ErrorIfEmpty = true) const;
	void GetLeafLabelSet(set<string> &LabelSet,
	  bool ErrorIfEmpty = true, bool ErrorIfDupe = true) const;
	void LogBreadthFirstState(const string &Msg = "") const;
	void SetParent(uint Node, uint Parent);
	void SetLength(uint Node, double Length);
	void UpdateLabel(uint Node, const string &NewLabel);
	void DrawText(FILE *f = g_fLog, bool WithLengths = true) const;
	void DrawPt(uint x, uint y, char c) const;
	void DrawStr(uint x, uint y, const string &s) const;
	double GetSumLengths() const;
	void GetPureSubtrees(const set<uint> &LeafNodeSet,
	  vector<uint> &SubtreeNodes) const;
	uint InsertNodeBetweenNodeAndParent(uint Node, const string &Label);
	void InsertSubtreeBelow(uint ThisNode, double Length,
	  const TreeX &T, uint NodeT);
	double GetNodePairDist_Rooted(uint Node1, uint Node2) const;
	uint GetLCANode_Rooted(uint Node1, uint Node2) const;
	void GetSubtreeLeafDistsSorted_Rooted(uint Node,
	  vector<double> &Dists) const;
	double GetBootFract(uint Node) const;
	double GetMedianLeafDist(uint Node) const;
	bool IsInSubtree(uint SubtreeNode, uint Node) const;
	void CollapseUnary();
	void CollapseUnaryNode(uint Node);

// OnNode callbacks
	bool OnNode_LCA(uint PrevNode, uint Node);
	bool OnNode_InsertRootAbove(uint PrevNode, uint Node);

public:
	static bool StaticOnNode_LCA(uint PrevNode, uint Node, void *UserData);
	static bool StaticOnNode_InsertRootAbove(uint PrevNode, uint Node, void *UserData);
	static double AddLengths(double  Length1, double  Length2);
	};

void TreesFromFile(const string &FileName, vector<TreeX *> &Trees);
void TreesToFile(const vector<TreeX *> &Trees, const string &FileName);
