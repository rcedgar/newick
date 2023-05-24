#include "myutils.h"
#include "treen.h"
#include "consmaker.h"

void MakeOutputFileName(const string &Pattern, string &FN, uint Iter)
	{
	size_t L = Pattern.size();
	size_t n = Pattern.find('@');
	if (n == string::npos)
		Die("Must be @ in -pattern");
	string Prefix = Pattern.substr(0, n);
	string Suffix;
	if (n + 1 < L)
		Suffix = Pattern.substr(n+1, L-(n+1));
	string sn;
	Ps(sn, "%u", Iter);
	FN = Prefix + sn + Suffix;
	}

#define DOTEST	0

#if DOTEST
void GenerateRandomTree(const vector<string> &LeafLabels, bool Rooted, 
  double MinLength, double MaxLength, TreeN &T);

static uint g_TestIndex;

static void Test1(const TreeN &T1, const TreeN &T2)
	{
	string s1, s2;
	T1.ToNewickStr(s1, false);
	T2.ToNewickStr(s2, false);

	Log("\n");
	Log("________________________________________________\n");
	Log("Tree1      %s\n", s1.c_str());
	Log("Tree2      %s\n", s2.c_str());

	ConsMaker CM;
	CM.MakeConsensus(T1, T2);

	string s;
	CM.m_ConsTree.ToNewickStr(s, false);
	Log("Consensus  %s\n", s.c_str());

	if (opt(savetrees))
		{
		string FN1;
		string FN2;
		string FNC;

		Ps(FN1, "../test/fnx.%u.newick", g_TestIndex);
		Ps(FN2, "../test/fny.%u.newick", g_TestIndex);
		Ps(FNC, "../test/fnc.%u.newick", g_TestIndex);
		++g_TestIndex;

		T1.ToNewickFile(FN1);
		T2.ToNewickFile(FN2);
		CM.m_ConsTree.ToNewickFile(FNC);
		}
	}

static void Test2(const string &s1, const string &s2)
	{
	TreeN T1;
	TreeN T2;

	T1.FromNewickStr(s1);
	T2.FromNewickStr(s2);

	Test1(T1, T2);
	Test1(T2, T1);
	}

static void Tests(const string &s1, const string &s2)
	{
	TreeN T1;
	TreeN T2;

	T1.FromNewickStr(s1);
	T2.FromNewickStr(s2);

	Test1(T1, T2);
	}

static void TestR(uint N, uint M1, uint M2)
	{
	vector<string> Labels1;
	vector<string> Labels2;

	for (uint i = 0; i < N; ++i)
		{
		string Label;
		Label += ('A' + i);
		Labels1.push_back(Label);
		Labels2.push_back(Label);
		}

	for (uint i = 0; i < M1; ++i)
		{
		string Label;
		Label += ('m' + i);
		Labels1.push_back(Label);
		}

	for (uint i = 0; i < M2; ++i)
		{
		string Label;
		Label += ('m' + M1 + i);
		Labels2.push_back(Label);
		}

	TreeN T1;
	TreeN T2;
	GenerateRandomTree(Labels1, true, 1.0, 1.0, T1);
	GenerateRandomTree(Labels2, true, 2.0, 2.0, T2);

	Test1(T1, T2);
	Test1(T2, T1);
	}

static void Test()
	{
	opt_randseed = 1;
	optset_randseed = true;
	//Tests(
	//"(m:1,(n:1,o:1):1);",
	//"(r:2,(p:2,q:2):2);");
	//return;

	//Tests(	"(n:2,(C:2,((B:2,(A:2,(D:2,E:2):2):2):2,(F:2,m:2):2):2):2);",
	//		"((C:1,E:1):1,((A:1,D:1):1,(B:1,F:1):1):1);");
	//return;

	//Test2(	"((C:1,E:1):1,((A:1,D:1):1,(B:1,F:1):1):1);",
	//		"(n:2,(C:2,((B:2,(A:2,(D:2,E:2):2):2):2,(F:2,m:2):2):2):2);");
	//return;

	//Test2(	"(((B,C),A),(D,E));",
	//		"(((B,C),A),(D,(W,X)));");

	//Test2(	"(((B,C),A),(D,E));",
	//		"(((B,(W,X)),A),(D,E));");

	const uint ITERS = 100;
	for (uint Iter = 0; Iter < ITERS; ++Iter)
		{
		ProgressStep(Iter, ITERS, "Testing");
		uint N = randu32()%5 + 5;
		uint M1 = randu32()%4;
		uint M2 = randu32()%4;
		TestR(N, M1, M2);

		N = 0;
		M1 = 3 + randu32()%4;
		M2 = 3 + randu32()%4;
		TestR(N, M1, M2);

		N = 3 + randu32()%4;
		M1 = randu32()%4;
		M2 = randu32()%4;
		TestR(N, M1, M2);
		}
	}
#endif // DOTEST

void cmd_consensus2()
	{
#if DOTEST
	Test();
	return;
#endif

	const string &InputFileName = opt(consensus2);
	const string &Input2FileName = opt(input2);
	const string &OutputFileName = opt(output);

	TreeN Tree1;
	TreeN Tree2;

	Tree1.FromNewickFile(InputFileName);
	Tree2.FromNewickFile(Input2FileName);

	ConsMaker CM;
	CM.MakeConsensus(Tree1, Tree2);

	CM.m_ConsTree.ToNewickFile(OutputFileName);
	}

void cmd_consensus()
	{
#if DOTEST
	Test();
	return;
#endif

	const string &InputFileName = opt(consensus);
	const string &OutputFileName = opt(output);
	const string &Pattern = opt(pattern);

	vector<TreeN *> Trees;
	TreesFromFile(InputFileName, Trees);
	const uint TreeCount = SIZE(Trees);
	asserta(TreeCount >= 2);

	TreeN *Tree1 = Trees[0];
	for (uint Iter = 1; Iter < TreeCount; ++Iter)
		{
		Progress("\n=== Iter %u ===\n", Iter);

		TreeN *Tree2 = Trees[Iter];

		ConsMaker CM;
		CM.MakeConsensus(*Tree1, *Tree2);
		TreeN *ConsTree = new TreeN;
		CM.m_ConsTree.CopyNormalized(*ConsTree);

		if (optset_pattern)
			{
			string FN;
			MakeOutputFileName(Pattern, FN, Iter);
			ConsTree->ToNewickFile(FN);
			}

		Tree1->Clear();
		Tree2->Clear();

		Tree1 = ConsTree;
		}
	Tree1->ToNewickFile(OutputFileName);
	}
