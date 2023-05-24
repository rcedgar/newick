#include "myutils.h"
#include "treex.h"

static bool OnEdge(uint Node1, uint Node2, void *UserData)
	{
	Log("OnEdge(%u, %u)", Node1, Node2);
	const TreeX &TX = *(const TreeX *) UserData;
	if (TX.IsLeaf(Node1))
		Log(" 1>%s", TX.GetLabel(Node1).c_str());
	if (TX.IsLeaf(Node2))
		Log(" 2>%s", TX.GetLabel(Node2).c_str());
	Log("\n");
	return true;
	}

static bool OnNode(uint PrevNode, uint Node, void *UserData)
	{
	Log("OnNode(%u)", Node);
	if (PrevNode == UINT_MAX)
		Log(" prev=*");
	else
		Log( "prev=%u", PrevNode);
	const TreeX &TX = *(const TreeX *) UserData;
	if (TX.IsLeaf(Node))
		Log(" >%s", TX.GetLabel(Node).c_str());
	Log("\n");
	return true;
	}

static void TestLCA1(NewickParser2 &P, TreeX &TX, const string &LabelStr,
  const string &Str)
	{
	vector<string> Labels;
	Split(LabelStr, Labels, ',');

	Log("______________________________________________________________\n");
	ProgressLog("\n");
	ProgressLog("  %s\n", Str.c_str());
	P.FromStr(Str);
	//P.LogMe();

	TX.FromNewickStr(Str);
	TX.LogMe();
	TX.DrawText();

	set<uint> LeafNodeSet;
	for (uint i = 0; i < SIZE(Labels); ++i)
		{
		uint Node = TX.GetNodeByLabel(Labels[i], true);
		LeafNodeSet.insert(Node);
		}

	uint FromNode, ToNode;
	TX.GetLCAEdge(LeafNodeSet, FromNode, ToNode);
	Log("%s LCA %u -> %u\n", LabelStr.c_str(), FromNode, ToNode);
	}

static void TestRBO1(NewickParser2 &P, TreeX &TX, const string &LabelStr,
  const string &Str)
	{
	vector<string> Labels;
	Split(LabelStr, Labels, ',');

	Log("______________________________________________________________\n");
	ProgressLog("\n");
	ProgressLog("  %s\n", Str.c_str());
	P.FromStr(Str);
	//P.LogMe();

	TX.FromNewickStr(Str);
	if (TX.m_Rooted)
		TX.Unroot();
	Log("Before root by %s:\n", LabelStr.c_str());
	TX.LogMe();
	TX.DrawText(g_fLog);

	Log("\nAfter root by %s:\n", LabelStr.c_str());
	TX.SetRootByGroupLabels(Labels);
	TX.LogMe();
	}

static void TestBFS1(NewickParser2 &P, TreeX &TX, const string &LabelStrRoot,
  const string &LabelStrOG, const string &Str)
	{
	vector<string> Labels;
	Split(LabelStrRoot, Labels, ',');

	Log("______________________________________________________________\n");
	ProgressLog("\n");
	ProgressLog("  %s\n", Str.c_str());
	P.FromStr(Str);
	//P.LogMe();

	TX.FromNewickStr(Str);
	if (TX.m_Rooted)
		TX.Unroot();
	Log("Before root by %s:\n", LabelStrRoot.c_str());
	TX.LogMe();
	TX.DrawText(g_fLog);

	Log("\nAfter root by %s:\n", LabelStrRoot.c_str());
	TX.SetRootByGroupLabels(Labels);
	TX.LogMe();

	Split(LabelStrOG, Labels, ',');
	set<uint> NodeSet;
	for (uint i = 0; i < SIZE(Labels); ++i)
		{
		uint Node = TX.GetNodeByLabel(Labels[i]);
		NodeSet.insert(Node);
		}

	uint TP, FP, FN;
	Log("BPFs %s\n", LabelStrOG.c_str());
	TX.DrawText();
	TX.GetBestFitSubtree(NodeSet, 0.5, TP, FP, FN);
	}

