#include "myutils.h"
#include "featuretable.h"
#include "treen.h"

void GetGroupLeafNodeSet(const TreeN &T, const FeatureTable &FT,
  uint ValueIndex, set<uint> &LeafNodeSet)
	{
	vector<string> FeatureLabels;
	FT.GetLabels_ByValueIndex(ValueIndex, FeatureLabels);
	for (uint i = 0; i < SIZE(FeatureLabels); ++i)
		{
		const string &Label = FeatureLabels[i];
		uint Node = (opt(accs) ?
		  T.GetNodeByAcc(Label, false) : T.GetNodeByLabel(Label, false));
		if (Node == UINT_MAX)
			continue;
		if (T.IsLeaf(Node))
			LeafNodeSet.insert(Node);
		}
	}

void GetLCAs(const TreeN &T, const FeatureTable &FT, bool AllowInvert,
  vector<string> &Values, vector<uint> &LCAs, vector<uint> &GroupSizes,
  vector<uint> &SubtreeSizes, vector<uint> &FPs, vector<uint> &FNs,
  vector<bool> &Inverts, vector<double> &MonoFs)
	{
	asserta(SIZE(FT.m_LeafNodeSet) > 0);

	uint ValueCount = FT.GetValueCount();
	for (uint ValueIndex = 0; ValueIndex < ValueCount; ++ValueIndex)
		{
		const string &Value = FT.GetValue(ValueIndex);

		set<uint> GroupLeafNodes;
		GetGroupLeafNodeSet(T, FT, ValueIndex, GroupLeafNodes);

		uint FP = UINT_MAX;
		uint FN = UINT_MAX;
		bool Invert = false;
		uint GroupSize = SIZE(GroupLeafNodes);
		uint SubtreeSize = SIZE(GroupLeafNodes);
		uint LCA = UINT_MAX;
		if (GroupSize > 0)
			{
			asserta(SIZE(FT.m_LeafNodeSet) > SIZE(GroupLeafNodes));
			LCA = T.GetBestFitSubtree2(FT.m_LeafNodeSet, GroupLeafNodes,
			  AllowInvert, FP, FN, Invert);
			asserta(LCA != UINT_MAX);
			}

		double MonoF = 1.0 - double(FP + FN)/(GroupSize + FP);

		Values.push_back(Value);
		LCAs.push_back(LCA);
		GroupSizes.push_back(GroupSize);
		SubtreeSizes.push_back(SubtreeSize);
		FPs.push_back(FP);
		FNs.push_back(FN);
		Inverts.push_back(Invert);
		MonoFs.push_back(MonoF);

		Log("Value=%s", Value.c_str());
		Log(", GroupSize=%u", GroupSize); 
		Log(", SubtreeSize=%u", SubtreeSize); 
		Log(", LCA=%u", LCA);
		Log(", Invert %c", tof(Invert));
		Log(", FPs=%u", FP);
		Log(", FNs=%u", FN);
		Log(", Errs=%u", FP + FN);
		Log(", MonoF=%.4f", MonoF);
		Log("\n");
		}	
	}
