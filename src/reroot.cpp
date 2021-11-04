#include "myutils.h"
#include "tree2.h"

void Tree2::SetRoot(uint Nodei, uint Nodej)
	{
	asserta(!IsRooted());
	Validate();

	uint NodeCount = GetNodeCount();
	asserta(Nodei < NodeCount);
	asserta(Nodej < NodeCount);

	double Length = GetEdgeLength(Nodei, Nodej);

	pair<uint, uint> EdgePair;
	GetEdgePair(Nodei, Nodej, EdgePair);
	size_t n = m_EdgeToLength.erase(EdgePair);
	asserta(n == 1);

	m_Root = NodeCount;
	m_Nbrs1.push_back(UINT_MAX);
	m_Nbrs2.push_back(Nodei);
	m_Nbrs3.push_back(Nodej);
	m_Labels.push_back("ROOT");

	double L2 = (Length == MISSING_LENGTH ? MISSING_LENGTH : Length/2);
	SetLength(Nodei, m_Root, L2);
	SetLength(Nodej, m_Root, L2);

	uint Nodei1 = GetEdge1(Nodei);
	uint Nodei2 = GetEdge2(Nodei);
	uint Nodei3 = GetEdge3(Nodei);

	uint Nodej1 = GetEdge1(Nodej);
	uint Nodej2 = GetEdge2(Nodej);
	uint Nodej3 = GetEdge3(Nodej);

	SetEdge(Nodei, 1, m_Root);
	SetEdge(Nodej, 1, m_Root);

	if (Nodei1 == Nodej)
		{
		//m_Nbrs2[Nodei] = Nodei2;
		//m_Nbrs3[Nodei] = Nodei3;
		OrientNode(Nodei2, Nodei);
		OrientNode(Nodei3, Nodei);
		}
	else if (Nodei2 == Nodej)
		{
		m_Nbrs2[Nodei] = Nodei1;
		m_Nbrs3[Nodei] = Nodei3;
		OrientNode(Nodei1, Nodei);
		OrientNode(Nodei3, Nodei);
		}
	else if (Nodei3 == Nodej)
		{
		m_Nbrs2[Nodei] = Nodei1;
		m_Nbrs3[Nodei] = Nodei2;
		OrientNode(Nodei1, Nodei);
		OrientNode(Nodei2, Nodei);
		}
	else
		asserta(false);

	if (Nodej1 == Nodei)
		{
		//m_Nbrs2[Nodej] = Nodej2;
		//m_Nbrs3[Nodej] = Nodej3;
		OrientNode(Nodej2, Nodej);
		OrientNode(Nodej3, Nodej);
		}
	else if (Nodej2 == Nodei)
		{
		m_Nbrs2[Nodej] = Nodej1;
		m_Nbrs3[Nodej] = Nodej3;
		OrientNode(Nodej1, Nodej);
		OrientNode(Nodej3, Nodej);
		}
	else if (Nodej3 == Nodei)
		{
		m_Nbrs2[Nodej] = Nodej1;
		m_Nbrs3[Nodej] = Nodej2;
		OrientNode(Nodej1, Nodej);
		OrientNode(Nodej2, Nodej);
		}
	else
		asserta(false);

	Validate();
//	Log("Re-root (unrooted) validate ok\n");
	}

