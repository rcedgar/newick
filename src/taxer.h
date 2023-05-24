#pragma once

#include "treex.h"
#include "featuretable.h"

class Taxer
	{
public:
	uint m_TreeIndex = UINT_MAX;
	double m_MinTPFract = DBL_MAX;
	TreeX *m_T = 0;
	FeatureTable m_FT;
	uint m_ValueCount = 0;
	uint m_NodeIndexCount = 0;
	uint m_KFold = 0;
	set<uint> m_HoldOutSet;
	vector<vector<uint> > m_NodeToValueCountVec;
	map<uint, uint> m_BestNodeToValueIndex;

	vector<uint> m_BestNodes;
	vector<uint> m_Ns;
	vector<uint> m_TPs;
	vector<uint> m_FPs;
	vector<uint> m_FNs;
	vector<double> m_Accs;

	vector<uint> m_NodeToTrueValueIndex;
	vector<uint> m_NodeToPredictedValueIndex;
	vector<string> m_NodeToTrueValue;
	vector<string> m_NodeToPredictedValue;
	vector<string> m_NodeToResult;

	uint m_QN = 0;
	uint m_QTP = 0;
	uint m_QFP = 0;
	uint m_QFN = 0;
	uint m_RN = 0;
	uint m_RTP = 0;
	uint m_RFP = 0;
	uint m_RFN = 0;

public:
	void Reset()
		{
		m_TreeIndex = UINT_MAX;
		m_T = 0;
		m_FT.Clear();
		m_ValueCount = 0;
		m_NodeIndexCount = 0;
		m_KFold = 0;
		m_HoldOutSet.clear();
		m_NodeToValueCountVec.clear();
		m_BestNodeToValueIndex.clear();
		m_BestNodes.clear();
		m_Ns.clear();
		m_TPs.clear();
		m_FPs.clear();
		m_FNs.clear();
		m_Accs.clear();
		m_NodeToPredictedValueIndex.clear();
		m_NodeToTrueValueIndex.clear();
		m_NodeToPredictedValue.clear();
		m_NodeToTrueValue.clear();
		m_NodeToResult.clear();
		m_QN = 0;
		m_QTP = 0;
		m_QFP = 0;
		m_QFN = 0;
		m_RN = 0;
		m_RTP = 0;
		m_RFP = 0;
		m_RFN = 0;
		}

	void Init(uint TreeIndex, TreeX &T,
	  uint K, double MinTPFract);
	void SetHoldOut(uint K);
	void SetValueCountVec();
	void SetBestNode(uint ValueIndex);
	void SetBestNodes();
	void SetResults();
	void SetResult(uint Node);
	uint GetTrueValueIndex(uint Node) const;
	uint GetPredictedValueIndex(uint Node) const;
	bool IsQuery(uint Node) const;

	void ToFev(FILE *f) const;
	void ToTsv(FILE *f) const;
	void ToNewick(FILE *f) const;
	void TreeToFev(FILE *f) const;
	void ValueToFev(FILE *f, uint ValueIndex) const;
	void NodeToFev(FILE *f, uint Node) const;
	void NodeToTsv(FILE *f, uint Node) const;

public:
	static double GetAcc(uint N, uint TP, uint FP, uint FN);
	};
