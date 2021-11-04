#include "myutils.h"
#include "treen.h"

uint TreeN::GetNodeCountToFurthestLeaf(uint Node) const
	{
	if (IsLeaf(Node))
		return 1;
	uint Left = GetLeft(Node);
	uint Right = GetRight(Node);
	uint n = 1 + max(GetNodeCountToFurthestLeaf(Left),  
	  GetNodeCountToFurthestLeaf(Right));
	return n;
	}

static void SetL(TreeN &T, uint Node)
	{
	if (T.IsRoot(Node))
		return;

	if (opt(unitlengths))
		{
		T.m_NodeToLength[Node] = 1;
		return;
		}

	uint Parent = T.GetParent(Node);
	uint NodeN = T.GetNodeCountToFurthestLeaf(Node);
	uint ParentN = T.GetNodeCountToFurthestLeaf(Parent);
	asserta(NodeN < ParentN);
	uint d = ParentN - NodeN;
	T.m_NodeToLength[Node] = d;
	}

static void ConvertToCladogram(TreeN &T)
	{
	asserta(T.IsNormalized());
	T.m_NodeToLength.clear();
	const uint NodeCount = T.GetNodeCount();
	for (uint Node = 0; Node < NodeCount; ++Node)
		{
		if (T.IsRoot(Node))
			{
			T.m_NodeToLength[Node] = MISSING_LENGTH;
			continue;
			}

		SetL(T, Node);
		}
	}

void cmd_clado()
	{
	const string &InputFileName = opt(clado);
	const string &OutputFileName = opt(output);
	FILE *f = CreateStdioFile(OutputFileName);

	vector<TreeN *> Trees;
	TreesFromFile(InputFileName, Trees);
	uint TreeCount = SIZE(Trees);

	for (uint TreeIndex = 0; TreeIndex < TreeCount; ++TreeIndex)
		{
		ProgressStep(TreeIndex, TreeCount, "Processing");
		TreeN &T = *Trees[TreeIndex];
		ConvertToCladogram(T);
		T.ToNewickFile(f, false);
		}

	CloseStdioFile(f);
	}
