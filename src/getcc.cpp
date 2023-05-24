#include "myutils.h"

#define TRACE	0

#if	TRACE
static vector<string> g_Labels;
#endif

void GetConnComps(const vector<uint> &FromIndexes, const vector<uint> &ToIndexes,
  vector<vector<uint> > &CCs, bool ShowProgress)
	{
	CCs.clear();

	const uint EdgeCount = SIZE(FromIndexes);
	asserta(SIZE(ToIndexes) == EdgeCount);
	if (EdgeCount == 0)
		return;

	uint MaxIndex = 0;
	for (uint EdgeIndex = 0; EdgeIndex < EdgeCount; ++EdgeIndex)
		{
		uint From = FromIndexes[EdgeIndex];
		uint To = ToIndexes[EdgeIndex];

		if (From > MaxIndex)
			MaxIndex = From;
		if (To > MaxIndex)
			MaxIndex = To;
		}
	uint SeqCount = MaxIndex + 1;

	vector<vector<uint> > AdjMx(SeqCount);
	for (uint NodeIndex = 0; NodeIndex < SeqCount; ++NodeIndex)
		AdjMx[NodeIndex].push_back(NodeIndex);

	for (uint EdgeIndex = 0; EdgeIndex < EdgeCount; ++EdgeIndex)
		{
		if (ShowProgress)
			ProgressStep(EdgeIndex, EdgeCount, "CCs adj mx");

		uint From = FromIndexes[EdgeIndex];
		asserta(From < SeqCount);

		uint To = ToIndexes[EdgeIndex];
		asserta(To < SeqCount);
#if	TRACE
		Log("BuildMx %s(%u) -> %s(%u)\n",
		  g_Labels[From].c_str(), From, g_Labels[To].c_str(), To);
#endif
		if (From != To)
			{
			AdjMx[From].push_back(To);
			AdjMx[To].push_back(From);
			}
		}

#if	DEBUG
	{
	for (uint NodeIndex1 = 0; NodeIndex1 < SeqCount; ++NodeIndex1)
		{
		const vector<uint> &v1 = AdjMx[NodeIndex1];
		for (uint i = 0; i < SIZE(v1); ++i)
			{
			uint NodeIndex2 = v1[i];
			const vector<uint> &v2 = AdjMx[NodeIndex2];
			bool Found = false;
			for (uint j = 0; j < SIZE(v2); ++j)
				{
				if (v2[j] == NodeIndex1)
					{
					Found = true;
					break;
					}
				}
			asserta(Found);
			}
		}
	}
#endif

#if	TRACE
	{
	Log("AdjMx\n");
	for (uint NodeIndex1 = 0; NodeIndex1 < SeqCount; ++NodeIndex1)
		{
		const vector<uint> &v = AdjMx[NodeIndex1];
		Log("%s(%u): ", g_Labels[NodeIndex1].c_str(), NodeIndex1);
		for (uint i = 0; i < SIZE(v); ++i)
			{
			uint NodeIndex2 = v[i];
			Log(" %s(%u)", g_Labels[NodeIndex2].c_str(), NodeIndex2);
			}
		Log("\n");
		}
	}
#endif
	vector<bool> Assigned(SeqCount, false);
	vector<bool> Pended(SeqCount, false);

	uint CCCount = 0;
	uint DoneCount = 0;
	for (uint NodeIndex = 0; NodeIndex < SeqCount; ++NodeIndex)
		{
		if (ShowProgress)
			ProgressStep(NodeIndex, SeqCount, "CCs clustering");

		if (Assigned[NodeIndex])
			{
			asserta(Pended[NodeIndex]);
			continue;
			}
#if	TRACE
		Log("\n");
		Log("CC%u  New CC, seed %u (%s)\n", CCCount, NodeIndex, g_Labels[NodeIndex].c_str());
#endif

		vector<uint> Empty;
		CCs.push_back(Empty);

		vector<uint> Pending;
		asserta(!Pended[NodeIndex]);
		Pending.push_back(NodeIndex);
		Pended[NodeIndex] = true;
		++DoneCount;

		while (!Pending.empty())
			{
			uint n = SIZE(Pending);
			asserta(n > 0);

			uint NodeIndex2 = Pending.back();
			Pending.pop_back();
#if	TRACE
			Log("CC%u pop %u(%s)\n", CCCount, NodeIndex2, g_Labels[NodeIndex2].c_str());
#endif

			asserta(NodeIndex2 < SeqCount);
			asserta(Pended[NodeIndex2]);
			asserta(!Assigned[NodeIndex2]);
			CCs[CCCount].push_back(NodeIndex2);
#if	TRACE
			Log("CC%u assign %u(%s)\n", CCCount, NodeIndex2, g_Labels[NodeIndex2].c_str());
#endif
			Assigned[NodeIndex2] = true;
			const vector<uint> &Neighbors = AdjMx[NodeIndex2];
			uint NeighborCount = SIZE(Neighbors);
			for (uint i = 0; i < NeighborCount; ++i)
				{
				uint NeighborNodeIndex = Neighbors[i];
				asserta(NeighborNodeIndex < SeqCount);
#if	TRACE
				Log("CC%u neighbor %u(%s) -> %u(%s), pended %c\n",
				  CCCount,
				  NodeIndex2, g_Labels[NodeIndex2].c_str(),
				  NeighborNodeIndex, g_Labels[NeighborNodeIndex].c_str(),
				  tof(Pended[NeighborNodeIndex]));
#endif
				if (!Pended[NeighborNodeIndex])
					{
					asserta(!Assigned[NeighborNodeIndex]);
					Pending.push_back(NeighborNodeIndex);
					Pended[NeighborNodeIndex] = true;
					++DoneCount;
#if	TRACE
					Log("CC%u pend %u(%s)\n", CCCount, NeighborNodeIndex, g_Labels[NeighborNodeIndex].c_str());
#endif
					}
				}
			}
		++CCCount;
		}
	}
