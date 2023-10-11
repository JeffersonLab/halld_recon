//
//
// This supplies the InitPlugin routine for the plugin. It
// registers a factory generator which in turn will create
// factories that shadow the digihit factories for data
// types we want to filter.
//

#include <iostream>
using namespace std;

#include <JANA/Compatibility/jerror.h>
#include <JANA/JFactoryGenerator.h>

#include "JFactoryGenerator_RSAI_KO.h"

static void Usage(void);

// Routine used to create our JEventProcessor
#include <JANA/JApplication.h>
extern "C"{
void InitPlugin(JApplication *app){
	InitJANAPlugin(app);
	app->Add(new JFactoryGenerator_RSAI_KO());

	bool show_help = false;
	app->SetDefaultParameter("KO:HELP", show_help, "Set this to any value to print the help message for the RSAI_KO plugin.");
	if( show_help ) Usage();
}
} // "C"

//------------------
// Usage
//
// This is called when KO:USAGE is set. It prints out some useful info about
// the RSAI_KO plugin.
//------------------
void Usage(void)
{
	cout << "--------------------------------------------------------" << endl;
	cout << "RSAI_KO Usage: " << endl;

	cout << "--------------------------------------------------------" << endl;
	japp->Quit();
}
