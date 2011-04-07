#include <vcIncludes.hpp>
#include <vcRoot.hpp>
#include <vcControlPath.hpp>
#include <vcDataPath.hpp>
#include <vcSystem.hpp>

int64_t vcControlPath::_free_index = 0;

vcCompatibilityLabel::vcCompatibilityLabel(vcControlPath* cp, string id): vcCPElement(cp,id)
{
  this->_labeled_in_arc.first = NULL;
}

void vcCompatibilityLabel::Add_In_Arc(vcCompatibilityLabel* u, pair<vcTransition*, int>& arc)
{
  assert(this->_labeled_in_arc.first == NULL);
  this->_labeled_in_arc.first = u;
  this->_labeled_in_arc.second = arc;
}

void vcCompatibilityLabel::Add_In_Arc(vcCompatibilityLabel* u)
{
  this->_unlabeled_in_arcs.insert(u);
}

vcCompatibilityLabel* vcCompatibilityLabel::Reduce(vcControlPath* cp)
{
  vcCompatibilityLabel* ret_label = this;
  bool change_flag = true;;

  while(change_flag)
    {
      
      change_flag = false;
      if(this->_unlabeled_in_arcs.size() > 0)
	assert(this->_labeled_in_arc.first == NULL);
      if(this->_labeled_in_arc.first != NULL)
	assert(this->_unlabeled_in_arcs.size() ==  0);
      
      if(this->_unlabeled_in_arcs.size() == 0)
	return(ret_label);
      
      if(this->_unlabeled_in_arcs.size() == 1)
	{
	  {// just one labeled in arc.
	    ret_label = (*(this->_unlabeled_in_arcs.begin()));
	  }
	  return(ret_label);
	}
      
      // more than one incoming unlabeled edge
      
      vector<vcCompatibilityLabel* > erase_vec;
      vector<vcCompatibilityLabel* > insert_vec;
      
      
      // First, eliminate unlabeled paths of length 2.
      for(set<vcCompatibilityLabel*>::iterator iter = this->_unlabeled_in_arcs.begin();
	  iter != this->_unlabeled_in_arcs.end();
	  iter++)
	{
	  vcCompatibilityLabel* u = *(iter);
	  if(u->_unlabeled_in_arcs.size() > 0)
	    {
	      erase_vec.push_back(u);
	      for(set<vcCompatibilityLabel*>::iterator miter = u->_unlabeled_in_arcs.begin();
		  miter != u->_unlabeled_in_arcs.end();
		  miter++)
		{
		  insert_vec.push_back((*miter));
		}
	    }
	}
      // remove all elements in erase set from unlabeled_in_arcs
      for(int idx = 0; idx < erase_vec.size(); idx++)
	this->_unlabeled_in_arcs.erase(erase_vec[idx]);
      
      // add all elements in insert set to unlabeled_in_arcs
      for(int idx = 0; idx < insert_vec.size(); idx++)
	this->_unlabeled_in_arcs.insert(insert_vec[idx]);
      
      
      set<vcCompatibilityLabel*> insert_set;
      
      // OK, now you have multiple incoming unlabeled arcs..
      // try to reduce.
      map<vcCompatibilityLabel*, map<vcTransition*,int> > reduce_map;
      for(set<vcCompatibilityLabel*>::iterator iter = this->_unlabeled_in_arcs.begin();
	  iter != this->_unlabeled_in_arcs.end();
	  iter++)
	{
	  vcCompatibilityLabel* u = (*iter);
	  assert(u->_unlabeled_in_arcs.size() == 0);
	  
	  vcCompatibilityLabel* v = u->_labeled_in_arc.first;
	  if(v != NULL)
	    {
	      
	      vcTransition* t = u->_labeled_in_arc.second.first;
	      
	      if(reduce_map[v].find(t) == reduce_map[v].end())
		(reduce_map[v])[t] = 1;
	      else
		((reduce_map[v])[t])++;
	      
	      if((reduce_map[v])[t] == t->Get_Number_Of_Successors())
		{
		  insert_set.insert(v);
		}
	    }
	}
      
      // based on the insert set, compute the reduce set.
      set<vcCompatibilityLabel*> erase_set;
      for(set<vcCompatibilityLabel*>::iterator iter = this->_unlabeled_in_arcs.begin();
	  iter != this->_unlabeled_in_arcs.end();
	  iter++)
	{
	  vcCompatibilityLabel* u = (*iter);
	  assert(u->_unlabeled_in_arcs.size() == 0);
	  
	  vcCompatibilityLabel* v = u->_labeled_in_arc.first;
	  if(v != NULL)
	    {
	      if(insert_set.find(v) != insert_set.end())
		{
		  erase_set.insert(u);
		}
	    }
	}
      
      // now remove elements from the erase set and
      // add elements from the insert set
      for(set<vcCompatibilityLabel*>::iterator iter = erase_set.begin();
	  iter != erase_set.end();
	  iter++)
	{
	  this->_unlabeled_in_arcs.erase((*iter));
	  change_flag = true;
	}
      
      for(set<vcCompatibilityLabel*>::iterator iter = insert_set.begin();
	  iter != insert_set.end();
	  iter++)
	{
	  this->_unlabeled_in_arcs.insert((*iter));
	  change_flag = true;
	}
      
      // thats it.  you have done one pass of the  reduction.. one last step remains..
      // after the previous reduction, you may be left with a unique
      // unlabeled in arc... In this case, break out of the
      // loop..
      if(this->_unlabeled_in_arcs.size() == 1)
	{
	  ret_label = *(this->_unlabeled_in_arcs.begin());
	  break;
	}
    }

  // one final reduction needs to be done..
  // it is possible that the return label already
  // exists in the set of computed labels..
  //  if(ret_label->Is_Join())
  //    ret_label = cp->Find_Or_Map_Join_Label(ret_label->Get_Join_String(),ret_label);

  return(ret_label);
}

bool vcCompatibilityLabel::Is_Compatible(vcCompatibilityLabel* other)
{
  if(this == other)
    return true;
  else if(((vcControlPath*)(this->_parent))->Are_Compatible(this,other))
    return true;
  else
    return false;
}


bool operator==(vector<vcCompatibilityLabel*>& u, vector<vcCompatibilityLabel*>& v)
{
  bool ret_flag = true;
  for(int idx = 0; idx < u.size(); idx++)
    {
      for(int jdx = 0; idx < v.size(); jdx++)
	{
	  if(!(u[idx]->Is_Compatible(v[idx])))
	    {
	      ret_flag = false;
	      break;
	    }
	}
    }
  return(ret_flag);
}

bool operator==(set<vcCompatibilityLabel*>& u, set<vcCompatibilityLabel*>& v)
{
  bool ret_flag = true;
  for(set<vcCompatibilityLabel*>::iterator uiter = u.begin();
      uiter != u.end();
      uiter++)
    {
      for(set<vcCompatibilityLabel*>::iterator viter = v.begin();
	  viter != v.end();
	  viter++)
	{
	  if(!(*uiter)->Is_Compatible((*viter)))
	    {
	      ret_flag = false;
	      break;
	    }
	}
    }
  return(ret_flag);
}

