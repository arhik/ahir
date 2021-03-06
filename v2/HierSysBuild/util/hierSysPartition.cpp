#include <signal.h>
#include <Value.hpp>
#include <hierSystem.h>
#include <hierSysParser.hpp>
#include <hierSysLexer.hpp>
#include <rtlThread.h>
using namespace std;
  
// global variables
map<string, hierPipe* > __pmap;

// command-line parsing
extern int optind;
extern char *optarg;
int opt;
int option_index = 0;
struct option long_options[] = {
    {"relaxed-component-visibility", 0, 0, 0},
    {"depend", required_argument, 0, 0},
    {0, 0, 0, 0}
};

void Handle_Segfault(int signal)
{
  cerr << "Error: in hierSys2C: segmentation fault! giving up!!" << endl;
  exit(-1);
}

void Usage_hierSysPartition()
{
  cerr << "brief description: reads hierarchical system, checks it for errors, flattens and partitions into HW/SW as specified. " << endl;
  cerr << "Usage: " << endl;
  cerr << "hierSysPartition [-i <hierarchical-instance-name>]* <file-name> (<file-name>)* " << endl;
  cerr << "   -i <hierarchical-instance-name>   include this instance in the HW side." << endl;
  cerr << "IN PROGRESS :-) " << endl;
}


int  Parse(string filename, vector<hierSystem*>& sys_vec)
{
  int ret_val = 0;
  hierSystem* sys = NULL;
  ifstream infile;
  
  infile.open(filename.c_str());
  if(!infile.is_open())
    {
      cerr << "Error: Could not read file " << filename << endl;
      exit(1);
    }

  hierSysLexer* lexer = new hierSysLexer(infile);
  hierSysParser* parser = new hierSysParser(*lexer);
  
  lexer->setFilename(filename);
  parser->setFilename(filename);
  
  try
    {
      parser->sys_Description(sys_vec, __pmap);
    }
  catch(ANTLR_USE_NAMESPACE(antlr)RecognitionException& re)
    {
      cerr << "Error: Parsing Exception: " << re.toString() << endl;
    }
  
  infile.close();
  delete parser;

  return(0);

}

int main(int argc, char* argv[])
{

  int ret_val = 0;


  signal(SIGSEGV, Handle_Segfault);


  if(argc < 2)
    {
	Usage_hierSysPartition();
      	exit(1);
    }

  string opt_string;
  set<string> hw_instances;
  while ((opt = getopt_long(argc, argv, "i:", long_options, &option_index)) != -1) {
	switch(opt) {
		case 'i': 
			opt_string = optarg;
			cerr << "Info: include instance " << opt_string  << " on HW side." << endl;
			hw_instances.insert(opt_string);
			break;
			break;
		default: cerr << "Error: unknown option " << opt << endl; ret_val = 1; break;
	}
    }

  vector<hierSystem*> sys_vec;
  for(int I = optind;  I < argc; I++) {
  	string filename = argv[I];
  	int pstat = Parse(filename, sys_vec);
  	if(pstat) {
		cerr << "Error: in parsing file " << filename << endl;
		ret_val = 1;
	}
  }
      
  if(ret_val) return(ret_val);

  for(int I = 0, fI = sys_vec.size(); I < fI; I++) {
	hierSystem* sys = sys_vec[I];
	if(sys->Get_Error() || sys->Check_For_Errors()) {
		cerr << "Error: in building system " << sys->Get_Id() << endl;
		ret_val = 1;	
	}
  }

  hierSystem* top_sys = sys_vec[sys_vec.size() - 1];
  hierInstanceGraph* g = NULL;
  top_sys->Build_Instance_Hierarchy(&g);

  assert (g != NULL);
  g->Build_Connectivity();
  g->Print(cout);

  map<hierPipeInstance*, hierPipeInstance*> root_pipe_map;
  g->Set_Root_Pipes(root_pipe_map);

  FlatLeafGraph* fg = NULL;
  g->Build_Flat_Leaf_Graph(&fg);
  assert(fg != NULL);
  fg->Print(cout);


  if(hw_instances.size() > 0)
  {
	FlatLeafGraph* sw_graph = NULL;
	FlatLeafGraph* hw_graph = NULL;

	top_sys->Partition_Flat_Graph(fg, hw_instances, &sw_graph, &hw_graph);

	cout << "Info: software-side graph" << endl;
	sw_graph->Print(cout);

	string sw_hsys_filename = "__SW_SYS.HSYS";	
        ofstream sw_outfile; 
	sw_outfile.open(sw_hsys_filename.c_str());
	sw_graph->Print_Hsys_File("sys", "work", sw_outfile);
	sw_outfile.close();

	cout << "Info: hardware-side graph" << endl;
	hw_graph->Print(cout);

	string hw_hsys_filename = "__HW_SYS.HSYS";	
        ofstream hw_outfile; 
	hw_outfile.open(hw_hsys_filename.c_str());
	hw_graph->Print_Hsys_File("sys", "work", hw_outfile);
	hw_outfile.close();

	string pc_filename = "__PIPE_CLASSIFICATION.TXT";	
	ofstream pc_file;
	pc_file.open(pc_filename.c_str());
	fg->Print_Pipe_Classifications(hw_instances,pc_file);
	pc_file.close();

	
  }

  return(ret_val);
}
