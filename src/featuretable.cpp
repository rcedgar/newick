#include "myutils.h"
#include "featuretable.h"
#include "treen.h"
#include "treex.h"
#include <set>

void GetAccFromLabel(const string &Label, string &Acc)
	{
	string TmpAcc; // to allow Label and Acc args to be same string
	for (uint i = 0; i < SIZE(Label); ++i)
		{
		char c = Label[i];
		if (isalnum(c) || c == '_')
			TmpAcc += c;
		else
			break;
		}
	Acc = TmpAcc;
	}

uint FeatureTable::GetLabelIndex(const string &Label) const
	{
	string Label2;
	if (m_UseAccs)
		GetAccFromLabel(Label, Label2);
	else
		Label2 = Label;
	map<string, uint>::const_iterator p = m_LabelToIndex.find(Label2);
	if (p == m_LabelToIndex.end())
		return UINT_MAX;
	return p->second;
	}

void FeatureTable::GetLabels_ByValueIndex(uint ValueIndex, vector<string> &Labels) const
	{
	Labels.clear();
	const uint LabelCount = GetLabelCount();
	asserta(SIZE(m_LabelIndexToValueIndex) == LabelCount);
	for (uint LabelIndex = 0; LabelIndex < LabelCount; ++LabelIndex)
		{
		if (m_LabelIndexToValueIndex[LabelIndex] == ValueIndex)
			{
			const string &Label = m_Labels[LabelIndex];
			if (Label != "")
				Labels.push_back(Label);
			}
		}
	}

void FeatureTable::GetLabels_ByValueIndex(uint ValueIndex, set<string> &Labels) const
	{
	Labels.clear();
	const uint LabelCount = GetLabelCount();
	asserta(SIZE(m_LabelIndexToValueIndex) == LabelCount);
	for (uint LabelIndex = 0; LabelIndex < LabelCount; ++LabelIndex)
		{
		if (m_LabelIndexToValueIndex[LabelIndex] == ValueIndex)
			{
			const string &Label = m_Labels[LabelIndex];
			if (Label != "")
				Labels.insert(Label);
			}
		}
	}

uint FeatureTable::GetLabelCount_ByValueIndex(uint ValueIndex) const
	{
	const uint LabelCount = GetLabelCount();
	asserta(SIZE(m_LabelIndexToValueIndex) == LabelCount);
	uint n = 0;
	for (uint LabelIndex = 0; LabelIndex < LabelCount; ++LabelIndex)
		{
		if (m_LabelIndexToValueIndex[LabelIndex] == ValueIndex)
			++n;
		}
	return n;
	}

void FeatureTable::FromFile(const string &FileName)
	{
	vector<string> Labels;
	vector<string> Values;
	DataFromFile(FileName, Labels, Values);
	FromVecs(Labels, Values);
	}

void FeatureTable::DataFromFile(const string &FileName,
  vector<string> &Labels,
  vector<string > &Values)
	{
	Labels.clear();
	Values.clear();

	FILE *f = OpenStdioFile(FileName);
	string Line;
	vector<string> Fields;

	while (ReadLineStdioFile(f, Line))
		{
		Split(Line, Fields, '\t');
		if (SIZE(Fields) != 2)
			Die("Expected 2 fields in '%s'", FileName.c_str());
		Labels.push_back(Fields[0]);
		Values.push_back(Fields[1]);
		}
	}

