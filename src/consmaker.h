#pragma once

#include "treen.h"

class ConsMaker
	{
public:
	const TreeN *m_Tree1 = 0;
	const TreeN *m_Tree2 = 0;

	set<string> m_LabelsBoth;
	set<string> m_Labels1Only;
	set<string> m_Labels2Only;

	TreeN m_ConsTree;

	vector<uint> m_Pures1;
	vector<uint> m_Pures2;

	uint m_NewSubtreeCount = 0;
	uint m_NewNodeCount = 0;

public:
	void MakeConsensus(const TreeN &Tree1, const TreeN &Tree2);
	void MakeConsensus_Disjoint();
	void AddSubtree(uint SubtreeNode2);
	void InsertSubtree(uint SubtreeNode2, uint NodeC);
	void InsertSubtreeNode(const TreeN &T, uint NodeT, uint NodeC);
	void ValidateConsensusLabels();

public:
	static void IntersectLabels(const TreeN &Tree1, const TreeN &Tree2,
	  set<string> &LabelsBoth, set<string> &Labels1Only, set<string> &Labels2Only,
	  bool ErrorIfEmpty, bool ErrorIfDupe);
	};
