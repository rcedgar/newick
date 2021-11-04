#include "myutils.h"
#include "clustermaker.h"

void Shuffle(vector<unsigned> &v);

uint ClusterMaker::GetSubsetSize(uint MaxPerCluster) const
	{
	uint Size = 0;
	const uint SelectedNodeCount = SIZE(m_SubtreeNodes);
	for (uint i = 0; i < SelectedNodeCount; ++i)
		{
		uint Node = m_SubtreeNodes[i];
		const vector<uint> &LeafNodes = m_SubtreeLeafNodesVec[i];
		const uint SubtreeLeafCount = SIZE(LeafNodes);
		if (SubtreeLeafCount >= 3)
			Size += 3;
		else
			{
			asserta(SubtreeLeafCount == 1 || SubtreeLeafCount == 2);
			Size += SubtreeLeafCount;
			}
		}
	return Size;
	}

void cmd_cluster2()
	{
	const string &TreeFileName = opt(cluster2);
	Tree2 T;
	T.FromFile(TreeFileName);

	asserta(optset_subsetsize);
	asserta(optset_maxpercluster);
	const uint TargetSubsetSize = opt(subsetsize);
	const uint MaxPerCluster = opt(maxpercluster);

	ClusterMaker CM;
	uint SizeLo = UINT_MAX;
	uint SizeHi = UINT_MAX;
	double DistLo = -1;
	double DistHi = -1;

	const uint ITERS1 = 100;
	const float FACTOR = 1.1;
	double d = 0.01;
	for (uint Iter = 0; Iter < ITERS1; ++Iter)
		{
		CM.Run(T, d);
		uint Size = CM.GetSubsetSize(MaxPerCluster);
		ProgressStep(Iter, ITERS1, "Pass 1 d=%.3g size=%u", d, Size);
		if (Size < TargetSubsetSize)
			{
			DistLo = d;
			SizeLo = Size;
			ProgressStep(ITERS1-1, ITERS1, "Pass 1 d=%.3g size=%u", d, Size);
			break;
			}
		if (Size > TargetSubsetSize)
			{
			DistHi = d;
			SizeHi = Size;
			}
		d *= FACTOR;
		}

	asserta(DistLo > DistHi);
	asserta(DistLo != DistHi);
	const uint ITERS2 = 100;
	double Delta = (DistLo - DistHi)/100;
	uint BestSize = SizeLo;
	uint BestAbsDiff = UINT_MAX;
	double Bestd = DistLo;
	for (uint Iter = 0; Iter < ITERS2; ++Iter)
		{
		ProgressStep(Iter, ITERS2, "Pass 2 d=%.3g size=%u", Bestd, BestSize);
		double d = DistHi + Iter*Delta;
		CM.Run(T, d);
		uint Size = CM.GetSubsetSize(MaxPerCluster);
		uint AbsDiff = (Size > TargetSubsetSize ?
		  Size - TargetSubsetSize :
		  TargetSubsetSize - Size);
		if (BestSize == UINT_MAX || AbsDiff < BestAbsDiff)
			{
			BestSize = Size;
			BestAbsDiff = AbsDiff;
			Bestd = d;
			if (BestAbsDiff == 0)
				{
				ProgressStep(ITERS2-1, ITERS2, "Pass 2 d=%.3g size=%u", Bestd, BestSize);
				break;
				}
			}
		}

	CM.Run(T, Bestd);
	uint Size = CM.GetSubsetSize(MaxPerCluster);
	ProgressLog("Bestd %.3g size %u\n", Bestd, Size);
	CM.ToTSV(opt(tsvout));
	CM.ToNewick(opt(output));

	if (!optset_labelsout)
		return;

	FILE *f = CreateStdioFile(opt(labelsout));
	const uint N = SIZE(CM.m_SubtreeLeafNodesVec);
	for (uint i = 0; i < N; ++i)
		{
		vector<uint> LeafNodes = CM.m_SubtreeLeafNodesVec[i];
		Shuffle(LeafNodes);
		uint n = SIZE(LeafNodes);
		asserta(n > 0);
		if (n > MaxPerCluster)
			n = MaxPerCluster;
		for (uint j = 0; j < n; ++j)
			{
			uint Node = LeafNodes[j];
			const string &Label = T.GetLabel(Node);
			fprintf(f, "%s\n", Label.c_str());
			}
		}
	CloseStdioFile(f);
	}
