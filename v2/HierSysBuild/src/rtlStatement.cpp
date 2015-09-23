#include <istream>
#include <ostream>
#include <assert.h>
#include <hierSystem.h>
#include <rtlInclusions.h>
#include <rtlEnums.h>
#include <rtlType.h>
#include <Value.hpp>
#include <rtlValue.h>
#include <rtlObject.h>
#include <rtlExpression.h>
#include <rtlStatement.h>
#include <rtlThread.h>
	
rtlStatement::rtlStatement(rtlThread* p):hierRoot()
{
	_parent_thread = p;
}

	
rtlAssignStatement::rtlAssignStatement(rtlThread* p,bool volatile_flag,  rtlExpression* tgt, rtlExpression* src):rtlStatement(p)
{
	_target = tgt;
	tgt->Set_Is_Target(true);

	_source = src;
	_volatile = volatile_flag;
	tgt->Set_Is_Volatile(volatile_flag);
}




	
void rtlAssignStatement::Print(ostream& ofile)
{
	if(_volatile)
		ofile << " $volatile ";
	_target->Print(ofile);
	ofile << " := ";
	_source->Print(ofile);
	ofile << endl;
}

	
void rtlAssignStatement::Print_C(ostream& source_file)
{
	source_file << "//";
	this->Print(source_file); 

	rtlType* tt = _target->Get_Type();
	if(tt->Is("rtlIntegerType"))
	{
		_target->Print_C(source_file);
		source_file << " = " ;
		_source->Print_C(source_file);
		source_file << ";" << endl;
	}
	else if(tt->Is("rtlUnsignedType") || tt->Is("rtlSignedType"))
	{
		_target->Print_C(source_file); // TODO: object references as target handled differently.
						//  In this case, the printing is only of rvalues in
						// the expression (e.g. array index expressions).
		_source->Print_C(source_file);
		source_file  << "bit_vector_bitcast_to_bit_vector(&(" << _target->Get_C_Name() << "), &(" 
				<< _source->Get_C_Name() << "));" << endl;
	}
	else
		assert(0);
}

	
rtlEmitStatement::rtlEmitStatement(rtlThread* p, rtlObject* emittee):rtlStatement(p)
{
	_object = emittee;
	_object->Set_Is_Emitted(true);
}

void rtlEmitStatement::Print(ostream& ofile)
{
	ofile << " $emit " << _object->Get_Id() << endl;
}

void rtlEmitStatement::Print_C(ostream& source_file)
{
	source_file << "//";
	this->Print(source_file); 
	rtlType* tt = _object->Get_Type();
	rtlInterfaceGroup* ng = _object->Get_Group();

	if(ng != NULL)
	{
		if(ng->_is_pipe_access)
		{
			source_file << "{" << endl;
			source_file << "char __ack_flag;" << endl;
			source_file << "probeMatcher("
				   << 	"__sstate->__" << ng->Get_Id() << ","
				   <<   (ng->_is_input ? "0" : "1") << ","
				   <<   "1, &__ack_flag, &(" 
				   <<    ng->_data->Get_C_Name() << "));" << endl;
			source_file << "bit_vector_assign_uint64(0, &(" << ng->_ack->Get_C_Name() << "), __ack_flag);" << endl;
			source_file << "}" << endl;
		}
		else
		{
			if(ng->_is_input)
			{
				source_file << "bit_vector_bitcast_to_bit_vector(&(" << ng->_data->Get_C_Name() << "), "
						<< "getSignalValue(__sstate->__" << ng->Get_Id() << "));" << endl;
			}
			else
			{
				source_file << "assignSignalValue(__sstate->__" << ng->Get_Id() << ", &(" 
						<< ng->_data->Get_C_Name() << "));" << endl;
			}
		}
	}
	else
	{
		if(tt->Is("rtlIntegerType"))
		{
			source_file << "__sstate->" << _object->Get_C_Name() << " = 1";
		}
		else if(tt->Is("rtlUnsignedType") || tt->Is("rtlSignedType"))
		{
			source_file << "bit_vector_set(&(" << _object->Get_C_Name() << "));" << endl;
		}
		else
			assert(0);
	}
}
	
rtlGotoStatement::rtlGotoStatement(rtlThread* p, string lbl):rtlStatement(p) 
{ 
	_label = lbl;
}

void rtlGotoStatement::Print(ostream& ofile)
{
	ofile << " $goto " << _label << endl;
	if(_parent_thread->Get_Statement(_label) == NULL)
	{
		_parent_thread->Get_Parent()->Report_Error("goto " + _label + " : no statement with this label in thread "
				+ _parent_thread->Get_Id());
	}
}

void rtlGotoStatement::Print_C(ostream& source_file)
{
	source_file << "//";
	this->Print(source_file); 
	source_file << "__sstate->_next_state = " << stateEnum(_label) << ";" << endl;
}

void rtlIfStatement::Print(ostream& ofile)
{
	ofile << " $if ";
	_test->Print(ofile);
	ofile << endl;
	_if_block->Print(ofile);
	ofile << " $else " << endl;
	_else_block->Print(ofile);
	ofile << endl;
}

void rtlIfStatement::Print_C(ostream& source_file)
{
	_test->Print_C(source_file);
	source_file << "// if ";
	_test->Print(source_file);
	source_file << endl;

	source_file << "if (" << _test->C_Int_Reference() << ")";
	_if_block->Print_C(source_file);
	if(_else_block != NULL)
	{
		source_file << "// else" << endl;
		source_file << "else " << endl;
		_else_block->Print_C(source_file);
	}
}

void rtlBlockStatement::Print(ostream& ofile)
{
	ofile << "{" << endl;
	for(int I = 0, fI = _statement_block.size(); I < fI; I++)
	{
		_statement_block[I]->Print(ofile);
	}
	ofile << "}" << endl;
}

void rtlBlockStatement::Print_C(ostream& source_file)
{
	source_file << "{" << endl;
	for(int I = 0, fI = _statement_block.size(); I < fI; I++)
	{
		_statement_block[I]->Print_C(source_file);
	}
	source_file << "}" << endl;
}

void rtlLabeledBlockStatement::Print(ostream& ofile)
{
	ofile << "<" << _label << "> ";
	this->rtlBlockStatement::Print(ofile);
}

void rtlLabeledBlockStatement::Print_C(ostream& source_file)
{
	// no label... 
	this->rtlBlockStatement::Print_C(source_file);
}

rtlNullStatement::rtlNullStatement(rtlThread* p): rtlStatement(p)
{
}

void rtlNullStatement::Print(ostream& ofile)
{
	ofile << " $null " << endl;
}

