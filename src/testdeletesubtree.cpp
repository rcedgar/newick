#include "myutils.h"
#include "treex.h"
#include "consmakerx.h"

#define TEST	1

#if TEST

void Shuffle(vector<unsigned> &v);
void GenerateRandomTree(const vector<string> &LeafLabels, bool Rooted, 
  double MinLength, double MaxLength, TreeX &T);

static uint g_OkCount = 0;

static void TestT1n(uint Node, const string &Str)
	{
	TreeX T;
	T.FromNewickStr(Str);
	asserta(T.IsRootedBinary());

	asserta(!T.IsLeaf(Node));
	asserta(Node != T.m_Origin);
	asserta(T.IsNode(Node));

	Log("\n");
	Log("%s\n", Str.c_str());
	Log("Delete subtree under %u\n", Node);
	T.LogMe();
	T.DeleteSubtree(Node, "DEL", false);
	Log("\nAfter delete:\n");
	T.LogMe();
	T.Validate();
	++g_OkCount;
	ProgressLog("Ok\n");
	}

static void TestT1(TreeX &T)
	{
	asserta(T.IsRootedBinary());

	string Str;
	T.ToNewickStr(Str, false);
	const uint NIC = T.GetNodeIndexCount();
	vector<uint> Nodes;
	for (uint i = 0; i < NIC; ++i)
		Nodes.push_back(i);
	Shuffle(Nodes);
	for (uint i = 0; i < NIC; ++i)
		{
		uint Node = Nodes[i];
		if (T.IsLeaf(Node))
			continue;
		if (Node == T.m_Origin)
			continue;
		TestT1n(Node, Str);
		}
	}

static void Test1(const string &Str)
	{
	TreeX T;
	T.FromNewickStr(Str);
	TestT1(T);
	}

static void TestHand()
	{
	Test1("((A,B),(C,D));");
	Test1("((A,B),(C,D));");
	Test1("((A,B),(C,D));");
	Test1("((X,Y),(C,D));");
	}

static void TestR(uint N)
	{
	vector<string> Labels;

	for (uint i = 0; i < N; ++i)
		{
		string Label;
		Ps(Label, "A%u", i);
		Labels.push_back(Label);
		}

	TreeX T;
	GenerateRandomTree(Labels, true, 1.0, 1.0, T);

	TestT1(T);
	}

static void TestRs()
	{
	ResetRand(1);
	const uint ITERS = 100;
	const uint K = 100;
	for (uint Iter = 0; Iter < ITERS; ++Iter)
		{
		ProgressStep(Iter, ITERS, "Testing");
		uint N = randu32()%K + K;
		TestR(N);
		}
	}

void cmd_testdeletesubtree()
	{
	opt(testdeletesubtree);
	//TestT1n(12,
//"(A0:1,(((A4:1,A8:1):1,(A2:1,A5:1):1):1,(A1:1,(A7:1,(A3:1,A6:1):1):1):1):1);");
//	TestHand();
	TestRs();
	return;
	}

#else // TEST
void cmd_testrandomssubtree() {}
#endif // TEST