void vcCompatibilityLabel::Print(ostream& ofile)
{
  ofile << "label " << this->Get_Id() << " : " << endl;
  if(this->_labeled_in_arc.first != NULL)
    {
      ofile << "\t labeled predecessor (" << this->_labeled_in_arc.first->Get_Id() << ", " 
	    << this->_labeled_in_arc.second.first->Get_Hierarchical_Id() << ", " 
	    << this->_labeled_in_arc.second.second << ")" << endl;
    }
  else if(this->_unlabeled_in_arcs.size() > 0)
    {
      ofile << "\t unlabeled predecessor(s) " << endl;
      for(set<vcCompatibilityLabel*>::iterator iter = this->_unlabeled_in_arcs.begin();
	  iter != this->_unlabeled_in_arcs.end();
	  iter++)
	{
	  ofile << "\t\t" << (*iter)->Get_Id() << endl;
	}
      ofile << endl;
    }
  else
    ofile << endl;
}

vcCPElement::vcCPElement(vcCPElement* parent, string id):vcRoot(id)
{
  this->_index = vcControlPath::_free_index;
  vcControlPath::_free_index++;

  this->_compatibility_label = NULL;
  this->_parent = parent;
}
void vcCPElement::Get_Hierarchical_Ref(vector<string>& ref_vec)
{
  if(this->_parent != NULL)
    this->_parent->Get_Hierarchical_Ref(ref_vec);
  ref_vec.push_back(this->Get_Id());
}
string vcCPElement::Get_Hierarchical_Id()
{
  string ret_string;
  vector<string> ref_vec;
  this->Get_Hierarchical_Ref(ref_vec);
  for(int idx = 0; idx < ref_vec.size(); idx++)
    {
      ret_string += ref_vec[idx];
      if((idx+1) < ref_vec.size())
	ret_string += vcLexerKeywords[__SLASH];
    }
  return(ret_string);
}

void vcCPElement::Print_Successors(ostream& ofile)
{
  ofile << this->Get_Hierarchical_Id() << endl
	<< "\t (label =  " << this->Get_Compatibility_Label()->Get_Id() 
	<< ")" << endl << "\t -> " << endl;

  for(int idx =0; idx < this->_successors.size(); idx++)
    {
      ofile << "\t\t" << this->_successors[idx]->Get_Hierarchical_Id() << endl;
    }
  ofile << endl;
}
vcTransition::vcTransition(vcCPElement* parent, string id):vcCPElement(parent, id)
{
  _is_input = false;
  _is_output = false;
  _is_dead = false;
}



void vcTransition::Print(ostream& ofile)
{
  ofile << vcLexerKeywords[__TRANSITION] << " " << this->Get_Label() << endl;
}

#define MAX(x,y) (x > y ? x : y)

//
// slightly complicated but not too bad.
// you will need to instantiate places if the transition is a join..
//
void vcTransition::Print_VHDL(ostream& ofile)
{
  if(this->Get_Is_Dead())
  {
	// it will never ever fire... tie it to false.
      ofile << this->Get_Exit_Symbol() << " <= false;" << endl;
      return;
  }

  // every transition has at least one predecessor (entry of control-path has no
  // explicit predecessor, but does have an implicit one.
  
  if(this->Get_Number_Of_Predecessors() > 1)
    {

      ofile << this->Get_VHDL_Id() << "_block : Block -- non-trivial join transition " << this->Get_Hierarchical_Id() << " {" <<  endl;

      ofile << "signal " <<  this->Get_VHDL_Id() << "_predecessors: BooleanArray(" 
	    << this->Get_Number_Of_Predecessors()-1 << " downto 0);" << endl;
      ofile << "-- }" << endl << "begin -- {" << endl;

      for(int idx = 0; idx < this->Get_Number_Of_Predecessors(); idx++)
	{
	  vcCPElement* pred = this->Get_Predecessors()[idx];
	  ofile << this->Get_VHDL_Id() << "_predecessors(" << idx << ") <= "
		<< pred->Get_Exit_Symbol() << ";" << endl;
	}

      if(this->Get_Is_Input())
	{
	  ofile << this->Get_VHDL_Id() << "_join: join_with_input -- {" << endl
		<< "port map( -- {"
		<< "preds => " << this->Get_VHDL_Id() <<  "_predecessors," << endl
		<< "symbol_in => " << this->Get_DP_To_CP_Symbol() << "," << endl
		<< "symbol_out => " << this->Get_Exit_Symbol() << "," << endl
		<< "clk => clk," << endl
		<< "reset => reset); -- }}" << endl;
	}
      else
	{
	  ofile << this->Get_VHDL_Id() << "_join: join -- {" << endl
		<< "port map( -- {"
		<< "preds => " << this->Get_VHDL_Id() <<  "_predecessors," << endl
		<< "symbol_out => " << this->Get_Exit_Symbol() << "," << endl
		<< "clk => clk," << endl
		<< "reset => reset); -- }}" << endl;
	}
    }
  else if(this->Get_Number_Of_Predecessors() == 1)
    {
      // if only one predecessor, then direct connection from predecessors(0).
      if(!this->_is_input)
	{
	  vcCPElement* pred = this->Get_Predecessors()[0];
	  ofile <<  this->Get_Exit_Symbol() << " <= " << pred->Get_Exit_Symbol() << "; -- transition " << this->Get_Hierarchical_Id() << endl;
	}
      else
	{
	  ofile <<  this->Get_Exit_Symbol() << " <= " << this->Get_DP_To_CP_Symbol() << "; -- transition " << this->Get_Hierarchical_Id() << endl;
	}
    }
  else
    {
      ofile <<  this->Get_Exit_Symbol() << "  <= " << this->Get_Parent()->Get_Start_Symbol() << "; -- transition " << this->Get_Hierarchical_Id() << endl;
    }

  if(this->Get_Is_Output())
    ofile << this->Get_CP_To_DP_Symbol() << " <= " << this->Get_Exit_Symbol() << "; -- link to DP" << endl;
  
  if(this->Get_Number_Of_Predecessors() > 1) // block was used...
    ofile << "-- }" << endl << "end Block; -- non-trivial join transition " << this->Get_Hierarchical_Id() << endl;

}

string vcTransition::Get_CP_To_DP_Symbol()
{
  string ret_string;
  for(int idx = 0; idx < _dp_link.size(); idx++)
    {
      int req_idx = _dp_link[idx].first->Get_Req_Index(this);
      if(req_idx >= 0)
	{
	  if(ret_string != "")
	    ret_string += "_";

	  ret_string += _dp_link[idx].first->Get_Id() + "_" + "req_" + IntToStr(req_idx);
	}
    }
  return(To_VHDL(ret_string));
}
string vcTransition::Get_DP_To_CP_Symbol()
{
  string ret_string;
  for(int idx = 0; idx < _dp_link.size(); idx++)
    {
      int ack_idx = _dp_link[idx].first->Get_Ack_Index(this);
      if(ack_idx >= 0)
	{
	  if(ret_string != "")
	    ret_string += "_";

	  ret_string += _dp_link[idx].first->Get_Id() + "_" + "ack_" + IntToStr(ack_idx);
	}
    }
  return(To_VHDL(ret_string));
}
vcPlace::vcPlace(vcCPElement* parent, string id, unsigned int init_marking):vcCPElement(parent, id)
{
  this->_initial_marking = init_marking;
}

