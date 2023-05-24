#include "myutils.h"
#include "featuretable.h"
#include "treen.h"

void StringsFromFile(const string &FileName, vector<string> &Strings);
void TreesFromFile(const string &FileName, vector<TreeN *> &Trees);
void TreesToFile(const vector<TreeN *> &Trees, const string &FileName);

void TreeN::GetLeafNodeSet(set<uint> &LeafNodeSet) const
	{
	LeafNodeSet.clear();
	for (map<uint, uint>::const_iterator p = m_NodeToParent.begin();
	  p != m_NodeToParent.end(); ++p)
		{
		uint Node = p->first;
		if (IsLeaf(Node))
			LeafNodeSet.insert(Node);
		}
	}

uint TreeN::GetBestFitSubtree1(const set<uint> &InGroupLeafNodes,
  bool AllowInvert, uint &FPs, uint &FNs, bool &Invert) const
	{
	set<uint> AllLeafNodes;
	GetLeafNodeSet(AllLeafNodes);
	uint BestSubtree = GetBestFitSubtree2(AllLeafNodes, InGroupLeafNodes,
	  AllowInvert, FPs, FNs, Invert);
	return BestSubtree;
	}

uint TreeN::GetBestFitSubtree2(const set<uint> &AllGroupLeafNodes,
  const set<uint> &InGroupLeafNodes, bool AllowInvert,
  uint &FPs, uint &FNs, bool &Invert) const
	{
	FPs = 0;
	FNs = 0;
	const uint InGroupSize = SIZE(InGroupLeafNodes);
	const uint AllGroupSize = SIZE(AllGroupLeafNodes);
	
	asserta(InGroupSize > 0);
	asserta(InGroupSize <= AllGroupSize);
	for (set<uint>::const_iterator p = InGroupLeafNodes.begin();
	  p != InGroupLeafNodes.end(); ++p)
		{
		uint LeafNode = *p;
		asserta(IsLeaf(LeafNode));
		asserta(AllGroupLeafNodes.find(LeafNode) 
		  != AllGroupLeafNodes.end());
		}
	if (InGroupSize == 1)
		{
		uint LeafNode = *InGroupLeafNodes.begin();
		return LeafNode;
		}

	uint OutgroupSize = AllGroupSize - InGroupSize;
	if (OutgroupSize == 0)
		return m_Root;
	asserta(OutgroupSize > 0);

	const uint MinFNs = InGroupSize/2;
	uint MinErrs = 2*InGroupSize;
	uint BestFitNode = UINT_MAX;
	Invert = false;
	uint NodeCount = GetNodeCount();
	for (uint SubtreeNode = 0; SubtreeNode < NodeCount; ++SubtreeNode)
		{
		asserta(IsNode(SubtreeNode));
		if (IsLeaf(SubtreeNode))
			continue;

		vector<uint> SubtreeLeafNodes;
		AppendSubtreeLeafNodes(SubtreeNode, SubtreeLeafNodes);
		const uint SubtreeSize = SIZE(SubtreeLeafNodes);

		uint SubtreeInGroupCount = 0;
		uint SubtreeOtherCount = 0;
		for (uint i = 0; i < SubtreeSize; ++i)
			{
			uint LeafNode = SubtreeLeafNodes[i];
			set<uint>::const_iterator pAll = AllGroupLeafNodes.find(LeafNode);
			if (pAll == AllGroupLeafNodes.end())
				continue;

			set<uint>::const_iterator pIn = InGroupLeafNodes.find(LeafNode);
			bool InGroup = (pIn != InGroupLeafNodes.end());
			if (InGroup)
				++SubtreeInGroupCount;
			else
				++SubtreeOtherCount;
			}

		asserta(InGroupSize >= SubtreeInGroupCount);
		uint SubtreeFNs = InGroupSize - SubtreeInGroupCount;
		uint SubtreeFPs = SubtreeOtherCount;
		uint SubtreeErrs = SubtreeFPs + SubtreeFNs;
		if (SubtreeFNs <= MinFNs && SubtreeErrs < MinErrs)
			{
			Invert = false;
			MinErrs = SubtreeErrs;
			BestFitNode = SubtreeNode;
			FPs = SubtreeFPs;
			FNs = SubtreeFNs;
			}

		if (AllowInvert)
			{
			uint SubtreeAllGroupCount = SubtreeInGroupCount + SubtreeOtherCount;
			asserta(SubtreeAllGroupCount <= AllGroupSize);

			asserta(AllGroupSize >= SubtreeAllGroupCount);
			uint InvertAllGroupCount = AllGroupSize - SubtreeAllGroupCount;

			asserta(InGroupSize >= SubtreeInGroupCount);
			uint InvertInGroupCount = InGroupSize - SubtreeInGroupCount;

			asserta(InvertAllGroupCount >= InvertInGroupCount);
			uint InvertOtherCount = InvertAllGroupCount - InvertInGroupCount;

			asserta(InGroupSize >= InvertInGroupCount);
			uint InvertFNs = InGroupSize - InvertInGroupCount;
			uint InvertFPs = InvertOtherCount;
			uint InvertErrs = InvertFPs + InvertFNs;
			if (InvertFNs <= MinFNs && InvertErrs < MinErrs)
				{
				Invert = true;
				MinErrs = InvertErrs;
				BestFitNode = SubtreeNode;
				FPs = InvertFPs;
				FNs = InvertFNs;
				}
			}
		}
	//asserta(BestFitNode != UINT_MAX);
	return BestFitNode;
	}

