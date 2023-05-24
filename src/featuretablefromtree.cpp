#include "myutils.h"
#include "featuretable.h"
#include "tree2.h"
#include "treen.h"
#include "treex.h"

void SetFeatureTable(const TreeN &T, FeatureTable &FT)
	{
	if (optset_ff)
		{
		const string &FF = opt(ff);
		if (SIZE(FF) != 2 || !isdigit(FF[1]))
			Die("Invalid ff");

		char Sep = FF[0];
		char Digit = FF[1];
		if (Digit == '0')
			Die("Invalid ff (field must be >0)");
		uint FieldIndex = uint(Digit - '0') - 1;
		FT.FromTree(T, Sep, FieldIndex);
		}
	else if (optset_features)
		{
		FT.FromFile(opt(features));
		FT.SetLeafNodeSet(T);
		}
	else
		Die("Must set -ff or -features");
	}

void SetFeatureTable(const TreeX &T, FeatureTable &FT)
	{
	if (optset_ff)
		{
		const string &FF = opt(ff);
		if (SIZE(FF) != 2 || !isdigit(FF[1]))
			Die("Invalid ff");

		char Sep = FF[0];
		char Digit = FF[1];
		if (Digit == '0')
			Die("Invalid ff (field must be >0)");
		uint FieldIndex = uint(Digit - '0') - 1;
		FT.FromTree(T, Sep, FieldIndex);
		}
	else if (optset_features)
		{
		FT.FromFile(opt(features));
		FT.SetLeafNodeSet(T);
		}
	else
		Die("Must set -ff or -features");
	}

void SetFeatureTable2(const Tree2 &T2, FeatureTable &FT)
	{
	TreeN TN;
	TN.FromTree2(T2);
	SetFeatureTable(TN, FT);
	}

void AppendValuesFromTree(const TreeN &T, char Sep, uint FieldIndex,
  const string &MissingValue, set<string> &Values)
	{
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

		vector<string> Fields;
		Split(FullLabel, Fields, Sep);
		if (SIZE(Fields) <= FieldIndex)
			continue;

		const string &Value = Fields[FieldIndex];
		if (Value.empty() || Value == MissingValue)
			continue;

		Values.insert(Value);
		}
	}

void GetValuesFromTrees(vector<TreeN *> &Trees, char Sep, uint FieldIndex,
  const string &MissingValue, vector<string> &Values)
	{
	Values.clear();

	set<string> setValues;
	const uint N = SIZE(Trees);
	for (uint i = 0; i < N; ++i)
		AppendValuesFromTree(*Trees[i], Sep, FieldIndex, MissingValue,
		  setValues);

	for (set<string>::const_iterator p = setValues.begin();
	  p != setValues.end(); ++p)
		Values.push_back(*p);
	}
