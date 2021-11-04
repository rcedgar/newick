#pragma once

#include <map>
#include <set>

class TreeN;

class FeatureTable
	{
public:
	vector<string> m_Labels;
	vector<string> m_FullLabels;
	vector<string> m_Values;
	map<string, uint> m_LabelToIndex;
	map<string, uint> m_ValueToIndex;
	vector<uint> m_LabelIndexToValueIndex;
	string m_NA = ".";
	bool m_UseAccs = opt(accs);

// Subset of leaf nodes with features
	set<uint> m_LeafNodeSet;

public:
	void Clear()
		{
		m_Labels.clear();
		m_FullLabels.clear();
		m_Values.clear();
		m_LabelToIndex.clear();
		m_ValueToIndex.clear();
		m_LabelIndexToValueIndex.clear();
		m_NA = ".";
		m_LeafNodeSet.clear();
		}

	void FromFile(const string &FileName);
	void FromVecs(const vector<string> &Labels,
	  const vector<string > &Values);
	void DataFromFile(const string &FileName,
	  vector<string> &Labels,
	  vector<string> &Values);
	void FromTree(const TreeN &T, char Sep, uint FieldIndex);

	uint GetLabelCount() const { return SIZE(m_Labels); }
	uint GetValueCount() const { return SIZE(m_Values); }

	const string &GetLabel(uint LabelIndex) const
		{
		asserta(LabelIndex < SIZE(m_Labels));
		return m_Labels[LabelIndex];
		}

	const string &GetValue(uint ValueIndex) const
		{
		if (ValueIndex == UINT_MAX)
			return m_NA;
		asserta(ValueIndex < SIZE(m_Values));
		return m_Values[ValueIndex];
		}

	uint GetValueIndex(const string &Value) const
		{
		map<string, uint>::const_iterator p = m_ValueToIndex.find(Value);
		if (p == m_ValueToIndex.end())
			return UINT_MAX;
		return p->second;
		}

	uint GetValueIndex_ByLabel(const string &Label) const
		{
		uint LabelIndex = GetLabelIndex(Label);
		if (LabelIndex == UINT_MAX)
			return UINT_MAX;
		assert(LabelIndex < SIZE(m_LabelIndexToValueIndex));
		uint ValueIndex = m_LabelIndexToValueIndex[LabelIndex];
		asserta(ValueIndex == UINT_MAX || ValueIndex < SIZE(m_Values));
		return ValueIndex;
		}

	void GetValue_ByLabel(const string &Label, string &Value,
	  bool ErrorIfNotFound) const
		{
		Value.clear();
		uint LabelIndex = GetLabelIndex(Label);
		if (LabelIndex == UINT_MAX)
			{
			if (ErrorIfNotFound)
				Die("No value for label '%s'", Label.c_str());
			return;
			}
		assert(LabelIndex < SIZE(m_LabelIndexToValueIndex));
		uint ValueIndex = m_LabelIndexToValueIndex[LabelIndex];
		asserta(ValueIndex == UINT_MAX || ValueIndex < SIZE(m_Values));
		if (ValueIndex == UINT_MAX)
			{
			if (ErrorIfNotFound)
				Die("No value for label '%s'", Label.c_str());
			return;
			}
		Value = m_Values[ValueIndex];
		}

	uint GetValueIndex_ByLabelIndex(uint LabelIndex) const
		{
		asserta(LabelIndex < SIZE(m_LabelIndexToValueIndex));
		uint ValueIndex = m_LabelIndexToValueIndex[LabelIndex];
		asserta(ValueIndex == UINT_MAX || ValueIndex < SIZE(m_Values));
		return ValueIndex;
		}

	uint GetLabelIndex_FailOnError(const string &Label) const
		{
		map<string, uint>::const_iterator p = m_LabelToIndex.find(Label);
		if (p == m_LabelToIndex.end())
			Die("Unknown label '%s'", Label.c_str());
		return p->second;
		}

	uint GetLabelIndex(const string &Label) const;
	uint GetLabelCount_ByValueIndex(uint ValueIndex) const;
	void GetLabels_ByValueIndex(uint ValueIndex, vector<string> &Labels) const;

	void SetLeafNodeSet(const TreeN &T);
	};
