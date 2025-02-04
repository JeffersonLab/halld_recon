// Author: David Lawrence  June 25, 2004
//
//
// hd_ana.cc
//

#include <iostream>
using namespace std;

#include <termios.h>

#include <JANA/CLI/JMain.h>
#include "MyProcessor.h"
#include "DANA/DApplication.h"

void PrintFactoryList(JApplication *app);
void ParseCommandLineArguments(int &narg, char *argv[]);
void Usage();

bool LIST_FACTORIES = false;
bool SPARSIFY_SUMMARY = true;
bool ALLOW_SPARSIFIED_EVIO = true;

//-----------
// main
//-----------
int main(int narg, char *argv[])
{
	// Parse the command line
	ParseCommandLineArguments(narg, argv);

	auto options = jana::ParseCommandLineOptions(narg, argv);
	DApplication dapp(narg, argv);
	JApplication* app = dapp.GetJApp();

	// Set tag prefix for JANA streams to empty
	jout.SetTag("");
	
	// If LIST_FACTORIES is set, print all factories and exit
	if(LIST_FACTORIES){
		// TODO: This should be in JANA, not here
		PrintFactoryList(app);
		return 0;
	}

	// This monkeyshines is needed to get getchar() to return single
	// characters without waiting for the user to hit return
	struct termios t;
	tcgetattr(fileno(stdin), &t);
	t.c_lflag &= (~ICANON);
	//t.c_cc[VMIN] = 1;
	tcsetattr(fileno(stdin), TCSANOW, &t);

	// If only DEPICSvalue is specified to print, then automatically
	// turn on sparsified reading unless user specifically requests
	// that we don't.
	if(toprint.size()==1 && ALLOW_SPARSIFIED_EVIO){
		if( toprint.find("EPICSvalue") != toprint.end() ){
			cout << endl;
			cout << "-- Only DEPICSvalue objects requested" << endl;
			cout << "-- * Enabling EVIO file mapping and sparse readout" << endl;
			cout << "-- * Automatically invoking -f and -s options" << endl;
			cout << endl;
			app->SetParameterValue("EVIO:SPARSE_READ", true);
			app->SetParameterValue("EVIO:EVENT_MASK", string("EPICS"));
			PRINT_SUMMARY_HEADER = false;
			SKIP_BORING_EVENTS = 1;
		}
	}

	app->SetTicker(false); // Disable ticker
        app->SetTimeoutEnabled(false);

	app->Add(new MyProcessor);

	// Run JANA
	auto exitCode = jana::Execute(app, options);
	// delete app; TODO: RR Look at https://shorturl.at/bcoV9 for details of this error and figure out the real reason!

	if( exitCode ) cerr << "Exit code: " << exitCode << endl;
	return exitCode;
}

//-----------
// PrintFactoryList
//-----------
void PrintFactoryList(JApplication *app)
{
	// When we get here, the Run() method hasn't been
	// called so the JEventLoop objects haven't
	// been created yet and consequently the factory objects
	// don't yet exist. Since we want the "list factories"
	// option to work even without an input file, we need
	// to first make the factories before we can list them.
	// To do this we only need to instantiate a JEventLoop object
	// passing it our "app" pointer. The JEventLoop will automatically
	// register itself with the DApplication and the factories
	// will be made, even ones from plugins passed on the command
	// line.
	// TODO: This should be in JANA, not here
	app->Initialize();

	// Print header
	cout<<endl;
	cout<<"  Factory List"<<endl;
	cout<<"-------------------------"<<endl;


	// Get list of factories and print out the data type and tags
	// Printing out the data types and tags.
	auto factory_summaries = app->GetComponentSummary().factories;
	for (const auto &factory_summary : factory_summaries) {
		cout << " " << factory_summary.object_name;
		if(factory_summary.factory_tag != "") {
			cout << " : " << factory_summary.factory_tag;
		}
		cout << endl;
	}

	cout<<endl;
	cout<<" "<<factory_summaries.size()<<" factories registered"<<endl;
	cout<<endl;
}

//-----------
// ParseCommandLineArguments
//-----------
void ParseCommandLineArguments(int &narg, char *argv[])
{
	if(narg==1)Usage();

	for(int i=1;i<narg;i++){
		if(argv[i][0] != '-')continue;
		switch(argv[i][1]){
			case 'h':
				Usage();
				break;
			case 'D':
				toprint.insert(&argv[i][2]);
				break;
     		        case 'q':
		                QUIT_AFTER_FINDING_NTH = true;
				SKIP_BORING_EVENTS = true;
				PAUSE_BETWEEN_EVENTS = false;
				if (strlen(argv[i])>2) {  // N_TO_FIND defaults to 0(=all events) if not specified
				  long int number_to_find = strtol(&argv[i][2],NULL,10);
				  N_TO_FIND = (int)number_to_find;
				}
				break;
			case 'p':
				PAUSE_BETWEEN_EVENTS = false;
				break;
			case 's':
				SKIP_BORING_EVENTS = true;
				break;
			case 'A':
				PRINT_ALL = true;
				break;
			case 'c':
				PRINT_CORE = true;
				PRINT_SUMMARY_HEADER = true;
				break;
			case 'L':
				LIST_FACTORIES = true;
				break;
			case 'a':
				LIST_ASSOCIATED_OBJECTS = true;
				break;
			case 'S':
				SPARSIFY_SUMMARY = false;
				break;
			case 'f':
				cout << "WARNING: -f option is deprecated as it is now the default" << endl;
				break;
			case 'V':
				PRINT_SUMMARY_ALL    = true;
				PRINT_SUMMARY_HEADER = true;
				break;
			case 'b':
				PRINT_STATUS_BITS = true;
				break;
			case 'e':
				ALLOW_SPARSIFIED_EVIO = false;
				break;
		}
	}
	
	if(toprint.empty() && !PRINT_ALL ){
		PRINT_CORE = true;
		PRINT_SUMMARY_HEADER = true;
	}
}

//-----------
// Usage
//-----------
void Usage()
{
	cout<<"Usage:"<<endl;
	cout<<"       hd_dump [options] source1 source2 ..."<<endl;
	cout<<endl;
	cout<<"Print the contents of a Hall-D data source (e.g. a file)"<<endl;
	cout<<"to the screen."<<endl;
	cout<<endl;
	cout<<"Options:"<<endl;
	cout<<endl;
	cout<<"   -h        Print this message"<<endl;
	cout<<"   -Dname    Print the data of type \"name\" (can be used multiple times)"<<endl;
	cout<<"   -A        Print ALL data types (overrides -s and -DXXX options)"<<endl;
	cout<<"   -L        List available factories and exit"<<endl;
	cout<<"   -p        Don't pause for keystroke between events (def. is to pause)"<<endl;
	cout<<"   -s        Skip events which don't have any of the specified data types"<<endl;
	cout<<"   -qn       Quit after printing n events (default n=1), skip boring events, and don't pause"<<endl;
	cout<<"   -a        List types and number of associated objects"<<endl;
	cout<<"   -S        Don't suppress printing of factories with no objects in summary"<<endl;
	cout<<"   -c        Print summary header lisiting for select factories."<<endl;
	cout<<"   -V        Print summary header lisiting for all factories."<<endl;
	cout<<"             (warning: this activates every single factory!)"<<endl;
	cout<<"   -b        Print event status bits"<<endl;
	cout<<"   -e        Don't allow automatic EVIO sparse readout for EPICS data"<<endl;
	cout<<endl;

	jana::PrintUsage();
	exit(0);
}


