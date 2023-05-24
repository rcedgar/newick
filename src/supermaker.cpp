#include "myutils.h"
#include "supermaker.h"
#include "sort.h"

void SuperMaker::Load(const string &FileName)
	{
	asserta(m_Trees.empty());

	TreesFromFile(FileName, m_Trees);

	const uint TreeCount = GetTreeCount();

	m_RootDists.resize(TreeCount);
	m_LeafCounts.resize(TreeCount);
	m_Bootstraps.resize(TreeCount);

	for (uint TreeIndex = 0; TreeIndex < TreeCount; ++TreeIndex)
		{
		const TreeN &T = *m_Trees[TreeIndex];

		T.GetRootDists(m_RootDists[TreeIndex]);
		T.GetSubtreeLeafCounts(m_LeafCounts[TreeIndex]);
		T.GetBootstraps(m_Bootstraps[TreeIndex]);

		vector<uint> Nodes;
		T.GetNodes(Nodes);
		const uint N = SIZE(Nodes);
		for (uint i = 0; i < N; ++i)
			{
			uint Node = Nodes[i];
			if (T.IsLeaf(Node))
				{
				const string &Label = T.GetLabel(Node);
				AddLabel(TreeIndex, Node, Label);
				}
			}
		}
	}

void SuperMaker::AddLabel(uint TreeIndex, uint Node, const string &Label)
	{
	uint Index = UINT_MAX;
	if (m_LabelToIndex.find(Label) == m_LabelToIndex.end())
		{
		Index = SIZE(m_Labels);
		m_Labels.push_back(Label);
		m_LabelToIndex[Label] = Index;

		vector<uint> Empty;
		m_LabelIndexToTreeIndexes.push_back(Empty);
		m_LabelIndexToLeafNodeIndexes.push_back(Empty);
		}
	else
		Index = m_LabelToIndex[Label];

	m_LabelIndexToTreeIndexes[Index].push_back(TreeIndex);
	m_LabelIndexToLeafNodeIndexes[Index].push_back(Node);
	}

const TreeN &SuperMaker::GetTree(uint TreeIndex) const
	{
	asserta(TreeIndex < SIZE(m_Trees));
	return *m_Trees[TreeIndex];
	}

const vector<double> &SuperMaker::GetRootDists(uint TreeIndex) const
	{
	asserta(TreeIndex < SIZE(m_RootDists));
	return m_RootDists[TreeIndex];
	}

const vector<double> &SuperMaker::GetBootstraps(uint TreeIndex) const
	{
	asserta(TreeIndex < SIZE(m_Bootstraps));
	return m_Bootstraps[TreeIndex];
	}

const vector<uint> &SuperMaker::GetLeafCounts(uint TreeIndex) const
	{
	asserta(TreeIndex < SIZE(m_LeafCounts));
	return m_LeafCounts[TreeIndex];
	}

double SuperMaker::CalcUpperLeafDist(uint TreeIndex, uint NodeIndex) const
	{
	const vector<double> &RootDists = GetRootDists(TreeIndex);
	const TreeN &T = GetTree(TreeIndex);
	vector<uint> LeafNodes;
	T.GetSubtreeLeafNodes(NodeIndex, LeafNodes);
	asserta(NodeIndex < SIZE(RootDists));
	double ThisRootDist = RootDists[NodeIndex];
	vector<double> LeafDists;
	const uint N = SIZE(LeafNodes);
	for (uint i = 0; i < N; ++i)
		{
		uint Leaf = LeafNodes[i];
		asserta(Leaf < SIZE(RootDists));
		double d = RootDists[Leaf];
		LeafDists.push_back(d - ThisRootDist);
		}
	vector<uint> Order;
	QuickSortInPlace(LeafDists.data(), N);
	double UpperDist = LeafDists[3*N/4];
	return UpperDist;
	}

double SuperMaker::GetBootstrap(uint TreeIndex, uint NodeIndex) const
	{
	const vector<double>& v = GetBootstraps(TreeIndex);
	asserta(NodeIndex < SIZE(v));
	return v[NodeIndex];
	}

double SuperMaker::GetRootDist(uint TreeIndex, uint NodeIndex) const
	{
	const vector<double>& v = GetRootDists(TreeIndex);
	asserta(NodeIndex < SIZE(v));
	return v[NodeIndex];
	}

uint SuperMaker::GetLeafCount(uint TreeIndex, uint NodeIndex) const
	{
	const vector<uint> &v = GetLeafCounts(TreeIndex);
	asserta(NodeIndex < SIZE(v));
	return v[NodeIndex];
	}

double SuperMaker::CalcScore(uint TreeIndex, uint NodeIndex) const
	{
	uint LeafCount = GetLeafCount(TreeIndex, NodeIndex);
	if (LeafCount < m_MinLeafCount)
		return 0;
	double Bootstrap = GetBootstrap(TreeIndex, NodeIndex);
	double RootDist = GetRootDist(TreeIndex, NodeIndex);
	return 0;
	}
