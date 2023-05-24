#include "myutils.h"
#include "taxer.h"

void Taxer::SetHoldOut(uint K)
	{
	m_KFold = K;
	if (K == 0)
		return;
	asserta(K > 1);

	set<string> LabelSet;
	set<uint> LeafNodeSet;
	for (uint ValueIndex = 0; ValueIndex < m_ValueCount; ++ValueIndex)
		{
		m_FT.GetLabels_ByValueIndex(ValueIndex, LabelSet);
		m_T->LabelSetToLeafNodeSet(LabelSet, LeafNodeSet);
		for (set<uint>::const_iterator p = LeafNodeSet.begin();
			p != LeafNodeSet.end(); ++p)
			{
			uint r = randu32();
			if (r%m_KFold == 0)
				{
				uint LeafNode = *p;
				m_HoldOutSet.insert(LeafNode);
				}
			}
		}
	}

void Taxer::SetValueCountVec()
	{
	vector<uint> Path;
	set<string> LabelSet;
	set<uint> LeafNodeSet;
	m_NodeToValueCountVec.clear();
	m_NodeToValueCountVec.resize(m_NodeIndexCount);
	for (uint Node = 0; Node < m_NodeIndexCount; ++Node)
		m_NodeToValueCountVec[Node].resize(m_ValueCount, 0);

	for (uint ValueIndex = 0; ValueIndex < m_ValueCount; ++ValueIndex)
		{
		m_FT.GetLabels_ByValueIndex(ValueIndex, LabelSet);
		m_T->LabelSetToLeafNodeSet(LabelSet, LeafNodeSet);

		for (set<uint>::const_iterator p = LeafNodeSet.begin();
		  p != LeafNodeSet.end(); ++p)
			{
			uint LeafNode = *p;
			if (m_KFold != UINT_MAX &&
			  m_HoldOutSet.find(LeafNode) != m_HoldOutSet.end())
				continue;

			m_T->GetPathToOrigin2(LeafNode, Path);
			for (uint i = 0; i < SIZE(Path); ++i)
				{
				uint Node = Path[i];
				m_NodeToValueCountVec[Node][ValueIndex] += 1;
				}
			}
		}
	}

double Taxer::GetAcc(uint N, uint TP, uint FP, uint FN)
	{
	asserta(TP <= N && FN <= N);
	if (N == 0)
		return 0;
	//double Acc = double(TP)/(N + FP + FN);
	double Acc = double(TP)/(N + FP);
	return Acc;
	}

void Taxer::SetBestNode(uint ValueIndex)
	{
	uint N = m_FT.GetLabelCount_ByValueIndex(ValueIndex);
	uint BestNode = UINT_MAX;
	double BestAcc = -1;
	uint TP = UINT_MAX;
	uint FP = UINT_MAX;
	uint FN = UINT_MAX;

	uint MinTP = uint(N*m_MinTPFract);
	if (MinTP == 0)
		MinTP = 1;
	for (uint Node = 0; Node < m_NodeIndexCount; ++Node)
		{
		if (!m_T->IsNode(Node))
			continue;
		const vector<uint> &ValueToCount = m_NodeToValueCountVec[Node];
		asserta(SIZE(ValueToCount) == m_ValueCount);
		uint TP2 = 0;
		uint FP2 = 0;
		for (uint ValueIndex2 = 0; ValueIndex2 < m_ValueCount;
			++ValueIndex2)
			{
			uint n = ValueToCount[ValueIndex2];
			if (ValueIndex2 == ValueIndex)
				TP2 = n;
			else
				FP2 += n;
			}
		if (TP2 < MinTP)
			continue;
		asserta(TP2 <= N);
		uint FN2 = N - TP2;
//		uint Errs2 = FP2 + 2*FN2;
		double Acc2 = GetAcc(N, TP2, FP2, FN2);
		if (BestNode == UINT_MAX || Acc2 >= BestAcc)
			{
			BestNode = Node;
			BestAcc = Acc2;
			TP = TP2;
			FP = FP2;
			FN = FN2;
			}
		}

	m_Ns[ValueIndex] = N;
	m_TPs[ValueIndex] = TP;
	m_FPs[ValueIndex] = FP;
	m_FNs[ValueIndex] = FN;
	m_Accs[ValueIndex] = BestAcc;

	if (BestNode != UINT_MAX)
		{
		m_BestNodeToValueIndex[BestNode] = ValueIndex;

		const string &ValueName = m_FT.GetValue(ValueIndex);
		string NewLabel = "LCA_" + ValueName;
		m_T->UpdateLabel(BestNode, NewLabel);
		}
	}

