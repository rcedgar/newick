#include "myutils.h"
#include "svg.h"
#include "treen.h"

void TreesFromFile(const string &FileName, vector<TreeN *> &Trees);
void GetValuesFromTrees(vector<TreeN *> &Trees, char Sep, uint FieldIndex,
  const string &MissingValue, vector<string> &Values);

// https://martin.ankerl.com/2009/12/09/how-to-create-random-colors-programmatically/

void ColorsFromFile(const string &FileName,
  vector<string> &Values, vector<string> &Colors)
	{
	Values.clear();
	Colors.clear();
	FILE *f = OpenStdioFile(FileName);
	string Line;
	vector<string> Fields;
	while (ReadLineStdioFile(f, Line))
		{
		Split(Line, Fields, '\t');
		asserta(SIZE(Fields) == 2);
		const string &Value = Fields[0];
		const string &Color = Fields[1];
		Values.push_back(Value);
		Colors.push_back(Color);
		}
	CloseStdioFile(f);
	}

static uint hsv_to_rgb(double h, double s, double v)
	{
	int h_i = int(h*6);
	double f = h*6 - h_i;
	double p = v * (1 - s);
	double q = v * (1 - f*s);
	double t = v * (1 - (1 - f) * s);

	double r, g, b;
#define Assign(x, y, z)	{ r = x; b = y; g = z; }
	if (h_i == 0)
		Assign(v, t, p)
	else if (h_i == 1)
		Assign(q, v, p)
	else if (h_i == 2)
		Assign(p, v, t)
	else if (h_i == 3)
		Assign(p, q, v)
	else if (h_i == 4)
		Assign(t, p, v)
	else if (h_i == 5)
		Assign(v, p, q)
	else
		asserta(false);
#undef Assign

	uint R = uint(r*256);
	uint G = uint(g*256);
	uint B = uint(b*256);
	asserta(R >= 0 && R < 256);
	asserta(G >= 0 && G < 256);
	asserta(B >= 0 && B < 256);

	uint RGB = (R << 16) + (G << 8) + B;
	return RGB;
	}

static double g_h = 3.1415;
uint GetRandomColor()
	{
	g_h += 0.618033988749895;
	g_h = fmod(g_h, 1.0);
	uint RGB = hsv_to_rgb(g_h, 0.5, 0.95);
	return RGB;
	}

void GetRandomColors(uint N, vector<string> &Colors)
	{
	Colors.clear();
	for (uint i = 0; i < N; ++i)
		{
		uint RGB = GetRandomColor();
		string Color;
		Ps(Color, "#%06x", RGB);
		Colors.push_back(Color);
		}
	}

void InitRandomColor(uint Seed)
	{
	g_h = double(Seed)*1023.456;
	g_h = fmod(g_h, 1.0);
	}

void cmd_palette()
	{
	const string &OutputFileName = opt(palette);

	uint Seed = 1;
	if (optset_randseed)
		Seed = opt(randseed);
	
	const uint MAXN = 32;

	const double SZ = 10;
	const double MARGIN = 3;

	double FigWidth = (SZ + MARGIN)*MAXN + 3*MARGIN + 20;
	double FigHeight = (SZ + 2*MARGIN)*MAXN + 3*MARGIN + 20;

	Svg S;
	S.Open(OutputFileName, FigWidth, FigHeight);

	double Y = 3*MARGIN;
	for (uint N = 2; N <= MAXN; ++N)
		{
		InitRandomColor(Seed);
		vector<string> Colors;
		GetRandomColors(N, Colors);
		double X = 3*MARGIN;
		for (uint i = 0; i < N; ++i)
			{
			const string Color = Colors[i];
			S.Rect(X, Y, SZ, SZ, 1, Color, Color);
			X += SZ + MARGIN;
			}
		Y += SZ + 2*MARGIN;
		}

	S.Close();
	}

void cmd_tree2palette()
	{
	const string &TreeFileName = opt(tree2palette);
	asserta(optset_ff);

	const string &FF = opt(ff);
	if (SIZE(FF) != 2 || !isdigit(FF[1]))
		Die("Invalid ff");

	char Sep = FF[0];
	char Digit = FF[1];
	if (Digit == '0')
		Die("Invalid ff (field must be >0)");
	uint FieldIndex = uint(Digit - '0') - 1;

	vector<TreeN *> Trees;
	TreesFromFile(TreeFileName, Trees);

	vector<string> Values;
	GetValuesFromTrees(Trees, Sep, FieldIndex, ".", Values);


	uint Seed = 1;
	if (optset_randseed)
		Seed = opt(randseed);
	InitRandomColor(Seed);

	const uint ValueCount = SIZE(Values);
	vector<string> Colors;
	GetRandomColors(ValueCount, Colors);
	asserta(SIZE(Colors) == ValueCount);

	FILE *fOut = CreateStdioFile(opt(output));
	for (uint ValueIndex = 0; ValueIndex < ValueCount; ++ValueIndex)
		{
		const string &Value = Values[ValueIndex];
		const string &Color = Colors[ValueIndex];
		ProgressLog("  %s    %s\n", Color.c_str(), Value.c_str());
		Pf(fOut, "%s	%s\n", Value.c_str(), Color.c_str());
		}
	CloseStdioFile(fOut);
	}
