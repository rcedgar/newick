#include "myutils.h"
#include "treex.h"
#include "divconker.h"
#include "sort.h"

// e.g. -ff _4					
//  => separator '_' field nr 4
static char g_FFSep = 0;		// separator
static uint g_FFFN = UINT_MAX;	// field nr

static void LocalGetValueFromLabel(const string &Label, string &Value)
	{
	FeatureTable::GetValueFromLabel(Label, g_FFSep, g_FFFN, Value);
	}

static void GetValueSetFromTrees(const vector<TreeX *> &Trees,
  set<string> &ValueSet)
	{
	ValueSet.clear();
	const uint TreeCount = SIZE(Trees);
	for (uint TreeIndex = 0; TreeIndex < TreeCount; ++TreeIndex)
		{
		const TreeX &T = *Trees[TreeIndex];
		const uint NodeCount = T.GetNodeIndexCount();
		for (uint Node = 0; Node < NodeCount; ++Node)
			{
			if (!T.IsLeaf(Node))
				continue;
			const string &Label = T.GetLabel(Node);
			string Value;
			LocalGetValueFromLabel(Label, Value);
			if (Value != "" && Value != ".")
				ValueSet.insert(Value);
			}
		}
	}

static void QC(FILE *f, const set<string> &ValueSet,
  const vector<vector<string> > &CCLabelVec)
	{
	if (f == 0)
		return;

	const uint CCCount = SIZE(CCLabelVec);
	vector<vector<string> > ValuesVec(CCCount);
	vector<string> Values;
	map<string, uint> ValueToIndex;
	map<string, uint> ValueToTotal;
	map<uint, uint> CCToValueSize;
	for (uint CCIndex = 0; CCIndex < CCCount; ++CCIndex)
		{
		uint ValueSize = 0;
		const vector<string> &Labels = CCLabelVec[CCIndex];
		const uint N = SIZE(Labels);
		for (uint i = 0; i < N; ++i)
			{
			const string &Label = CCLabelVec[CCIndex][i];
			string Value;
			LocalGetValueFromLabel(Label, Value);
			if (Value == "" || Value == ".")
				continue;
			if (ValueToIndex.find(Value) == ValueToIndex.end())
				{
				uint Index = SIZE(Values);
				Values.push_back(Value);
				ValueToIndex[Value] = Index;
				ValueToTotal[Value] = 1;
				}
			else
				ValueToTotal[Value] += 1;
			ValuesVec[CCIndex].push_back(Value);
			++ValueSize;
			}
		CCToValueSize[CCIndex] = ValueSize;
		}
	const uint ValueCount = SIZE(Values);

	map<string, map<uint, uint> > ValueToCCToCount;
	map<uint, string> CCToTopValue;
	uint TotalSize = 0;
	for (uint CCIndex = 0; CCIndex < CCCount; ++CCIndex)
		{
		uint CCValueSize = 0; // Number of labels with a value
		const vector<string> &Labels = CCLabelVec[CCIndex];
		const uint N = SIZE(Labels);
		map<string, uint> ValueToCount;
		for (uint i = 0; i < N; ++i)
			{
			const string &Label = CCLabelVec[CCIndex][i];
			string Value;
			LocalGetValueFromLabel(Label, Value);
			if (Value == "" || Value == ".")
				continue;
			++CCValueSize;
			if (ValueToCount.find(Value) == ValueToCount.end())
				ValueToCount[Value] = 1;
			else
				ValueToCount[Value] += 1;
			TotalSize += 1;
			}
		asserta(CCValueSize == CCToValueSize[CCIndex]);
		uint CCValueCount = SIZE(ValueToCount);
		fprintf(f, "cc_sz=%u", CCIndex);
		fprintf(f, "\tvalue_size=%u", CCValueSize);
		fprintf(f, "\tunique_values=%u", CCValueCount);
		fprintf(f, "\tall_size=%u", N);
		fprintf(f, "\n");
		if (ValueToCount.empty())
			continue;

		vector<string> CCValues;
		vector<uint> CCCounts;
		map<uint, uint> EmptyMap;
		CountMapToVecs(ValueToCount, CCValues, CCCounts);
		for (uint i = 0; i < SIZE(CCValues); ++i)
			{
			const string &Value = CCValues[i];
			uint n = CCCounts[i];
			ValueToCCToCount[Value][CCIndex] = n;
			}

		for (uint i = 0; i < SIZE(CCValues); ++i)
			{
			const string &Value = CCValues[i];
			uint Total = ValueToTotal[Value];
			uint n = CCCounts[i];
			double CCFract = double(n)/CCValueSize;
			double ValueFract = double(n)/Total;

			if (i == 0)
				CCToTopValue[CCIndex] = Value;

			fprintf(f, "cc_value=%u", CCIndex);
			fprintf(f, "\trank=%u", i+1);
			fprintf(f, "\tvalue=%s", Value.c_str());
			fprintf(f, "\tsize=%u", n);
			fprintf(f, "\tall_size=%u", N);
			fprintf(f, "\tccfract=%.4f", CCFract);
			fprintf(f, "\tvfract=%.4f", ValueFract);
			fprintf(f, "\n");
			}
		}

	map<string, uint> ValueToTopCC;
	map<string, uint> ValueToTopCCCount;
	for (map<string, map<uint, uint> >::const_iterator p = ValueToCCToCount.begin();
	  p != ValueToCCToCount.end(); ++p)
		{
		const string &Value = p->first;
		const map<uint, uint> &CCToCount = p->second;
		uint Total = ValueToTotal[Value];
		double Fract = double(Total)/TotalSize;

		fprintf(f, "value_sz=%s", Value.c_str());
		fprintf(f, "\tsize=%u", Total);
		fprintf(f, "\tpct=%.1f", 100.0*Fract);
		fprintf(f, "\n");

		vector<uint> CCIndexes;
		vector<uint> Counts;
		CountMapToVecs(CCToCount, CCIndexes, Counts);

		uint CCN = SIZE(CCIndexes);
		for (uint i = 0; i < CCN; ++i)
			{
			uint CCIndex = CCIndexes[i];
			uint n = Counts[i];
			double ValueFract = double(n)/Total;
			if (ValueFract >= 0.5)
				{
				ValueToTopCC[Value] = CCIndex;
				ValueToTopCCCount[Value] = n;
				}
			fprintf(f, "value_cc=%s", Value.c_str());
			fprintf(f, "\trank=%u", i+1);
			fprintf(f, "\tcc=%u", CCIndex);
			fprintf(f, "\tn=%u", n);
			fprintf(f, "\tvfract=%.4f", ValueFract);
			fprintf(f, "\n");
			}
		}

	double SumAcc = 0;
	uint AccCount = 0;
	for (uint ValueIndex = 0; ValueIndex < ValueCount; ++ValueIndex)
		{
		const string &Value = Values[ValueIndex];
		uint ValueSize = ValueToTotal[Value];
		fprintf(f, "value_to_cc=%s", Value.c_str());
		fprintf(f, "\tvsize=%u", ValueSize);
		if (ValueToTopCC.find(Value) == ValueToTopCC.end())
			{
			fprintf(f, "\tcc=none\n");
			continue;
			}
		uint CCIndex = ValueToTopCC[Value];
		if (CCToTopValue.find(CCIndex) == CCToTopValue.end())
			{
			fprintf(f, "\tcc=none\n");
			continue;
			}
		const string &TopValue = CCToTopValue[CCIndex];
		if (TopValue != Value)
			{
			fprintf(f, "\tcc=none\n");
			continue;
			}
		uint CCSize = CCToValueSize[CCIndex];
		uint n = ValueToTopCCCount[Value];
		uint TP = n;
		uint FN = ValueSize - n;
		uint FP = CCSize - n;
		double TPF = double(n)/CCSize;
		double FNF = double(FN)/ValueSize;
		double FPF = double(FP)/CCSize;
		double Acc = TPF - FPF;
		SumAcc += Acc;
		++AccCount;

		fprintf(f, "\tcc=%u", CCIndex);
		fprintf(f, "\tccsize=%u", CCSize);
		fprintf(f, "\tTP=%u", TP);
		fprintf(f, "\tFP=%u", FP);
		fprintf(f, "\tFN=%u", FN);
		fprintf(f, "\tTPF=%.4f", TPF);
		fprintf(f, "\tFPF=%.4f", FPF);
		fprintf(f, "\tFNF=%.4f", FNF);
		fprintf(f, "\tacc=%.4f", Acc);
		fprintf(f, "\n");
		}

	uint TotalValueCount = SIZE(ValueSet);
	double MeanAcc = SumAcc/AccCount;
	fprintf(f, "cc_count=%u", CCCount);
	fprintf(f, "\ttotal_value_count=%u", TotalValueCount);
	fprintf(f, "\tcc_value_count=%u", ValueCount);
	fprintf(f, "\tmean_acc=%.4f\n", MeanAcc);
	}

