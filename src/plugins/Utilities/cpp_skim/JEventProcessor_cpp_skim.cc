// $Id$
//
//    File: JEventProcessor_cpp_skim.cc
// Created: Tue Apr 16 10:14:04 EDT 2024
// Creator: ilarin (on Linux ifarm1802.jlab.org 3.10.0-1160.102.1.el7.x86_64 x86_64)
//

#include "JEventProcessor_cpp_skim.h"


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

  const DEventWriterEVIO* locEventWriterEVIO = NULL;
  event->GetSingle(locEventWriterEVIO);
  if(locEventWriterEVIO == NULL) throw JException("JEventProcessor_cpp_skim: locEventWriterEVIO is not availabl");

  if(GetStatusBit(event, kSTATUS_BOR_EVENT)) { // Begin of Run event
    locEventWriterEVIO->Write_EVIOEvent(event,"cpp_2c");
    return;
  }

  if(GetStatusBit(event, kSTATUS_EPICS_EVENT)) { // Epics event
    if(num_epics_events<5) locEventWriterEVIO->Write_EVIOEvent(event,"cpp_2c");
     ++num_epics_events;
    return;
  }

	vector<const DTrackTimeBased*> tracks;
	event->Get(tracks);

  int npos = 0, nneg = 0;
	for(unsigned int j=0; j<tracks.size(); j++) {
		const DTrackTimeBased *track = tracks[j];
    double p = track->momentum().Mag();
    DVector3 origin =  track->position();
    if(p<0.2 || p>15.0 || fabs(origin.Z()-1.0)>90.) continue;

    double q = track->charge();
    if(q> 0.1) ++npos;
    if(q<-0.1) ++nneg;
  }

  if(npos && nneg) locEventWriterEVIO->Write_EVIOEvent(event,"cpp_2c");

	return;
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