void vcPlace::Print(ostream& ofile)
{
  ofile << vcLexerKeywords[__PLACE] << " " << this->Get_Label() << endl;
}

void vcPlace::Print_VHDL(ostream& ofile)
{
  // 
  // the place is simply optimized away to a direct connection..
  //
  ofile << this->Get_Exit_Symbol() <<  "  <=  ";
  for(int idx = 0; idx < this->Get_Number_Of_Predecessors(); idx++)
    {
      if(idx > 0)
	ofile << " or ";
      
      ofile <<  this->Get_Predecessors()[idx]->Get_Exit_Symbol();
    }
  ofile << "; -- place " << this->Get_Hierarchical_Id() << " (optimized away) " << endl;
}

vcCPBlock::vcCPBlock(vcCPBlock* parent, string id): vcCPElement((vcCPElement*)parent, id)
{
  _entry = new vcTransition(this,vcLexerKeywords[__ENTRY]);
  _exit = new vcTransition(this,vcLexerKeywords[__EXIT]);
}

void vcCPBlock::Add_CPElement(vcCPElement* cpe)
{
  if(cpe != NULL)
    {
      assert(this->_element_map.find(cpe->Get_Id()) == this->_element_map.end());
      
      this->_element_map[cpe->Get_Id()] = cpe;
      this->_elements.push_back(cpe);
    }
}

vcCPElement* vcCPBlock::Find_CPElement(string cname)
{
  vcCPElement* ret_cpe = NULL;
  if(cname == vcLexerKeywords[__ENTRY])
    ret_cpe = (vcCPElement*) this->_entry;
  else if(cname == vcLexerKeywords[__EXIT])
    ret_cpe = (vcCPElement*) this->_exit;
  else if(this->_element_map.find(cname) != this->_element_map.end())
    ret_cpe = ((*(this->_element_map.find(cname))).second);
  return(ret_cpe);
}



void vcCPBlock::Print_VHDL_Start_Symbol_Assignment(ostream& ofile)
{
  assert(this->Get_Predecessors().size() == 1);
  ofile << this->Get_Start_Symbol() << " <= " << this->Get_Predecessors()[0]->Get_Exit_Symbol() << "; -- control passed to block" << endl;
}


void vcCPBlock::Print_VHDL_Exit_Symbol_Assignment(ostream& ofile)
{
  ofile << this->Get_Exit_Symbol() << " <= " << this->_exit->Get_Exit_Symbol() << "; -- control passed from block " << endl;
}

void vcCPBlock::Print_VHDL(ostream& ofile)
{
  
  // Hack alert!
  string id = (this->Get_Hierarchical_Id() == "" ? "control-path" : this->Get_Hierarchical_Id());

  // declare all exit flags.
  ofile << this->Get_VHDL_Id() << ": Block -- " << id << " {" << endl;
  ofile << "signal " << this->Get_Start_Symbol() << ": Boolean;" << endl;
  ofile << "signal " << this->_entry->Get_Exit_Symbol() << ": Boolean;" << endl;
  ofile << "signal " << this->_exit->Get_Exit_Symbol() << ": Boolean;" << endl;
  for(int idx = 0; idx < this->_elements.size(); idx++)
    {
      ofile << "signal " << _elements[idx]->Get_Exit_Symbol() << " : Boolean;" << endl;
    }
  ofile << "-- }" << endl << "begin -- {" << endl;
  this->Print_VHDL_Start_Symbol_Assignment(ofile);

  this->_entry->Print_VHDL(ofile);
  for(int idx = 0; idx < this->_elements.size(); idx++)
    {
      this->_elements[idx]->Print_VHDL(ofile);
    }
  this->_exit->Print_VHDL(ofile);
  this->Print_VHDL_Exit_Symbol_Assignment(ofile);

  ofile << "-- }" << endl << "end Block; -- " << id << endl;
}

void vcCPBlock::Print_Elements(ostream& ofile)
{
  for(int idx = 0; idx < this->_elements.size(); idx++)
    this->_elements[idx]->Print(ofile);
}

bool vcCPBlock::Check_Structure() 
{ 
  bool ret_flag = true;
  for(int idx = 0; idx < this->_elements.size(); idx++)
    {
      ret_flag = (ret_flag && this->_elements[idx]->Check_Structure());
      if(!ret_flag)
	break;
    }
  return(ret_flag);
}


void vcCPBlock::Print_Structure(ostream& ofile)
{
  for(int idx = 0; idx < this->_elements.size(); idx++)
    {
      this->_elements[idx]->Print_Structure(ofile);
    }
}


void vcCPBlock::BFS_Order(bool reverse_flag, vcCPElement* start_element, int& num_visited, 
			  vector<vcCPElement*>& bfs_ordered_elements,
			  set<vcCPElement*>& visited_set)
{
  deque<vcCPElement*> bfs_queue;
  set<vcCPElement*> on_queue_set;
  bfs_queue.push_back(start_element);
  on_queue_set.insert(start_element);
  num_visited = 0;

  while(!bfs_queue.empty())
    {
      vcCPElement* top = (bfs_queue.front());

      bfs_queue.pop_front();
      on_queue_set.erase(top);

      visited_set.insert(top);
      bfs_ordered_elements.push_back(top);

      num_visited++;

      vector<vcCPElement*>& adj = (reverse_flag ? top->Get_Predecessors() : top->Get_Successors());
      for(int idx = 0; idx < adj.size(); idx++)
	{
	  vcCPElement* w = adj[idx];
	  if((on_queue_set.find(w) == on_queue_set.end()) && (visited_set.find(w) == visited_set.end()))
	    {// push into queue if not already present in the queue and if not already popped from front..
	      bfs_queue.push_back(w);
	      on_queue_set.insert(w);
	    }
	}
    }
}


void vcCPBlock::DFS_Order(bool reverse_flag, vcCPElement* start_element, bool& cycle_flag, int& num_visited, 
			  vector<vcCPElement*>& dfs_ordered_elements,
			  set<vcCPElement*>& visited_set)
{
  set<vcCPElement*> on_queue_set;
  deque<vcCPElement*> dfs_queue;
  
  dfs_queue.push_front(start_element);
  on_queue_set.insert(start_element);

  num_visited = 1;
  visited_set.insert(start_element);
  dfs_ordered_elements.push_back(start_element);

  while(!dfs_queue.empty())
    {
      vcCPElement* top = dfs_queue.front();
      vector<vcCPElement*>& adj = (reverse_flag ? top->Get_Predecessors() : top->Get_Successors());
      bool all_neighbours_visited = true;
      for(int idx = 0; idx < adj.size(); idx++)
	{
	  vcCPElement* w = adj[idx];
	  if(on_queue_set.find(w) != on_queue_set.end())
	    {
	      cycle_flag = true;
	    }
	  else if(visited_set.find(w) == visited_set.end())
	    {
	      num_visited++;

	      dfs_ordered_elements.push_back(w);
	      visited_set.insert(w);
	      dfs_queue.push_front(w);
	      on_queue_set.insert(w);
	      all_neighbours_visited = false;

	      break;
	    }
	}

      if(all_neighbours_visited)
	{
	  dfs_queue.pop_front();
	  on_queue_set.erase(top);
	}
    }
}