void Taxer::SetBestNodes()
	{
#define X(x)	m_##x.clear(); m_##x.resize(m_ValueCount, UINT_MAX)
	X(BestNodes);
	X(Ns);
	X(TPs);
	X(FPs);
	X(FNs);
#undef X
	m_Accs.resize(m_ValueCount, DBL_MAX);
	m_BestNodeToValueIndex.clear();

	for (uint ValueIndex = 0; ValueIndex < m_ValueCount; ++ValueIndex)
		SetBestNode(ValueIndex);
	}

uint Taxer::GetPredictedValueIndex(uint Node) const
	{
	if (!m_T->IsLeaf(Node))
		return UINT_MAX;

	vector<uint> Path;
	m_T->GetPathToOrigin2(Node, Path);
	uint ValueIndex = UINT_MAX;
	for (uint i = 0; i < SIZE(Path); ++i)
		{
		uint Node2 = Path[i];
		map<uint, uint>::const_iterator p =
		  m_BestNodeToValueIndex.find(Node2);
		if (p != m_BestNodeToValueIndex.end())
			{
			ValueIndex = p->second;
			return ValueIndex;
			}
		}
	return UINT_MAX;
	}

bool Taxer::IsQuery(uint Node) const
	{
	if (!m_T->IsLeaf(Node))
		return false;
	if (m_HoldOutSet.find(Node) == m_HoldOutSet.end())
		return false;
	return true;
	}

void Taxer::SetResult(uint Node)
	{
	if (!m_T->IsLeaf(Node))
		return;
	uint TrueValueIndex = GetTrueValueIndex(Node);
	uint PredictedValueIndex = GetPredictedValueIndex(Node);

	const string &TrueValue = m_FT.GetValue(TrueValueIndex);
	const string &PredictedValue = m_FT.GetValue(PredictedValueIndex);
	bool Correct = (TrueValue == PredictedValue);
	bool HasTrueValue = (TrueValueIndex != UINT_MAX);
	bool HasPredictedValue = (PredictedValueIndex != UINT_MAX);

	m_NodeToTrueValueIndex[Node] = TrueValueIndex;
	m_NodeToPredictedValueIndex[Node] = PredictedValueIndex;

	m_NodeToTrueValue[Node] = TrueValue;
	m_NodeToPredictedValue[Node] = PredictedValue;

	bool IsQ = IsQuery(Node);
	string Result;
	if (IsQ)
		{
		if (HasTrueValue)
			{
			++m_QN;
			if (Correct)
				{
				++m_QTP;
				Result = "QTP";
				}
			else
				{
				if (HasPredictedValue)
					{
					++m_QFP;
					Result = "QFP";
					}
				else
					{
					++m_QFN;
					Result = "QFN";
					}
				}
			}
		else
			Result = "QUN";
		}
	else
		{
		if (HasTrueValue)
			{
			++m_RN;
			if (Correct)
				{
				++m_RTP;
				Result = "RTP";
				}
			else
				{
				if (HasPredictedValue)
					{
					++m_RFP;
					Result = "RFP";
					}
				else
					{
					++m_RFN;
					Result = "RFN";
					}
				}
			}
		else
			Result = "RUN";
		}
	m_NodeToResult[Node] = Result;
	}

void Taxer::SetResults()
	{
	m_QN = 0;
	m_QTP = 0;
	m_QFP = 0;
	m_QFN = 0;
	m_RN = 0;
	m_RTP = 0;
	m_RFP = 0;
	m_RFN = 0;

	m_NodeToResult.resize(m_NodeIndexCount, "IN");

	m_NodeToTrueValueIndex.resize(m_NodeIndexCount, UINT_MAX);
	m_NodeToPredictedValueIndex.resize(m_NodeIndexCount, UINT_MAX);

	m_NodeToTrueValue.resize(m_NodeIndexCount, "IN");
	m_NodeToPredictedValue.resize(m_NodeIndexCount, "IN");

	for (uint Node = 0; Node < m_NodeIndexCount; ++Node)
		SetResult(Node);
	}

uint Taxer::GetTrueValueIndex(uint Node) const
	{
	if (!m_T->IsLeaf(Node))
		return UINT_MAX;
	const string &Label = m_T->GetLabel(Node);
	uint TrueValueIndex = m_FT.GetValueIndex_ByLabel(Label);
	return TrueValueIndex;
	}

