#pragma once

#include "treen.h"

class ConsOrder
	{
public:
	vector<set<string> *> m_LabelSetVec;
	vector<vector<uint> > m_Mx;
	vector<uint> m_Pending;
	uint m_TreeCount = UINT_MAX;
	vector<string> m_Names;

public:
	void AddTree(const TreeN &T);
	void InitMx();
	void InitPending();
	uint GetFirstOnly(uint i, uint j) const;
	void GetBestJoin(uint &i, uint &j) const;
	void DeleteFromPending(uint i, uint j);
	uint Join(uint i, uint j);
	const char *GetName(uint i) const;
	};
