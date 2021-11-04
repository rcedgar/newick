#pragma once

enum NEWICK_TOKEN_TYPE
	{
	NTT_Unknown,
	NTT_EOF,

// Returned from Tree::GetToken:
	NTT_Lparen,
	NTT_Rparen,
	NTT_Colon,
	NTT_Comma,
	NTT_Semicolon,
	NTT_String,

// Following are never returned from Tree::GetToken:
	NTT_SingleQuotedString,
	NTT_DoubleQuotedString,
	NTT_Comment
	};

const char *NTTToStr(NEWICK_TOKEN_TYPE NTT);