static void TestTraverse1(NewickParser2 &P, TreeX &TX, const string &Label,
  const string &Str)
	{
	Log("______________________________________________________________\n");
	ProgressLog("\n");
	ProgressLog("  %s\n", Str.c_str());
	P.FromStr(Str);
	//P.LogMe();

	TX.FromNewickStr(Str);
	TX.LogMe();

	uint Node = TX.GetNodeByLabel(Label);
	uint Parent = TX.GetParent(Node);
	TX.Traverse_BreadthFirst(Node, &TX, OnNode);
	}

static void TestTraverseNode1(NewickParser2 &P, TreeX &TX, const string &Label,
  const string &Str)
	{
	TestTraverse1(P, TX, Label, Str);

	Log("______________________________________________________________\n");
	ProgressLog("\n");
	ProgressLog("  %s\n", Str.c_str());
	P.FromStr(Str);
	//P.LogMe();

	TX.FromNewickStr(Str);
	TX.LogMe();

	uint Node = TX.GetNodeByLabel(Label);
	TX.Traverse_BreadthFirst(Node, &TX, OnNode);
	}

static void Test1(NewickParser2 &P, TreeX &TX,
  const string &Str,
  const string &StrLabels,
  const string &StrLengths)
	{
	Log("______________________________________________________________\n");
	ProgressLog("\n");
	ProgressLog("  %s\n", Str.c_str());
	P.FromStr(Str);
	//P.LogMe();

	TX.FromNewickStr(Str);
	TX.LogMe();
	TX.DrawText(g_fLog);
	TX.Validate();

	string Str2;
	TX.ToNewickStr(Str2, false);
	Log("ToNewickStr = %s\n", Str2.c_str());

	TX.Normalize();
	}

static void TestTraverse()
	{
	NewickParser2 P;
	TreeX TX;
	TestTraverse1(P, TX, "Human",
"(Bovine:0.69395,(Gibbon:0.36079,(Orang:0.33636,(Gorilla:0.17147,(Chimp:0.19268, Human:0.11927):0.08386):0.06124):0.15057):0.54939,Mouse:1.21460):0.10;");

	TestTraverse1(P, TX, "dog",
"((raccoon:19.19959,bear:6.80041):0.84600,((sea_lion:11.99700, seal:12.00300):7.52973,((monkey:100.85930,cat:47.14069):20.59201, weasel:18.87953):2.09460):3.87382,dog:25.46154);");

	TestTraverse1(P, TX, "Gamma",
"(Alpha,Beta,Gamma,Delta,,Epsilon,,,);");

	TestTraverse1(P, TX, "Pongo",
"(Bovine:0.69395,(Hylobates:0.36079,(Pongo:0.33636,(G._Gorilla:0.17147, (P._paniscus:0.19268,H._sapiens:0.11927):0.08386):0.06124):0.15057):0.54939, Rodent:1.21460);");
	}

static void TestTraverseNode()
	{
	NewickParser2 P;
	TreeX TX;
	TestTraverseNode1(P, TX, "Human",
"(Bovine:0.69395,(Gibbon:0.36079,(Orang:0.33636,(Gorilla:0.17147,(Chimp:0.19268, Human:0.11927):0.08386):0.06124):0.15057):0.54939,Mouse:1.21460):0.10;");

	TestTraverseNode1(P, TX, "dog",
"((raccoon:19.19959,bear:6.80041):0.84600,((sea_lion:11.99700, seal:12.00300):7.52973,((monkey:100.85930,cat:47.14069):20.59201, weasel:18.87953):2.09460):3.87382,dog:25.46154);");

	TestTraverseNode1(P, TX, "Gamma",
"(Alpha,Beta,Gamma,Delta,,Epsilon,,,);");

	TestTraverseNode1(P, TX, "Pongo",
"(Bovine:0.69395,(Hylobates:0.36079,(Pongo:0.33636,(G._Gorilla:0.17147, (P._paniscus:0.19268,H._sapiens:0.11927):0.08386):0.06124):0.15057):0.54939, Rodent:1.21460);");
	}

