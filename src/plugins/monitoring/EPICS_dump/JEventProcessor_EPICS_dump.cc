// $Id$
//
//    File: JEventProcessor_EPICS_dump.cc
// Created: Fri Nov  9 11:58:09 EST 2012
// Creator: wolin (on Linux stan.jlab.org 2.6.32-279.11.1.el6.x86_64 x86_64)


#include <stdint.h>
#include <vector>
#include <deque>
#include <string>
#include <iostream>
#include <algorithm>
#include <stdio.h>
#include <stdlib.h>

#include "TH2I.h"

#include "JEventProcessor_EPICS_dump.h"

#include "TRIGGER/DL1Trigger.h"
#include <DANA/DEvent.h>
#include <DANA/DStatusBits.h>
#include "DAQ/DEPICSvalue.h"

using namespace std;

#include <TDirectory.h>
#include <TH1.h>


// root hist pointers
static TH1I* h1epics_AD00 = NULL;
static TH2I* h2epics_pos_inner = NULL;
static TH2I* h2epics_pos_outer = NULL;
static TH1F* h1epics_AD00_VSevent = NULL;
static TH1F* h1epics_entries_VSevent = NULL;

//----------------------------------------------------------------------------------


// Routine used to create our JEventProcessor
extern "C"{
  void InitPlugin(JApplication *locApplication){
    InitJANAPlugin(locApplication);
    locApplication->Add(new JEventProcessor_EPICS_dump());
  }
}


//----------------------------------------------------------------------------------


JEventProcessor_EPICS_dump::JEventProcessor_EPICS_dump() {
	SetTypeName("JEventProcessor_EPICS_dump");
}


//----------------------------------------------------------------------------------


JEventProcessor_EPICS_dump::~JEventProcessor_EPICS_dump() {
}


//----------------------------------------------------------------------------------

void JEventProcessor_EPICS_dump::Init() {

	auto app = GetApplication();
	lockService = app->GetService<JLockService>();

	// First thread to get here makes all histograms. If one pointer is
	// already not NULL, assume all histograms are defined and return now
	if(h1epics_AD00 != NULL){
		return;
	}
	
	// create root folder for trig and cd to it, store main dir
	TDirectory *main = gDirectory;
	gDirectory->mkdir("EPICS_dump")->cd();
	
	// book hist
	int const nbins=100;
	
	h1epics_AD00 = new TH1I("h1epics_AD00", "Current AD00",nbins,0,500);
	h1epics_AD00->SetXTitle("Current AD00 (nA)");
	h1epics_AD00->SetYTitle("counts");
	
	h2epics_pos_inner = new TH2I("h1epics_pos_inner", "Position AC inner",nbins,-10,10,nbins,-10,10);
	h2epics_pos_inner->SetXTitle("Position AC inner x (mm)");
	h2epics_pos_inner->SetYTitle("Position AC inner y (mm)");
	h2epics_pos_outer = new TH2I("h1epics_pos_outer", "Position AC outer",nbins,-20,20,nbins,-20,20);
	h2epics_pos_outer->SetXTitle("Position AC outer x (mm)");
	h2epics_pos_outer->SetYTitle("Position AC outer y (mm)");
	
        h1epics_AD00_VSevent = new TH1F("h1epics_AD00_VSevent", "Current AD00 (nA)",200,0,2e6);
	h1epics_AD00_VSevent->SetXTitle("Event number");
	h1epics_AD00_VSevent->SetYTitle("Current (nA)");

	h1epics_entries_VSevent = new TH1F("h1epics_entries_VSevent", "No epics events/bin",200,0,2e6);
	h1epics_entries_VSevent->SetXTitle("Event number");
	h1epics_entries_VSevent->SetYTitle("No of epics events");
	
	// back to main dir
	main->cd();
	
	return;
}


//----------------------------------------------------------------------------------


void JEventProcessor_EPICS_dump::BeginRun(const std::shared_ptr<const JEvent>& t) {
  // This is called whenever the run number changes
  return;
}


//----------------------------------------------------------------------------------


