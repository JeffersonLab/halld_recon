// $Id: DEventProcessor_candidate_tree.cc 6193 2010-03-23 13:17:10Z davidl $
//
//    File: DEventProcessor_candidate_tree.cc
// Created: Fri Aug 20 13:22:33 EDT 2010
// Creator: davidl (on Darwin harriet.jlab.org 9.8.0)
//

#include <iostream>
#include <iomanip>
#include <cmath>
using namespace std;

#include "DEventProcessor_candidate_tree.h"

#include <TROOT.h>

#include <JANA/JApplication.h>
#include <JANA/JEvent.h>

#include <DANA/DEvent.h>
#include <TRACKING/DMCThrown.h>
#include <TRACKING/DMCTrackHit.h>
#include <TRACKING/DTrackCandidate.h>
#include <TRACKING/DTrackHitSelectorTHROWN.h>
#include <PID/DParticle.h>
#include <FDC/DFDCGeometry.h>
#include <CDC/DCDCTrackHit.h>
#include <FDC/DFDCPseudo.h>
#include <HDGEOMETRY/DGeometry.h>
#include <PID/DChargedTrack.h>
#include <TRACKING/DTrackTimeBased.h>
#include <DVector2.h>
#include <particleType.h>


// Routine used to create our DEventProcessor
extern "C"{
void InitPlugin(JApplication *app){
	InitJANAPlugin(app);
	app->Add(new DEventProcessor_candidate_tree());
}
} // "C"


//------------------
// DEventProcessor_candidate_tree
//------------------
DEventProcessor_candidate_tree::DEventProcessor_candidate_tree()
{
	cdchit_ptr = &cdchit;
	fdchit_ptr = &fdchit;
	trk_ptr = &trk;
	
	target.origin.SetXYZ(0.0, 0.0, 65.0);
	target.sdir.SetXYZ(1.0, 0.0, 0.0);
	target.tdir.SetXYZ(0.0, 1.0, 0.0);
	target.udir.SetXYZ(0.0, 0.0, 1.0);
	target.L = 30.0;
	
	pthread_mutex_init(&mutex, NULL);
	
	NLRgood = 0;
	NLRbad = 0;
	NLRfit_unknown = 0;
	Nevents = 0;
	
	Nwarnings = 0;
}

//------------------
// ~DEventProcessor_candidate_tree
//------------------
DEventProcessor_candidate_tree::~DEventProcessor_candidate_tree()
{

}

//------------------
// Init
//------------------
void DEventProcessor_candidate_tree::Init()
{
	// Create TRACKING directory
	TDirectory *dir = (TDirectory*)gROOT->FindObject("TRACKING");
	if(!dir)dir = new TDirectoryFile("TRACKING","TRACKING");
	dir->cd();

	cdchits = new TTree("cdchit","CDC hits");
	cdchits->Branch("cdchit","dchit",&cdchit_ptr);

	fdchits = new TTree("fdchit","FDC hits");
	fdchits->Branch("fdchit","dchit",&fdchit_ptr);

	ttrack = new TTree("track","Track");
	ttrack->Branch("track","trackpar",&trk_ptr);

	dir->cd("../");
}

//------------------
// BeginRun
//------------------
void DEventProcessor_candidate_tree::BeginRun(const std::shared_ptr<const JEvent>& event)
{	
	LockState();
	lorentz_def = GetLorentzDeflections(event);
	bfield = GetBfield(event);
	UnlockState();
}

//------------------
// EndRun
//------------------
void DEventProcessor_candidate_tree::EndRun()
{
}

//------------------
// Finish
//------------------
void DEventProcessor_candidate_tree::Finish()
{
	char str[256];
	sprintf(str,"%3.4f%%", 100.0*(double)NLRbad/(double)(NLRbad+NLRgood));
	
	cout<<endl<<setprecision(4);
	cout<<"       NLRgood: "<<NLRgood<<endl;
	cout<<"        NLRbad: "<<NLRbad<<endl;
	cout<<"     NLRfit==0: "<<NLRfit_unknown<<endl;
	cout<<"Percentage bad: "<<str<<endl;
	cout<<"       Nevents: "<<Nevents<<endl;
	cout<<endl;
}