void Tree2::Unroot()
	{
	if (!IsRooted())
		return;

	const uint OldNodeCount = GetNodeCount();
	const uint OldRoot = m_Root;
	const uint OldRootLeft = GetLeft(OldRoot);
	const uint OldRootRight = GetRight(OldRoot);

	asserta(OldRootLeft != UINT_MAX);
	asserta(OldRootRight != UINT_MAX);
	asserta(OldRootLeft != OldRootRight);

	double LeftLength = GetEdgeLengthToLeftChild(OldRoot);
	double RightLength = GetEdgeLengthToRightChild(OldRoot);

	double RootEdgeLength = 0;
	if (LeftLength == MISSING_LENGTH && RightLength == MISSING_LENGTH)
		RootEdgeLength = MISSING_LENGTH;
	if (LeftLength != MISSING_LENGTH)
		RootEdgeLength += LeftLength;
	if (RightLength != MISSING_LENGTH)
		RootEdgeLength += RightLength;

	vector<uint> OldNodeToNewNode(OldNodeCount, UINT_MAX);
	vector<uint> NewNodeToOldNode(OldNodeCount-1, UINT_MAX);
	for (uint OldNode = 0; OldNode < OldNodeCount; ++OldNode)
		{
		uint NewNode = UINT_MAX;
		if (OldNode < OldRoot)
			NewNode = OldNode;
		else if (OldNode > OldRoot)
			NewNode = OldNode - 1;
		if (NewNode != UINT_MAX)
			NewNodeToOldNode[NewNode] = OldNode;
		OldNodeToNewNode[OldNode] = NewNode;
		}

	vector<uint> NewEdges1;
	vector<uint> NewEdges2;
	vector<uint> NewEdges3;
	vector<string> NewLabels;
	map<pair<uint, uint>, double> NewEdgeToLength;
	for (uint OldNode = 0; OldNode < OldNodeCount; ++OldNode)
		{
		if (OldNode == OldRoot)
			continue;
		uint NewNode = OldNodeToNewNode[OldNode];
		asserta(NewNode == SIZE(NewLabels));

		uint OldEdge1 = m_Nbrs1[OldNode];
		uint OldEdge2 = m_Nbrs2[OldNode];
		uint OldEdge3 = m_Nbrs3[OldNode];
		const string &Label = m_Labels[OldNode];
		double Length = GetEdgeLengthToParent(OldNode);

		uint NewEdge1 = (OldEdge1 == UINT_MAX ? UINT_MAX : OldNodeToNewNode[OldEdge1]);
		uint NewEdge2 = (OldEdge2 == UINT_MAX ? UINT_MAX : OldNodeToNewNode[OldEdge2]);
		uint NewEdge3 = (OldEdge3 == UINT_MAX ? UINT_MAX : OldNodeToNewNode[OldEdge3]);

		if (OldNode == OldRootLeft)
			{
			asserta(OldEdge1 == OldRoot);
			NewEdge1 = OldNodeToNewNode[OldRootRight];
			}
		else if (OldNode == OldRootRight)
			{
			asserta(OldEdge1 == OldRoot);
			NewEdge1 = OldNodeToNewNode[OldRootLeft];
			}

		NewEdges1.push_back(NewEdge1);
		NewEdges2.push_back(NewEdge2);
		NewEdges3.push_back(NewEdge3);
		NewLabels.push_back(Label);
		uint OldParent = GetParent(OldNode);
		if (OldParent != OldRoot)
			{
			uint NewParent = OldNodeToNewNode[OldParent];
			pair<uint, uint> EdgePair;
			GetEdgePair(NewNode, NewParent, EdgePair);
			NewEdgeToLength[EdgePair] = Length;
			}
		}

	asserta(OldRootLeft < SIZE(OldNodeToNewNode));
	asserta(OldRootRight < SIZE(OldNodeToNewNode));

	uint NewRootLeft = OldNodeToNewNode[OldRootLeft];
	uint NewRootRight = OldNodeToNewNode[OldRootRight];

	pair<uint, uint> EdgePair;
	GetEdgePair(NewRootLeft, NewRootRight, EdgePair);
	NewEdgeToLength[EdgePair] = RootEdgeLength;

	m_Root = UINT_MAX;
	m_Nbrs1 = NewEdges1;
	m_Nbrs2 = NewEdges2;
	m_Nbrs3 = NewEdges3;
	m_Labels = NewLabels;
	m_EdgeToLength = NewEdgeToLength;

	Validate();
	}
