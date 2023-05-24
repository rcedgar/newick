#include "myutils.h"
#include "divconker.h"
#include "quarts.h"
#include "sort.h"
#include <map>

void GetGroupLeafNodeSet(const TreeX &T, const FeatureTable &FT,
  uint ValueIndex, set<uint> &LeafNodeSet);
void GetConnComps(const vector<uint> &FromIndexes, const vector<uint> &ToIndexes,
  vector<vector<uint> > &CCs, bool ShowProgress);

double DivConker::GetNodeScore(const TreeX &T, uint Node) const
	{
	if (!T.IsNode(Node))
		return -1;
	if (T.IsLeaf(Node))
		return -1;
	if (Node == T.m_Origin)
		return -1;
	double BF = T.GetBootFract(Node);
	if (BF < m_MinBootstrapFract)
		return -1;
	uint n = T.GetSubtreeLeafCount_Rooted(Node);
	if (n < m_MinClusterSize)
		return -1;
	double d = T.GetMedianLeafDist(Node);
	if (d < m_MinMedianLeafDist || d > m_MaxMedianLeafDist)
		return -1;
	double Score = BF*sqrt(n);
	return Score;
	}

uint DivConker::GetLabelIndex(const string &Label, bool Init)
	{
	if (Label == "")
		return UINT_MAX;
	uint Index = UINT_MAX;
	if (Init)
		{
		asserta(m_LabelToIndex.find(Label) == m_LabelToIndex.end());
		m_Labels.push_back(Label);
		Index = m_LabelCount++;
		m_LabelToIndex[Label] = Index;
		}
	else
		{
		map<string, uint>::const_iterator p = m_LabelToIndex.find(Label);
		asserta(p != m_LabelToIndex.end());
		Index = p->second;
		}
	return Index;
	}

void DivConker::AddTreeToIndex(const TreeX &T, uint TreeIndex)
	{
	const uint NodeCount = T.GetNodeIndexCount();
	for (uint Node = 0; Node < NodeCount; ++Node)
		{
		if (T.IsLeaf(Node))
			{
			const string &Label = T.GetLabel(Node);
			GetLabelIndex(Label, TreeIndex == 0);
			}
		}
	}

void DivConker::AddEdges(const vector<string> &Labels)
	{
	vector<uint> LabelIndexes;
	const uint N = SIZE(Labels);
	for (uint i = 0; i < N; ++i)
		{
		uint LabelIndex = GetLabelIndex(Labels[i], false);
		if (LabelIndex != UINT_MAX)
			{
			asserta(LabelIndex < m_LabelCount);
			LabelIndexes.push_back(LabelIndex);
			}
		}
	
	const uint M = SIZE(LabelIndexes);
	for (uint i = 0; i < M; ++i)
		{
		uint ni = LabelIndexes[i];
		asserta(ni < m_LabelCount);
		for (uint j = i + 1; j < M; ++j)
			{
			uint nj = LabelIndexes[j];
			asserta(nj < m_LabelCount);

			uint nmin = min(ni, nj);
			uint nmax = max(ni, nj);
			pair<uint, uint> Edge(nmin, nmax);
			map<pair<uint, uint>, uint>::const_iterator p =
			  m_EdgeToCount.find(Edge);
			if (p == m_EdgeToCount.end())
				m_EdgeToCount[Edge] = 1;
			else
				m_EdgeToCount[Edge] += 1;
			}
		}
	ProgressLog("%s edges\n", IntToStr(m_EdgeToCount.size()));
	}

void DivConker::AddTree(uint TreeIndex)
	{
	TreeX &T = *m_Trees[TreeIndex];
	AddTreeToIndex(T, TreeIndex);

	uint LeafCount = T.GetLeafCount();
	uint AssignedCount = 0;
	for (uint Iter = 0; Iter < m_MaxClusters; ++Iter)
		{
		uint TopNode = UINT_MAX;
		double TopScore = 1;
		for (uint Node = 0; Node < T.GetNodeIndexCount(); ++Node)
			{
			if (!T.IsNode(Node))
				continue;
			double Score = GetNodeScore(T, Node);
			if (Score > TopScore)
				{
				TopScore = Score;
				TopNode = Node;
				}
			}
		if (TopNode == UINT_MAX)
			break;

		vector<uint> LeafNodes;
		T.GetSubtreeLeafNodes_Rooted(TopNode, LeafNodes);
		uint Size = SIZE(LeafNodes);

		vector<string> SubtreeLeafLabels;
		for (uint i = 0; i < Size; ++i)
			{
			uint LeafNode = LeafNodes[i];
			const string &Label = T.GetLabel(LeafNode);
			if (Label != "")
				SubtreeLeafLabels.push_back(Label);
			}

		AddEdges(SubtreeLeafLabels);

		AssignedCount += Size;
		double Pct = GetPct(AssignedCount, LeafCount);
		ProgressLog("Tree %u iter %u DQ%u(%.3g) size=%u total=%u / %u (%.1f%%)\n",
			TreeIndex, Iter, TopNode, TopScore, Size, AssignedCount, LeafCount, Pct);
		T.DeleteSubtree(TopNode, "", false);
		T.Validate();
		asserta(T.IsRootedBinary());
		if (Pct > m_MaxPct)
			break;
		}
	Log("\n");
	}