static void TestLCA()
	{
	NewickParser2 P;
	TreeX TX;

	TestLCA1(P, TX, "Human,Chimp,Gorilla,Gibbon",
"(Bovine:0.69395,(Gibbon:0.36079,(Orang:0.33636,(Gorilla:0.17147,(Chimp:0.19268, Human:0.11927):0.08386):0.06124):0.15057):0.54939,Mouse:1.21460):0.10;");

	TestLCA1(P, TX, "Human,Orang",
"(Bovine:0.69395,(Gibbon:0.36079,(Orang:0.33636,(Gorilla:0.17147,(Chimp:0.19268, Human:0.11927):0.08386):0.06124):0.15057):0.54939,Mouse:1.21460):0.10;");

	TestLCA1(P, TX, "Paniscus,Sapiens",
"(Bovine:0.69395,(Hylobates:0.36079,(Pongo:0.33636,(G._Gorilla:0.17147, (Paniscus:0.19268,Sapiens:0.11927):0.08386):0.06124):0.15057):0.54939, Rodent:1.21460);");

	TestLCA1(P, TX, "Paniscus,Pongo",
"(Bovine:0.69395,(Hylobates:0.36079,(Pongo:0.33636,(G._Gorilla:0.17147, (Paniscus:0.19268,Sapiens:0.11927):0.08386):0.06124):0.15057):0.54939, Rodent:1.21460);");
	}

static void TestRBO()
	{
	NewickParser2 P;
	TreeX TX;

	TestRBO1(P, TX, "Human,Chimp",
"(Bovine:0.69395,(Gibbon:0.36079,(Orang:0.33636,(Gorilla:0.17147,(Chimp:0.19268, Human:0.11927):0.08386):0.06124):0.15057):0.54939,Mouse:1.21460):0.10;");

	TestRBO1(P, TX, "Human,Orang",
"(Bovine:0.69395,(Gibbon:0.36079,(Orang:0.33636,(Gorilla:0.17147,(Chimp:0.19268, Human:0.11927):0.08386):0.06124):0.15057):0.54939,Mouse:1.21460):0.10;");

	TestRBO1(P, TX, "Paniscus,Sapiens",
"(Bovine:0.69395,(Hylobates:0.36079,(Pongo:0.33636,(G._Gorilla:0.17147, (Paniscus:0.19268,Sapiens:0.11927):0.08386):0.06124):0.15057):0.54939, Rodent:1.21460);");
	
	TestRBO1(P, TX, "Paniscus,Pongo",
"(Bovine:0.69395,(Hylobates:0.36079,(Pongo:0.33636,(G._Gorilla:0.17147, (Paniscus:0.19268,Sapiens:0.11927):0.08386):0.06124):0.15057):0.54939, Rodent:1.21460);");
	}

static void TestBFS()
	{
	NewickParser2 P;
	TreeX TX;

	TestBFS1(P, TX, "Human,Chimp", "Bovine,Gibbon",
"(Bovine:0.69395,(Gibbon:0.36079,(Orang:0.33636,(Gorilla:0.17147,(Chimp:0.19268, Human:0.11927):0.08386):0.06124):0.15057):0.54939,Mouse:1.21460):0.10;");

//	TestBFS1(P, TX, "Human,Orang",
//"(Bovine:0.69395,(Gibbon:0.36079,(Orang:0.33636,(Gorilla:0.17147,(Chimp:0.19268, Human:0.11927):0.08386):0.06124):0.15057):0.54939,Mouse:1.21460):0.10;");
//
//	TestBFS1(P, TX, "Paniscus,Sapiens",
//"(Bovine:0.69395,(Hylobates:0.36079,(Pongo:0.33636,(G._Gorilla:0.17147, (Paniscus:0.19268,Sapiens:0.11927):0.08386):0.06124):0.15057):0.54939, Rodent:1.21460);");
//	
//	TestBFS1(P, TX, "Paniscus,Pongo",
//"(Bovine:0.69395,(Hylobates:0.36079,(Pongo:0.33636,(G._Gorilla:0.17147, (Paniscus:0.19268,Sapiens:0.11927):0.08386):0.06124):0.15057):0.54939, Rodent:1.21460);");
	}

