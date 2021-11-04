#include "myutils.h"
#include "clustermaker.h"
#include "sort.h"
#include <set>

void ClusterMaker::Run(const Tree2 &T, double MaxDistFromFarthestLeaf)
	{
	Clear();

	m_T = &T;
	m_MaxDistFromFarthestLeaf = MaxDistFromFarthestLeaf;

	vector<double> Dists;
	const uint NodeCount = m_T->GetNodeCount();
	for (uint Node = 0; Node < NodeCount; ++Node)
		{
		double d = m_T->GetMaxLeafDist(Node);
		if (d <= MaxDistFromFarthestLeaf)
			{
			uint Parent = m_T->GetParent(Node);
			double dp = m_T->GetMaxLeafDist(Parent);
			if (dp > MaxDistFromFarthestLeaf)
				{
				Dists.push_back(d);
				m_SubtreeNodes.push_back(Node);

				vector<uint> SubtreeLeafNodes;
				m_T->GetSubtreeLeafNodes(Node, SubtreeLeafNodes);
				m_SubtreeLeafNodesVec.push_back(SubtreeLeafNodes);
				}
			}
		}
	Validate();
	}

void ClusterMaker::Validate() const
	{
	set<uint> NodeSet;
	const uint SelectedNodeCount = SIZE(m_SubtreeNodes);
	asserta(SIZE(m_SubtreeLeafNodesVec) == SelectedNodeCount);
	for (uint i = 0; i < SelectedNodeCount; ++i)
		{
		uint Node = m_SubtreeNodes[i];
		asserta(NodeSet.find(Node) == NodeSet.end());
		NodeSet.insert(Node);
		}

	const uint LeafCount = m_T->GetLeafCount();
	uint SelectedLeafCount = 0;
	for (uint i = 0; i < SelectedNodeCount; ++i)
		{
		uint Node = m_SubtreeNodes[i];
		vector<uint> Path;
		m_T->GetPathToRoot(Node, Path);
		for (uint j = 0; j < SIZE(Path); ++j)
			{
			uint Node2 = Path[j];
			if (j == 0)
				asserta(Node2 == Node);
			else
				asserta(NodeSet.find(Node2) == NodeSet.end());
			}
		const vector<uint> &SubtreeLeafNodes = m_SubtreeLeafNodesVec[i];
		uint SubtreeLeafCount = SIZE(SubtreeLeafNodes);
		SelectedLeafCount += SubtreeLeafCount;
		}
	asserta(SelectedLeafCount = LeafCount);
	}

void ClusterMaker::ToTSV(const string &FileName) const
	{
	if (FileName.empty())
		return;
	FILE *f = CreateStdioFile(FileName);

	const uint SelectedNodeCount = SIZE(m_SubtreeNodes);
	for (uint i = 0; i < SelectedNodeCount; ++i)
		{
		uint Node = m_SubtreeNodes[i];
		const vector<uint> &LeafNodes = m_SubtreeLeafNodesVec[i];
		const uint SubtreeLeafCount = SIZE(LeafNodes);
		fprintf(f, "C\t%u\t%u\n", i+1, SubtreeLeafCount);
		}

	for (uint i = 0; i < SelectedNodeCount; ++i)
		{
		uint Node = m_SubtreeNodes[i];
		const vector<uint> &LeafNodes = m_SubtreeLeafNodesVec[i];
		const uint SubtreeLeafCount = SIZE(LeafNodes);
		for (uint j = 0; j < SubtreeLeafCount; ++j)
			{
			const uint LeafNode = LeafNodes[j];
			const string &Label = m_T->GetLabel(LeafNode);
			double Dist = m_T->GetDistance(LeafNode, Node);
			asserta(Dist <= m_MaxDistFromFarthestLeaf);

			fprintf(f, "L");
			fprintf(f, "\t%u", i+1);
			fprintf(f, "\t%u", Node);
			fprintf(f, "\t%u", LeafNode);
			fprintf(f, "\t%.4g", Dist);
			fprintf(f, "\t%s", Label.c_str());
			fprintf(f, "\n");
			}
		}
	CloseStdioFile(f);
	ProgressLog("MaxDist=%.4g, Clusters=%u\n",
	  m_MaxDistFromFarthestLeaf, SelectedNodeCount);
	}

void ClusterMaker::ToNewick(const string &FileName) const
	{
	if (FileName.empty())
		return;

	vector<string> NewLabels;
	const uint N = SIZE(m_SubtreeNodes);
	for (uint i = 0; i < N; ++i)
		{
		string NewLabel;
		Ps(NewLabel, "Cluster%u", i+1);
		NewLabels.push_back(NewLabel);
		}

	Tree2 SubsetTree;
	MakeSubsetNodes(*m_T, m_SubtreeNodes, NewLabels, SubsetTree);

	SubsetTree.ToNewickFile(FileName);
	}

void cmd_cluster()
	{
	const string &TreeFileName = opt(cluster);
	Tree2 T;
	T.FromFile(TreeFileName);

	double MaxDistFromFarthestLeaf = opt(maxdist);

	ClusterMaker CM;
	CM.Run(T, MaxDistFromFarthestLeaf);
	CM.ToTSV(opt(tsvout));
	CM.ToNewick(opt(output));
	}