void cmd_rootbyoutgroup()
	{
	const string &InputFileName = opt(rootbyoutgroup);
	const string &TsvFileName = opt(tsvout);
	FILE *fTsv = CreateStdioFile(TsvFileName);

	vector<TreeN *> Trees;
	TreesFromFile(InputFileName, Trees);
	const uint TreeCount = SIZE(Trees);

	const string NewRootLabel = optset_root_label ? opt(root_label) : "";
	const string OldRootLabel = optset_old_root_label ? opt(old_root_label) : "-";

	vector<string> GroupLabelsVec;
	set<string> GroupLabels;
	string GroupName;

	if (optset_labels)
		{
		StringsFromFile(opt(labels), GroupLabelsVec);
		for (uint i = 0; i < SIZE(GroupLabelsVec); ++i)
			GroupLabels.insert(GroupLabelsVec[i]);
		}
	else if (optset_outgroup)
		GroupName = opt(outgroup); // substring matching
	else
		Die("Must specify -labels or -outgroup");

	for (uint TreeIndex = 0; TreeIndex < TreeCount; ++TreeIndex)
		{
		TreeN &T = *Trees[TreeIndex];

		vector<uint> LeafNodes;
		T.GetLeafNodes(LeafNodes);
		const uint LeafCount = SIZE(LeafNodes);

		set<uint> GroupLeafNodes;
		if (optset_labels)
			T.GetGroupLeafNodes(GroupLabels, GroupLeafNodes);
		else if (optset_outgroup)
			T.GetGroupLeafNodes(GroupName, GroupLeafNodes);
		else
			asserta(false);

		if (GroupLeafNodes.empty())
			Die("Outgroup not found");

		uint FPs = UINT_MAX;
		uint FNs = UINT_MAX;
		bool Invert = false;
		bool FitOk = false;
		uint LCA = T.GetBestFitSubtree1(GroupLeafNodes, true, FPs, FNs, Invert);
		if (LCA == UINT_MAX)
			{
			Warning("No fit found to outgroup, using existing root");
			T.ForceBinaryRoot();
			}
		else
			{
			FitOk = true;
			const string &LCALabel = T.GetLabel(LCA);

			Log("@root ");
			ProgressLog("lca=%u lcalabel=%s FPs=%u FNs=%u invert=%c\n",
			  LCA, LCALabel.c_str(), FPs, FNs, tof(Invert));

			if (NewRootLabel == "=")
				{
				uint OldRoot = T.GetRoot();
				const string &OldLab = T.GetLabel(OldRoot);
				T.InsertRootAbove(LCA, OldRootLabel, OldLab);
				}
			else
				T.InsertRootAbove(LCA, OldRootLabel, NewRootLabel);
			}

		if (fTsv != 0)
			{
			string RootLabel;
			T.GetLabel(T.m_Root, RootLabel);
			if (RootLabel == "")
				RootLabel = ".";
			uint Size = SIZE(GroupLeafNodes);
			uint LeafCount = T.GetLeafCount();

			fprintf(fTsv, "file=%s", InputFileName.c_str());
			fprintf(fTsv, "\ttree=%u", TreeIndex);
			fprintf(fTsv, "\ttreesize=%u", LeafCount);
			fprintf(fTsv, "\trootlabel=%s", RootLabel.c_str());
			fprintf(fTsv, "\tgroupsize=%u", Size);
			fprintf(fTsv, "\tFPs=%u", FPs);
			fprintf(fTsv, "\tFNs=%u", FNs);
			fprintf(fTsv, "\tfitok=%c", tof(FitOk));
			fprintf(fTsv, "\n");
			}

		T.Ladderize(opt(right));
		}

	TreesToFile(Trees, opt(output));
	CloseStdioFile(fTsv);
	}
