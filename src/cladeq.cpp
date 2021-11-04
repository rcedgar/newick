#include "myutils.h"
#include "featuretable.h"
#include "biparter.h"

void SetFeatureTable(const TreeN &T, FeatureTable &FT);

static void GetThisOther(uint ValueIndex, uint Node, 
  const vector<uint> &ValueIndexToTotal, const vector<uint> &ValueCounts,
  uint &LeftThis, uint &LeftOther, uint &RightThis, uint &RightOther)
	{
	LeftThis = 0;
	LeftOther = 0;
	RightThis = 0;
	RightOther = 0;

	const uint ValueCount = SIZE(ValueIndexToTotal);
	for (uint ValueIndex2 = 0; ValueIndex2 < ValueCount; ++ValueIndex2)
		{
		uint Total = ValueIndexToTotal[ValueIndex2];
		uint n = ValueCounts[ValueIndex2];
		asserta(n <= Total);
		if (ValueIndex2 == ValueIndex)
			{
			LeftThis += n;
			RightThis += (Total - n);
			}
		else
			{
			LeftOther += n;
			RightOther += (Total - n);
			}
		}
	}

static uint GetWrongCount(uint Node, uint ValueIndex,
  const vector<uint> &ValueIndexToTotal,
  const vector<vector<uint> > &NodeToValueCounts)
	{
	uint N = ValueIndexToTotal[ValueIndex];
	const vector<uint> &ValueCounts = NodeToValueCounts[Node];
	if (SIZE(ValueCounts) == 0)
		return UINT_MAX;

	uint LeftThis, LeftOther, RightThis, RightOther;
	GetThisOther(ValueIndex, Node, ValueIndexToTotal, ValueCounts,
		LeftThis, LeftOther, RightThis, RightOther);
	asserta(LeftThis + RightThis == N);
	uint WrongLeft = RightThis + LeftOther;
	uint WrongRight = LeftThis + RightOther;
	uint Wrong = min(WrongLeft, WrongRight);
#if 0//TRACE
	Log("Node %u Value %u Lt %u Rt %u Lo %u Ro %u WL %u WR %u N %u\n",
	  Node, ValueIndex, LeftThis, RightThis, LeftOther, RightOther,
	  WrongLeft, WrongRight, N);
#endif
	return Wrong;
	}

static void DoValue(uint ValueIndex, const string &Value,
  const vector<uint> &ValueIndexToTotal,
  const vector<vector<uint> > &NodeToValueCounts,
  uint &BestNode, uint &BestWrongCount)
	{
	const uint NodeCount = SIZE(NodeToValueCounts);
	BestWrongCount = UINT_MAX;
	BestNode = UINT_MAX;
	for (uint Node = 0; Node < NodeCount; ++Node)
		{
		uint WrongCount = GetWrongCount(Node, ValueIndex,
		  ValueIndexToTotal, NodeToValueCounts);
		if (WrongCount < BestWrongCount)
			{
			BestWrongCount = WrongCount;
			BestNode = Node;
			}
		}
	uint N = ValueIndexToTotal[ValueIndex];
#if 0//TRACE
	Log(">>> Value %u %s WC %u / %u BestNode %u\n",
	  ValueIndex, Value.c_str(), BestWrongCount, N, BestNode);
#endif

// Special case because leaf+other partitions are not considered
	if (BestWrongCount >= N)
		BestWrongCount = N - 1;
	}

// CladeQ = Correct/N
//	Correct = sum_over_groups (Size-Wrong)
//	N = sum_over_groups (Size)
//	Wrong = minimum_over_edges (NrOther_in_BestSplit + NrThis_out_BestSplit)
void cmd_cladeq()
	{
	const string &TreeFileName = opt(cladeq);

	TreeN T;
	T.FromNewickFile(TreeFileName);

	FeatureTable FT;
	SetFeatureTable(T, FT);

	BiParter BP;
	BP.Init(T);

	vector<uint> ValueIndexToTotal;
	vector<vector<uint> > NodeToValueCounts;
	BP.CountFeatures(FT, ValueIndexToTotal, NodeToValueCounts);

	uint N = 0;
	uint SumWrong = 0;
	const uint ValueCount = FT.GetValueCount();
	for (uint ValueIndex = 0; ValueIndex < ValueCount; ++ValueIndex)
		{
		uint BestNode = UINT_MAX;
		uint WrongCount = UINT_MAX;
		uint Total = ValueIndexToTotal[ValueIndex];
		if (Total <= 1)
			continue;
		N += Total;
		const string &Value = FT.GetValue(ValueIndex);
		DoValue(ValueIndex, Value, ValueIndexToTotal, NodeToValueCounts,
		  BestNode, WrongCount);
		SumWrong += WrongCount;
		}

	if (N == 0)
		{
		asserta(SumWrong == 0);
		ProgressLog("Wrong = %u / %u, CladeQ = undefined\n", SumWrong, N);
		}
	else
		{
		asserta(N > 0);
		asserta(SumWrong < N);
		uint Correct = N - SumWrong;
		double CladeQ = double(Correct)/N;
		ProgressLog("Wrong = %u / %u, CladeQ = %.4f\n", SumWrong, N, CladeQ);
		}
	}
