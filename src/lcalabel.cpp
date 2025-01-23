#include "myutils.h"
#include "featuretable.h"
#include "treen.h"

void SetFeatureTable(const TreeN &T, FeatureTable &FT);
void GetLCAs(const TreeN &T, const FeatureTable &FT, bool AllowInvert,
  vector<string> &Values, vector<uint> &LCAs, vector<uint> &GroupSizes,
  vector<uint> &SubtreeSizes, vector<uint> &FPs, vector<uint> &FNs,
  vector<bool> &Inverts, vector<double> &MonoFs);

void UpdateLCALabel(const string &OldLabel, const string &Value,
  bool IsLeaf,  string &NewLabel)
	{
	if (!opt(lcaconfs))
		{
		NewLabel = Value;
		return;
		}

	if (EndsWith(OldLabel, "<"))
		{
		NewLabel = Value + "+" + OldLabel;
		return;
		}

	if (OldLabel == "")
		NewLabel = Value;
	else
		NewLabel = Value + "<" + OldLabel + ">";
	}

void cmd_lcalabel()
	{
	TreeN T;
	T.FromNewickFile(opt(lcalabel));

	FeatureTable FT;
	SetFeatureTable(T, FT);

	vector<string> Values;
	vector<uint> LCAs;
	vector<uint> GroupSizes;
	vector<uint> SubtreeSizes;
	vector<uint> FPs;
	vector<uint> FNs;
	vector<bool> Inverts;
	vector<double> MonoFs;
	GetLCAs(T, FT, false, Values, LCAs, GroupSizes, SubtreeSizes,
	  FPs, FNs, Inverts, MonoFs);

	const uint FoundCount = SIZE(LCAs);
	ProgressLog("%u LCAs found\n", FoundCount);
	for (uint i = 0; i < FoundCount; ++i)
		{
		const string &Value = Values[i];
		uint LCA = LCAs[i];
		if (LCA == UINT_MAX)
			continue;
		asserta(T.IsNode(LCA));
		const string &Label = T.GetLabel(LCA);
		string NewLabel;
		bool IsLeaf = T.IsLeaf(LCA);
		UpdateLCALabel(Label, Value, IsLeaf, NewLabel);
		T.UpdateLabel(LCA, NewLabel);
		}

	T.ToNewickFile(opt(output));
	T.Ladderize(opt(right));
	}
