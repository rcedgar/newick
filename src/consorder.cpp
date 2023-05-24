#include "myutils.h"
#include "consorder.h"

const char *ConsOrder::GetName(uint i) const
	{
	asserta(i < SIZE(m_Names));
	return m_Names[i].c_str();
	}

void ConsOrder::AddTree(const TreeN &T)
	{
	set<string> &NewLabelSet = *new set<string>;
	T.GetLeafLabelSet(NewLabelSet, true, true);
	m_LabelSetVec.push_back(&NewLabelSet);

	string RootLabel;
	T.GetLabel(T.m_Root, RootLabel);
	m_Names.push_back(RootLabel);

	asserta(SIZE(m_Names) == SIZE(m_LabelSetVec));
	}

uint ConsOrder::GetFirstOnly(uint i, uint j) const
	{
	asserta(i < SIZE(m_LabelSetVec));
	asserta(j < SIZE(m_LabelSetVec));

	const set<string> &LabelSeti = *m_LabelSetVec[i];
	const set<string> &LabelSetj = *m_LabelSetVec[j];

	uint ni = 0;
	uint Nij = 0;
	for (set<string>::const_iterator p = LabelSeti.begin();
	  p != LabelSeti.end(); ++p)
		{
		const string &Label = *p;
		if (LabelSetj.find(Label) == LabelSetj.end())
			++ni;
		else
			++Nij;
		}

	const char *Namei = GetName(i);
	const char *Namej = GetName(j);
	Log("i=%s j=%s Ni=%u Nj=%u Nij=%u ni=%u\n",
	  Namei, Namej, SIZE(LabelSeti), SIZE(LabelSetj), Nij, ni);
	return ni;
	}

void ConsOrder::InitMx()
	{
	m_TreeCount = SIZE(m_LabelSetVec);
	m_Mx.resize(m_TreeCount);
	for (uint i = 0; i < m_TreeCount; ++i)
		m_Mx[i].resize(m_TreeCount, UINT_MAX);

	uint JoinCount = m_TreeCount - 1;
	asserta(SIZE(m_Names) == m_TreeCount);
	for (uint i = 0; i < JoinCount; ++i)
		{
		string Name;
		Ps(Name, "Join.%u", i);
		m_Names.push_back(Name);
		}

	for (uint i = 0; i < m_TreeCount; ++i)
		{
		for (uint j = 0; j < m_TreeCount; ++j)
			{
			uint n = GetFirstOnly(i, j);
			m_Mx[i][j] = n;
			}
		}
	}

void ConsOrder::InitPending()
	{
	asserta(m_TreeCount > 0 && m_Pending.empty());
	for (uint i = 0; i < m_TreeCount; ++i)
		m_Pending.push_back(i);
	}

void ConsOrder::DeleteFromPending(uint i, uint j)
	{
	vector<uint> NewPending;
	const uint N = SIZE(m_Pending);
	bool Foundi = false;
	bool Foundj = false;
	for (uint k = 0; k < N; ++k)
		{
		uint m = m_Pending[k];
		if (m == i)
			Foundi = true;
		else if (m == j)
			Foundj = true;
		else
			NewPending.push_back(m);
		}
	asserta(Foundi && Foundj);
	asserta(SIZE(NewPending) + 2 == SIZE(m_Pending));
	m_Pending = NewPending;
	}

void ConsOrder::GetBestJoin(uint &i, uint &j) const
	{
	i = UINT_MAX;
	j = UINT_MAX;
	const uint N = SIZE(m_Pending);
	uint Bestn = UINT_MAX;
	for (uint ki = 0; ki < N; ++ki)
		{
		uint ii = m_Pending[ki];
		for (uint kj = 0; kj < N; ++kj)
			{
			if (ki == kj)
				continue;
			uint jj = m_Pending[kj];
			uint n = GetFirstOnly(ii, jj);
			if (Bestn == UINT_MAX || n > Bestn)
				{
				i = ii;
				j = jj;
				Bestn = n;
				}
			}
		}
	asserta(Bestn != UINT_MAX);
	asserta(i != UINT_MAX);
	asserta(j != UINT_MAX);
	}

uint ConsOrder::Join(uint i, uint j)
	{
	uint NodeCount = SIZE(m_LabelSetVec);
	asserta(i < NodeCount);
	asserta(j < NodeCount);
	asserta(i != j);

	const set<string> &LabelSeti = *m_LabelSetVec[i];
	const set<string> &LabelSetj = *m_LabelSetVec[j];
	
	set<string> &NewLabelSet = *new set<string>;
	m_LabelSetVec.push_back(&NewLabelSet);

	for (set<string>::const_iterator p = LabelSeti.begin();
	  p != LabelSeti.end(); ++p)
		NewLabelSet.insert(*p);

	for (set<string>::const_iterator p = LabelSetj.begin();
	  p != LabelSetj.end(); ++p)
		NewLabelSet.insert(*p);

	return NodeCount;
	}

void cmd_consorder()
	{
	const string &InputFileName = opt(consorder);
	FILE *fOut = CreateStdioFile(opt(output));
	if (fOut != 0)
		setbuf(fOut, 0);

	vector<TreeN *> Trees;
	Progress("Start reading trees\n");
	TreesFromFile(InputFileName, Trees);
	const uint TreeCount = SIZE(Trees);
	asserta(TreeCount >= 2);
	Progress("Done reading trees\n");

	ConsOrder CO;

	for (uint i = 0; i < TreeCount; ++i)
		{
		ProgressStep(i, TreeCount, "Adding trees");
		TreeN &T = *Trees[i];
		CO.AddTree(T);
		}
	CO.InitMx();
	CO.InitPending();

	uint JoinCount = TreeCount - 1;
	for (uint JoinIndex = 0; JoinIndex < JoinCount; ++JoinIndex)
		{
		ProgressStep(JoinIndex, JoinCount, "Joining");
		uint i, j;
		CO.GetBestJoin(i, j);
		uint k = CO.Join(i, j);
		asserta(k < SIZE(CO.m_LabelSetVec));
		uint Size = SIZE(*CO.m_LabelSetVec[k]);
		CO.DeleteFromPending(i, j);
		CO.m_Pending.push_back(k);

		const char *Labeli = CO.GetName(i);
		const char *Labelj = CO.GetName(j);
		const char *Labelk = CO.GetName(TreeCount + JoinIndex);

		Log("join %s %s %s\n", Labeli, Labelj, Labelk);

		if (fOut != 0)
			{

			fprintf(fOut, "%u", JoinIndex);
			fprintf(fOut, "\t%u", i);
			fprintf(fOut, "\t%u", j);
			fprintf(fOut, "\t%u", k);
			fprintf(fOut, "\t%u", Size);
			fprintf(fOut, "\t%s", Labeli);
			fprintf(fOut, "\t%s", Labelj);
			fprintf(fOut, "\t%s", Labelk);
			fprintf(fOut, "\n");
			}
		}
	asserta(SIZE(CO.m_Pending) == 1);
	CloseStdioFile(fOut);
	}
