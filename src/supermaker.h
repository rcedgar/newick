#pragma once

#include "treen.h"

class SuperMaker
	{
public:
	uint m_MinLeafCount = 10;

	vector<TreeN *> m_Trees;
	vector<uint> m_NodeIndexes;
	vector<uint> m_TreeIndexes;
	vector<set<uint> > m_LabelIndexSets;
	vector<double> m_Scores;

	vector<string> m_Labels;
	map<string, uint> m_LabelToIndex;
	vector<vector<uint> > m_LabelIndexToTreeIndexes;
	vector<vector<uint> > m_LabelIndexToLeafNodeIndexes;

	vector<vector<double> > m_RootDists;
	vector<vector<uint> > m_LeafCounts;
	vector<vector<double> > m_Bootstraps;

public:

	void Load(const string &FileName);
	const uint GetTreeCount() const { return SIZE(m_Trees); }
	void AddLabel(uint TreeIndex, uint Node, const string &Label);
	const vector<double> &GetRootDists(uint TreeIndex) const;
	const vector<double> &GetBootstraps(uint TreeIndex) const;
	const vector<uint> &GetLeafCounts(uint TreeIndex) const;
	double CalcScore(uint TreeIndex, uint NodeIndex) const;
	double CalcUpperLeafDist(uint TreeIndex, uint NodeIndex) const;
	double GetRootDist(uint TreeIndex, uint NodeIndex) const;
	uint GetLeafCount(uint TreeIndex, uint NodeIndex) const;
	double GetBootstrap(uint TreeIndex, uint NodeIndex) const;
	const TreeN &GetTree(uint TreeIndex) const;
	};