//------------------
// Process
//------------------
void DEventProcessor_candidate_tree::Process(const std::shared_ptr<const JEvent>& locEvent)
{
	auto eventnumber = locEvent->GetEventNumber();

	vector<const DTrackCandidate*> candidates;
	vector<const DMCThrown*> mcthrowns;
	vector<const DMCTrackHit*> mctrackhits;

	locEvent->Get(candidates);
	locEvent->Get(mcthrowns);
	locEvent->Get(mctrackhits);
	
	Nevents++;
	
	// Only look at events with one thrown and one reconstructed particle
	if(mcthrowns.size() !=1 || candidates.size()<1 ){
		if(Nwarnings<10){
			_DBG_<<" mcthrowns.size()="<<mcthrowns.size()<<" candidates.size()="<<candidates.size()<<endl;
			Nwarnings++;
			if(Nwarnings==10)_DBG_<<"Last warning!!"<<endl;
		}
		return;
	}
	const DTrackCandidate *candidate = candidates[0]; // technically, this could have more than 1 candidate!
	const DMCThrown *thrown = mcthrowns[0];
	const DTrackCandidate* recon = candidate;
	
	// Loop over found reconstructed particles looking for best match to thrown
	int min_chisq=1.0E8;
	TVector3 mc_mom(thrown->momentum().X(), thrown->momentum().Y(), thrown->momentum().Z());
	for(unsigned int i=0; i<candidates.size(); i++){
		TVector3 mom(candidates[i]->momentum().X(), candidates[i]->momentum().Y(), candidates[i]->momentum().Z());

		if(mom.Mag()<1.0E-9)continue; // Thrown momentum should be > 1eV/c
		
		// Round-off errors can cause cos_phi to be outside the allowed range of -1 to +1
		// Force it to be inside that range in necessary.
		double cos_phi = mom.Dot(mc_mom)/mom.Mag()/mc_mom.Mag();
		if(cos_phi>1.)cos_phi=1.0;
		if(cos_phi<-1.)cos_phi=-1.0;
		
		double delta_pt = (mom.Pt()-mc_mom.Pt())/mc_mom.Pt();
		double delta_theta = (mom.Theta() - mc_mom.Theta())*1000.0; // in milliradians
		double delta_phi = acos(cos_phi)*1000.0; // in milliradians
		double chisq = pow(delta_pt/0.04, 2.0) + pow(delta_theta/20.0, 2.0) + pow(delta_phi/20.0, 2.0);

		if(chisq<min_chisq){
			min_chisq = chisq;
			recon = candidates[i];
		}
	}
	
	// Get CDC and FDC hits for reconstructed particle
	vector<const DCDCTrackHit *> cdctrackhits;
	vector<const DFDCPseudo *> fdcpseudohits;
	recon->Get(cdctrackhits);
	recon->Get(fdcpseudohits);
	
	// Here, we need to cast away the const-ness of the DReferenceTrajectory so we
	// can use it to find DOCA points for each wire. This is OK to do here even
	// outside of the mutex lock.
	DReferenceTrajectory *rt = new DReferenceTrajectory(bfield);
	rt->Swim(recon->position(), recon->momentum(), recon->charge());
	if(!rt)return;

	// Although we are only filling objects local to this plugin, TTree::Fill() periodically writes to file: Global ROOT lock
	GetLockService(locEvent)->RootWriteLock(); //ACQUIRE ROOT LOCK

	// Loop over CDC hits
	int NLRcorrect_this_track = 0;
	int NLRincorrect_this_track = 0;
	for(unsigned int i=0; i<cdctrackhits.size(); i++){
		const DCDCTrackHit *cdctrackhit = cdctrackhits[i];

		// The hit info structure is used to pass info both in and out of FindLR()
		hit_info_t hit_info;
		hit_info.rt = (DReferenceTrajectory*)rt;
		hit_info.wire = cdctrackhit->wire;
		hit_info.tdrift = cdctrackhit->tdrift;
		hit_info.FindLR(mctrackhits);
		cdchit.eventnumber = eventnumber;
		cdchit.wire = cdctrackhit->wire->straw;
		cdchit.layer = cdctrackhit->wire->ring;
		cdchit.t = cdctrackhit->tdrift;
		cdchit.tof = hit_info.tof;
		cdchit.doca = hit_info.doca;
		cdchit.resi = hit_info.dist - hit_info.doca;
		cdchit.u = 0.0;
		cdchit.u_pseudo = 0.0;
		cdchit.u_lorentz = 0.0;
		cdchit.resic = 0.0;
		cdchit.trk_chisq = recon->chisq;
		cdchit.trk_Ndof = recon->Ndof;
		cdchit.LRis_correct = hit_info.LRis_correct;
		cdchit.LRfit = hit_info.LRfit;
		cdchit.pos_doca = hit_info.pos_doca;
		cdchit.pos_wire = hit_info.pos_wire;
		
		cdchits->Fill();

		if(cdchit.LRfit!=0){
			if(cdchit.LRis_correct){
				NLRcorrect_this_track++;
				NLRgood++;
			}else{
				NLRincorrect_this_track++;
				NLRbad++;
			}
		}else{
			NLRfit_unknown++;
		}

	}

#if 1
	// Loop over FDC hits
	for(unsigned int i=0; i<fdcpseudohits.size(); i++){
		const DFDCPseudo *fdcpseudohit = fdcpseudohits[i];

		// Lorentz corrected poisition along the wire is contained in x,y values.
		//DVector3 wpos(fdcpseudohit->x, fdcpseudohit->y, fdcpseudohit->wire->origin.Z());
		//DVector3 wdiff = wpos - fdcpseudohit->wire->origin;
		//double u_corr = fdcpseudohit->wire->udir.Dot(wdiff);

		// The hit info structure is used to pass info both in and out of FindLR()
		hit_info_t hit_info;
		hit_info.rt = (DReferenceTrajectory*)rt;
		hit_info.wire = fdcpseudohit->wire;
		hit_info.tdrift = fdcpseudohit->time;
		hit_info.FindLR(mctrackhits, lorentz_def);
		
		fdchit.eventnumber = eventnumber;
		fdchit.wire = fdcpseudohit->wire->wire;
		fdchit.layer = fdcpseudohit->wire->layer;
		fdchit.t = fdcpseudohit->time;
		fdchit.tof = hit_info.tof;
		fdchit.doca = hit_info.doca;
		fdchit.resi = hit_info.dist - hit_info.doca;
		fdchit.u = hit_info.u;
		fdchit.u_pseudo = fdcpseudohit->s;
		fdchit.u_lorentz = hit_info.u_lorentz;
		fdchit.resic = hit_info.u - (fdcpseudohit->s + hit_info.u_lorentz) ;
		fdchit.trk_chisq = recon->chisq;
		fdchit.trk_Ndof = recon->Ndof;
		fdchit.LRis_correct = hit_info.LRis_correct;
		fdchit.LRfit = hit_info.LRfit;
		fdchit.pos_doca = hit_info.pos_doca;
		fdchit.pos_wire = hit_info.pos_wire;
		
		fdchits->Fill();

		if(fdchit.LRfit!=0){
			if(fdchit.LRis_correct){
				NLRcorrect_this_track++;
				NLRgood++;
			}else{
				NLRincorrect_this_track++;
				NLRbad++;
			}
		}else{
			NLRfit_unknown++;
		}
	}
#endif

	// Find the z vertex position of fit track using reference trajectory
	double doca_tgt = rt->DistToRT(&target);
	DVector3 tmp = rt->GetLastDOCAPoint();
	TVector3 tgt_doca(tmp.X(), tmp.Y(), tmp.Z());

	// Fill in track tree
	trk.eventnumber = eventnumber;
	trk.pthrown = TVector3(thrown->momentum().X(), thrown->momentum().Y(), thrown->momentum().Z());
	trk.pfit = TVector3(recon->momentum().X(), recon->momentum().Y(), recon->momentum().Z());
	trk.z_thrown = thrown->position().Z();
	trk.z_fit = tgt_doca.Z();
	trk.z_can = candidate->position().Z();
	trk.r_fit = doca_tgt;
	trk.NLRcorrect = NLRcorrect_this_track;
	trk.NLRincorrect = NLRincorrect_this_track;
	
	ttrack->Fill();

	// Unlock mutex
	GetLockService(locEvent)->RootUnLock(); //RELEASE ROOT LOCK
	
	delete rt;
}

