#include "myutils.h"
#include "taxer.h"

void cmd_tax()
	{
	const string &InputFileName = opt(tax);
	FILE *fTsv = CreateStdioFile(opt(tsvout));
	FILE *fFev = CreateStdioFile(opt(fevout));
	FILE *fOut = CreateStdioFile(opt(output));

	uint K = 0;
	if (optset_kfold)
		K = opt(kfold);
	double MinTPFract = 0.5;
	if (optset_mintpfract)
		MinTPFract = opt(mintpfract);

	vector<TreeX *> Trees;
	TreesFromFile(InputFileName, Trees);
	const uint TreeCount = SIZE(Trees);

	Taxer TheTaxer;

	for (uint TreeIndex = 0; TreeIndex < TreeCount; ++TreeIndex)
		{
		TreeX &T = *Trees[TreeIndex];
		TheTaxer.Init(TreeIndex, T, K, MinTPFract);
		TheTaxer.ToFev(fFev);
		TheTaxer.ToTsv(fTsv);
		TheTaxer.ToNewick(fOut);
		}
	CloseStdioFile(fTsv);
	CloseStdioFile(fFev);
	CloseStdioFile(fOut);
	}
