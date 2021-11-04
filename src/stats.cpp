#include "myutils.h"
#include "quarts.h"
#include "treen.h"

double CalcDiameter(const TreeN &T, string &Label1, string &Label2);

static void Stats(FILE *f, const TreeN &T)
	{
	if (f == 0)
		return;

	uint N = T.GetNodeCount();
	bool Rooted;
	bool Binary = T.IsBinary(Rooted);
	uint L = T.GetLeafCount();

	uint MissingLengthCount = 0;
	uint InternalNodeLabelCount = 0;
	uint LeafLabelCount = 0;
	uint IntLabelCount = 0;
	uint FloatLabelCount = 0;
	uint LengthCount = 0;
	uint EdgeCount = 0;

	vector<uint> DegreeToCount;

	for (uint Node = 0; Node < N; ++Node)
		{
		uint Degree = T.GetChildCount(Node);
		if (Degree >= SIZE(DegreeToCount))
			DegreeToCount.resize(Degree+1, 0);
		++DegreeToCount[Degree];
		EdgeCount += Degree;
		double Length = T.GetLength(Node);
		if (Length != MISSING_LENGTH)
			++LengthCount;
		const string &Label = T.GetLabel(Node);
		if (!Label.empty())
			{
			if (T.IsLeaf(Node))
				++LeafLabelCount;
			else
				{
				++InternalNodeLabelCount;
				if (IsValidIntStr(Label))
					++IntLabelCount;
				else if (IsValidFloatStr(Label))
					++FloatLabelCount;
				}
			}
		}

	fprintf(f, "Binary            %s (%s)\n", 
	  Binary ? "Yes" : "No",
	  Rooted ? "rooted" : "unrooted");

	fprintf(f, "Leaves            %u\n", L);
	fprintf(f, "Nodes             %u\n", N);
	fprintf(f, "Lengths           %u\n", LengthCount);
	fprintf(f, "Leaf labels       %u\n", LeafLabelCount);
	fprintf(f, "Int. node labels  %u  (%u ints %u floats)\n",
	  InternalNodeLabelCount, IntLabelCount, FloatLabelCount);

	vector<double> RootDists;
	vector<double> LeafRootDists;
	T.GetRootDists(RootDists);
	T.GetLeafRootDists(LeafRootDists);

	QuartsDouble QD;
	GetQuarts(LeafRootDists, QD);
	fprintf(f, "Min height        %.3g\n", QD.Min);
	fprintf(f, "Avg height        %.3g\n", QD.Avg);
	fprintf(f, "Max height        %.3g\n", QD.Max);

	if (Binary)
		{
		string Label1, Label2;
		double Diameter = CalcDiameter(T, Label1, Label2);
		if (Label1 != "" && Label2 != "")
			{
			uint Node1 = T.GetNodeByLabel(Label1, true);
			uint Node2 = T.GetNodeByLabel(Label2, true);
			double RootDist1 = RootDists[Node1];
			double RootDist2 = RootDists[Node2];

			fprintf(f, "Diameter          %.3g  %s (%.3g) ---> %s (%.3g)\n",
			  Diameter, Label1.c_str(), RootDist1, Label2.c_str(), RootDist2);
			}
		}

	fprintf(f, "\n");
	fprintf(f, "Degree  Nodes\n");
	for (uint Degree = 0; Degree < SIZE(DegreeToCount); ++Degree)
		{
		uint n = DegreeToCount[Degree];
		if (n > 0)
			fprintf(f, "%6u  %5u\n", Degree, n);
		}
	}

void cmd_stats()
	{
	const string &FileName = opt(stats);
	vector<TreeN *> Trees;
	TreesFromFile(FileName, Trees);
	const uint TreeCount = SIZE(Trees);

	FILE *f = CreateStdioFile(opt(output));
	for (uint TreeIndex = 0; TreeIndex < TreeCount; ++TreeIndex)
		{
		const TreeN &T = *Trees[TreeIndex];
		Stats(stdout, T);
		Stats(f, T);
		}
	CloseStdioFile(f);
	}