void DivConker::SelectEdges()
	{
	uint TreeCount = GetTreeCount();
	const uint EdgeCount = SIZE(m_EdgeToCount);
	const uint MinCount = TreeCount/2 + 1;
	for (map<pair<uint, uint>, uint>::const_iterator p =
	  m_EdgeToCount.begin(); p != m_EdgeToCount.end(); ++p)
		{
		uint Count = p->second;
		if (Count >= MinCount)
			{
			const pair<uint, uint> &Edge = p->first;
			uint From = Edge.first;
			uint To = Edge.second;
			asserta(From < m_LabelCount);
			asserta(To < m_LabelCount);
			m_SelectedFroms.push_back(From);
			m_SelectedTos.push_back(To);
			}
		}

	uint SelectedEdgeCount = SIZE(m_SelectedFroms);
	ProgressLog("%u / %u edges selected\n",
	  SelectedEdgeCount, EdgeCount);
	}

void DivConker::MakeCCs()
	{
	m_CCs.clear();
	vector<vector<uint> > CCs;
	GetConnComps(m_SelectedFroms, m_SelectedTos, CCs, true);
	const uint CCCount = SIZE(CCs);

	map<uint, uint> CCToSize;
	for (uint i = 0; i < CCCount; ++i)
		{
		uint Size = SIZE(CCs[i]);
		if (Size >= m_MinClusterSize)
			{
			uint CCIndex = SIZE(m_CCs);
			m_CCs.push_back(CCs[i]);
			CCToSize[CCIndex] = Size;
			}
		}

	CountMapToVecs(CCToSize, m_SortedCCIndexes, m_SortedCCSizes);
	}

void DivConker::Run(const vector<TreeX *> &Trees)
	{
	Reset();
	m_Trees = Trees;
	uint TreeCount = SIZE(Trees);
	for (uint TreeIndex = 0; TreeIndex < TreeCount; ++TreeIndex)
		AddTree(TreeIndex);
	SelectEdges();
	MakeCCs();
	}

void DivConker::CCsToTsv(FILE *f) const
	{
	if (f == 0)
		return;

	const uint CCCount = SIZE(m_SortedCCIndexes);
	for (uint k = 0; k < CCCount; ++k)
		{
		uint CCIndex = m_SortedCCIndexes[k];
		CCToTsv(f, k, CCIndex);
		}
	}

const string &DivConker::GetLabel(uint LabelIndex) const
	{
	asserta(LabelIndex < SIZE(m_Labels));
	return m_Labels[LabelIndex];
	}

void DivConker::CCToTsv(FILE *f, uint k, uint CCIndex) const
	{
	if (f == 0)
		return;

	asserta(CCIndex < SIZE(m_CCs));
	const vector<uint> &LabelIndexes = m_CCs[CCIndex];

	const uint n = SIZE(LabelIndexes);
	for (uint i = 0; i < n; ++i)
		{
		fprintf(f, "%u", k);
		uint LabelIndex = LabelIndexes[i];
		const string &Label = GetLabel(LabelIndex);
		fprintf(f, "\t%s", Label.c_str());
		fprintf(f, "\n");
		}
	}

void DivConker::GetCCLabelVec(vector<vector<string> > &LabelVec) const
	{
	const uint CCCount = SIZE(m_SortedCCIndexes);
	LabelVec.resize(CCCount);
	for (uint k = 0; k < CCCount; ++k)
		{
		uint CCIndex = m_SortedCCIndexes[k];
		const vector<uint> &LabelIndexes = m_CCs[CCIndex];

		const uint n = SIZE(LabelIndexes);
		for (uint i = 0; i < n; ++i)
			{
			uint LabelIndex = LabelIndexes[i];
			const string &Label = GetLabel(LabelIndex);
			LabelVec[k].push_back(Label);
			}
		}
	}
