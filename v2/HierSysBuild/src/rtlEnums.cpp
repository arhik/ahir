#include <rtlInclusions.h>
#include <rtlEnums.h>


string rtlOp_To_String(rtlOperation op)
{
	string ret_string;
	switch(op)
	{

		case __NOP:
			break;
		case __NOT:
			ret_string = "~";
			break;
		case __OR:
			ret_string = "|";
			break;
		case __AND:
			ret_string = "&";
			break;
		case __XOR:
			ret_string = "^";
			break;
		case __NOR:
			ret_string = "~|";
			break;
		case __NAND:
			ret_string = "~&";
			break;
		case __XNOR:
			ret_string = "~^";
			break;
		case __SHL:
			ret_string = "<<";
			break;
		case __SHR:
			ret_string = ">>";
			break;
		case __ROR:
			ret_string = ">o>";
			break;
		case __ROL:
			ret_string = "<o<";
			break;
		case __EQUAL:
			ret_string = "==";
			break;
		case __NOTEQUAL:
			ret_string = "!=";
			break;
		case __LESS:
			ret_string = "<";
			break;
		case __LESSEQUAL:
			ret_string = "<=";
			break;
		case __GREATER:
			ret_string = ">";
			break;
		case __GREATEREQUAL:
			ret_string = ">=";
			break;
		case __PLUS:
			ret_string = "+";
			break;
		case __MINUS:
			ret_string = "-";
			break;
		case __MUL:
			ret_string = "*";
			break;
		case __CONCAT:
			ret_string = "&&";
			break;
		default:
			break;
	}

	return(ret_string);

}

string rtlOp_To_Vhdl_String(rtlOperation op)
{
	string ret_string;
	switch(op)
	{

		case __NOP:
			break;
		case __NOT:
			ret_string = "not";
			break;
		case __OR:
			ret_string = "or";
			break;
		case __AND:
			ret_string = "and";
			break;
		case __XOR:
			ret_string = "xor";
			break;
		case __NOR:
			ret_string = "nor";
			break;
		case __NAND:
			ret_string = "nand";
			break;
		case __XNOR:
			ret_string = "xnor";
			break;
		case __SHL:
			ret_string = "shl";
			break;
		case __SHR:
			ret_string = "shr";
			break;
		case __ROR:
			ret_string = "ror";
			break;
		case __ROL:
			ret_string = "rol";
			break;
		case __EQUAL:
			ret_string = "=";
			break;
		case __NOTEQUAL:
			ret_string = "/=";
			break;
		case __LESS:
			ret_string = "<";
			break;
		case __LESSEQUAL:
			ret_string = "<=";
			break;
		case __GREATER:
			ret_string = ">";
			break;
		case __GREATEREQUAL:
			ret_string = ">=";
			break;
		case __PLUS:
			ret_string = "+";
			break;
		case __MINUS:
			ret_string = "-";
			break;
		case __MUL:
			ret_string = "*";
			break;
		case __CONCAT:
			ret_string = "&";
			break;
		default:
			break;
	}

	return(ret_string);

}
