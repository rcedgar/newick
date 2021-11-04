#include "myutils.h"
#include "tree2.h"

void Tree2::ToTSV(const string &FileName) const
	{
	if (FileName.empty())
		return;

	FILE *f = CreateStdioFile(FileName);
	ToTSV(f);
	CloseStdioFile(f);
	}

void Tree2::ToTSV(FILE *f) const
	{
	vector<string> Lines;
	ToTSVStrings(Lines);
	for (uint i = 0; i < SIZE(Lines); ++i)
		fprintf(f, "%s\n", Lines[i].c_str());
	}

void Tree2::ToJust(FILE *f) const
	{
	vector<string> Lines;
	ToJustifiedStrings(Lines);
	for (uint i = 0; i < SIZE(Lines); ++i)
		fprintf(f, "%s\n", Lines[i].c_str());
	}

void Tree2::GetLabelToLeafNodeIndexMap(map<string, uint> &LabelToLeafNodeIndex) const
	{
	LabelToLeafNodeIndex.clear();
	const uint NodeCount = GetNodeCount();
	for (uint Node = 0; Node < NodeCount; ++Node)
		{
		if (IsLeaf(Node))
			{
			const string &Label = GetLabel(Node);
			if (Label.empty())
				Die("Tree2::GetLabelToLeafNodeIndexMap() empty leaf label");
			if (LabelToLeafNodeIndex.find(Label) != LabelToLeafNodeIndex.end())
				Die("Tree2::GetLabelToLeafNodeIndexMap() dupe label >%s",
				  Label.c_str());
			LabelToLeafNodeIndex[Label] = Node;
			}
		}
	}

static void PsaTabLength(string &s, double Length)
	{
	if (Length == MISSING_LENGTH)
		Psa(s, "\t*");
	else
		Psa(s, "\t%.3g", Length);
	}

void Tree2::ToTSVStrings(vector<string> &Lines) const
	{
	Lines.clear();

	const uint NodeCount = GetNodeCount();
	const uint EdgeCount = GetEdgeCount();
	const uint LeafCount = GetLeafCount();

	bool Rooted = IsRooted();
	if (Rooted)
		Lines.push_back("Node	Parent	Left	Right	Length	Type	Label");
	else
		Lines.push_back("Node	Nbr1	Nbr2	Nbr3	Length1	Length2	Length3	Type	Label");

	for (uint Node = 0; Node < NodeCount; ++Node)
		{
		string Line;
		uint Nbr1 = GetEdge1(Node);
		uint Nbr2 = GetEdge2(Node);
		uint Nbr3 = GetEdge3(Node);
		const string &Label = GetLabel(Node);

		Ps(Line, "%u", Node);
		if (Nbr1 == UINT_MAX) Psa(Line, "\t*"); else Psa(Line, "\t%u", Nbr1);
		if (Nbr2 == UINT_MAX) Psa(Line, "\t*"); else Psa(Line, "\t%u", Nbr2);
		if (Nbr3 == UINT_MAX) Psa(Line, "\t*"); else Psa(Line, "\t%u", Nbr3);
		if (Rooted)
			{
			if (IsRoot(Node))
				PsaTabLength(Line, MISSING_LENGTH);
			else
				{
				double Length = GetEdgeLengthToParent(Node);
				PsaTabLength(Line, Length);
				}
			}
		else
			{
			double Length1 = (Nbr1 == UINT_MAX ? MISSING_LENGTH : GetEdgeLength(Node, Nbr1));
			double Length2 = (Nbr2 == UINT_MAX ? MISSING_LENGTH : GetEdgeLength(Node, Nbr2));
			double Length3 = (Nbr3 == UINT_MAX ? MISSING_LENGTH : GetEdgeLength(Node, Nbr3));
			PsaTabLength(Line, Length1);
			PsaTabLength(Line, Length2);
			PsaTabLength(Line, Length3);
			}
		if (IsRoot(Node))
			Psa(Line, "\tRoot");
		else if (IsLeaf(Node))
			Psa(Line, "\tLeaf");
		else
			Psa(Line, "\tInt");
		Psa(Line, "\t%s", Label.c_str());

		Lines.push_back(Line);
		}
	}

static void PsaJustLength(string &s, double Length, uint w)
	{
	if (Length == MISSING_LENGTH)
		Psa(s, "  %*.*s", w, w, "*");
	else
		Psa(s, "  %*.3g", w, Length);
	}

