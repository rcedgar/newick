#pragma once

#include "treen.h"
#include "featuretable.h"
#include <set>

class BiParter
	{
public:
	TreeN m_T;

// Leaf labels sorted alphabetically, this so that
// any tree with same set of labels will have 
// same label indexes.
	vector<string> m_Labels;
	map<string, uint> m_LabelToIndex;
	vector<uint> m_NodeToLabelIndex;
	vector<uint> m_LabelIndexToNode;

// PartVec[Node] is boolean vector.
// Vector has one true/false for each label index.
// Labels are indexed as above.
// True/false is disambiguated by requiring that first 
// entry is always false.
	vector<vector<bool> > m_PartVec;
	vector<uint> m_UniquePartNodes;

// One vector of node indexes for every entry 
// in m_UniquePartNodes.
	vector<vector<uint> > m_NodeToDupeNodes;

// Hash table entries are node indexes.
// Collisions should be rare.
	vector<vector<uint> > m_HashTable;

public:
	void Init(const TreeN &T);
	void LogMe() const;
	void LogPatterns() const;
	uint Search(const vector<bool> &Part) const;
	void SearchBestMatch(const vector<string> &Labels,
	  uint &Node, vector<string> &MissingLabels, uint &OtherCount) const;
	void CountFeatures(const FeatureTable &FT,
	  vector<uint> &ValueIndexToTotal,
	  vector<vector<uint> > &NodeToValueCounts);
	void GetPartInternalNodeLabels(uint PartIndex, vector<string> &Labels) const;
	void GetPartInternalNodes(uint PartIndex, vector<uint> &Nodes) const;
	void ToTSV(const string &Name, FILE *f) const;
	uint GetLabelIndex(const string &Label) const;
	uint GetOtherCount(const vector<bool> &Part, const vector<bool> &Query) const;

private:
	void SetHashTable();
	uint GetHash(const vector<bool> &Part) const;
	uint GetHash(uint Node) const;
	};
