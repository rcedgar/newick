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

void cmd_getlcasubtrees()
	{
	vector<TreeN *> Trees;
	TreesFromFile(opt(getlcasubtrees), Trees);
	FILE *fFev = CreateStdioFile(opt(fevout));

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

		if (fFev != 0)
			{
			const uint ValueCount = SIZE(Values);
			asserta(SIZE(LCAs) == ValueCount);
			asserta(SIZE(FPs) == ValueCount);
			asserta(SIZE(FNs) == ValueCount);
			for (uint ValueIndex = 0; ValueIndex < ValueCount; ++ValueIndex)
				{
				const string &Value = Values[ValueIndex];
				uint LCA = LCAs[ValueIndex];
				if (LCA == UINT_MAX)
					{
					fprintf(fFev, "feature=%s\tsubtree_size=0\n", Value.c_str());
					continue;
					}
				uint SubtreeSize = SubtreeSizes[ValueIndex];
				uint N = GroupSizes[ValueIndex];
				uint FP = FPs[ValueIndex];
				uint FN = FNs[ValueIndex];
				fprintf(fFev, "feature=%s\tsubtree_size=%u\tN=%u\tFP=%u\tFN=%u\n",
						Value.c_str(), SubtreeSize, N, FP, FN);
				vector<string> Labels;
				T.GetSubtreeSortedLeafLabels(LCA, Labels);
				for (uint i = 0; i < SIZE(Labels); ++i)
					{
					const string &Label = Labels[i];
					string LabelValue;
					FT.GetValue_ByLabel(Label, LabelValue, false);
					fprintf(fFev, "feature_label=%s\tleaf=%u\tlabel=%s\tvalue=%s",
							Value.c_str(), i, Labels[i].c_str(), LabelValue.c_str());
					if (LabelValue == Value)
						fprintf(fFev, "\tcorrect=TP");
					else
						fprintf(fFev, "\tcorrect=FP");
					fprintf(fFev, "\n");
					}
				}
			}
		}

	CloseStdioFile(fFev);
	}