void cmd_testx()
	{
	opt(testx);

	//TestTraverseNode();
	//TestRBO();
	//TestLCA();
	TestBFS();
	return;

	NewickParser2 P;
	TreeX TX;

// My examples
	Test1(P, TX, "(A, B, C);",					"A,B,C",		"");
	Test1(P, TX, "(A, (B, C));",				"A,B,C",		"");
	Test1(P, TX, "((A:2,B:3):4,C:5);",			"A,B,C",		"2,3,4,5");
	Test1(P, TX, "((A:1,B:2):5,(C:3,D:4));",	"A,B,C,D",		"1,2,5,3,4");
	Test1(P, TX, "((A:1,B:2):5,(C:3,D:4));",	"A,B,C,D",		"1,2,5,3,4");
	Test1(P, TX, "((A,B),(C,D));",				"A,B,C,D",		"");
	Test1(P, TX, "(A, B, C, D, E);",			"A,B,C,D,E",	"");
	Test1(P, TX, "((:1,:2):5,(:3,:4));",		"",				"1,2,5,3,4");
	Test1(P, TX, "(,(,));",						"",				"");
	Test1(P, TX, "((,),(,));",					"",				"");
	Test1(P, TX, "(,,);",						"",				"");
	Test1(P, TX, "(A:1,B:2);",		"A,B",				"1,2");
	Test1(P, TX, "(:1,B:2);",		"B",				"1,2");
	Test1(P, TX, "(:1,:2);",		"",				"1,2");
	Test1(P, TX, "(A:1,:2);",		"A",				"1,2");

// Wikipedia examples
	Test1(P, TX, "(,,(,));",					"",				"");
	Test1(P, TX, "(A,B,(C,D));",				"A,B,C,D",		"");
	Test1(P, TX, "(A,B,(C,D)E)F;",				"A,B,C,D,E,F",	"");
	Test1(P, TX, "(:0.1,:0.2,(:0.3,:0.4):0.5);",		"",		"0.1,0.2,0.3,0.4,0.5");
	Test1(P, TX, "(:0.1,:0.2,(:0.3,:0.4):0.5):0.0;",	"",		"0.1,0.2,0.3,0.4,0.5,0.0");
	Test1(P, TX, "(A:0.1,B:0.2,(C:0.3,D:0.4):0.5);",	"",		"0.1,0.2,0.3,0.4,0.5");
	Test1(P, TX, "(A:0.1,B:0.2,(C:0.3,D:0.4)E:0.5)F;",	"",		"0.1,0.2,0.3,0.4,0.5");
	Test1(P, TX, "((B:0.2,(C:0.3,D:0.4)E:0.5)F:0.1)A;",	"",		"0.1,0.2,0.3,0.4,0.5");

// Examples from https://evolution.genetics.washington.edu/phylip/newicktree.html (2021-03-28)
	Test1(P, TX, "((raccoon:19.19959,bear:6.80041):0.84600,((sea_lion:11.99700, seal:12.00300):7.52973,((monkey:100.85930,cat:47.14069):20.59201, weasel:18.87953):2.09460):3.87382,dog:25.46154);",
	  "raccoon,bear,sea_lion,seal,monkey,cat,weasel,dog",
	  "19.19959,6.80041,0.84600,11.99700,12.00300,7.52973,100.85930,47.14069,20.59201, 18.87953,2.09460,3.87382,25.46154");

	Test1(P, TX, "(Bovine:0.69395,(Gibbon:0.36079,(Orang:0.33636,(Gorilla:0.17147,(Chimp:0.19268, Human:0.11927):0.08386):0.06124):0.15057):0.54939,Mouse:1.21460):0.10;",
	  "Bovine,Gibbon,Orang,Gorilla,Chimp,Human,Mouse",
	  "0.69395,0.36079,0.33636,0.17147,0.19268,0.11927,0.08386,0.06124,0.15057,0.54939,1.21460,0.10");
 
	Test1(P, TX, "(Bovine:0.69395,(Hylobates:0.36079,(Pongo:0.33636,(G._Gorilla:0.17147, (P._paniscus:0.19268,H._sapiens:0.11927):0.08386):0.06124):0.15057):0.54939, Rodent:1.21460);",
	  "Bovine,Hylobates,Pongo,G._Gorilla,P._paniscus,H._sapiens,Rodent",
	  "0.69395,0.36079,0.33636,0.17147,0.19268,0.11927,0.08386,0.06124,0.15057,0.54939,1.21460");

	Test1(P, TX, "((A,B),(C,D));", "A,B,C,D", "");

	Test1(P, TX, "(Alpha,Beta,Gamma,Delta,,Epsilon,,,);", "Alpha,Beta,Gamma,Delta,Epsilon", "");
	}