void Tree2::ToJustifiedStrings(vector<string> &Lines) const
	{
	Lines.clear();

	const uint NodeCount = GetNodeCount();
	const uint EdgeCount = GetEdgeCount();
	const uint LeafCount = GetLeafCount();

	bool Rooted = IsRooted();
	if (Rooted)
		Lines.push_back("   Node   Parent     Left    Right   Length  Type  Label");
	else
		Lines.push_back("   Node     Nbr1     Nbr2     Nbr3  Length1  Length2  Length3  Type  Label");

	for (uint Node = 0; Node < NodeCount; ++Node)
		{
		string Line;
		uint Nbr1 = GetEdge1(Node);
		uint Nbr2 = GetEdge2(Node);
		uint Nbr3 = GetEdge3(Node);
		const string &Label = GetLabel(Node);

		Ps(Line, "%7u", Node);
		if (Nbr1 == UINT_MAX) Psa(Line, "        *"); else Psa(Line, "  %7u", Nbr1);
		if (Nbr2 == UINT_MAX) Psa(Line, "        *"); else Psa(Line, "  %7u", Nbr2);
		if (Nbr3 == UINT_MAX) Psa(Line, "        *"); else Psa(Line, "  %7u", Nbr3);
		if (Rooted)
			{
			double Length = GetEdgeLengthToParent(Node, false);
			PsaJustLength(Line, Length, 7);
			}
		else
			{
			double Length1 = (Nbr1 == UINT_MAX ? MISSING_LENGTH : GetEdgeLength(Node, Nbr1, false));
			double Length2 = (Nbr2 == UINT_MAX ? MISSING_LENGTH : GetEdgeLength(Node, Nbr2, false));
			double Length3 = (Nbr3 == UINT_MAX ? MISSING_LENGTH : GetEdgeLength(Node, Nbr3, false));
			PsaJustLength(Line, Length1, 7);
			PsaJustLength(Line, Length2, 7);
			PsaJustLength(Line, Length3, 7);
			}
		if (IsRoot(Node))
			Psa(Line, "  Root");
		else if (IsLeaf(Node))
			Psa(Line, "  Leaf");
		else
			Psa(Line, "   Int");
		Psa(Line, "  %s", Label.c_str());

		Lines.push_back(Line);
		}
	}

void Tree2::FromTSVFile(const string &FileName)
	{
	vector<string> Lines;
	FILE *f = OpenStdioFile(FileName);
	string Line;
	while (ReadLineStdioFile(f, Line))
		Lines.push_back(Line);
	CloseStdioFile(f);
	FromTSVStrings(Lines);
	}

void Tree2::FromTSVStrings(const vector<string> &Lines)
	{
	asserta(!Lines.empty());
	Clear();
	asserta(StartsWith(Lines[0], "Node"));
	const uint NodeCount = SIZE(Lines)-1;
	m_Nbrs1.resize(NodeCount, UINT_MAX);
	m_Nbrs2.resize(NodeCount, UINT_MAX);
	m_Nbrs3.resize(NodeCount, UINT_MAX);
	m_EdgeToLength.clear();
	m_Labels.resize(NodeCount, "");

//    0     1       2       3        4     5        6
// Node	Nbr1	Nbr2	Nbr3	Length	Type	Label
	for (uint Node = 0; Node + 1 < SIZE(Lines); ++Node)
		{
		vector<string> Fields;
		Split(Lines[Node+1], Fields, '\t');
		if (SIZE(Fields) == 6)
			Fields.push_back("");
		asserta(SIZE(Fields) == 7);

		uint Node2 = StrToUint(Fields[0], true);
		asserta(Node2 == Node);
		uint Nbr1 = StrToUint(Fields[1], true);
		uint Nbr2 = StrToUint(Fields[2], true);
		uint Nbr3 = StrToUint(Fields[3], true);
		double Length = StrToFloat(Fields[4], true);
		const string &Type = Fields[5];
		asserta(Type == "Leaf" || Type == "Root" || Type == "Int");
		const string &Label = Fields[6];

		asserta(Node < NodeCount);
		m_Nbrs1[Node] = Nbr1;
		m_Nbrs2[Node] = Nbr2;
		m_Nbrs3[Node] = Nbr3;
		//m_Lengths[Node] = Length;
		asserta(false);//TODO
		m_Labels[Node] = Label;

		if (Type == "Root")
			m_Root = Node;
		}
	Validate();
	}

void cmd_newick2tsv2()
	{
	Tree2 T;
	T.FromNewickFile(opt(newick2tsv));
	T.ToTSV(opt(output));
	}

void cmd_tsv2newick()
	{
	Tree2 T;
	T.FromTSVFile(opt(tsv2newick));
	T.ToNewickFile(opt(output));
	}
