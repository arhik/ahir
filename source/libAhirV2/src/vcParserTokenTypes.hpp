#ifndef INC_vcParserTokenTypes_hpp_
#define INC_vcParserTokenTypes_hpp_

/* $ANTLR 2.7.7 (2006-11-01): "vc.g" -> "vcParserTokenTypes.hpp"$ */

#ifndef CUSTOM_API
# define CUSTOM_API
#endif

#ifdef __cplusplus
struct CUSTOM_API vcParserTokenTypes {
#endif
	enum {
		EOF_ = 1,
		PIPE = 4,
		UINTEGER = 5,
		MEMORYSPACE = 6,
		LBRACE = 7,
		RBRACE = 8,
		CAPACITY = 9,
		DATAWIDTH = 10,
		ADDRWIDTH = 11,
		OBJECT = 12,
		COLON = 13,
		MODULE = 14,
		SIMPLE_IDENTIFIER = 15,
		EQUIVALENT = 16,
		LPAREN = 17,
		RPAREN = 18,
		OPEN = 19,
		DIV_OP = 20,
		ENTRY = 21,
		EXIT = 22,
		CONTROLPATH = 23,
		PLACE = 24,
		TRANSITION = 25,
		SERIESBLOCK = 26,
		PARALLELBLOCK = 27,
		BRANCHBLOCK = 28,
		MERGE = 29,
		BRANCH = 30,
		FORKBLOCK = 31,
		JOIN = 32,
		FORK = 33,
		DATAPATH = 34,
		PLUS_OP = 35,
		MINUS_OP = 36,
		MUL_OP = 37,
		SHL_OP = 38,
		SHR_OP = 39,
		GT_OP = 40,
		GE_OP = 41,
		EQ_OP = 42,
		LT_OP = 43,
		LE_OP = 44,
		NEQ_OP = 45,
		BITSEL_OP = 46,
		CONCAT_OP = 47,
		OR_OP = 48,
		AND_OP = 49,
		XOR_OP = 50,
		NOR_OP = 51,
		NAND_OP = 52,
		XNOR_OP = 53,
		NOT_OP = 54,
		BRANCH_OP = 55,
		SELECT_OP = 56,
		ASSIGN_OP = 57,
		CALL = 58,
		INLINE = 59,
		IOPORT = 60,
		IN = 61,
		OUT = 62,
		LOAD = 63,
		FROM = 64,
		STORE = 65,
		TO = 66,
		PHI = 67,
		LBRACKET = 68,
		RBRACKET = 69,
		CONSTANT = 70,
		WIRE = 71,
		COMMA = 72,
		BINARYSTRING = 73,
		HEXSTRING = 74,
		MINUS = 75,
		LITERAL_C = 76,
		LITERAL_M = 77,
		INT = 78,
		FLOAT = 79,
		POINTER = 80,
		ARRAY = 81,
		OF = 82,
		RECORD = 83,
		ATTRIBUTE = 84,
		IMPLIES = 85,
		QUOTED_STRING = 86,
		DPE = 87,
		LIBRARY = 88,
		REQS = 89,
		ACKS = 90,
		HIDDEN = 91,
		PARAMETER = 92,
		PORT = 93,
		MAP = 94,
		MIN = 95,
		MAX = 96,
		DPEINSTANCE = 97,
		LINK = 98,
		AT = 99,
		UGT_OP = 100,
		UGE_OP = 101,
		ULT_OP = 102,
		ULE_OP = 103,
		UNORDERED_OP = 104,
		WHITESPACE = 105,
		SINGLELINECOMMENT = 106,
		HIERARCHICAL_IDENTIFIER = 107,
		ALPHA = 108,
		DIGIT = 109,
		NULL_TREE_LOOKAHEAD = 3
	};
#ifdef __cplusplus
};
#endif
#endif /*INC_vcParserTokenTypes_hpp_*/
