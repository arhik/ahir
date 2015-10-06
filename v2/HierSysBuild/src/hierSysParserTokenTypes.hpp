#ifndef INC_hierSysParserTokenTypes_hpp_
#define INC_hierSysParserTokenTypes_hpp_

/* $ANTLR 2.7.7 (2006-11-01): "hierSys.g" -> "hierSysParserTokenTypes.hpp"$ */

#ifndef CUSTOM_API
# define CUSTOM_API
#endif

#ifdef __cplusplus
struct CUSTOM_API hierSysParserTokenTypes {
#endif
	enum {
		EOF_ = 1,
		SYSTEM = 4,
		SIMPLE_IDENTIFIER = 5,
		LIBRARY = 6,
		IN = 7,
		PIPE = 8,
		SIGNAL = 9,
		UINTEGER = 10,
		DEPTH = 11,
		OUT = 12,
		LBRACE = 13,
		RBRACE = 14,
		INSTANCE = 15,
		COLON = 16,
		IMPLIES = 17,
		LIFO = 18,
		UINT = 19,
		LESS = 20,
		GREATER = 21,
		THREAD = 22,
		DEFAULT = 23,
		NOW = 24,
		TICK = 25,
		STRING = 26,
		VARIABLE = 27,
		CONSTANT = 28,
		ASSIGNEQUAL = 29,
		SPLIT = 30,
		LPAREN = 31,
		RPAREN = 32,
		NuLL = 33,
		GOTO = 34,
		LOG = 35,
		IF = 36,
		ELSE = 37,
		BINARY = 38,
		HEXADECIMAL = 39,
		COMMA = 40,
		REQ = 41,
		ACK = 42,
		LBRACKET = 43,
		RBRACKET = 44,
		SLICE = 45,
		MUX = 46,
		NOT = 47,
		OR = 48,
		AND = 49,
		NOR = 50,
		NAND = 51,
		XOR = 52,
		XNOR = 53,
		SHL = 54,
		SHR = 55,
		ROL = 56,
		ROR = 57,
		PLUS = 58,
		MINUS = 59,
		MUL = 60,
		DIV = 61,
		EQUAL = 62,
		NOTEQUAL = 63,
		LESSEQUAL = 64,
		GREATEREQUAL = 65,
		CONCAT = 66,
		INTEGER = 67,
		UNSIGNED = 68,
		SIGNED = 69,
		ARRAY = 70,
		OF = 71,
		GROUP = 72,
		EMIT = 73,
		WHITESPACE = 74,
		SINGLELINECOMMENT = 75,
		ALPHA = 76,
		DIGIT = 77,
		NULL_TREE_LOOKAHEAD = 3
	};
#ifdef __cplusplus
};
#endif
#endif /*INC_hierSysParserTokenTypes_hpp_*/
