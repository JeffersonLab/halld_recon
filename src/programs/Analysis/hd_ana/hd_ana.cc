// Author: David Lawrence  June 25, 2004
//
//
// hd_ana.cc
//

#include "DANA/DApplication.h"
using namespace std;

void Usage();


//-----------
// main
//-----------
int main(int narg, char *argv[])
{
	// Instantiate an event loop object
	DApplication dapp(narg, argv);
	JApplication* app = dapp.GetJApp();

	if(narg<=1) Usage();

	// Run though all events, calling our event processor's methods
	auto exitCode = jana::Execute(app, dapp.GetUserOptions());

	if( exitCode ) cerr << "Exit code: " << exitCode << endl;
	return exitCode;
}

//-----------
// Usage
//-----------
void Usage()
{
	cout<<endl;
	cout<<"Usage:"<<endl;
	cout<<"    hd_ana [options] source1 source2 source3 ..."<<endl;
	cout<<endl;
	jana::PrintUsage();
	cout<<endl;
	
	exit(0);
}

