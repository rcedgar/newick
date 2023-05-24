#include "myutils.h"
#include "featuretable.h"
#include "treen.h"

void SetFeatureTable(const TreeN &T, FeatureTable &FT);
void GetGroupLeafNodeSet(const TreeN &T, const FeatureTable &FT,
  uint ValueIndex, set<uint> &LeafNodeSet);

void cmd_bestfitsubtree()
	{
	TreeN T;
	T.FromNewickFile(opt(bestfitsubtree));

	asserta(optset_label);
	const string &TheValue = opt(label);

	bool Rooted;
	bool Binary = T.IsBinary(Rooted);
	asserta(Rooted);
	const uint NodeCount = T.GetNodeCount();

	FeatureTable FT;
	SetFeatureTable(T, FT);
	uint ValueCount = FT.GetValueCount();

	const uint TheValueIndex = FT.GetValueIndex(TheValue);
	asserta(TheValueIndex < ValueCount);

	vector<uint> NodeToValue(NodeCount, UINT_MAX);
	uint TheValueSize = 0;
	for (uint Node = 0; Node < NodeCount; ++Node)
		{
		if (!T.IsLeaf(Node))
			continue;
		const string &Label = T.GetLabel(Node);
		uint ValueIndex = FT.GetValueIndex_ByLabel(Label);
		if (ValueIndex == TheValueIndex)
			++TheValueSize;
		NodeToValue[Node] = ValueIndex;
		}

	Log(" Node         Label     TP     FP     FN   Errs\n");
	//   12345  123456789012  12345  12345  12345  12345

	uint BestNode = UINT_MAX;
	uint BestErrs = UINT_MAX;
	for (uint SubtreeNode = 0; SubtreeNode < NodeCount; ++SubtreeNode)
		{
		asserta(T.IsNode(SubtreeNode));

		const string &SubtreeLabel = T.GetLabel(SubtreeNode);

		vector<uint> SubtreeLeafNodes;
		T.AppendSubtreeLeafNodes(SubtreeNode, SubtreeLeafNodes);
		const uint N = SIZE(SubtreeLeafNodes);

		vector<uint> ValueToCount(ValueCount, 0);
		uint SubtreeSize = 0;
		uint TP = 0;
		uint FP = 0;
		for (uint i = 0; i < N; ++i)
			{
			uint Leaf = SubtreeLeafNodes[i];
			uint Value = NodeToValue[Leaf];
			if (Value != UINT_MAX)
				{
				asserta(Value < ValueCount);
				++(ValueToCount[Value]);
				++SubtreeSize;
				if (Value == TheValueIndex)
					++TP;
				else
					++FP;
				}
			}

		if (TP < TheValueSize/4)
			continue;

		asserta(TP == ValueToCount[TheValueIndex]);
		asserta(TP <= TheValueSize);
		uint FN = TheValueSize - TP;
		uint Errs = FP + FN;
		if (BestNode == UINT_MAX || Errs < BestErrs)
			{
			BestNode = SubtreeNode;
			BestErrs = Errs;
			}

		Log("%5u", SubtreeNode);
		Log("  %12.12s", SubtreeLabel.c_str());
		Log("  %5u", TP);
		Log("  %5u", FP);
		Log("  %5u", FN);
		Log("  %5u", Errs);

		for (uint ValueIndex = 0; ValueIndex < ValueCount; ++ValueIndex)
			{
			if (ValueIndex == TheValueIndex)
				continue;
			const string &Value = FT.GetValue(ValueIndex);
			uint n = ValueToCount[ValueIndex];
			if (n > 0)
				Log("  %s=%u", Value.c_str(), n);
			}

		Log("\n");
		}

	Log("BestNode %u\n", BestNode);
	}
