#include "myutils.h"
#include "treex.h"
#include "consmakerx.h"

#define TEST	0

#if TEST

void GenerateRandomTree(const vector<string> &LeafLabels, bool Rooted, 
  double MinLength, double MaxLength, TreeX &T);

static void TestT12(const TreeX &T1, const TreeX &T2)
	{
	const bool Verbose = false;
	if (Verbose)
		{
		Log("\n________ T1 _____________\n");
		T1.DrawText();

		Log("\n________ T2 _____________\n");
		T2.DrawText();
		}

	ConsMakerX CM;
	CM.MakeConsensus(T1, T2);

	if (Verbose)
		{
		Log("\n________ CT _____________\n");
		CM.m_ConsTree.DrawText();
		}
	}

static void Test12(const string &Str1, const string &Str2)
	{
	TreeX T1;
	TreeX T2;

	T1.FromNewickStr(Str1);
	T2.FromNewickStr(Str2);

	TestT12(T1, T2);
	}

static void Test1(const string &Str1, const string &Str2)
	{
	Test12(Str1, Str2);
	Test12(Str2, Str1);
	}

static void TestHand()
	{
	Test1("((A,B),(C,D));", "((A,B),(C,E));");
	Test1("((A,B),(C,D));", "((E,F),(G,H));");
	Test1("((A,B),(C,D));", "((E,F),(G,H));");
	Test1("((X,Y),(C,D));", "(X,Y);");
	}

static void TestR(uint N, uint M1, uint M2)
	{
	vector<string> Labels1;
	vector<string> Labels2;

	for (uint i = 0; i < N; ++i)
		{
		string Label;
		Ps(Label, "A%u", i);
		Labels1.push_back(Label);
		Labels2.push_back(Label);
		}

	for (uint i = 0; i < M1; ++i)
		{
		string Label;
		Ps(Label, "b%u", i);
		Labels1.push_back(Label);
		}

	for (uint i = 0; i < M2; ++i)
		{
		string Label;
		Ps(Label, "c%u", i);
		Labels2.push_back(Label);
		}

	TreeX T1;
	TreeX T2;
	GenerateRandomTree(Labels1, true, 1.0, 1.0, T1);
	GenerateRandomTree(Labels2, true, 2.0, 2.0, T2);

	TestT12(T1, T2);
	TestT12(T2, T1);
	}

static void TestRs()
	{
	ResetRand(1);
	const uint ITERS = 100;
	const uint K = 512;
	for (uint Iter = 0; Iter < ITERS; ++Iter)
		{
		ProgressStep(Iter, ITERS, "Testing");
		uint N = randu32()%K + K;
		uint M1 = randu32()%K;
		uint M2 = randu32()%K;
		TestR(N, M1, M2);

		N = 0;
		M1 = 3 + randu32()%K;
		M2 = 3 + randu32()%K;
		TestR(N, M1, M2);

		N = 3 + randu32()%K;
		M1 = randu32()%K;
		M2 = randu32()%K;
		TestR(N, M1, M2);
		}
	}

void cmd_consensus2x()
	{
	opt(consensus2x);
	TestRs();
	return;
	}

#else // TEST

void cmd_consensus2x()
	{
	const string &InputFileName = opt(consensus2x);
	const string &Input2FileName = opt(input2);
	const string &OutputFileName = opt(output);

	TreeX Tree1;
	TreeX Tree2;

	Tree1.FromNewickFile(InputFileName);
	Tree2.FromNewickFile(Input2FileName);

	ConsMakerX CM;
	CM.MakeConsensus(Tree1, Tree2);

	CM.m_ConsTree.ToNewickFile(OutputFileName);
	}

#endif // TEST
