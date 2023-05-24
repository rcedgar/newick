#include "myutils.h"

void Usage(FILE *f);

int main(int argc, char **argv)
	{
#ifdef _MSC_VER
	_putenv("TZ=");
#endif
	setbuf(stdout, 0);
	setbuf(stderr, 0);

	if (argc <= 1)
		{
		Usage(stdout);
		return 0;
		}

	for (int i = 1; i < argc; ++i)
		{
		string s = string(argv[i]);
		if (s == "-h")
			{
			Usage(stdout);
			return 0;
			}
		}

	MyCmdLine(argc, argv);

	if (!opt(quiet))
		{
		PrintProgramInfo(stdout);
		PrintCopyright(stdout);
		}

	SetLogFileName(opt(log));
	LogProgramInfoAndCmdLine();

	extern vector<string> g_Argv;
	uint n = SIZE(g_Argv);
	asserta(n > 0);
	string ShortCmdLine = g_Argv[1];
	if (n > 2)
		ShortCmdLine += " " + g_Argv[2];

	ProgressPrefix(false);
	Progress("[%s]\n", ShortCmdLine.c_str() + 1);
	ProgressPrefix(true);

	CMD Cmd = GetCmd();
	switch (Cmd)
		{
#define A(x)	case CMD_##x: { void cmd_##x(); cmd_##x(); break; }
#include "cmds.h"
	default:
		asserta(false);
		}

	CheckUsedOpts(opt_log_used_opts);

	LogElapsedTimeAndRAM();
	return 0;
	}
