#ifndef quarts_h
#define quarts_h

struct Quarts
	{
	unsigned Min;
	unsigned LoQ;
	unsigned Med;
	unsigned HiQ;
	unsigned Max;
	unsigned Total;
	double Avg;
	};

struct QuartsDouble
	{
	double Min;
	double LoQ;
	double Med;
	double HiQ;
	double Max;
	double Total;
	double Avg;
	double StdDev;
	};

void GetQuarts(const vector<unsigned> &v, Quarts &Q);
void GetQuarts(const vector<double> &v, QuartsDouble &Q);

#endif // quarts_h
