// $Id: $
//
//    File: DEventProcessor_cdc_hists.cc
//

#include <iostream>
#include <cmath>
using namespace std;

#include <TThread.h>

#include <JANA/JEvent.h>

#include "DEventProcessor_cdc_hists.h"

#include <DANA/DEvent.h>
#include <TRACKING/DMCThrown.h>
#include <TRACKING/DMCTrackHit.h>
#include <TRACKING/DMCThrown.h>
#include <TRACKING/DReferenceTrajectory.h>
#include <CDC/DCDCHit.h>
#include <CDC/DCDCTrackHit.h>
#include <FDC/DFDCHit.h>
#include <DVector2.h>


// Routine used to create our DEventProcessor
extern "C"{
void InitPlugin(JApplication *app){
	InitJANAPlugin(app);
	app->Add(new DEventProcessor_cdc_hists());
}
} // "C"


//------------------
// DEventProcessor_cdc_hists
//------------------
DEventProcessor_cdc_hists::DEventProcessor_cdc_hists()
{	
	cdc_ptr = &cdc;
	cdchit_ptr = &cdchit;
	
	pthread_mutex_init(&mutex, NULL);
}

//------------------
// ~DEventProcessor_cdc_hists
//------------------
DEventProcessor_cdc_hists::~DEventProcessor_cdc_hists()
{
}

//------------------
// Init
//------------------
void DEventProcessor_cdc_hists::Init()
{
	// open ROOT file (if needed)
	//if(ROOTfile != NULL) ROOTfile->cd();
	
	// Create THROWN directory
	TDirectory *dir = new TDirectoryFile("CDC","CDC");
	dir->cd();

	// Create Tree
	cdctree = new TTree("cdc","CDC Truth points");
	cdchittree = new TTree("cdchit","CDC Hits");
	cdcbranch = cdctree->Branch("T","CDC_branch",&cdc_ptr);
	cdchitbranch = cdchittree->Branch("H","CDChit_branch",&cdchit_ptr);
	
	idEdx = new TH1D("idEdx","Integrated dE/dx in CDC", 10000, 0.0, 1.0E-3);
	idEdx->SetXTitle("dE/dx (GeV/cm)");
	idEdx_vs_p = new TH2D("idEdx_vs_p","Integrated dE/dx vs. momentum in CDC", 100, 0.0, 1.0, 1000, 0.0, 1.0E1);
	idEdx->SetXTitle("momentum (GeV/c)");
	idEdx->SetYTitle("dE/dx (MeV/g^{-1}cm^{2})");

	// Go back up to the parent directory
	dir->cd("../");
}

//------------------
// BeginRun
//------------------
void DEventProcessor_cdc_hists::BeginRun(const std::shared_ptr<const JEvent>& event)
{
	LockState();
	bfield = DEvent::GetBfield(event);
	UnlockState();
}

//------------------
// EndRun
//------------------
void DEventProcessor_cdc_hists::EndRun()
{
}

//------------------
// Finish
//------------------
void DEventProcessor_cdc_hists::Finish()
{
}

//------------------
// Process
//------------------
void DEventProcessor_cdc_hists::Process(const std::shared_ptr<const JEvent>& event)
{
	vector<const DMCTrackHit*> mctrackhits;
	vector<const DCDCHit*> cdchits;
	vector<const DCDCTrackHit*> cdctrackhits;
	vector<const DFDCHit*> fdchits;
	vector<const DMCThrown*> mcthrowns;
	event->Get(mctrackhits);
	event->Get(cdchits);
	event->Get(cdctrackhits);
	event->Get(fdchits);
	event->Get(mcthrowns);
	
	// Find number of wire hits in FDC
	int Nfdc_wire_hits = 0;
	for(unsigned int i=0; i<fdchits.size(); i++)if(fdchits[i]->type==0)Nfdc_wire_hits++;
	
	// Swim reference trajectory for first thrown track
	DReferenceTrajectory* rt = new DReferenceTrajectory(bfield);
	const DMCThrown *mcthrown = mcthrowns.size()>0 ? mcthrowns[0]:NULL;
	if(mcthrown){
		rt->Swim(mcthrown->position(), mcthrown->momentum(), mcthrown->charge());
	}

	// Although we are only filling objects local to this plugin, TTree::Fill() periodically writes to file: Global ROOT lock
	GetLockService(event)->RootWriteLock(); //ACQUIRE ROOT LOCK
	
	// Loop over all truth hits, ignoring all but CDC hits
	for(unsigned int i=0; i<mctrackhits.size(); i++){
		const DMCTrackHit *mctrackhit = mctrackhits[i];
		if(mctrackhit->system != SYS_CDC)continue;
		
		double r = mctrackhit->r;
		double phi = mctrackhit->phi;
		double x = r*cos(phi);
		double y = r*sin(phi);
		cdc.pos_truth.SetXYZ(x, y, mctrackhit->z);
		
		cdctree->Fill();
	}

	// Loop over all real hits
	double Q = 0.0;
	double dxtot = 0.0;
	for(unsigned int i=0; i<cdchits.size(); i++){
		const DCDCHit *hit = cdchits[i];
		const DCDCWire *wire = (cdchits.size() == cdctrackhits.size()) ? cdctrackhits[i]->wire:NULL;
		
		cdchit.ring		= hit->ring;
		cdchit.straw	= hit->straw;
		cdchit.q			= hit->q;
		cdchit.dx		= 0.0;
		cdchit.t			= hit->t;
		cdchit.pthrown = mcthrowns.size()>0 ? mcthrowns[0]->momentum().Mag():-1000.0;
		cdchit.ncdchits	= (int)cdchits.size();
		cdchit.ntothits	= (int)cdchits.size() + Nfdc_wire_hits;
		
		if(mcthrown && wire){
			cdchit.dx = rt->Straw_dx(wire, 0.8);
		}
		
		if(cdchit.dx!=0.0){
			Q += cdchit.q;
			dxtot += cdchit.dx;
		}
		
		// Find residual of hit with "thrown" track (if present)
		if(mcthrown && wire){
			double s;
			double doca = rt->DistToRT(wire, &s);
			double mass = 0.13957; // assume pion
			double beta = 1.0/sqrt(1.0 + pow(mass/mcthrown->momentum().Mag(), 2.0))*2.998E10;
			double tof = s/beta/1.0E-9; // in ns
			double dist = (cdchit.t-tof)*55.0E-4;
			cdchit.resi_thrown = doca-dist;
		}else{
			cdchit.resi_thrown = 0.0;
		}

		cdchittree->Fill();
	}
	
	// Fill dE/dx histograms
	if(((Nfdc_wire_hits+cdchits.size()) >= 10) && (cdchits.size()>=5)){
		if(dxtot>0.0){
			idEdx->Fill(Q/dxtot);
			if(mcthrown){
				// The CDC gas is 85% Ar, 15% CO2 by mass.
				// density of  Ar: 1.977E-3 g/cm^3
				// density of CO2: 1.66E-3 g/cm^3
				double density = 0.85*1.66E-3 + 0.15*1.977E-3;
				idEdx_vs_p->Fill(mcthrown->momentum().Mag(), Q/dxtot*1000.0/density);
			}
		}
	}

	GetLockService(event)->RootUnLock(); //RELEASE ROOT LOCK
	
	delete rt;
}
