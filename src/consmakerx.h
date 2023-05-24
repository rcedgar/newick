#pragma once

#include "treex.h"

class ConsMakerX
	{
public:
	const TreeX *m_Tree1 = 0;
	const TreeX *m_Tree2 = 0;

	set<string> m_LabelsBoth;
	set<string> m_Labels1Only;
	set<string> m_Labels2Only;

	set<uint> m_LeafNodes2Only;

	TreeX m_ConsTree;

	vector<uint> m_Pures1;
	vector<uint> m_Pures2;

	uint m_NewSubtreeCount = 0;
	uint m_NewNodeCount = 0;

public:
	void MakeConsensus(const TreeX &Tree1, const TreeX &Tree2);
	void MakeConsensus_Disjoint();
	void AddSubtree(uint SubtreeNode2);
	void InsertSubtree(uint SubtreeNode2, uint NodeC);
	void ValidateConsensusLabels();

public:
	static void IntersectLabels(const TreeX &Tree1, const TreeX &Tree2,
	  set<string> &LabelsBoth, set<string> &Labels1Only, set<string> &Labels2Only,
	  bool ErrorIfEmpty, bool ErrorIfDupe);
	};
