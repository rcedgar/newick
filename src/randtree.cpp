#include "myutils.h"
#include "tree2.h"
#include "treen.h"
#include "treex.h"
#include <set>
#include <list>

void StringsFromFile(const string &FileName, vector<string> &Strings);

/***
Fisher-Yates shuffle:
To shuffle an array a of n elements (indices 0 .. n-1):
for i from n - 1 downto 1 do
    j := random integer with 0 <= j <= i
    exchange a[j] and a[i]
***/
void Shuffle(vector<unsigned> &v)
	{
	const unsigned N = SIZE(v);
	for (unsigned i = N - 1; i >= 1; --i)
		{
		unsigned j = randu32()%(i + 1);
		
		unsigned vi = v[i];
		unsigned vj = v[j];

		v[i] = vj;
		v[j] = vi;
		}
	}

/***
Unrooted:
	N is even
	E = N - 1
	L = N/2 + 1
	N = 2*(L - 1)
***/

static double GetRandomLength(double MinLength, double MaxLength)
	{
	asserta(MinLength <= MaxLength);
	uint r = randu32()%100;
	double d = MinLength + ((MaxLength - MinLength)*r)/100.0;
	asserta(d <= MaxLength);
	return d;
	}

static double GetLength(double Min, double Max)
	{
	const uint M = 1000000;
	uint r = randu32()%M;
	double f = double(r)/M;
	double Length = Min + f*f*(Max - Min);
	return Length;
	}

static uint GetRandomPending(vector<uint> &Pending)
	{
	uint N = SIZE(Pending);
	asserta(N > 0);
	uint r = randu32()%N;
	uint NextNode = Pending[r];
	vector<uint> NewPending;
	for (uint i = 0; i < N; ++i)
		{
		uint PendingNode = Pending[i];
		if (PendingNode != NextNode)
			NewPending.push_back(PendingNode);
		}
	Pending = NewPending;
	return NextNode;
	}

void GenerateRandomTree(const vector<string> &LeafLabels, bool Rooted, 
  double MinLength, double MaxLength, Tree2 &T)
	{
	const uint LeafCount = SIZE(LeafLabels);

	asserta(LeafCount > 1);
	asserta(MinLength <= MaxLength);
	T.Clear();

	uint NodeCount = 2*LeafCount - 1;
	vector<double> Lengths;
	for (uint i = 0; i < NodeCount; ++i)
		Lengths.push_back(GetLength(MinLength, MaxLength));

	vector<uint> Parents(NodeCount, UINT_MAX);

	vector<string> Labels;
	vector<uint> Pending;
	for (uint i = 0; i < LeafCount; ++i)
		{
		Pending.push_back(i);
		Labels.push_back(LeafLabels[i]);
		}

	uint InternalNodeCount = LeafCount - 1;
	for (uint i = 0; i < InternalNodeCount; ++i)
		{
		uint Parent = LeafCount + i;
		uint Left = GetRandomPending(Pending);
		uint Right = GetRandomPending(Pending);

		double Length = GetLength(MinLength, MaxLength);
		Labels.push_back("");

		asserta(Parents[Left] == UINT_MAX);
		asserta(Parents[Right] == UINT_MAX);

		Parents[Left] = Parent;
		Parents[Right] = Parent;

		Pending.push_back(Parent);
		}

	T.FromVectors(Labels, Parents, Lengths);
	if (!Rooted)
		{
//		T.LogMe();
		T.Unroot();
		}
//	T.LogMe();
	T.Validate();
	}

void GenerateRandomTree(const vector<string> &LeafLabels, bool Rooted, 
  double MinLength, double MaxLength, TreeN &T)
	{
	Tree2 T2;
	GenerateRandomTree(LeafLabels, Rooted, MinLength, MaxLength, T2);
	T.FromTree2(T2);
	}

void GenerateRandomTree(const vector<string> &LeafLabels, bool Rooted, 
  double MinLength, double MaxLength, TreeX &T)
	{
	TreeN TN;
	GenerateRandomTree(LeafLabels, Rooted, MinLength, MaxLength, TN);
	string Str;
	TN.ToNewickStr(Str, true);
	T.FromNewickStr(Str);
	}

void GenerateRandomTree(uint LeafCount, bool Rooted, 
  double MinLength, double MaxLength, Tree2 &T)
	{
	asserta(LeafCount > 1);
	asserta(MinLength <= MaxLength);
	T.Clear();

	vector<string> Labels;
	for (uint i = 0; i < LeafCount; ++i)
		{
		string Label;
		Ps(Label, "Leaf%u", i+1);
		Labels.push_back(Label);
		}

	GenerateRandomTree(Labels, Rooted, MinLength, MaxLength, T);
	}

static void _cmd_test()
	{
	opt(test);
	ResetRand(1);
	Tree2 T;
	bool Rooted = true;
	for (uint i = 0; i < 100; ++i)
		{
		ProgressStep(i, 100, "Rand trees");
		uint LeafCount = 3 + randu32()%10;
		Rooted = !Rooted;
		GenerateRandomTree(LeafCount, Rooted, 0.1, 0.2, T);
		T.LogMe();
		}
	}

void cmd_randtree()
	{
	const string &LabelsFileName = opt(randtree);
	double MinLength = 1.0;
	double MaxLength = 1.0;
	if (optset_minlength)
		MinLength = opt(minlength);
	if (optset_maxlength)
		MaxLength = opt(maxlength);
	asserta(MinLength <= MaxLength);
	const bool IsRooted = opt(rooted);

	vector<string> Labels;
	StringsFromFile(LabelsFileName, Labels);

	Tree2 T;
	GenerateRandomTree(Labels, IsRooted, MinLength, MaxLength, T);
	T.ToNewickFile(opt(output));
	}