void cmd_olcs()
	{
	const string &InputFileName = opt(olcs);
	FILE *fTsv = CreateStdioFile(opt(tsvout));
	FILE *fFev = CreateStdioFile(opt(fevout));

	asserta(optset_ff);
	const string &FF = opt(ff);
	if (SIZE(FF) != 2 || !isdigit(FF[1]))
		Die("Invalid ff");

	g_FFSep = FF[0];
	char Digit = FF[1];
	if (Digit == '0')
		Die("Invalid ff (field must be >0)");
	g_FFFN = uint(Digit - '0') - 1;

	DivConker DK;
	DK.m_MaxMedianLeafDist = (optset_maxmedleafdist ? optset_maxmedleafdist : 1.6);

#define X(x, def, optname)	DK.m_##x = (optset_##optname ? opt(optname) : def);
	X(MinMedianLeafDist, 1.5, minmedleafdist);
	X(MaxMedianLeafDist, 2.3, maxmedleafdist);
	X(MinBootstrapFract, 0.85, minboot);
#undef X

	vector<TreeX *> Trees;
	TreesFromFile(InputFileName, Trees);
	const uint TreeCount = SIZE(Trees);
	asserta(TreeCount > 0);

	set<string> ValueSet;
	GetValueSetFromTrees(Trees, ValueSet);

	DK.Run(Trees);
	DK.CCsToTsv(fTsv);
	CloseStdioFile(fTsv);

	vector<vector<string> > CCLabelVec;
	DK.GetCCLabelVec(CCLabelVec);
	QC(fFev, ValueSet, CCLabelVec);
	}
