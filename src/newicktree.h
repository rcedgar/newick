#pragma once

const double MISSING_LENGTH = DBL_MAX;

class NewickTree
	{
public:
// There is always a root, even if unrooted.
	uint m_Root;
	vector<uint> m_Parents;
	vector<string> m_Labels;
	vector<bool> m_IsLeafs;
	vector<double> m_Lengths;

public:
	NewickTree() { Clear(); }

	void Clear()
		{
		m_Root = UINT_MAX;
		m_Parents.clear();
		m_Labels.clear();
		m_IsLeafs.clear();
		m_Lengths.clear();
		}
	uint GetNodeCount() const { return SIZE(m_Labels); }

	void LogMe() const;
	uint GetLeafCount() const;
	void Validate() const;
	uint GetParent(uint Node) const;
	const string &GetLabel(uint Node) const;
	double GetLength(uint Node) const;
	void GetNonParentEdges(vector<vector<uint> > &Edges) const;
	bool IsBinary() const;
	bool HasBinaryRoot() const;
	};
