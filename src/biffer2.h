#pragma once

#include "tree2.h"
#include "featuretable.h"

uint32_t murmur3_32(const byte* key, size_t len, uint32_t seed);
typedef vector<bool> *vbptr;

// Enumerate bifurcations of a tree
class Biffer2
	{
public:
	Tree2 *m_T;
	uint m_LabelCount;
	map<string, uint> m_LabelToIndex;
	vector<uint> m_NodeToLabelIndex;
	vector<string> m_Labels;
	map<uint, vector<uint> > m_HashTable;
	vector<vbptr> m_Bifs;
	vector<uint> m_BifNodesi;
	vector<uint> m_BifNodesj;

public:
	void Clear()
		{
		m_T = 0;
		m_LabelCount = 0;
		m_LabelToIndex.clear();
		m_NodeToLabelIndex.clear();
		m_Labels.clear();
		m_HashTable.clear();
		m_BifNodesi.clear();
		m_BifNodesj.clear();
		}

	Biffer2()
		{
		Clear();
		}

public:
	void Init(Tree2 &T, bool IncludeLeaves);
	void Init_Labels(Tree2 &T, const map<string, uint> &LabelToIndex,
	  bool IncludeLeaves);
	void BuildHashTable(bool IncludeLeaves);
	void LogMe() const;

public:
	uint GetLeafCount() const	{ return m_LabelCount; }
	uint GetLabelCount() const	{ return m_LabelCount; }
	uint GetBifCount() const	{ return SIZE(m_Bifs); }
	const string &GetLabel(uint LabelIndex) const
		{
		asserta(LabelIndex < SIZE(m_Labels));
		return m_Labels[LabelIndex];
		}

public:
	const vector<bool> &GetBif(uint BifIndex) const;
	uint GetBifByEdge(uint Nodei, uint Nodej, vector<bool> &Bif) const;
	uint GetBifIndex_ByEdge(uint Nodei, uint Nodej) const;
	void OrientBif(vector<bool> &Bif) const;
	bool ShouldInvert(const vector<bool> &Bif) const;
	uint32 HashBif(const vector<bool> &Bif) const;
	void BifToStr(const vector<bool> &Bif, bool Yes, string &s) const;
	bool BifEq(const vector<bool> &Bif1,
	  const vector<bool> &Bif2) const;
	};
