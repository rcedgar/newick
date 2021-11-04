#include "myutils.h"
#include "featuretable.h"

// superkingdom:Bacteria,clade:Terrabacteria group,phylum:Actinobacteria,class:Actin...
static void GetTaxName(const string &TaxStr, const string &RankName,
  string &Name)
	{
	Name.clear();
	const string RankNameColon = RankName + ":";
	size_t n = TaxStr.find(RankNameColon);
	if (n == string::npos)
		return;
	for (size_t i = n + RankNameColon.size(); i < TaxStr.size(); ++i)
		{
		char c = TaxStr[i];
		if (c == ',')
			return;
		Name += c;
		}
	}

void ReadTaxTable(const string &FileName, const vector<string> &Ranks,
  vector<FeatureTable *> &FTs)
	{
	FTs.clear();
	const uint RankCount = SIZE(Ranks);

	vector<string> Accs;
	vector<vector<string> > NamesVec;

	string Line;
	vector<string> Fields;
	FILE *f = OpenStdioFile(FileName);
	while (ReadLineStdioFile(f, Line))
		{
		Split(Line, Fields, '\t');
		asserta(SIZE(Fields) == 2);
		const string &Label = Fields[0];
		const string &TaxStr = Fields[1];

		string Acc;
		GetAccFromLabel(Label, Acc);

		vector<string> Names;
		for (uint i = 0; i < RankCount; ++i)
			{
			const string &RankName = Ranks[i];
			string Name;
			GetTaxName(TaxStr, RankName, Name);
			Names.push_back(Name);
			}
		Accs.push_back(Acc);
		NamesVec.push_back(Names);
		}
	const uint AccCount = SIZE(Accs);

	for (uint i = 0; i < RankCount; ++i)
		{
		vector<string> Values;
		for (uint j = 0; j < AccCount; ++j)
			{
			const string &Value = NamesVec[j][i];
			Values.push_back(Value);
			}
		FeatureTable *FT = new FeatureTable;
		FT->FromVecs(Accs, Values);
		FTs.push_back(FT);
		}
	}