void vcCPBlock::Print_Missing_Elements(set<vcCPElement*>& visited_set)
{
  cerr <<"\t un-visited elements" << endl;
  for(int idx = 0; idx < this->_elements.size(); idx++)
    {
      if(visited_set.find(_elements[idx]) == visited_set.end())
	cerr << "\t" << _elements[idx]->Get_Id() << endl;
    }
}

void vcCPBlock::Compute_Compatibility_Labels(vcCompatibilityLabel* in_label, vcControlPath* cp) {assert(0);}
void vcCPBlock::Update_Predecessor_Successor_Links() 
{
  for(int idx = 0; idx < this->_elements.size(); idx++)
    {
      this->_elements[idx]->Update_Predecessor_Successor_Links();
    }
}

vcCPSeriesBlock::vcCPSeriesBlock(vcCPBlock* p, string id):vcCPBlock(p, id)
{
}


void vcCPSeriesBlock::Print(ostream& ofile)
{
  ofile << vcLexerKeywords[__SERIESBLOCK]  << " [" << this->Get_Id() << "] {" << endl;
  this->Print_Elements(ofile);
  ofile << "\n// end series-block " << this->Get_Id() << endl << "}" << endl;
}


void vcCPSeriesBlock::Print_Structure(ostream& ofile)
{
  ofile << "Structure of Series Region " << this->Get_Hierarchical_Id() << " {" << endl;
  this->_entry->Print_Successors(ofile);
  for(int idx = 0; idx < this->_elements.size(); idx++)
    {
      this->_elements[idx]->Print_Successors(ofile);
    }
  this->_exit->Print_Successors(ofile); 
  ofile << "}" << endl;

  this->vcCPBlock::Print_Structure(ofile);
}


void vcCPSeriesBlock::Compute_Compatibility_Labels(vcCompatibilityLabel* in_label, vcControlPath* cp)
{
  this->Set_Compatibility_Label(in_label);

  this->_entry->Set_Compatibility_Label(in_label);
  for(int idx = 0; idx < this->_elements.size(); idx++)
    this->_elements[idx]->Compute_Compatibility_Labels(in_label,cp);
  this->_exit->Set_Compatibility_Label(in_label);
}

void vcCPSeriesBlock::Update_Predecessor_Successor_Links()
{

  if(this->_elements.size() > 0)
    {
      this->_entry->Add_Successor(this->_elements.front());
      this->_elements.front()->Add_Predecessor(this->_entry);
      for(int idx = 0; idx < this->_elements.size(); idx++)
	{
	  if(idx > 0)
	    {
	      this->_elements[idx]->Add_Predecessor(this->_elements[idx-1]);
	    }
	  if(idx+1 < this->_elements.size())
	    {
	      this->_elements[idx]->Add_Successor(this->_elements[idx+1]);
	    }
	}
      this->_elements.back()->Add_Successor(this->_exit);
      this->_exit->Add_Predecessor(this->_elements.back());
    }
  else
    {
      this->_entry->Add_Successor(this->_exit);
      this->_exit->Add_Predecessor(this->_entry);
    }

  this->vcCPBlock::Update_Predecessor_Successor_Links();
}

vcCPParallelBlock::vcCPParallelBlock(vcCPBlock* p, string id):vcCPBlock(p, id)
{

}


void vcCPParallelBlock::Print(ostream& ofile)
{
  ofile << vcLexerKeywords[__PARALLELBLOCK]  << " [" << this->Get_Id() << "] {" << endl;
  this->Print_Elements(ofile);
  ofile << "\n// end  parallel-block " << this->Get_Id() << endl << "}" << endl;
}


void vcCPParallelBlock::Print_Structure(ostream& ofile)
{
  string id = (this->Get_Hierarchical_Id());
  if(id == "")
    id = this->Get_Id();

  ofile << "Structure of  Region " << id << " {" << endl;
  this->_entry->Print_Successors(ofile);
  for(int idx = 0; idx < this->_elements.size(); idx++)
    {
      this->_elements[idx]->Print_Successors(ofile);
    }
  this->_exit->Print_Successors(ofile);
  ofile << "}" << endl;

  this->vcCPBlock::Print_Structure(ofile);

}



void vcCPParallelBlock::Compute_Compatibility_Labels(vcCompatibilityLabel* in_label, vcControlPath* cp)
{
  this->Set_Compatibility_Label(in_label);
  this->_entry->Set_Compatibility_Label(in_label);

  if(this->_elements.size() > 1)
    {
      for(int idx = 0; idx < this->_elements.size(); idx++)
	{
	  string hid = this->Get_Hierarchical_Id();
	  if(hid == "")
	    hid = this->Get_Id();

	  string id = cp->Get_Id() + "/" +  hid + "[" + IntToStr(idx) + "]";
	  vcCompatibilityLabel* nl = cp->Make_Compatibility_Label(id);

	  pair<vcTransition*,int> p(this->_entry,idx);
	  nl->Add_In_Arc(in_label,p);
	  this->_elements[idx]->Compute_Compatibility_Labels(nl,cp);
	}
    }
  else if(this->_elements.size() == 1)
    this->_elements[0]->Compute_Compatibility_Labels(in_label,cp);

  this->_exit->Set_Compatibility_Label(in_label);
}
void vcCPParallelBlock::Update_Predecessor_Successor_Links()
{
  if(!this->_elements.empty())
    {
      for(int idx = 0; idx < this->_elements.size(); idx++)
	{
	  this->_entry->Add_Successor(this->_elements[idx]);
	  this->_elements[idx]->Add_Predecessor(this->_entry);

	  this->_exit->Add_Predecessor(this->_elements[idx]);
	  this->_elements[idx]->Add_Successor(this->_exit);
	}
    }
  else
    {
      this->_entry->Add_Successor(this->_exit);
      this->_exit->Add_Predecessor(this->_entry);
    }
  this->vcCPBlock::Update_Predecessor_Successor_Links();
}

vcCPBranchBlock::vcCPBranchBlock(vcCPBlock* p, string id):vcCPSeriesBlock(p, id)
{
}

void vcCPBranchBlock::Add_Branch_Point(string bp_name, vector<string>& choice_cpe_vec)
{
  vcCPElement* bp = this->Find_CPElement(bp_name);
  
  if(!(bp != NULL && bp->Is("vcPlace")))
    {
      if(bp == NULL)
	vcSystem::Error("branch point " + bp_name + " does not exist"); 
      else
	vcSystem::Error("branch point " + bp_name + " is not a place"); 
      return;
    }

  for(int idx = 0; idx < choice_cpe_vec.size(); idx++)
    {
      vcCPElement* bre = this->Find_CPElement(choice_cpe_vec[idx]);
      if(bre == NULL)
	{
	  vcSystem::Error("did not find branch choice region " + choice_cpe_vec[idx]); 	  
	  return;
	}
      this->_branch_map[((vcPlace*)bp)].push_back(bre);
    }
}

