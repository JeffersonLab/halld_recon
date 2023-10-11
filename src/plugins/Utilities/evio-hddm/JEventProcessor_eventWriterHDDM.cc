// $Id$
//
//    File: JEventProcessor_eventWriterHDDM.cc
// Created: Mon Mar  6 14:11:50 EST 2017
// Creator: tbritton (on Linux halld03.jlab.org 3.10.0-514.6.1.el7.x86_64 x86_64)


#include "JEventProcessor_eventWriterHDDM.h"


// Routine used to create our JEventProcessor
extern "C"{
void InitPlugin(JApplication *app){
	InitJANAPlugin(app);
	app->Add(new JEventProcessor_eventWriterHDDM());
}
} // "C"


//------------------
// JEventProcessor_eventWriterHDDM (Constructor)
//------------------
JEventProcessor_eventWriterHDDM::JEventProcessor_eventWriterHDDM()
{
	SetTypeName("JEventProcessor_eventWriterHDDM");
}

//------------------
// ~JEventProcessor_eventWriterHDDM (Destructor)
//------------------
JEventProcessor_eventWriterHDDM::~JEventProcessor_eventWriterHDDM()
{

}

//------------------
// Init
//------------------
void JEventProcessor_eventWriterHDDM::Init()
{
	// This is called once at program startup. 
}

//------------------
// BeginRun
//------------------
void JEventProcessor_eventWriterHDDM::BeginRun(const std::shared_ptr<const JEvent>& event)
{
	// This is called whenever the run number changes
}

//------------------
// Process
//------------------
void JEventProcessor_eventWriterHDDM::Process(const std::shared_ptr<const JEvent>& event)
{
  // Write this event to the rest output stream.                                                                                                                                                             
    vector<const DEventWriterHDDM*> locEventWriterHDDMVector;
  event->Get(locEventWriterHDDMVector);
  locEventWriterHDDMVector[0]->Write_HDDMEvent(event, "");
  //  std::cout<<"done"<<std::endl;
}

//------------------
// EndRun
//------------------
void JEventProcessor_eventWriterHDDM::EndRun()
{
	// This is called whenever the run number changes, before it is
	// changed to give you a chance to clean up before processing
	// events from the next run number.
}

//------------------
// Finish
//------------------
void JEventProcessor_eventWriterHDDM::Finish()
{
	// Called before program exit after event processing is finished.
}

