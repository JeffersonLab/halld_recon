//
// JEventProcessor_danarest.cc
//
// JANA event processor plugin writes out rest events to a file
//
// Richard Jones, 1-July-2012

#include "JEventProcessor_danarest.h"
#include <TRACKING/DMCThrown.h>

// Make us a plugin
// for initializing plugins
extern "C" {
   void InitPlugin(JApplication *app) {
      InitJANAPlugin(app);
      app->Add(new JEventProcessor_danarest());
   }
} // "extern C"

//-------------------------------
// Init
//-------------------------------
void JEventProcessor_danarest::Init()
{
}

//-------------------------------
// BeginRun
//-------------------------------
void JEventProcessor_danarest::BeginRun(const std::shared_ptr<const JEvent>& event)
{
}

//-------------------------------
// Process
//-------------------------------
void JEventProcessor_danarest::Process(const std::shared_ptr<const JEvent>& event)
{
  // Check if we have thrown events (therefore MC)
  vector<const DMCThrown *>throwns;
  event->Get(throwns);

	//CHECK TRIGGER TYPE
	const DTrigger* locTrigger = NULL;
	event->GetSingle(locTrigger);
	if(throwns.size()==0 && (!locTrigger->Get_IsPhysicsEvent()))
		return;

	// Write this event to the rest output stream.
	vector<const DEventWriterREST*> locEventWriterRESTVector;
	event->Get(locEventWriterRESTVector);
	locEventWriterRESTVector[0]->Write_RESTEvent(event, "");
}

//-------------------------------
// EndRun
//-------------------------------
void JEventProcessor_danarest::EndRun()
{
}

//-------------------------------
// Finish
//-------------------------------
void JEventProcessor_danarest::Finish()
{
}