void vcCPBranchBlock::Add_Merge_Point(string merge_place, string merge_region)
{
  vcCPElement* mp = this->Find_CPElement(merge_place);
  vcCPElement* mr = this->Find_CPElement(merge_region);

  if(mp == NULL)
    {
      vcSystem::Error("did not find place " + merge_place);
      return;
    }

  if(!mp->Is("vcPlace"))
    {
      vcSystem::Error("merge point " + merge_place + " is not a place");
      return;
    }

  if(mr == NULL)
    {
      vcSystem::Error("merged region " + merge_region + " not found");
      return;
    }

  this->_merge_map[(vcPlace*)mp].push_back(mr);
}

void vcCPBranchBlock::Print(ostream& ofile)
{
  ofile << vcLexerKeywords[__BRANCHBLOCK]  << " [" << this->Get_Id() << "] {" << endl;
  this->Print_Elements(ofile);

  // now print the branch and merge points.
  for(map<vcPlace*,vector<vcCPElement*>,vcRoot_Compare>::iterator iter = _branch_map.begin();
      iter != _branch_map.end();
      iter++)
    {
      ofile << (*iter).first->Get_Id() << " " << 
	vcLexerKeywords[__BRANCH] << " (";
      for(int idx = 0; idx < (*iter).second.size(); idx++)
	{
	  ofile << " " << (*iter).second[idx]->Get_Id() << " ";
	}
      ofile << ")" << endl;
    }

  for(map<vcPlace*,vector<vcCPElement*>,vcRoot_Compare>::iterator iter = _merge_map.begin();
      iter != _merge_map.end();
      iter++)
    {
      ofile << (*iter).first->Get_Id() << " " << vcLexerKeywords[__MERGE] << " (";
      for(int idx = 0; idx < (*iter).second.size(); idx++)
	{
	  ofile << " " << (*iter).second[idx]->Get_Id() << " ";
	}
      ofile << ")" << endl;
    }
  ofile << "\n// end branch-block " << this->Get_Id() << endl << "}" << endl;
}


bool vcCPBranchBlock::Check_Structure()
{

  bool ret_flag = this->vcCPBlock::Check_Structure();
  if(ret_flag)
    {

      vector<vcCPElement*> reachable_elements;
      bool cycle_flag = false;
      int num_visited = 0;

      set<vcCPElement*> visited_set;
      this->BFS_Order(false,this->_entry, num_visited, reachable_elements,visited_set);
      if(num_visited != (this->_elements.size() + 2))
	{
	  ret_flag = false;
	  vcSystem::Error("all elements not reachable from entry in region " + this->Get_Hierarchical_Id());
	  this->Print_Missing_Elements(visited_set);
	}

      reachable_elements.clear();
      num_visited = 0;
      cycle_flag = false;
      visited_set.clear();
      this->BFS_Order(true, this->_exit, num_visited, reachable_elements,visited_set);
      if(num_visited != (this->_elements.size() + 2))
	{
	  ret_flag = false;
	  vcSystem::Error("exit not reachable from every element in region " + this->Get_Hierarchical_Id());
	  this->Print_Missing_Elements(visited_set);
	}
    }

  return(ret_flag);
}

void vcCPBranchBlock::Update_Predecessor_Successor_Links()
{
  // branches
  for(map<vcPlace*,vector<vcCPElement*>,vcRoot_Compare>::iterator iter = this->_branch_map.begin();
      iter != this->_branch_map.end();
      iter++)
    {
      vcPlace* p = (*iter).first;
      for(int idx = 0; idx < (*iter).second.size(); idx++)
	{
	  vcCPElement* e = (*iter).second[idx];
	  e->Add_Predecessor(p);
	  p->Add_Successor(e);
	}
    }

  // merges
  for(map<vcPlace*,vector<vcCPElement*>,vcRoot_Compare>::iterator iter = this->_merge_map.begin();
      iter != this->_merge_map.end();
      iter++)
    {
      vcPlace* p = (*iter).first;
      for(int idx = 0; idx < (*iter).second.size(); idx++)
	{
	  vcCPElement* e = (*iter).second[idx];
	  e->Add_Successor(p);
	  p->Add_Predecessor(e);
	}
    }

  this->vcCPBlock::Update_Predecessor_Successor_Links();
}

vcCPForkBlock::vcCPForkBlock(vcCPBlock* p, string id):vcCPParallelBlock(p, id)
{
}



void vcCPForkBlock::Add_Fork_Point(vcTransition* fp, vcCPElement* fre)
{

  if((this->_fork_map.find(fp) == this->_fork_map.end())
     ||
     (this->_fork_map[fp].find(fre) == this->_fork_map[fp].end()))
    {
      this->_fork_map[((vcTransition*)fp)].insert(fre);
      this->_forked_elements.insert(fre);
      
      
      fp->Add_Successor(fre);
      
      if(fre->Is_Transition())
	{
	  this->Add_Join_Point((vcTransition*)fre, fp);
	}
      else
	fre->Add_Predecessor(fp);
    }
}


void vcCPForkBlock::Add_Fork_Point(string& fork_name, vector<string>& fork_cpe_vec)
{
  vcCPElement* fp = this->Find_CPElement(fork_name);
  if(fp == NULL)
    {
      vcSystem::Error("did not find fork point " + fork_name);
    }
  else if(!fp->Is("vcTransition"))
    {
      vcSystem::Error("fork point " + fork_name + " is not a transition");
    }
  else
    {
      for(int idx = 0; idx < fork_cpe_vec.size(); idx++)
	{
	  vcCPElement* fre = this->Find_CPElement(fork_cpe_vec[idx]);
	  if(fre == NULL)
	    {
	      vcSystem::Error("did not find forked region " + fork_cpe_vec[idx]);
	      return;
	    }
	  else 
	    this->Add_Fork_Point((vcTransition*)fp, fre);
	}
    }
}

void vcCPForkBlock::Add_Join_Point(vcTransition* jp, vcCPElement* jre)
{

  if((this->_join_map.find(jp) == this->_join_map.end())
     ||
     (this->_join_map[jp].find(jre) == this->_join_map[jp].end()))
    {
      this->_join_map[((vcTransition*)jp)].insert(jre);
      this->_joined_elements.insert(jre);
      
      jp->Add_Predecessor(jre);
      
      if(jre->Is_Transition())
	{
	  this->Add_Fork_Point((vcTransition*)jre, jp);
	}
      else
	jre->Add_Successor(jp);
    }
}

void vcCPForkBlock::Add_Join_Point(string& join_name, vector<string>& join_cpe_vec)
{
  vcCPElement* jp = this->Find_CPElement(join_name);
  if(jp == NULL)
    vcSystem::Error("did not find fork point " + join_name);
  else if(!jp->Is("vcTransition"))
    vcSystem::Error("fork point " + join_name + " is not a transition");
  else
    {
      for(int idx = 0; idx < join_cpe_vec.size(); idx++)
	{
	  vcCPElement* jre = this->Find_CPElement(join_cpe_vec[idx]);
	  if(jre == NULL)
	    {
	      vcSystem::Error("did not find joined region " + join_cpe_vec[idx]);
	      return;
	    }
	  else
	    this->Add_Join_Point((vcTransition*)jp,jre);
	}
    }
}