void JEventProcessor_EPICS_dump::Process(const std::shared_ptr<const JEvent> &locEvent) {
	// This is called for every event. Use of common resources like writing
	// to a file or filling a histogram should be mutex protected. Using
	// loop-Get(...) to get reconstructed objects (and thereby activating the
	// reconstruction algorithm) should be done outside of any mutex lock
	// since multiple threads may call this method at the same time.

	auto locEventNumber = locEvent->GetEventNumber();
	
	vector<const DEPICSvalue*> epicsvalues;
	locEvent->Get(epicsvalues);	

	bool isEPICS = GetStatusBit(locEvent, kSTATUS_EPICS_EVENT);
	if(!isEPICS)
	  return;
	
	// FILL HISTOGRAMS
	// Since we are filling histograms local to this plugin, it will not interfere with other ROOT operations: can use plugin-wide ROOT fill lock
	lockService->RootWriteLock(); //ACQUIRE ROOT FILL LOCK
	
	// process EPICS records
	//printf (" Event=%d is an EPICS record\n",(int)locEventNumber);
	
	// read in whatever epics values are in this event
	
	// save their values
	float pos_default=-1000.;
	float xpos_inner = pos_default;
	float ypos_inner = pos_default;;
	float xpos_outer = pos_default;
	float ypos_outer = pos_default;
	for(vector<const DEPICSvalue*>::const_iterator val_itr = epicsvalues.begin();
	    val_itr != epicsvalues.end(); val_itr++) {
		const DEPICSvalue* epics_val = *val_itr;
		//cout << "EPICS:  " << epics_val->name << " = " << epics_val->sval << endl;
		float fconv = atof(epics_val->sval.c_str());
		unsigned int iconv = atoi(epics_val->sval.c_str());
		bool isDigit = epics_val->name.length()> 12 && isdigit(epics_val->name[12]);
		// cout << "isDigit=" << isDigit << " string=" << epics_val->name << endl;
		if ((epics_val->name.substr(0,11) == "BCAL:pulser") & isdigit(epics_val->name[11])) {
			val_itr++;   // increment counter and get low order word
			epics_val = *val_itr;
			//cout << "EPICS:  " << epics_val->name << " = " << epics_val->sval << endl;
			unsigned int iconv_low = atoi(epics_val->sval.c_str());
			// cout << "BCAL:pulser status=" << epics_val->name.substr(0,11) << endl;
			// printf ("BCAL:pulser iconv=%d, %0X, iconv_low=%d %0X\n",iconv,iconv,iconv_low,iconv_low);
			iconv_low = iconv_low >> 31;
			iconv  = ((iconv <<1) & 0XFFFF) + iconv_low;
			// cout << "BCAL:pulser status=" << epics_val->name.substr(0,11) << endl;
			//printf ("BCAL:pulser status=%d, %0X, iconv_low=%0X\n",iconv,iconv,iconv_low);
		}     
		else if ((epics_val->name.substr(0,11) == "BCAL:pulser") & isDigit) {
			//double freq = 1.e8/fconv;  // cover to s: period is in units 10 ns
			//cout << "BCAL:pulser=" << epics_val->name.substr(0,11) << epics_val->fval <<  " freq=" << freq <<endl;
		}
		else if (epics_val->name == "IBCAD00CRCUR6") {
			h1epics_AD00->Fill(fconv);
			h1epics_AD00_VSevent->Fill((float)locEventNumber,fconv);
			h1epics_entries_VSevent->Fill((float)locEventNumber);
			//cout << "IBCAD00CRCUR6 " << epics_val->name << " fconv=" << fconv << endl; 
		}
		else if (epics_val->name == "AC:inner:position:x") {
			xpos_inner = fconv;
	        }  
		else if (epics_val->name == "AC:inner:position:y") {
			ypos_inner = fconv;
	        }  
		else if (epics_val->name == "AC:outer:position:x") {
			xpos_outer = fconv;
	        }  
		else if (epics_val->name == "AC:outer:position:y") {
			ypos_outer = fconv;
	        }  
	}
	if (xpos_inner> pos_default && ypos_inner > pos_default) { 
		h2epics_pos_inner->Fill(xpos_inner,ypos_inner);
	}
	if (xpos_outer> pos_default && ypos_outer > pos_default) { 
		h2epics_pos_outer->Fill(xpos_outer,ypos_outer);
	}
	
	lockService->RootUnLock(); //RELEASE ROOT FILL LOCK
	
	return;
}


//----------------------------------------------------------------------------------


void JEventProcessor_EPICS_dump::EndRun() {
  // This is called whenever the run number changes, before it is
  // changed to give you a chance to clean up before processing
  // events from the next run number.
  return;
}


//----------------------------------------------------------------------------------


void JEventProcessor_EPICS_dump::Finish() {
  // Called before program exit after event processing is finished.
  return;
}


//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