void FeatureTable::FromVecs(const vector<string> &Labels,
  const vector<string> &Values)
	{
	Clear();
	const uint N = SIZE(Labels);
	asserta(SIZE(Values) == N);
	for (uint i = 0; i < N; ++i)
		{
		const string &FullLabel = Labels[i];
		string Label = FullLabel;
		if (m_UseAccs)
			GetAccFromLabel(FullLabel, Label);
		const string &Value = Values[i];
		if (m_LabelToIndex.find(Label) != m_LabelToIndex.end())
			Die("Dupe label >%s", Label.c_str());

		uint LabelIndex = SIZE(m_Labels);
		m_LabelToIndex[Label] = LabelIndex;
		m_Labels.push_back(Label);
		m_FullLabels.push_back(FullLabel);

		uint ValueIndex = UINT_MAX;
		if (Value != m_NA)
			{
			map<string, uint>::const_iterator p = m_ValueToIndex.find(Value);
			if (p == m_ValueToIndex.end())
				{
				ValueIndex = SIZE(m_Values);
				m_Values.push_back(Value);
				m_ValueToIndex[Value] = ValueIndex;
				}
			else
				ValueIndex = p->second;
			}
		m_LabelIndexToValueIndex.push_back(ValueIndex);
		}
	}

void FeatureTable::FromTree(const TreeN &T, char Sep, uint FieldIndex)
	{
	Clear();

	const uint NodeCount = T.GetNodeCount();
	vector<uint> Nodes;
	T.GetNodes(Nodes);

	for (uint k = 0; k < NodeCount; ++k)
		{
		uint Node = Nodes[k];
		if (!T.IsLeaf(Node))
			continue;
		const string &FullLabel = T.GetLabel(Node);
		if (FullLabel.empty())
			continue;

		string Acc;
		if (opt(accs))
			GetAccFromLabel(FullLabel, Acc);

		const string &IndexedLabel = (opt(accs) ? Acc : FullLabel);

		if (m_LabelToIndex.find(IndexedLabel) != m_LabelToIndex.end())
			Die("Dupe label >%s", IndexedLabel.c_str());
		uint LabelIndex = SIZE(m_Labels);
		m_Labels.push_back(IndexedLabel);
		m_LabelToIndex[IndexedLabel] = LabelIndex;

		vector<string> Fields;
		Split(FullLabel, Fields, Sep);
		if (SIZE(Fields) <= FieldIndex)
			continue;

		const string &Value = Fields[FieldIndex];
		if (Value.empty() || Value == m_NA)
			continue;

		if (m_ValueToIndex.find(Value) == m_ValueToIndex.end())
			{
			uint ValueIndex = SIZE(m_Values);
			m_Values.push_back(Value);
			m_ValueToIndex[Value] = ValueIndex;
			}
		}

	const uint LabelCount = SIZE(m_Labels);
	m_LabelIndexToValueIndex.resize(LabelCount, UINT_MAX);

	for (uint k = 0; k < NodeCount; ++k)
		{
		uint Node = Nodes[k];
		if (!T.IsLeaf(Node))
			continue;
		const string &FullLabel = T.GetLabel(Node);
		if (FullLabel.empty())
			continue;
		string IndexedLabel;
		if (opt(accs))
			{
			string Acc;
			GetAccFromLabel(FullLabel, Acc);
			IndexedLabel = Acc;
			}
		else
			IndexedLabel = FullLabel;
		vector<string> Fields;
		Split(FullLabel, Fields, Sep);
		if (SIZE(Fields) <= FieldIndex)
			continue;
		const string &Value = Fields[FieldIndex];
		if (Value.empty() || Value == m_NA)
			continue;

		uint LabelIndex = m_LabelToIndex[IndexedLabel];
		asserta(m_LabelToIndex.find(IndexedLabel) != m_LabelToIndex.end());

		asserta(m_ValueToIndex.find(Value) != m_ValueToIndex.end());
		uint ValueIndex = m_ValueToIndex[Value];
		m_LabelIndexToValueIndex[LabelIndex] = ValueIndex;
		m_LeafNodeSet.insert(Node);
		}
	}

void FeatureTable::SetLeafNodeSet(const TreeN &T)
	{
	asserta(T.IsNormalized());
	const uint NodeCount = T.GetNodeCount();
	for (uint Node = 0; Node < NodeCount; ++Node)
		{
		if (!T.IsLeaf(Node))
			continue;
		const string &FullLabel = T.GetLabel(Node);
		if (FullLabel.empty())
			continue;
		string IndexedLabel;
		if (opt(accs))
			{
			string Acc;
			GetAccFromLabel(FullLabel, Acc);
			IndexedLabel = Acc;
			}
		else
			IndexedLabel = FullLabel;

		uint ValueIndex = GetValueIndex_ByLabel(IndexedLabel);
		if (ValueIndex != UINT_MAX)
			m_LeafNodeSet.insert(Node);
		}
	}