void vcCPForkBlock::Print(ostream& ofile)
{
  ofile << vcLexerKeywords[__FORKBLOCK]  << " [" << this->Get_Id() << "] {" << endl;
  this->Print_Elements(ofile);


  for(map<vcTransition*,set<vcCPElement*>,vcRoot_Compare>::iterator iter = _fork_map.begin();
      iter != _fork_map.end();
      iter++)
    {
      ofile << (*iter).first->Get_Id() << " " << 
	vcLexerKeywords[__FORK] << " (" ;
      for(set<vcCPElement*>::iterator siter = (*iter).second.begin(), sfiter = (*iter).second.end();
	  siter != sfiter;
	  siter++)
	{
	  ofile << " " << (*siter)->Get_Id() << " ";
	}
      ofile << ")" << endl;
    }


  for(map<vcTransition*,set<vcCPElement*>,vcRoot_Compare>::iterator iter = _join_map.begin();
      iter != _join_map.end();
      iter++)
    {
      ofile <<  (*iter).first->Get_Id() << " " << 
	vcLexerKeywords[__JOIN] << " (" ;
      for(set<vcCPElement*>::iterator siter = (*iter).second.begin(), sfiter = (*iter).second.end();
	  siter != sfiter;
	  siter++)
	{
	  ofile << " " << (*siter)->Get_Id() << " ";
	}
      ofile << ")" << endl;
    }
  ofile << "\n// end fork-block " << this->Get_Id() << endl << "}" << endl;
}

bool vcCPForkBlock::Check_Structure()
{
  bool ret_flag = this->vcCPBlock::Check_Structure();
  if(ret_flag)
    {
      vector<vcCPElement*> reachable_elements;
      bool cycle_flag = false;
      int num_visited = 0;

      set<vcCPElement*> visited_set;
      this->DFS_Order(false, this->_entry, cycle_flag, num_visited, reachable_elements,visited_set);
      if(num_visited != (this->_elements.size() + 2))
	{
	  ret_flag = false;
	  vcSystem::Error("all elements not reachable from entry in region " + this->Get_Hierarchical_Id());
	  this->Print_Missing_Elements(visited_set);
	}

      if(cycle_flag)
	{
	  vcSystem::Error("Cycles present in fork region " + this->Get_Hierarchical_Id());
	  ret_flag = false;
	}

      reachable_elements.clear();
      num_visited = 0;
      cycle_flag = false;
      visited_set.clear();
      this->BFS_Order(true, this->_exit, num_visited, reachable_elements,visited_set);
      if(num_visited != (this->_elements.size() + 2))
	{
	  ret_flag = false;
	  vcSystem::Error("exit not reachable from every element in region " + this->Get_Hierarchical_Id());
	  this->Print_Missing_Elements(visited_set);
	}
    }

  return(ret_flag);
}

// assumption..  that the graph is acyclic, with a single source and a single sink.
void vcCPForkBlock::Precedence_Order(bool reverse_flag, vcCPElement* start_element, vector<vcCPElement*>& precedence_order)
{
  int idx = 0;

  if(!reverse_flag)
    assert(start_element->Get_Predecessors().size() == 0);
  else
    assert(start_element->Get_Successors().size() == 0);

  set<vcCPElement*> scanned_set;
  scanned_set.insert(start_element);

  set<vcCPElement*> ordered_set;
  // the first candidate.
  while(scanned_set.size() > 0)
    {
      vcCPElement* top =  *(scanned_set.begin());
      scanned_set.erase(top);

      precedence_order.push_back(top);
      ordered_set.insert(top);

      // scan all neighbours of top_element.
      vector<vcCPElement*>& adj = (reverse_flag ? top->Get_Predecessors() : top->Get_Successors());
      for(int idx = 0; idx < adj.size(); idx++)
	{
	  bool add_flag = true;
	  vcCPElement* w = adj[idx];

	  // all predecessors of w marked?
	  vector<vcCPElement*>& adjw = (reverse_flag ? w->Get_Successors() : w->Get_Predecessors());
	  for(int idx2 = 0; idx2 < adjw.size(); idx2++)
	    {
	      vcCPElement* u = adjw[idx2];
	      if(ordered_set.find(u) == ordered_set.end())
		{
		  add_flag = false;
		  break;
		}
	    }
	      
	  if(add_flag) 
	    {
	      scanned_set.insert(w);
	    }
	}
    }
}

void vcCPForkBlock::Compute_Compatibility_Labels(vcCompatibilityLabel* in_label, vcControlPath*  cp)
{
  // inherited label is passed down from _entry.
  // create additional labels whenever there is 
  // a fork.  Whenever there is a join, do 
  // a reduce.
  this->Set_Compatibility_Label(in_label);

  vector<vcCPElement*> reachable_elements;
  int num_visited = 0;
  
  this->Precedence_Order(false, this->_entry,reachable_elements);
  this->_entry->Set_Compatibility_Label(in_label);

  for(int idx = 0; idx < reachable_elements.size(); idx++)
    {

      if(reachable_elements[idx]->Is("vcTransition"))
	{
	  vcTransition* t = (vcTransition*) reachable_elements[idx];

	  map<vcTransition*, set<vcCPElement*>, vcRoot_Compare>::iterator join_iter = this->_join_map.find(t);
	  if(join_iter != this->_join_map.end())
	    {


	      if((*join_iter).second.size() > 1)
		{
		  string id = cp->Get_Id()  + "/" + t->Get_Hierarchical_Id();
		  vcCompatibilityLabel* nl = cp->Make_Compatibility_Label(id);

		  // multiple incoming..
		  for(set<vcCPElement*>::iterator siter = (*join_iter).second.begin(), 
			sfiter = (*join_iter).second.end();
		      siter != sfiter;
		      siter++)
		    {
		      vcCPElement* cpe = (*siter);
		      assert(cpe->Get_Compatibility_Label());
		      
		      nl->Add_In_Arc(cpe->Get_Compatibility_Label());
		    }
		  
		  vcCompatibilityLabel* reduced_label = nl->Reduce(cp);
		  if(reduced_label != nl)
		    {
		      cp->Delete_Compatibility_Label(nl);
		    }

		  t->Compute_Compatibility_Labels(reduced_label,cp);
		}
	      else
		{// trivial join, inherit the label of the predecessor
		  t->Compute_Compatibility_Labels((*((*join_iter).second.begin()))->Get_Compatibility_Label(),cp);
		}
	    }

	  // is it a fork?
	  map<vcTransition*, set<vcCPElement*>, vcRoot_Compare>::iterator fork_iter = this->_fork_map.find(t);
	  if(fork_iter != this->_fork_map.end())
	    {
	      // ok, t was a fork
	      if((*fork_iter).second.size() > 1)
		{
		  // t has multiple successors
		  
		  int idx2 = 0;
		  for(set<vcCPElement*>::iterator siter = (*fork_iter).second.begin(), 
			sfiter = (*fork_iter).second.end();
		      siter != sfiter;
		      siter++)
		    {

		      // for each, create a new label
		      string id = cp->Get_Id() + "/" + t->Get_Hierarchical_Id() + "[" +IntToStr(idx2) + "]";
		      vcCompatibilityLabel* nl = cp->Make_Compatibility_Label(id);
		      pair<vcTransition*, int> npair(t,idx2);
		      nl->Add_In_Arc(t->Get_Compatibility_Label(), npair);

		      // propagate the label into the successor
		      (*siter)->Compute_Compatibility_Labels(nl,cp);

		      idx2++;
		    }
		}
	      else
		{
		  (*((*fork_iter).second.begin()))->Compute_Compatibility_Labels(t->Get_Compatibility_Label(),cp);
		}
	    }
	}
    }
}

