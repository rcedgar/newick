#include "myutils.h"
#include "featuretable.h"
#include "treen.h"

void GetFractConfs(const TreeN &T, vector<double> &Confs);

double GetNodeToAnyConflict(FILE *fTsv, const TreeN &T,
  const vector<double> &FractConfs,
  const FeatureTable &FT, vector<bool> &NodeToAnyConflict);

void SetFeatureTable(const TreeN &T, FeatureTable &FT);

void cmd_taxq2()
	{
	const string &TreeFileName = opt(taxq2);
	const string &OutputFileName = opt(output);
	FILE *fTsv = CreateStdioFile(OutputFileName);

	TreeN T;
	T.FromNewickFile(TreeFileName);
	asserta(T.IsNormalized());
	const uint NodeCount = T.GetNodeCount();
	asserta(T.IsNormalized());

	FeatureTable FT;
	SetFeatureTable(T, FT);

	vector<double> FractConfs;
	GetFractConfs(T, FractConfs);
	asserta(SIZE(FractConfs) == NodeCount);

	vector<bool> NodeToAnyConflict;
	double Q = GetNodeToAnyConflict(fTsv, T, FractConfs, FT, NodeToAnyConflict);
	for (uint Node = 0; Node < NodeCount; ++Node)
		{
		if (T.IsLeaf(Node) || T.IsRoot(Node))
			continue;

		double FractConf = FractConfs[Node];
		bool AnyConflict = NodeToAnyConflict[Node];
		Pf(fTsv, "eval");
		Pf(fTsv, "\tnode=%u", Node);
		if (FractConf < 0)
			Pf(fTsv, "\tconf=.");
		else
			Pf(fTsv, "\tconf=%.3f", FractConf);
		Pf(fTsv, "\tconflict=%c", yon(AnyConflict));
		Pf(fTsv, "\n");
		}

	CloseStdioFile(fTsv);
	}
