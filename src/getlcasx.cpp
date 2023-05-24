#include "myutils.h"
#include "featuretable.h"
#include "treex.h"

const double MIN_TP_FRACT = 0.5;

void GetGroupLeafNodeSet(const TreeX &T, const FeatureTable &FT,
  uint ValueIndex, set<uint> &LeafNodeSet)
	{
	LeafNodeSet.clear();
	asserta(!optset_accs);
	vector<string> FeatureLabels;
	FT.GetLabels_ByValueIndex(ValueIndex, FeatureLabels);
	for (uint i = 0; i < SIZE(FeatureLabels); ++i)
		{
		const string &Label = FeatureLabels[i];
		uint Node = T.GetNodeByLabel(Label, false);
		if (Node == UINT_MAX)
			continue;
		if (T.IsLeaf(Node))
			LeafNodeSet.insert(Node);
		}
	}

void GetLCAs(TreeX &T, const FeatureTable &FT, bool AllowInvert,
  vector<string> &Values, vector<uint> &LCAs, vector<uint> &GroupSizes,
  vector<uint> &SubtreeSizes, vector<uint> &FPs, vector<uint> &FNs,
  vector<double> &MonoFs)
	{
	asserta(SIZE(FT.m_LeafNodeSet) > 0);

	uint ValueCount = FT.GetValueCount();
	for (uint ValueIndex = 0; ValueIndex < ValueCount; ++ValueIndex)
		{
		const string &Value = FT.GetValue(ValueIndex);

		set<uint> GroupLeafNodes;
		GetGroupLeafNodeSet(T, FT, ValueIndex, GroupLeafNodes);

		uint TP = UINT_MAX;
		uint FP = UINT_MAX;
		uint FN = UINT_MAX;
		uint GroupSize = SIZE(GroupLeafNodes);
		uint SubtreeSize = SIZE(GroupLeafNodes);
		uint LCA = UINT_MAX;
		if (GroupSize > 0)
			{
			asserta(SIZE(FT.m_LeafNodeSet) > SIZE(GroupLeafNodes));
			LCA = T.GetBestFitSubtree(FT.m_LeafNodeSet, MIN_TP_FRACT,
			  TP, FP, FN);
			if (LCA == UINT_MAX)
				Warning("No LCA for %s", Value.c_str());
			}

		double MonoF = 1.0 - double(FP + FN)/(GroupSize + FP);

		Values.push_back(Value);
		LCAs.push_back(LCA);
		GroupSizes.push_back(GroupSize);
		SubtreeSizes.push_back(SubtreeSize);
		FPs.push_back(FP);
		FNs.push_back(FN);
		MonoFs.push_back(MonoF);

		Log("Value=%s", Value.c_str());
		Log(", GroupSize=%u", GroupSize); 
		Log(", SubtreeSize=%u", SubtreeSize); 
		Log(", LCA=%u", LCA);
		Log(", FPs=%u", FP);
		Log(", FNs=%u", FN);
		Log(", Errs=%u", FP + FN);
		Log(", MonoF=%.4f", MonoF);
		Log("\n");
		}	
	}
