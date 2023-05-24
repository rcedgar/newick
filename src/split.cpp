#include "myutils.h"
#include "splitter.h"
#include "sort.h"

/***
Attempts to split tree into SplitCount subtrees of
approximately equal size. 
Greedily selects the largest remaining subtree.
***/

void Splitter::Run(const Tree2 &T, uint SplitCount)
	{
	m_T = &T;
	const uint NodeCount = m_T->GetNodeCount();
	const uint Root = m_T->GetRoot();
	m_TargetSize = NodeCount/SplitCount;
	m_SplitCount = SplitCount;
	asserta(m_TargetSize > 1);
	m_SubtreeNodes.push_back(m_T->m_Root);
	for (m_SplitIndex = 1; m_SplitIndex < m_SplitCount; ++m_SplitIndex)
		{
		asserta(SIZE(m_SubtreeNodes) == m_SplitIndex);
		uint BiggestNode = GetBiggestNode();
		uint Left = m_T->GetLeft(BiggestNode);
		uint Right = m_T->GetRight(BiggestNode);
		vector<uint> NewSubtreeNodes;
		for (uint i = 0; i < SIZE(m_SubtreeNodes); ++i)
			{
			uint Node = m_SubtreeNodes[i];
			if (Node == BiggestNode)
				{
				uint Left = m_T->GetLeft(BiggestNode);
				NewSubtreeNodes.push_back(Left);
				NewSubtreeNodes.push_back(Right);
				}
			else
				NewSubtreeNodes.push_back(Node);
			}
		m_SubtreeNodes = NewSubtreeNodes;
		LogState();
		}
	asserta(SIZE(m_SubtreeNodes) == m_SplitCount);
	}

void Splitter::GetSizeOrder(vector<uint> &Order) const
	{
	vector<uint> Sizes;
	for (uint i = 0; i < SIZE(m_SubtreeNodes); ++i)
		{
		uint Node = m_SubtreeNodes[i];
		uint Size = m_T->GetSubtreeLeafCount(Node);
		Sizes.push_back(Size);
		}
	uint N = SIZE(Sizes);
	Order.resize(N);
	QuickSortOrderDesc(Sizes.data(), N, Order.data());
	}

void Splitter::GetSubtree(Tree2 &Subtree) const
	{
	asserta(m_T != 0);
	const uint Size = SIZE(m_SubtreeNodes);
	const char *Prefix = "";
	if (optset_prefix)
		Prefix = opt(prefix).c_str();
	vector<string> SubsetLabels;
	for (uint i = 0; i < Size; ++i)
		{
		string Label;
		Ps(Label, "%s%u", Prefix, i+1);
		SubsetLabels.push_back(Label);
		}

	MakeSubsetNodes(*m_T, m_SubtreeNodes, SubsetLabels,
	  Subtree);
	}

void Splitter::LogState() const
	{
	Log("\n");
	Log("_______________ Split %u ______________\n", m_SplitIndex);
	Log(" Node   Size  LSize  RSize\n");
	//   12345  12345  12345  12345\n");
	vector<uint> Order;
	GetSizeOrder(Order);
	uint SumSize = 0;
	for (uint i = 0; i < SIZE(m_SubtreeNodes); ++i)
		{
		uint k = Order[i];
		uint Node = m_SubtreeNodes[k];
		uint Size = m_T->GetSubtreeLeafCount(Node);
		SumSize += Size;
		Log("%5u", Node);
		Log("  %5u", Size);
		if (!m_T->IsLeaf(Node))
			{
			uint Left = m_T->GetLeft(Node);
			uint Right = m_T->GetLeft(Node);
			uint LSize = m_T->GetSubtreeLeafCount(Left);
			uint RSize = m_T->GetSubtreeLeafCount(Right);
			Log("  %5u  %5u", LSize, RSize);
			}
		Log("\n");
		}
	Log("Total %u\n", SumSize);
	}

uint Splitter::GetBiggestNode() const
	{
	uint MaxSize = 0;
	uint MaxNode = UINT_MAX;
	for (uint i = 0; i < SIZE(m_SubtreeNodes); ++i)
		{
		uint Node = m_SubtreeNodes[i];
		uint Size = m_T->GetSubtreeLeafCount(Node);
		if (Size > MaxSize)
			{
			MaxNode = Node;
			MaxSize = Size;
			}
		}
	asserta(MaxNode != UINT_MAX);
	return MaxNode;
	}

void Splitter::WriteLabels(const string &FileNamePrefix) const
	{
	if (FileNamePrefix.empty())
		return;

	vector<uint> Order;
	GetSizeOrder(Order);

	string LabelsFileName;
	for (uint i = 0; i < SIZE(m_SubtreeNodes); ++i)
		{
		uint k = Order[i];
		uint Node = m_SubtreeNodes[k];
		vector<string> Labels;
		m_T->GetSubtreeLeafLabels(Node, Labels);
		LabelsFileName = opt(prefix);
		Psa(LabelsFileName, "%u", i+1);
		FILE *f = CreateStdioFile(LabelsFileName);
		for (uint j = 0; j < SIZE(Labels); ++j)
			fprintf(f, "%s\n", Labels[j].c_str());
		CloseStdioFile(f);
		}
	}

void cmd_split()
	{
	const string &TreeFileName = opt(split);

	uint n = 16;
	if (optset_n)
		n = opt(n);
	asserta(n > 1);

	Tree2 T;
	T.FromFile(TreeFileName);
	asserta(T.IsRooted());

	Splitter S;
	S.Run(T, n);
	S.WriteLabels(opt(prefix));

	if (optset_output)
		{
		Tree2 Subtree;
		S.GetSubtree(Subtree);
		Subtree.ToNewickFile(opt(output));
		}
	}
