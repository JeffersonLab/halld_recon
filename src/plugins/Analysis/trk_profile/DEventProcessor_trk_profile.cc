// $Id$
//
//    File: DEventProcessor_trk_profile.cc
// Created: Wed Jan 12 08:02:32 EST 2011
// Creator: davidl (on Darwin eleanor.jlab.org 10.6.0 i386)
//

#include "DEventProcessor_trk_profile.h"
#include <TRACKING/DTrackFitter.h>
#include <PID/DChargedTrack.h>


const DTrackFitter *fitter = NULL;

// Routine used to create our DEventProcessor
#include <JANA/JApplication.h>
extern "C"{
void InitPlugin(JApplication *app){
	InitJANAPlugin(app);
	app->Add(new DEventProcessor_trk_profile());
}
} // "C"


//------------------
// DEventProcessor_trk_profile (Constructor)
//------------------
DEventProcessor_trk_profile::DEventProcessor_trk_profile()
{

}

//------------------
// ~DEventProcessor_trk_profile (Destructor)
//------------------
DEventProcessor_trk_profile::~DEventProcessor_trk_profile()
{

}

//------------------
// Init
//------------------
void DEventProcessor_trk_profile::Init()
{
	// Create histograms here
	return;
}

//------------------
// BeginRun
//------------------
void DEventProcessor_trk_profile::BeginRun(const std::shared_ptr<const JEvent>& event)
{
	return;
}

//------------------
// Process
//------------------
void DEventProcessor_trk_profile::Process(const std::shared_ptr<const JEvent>& event)
{
	// Get DChargedTrack objects to activate tracking
	vector<const DChargedTrack*> charged_tracks;
	event->Get(charged_tracks);

	// Get the track fitter object for use later
	if(!fitter)event->GetSingle(fitter);

	return;
}

//------------------
// EndRun
//------------------
void DEventProcessor_trk_profile::EndRun()
{
	if(fitter){
		map<string, prof_time::time_diffs> prof_times;
		fitter->GetProfilingTimes(prof_times);
		
		double Ntracks = prof_times["Ntracks"].real; // a special entry keeps count of tracks in "real" slot
		
		cout<<endl;
		cout<<"Printing profiling info for track fitter ---("<<Ntracks<<" tracks)---"<<endl;
		map<string, prof_time::time_diffs>::iterator iter = prof_times.begin();
		for(; iter!=prof_times.end(); iter++){
			if(iter->first == "Ntracks")continue; // skip Ntracks which is special
			cout<< " "<<iter->first<<endl
				<<"   real="<< iter->second.real/Ntracks*1000.0<<"ms"<<endl
				<<"   prof="<< iter->second.prof/Ntracks*1000.0<<"ms"<<endl
				<<"   virt="<< iter->second.virt/Ntracks*1000.0<<"ms"<<endl
				<< endl;
		}
		cout<<endl;
	}

	return;
}

//------------------
// Finish
//------------------
void DEventProcessor_trk_profile::Finish()
{

	return;
}

