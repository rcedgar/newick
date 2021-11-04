#include "myutils.h"
#include "featuretable.h"
#include "treen.h"

double GetConf(const string &Label, bool UsePcts)
	{
	double Conf = 0;
	if (Label.empty())
		return -1.0;

	if (UsePcts)
		{
		if (!IsValidIntStr(Label))
			Warning("Invalid integer '%s', set to zero",
				Label.c_str());
		else
			{
			uint Pct = StrToUint(Label);
			if (Pct > 100)
				{
				Warning("Invalid percent '%s', set to zero",
					Label.c_str());
				Pct = 0;
				}
			Conf = Pct/100.0;
			}
		}
	else
		{
		if (!IsValidFloatStr(Label))
			{
			Warning("Invalid float '%s', set to zero",
				Label.c_str());
			Conf = 0;
			}
		else
			{
			Conf = StrToFloat(Label);
			if (Conf < 0 || Conf > 1)
				{
				Warning("Invalid fraction '%s', set to zero",
					Label.c_str());
				Conf = 0;
				}
			}
		}
	return Conf;
	}

void GetFractConfs(const TreeN &T, vector<double> &Confs)
	{
	Confs.clear();

	vector<uint> Nodes;
	T.GetNodes(Nodes);
	const uint NodeCount = SIZE(Nodes);
	uint IntNodeCount = 0;
	uint PctCount = 0;
	uint FractCount = 0;
	uint MissingCount = 0;
	uint OtherCount = 0;
	for (uint k = 0; k < NodeCount; ++k)
		{
		uint Node = Nodes[k];
		if (T.IsRoot(Node) || T.IsLeaf(Node))
			continue;
		const string &Label = T.GetLabel(Node);
		++IntNodeCount;
		if (Label.empty())
			++MissingCount;
		else if (IsValidIntStr(Label))
			{
			uint Pct = StrToUint(Label);
			if (Pct == 0 || Pct == 1)
				{
				++PctCount;
				++FractCount;
				}
			else if (Pct > 1 && Pct <= 100)
				++PctCount;
			else
				++OtherCount;
			}
		else if (IsValidFloatStr(Label))
			{
			double Fract = StrToFloat(Label);
			if (Fract >= 0 && Fract <= 1)
				++FractCount;
			else
				++OtherCount;
			}
		else
			++OtherCount;
		}

	bool UsePcts = false;
	if (PctCount > IntNodeCount/2)
		UsePcts = true;
	else if (FractCount > IntNodeCount/2)
		UsePcts = false;
	else
		Die("Internal node labels not recognized pcts %u, fracts %u, other %u",
		  PctCount, FractCount, IntNodeCount - PctCount - FractCount);

	for (uint k = 0; k < NodeCount; ++k)
		{
		uint Node = Nodes[k];
		if (T.IsRoot(Node) || T.IsLeaf(Node))
			{
			Confs.push_back(-1);
			continue;
			}
		const string &Label = T.GetLabel(Node);
		double Conf = GetConf(Label, UsePcts);
		Confs.push_back(Conf);
		}
	}

void cmd_collapse()
	{
	TreeN T;
	T.FromNewickFile(opt(collapse));

	double MinConf = 0.70;
	if (optset_minconf)
		MinConf = opt(minconf);
	if (MinConf < 0 || MinConf > 1)
		Die("minconf must be in range 0 .. 1");

	vector<double> Confs;
	GetFractConfs(T, Confs);

	vector<uint> Nodes;
	T.GetNodes(Nodes);
	const uint NodeCount = SIZE(Nodes);
	asserta(SIZE(Confs) == NodeCount);
	uint CollapseCount = 0;
	for (uint k = 0; k < NodeCount; ++k)
		{
		uint Node = Nodes[k];
		if (T.IsRoot(Node) || T.IsLeaf(Node))
			continue;
		double Conf = Confs[Node];
		if (Conf < 0)
			continue;
		if (Conf < MinConf)
			{
			++CollapseCount;
			T.CollapseNode(Node);
			}
		}

	ProgressLog("%u / %u nodes collapsed\n",
	  CollapseCount, NodeCount);

	T.ToNewickFile(opt(output));
	}
