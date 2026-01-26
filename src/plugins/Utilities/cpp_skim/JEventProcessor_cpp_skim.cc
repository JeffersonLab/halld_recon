// $Id$
//
//    File: JEventProcessor_cpp_skim.cc
// Created: Tue Apr 16 10:14:04 EDT 2024
// Creator: ilarin (on Linux ifarm1802.jlab.org 3.10.0-1160.102.1.el7.x86_64 x86_64)
//

#include "JEventProcessor_cpp_skim.h"
#include <TMath.h>

// Routine used to create our JEventProcessor
#include <JANA/JApplication.h>
#include <JANA/JFactoryT.h>
#include "DANA/DEvent.h"


extern "C"{
void InitPlugin(JApplication *app){
	InitJANAPlugin(app);
	app->Add(new JEventProcessor_cpp_skim());
}
} // "C"


//------------------
// JEventProcessor_cpp_skim (Constructor)
//------------------
JEventProcessor_cpp_skim::JEventProcessor_cpp_skim()
{

}

//------------------
// ~JEventProcessor_cpp_skim (Destructor)
//------------------
JEventProcessor_cpp_skim::~JEventProcessor_cpp_skim()
{

}

//------------------
// Init
//------------------
void JEventProcessor_cpp_skim::Init(void)
{
	// This is called once at program startup. 

	return; //NOERROR;
}

//------------------
// BeginRun
//------------------
void JEventProcessor_cpp_skim::BeginRun(const std::shared_ptr<const JEvent>& event)
{
	// This is called whenever the run number changes
  num_epics_events = 0;
	return; //NOERROR;
}

//------------------
// Process
//------------------
void JEventProcessor_cpp_skim::Process(const std::shared_ptr<const JEvent>& event)
{
	// This is called for every event. Use of common resources like writing
	// to a file or filling a histogram should be mutex protected. Using
	// loop->Get(...) to get reconstructed objects (and thereby activating the
	// reconstruction algorithm) should be done outside of any mutex lock
	// since multiple threads may call this method at the same time.
	// Here's an example:
	//
	// vector<const MyDataClass*> mydataclasses;
	// loop->Get(mydataclasses);
	//
	// japp->RootFillLock(this);
	//  ... fill historgrams or trees ...
	// japp->RootFillUnLock(this);


  const DEventWriterEVIO* locEventWriterEVIO = NULL;
  event->GetSingle(locEventWriterEVIO);
  if(locEventWriterEVIO == NULL) {
    cerr << "from JEventProcessor_cpp_skim: locEventWriterEVIO is not available" << endl;
    exit(1);
  }

  if(GetStatusBit(event, kSTATUS_BOR_EVENT)) { // Begin of Run event
    locEventWriterEVIO->Write_EVIOEvent(event,"cpp_2c");
    return; //NOERROR;
  }

  if(GetStatusBit(event, kSTATUS_EPICS_EVENT)) { // Epics event
    if(num_epics_events<5) locEventWriterEVIO->Write_EVIOEvent(event,"cpp_2c");
     ++num_epics_events;
    return; //NOERROR;
  }


	vector<const DTrackWireBased*> tracks;
	event->Get(tracks);

  int npos = 0, nneg = 0;
	for(unsigned int j=0; j<tracks.size(); j++) {
		const DTrackWireBased *track = tracks[j];
    double p = track->momentum().Mag();
    DVector3 origin =  track->position();
    if(p<0.2 || p>15.0 || fabs(origin.Z())>90. || origin.Pt()>2.7) continue;

    double chi  = track->chisq;
    int ndof = track->Ndof;
    double prob = TMath::Prob(chi,ndof);
    if(prob<1.e-3) continue;

    double q = track->charge();
    if(q> 0.1) ++npos;
    if(q<-0.1) ++nneg;
  }

  if(npos && nneg) locEventWriterEVIO->Write_EVIOEvent(event,"cpp_2c");

	return; //NOERROR;
}

//------------------
// EndRun
//------------------
void JEventProcessor_cpp_skim::EndRun(void)
{
	// This is called whenever the run number changes, before it is
	// changed to give you a chance to clean up before processing
	// events from the next run number.
	return; //NOERROR;
}

//------------------
// Finish
//------------------
void JEventProcessor_cpp_skim::Finish(void)
{
	// Called before program exit after event processing is finished.
	return; //NOERROR;
}

