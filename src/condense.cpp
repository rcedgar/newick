#include "myutils.h"
#include "featuretable.h"
#include "treen.h"

void SetFeatureTable(const TreeN &T, FeatureTable &FT);
void GetLCAs(const TreeN &T, const FeatureTable &FT, bool AllowInvert,
  vector<string> &Values, vector<uint> &LCAs, vector<uint> &GroupSizes,
  vector<uint> &SubtreeSizes, vector<uint> &FPs, vector<uint> &FNs,
  vector<bool> &Inverts, vector<double> &MonoFs);
void UpdateLCALabel(const string &OldLabel, const string &Value,
  bool IsLeaf,  string &NewLabel);

void cmd_condense()
	{
	vector<TreeN *> Trees;
	TreesFromFile(opt(condense), Trees);
	FILE *fTsv = CreateStdioFile(opt(tsvout));

	const uint TreeCount = SIZE(Trees);
	for (uint TreeIndex = 0; TreeIndex < TreeCount; ++TreeIndex)
		{
		TreeN &T = *Trees[TreeIndex];

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

		if (fTsv != 0)
			{
			const uint ValueCount = SIZE(Values);
			asserta(SIZE(LCAs) == ValueCount);
			asserta(SIZE(FPs) == ValueCount);
			asserta(SIZE(FNs) == ValueCount);
			for (uint ValueIndex = 0; ValueIndex < ValueCount; ++ValueIndex)
				{
				fprintf(fTsv, "tree=%u", TreeIndex);
				fprintf(fTsv, "\tvalue=%s", Values[ValueIndex].c_str());
				fprintf(fTsv, "\tsize=%u", GroupSizes[ValueIndex]);
				fprintf(fTsv, "\tFP=%u", FPs[ValueIndex]);
				fprintf(fTsv, "\tFN=%u", FNs[ValueIndex]);
				fprintf(fTsv, "\n");
				}
			}

		set<uint> LCASet;
		const uint FoundCount = SIZE(LCAs);
		Progress("Tree %u / %u, %u LCAs\n", TreeIndex+1, TreeCount, FoundCount);
		for (uint i = 0; i < FoundCount; ++i)
			{
			const string &Value = Values[i];
			uint LCA = LCAs[i];
			if (LCA == UINT_MAX)
				continue;
			asserta(T.IsNode(LCA));
			LCASet.insert(LCA);
			const string &Label = T.GetLabel(LCA);
			string NewLabel;
			bool IsLeaf = T.IsLeaf(LCA);
			UpdateLCALabel(Label, Value, IsLeaf, NewLabel);
			T.UpdateLabel(LCA, NewLabel);
			}

		T.Subset(LCASet);
		T.CollapseConfidenceUnary();
		T.CollapseUnary();
		T.Ladderize(opt(right));
		}

	TreesToFile(Trees, opt(output));

	CloseStdioFile(fTsv);
	}
