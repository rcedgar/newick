#pragma once

#include "treex.h"
#include "featuretable.h"

class DivConker
	{
public:
	vector<TreeX *> m_Trees;
	map<string, uint> m_LabelToIndex;
	map<pair<uint, uint>, uint> m_EdgeToCount;
	vector<uint> m_SelectedFroms;
	vector<uint> m_SelectedTos;
	vector<string> m_Labels;
	uint m_LabelCount = 0;
	double m_MinTPFract = 0.5;
	double m_MinMedianLeafDist = 1.5;
	double m_MaxMedianLeafDist = 2.3;
	double m_MinBootstrapFract = 0.85;
	uint m_MinClusterSize = 8;
	double m_MaxPct = 95;
	uint m_MaxClusters = 50;

	vector<vector<uint> > m_CCs;
	vector<uint> m_SortedCCIndexes;
	vector<uint> m_SortedCCSizes;

public:
	void Reset()
		{
		m_Trees.clear();
		m_LabelToIndex.clear();
		m_EdgeToCount.clear();
		m_SelectedFroms.clear();
		m_SelectedTos.clear();
		m_Labels.clear();
		m_CCs.clear();
		m_SortedCCIndexes.clear();
		m_SortedCCSizes.clear();
		}

	void Run(const vector<TreeX *> &Trees);
	uint GetTreeCount() const { return SIZE(m_Trees); }
	uint GetLabelIndex(const string &Label, bool Init);
	const string &GetLabel(uint LabelIndex) const;
	double GetNodeScore(const TreeX &T, uint Node) const;
	void AddTreeToIndex(const TreeX &T, uint TreeIndex);
	void AddEdges(const vector<string> &Labels);
	void AddTree(uint TreeIndex);
	void SelectEdges();
	void MakeCCs();
	void CCsToTsv(FILE *f) const;
	void CCToTsv(FILE *f, uint k, uint CCindex) const;
	void GetCCLabelVec(vector<vector<string> > &LabelVec) const;
	};
