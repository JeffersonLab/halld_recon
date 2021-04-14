// This file was copied and modified from the RootSpy source. It implements
// routines used by root macros such as resetting histograms.
//
//    File: rs_macroutils.cc
// Created: Sat Oct  7 10:14:30 EDT 2017
// Creator: davidl (on Darwin harriet 15.6.0 Darwin Kernel Version 15.6.0: Tue Apr 11 16:00:51 PDT 2017; root:xnu-3248.60.11.5.3~1/RELEASE_X86_64 x86_64)

// This file contains some global scope routines that are made
// available to macros. They allow the macros to interact with
// the RootSpy client in various ways that extend their functionality.

#include <iostream>
#include <map>
#include <thread>
#include <set>
using namespace std;

#include "rs_macroutils.h"

static map<string, int> rs_flags;

std::map<string, set<int> > rs_PadsToSave;  // key=macro basename  val=Tpad numbers to save (0=whole canvas)

// Calls from macros to have rootspy reset a histogram will simply
// add them to this list. The hd_rsai_gen will then reset them
// after the macros are run.
std::set<std::string> hnamepaths_to_reset;
std::set<std::string> macros_to_reset;
std::map<std::string, std::string> MACROS;

//...................................................
// REGISTER_ROOTSPY_MACRO
//
// Plugins built with sbms can automatically generate C++
// code that includes a macro as a string. To make that
// automatically to RootSpy, a dedicated class declaration
// is added to the bottom of the generated file and a
// static global variable of that type declared. This causes
// the constructor of that class to be called when the plugin
// is attached. The constructor looks for a symbol in called
// REGISTER_ROOTSPY_MACRO which, if found, it calls. We define
// that here so compiled in macros can be automatically found.
extern "C"{
void REGISTER_ROOTSPY_MACRO(string name, string path, string macro_data){

	std::cout << "Registering macro: " << name << std::endl;
	MACROS[name] = macro_data;
}
};

//-------------------
// rs_SetFlag
//
// Set a named flag that can be accessed by macros
//-------------------
void rs_SetFlag(const string flag, int val)
{
	rs_flags[flag] = val;
}

//-------------------
// rs_GetFlag
//
// Get a named flag and return it as an int. 
//-------------------
int rs_GetFlag(const string flag)
{
	if(rs_flags.count(flag)){
		return rs_flags[flag];
	}else{
		// cerr << "Unknown flag \"" << flag << "\" requested from macro!" << endl;
		return -1;
	}
}

//-------------------
// rs_ResetHisto
//
// Reset a histogram on the rootspy client. This
// is equivalent to pushing the "Reset" button
// on the RootSpy GUI program except this will
// only reset a single histogram. The histogram
// can be restored using re_RestoreHisto.
//-------------------
void rs_ResetHisto(const string hnamepath)
{
	// cout << "Scheduled reset of : " << hnamepath << endl;

	hnamepaths_to_reset.insert(hnamepath);
}

//-------------------
// rs_RestoreHisto
//
// Restore a histogram on the rootspy client. This
// is equivalent to pushing the "Restore" button
// on the RootSpy GUI program except this will
// only restore a single histogram. This reverses
// a previous call to re_ResetHisto.
//-------------------
void rs_RestoreHisto(const string hnamepath)
{
	cout << "Ignoring restoration of: " << hnamepath << endl;
}

//-------------------
// rs_ResetAllMacroHistos
//
// Reset all histograms associated with the specified
// macro. This is equivalent to pushing the "Reset"
// button on the RootSpy GUI program for each histogram
// a macro has specified in its comments. The histograms
// can be restored using re_RestoreAllMacroHistos.
//-------------------
void rs_ResetAllMacroHistos(const string hnamepath)
{
	cout << "Scheduled reset of all hists for macro : " << hnamepath << endl;

	macros_to_reset.insert(hnamepath);
}

//-------------------
// rs_RestoreAllMacroHistos
//
// Restore all histograms associated with the specified
// macro. This is equivalent to pushing the "Restore"
// button on the RootSpy GUI program for each histogram
// a macro has specified in its comments.
//-------------------
void rs_RestoreAllMacroHistos(const string hnamepath)
{
	cout << "Ignoring restoration of macro: " << hnamepath << endl;
}

//-------------------
// rs_SavePad
//
// This is used by macros to indicate that a specific pad of the
// current canvas should be saved to a file with the given basenames.
// This is used by RSAI to allow macros to indicate when an image
// should be saved and checked against an AI model.
//-------------------
void rs_SavePad(const string fname, int ipad)
{
	rs_PadsToSave[fname].insert( ipad );
}

