#include "myutils.h"
#include "newick_token.h"

/***
Tokens in Newick files are:
	( ) : , ;
	string
	'string'
	"string"
	[ comment ]

We can't safely distinguish between identifiers and floating point
numbers at the lexical level (because identifiers may be numeric,
or start with digits), so both edge lengths and identifiers are
returned as strings.
***/

const char *NTTToStr(NEWICK_TOKEN_TYPE NTT)
	{
	switch (NTT)
		{
#define c(x)	case NTT_##x: return #x;
	c(Unknown)
	c(Lparen)
	c(Rparen)
	c(Colon)
	c(Comma)
	c(Semicolon)
	c(String)
	c(SingleQuotedString)
	c(DoubleQuotedString)
	c(Comment)
#undef c
		}
	return "??";
	}
