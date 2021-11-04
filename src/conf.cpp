#include "myutils.h"
#include "tree2.h"
#include "biparter.h"

void TreesFromFile(const string &FileName, vector<TreeN *> &Trees);

void cmd_conf()
	{
	const string &TreeFileName = opt(conf);
	const string &TreesFileName = opt(trees);
	const string &OutputFileName = opt(output);
	const string &TSVFileName = opt(tsvout);

	TreeN T;
	T.FromNewickFile(TreeFileName);

	vector<TreeN *> Trees;
	TreesFromFile(TreesFileName, Trees);
	if (opt(self))
		Trees.push_back(&T);
	const uint TreeCount = SIZE(Trees);

	FILE *fTSV = CreateStdioFile(TSVFileName);

	BiParter BP1;
	BP1.Init(T);

	const vector<vector<bool> > &Parts = BP1.m_PartVec;
	const uint PartCount = SIZE(Parts);
	vector<uint> ReplicateCounts(PartCount, 0);

	const uint N = SIZE(Trees);
	BiParter BP2;
	for (uint i = 0; i < N; ++i)
		{
		const TreeN &T = *Trees[i];
		BP2.Init(T);

		string Name;
		Ps(Name, "tree%u", i);
		BP2.ToTSV(Name, fTSV);

		for (uint j = 0; j < PartCount; ++j)
			{
			const vector<bool> &Part = Parts[j];
			uint Node = BP2.Search(Part);
			if (Node != UINT_MAX)
				++ReplicateCounts[j];
			}
		}

	for (uint j = 0; j < PartCount; ++j)
		{
		const vector<bool> &Part = Parts[j];
		if (T.IsRoot(j) || T.IsLeaf(j))
			{
			asserta(Part.empty());
			continue;
			}
		uint RC = ReplicateCounts[j];
		uint Pct = (RC*100)/TreeCount;
		string sPct;
		Ps(sPct, "%u", Pct);

		vector<uint> Nodes;
		BP1.GetPartInternalNodes(j, Nodes);
		for (uint k = 0; k < SIZE(Nodes); ++k)
			{
			uint Node = Nodes[k];
			T.UpdateLabel(Node, sPct);
			}
		}

////////////////////////////////////////////
// Do NOT unroot -- loses a bootstrap value!
////////////////////////////////////////////
	//if (!T.IsRooted())
	//	T1->Unroot();
	T.ToNewickFile(OutputFileName);

	CloseStdioFile(fTSV);
	}