//------------------
// FindLR
//------------------
void DEventProcessor_candidate_tree::hit_info_t::FindLR(vector<const DMCTrackHit*> &mctrackhits, const DLorentzDeflections *lorentz_def)
{
	/// Decided on whether or not the reference trajectory is on the correct side of the wire
	/// based on truth information.
	///
	/// Upon entry, the rt, wire, and rdrift members should be set. The remaining fields are
	/// filled in on return from this method.
	///
	/// Essentially, this will look through the DMCTrackHit objects via the
	/// DTrackHitSelectorTHROWN::GetMCTrackHit method to find the truth point corresponding
	/// to this hit (if any). It then compares the vector pointing from the point of
	/// closest approach on the wire to the DOCA point so a similar vector pointing
	/// from the same place on the wire to the truth point. If the 2 vectors are within
	/// +/- 90 degrees, then the trajectory is said to be on the correct side of the wire.
	
	TVector3 udir(wire->udir.X(), wire->udir.Y(), wire->udir.Z());
	TVector3 origin(wire->origin.X(), wire->origin.Y(), wire->origin.Z());

	double s;
	doca = rt->DistToRT(wire, &s);
	DVector3 tmp1, tmp2;
	rt->GetLastDOCAPoint(tmp1, tmp2);
	TVector3 pos_doca(tmp1.X(), tmp1.Y(), tmp1.Z());
	TVector3 mom_doca(tmp2.X(), tmp2.Y(), tmp2.Z());
	
	TVector3 shift = udir.Cross(mom_doca);
	u = rt->GetLastDistAlongWire();
	pos_wire = origin + u*udir;

	// Estimate TOF assuming pion
	double mass = 0.13957;
	double beta = 1.0/sqrt(1.0 + pow(mass/mom_doca.Mag(), 2.0))*2.998E10;
	tof = s/beta/1.0E-9; // in ns
	dist = (tdrift - tof)*55E-4;
	
	// Find the Lorentz correction based on current track (if applicable)
	if(lorentz_def){
		TVector3 shift = udir.Cross(mom_doca);
		shift.SetMag(1.0);
		double LRsign = shift.Dot(pos_doca-pos_wire)<0.0 ? +1.0:-1.0;
		double alpha = mom_doca.Angle(TVector3(0,0,1));
		u_lorentz = LRsign*lorentz_def->GetLorentzCorrection(pos_doca.X(), pos_doca.Y(), pos_doca.Z(), alpha, dist);
	}else{
		u_lorentz = 0.0;
	}
	
	// Look for a truth hit corresponding to this wire. If none is found, mark the hit as 
	// 0 (i.e. neither left nor right) and continue to the next hit.
	const DMCTrackHit *mctrackhit = DTrackHitSelectorTHROWN::GetMCTrackHit(wire, dist, mctrackhits);
	if(!mctrackhit){
		LRfit = 0;
		LRis_correct = false; // can't really tell what to set this to
	}else{
		TVector3 pos_truth(mctrackhit->r*cos(mctrackhit->phi), mctrackhit->r*sin(mctrackhit->phi), mctrackhit->z);
		TVector3 pos_diff_truth = pos_truth-pos_wire;
		TVector3 pos_diff_traj  = pos_doca-pos_wire;
		
		LRfit = shift.Dot(pos_diff_traj)<0.0 ? -1:1;
		LRis_correct = pos_diff_truth.Dot(pos_diff_traj)>0.0;
	}
}