void vcCPForkBlock::Update_Predecessor_Successor_Links()
{

  // those that were not covered by explicit fork/join declarations
  vector<vcCPElement*> unforked_elements;
  vector<vcCPElement*> unjoined_elements;
  for(int idx = 0; idx < _elements.size(); idx++)
    {
      if(_elements[idx]->Get_Predecessors().size() == 0)
	unforked_elements.push_back(_elements[idx]);

      if(_elements[idx]->Get_Successors().size() == 0)
	unjoined_elements.push_back(_elements[idx]);	
    }




  // finally, those CPElements in the block which
  // are not forked from an explicitly specified fork
  // are assumed to be forked from $entry.
  if(unforked_elements.size() > 0)
    {
      for(int idx = 0; idx < unforked_elements.size(); idx++)
	{
	  this->Add_Fork_Point(this->_entry,unforked_elements[idx]);
	}
    }

  // finally, those CPElements in the block which
  // are not joined at an explicitly specified join
  // are assumed to join at $exit.
  if(unjoined_elements.size() > 0)
    {
      for(int idx = 0; idx < unjoined_elements.size(); idx++)
	{
	  this->Add_Join_Point(this->_exit, unjoined_elements[idx]);
	}
    }

  if(this->_exit->Get_Predecessors().size() == 0 || this->_entry->Get_Successors().size() == 0)
    {
      // will take care of both sides..
      this->Add_Fork_Point(this->_entry, this->_exit);
    }

  this->vcCPBlock::Update_Predecessor_Successor_Links();
}


vcControlPath::vcControlPath(string id):vcCPSeriesBlock(NULL, id)
{
}

// some thought here.. named transitions must be unique.
vcTransition* vcControlPath::Find_Transition(vector<string>& hier_id)
{
  vcCPElement* cpe = this;
  for(int idx = 0; idx < hier_id.size(); idx++)
    {
      cpe = cpe->Find_CPElement(hier_id[idx]);
      if(cpe == NULL)
	break;
    }
  
  if(cpe != NULL && cpe->Is("vcTransition"))
    return((vcTransition*)cpe);
  else
    return(NULL);
}

vcPlace* vcControlPath::Find_Place(vector<string>& hier_id)
{

  vcCPElement* cpe = this;
  for(int idx = 0; idx < hier_id.size(); idx++)
    {
      cpe = cpe->Find_CPElement(hier_id[idx]);
      if(cpe == NULL)
	break;
    }
  
  if(cpe != NULL && cpe->Is("vcPlace"))
    return((vcPlace*)cpe);
  else
    return(NULL);
}

void vcControlPath::Print(ostream& ofile)
{
  ofile << vcLexerKeywords[__CONTROLPATH] << " {" << endl;
  this->Print_Elements(ofile);
  this->Print_Attributes(ofile);
  ofile << "\n// end controlpath" << endl << "}" << endl;
}


void vcControlPath::Mark_As_Compatible(set<vcCompatibilityLabel*>& uset, set<vcCompatibilityLabel*>& vset)
{
  for(set<vcCompatibilityLabel*>::iterator iter = uset.begin(), fiter = uset.end();
      iter != fiter;
      iter++)
    {
      for(set<vcCompatibilityLabel*>::iterator iter2 = vset.begin(), fiter2 = vset.end();
	  iter2 != fiter2;
	  iter2++)
	{
	  _compatible_label_map[(*iter)].insert(*iter2);
	}
    }
}



void vcControlPath::Update_Compatibility_Map()
{
  // for each label u
  //    look at the successor set succ(u).
  //        If w,x are two successors of u with unlabeled inarcs
  //        then  w,x are compatible.
  // 
  //        If w,x are two successors and if their inarcs
  //        correspond to different forks, then w,x are compatible.
  for(set<vcCompatibilityLabel*>::iterator iter = this->_compatibility_label_set.begin();
      iter != this->_compatibility_label_set.end();
      iter++)
    {

      map<vcTransition*, set<vcCompatibilityLabel*> > fork_map;
      // for each label u
      vcCompatibilityLabel* u = (*iter);
      for(int idx = 0, fidx = u->Get_Number_Of_Successors(); idx < fidx; idx++)
	{
	  // look at all successors
	  vcCompatibilityLabel* w = (vcCompatibilityLabel*)(u->Get_Successor(idx));
	  
	  if(w->_labeled_in_arc.second.first != NULL)
	    {
	      // w and all its descendents are kept in fork_map
	      // entry for the fork.
	      fork_map[w->_labeled_in_arc.second.first].insert(w);
	      fork_map[w->_labeled_in_arc.second.first].insert(_label_descendent_map[w].begin(),
							       _label_descendent_map[w].end());
	    }
	}
      
      for(map<vcTransition*,set<vcCompatibilityLabel*> >::iterator citer = fork_map.begin(),
	    cfiter = fork_map.end();
	  citer != cfiter;
	  citer++)
	{
	  map<vcTransition*,set<vcCompatibilityLabel*> >::iterator niter = citer;
	  niter++;

	  for(; niter != cfiter; niter++)
	    {
	      this->Mark_As_Compatible((*citer).second, (*niter).second);
	    }
	}
    }
}

void vcControlPath::Print_Compatibility_Map(ostream& ofile)
{
  ofile << "Label Compatibility Map:" << endl;
  for(map<vcCompatibilityLabel*,set<vcCompatibilityLabel*> >::iterator iter = _compatible_label_map.begin(),
	fiter = _compatible_label_map.end();
      iter != fiter;
      iter++)
    {
      for(set<vcCompatibilityLabel*>::iterator siter = (*iter).second.begin(),
	    sfiter = (*iter).second.end();
	  siter != sfiter;
	  siter++)
	{
	  ofile << (*iter).first->Get_Id() << " === " << (*siter)->Get_Id() << endl;
	}
    }

  for(map<vcCompatibilityLabel*,set<vcCompatibilityLabel*> >::iterator iter = _label_descendent_map.begin(),
	fiter = _label_descendent_map.end();
      iter != fiter;
      iter++)
    {
      for(set<vcCompatibilityLabel*>::iterator siter = (*iter).second.begin(),
	    sfiter = (*iter).second.end();
	  siter != sfiter;
	  siter++)
	{
	  ofile << (*iter).first->Get_Id() << " >== " << (*siter)->Get_Id() << endl;
	}
    }
}

