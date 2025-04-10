
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <TGApplication.h>

#include "hdview2.h"
#include "MyProcessor.h"

int GO = 0; // 1=continuously display events 0=wait for user
bool PRINT_FACTORY_LIST = false;
bool SKIP_EPICS_EVENTS = true;
std::vector< std::string> REQUIRED_CLASSES_FOR_DRAWING;
REQUIRED_CLASSES_LOGIC_t REQUIRED_CLASSES_LOGIC=REQUIRED_CLASSES_LOGIC_OR;


TCanvas *maincanvas=NULL;
extern JApplication *japp;
JEvent *jevent =NULL;
MyProcessor *myproc = NULL;

int32_t RUNNUMBER = 9999; // set with RUNNUMBER config paramter

void PrintFactoryList(JApplication *app);
void ParseCommandLineArguments(int &narg, char *argv[]);
void Usage();

//-------------------
// main
//-------------------
int main(int narg, char *argv[])
{
	// Parse the command line arguments
	ParseCommandLineArguments(narg, argv);
	
	// Instantiate a DApplication object. This has to be done BEFORE
	// creating the TApplication object since that modifies the argument list.
	DApplication dapp(narg, argv);
	japp = dapp.GetJApp();
	
	// Check if user specified a run number via config. parameter
    RUNNUMBER = japp->RegisterParameter("RUNNUMBER", 0);
	
	// Create a ROOT TApplication object
	TApplication app("HDView", &narg, argv);
	
	// This is done AFTER creating the TApplication object so when the
	// init routine is called, the window will be mapped and it can
	// draw the detectors.
	myproc = new MyProcessor();
	japp->Add(myproc);

	// If the PRINT_FACTORY_LIST flag was set, then print the factory list
	if(PRINT_FACTORY_LIST) {
        PrintFactoryList(japp);
    }

    // Start JANA2 run in nonblocking (unsupervised) mode
	japp->Run(false,false);

    // Run ROOT UI, blocking until UI event loop finishes
	app.SetReturnFromRun(true);
	app.Run(); 

    // Shut down JANA2 processing
    japp->Stop(true, true);
    int exitcode = japp->GetExitCode();
	delete japp;
	return exitcode;
}

//-----------
// PrintFactoryList
//-----------
void PrintFactoryList(JApplication *app)
{
	// When we get here, the Run() method hasn't been
	// called so the JEventLoop objects haven't
	// been created yet and cansequently the factory objects
	// don't yet exist. Since we want the "list factories"
	// option to work even without an input file, we need
	// to first make the factories before we can list them.
	// To do this we only need to instantiate a JEventLoop object
	// passing it our "app" pointer. The JEventLoop will automatically
	// register itself with the DApplication and the factories
	// will be made, even ones from plugins passed on the command
	// line.
	
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
			case 'L':
				PRINT_FACTORY_LIST = true;
				//PrintFactoryList(japp);
				break;
			case 'D':
			{
				stringstream s_stream(&(argv[i][2])); //create string stream from the string
				while(s_stream.good()) {
					string substr;
					getline(s_stream, substr, ','); //get first string delimited by comma
					REQUIRED_CLASSES_FOR_DRAWING.push_back(substr);
				}				
				break;
			}
			case 'A':
				REQUIRED_CLASSES_LOGIC=REQUIRED_CLASSES_LOGIC_AND;
				break;
		}
	}
	
	// Check if DISPLAY environment variable is set and warn user if not.
	const char *DISPLAY = getenv("DISPLAY");
	if(DISPLAY == NULL){
		jerr << endl;
		jerr << " WARNING: You do not appear to have your DISPLAY environment" << endl;
		jerr << "          variable set. This may prevent the graphics window" << endl;
		jerr << "          from opening or even cause a seg. fault. Consider" << endl;
		jerr << "          setting this to something like \"localhost:0\" if" << endl;
		jerr << "          you have trouble opening the display." << endl;
		jerr << endl;
	}
}

//-----------
// Usage
//-----------
void Usage()
{
	cout<<"Usage:"<<endl;
	cout<<"       hdview2 [options] source1 source2 ..."<<endl;
	cout<<endl;
	cout<<"Print the contents of a Hall-D data source (e.g. a file)"<<endl;
	cout<<"to the screen."<<endl;
	cout<<endl;
	cout<<"Options:"<<endl;
	cout<<endl;
	cout<<"   -h        Print this message"<<endl;
	cout<<"   -L        List available factories and exit"<<endl;
	cout<<"   -Dname    Only draw events with at least one object of the data of type \"name\" (can be used multiple times. OR logic)"<<endl;
	cout<<"   -A        Use AND logic instead of OR for the classes listed in -D. (i.e. all of tham must be present rather than at least one)" << endl;
	cout<<endl;
	cout<<"JANA options:"<<endl;
	cout<<endl;

	jana::PrintUsage();

	exit(0);
}
