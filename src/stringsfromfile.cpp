#include "myutils.h"

void StringsFromFile(const string &FileName, vector<string> &Strings)
	{
	Strings.clear();
	FILE *f = OpenStdioFile(FileName);
	string Line;
	Progress("Reading %s...", FileName.c_str());
	while (ReadLineStdioFile(f, Line))
		Strings.push_back(Line);
	Progress("done.\n");
	CloseStdioFile(f);
	}