void vcControlPath::Compute_Compatibility_Labels()
{
  vcCompatibilityLabel* nl = this->Make_Compatibility_Label(this->Get_Id());
  this->vcCPSeriesBlock::Compute_Compatibility_Labels(nl,this);


  // set up connectivity
  for(set<vcCompatibilityLabel*>::iterator iter = this->_compatibility_label_set.begin();
      iter != this->_compatibility_label_set.end();
      iter++)
    {
      if((*iter)->_labeled_in_arc.first != NULL)
	{
	  (*iter)->_labeled_in_arc.first->Add_Successor((*iter));
	  (*iter)->Add_Predecessor((*iter)->_labeled_in_arc.first);
	}
      else 
	{
	  for(set<vcCompatibilityLabel*>::iterator siter = (*iter)->_unlabeled_in_arcs.begin();
	      siter != (*iter)->_unlabeled_in_arcs.end();
	      siter++)
	    {
	      (*iter)->Add_Predecessor((*siter));
	      (*siter)->Add_Successor((*iter));
	    }
	}
    }

  // check that there is exactly one vertex of out-degree 1.
  vcCompatibilityLabel* source = NULL;
  for(set<vcCompatibilityLabel*>::iterator iter = this->_compatibility_label_set.begin();
      iter != this->_compatibility_label_set.end();
      iter++)
    {
      if((*iter)->Get_Number_Of_Predecessors() == 0)
	{
	  if(source != NULL)
	    vcSystem::Error("Multiple source labels in compatibility graph \n");
	  assert(source == NULL && "more than one source in compatibility label graph?");
	  source = (*iter);
	}
    }

  // now check that all labels are reachable from the source
  // and that there is no cycle.
  bool cycle_flag = false;
  int num_visited = 0;
  vector<vcCPElement*> dfs_ordered_elements;
  set<vcCPElement*> v_set;
  this->vcCPBlock::DFS_Order(false, (vcCPElement*) source, cycle_flag, num_visited,dfs_ordered_elements,v_set);
  if(cycle_flag)
    {
      vcSystem::Error("Cycle in compatibility label graph\n");
    }
  if(num_visited != this->_compatibility_label_set.size())
    {
      vcSystem::Error("Some compatibility labels are not reachable from the source label\n");
    }

  // now build the descendant maps.
  // transitive closure..
  v_set.clear();
  this->vcCPBlock::BFS_Order(false, (vcCPElement*) source,num_visited, this->_bfs_ordered_labels,v_set);
  for(int hidx = this->_bfs_ordered_labels.size(); hidx > 0; hidx--)
    {
      int idx = hidx-1;

      vcCompatibilityLabel* u = (vcCompatibilityLabel*) (this->_bfs_ordered_labels[idx]);
      set<vcCompatibilityLabel*>& adj_set = this->_label_descendent_map[u];
      adj_set.insert(u);

      for(int adjidx = 0; adjidx < u->Get_Number_Of_Successors(); adjidx++)
	{
	  adj_set.insert(this->_label_descendent_map[((vcCompatibilityLabel*)(u->Get_Successor(adjidx)))].begin(),
			 this->_label_descendent_map[((vcCompatibilityLabel*) (u->Get_Successor(adjidx)))].end());
	}
    }


  this->Update_Compatibility_Map();
}

bool vcControlPath::Are_Compatible(vcCompatibilityLabel* u, vcCompatibilityLabel* v)
{
  if(u == v)
    return(true);
  else if(this->_label_descendent_map[u].find(v) != this->_label_descendent_map[u].end())
    return(true);
  else if(this->_label_descendent_map[v].find(u) != this->_label_descendent_map[v].end())
    return(true);
  else if(this->_compatible_label_map[u].find(v) != this->_compatible_label_map[u].end())
    return(true);
  else if(this->_compatible_label_map[v].find(u) != this->_compatible_label_map[v].end())
    return(true);
  else
    return(false);
}


bool vcControlPath::Lesser(vcCompatibilityLabel* u, vcCompatibilityLabel* v)
{
  return(this->_label_descendent_map[u].find(v) != this->_label_descendent_map[u].end());
}

bool vcControlPath::Greater(vcCompatibilityLabel* v, vcCompatibilityLabel* u)
{
  return(this->_label_descendent_map[u].find(v) != this->_label_descendent_map[u].end());
}


vcCompatibilityLabel* vcControlPath::Make_Compatibility_Label(string id)
{

#ifdef DEBUG
#else
  id = "cL" + Int64ToStr(vcControlPath::_free_index);
#endif 

  vcCompatibilityLabel* nl = new vcCompatibilityLabel(this,id);
  this->_compatibility_label_set.insert(nl);
  return(nl);
}

 void vcControlPath::Delete_Compatibility_Label(vcCompatibilityLabel* nl)
 {
   if(_compatibility_label_set.find(nl) != _compatibility_label_set.end())
     {
       _compatibility_label_set.erase(nl);
       delete nl;
     }
 }

 bool vcControlPath::Check_Structure()
 {
   this->vcCPSeriesBlock::Update_Predecessor_Successor_Links();
   this->vcCPSeriesBlock::Check_Structure();
 }

 void vcControlPath::Print_Compatibility_Labels(ostream& ofile)
 {
   ofile << "Label Summary " << endl;
  for(set<vcCompatibilityLabel*>::iterator iter = _compatibility_label_set.begin();
      iter != _compatibility_label_set.end();
      iter++)
    {
      ofile << "\t";
      (*iter)->Print(ofile);
      ofile << endl;
    }
  
  ofile << "Label Transitive Closure " << endl;
  for(map<vcCompatibilityLabel*, set<vcCompatibilityLabel*> >::iterator iter = this->_label_descendent_map.begin();
      iter != this->_label_descendent_map.end();
      iter++)
    {
      ofile <<  (*iter).first->Get_Id() << " ==> " << endl;
      for(set<vcCompatibilityLabel*>::iterator siter = (*iter).second.begin();
	  siter != (*iter).second.end();
	  siter++)
	{
	  ofile << "\t";
	  ofile << (*siter)->Get_Id() << endl;
	}
      ofile << endl;
    }
}

bool vcCompatibilityLabel_Compare::operator() (vcCompatibilityLabel* u, vcCompatibilityLabel* v) const
{
  if(u->Get_Parent() != v->Get_Parent())
    return(false);
  
  assert(u->Get_Parent() != NULL && (u->Get_Parent()->Kind() == "vcControlPath"));

  vcControlPath* cp = (vcControlPath*) u->Get_Parent();
  return(cp->Lesser(u,v));
}

void vcControlPath::Print_VHDL_Start_Symbol_Assignment(ostream& ofile)
{
  ofile << this->Get_Start_Symbol() << " <=  true when start = '1' else false; -- control passed to control-path." << endl;
}

void vcControlPath::Print_VHDL_Exit_Symbol_Assignment(ostream& ofile)
{
  ofile << "fin  <=  '1' when " << this->_exit->Get_Exit_Symbol() << " else '0'; -- fin symbol when control-path exits" << endl;
}
