//
// JEventProcessor_2trackskim.cc
//
// JANA event processor plugin to skim 2-track events to an EVIO file
//
// Paul Mattione, 19-November-2014

#include "JEventProcessor_2trackskim.h"

// for initializing plugins
extern "C" {
   void InitPlugin(JApplication *app)
	{
		InitJANAPlugin(app);
		app->Add(new JEventProcessor_2trackskim());
   }
} // "extern C"

//-------------------------------
// Init
//-------------------------------
void JEventProcessor_2trackskim::Init()
{
}

//-------------------------------
// BeginRun
//-------------------------------
void JEventProcessor_2trackskim::BeginRun(const std::shared_ptr<const JEvent>& event)
{
}

//-------------------------------
// Process
//-------------------------------
void JEventProcessor_2trackskim::Process(const std::shared_ptr<const JEvent>& event)
{

	const DEventWriterEVIO* locEventWriterEVIO = NULL;
	event->GetSingle(locEventWriterEVIO);

	//Save EPICS events
	vector<const DEPICSvalue*> locEPICSValues;
	event->Get(locEPICSValues);
	if(!locEPICSValues.empty())
	{
		locEventWriterEVIO->Write_EVIOEvent(event, "2tracks");
		return;
	}

	vector<const DChargedTrack*> locChargedTracks;
	event->Get(locChargedTracks, "PreSelect");
	if(locChargedTracks.size() >= 2)
	{
		locEventWriterEVIO->Write_EVIOEvent(event, "2tracks");
		return;
	}
}

//-------------------------------
// EndRun
//-------------------------------
void JEventProcessor_2trackskim::EndRun()
{
}

//-------------------------------
// Finish
//-------------------------------
void JEventProcessor_2trackskim::Finish()
{
}