void Taxer::Init(uint TreeIndex, TreeX &T,
  uint K, double MinTPFract)
	{
	asserta(K != 1);
	if (K == UINT_MAX)
		K = 0;
	asserta(MinTPFract > 0 && MinTPFract <= 1);

	Reset();
	m_TreeIndex = TreeIndex;
	m_T = &T;
	m_KFold = K;
	m_MinTPFract = MinTPFract;

	SetFeatureTable(T, m_FT);
	m_ValueCount = m_FT.GetValueCount();
	m_NodeIndexCount = m_T->m_AssignedNodeCount;
	SetHoldOut(K);
	SetValueCountVec();
	SetBestNodes();
	SetResults();
	}

void Taxer::ToNewick(FILE *f) const
	{
	m_T->ToNewickFile(f, true);
	}

void Taxer::ToTsv(FILE *f) const
	{
	if (f == 0)
		return;
	for (uint NodeIndex = 0; NodeIndex < m_NodeIndexCount; ++NodeIndex)
		NodeToTsv(f, NodeIndex);
	}

void Taxer::ToFev(FILE *f) const
	{
	if (f == 0)
		return;
	TreeToFev(f);
	for (uint ValueIndex = 0; ValueIndex < m_ValueCount; ++ValueIndex)
		ValueToFev(f, ValueIndex);
	for (uint NodeIndex = 0; NodeIndex < m_NodeIndexCount; ++NodeIndex)
		NodeToFev(f, NodeIndex);
	}

void Taxer::ValueToFev(FILE *f, uint ValueIndex) const
	{
	if (f == 0)
		return;

	fprintf(f, "value=%s", m_FT.GetValue(ValueIndex).c_str());
	fprintf(f, "\tacc=%.3g", m_Accs[ValueIndex]);
	fprintf(f, "\tN=%u", m_Ns[ValueIndex]);
	fprintf(f, "\tTP=%u", m_TPs[ValueIndex]);
	fprintf(f, "\tFP=%u", m_FPs[ValueIndex]);
	fprintf(f, "\tFN=%u", m_FNs[ValueIndex]);
	fprintf(f, "\n");
	}

void Taxer::NodeToFev(FILE *f, uint Node) const
	{
	if (f == 0)
		return;

	if (!m_T->IsLeaf(Node))
		return;

	const string &Label = m_T->GetLabel(Node);
	const string &PredVal = m_NodeToPredictedValue[Node];
	const string &TrueVal = m_NodeToTrueValue[Node];
	const string &Result = m_NodeToResult[Node];

	fprintf(f, "leaf=%u", m_TreeIndex);
	fprintf(f, "\tnode=%u", Node);
	fprintf(f, "\tlabel=%s", Label.c_str());
	fprintf(f, "\tpred=%s", PredVal.c_str());
	fprintf(f, "\ttrue=%s", TrueVal.c_str());
	fprintf(f, "\tresult=%s", Result.c_str());
	fprintf(f, "\n");
	}

void Taxer::NodeToTsv(FILE *f, uint Node) const
	{
	if (f == 0)
		return;

	if (!m_T->IsLeaf(Node))
		return;

	const string &Label = m_T->GetLabel(Node);
	const string &PredVal = m_NodeToPredictedValue[Node];
	const string &TrueVal = m_NodeToTrueValue[Node];
	const string &Result = m_NodeToResult[Node];

	fprintf(f, "%u", m_TreeIndex);
	fprintf(f, "\t%s", Label.c_str());
	fprintf(f, "\t%s", PredVal.c_str());
	fprintf(f, "\t%s", TrueVal.c_str());
	fprintf(f, "\t%s", Result.c_str());
	fprintf(f, "\n");
	}

void Taxer::TreeToFev(FILE *f) const
	{
	double QAcc = GetAcc(m_QN, m_QTP, m_QFP, m_QFN);
	double RAcc = GetAcc(m_RN, m_RTP, m_RFP, m_RFN);

	fprintf(f, "tree=%u", m_TreeIndex);
	fprintf(f, "\tqacc=%.3g", QAcc);
	fprintf(f, "\tracc=%.3g", RAcc);

#define X(x)	fprintf(f, "\t%s=%u", #x, m_##x)
	X(QN);
	X(QTP);
	X(QFP);
	X(QFN);
	X(RN);
	X(RTP);
	X(RFP);
	X(RFN);
#undef X
	fprintf(f, "\n");
	}
