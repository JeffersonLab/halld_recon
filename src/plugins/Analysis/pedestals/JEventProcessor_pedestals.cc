// $Id$
//
//    File: JEventProcessor_pedestals.cc
// Created: Fri Jun 20 22:13:58 EDT 2014
// Creator: davidl (on Darwin harriet.local 13.2.0 i386)
//

#include <TDirectoryFile.h>


#include "JEventProcessor_pedestals.h"
using namespace std;


#include <DAQ/Df250PulseData.h>
#include <DAQ/Df250PulseIntegral.h>
#include <DAQ/Df125PulseIntegral.h>
#include <DAQ/DF1TDCHit.h>
#include <DAQ/DModuleType.h>
#include <DANA/DEvent.h>

// Routine used to create our JEventProcessor
#include <JANA/JFactoryT.h>
extern "C"{
void InitPlugin(JApplication *app){
	InitJANAPlugin(app);
	app->Add(new JEventProcessor_pedestals());
}
} // "C"


//------------------
// JEventProcessor_pedestals (Constructor)
//------------------
JEventProcessor_pedestals::JEventProcessor_pedestals()
{
	SetTypeName("JEventProcessor_pedestals");
	//pthread_mutex_init(&mutex, NULL);
}

//------------------
// ~JEventProcessor_pedestals (Destructor)
//------------------
JEventProcessor_pedestals::~JEventProcessor_pedestals()
{

}

//------------------
// Init
//------------------
void JEventProcessor_pedestals::Init()
{
}

//------------------
// BeginRun
//------------------
void JEventProcessor_pedestals::BeginRun(const std::shared_ptr<const JEvent>& event)
{
}

//------------------
// Process
//------------------
void JEventProcessor_pedestals::Process(const std::shared_ptr<const JEvent>& event)
{
	vector<const Df250PulseData*> df250dats;
	vector<const Df250PulseIntegral*> df250pis;
	vector<const Df125PulseIntegral*> df125pis;
	event->Get(df250pis);
	event->Get(df125pis);

	// Lock ROOT mutex	
	GetLockService(event)->RootWriteLock();

	for(unsigned int i=0; i<df250dats.size(); i++){
		TH2D *h = GetHist(df250dats[i]);
        // save single-sample pedestal
		if(h) h->Fill(df250dats[i]->pedestal/df250dats[i]->nsamples_pedestal, (double)df250dats[i]->channel);
	}

	for(unsigned int i=0; i<df250pis.size(); i++){
		TH2D *h = GetHist(df250pis[i]);
		if(h) h->Fill(df250pis[i]->pedestal, (double)df250pis[i]->channel);
	}

	for(unsigned int i=0; i<df125pis.size(); i++){
		TH2D *h = GetHist(df125pis[i]);
		if(h) h->Fill(df125pis[i]->pedestal, (double)df125pis[i]->channel);
	}
	
	// Unlock ROOT mutex	
	GetLockService(event)->RootUnLock();
}

//------------------
// EndRun
//------------------
void JEventProcessor_pedestals::EndRun()
{
}

//------------------
// Finish
//------------------
void JEventProcessor_pedestals::Finish()
{
}

//------------------
// GetHist
//------------------
TH2D* JEventProcessor_pedestals::GetHist(const DDAQAddress *hit)
{
	TH2D *h = NULL;

	// Lock mutex
	//pthread_mutex_lock(&mutex);

	// First, check of histogram already exists for this crate, slot, channel
	csc_t csc(hit->rocid, hit->slot, 0);
	map<csc_t, TH2D*>::iterator iter = all_hists.find(csc);
	if(iter != all_hists.end()) h= iter->second;
	
	// Create histogram if not already existing
	if(!h){

		TDirectory *save_dir = gDirectory;
		
		// Make sure rocid directory exists
		char rocdirname[256];
		sprintf(rocdirname, "rocid%02d", hit->rocid);
		TDirectory *rocdir = (TDirectory*)gDirectory->FindObject(rocdirname);
		if(!rocdir) rocdir = new TDirectoryFile(rocdirname, rocdirname);
		rocdir->cd();
		
		// Determine module type
		DModuleType::type_id_t mod_type = DModuleType::UNKNOWN;
		if(dynamic_cast<const Df250PulseIntegral*>(hit) != NULL) mod_type = DModuleType::FADC250;
		else if(dynamic_cast<const Df125PulseIntegral*>(hit) != NULL) mod_type = DModuleType::FADC125;
		
		int Nchan = 0;
		int Nbins = 100;
		double xmin = -100.0;
		double xmax = +100.0;
		switch(mod_type){
			case DModuleType::FADC250:
				Nchan = 16;
				Nbins = 1050;
				xmin = -1000.0;
				xmax = 20000.0;
				break;
			case DModuleType::FADC125:
				Nchan = 72;
				Nbins = 1050;
				xmin = -1000.0;
				xmax = 100000.0;
				break;
			case DModuleType::F1TDC32: Nchan = 32; break;
			case DModuleType::F1TDC48: Nchan = 48; break;
			default:
				//pthread_mutex_unlock(&mutex);
				return NULL;
		}

		// Create histogram
		char hname[256];
		char title[256];
		sprintf(hname, "slot%02d", hit->slot);
		string mod_name = DModuleType::GetName(mod_type);
		sprintf(title, "Pedestals for %s roc=%d slot=%d", mod_name.c_str(), hit->rocid, hit->slot);
		h = new TH2D(hname, title, Nbins, xmin, xmax, Nchan, -0.5, (double)Nchan-0.5);
		h->SetXTitle("measured pedestal (scaled to total samples)");
		// h->SetYTitle("channel"); // somehow, this causes crashes when running root on the file later on ??!!
		h->SetStats(0);
		
		// Store histo in map
		all_hists[csc] = h;
		
		// Restore ROOT directory
		save_dir->cd();
	}

	// Unlock  mutex
	//pthread_mutex_unlock(&mutex);
	
	return h;
}


