// Author: David Lawrence  June 25, 2004
//
//
// hd_ana.cc
//

#include "MyProcessor.h"
#include <DANA/DApplication.h>
#include <iostream>

//-----------
// main
//-----------
int main(int narg, char *argv[])
{
	// Instantiate our event processor
	MyProcessor myproc;

	// Instantiate an event loop object
	DApplication dapp(narg, argv);
	JApplication* app = dapp.GetJApp();

	// Run though all events, calling our event processor's methods
	auto exitCode = jana::Execute(app, dapp.GetUserOptions());

	if( exitCode ) std::cerr << "Exit code: " << exitCode << std::endl;
	return exitCode;
}

