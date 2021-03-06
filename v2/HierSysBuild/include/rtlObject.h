#ifndef _rtl_Object_h__
#define _rtl_Object_h__

class rtlType;
class rtlInterfaceGroup;

class rtlObject: public hierRoot
{
	protected:

		rtlType* _type;
		int      _number_of_drivers;

		bool      _assigned_under_volatile;
		bool      _not_assigned_under_volatile;
		bool      _assigned_under_tick;

	public:

		rtlObject(string name, rtlType* t);

		virtual string Kind() {return("rtlObject");}

		virtual void Set_Assigned_Under_Volatile(bool v);
		virtual void Set_Not_Assigned_Under_Volatile(bool v);
		virtual void Set_Assigned_Under_Tick(bool v);

		virtual bool Get_Assigned_Under_Volatile() {return(_assigned_under_volatile);}
		virtual bool Get_Not_Assigned_Under_Volatile() {return(_not_assigned_under_volatile);}
		virtual bool Get_Assigned_Under_Tick() {return(_assigned_under_tick);}

		virtual bool Is_Variable() {return(false);}
		virtual bool Is_Signal()   {return(false);}
		virtual bool Is_Constant() {return(false);}
		virtual bool Is_InPort()   {return(false);}
		virtual bool Is_OutPort()  {return(false);}
		virtual bool Is_Pipe()     {return(false);}


		void Increment_Number_Of_Drivers() {_number_of_drivers++;}
		int Get_Number_Of_Drivers() {return(_number_of_drivers);}

		virtual rtlValue* Get_Value() {return(NULL);}
		rtlType* Get_Type() {return(_type);}

		virtual void Print(ostream& ofile) {assert(0);}

		virtual string Get_C_Name() {return("__sstate->" + this->Get_Id());}
		virtual string Get_C_Req_Name() {return("__sstate->" + this->Get_Id() + "__req");}
		virtual string Get_C_Ack_Name() {return("__sstate->" + this->Get_Id() + "__ack");}

		virtual string Get_C_Target_Name() 
		{
			if(this->Needs_Next())
				return("__sstate->__next__" + this->Get_Id());
			else
				return("__sstate->" + this->Get_Id());
		}
		virtual string Get_Variable_Id()
		{
			return(this->Get_Id());
		}
		virtual void Print_C_Struct_Field_Initialization(string obj_name, ostream& source_file);
		virtual void Print_C_Probe_Matcher(ostream& source_file);

		virtual void Set_Is_Emitted(bool v) {}
		virtual bool Get_Is_Emitted() {return(false);}

		virtual bool Needs_Next() {return(false);}
		virtual bool Needs_Next_Vhdl_Variable() {return(false);}
};

class rtlConstant: public rtlObject
{
	rtlValue* _value;
	public:

	rtlConstant(string name, rtlType* t, rtlValue* v); 
	virtual string Kind() {return("rtlConstant");}

	virtual bool Is_Constant() {return(true);}
	virtual rtlValue* Get_Value() {return(_value);}
	virtual void Print(ostream& ofile);

	virtual void Print_C_Struct_Field_Initialization(string obj_name,  ostream& source_file);
};

class rtlVariable: public rtlObject
{
	public:

		rtlVariable(string name, rtlType* t);
		virtual string Kind() {return("rtlVariable");}
		virtual bool Is_Variable() {return(true);}
		virtual void Print(ostream& ofile);

};


class rtlSignal: public rtlObject
{
	bool _is_pipe;
	bool _tick;

	public:

	rtlSignal(string name, rtlType* t);
	rtlSignal(bool is_pipe, string name, rtlType* t);

	virtual string Kind() {return("rtlSignal");}

	virtual bool Is_Signal() {return(true);}
	virtual bool Is_Pipe() {return(_is_pipe);}
	virtual void Print(ostream& ofile);
	virtual bool Needs_Next() {return(!this->Get_Assigned_Under_Volatile());}
	virtual bool Needs_Next_Vhdl_Variable() 
	{
		return(!this->Is_InPort() && !this->Get_Assigned_Under_Volatile() && !this->Get_Assigned_Under_Tick());
	}

	virtual string Get_Variable_Id()
	{
		return("n_e_x_t_" + this->Get_Id());
	}
};

class rtlInPort: public rtlSignal
{
	public:
		rtlInPort(bool is_pipe, string name, rtlType* t) : rtlSignal(is_pipe, name,t) {}
		virtual string Kind() {return("rtlInPort");}
		virtual bool Is_InPort() {return(true);}
		virtual void Print(ostream& ofile);

		// in-port does not need registering.
		virtual bool Needs_Next() {return(false);}

};

class rtlOutPort: public rtlSignal
{
	public:
		rtlOutPort(bool is_pipe, string name, rtlType* t) : rtlSignal(is_pipe, name,t) {}
		virtual string Kind() {return("rtlOutPort");}

		virtual bool Is_OutPort() {return(true);}
		virtual void Print(ostream& ofile);

		virtual bool Needs_Next() {return(!this->Get_Assigned_Under_Volatile());}

};
#endif
