#pragma once

#include "tree2.h"

class ClusterMaker
	{
public:
	const Tree2 *m_T = 0;
	double m_MaxDistFromFarthestLeaf = DBL_MAX;
	vector<uint> m_SubtreeNodes;
	vector<vector<uint> > m_SubtreeLeafNodesVec;

	void Clear()
		{
		m_T = 0;
		m_MaxDistFromFarthestLeaf = DBL_MAX;
		m_SubtreeNodes.clear();
		m_SubtreeLeafNodesVec.clear();
		}

public:
	void Run(const Tree2 &T, double MaxDistFromFarthestLeaf);
	void Validate() const;
	void ToTSV(const string &FileName) const;
	void ToNewick(const string &FileName) const;
	uint GetSubsetSize(uint MaxPerCluster) const;
	};