void FeatureTable::SetLeafNodeSet(const TreeX &T)
	{
	const uint NodeCount = T.m_AssignedNodeCount;
	for (uint Node = 0; Node < NodeCount; ++Node)
		{
		if (!T.IsNode(Node))
			continue;
		if (!T.IsLeaf(Node))
			continue;
		const string &FullLabel = T.GetLabel(Node);
		if (FullLabel.empty())
			continue;
		string IndexedLabel;
		if (opt(accs))
			{
			string Acc;
			GetAccFromLabel(FullLabel, Acc);
			IndexedLabel = Acc;
			}
		else
			IndexedLabel = FullLabel;

		uint ValueIndex = GetValueIndex_ByLabel(IndexedLabel);
		if (ValueIndex != UINT_MAX)
			m_LeafNodeSet.insert(Node);
		}
	}

void FeatureTable::FromTree(const TreeX &T, char Sep, uint FieldIndex)
	{
	Clear();

	for (uint Node = 0; Node < T.m_AssignedNodeCount; ++Node)
		{
		if (!T.IsNode(Node))
			continue;
		if (!T.IsLeaf(Node))
			continue;
		const string &FullLabel = T.GetLabel(Node);
		if (FullLabel.empty())
			continue;

		string Acc;
		if (opt(accs))
			GetAccFromLabel(FullLabel, Acc);

		const string &IndexedLabel = (opt(accs) ? Acc : FullLabel);

		if (m_LabelToIndex.find(IndexedLabel) != m_LabelToIndex.end())
			Die("Dupe label >%s", IndexedLabel.c_str());
		uint LabelIndex = SIZE(m_Labels);
		m_Labels.push_back(IndexedLabel);
		m_LabelToIndex[IndexedLabel] = LabelIndex;

		vector<string> Fields;
		Split(FullLabel, Fields, Sep);
		if (SIZE(Fields) <= FieldIndex)
			continue;

		const string &Value = Fields[FieldIndex];
		if (Value.empty() || Value == m_NA)
			continue;

		if (m_ValueToIndex.find(Value) == m_ValueToIndex.end())
			{
			uint ValueIndex = SIZE(m_Values);
			m_Values.push_back(Value);
			m_ValueToIndex[Value] = ValueIndex;
			}
		}

	const uint LabelCount = SIZE(m_Labels);
	m_LabelIndexToValueIndex.resize(LabelCount, UINT_MAX);

	for (uint Node = 0; Node < T.m_AssignedNodeCount; ++Node)
		{
		if (!T.IsNode(Node))
			continue;
		if (!T.IsLeaf(Node))
			continue;
		const string &FullLabel = T.GetLabel(Node);
		if (FullLabel.empty())
			continue;
		string IndexedLabel;
		if (opt(accs))
			{
			string Acc;
			GetAccFromLabel(FullLabel, Acc);
			IndexedLabel = Acc;
			}
		else
			IndexedLabel = FullLabel;
		vector<string> Fields;
		Split(FullLabel, Fields, Sep);
		if (SIZE(Fields) <= FieldIndex)
			continue;
		const string &Value = Fields[FieldIndex];
		if (Value.empty() || Value == m_NA)
			continue;

		uint LabelIndex = m_LabelToIndex[IndexedLabel];
		asserta(m_LabelToIndex.find(IndexedLabel) != m_LabelToIndex.end());

		asserta(m_ValueToIndex.find(Value) != m_ValueToIndex.end());
		uint ValueIndex = m_ValueToIndex[Value];
		m_LabelIndexToValueIndex[LabelIndex] = ValueIndex;
		m_LeafNodeSet.insert(Node);
		}
	}
