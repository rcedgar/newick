#pragma once

#include "tree2.h"

class Splitter
	{
public:
	const Tree2 *m_T = 0;
	uint m_SplitIndex = 0;
	uint m_SplitCount = 0;
	uint m_TargetSize = 0;
	vector<uint> m_SubtreeNodes;

public:
	void Run(const Tree2 &T, uint SplitCount);
	uint GetBiggestNode() const;
	void WriteLabels(const string &FileNamePrefix) const;
	void LogState() const;
	void GetSizeOrder(vector<uint> &Order) const;
	void GetSubtree(Tree2 &Subtree) const;
	};
