#include "myutils.h"
#include "tree2.h"
#include "treen.h"

double CalcDiameter(const Tree2 &T, string &Label1, string &Label2)
	{
	Tree2 C;
	C.FromTree(T);
	uint NodeCount = C.GetNodeCount();
	if (!C.IsRooted())
		{
		for (uint Node = 0; Node < NodeCount; ++Node)
			{
			if (!C.IsLeaf(Node))
				{
				uint Node2 = C.GetEdge1(Node);
				C.SetRoot(Node, Node2);
				break;
				}
			}
		}
	asserta(C.IsRooted());
	NodeCount = C.GetNodeCount();
	uint NegativeLengthCount = 0;
	for (map<pair<uint, uint>, double>::iterator p = C.m_EdgeToLength.begin();
	  p != C.m_EdgeToLength.end(); ++p)
		{
		double Length = p->second;
		if (Length != MISSING_LENGTH && Length < 0)
			{
			++NegativeLengthCount;
			p->second = 0;
			}
		}
	if (NegativeLengthCount > 0)
		Warning("%u negative edge lengths set to zero", NegativeLengthCount);

	vector<double> RootDists;
	C.GetRootDists(RootDists);
	asserta(SIZE(RootDists) == NodeCount);
	double MaxRootDist = 0;
	uint MaxNode = UINT_MAX;
	for (uint Node = 0; Node < NodeCount; ++Node)
		{
		double RootDist = RootDists[Node];
		if (RootDist > MaxRootDist)
			{
			MaxNode = Node;
			MaxRootDist = RootDist;
			}
		}
	if (MaxRootDist == 0)
		return 0;

	asserta(!C.IsRoot(MaxNode));
	asserta(C.IsLeaf(MaxNode));
	const string MaxLabel = C.GetLabel(MaxNode);
	asserta(MaxLabel != "");

	C.Validate();
	C.Unroot();

	uint UnrootedMaxNode = C.GetNodeByLabel(MaxLabel, true);
	uint UnrootedMaxEdge1 = C.GetEdge1(UnrootedMaxNode);
	uint UnrootedMaxEdge2 = C.GetEdge2(UnrootedMaxNode);
	uint UnrootedMaxEdge3 = C.GetEdge3(UnrootedMaxNode);

	asserta(UnrootedMaxEdge1 != UINT_MAX);
	asserta(UnrootedMaxEdge2 == UINT_MAX);
	asserta(UnrootedMaxEdge3 == UINT_MAX);

	C.SetRoot(UnrootedMaxNode, UnrootedMaxEdge1);

	const uint UnrootedNodeCount = C.GetNodeCount();
	C.GetRootDists(RootDists);
	asserta(SIZE(RootDists) == UnrootedNodeCount);

	double MaxRootDist2 = 0;
	uint MaxNode2 = UINT_MAX;
	for (uint Node = 0; Node < UnrootedNodeCount; ++Node)
		{
		double RootDist = RootDists[Node];
		if (RootDist > MaxRootDist)
			{
			MaxNode2 = Node;
			MaxRootDist2 = RootDist;
			}
		}
	if (MaxNode2 == UINT_MAX)
		return 0;
	asserta(MaxRootDist > 0);
	double Diameter = MaxRootDist2;
	Label1 = MaxLabel;
	Label2 = C.GetLabel(MaxNode2);
	return Diameter;
	}

double CalcDiameter(const TreeN &T, string &Label1, string &Label2)
	{
	Tree2 T2;
	T2.FromTreeN(T);
	double Diameter = CalcDiameter(T2, Label1, Label2);
	return Diameter;
	}

static void Test1(const string &NewickStr)
	{
	Tree2 T;
	T.FromStr(NewickStr);
	T.LogMe();

	string Label1;
	string Label2;
	double d = CalcDiameter(T, Label1, Label2);
	ProgressLog("\n");
	ProgressLog(" Tree  %s\n", NewickStr.c_str());
	ProgressLog(" Diam  %.3g\n", d);
	ProgressLog("Leaf1  %s\n", Label1.c_str());
	ProgressLog("Leaf2  %s\n", Label2.c_str());
	}

static void _cmd_test()
	{
	opt(test);
	Test1("((A:0.1,B:0.2):0.25,C:0.3);");
	Test1("((A:0.1,B:0.2):0.25,(C:0.3,D:0.4):0.5);");
	}
