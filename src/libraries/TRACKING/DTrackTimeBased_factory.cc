// $Id$
//
//    File: DTrackTimeBased_factory.cc
// Created: Thu Sep  4 14:02:44 EDT 2008
// Creator: davidl (on Darwin harriet.jlab.org 8.11.1 i386)
//


#include <iostream>
#include <iomanip>
#include <set>
#include <mutex>
#include <TMath.h>
using namespace std;

#define TOF_SIGMA 0.080   // TOF resolution in ns

#include <TROOT.h>

#include "DTrackTimeBased_factory.h"
#include <TRACKING/DTrackWireBased.h>
#include <TRACKING/DReferenceTrajectory.h>
#include <TRACKING/DTrackFitter.h>
#include <TRACKING/DTrackHitSelector.h>
#include <TRACKING/DMCTrackHit.h>
#include <SplitString.h>
#include "HDGEOMETRY/DMagneticFieldMapNoField.h"
#include <deque>

using namespace jana;

// Routine for sorting start times
bool DTrackTimeBased_T0_cmp(DTrackTimeBased::DStartTime_t a,
			    DTrackTimeBased::DStartTime_t b){
  return (a.system>b.system);
}

bool DTrackTimeBased_cmp(DTrackTimeBased *a,DTrackTimeBased *b){
  if (a->candidateid==b->candidateid) return a->mass()<b->mass();
  return a->candidateid<b->candidateid;
}

// Routines for sorting dEdx data
bool DTrackTimeBased_dedx_cmp(DParticleID::dedx_t a,DParticleID::dedx_t b){
  return a.dEdx > b.dEdx;
}

bool DTrackTimeBased_dedx_amp_cmp(DParticleID::dedx_t a,DParticleID::dedx_t b){
  return a.dEdx_amp > b.dEdx_amp;
}

// count_common_members
//------------------
template<typename T>
static unsigned int count_common_members(vector<T> &a, vector<T> &b)
{
	unsigned int n=0;
	for(unsigned int i=0; i<a.size(); i++){
		for(unsigned int j=0; j<b.size(); j++){
			if(a[i]==b[j])n++;
		}
	}
	
	return n;
}



//------------------
// init
//------------------
jerror_t DTrackTimeBased_factory::init(void)
{
	fitter = NULL;

	DEBUG_HISTS = false;
	//DEBUG_HISTS = true;
	DEBUG_LEVEL = 0;

	USE_HITS_FROM_WIREBASED_FIT=false;
	gPARMS->SetDefaultParameter("TRKFIT:USE_HITS_FROM_WIREBASED_FIT",
			      USE_HITS_FROM_WIREBASED_FIT);
	INSERT_MISSING_HYPOTHESES=true;
	gPARMS->SetDefaultParameter("TRKFIT:INSERT_MISSING_HYPOTHESES",
				    INSERT_MISSING_HYPOTHESES);

	gPARMS->SetDefaultParameter("TRKFIT:DEBUG_HISTS",DEBUG_HISTS);
	gPARMS->SetDefaultParameter("TRKFIT:DEBUG_LEVEL",DEBUG_LEVEL);
	
	vector<int> hypotheses;
	hypotheses.push_back(Positron);
	hypotheses.push_back(PiPlus);
	hypotheses.push_back(KPlus);
	hypotheses.push_back(Proton);
	hypotheses.push_back(Electron);
	hypotheses.push_back(PiMinus);
	hypotheses.push_back(KMinus);
	hypotheses.push_back(AntiProton);

	ostringstream locMassStream;
	for(size_t loc_i = 0; loc_i < hypotheses.size(); ++loc_i)
	{
		locMassStream << hypotheses[loc_i];
		if(loc_i != (hypotheses.size() - 1))
			locMassStream << ",";
	}

	string HYPOTHESES = locMassStream.str();
	gPARMS->SetDefaultParameter("TRKFIT:HYPOTHESES", HYPOTHESES);

	// Parse MASS_HYPOTHESES strings to make list of masses to try
	hypotheses.clear();
	SplitString(HYPOTHESES, hypotheses, ",");
	for(size_t loc_i = 0; loc_i < hypotheses.size(); ++loc_i)
	{
	  if(ParticleCharge(Particle_t(hypotheses[loc_i])) > 0){
	    mass_hypotheses_positive.push_back(hypotheses[loc_i]);
	  }
	  else if(ParticleCharge(Particle_t(hypotheses[loc_i])) < 0){
			mass_hypotheses_negative.push_back(hypotheses[loc_i]);
	  }
	}

	if(mass_hypotheses_positive.empty()){
		static once_flag pwarn_flag;
		call_once(pwarn_flag, [](){
			jout << endl;
			jout << "############# WARNING !! ################ " <<endl;
			jout << "There are no mass hypotheses for positive tracks!" << endl;
			jout << "Be SURE this is what you really want!" << endl;
			jout << "######################################### " <<endl;
			jout << endl;
		});
	}
	if(mass_hypotheses_negative.empty()){
		static once_flag nwarn_flag;
		call_once(nwarn_flag, [](){
			jout << endl;
			jout << "############# WARNING !! ################ " <<endl;
			jout << "There are no mass hypotheses for negative tracks!" << endl;
			jout << "Be SURE this is what you really want!" << endl;
			jout << "######################################### " <<endl;
			jout << endl;
		});
	}

	mNumHypPlus=mass_hypotheses_positive.size();
	mNumHypMinus=mass_hypotheses_negative.size();

	// Forces correct particle id (when available)
	PID_FORCE_TRUTH = false;
	gPARMS->SetDefaultParameter("TRKFIT:PID_FORCE_TRUTH", PID_FORCE_TRUTH);

	USE_SC_TIME=true;
	gPARMS->SetDefaultParameter("TRKFIT:USE_SC_TIME",USE_SC_TIME);

	USE_TOF_TIME=true;
	gPARMS->SetDefaultParameter("TRKFIT:USE_TOF_TIME",USE_TOF_TIME);

	USE_FCAL_TIME=true;
	gPARMS->SetDefaultParameter("TRKFIT:USE_FCAL_TIME",USE_FCAL_TIME);
	
	USE_BCAL_TIME=true;
	gPARMS->SetDefaultParameter("TRKFIT:USE_BCAL_TIME",USE_BCAL_TIME);
       
    SAVE_TRUNCATED_DEDX = false;
    gPARMS->SetDefaultParameter("TRK:SAVE_TRUNCATED_DEDX",SAVE_TRUNCATED_DEDX);
 
    COUNT_POTENTIAL_HITS = false;
    gPARMS->SetDefaultParameter("TRK:COUNT_POTENTIAL_HITS",COUNT_POTENTIAL_HITS);

	return NOERROR;
}

//------------------
// brun
//------------------
jerror_t DTrackTimeBased_factory::brun(jana::JEventLoop *loop, int32_t runnumber)
{
  // Get the geometry
  DApplication* dapp=dynamic_cast<DApplication*>(loop->GetJApplication());
  geom = dapp->GetDGeometry(runnumber);
   // Check for magnetic field
  dIsNoFieldFlag = (dynamic_cast<const DMagneticFieldMapNoField*>(dapp->GetBfield(runnumber)) != NULL);

  if(dIsNoFieldFlag){
    //Setting this flag makes it so that JANA does not delete the objects in 
    //_data.  This factory will manage this memory.
    //This is all of these pointers are just copied from the "StraightLine" 
    //factory, and should not be re-deleted.
    SetFactoryFlag(NOT_OBJECT_OWNER);
  }
  else{
    ClearFactoryFlag(NOT_OBJECT_OWNER); //This factory will create it's own obje
  }

  // Get pointer to TrackFitter object that actually fits a track
  vector<const DTrackFitter *> fitters;
  loop->Get(fitters);
  if(fitters.size()<1){
    _DBG_<<"Unable to get a DTrackFitter object! NO Charged track fitting will be done!"<<endl;
    return RESOURCE_UNAVAILABLE;
  }
  
  // Drop the const qualifier from the DTrackFitter pointer (I'm surely going to hell for this!)
  fitter = const_cast<DTrackFitter*>(fitters[0]);
  
  // Warn user if something happened that caused us NOT to get a fitter object pointer
  if(!fitter){
    _DBG_<<"Unable to get a DTrackFitter object! NO Charged track fitting will be done!"<<endl;
    return RESOURCE_UNAVAILABLE;
  }
	
  // Get the particle ID algorithms
  vector<const DParticleID *> pid_algorithms;
  loop->Get(pid_algorithms);
  if(pid_algorithms.size()<1){
    _DBG_<<"Unable to get a DParticleID object! NO PID will be done!"<<endl;
    return RESOURCE_UNAVAILABLE;
  }

  pid_algorithm = pid_algorithms[0];
  
  // Warn user if something happened that caused us NOT to get a pid_algorithm object pointer
  if(!pid_algorithm){
    _DBG_<<"Unable to get a DParticleID object! NO PID will be done!"<<endl;
    return RESOURCE_UNAVAILABLE;
  }

  // Get z positions of fdc wire planes
  geom->GetFDCZ(fdc_z_wires);
  // for now, assume the z extent of a package is the difference between the positions
  // of the two wire planes.  save half of this distance
  fdc_package_size = (fdc_z_wires[1]-fdc_z_wires[0]) / 2.;
  geom->GetFDCRmin(fdc_rmin_packages);
  geom->GetFDCRmax(fdc_rmax);
  
  // Get CDC wire geometry data
  geom->GetCDCWires(cdcwires);
  //   geom->GetCDCRmid(cdc_rmid); // THIS ISN'T IMPLEMENTED!!

  // extract the "mean" radius of each ring from the wire data
  cdc_rmid.clear();
  for(uint ring=0; ring<cdcwires.size(); ring++)
    if (COUNT_POTENTIAL_HITS){ 
      cdc_rmid.push_back( cdcwires[ring][0]->origin.Perp() );
    }
    
    if(DEBUG_HISTS){
		dapp->Lock();
		
		// Histograms may already exist. (Another thread may have created them)
		// Try and get pointers to the existing ones.
		fom_chi2_trk = (TH1F*)gROOT->FindObject("fom_chi2_trk");
		fom = (TH1F*)gROOT->FindObject("fom");
		hitMatchFOM = (TH1F*)gROOT->FindObject("hitMatchFOM");
		chi2_trk_mom = (TH2F*)gROOT->FindObject("chi2_trk_mom");

		if(!fom_chi2_trk)fom_chi2_trk = new TH1F("fom_chi2_trk","PID FOM: #chi^{2}/Ndf from tracking", 1000, 0.0, 100.0);
		if(!fom)fom = new TH1F("fom","Combined PID FOM", 1000, 0.0, 1.01);
		if(!hitMatchFOM)hitMatchFOM = new TH1F("hitMatchFOM","Total Fraction of Hit Matches", 101, 0.0, 1.01);
		if(!chi2_trk_mom)chi2_trk_mom = new TH2F("chi2_trk_mom","Track #chi^{2}/Ndf versus Kinematic #chi^{2}/Ndf", 1000, 0.0, 100.0, 1000, 0.,100.);


		Hstart_time=(TH2F*)gROOT->FindObject("Hstart_time");
		if (!Hstart_time) Hstart_time=new TH2F("Hstart_time",
						       "vertex time source vs. time",
						 300,-50,50,9,-0.5,8.5);

		dapp->Unlock();

	}

	JCalibration *jcalib = dapp->GetJCalibration(runnumber);
	map<string, double> targetparms;
	if (jcalib->Get("TARGET/target_parms",targetparms)==false){
	  TARGET_Z = targetparms["TARGET_Z_POSITION"];
	}
	else{
	  geom->GetTargetZ(TARGET_Z);
	}
	

	return NOERROR;
}

//------------------
// evnt
//------------------
jerror_t DTrackTimeBased_factory::evnt(JEventLoop *loop, uint64_t eventnumber)
{
  // Save event number to help with debugging
  myevt=eventnumber;
  if(!fitter)return NOERROR;

  if(dIsNoFieldFlag){
    //Clear previous objects: 
    //JANA doesn't do it because NOT_OBJECT_OWNER was set
    //It DID delete them though, in the "StraightLine" factory
    _data.clear();
     
    vector<const DTrackTimeBased*> locTimeBasedTracks;
    loop->Get(locTimeBasedTracks, "StraightLine");
    for(size_t loc_i = 0; loc_i < locTimeBasedTracks.size(); ++loc_i)
      _data.push_back(const_cast<DTrackTimeBased*>(locTimeBasedTracks[loc_i]));
    return NOERROR;
  }

  // Get candidates and hits
  vector<const DTrackWireBased*> tracks;
  loop->Get(tracks);
  if (tracks.size()==0) return NOERROR;
 
  // get start counter hits
  vector<const DSCHit*>sc_hits;
  if (USE_SC_TIME){
    loop->Get(sc_hits);
  }
  
  // Get TOF points
  vector<const DTOFPoint*> tof_points;
  if (USE_TOF_TIME){
    loop->Get(tof_points);
  }

  // Get BCAL and FCAL showers
  vector<const DBCALShower*>bcal_showers;
  if (USE_BCAL_TIME){
    loop->Get(bcal_showers);
  }
  vector<const DFCALShower*>fcal_showers;
  vector<const DFCALHit*>fcal_hits; // for fallback to single hits in FCAL
  if (USE_FCAL_TIME){
    loop->Get(fcal_hits);
    loop->Get(fcal_showers);
  }
  
  vector<const DMCThrown*> mcthrowns;
  loop->Get(mcthrowns, "FinalState");
   
  // Loop over candidates
  for(unsigned int i=0; i<tracks.size(); i++){
    const DTrackWireBased *track = tracks[i];

    unsigned int num=_data.size();

    // Create vector of start times from various sources
    vector<DTrackTimeBased::DStartTime_t>start_times;
    CreateStartTimeList(track,sc_hits,tof_points,bcal_showers,fcal_showers,fcal_hits,start_times);
	
    // Fit the track
    DoFit(track,start_times,loop,track->mass());
  
    //_DBG_<< "eventnumber:   " << eventnumber << endl;
    if (PID_FORCE_TRUTH && _data.size()>num) {
      // Add figure-of-merit based on difference between thrown and reconstructed momentum 
      // if more than half of the track's hits match MC truth hits and also (charge,mass)
      // match; add FOM=0 otherwise	  
      _data[_data.size()-1]->FOM=GetTruthMatchingFOM(i,_data[_data.size()-1],
						     mcthrowns);
    }
  } // loop over track candidates

  // Filter out duplicate tracks
  FilterDuplicates();

  // Fill in track data for missing hypotheses 
  if (INSERT_MISSING_HYPOTHESES){
    InsertMissingHypotheses(loop);
  }

  // Set MC Hit-matching information
  for(size_t loc_i = 0; loc_i < _data.size(); ++loc_i)
  {
    if(!mcthrowns.empty())
    {
      double hitFraction;
      int thrownIndex = GetThrownIndex(mcthrowns, (DKinematicData*)_data[loc_i], hitFraction);
      _data[loc_i]->dMCThrownMatchMyID = thrownIndex;
      _data[loc_i]->dNumHitsMatchedToThrown = int(hitFraction * float(Get_NumTrackHits(_data[loc_i])) + 0.01); // + 0.01 so that it rounds down properly
    }
    else
    {
      _data[loc_i]->dMCThrownMatchMyID = -1;
      _data[loc_i]->dNumHitsMatchedToThrown = 0;
    }
  }

  if (COUNT_POTENTIAL_HITS){
    // figure out the number of expected hits for this track based on the final fit
    for (size_t j=0; j<_data.size();j++){
      set<const DCDCWire *> expected_hit_straws;
      set<int> expected_hit_fdc_planes;
      
      vector<DTrackFitter::Extrapolation_t>cdc_extraps=_data[j]->extrapolations.at(SYS_CDC);
      for(uint i=0; i<cdc_extraps.size(); i++) {
	// figure out the radial position of the point to see which ring it's in
	DVector3 track_pos=cdc_extraps[i].position;
	double r = track_pos.Perp();
	uint ring=0;
	for(; ring<cdc_rmid.size(); ring++) {
	  //_DBG_ << "Rs = " << r << " " << cdc_rmid[ring] << endl;
	if( (r<cdc_rmid[ring]-0.78) || (fabs(r-cdc_rmid[ring])<0.78) )
	  break;
	}
	if(ring == cdc_rmid.size()) ring--;
	//_DBG_ << "ring = " << ring << endl;
	//_DBG_ << "ring = " << ring << "  stereo = " << cdcwires[ring][0]->stereo << endl;
	int best_straw=0;
	double best_dist_diff2=(track_pos - cdcwires[ring][0]->origin).Mag2();
	// match based on straw center
	for(uint straw=1; straw<cdcwires[ring].size(); straw++) {
	  const DCDCWire *wire=cdcwires[ring][straw];
	  DVector3 wire_position = wire->origin;  // start with the nominal wire center
	  // now take into account the z dependence due to the stereo angle
	  double dz = track_pos.Z() - wire_position.Z();
	  double ds = dz*tan(wire->stereo);
	  double phi=wire_position.Phi();
	  wire_position += DVector3(-ds*sin(phi), ds*cos(phi), dz);
	  double diff2 = (track_pos - wire_position).Mag2();
	  if( diff2 < best_dist_diff2 ){
	    best_straw = straw;
	    best_dist_diff2=diff2;
	  }
	}
	
	expected_hit_straws.insert(cdcwires[ring][best_straw]);
      }
      
      vector<DTrackFitter::Extrapolation_t>fdc_extraps=_data[j]->extrapolations.at(SYS_FDC);
      for(uint i=0; i<fdc_extraps.size(); i++) {
	// check to make sure that the track goes through the sensitive region of the FDC
	// assume one hit per plane
	double z = fdc_extraps[i].position.Z();
	double r = fdc_extraps[i].position.Perp();
	
	// see if we're in the "sensitive area" of a package
	for(uint plane=0; plane<fdc_z_wires.size(); plane++) {
	  int package = plane/6;
	  if(fabs(z-fdc_z_wires[plane]) < fdc_package_size) {
	    if( r<fdc_rmax && r>fdc_rmin_packages[package]) {
	      expected_hit_fdc_planes.insert(plane);
	    }
	    break; // found the right plane
	  }
	}
      }
      
      _data[j]->potential_cdc_hits_on_track = expected_hit_straws.size();
      _data[j]->potential_fdc_hits_on_track = expected_hit_fdc_planes.size();
    }
  } // if COUNT_POTENTIAL_HITS

  return NOERROR;
}

//------------------
// erun
//------------------
jerror_t DTrackTimeBased_factory::erun(void)
{
	return NOERROR;
}

//------------------
// fini
//------------------
jerror_t DTrackTimeBased_factory::fini(void)
{
	return NOERROR;
}

//------------------
// FilterDuplicates
//------------------
void DTrackTimeBased_factory::FilterDuplicates(void)
{
	/// Look through all current DTrackTimeBased objects and remove any
	/// that have most of their hits in common with another track
	
	if(_data.size()==0)return;

	if(DEBUG_LEVEL>2)_DBG_<<"Looking for clones of time-based tracks ..."<<endl;
	// We want to remove duplicate tracks corresponding to actual particles,
	// not just duplicate fitted tracks for certain mass hypotheses -- this
	// is partly because at a later stage the holes in the list of mass 
	// hypotheses are filled in, thereby spoiling the whole point of this
	// part of the code!
	// Keep track of pairs of candididate id's, one for which we want to 
	// keep all the results of fitting with different mass hypotheses,
	// the other for which we want to delete all the results of fitting.
	// We need both vectors to take into account potential ambiguities: 
	// for one mass hypothesis starting with one candidate may be "better"
	// than starting with a second clone candidate, whereas for a second
	// mass hypothesis, the opposite may be true.
	vector<unsigned int> candidates_to_keep;
	vector<unsigned int> candidates_to_delete;
	for(unsigned int i=0; i<_data.size()-1; i++){
		DTrackTimeBased *dtrack1 = _data[i];

		vector<const DCDCTrackHit*> cdchits1;
		vector<const DFDCPseudo*> fdchits1;
		dtrack1->Get(cdchits1);
		dtrack1->Get(fdchits1);
		// Total number of hits in this candidate
		unsigned int num_cdc1=cdchits1.size();
		unsigned int num_fdc1=fdchits1.size();
		unsigned int total1 = num_cdc1+num_fdc1;

		JObject::oid_t cand1=dtrack1->candidateid;
		for(unsigned int j=i+1; j<_data.size(); j++){
			DTrackTimeBased *dtrack2 = _data[j];
			if (dtrack2->candidateid==cand1) continue;
	
			vector<const DCDCTrackHit*> cdchits2;
			vector<const DFDCPseudo*> fdchits2;
			dtrack2->Get(cdchits2);
			dtrack2->Get(fdchits2);
			
			// Total number of hits in this candidate
			unsigned int num_cdc2=cdchits2.size();
			unsigned int num_fdc2=fdchits2.size();
			unsigned int total2 = num_cdc2+num_fdc2;
			
			// Count number of cdc and fdc hits in common
			unsigned int Ncdc = count_common_members(cdchits1, cdchits2);
			unsigned int Nfdc = count_common_members(fdchits1, fdchits2);
			
			if(DEBUG_LEVEL>3){
				_DBG_<<"cand1:"<<cand1<<" cand2:"<<dtrack2->candidateid<<endl;
				_DBG_<<"   Ncdc="<<Ncdc<<" num_cdc1="<<num_cdc1<<" num_cdc2="<<num_cdc2<<endl;
				_DBG_<<"   Nfdc="<<Nfdc<<" num_fdc1="<<num_fdc1<<" num_fdc2="<<num_fdc2<<endl;
			}
			unsigned int total = Ncdc + Nfdc;	
			// If the tracks share at most one hit, consider them
			// to be separate tracks
			if (total<=1) continue;

			// Deal with the case where there are cdc hits in 
			// common between the tracks but there were no fdc 
			// hits used in one of the tracks.
			if (Ncdc>0 && (num_fdc1>0 || num_fdc2>0) 
			    && (num_fdc1*num_fdc2)==0) continue;

			// Deal with the case where there are fdc hits in
			// common between the tracks but no cdc hits used in 
			// one of the tracks.			
			if (Nfdc>0 && (num_cdc1>0 || num_cdc2>0)
			    && (num_cdc1*num_cdc2)==0) continue;

			// Look for tracks with many common hits in the CDC
			if (num_cdc1>0 && num_cdc2>0){
			  if (double(Ncdc)/double(num_cdc1)<0.9) continue;
			  if (double(Ncdc)/double(num_cdc2)<0.9) continue;
			}
			// Look for tracks with many common hits in the FDC
			if (num_fdc1>0 && num_fdc2>0){
			  if (double(Nfdc)/double(num_fdc1)<0.9) continue;
			  if (double(Nfdc)/double(num_fdc2)<0.9) continue;
			}
			
			if(total1<total2){
			  candidates_to_delete.push_back(cand1);
			  candidates_to_keep.push_back(dtrack2->candidateid);
			}else if(total2<total1){
			  candidates_to_delete.push_back(dtrack2->candidateid);
			  candidates_to_keep.push_back(cand1);
			}else if(dtrack1->FOM > dtrack2->FOM){
			  candidates_to_delete.push_back(dtrack2->candidateid);
			  candidates_to_keep.push_back(cand1);
			}else{
			  candidates_to_delete.push_back(cand1);
			  candidates_to_keep.push_back(dtrack2->candidateid);
			}
		}
	}
	
	if(DEBUG_LEVEL>2)_DBG_<<"Found "<<candidates_to_delete.size()<<" time-based clones"<<endl;


	// Return now if we're keeping everyone
	if(candidates_to_delete.size()==0)return;

	// Deal with the ambiguity problem mentioned above
	for (unsigned int i=0;i<candidates_to_keep.size();i++){
	  for (unsigned int j=0;j<candidates_to_delete.size();j++){
	    if (candidates_to_keep[i]==candidates_to_delete[j]){
	      candidates_to_delete.erase(candidates_to_delete.begin()+j);
	      break;
	    }
	  }
	  
	}

	// Copy pointers that we want to keep to a new container and delete
	// the clone objects
	vector<DTrackTimeBased*> new_data;
	sort(_data.begin(),_data.end(),DTrackTimeBased_cmp);
	for (unsigned int i=0;i<_data.size();i++){
	  bool keep_track=true;
	  for (unsigned int j=0;j<candidates_to_delete.size();j++){
	    if (_data[i]->candidateid==candidates_to_delete[j]){
	      keep_track=false;
	      if(DEBUG_LEVEL>1){
		_DBG_<<"Deleting clone time-based fitted result "<<i
		     << " in event " << myevt << endl;
	      }
	      break;
	    }
	  }
	  if (keep_track){
	    new_data.push_back(_data[i]);
	  }
	  else delete _data[i];
	}
	_data = new_data;
}

// Returns a FOM based on difference between thrown and reconstructed momentum if track matches MC truth information, 
// returns a FOM=0 otherwise;
// a match requires identical masses and charges, and that more than half of the track's hits match the truth hits 
double DTrackTimeBased_factory::GetTruthMatchingFOM(int trackIndex,DTrackTimeBased *track,vector<const DMCThrown*>mcthrowns)  {
  bool match=false;
  
  DLorentzVector fourMom = track->lorentzMomentum(); 
  //DLorentzVector gen_fourMom[mcthrowns.size()];
   vector<DLorentzVector> gen_fourMom(mcthrowns.size());
  for(unsigned int i=0; i<mcthrowns.size(); i++){
    gen_fourMom[i] = mcthrowns[i]->lorentzMomentum();
  }
  
  // Get info for thrown track
  double f = 0.;
  int thrownIndex = GetThrownIndex(mcthrowns, track, f);
  if(thrownIndex<=0 || f<=0.5) return 0.;

  double delta_pt_over_pt = (fourMom.Pt()-gen_fourMom[thrownIndex-1].Pt())/gen_fourMom[thrownIndex-1].Pt();
  double delta_theta = (fourMom.Theta()-gen_fourMom[thrownIndex-1].Theta())*1000.0; // in milliradians
  double delta_phi = (fourMom.Phi()-gen_fourMom[thrownIndex-1].Phi())*1000.0; // in milliradians
  double chisq = pow(delta_pt_over_pt/0.04, 2.0) + pow(delta_theta/20.0, 2.0) + pow(delta_phi/20.0, 2.0);

  if (fabs(track->mass()-mcthrowns[thrownIndex-1]->mass())<0.01 && track->charge()==mcthrowns[thrownIndex-1]->charge()) 
    match = true;
  
  double trk_chi2=track->chisq;
  unsigned int ndof=track->Ndof;

  if(DEBUG_HISTS&&match){
    fom_chi2_trk->Fill(track->chisq);
    chi2_trk_mom->Fill(chisq/3.,trk_chi2/ndof);
    fom->Fill(TMath::Prob(chisq,3));
  }

  /*_DBG_ << "f: " << f << endl;
  _DBG_ << "trk_chi2: " << trk_chi2 << endl;
  _DBG_ << "ndof: " << ndof << endl;
  _DBG_ << "throwncharge: " << mcthrowns[thrownIndex-1]->charge() << endl;
  _DBG_ << "trackcharge: " << track->charge() << endl;
  _DBG_ << "chargediff: " << fabs(track->charge()-mcthrowns[thrownIndex-1]->charge()) << endl;
  _DBG_ << "thrownmass: " << mcthrowns[thrownIndex-1]->mass() << endl;
  _DBG_ << "trackmass: " << track->mass() << endl;
  _DBG_ << "massdiff: " << fabs(track->mass()-mcthrowns[thrownIndex-1]->mass()) << endl;
  _DBG_ << "chisq: " << chisq << endl;
  _DBG_ << "match?: " << match << endl;
  _DBG_ << "thrownIndex: " << thrownIndex << "   trackIndex: " << trackIndex << endl;
  _DBG_<< "track   " << setprecision(4) << "Px: " << fourMom.Px() << "    Py: " << fourMom.Py() << "   Pz: " << fourMom.Pz() << "   E: " << fourMom.E() << "    M: " << fourMom.M() << "   pt: " << fourMom.Pt() << "   theta: " << fourMom.Theta() << "   phi: " << fourMom.Phi() << endl; 
  _DBG_<< "thrown  " << setprecision(4) << "Px: " << gen_fourMom[thrownIndex-1].Px() << "    Py: " << gen_fourMom[thrownIndex-1].Py() << "   Pz: " << gen_fourMom[thrownIndex-1].Pz() << "   E: " << gen_fourMom[thrownIndex-1].E() << "    M: " << gen_fourMom[thrownIndex-1].M() << "   pt: " << gen_fourMom[thrownIndex-1].Pt() << "   theta: " << gen_fourMom[thrownIndex-1].Theta() << "   phi: " << gen_fourMom[thrownIndex-1].Phi() << endl;*/

  return (match) ?  TMath::Prob(chisq,3) : 0.0; 
}

//------------------
// GetThrownIndex
//------------------
int DTrackTimeBased_factory::GetThrownIndex(vector<const DMCThrown*>& locMCThrowns, const DKinematicData *kd, double &f)
{
	// The DKinematicData object should be a DTrackCandidate, DTrackWireBased, or DParticle which
	// has associated objects for the hits
	vector<const DCDCTrackHit*> cdctrackhits;
	kd->Get(cdctrackhits);
	vector<const DFDCPseudo*> fdcpseudos;
	kd->Get(fdcpseudos);

	int locTotalNumHits = cdctrackhits.size() + fdcpseudos.size();
	if(locTotalNumHits == 0)
	{
		f = 0;
		return -1;
	}

	// The track number is buried in the truth hit objects of type DMCTrackHit. These should be 
	// associated objects for the individual hit objects. We need to loop through them and
	// keep track of how many hits for each track number we find

	map<int, int> locHitMatches; //first int is MC my id, second is num hits
	for(size_t loc_i = 0; loc_i < locMCThrowns.size(); ++loc_i)
	{
		if(fabs(locMCThrowns[loc_i]->charge()) > 0.9)
			locHitMatches[locMCThrowns[loc_i]->myid] = 0;
	}

	// CDC hits
	for(size_t loc_i = 0; loc_i < cdctrackhits.size(); ++loc_i)
	{
		const DCDCHit* locCDCHit = NULL;
		cdctrackhits[loc_i]->GetSingle(locCDCHit);
		vector<const DCDCHit*> locTruthCDCHits;
      locCDCHit->Get(locTruthCDCHits);
		if(locTruthCDCHits.empty()) continue;

		int itrack = locTruthCDCHits[0]->itrack;
		if(locHitMatches.find(itrack) == locHitMatches.end())
			continue;
		++locHitMatches[itrack];
	}

	// FDC hits
	for(size_t loc_i = 0; loc_i < fdcpseudos.size(); ++loc_i)
	{
		vector<const DMCTrackHit*> mctrackhits;
		fdcpseudos[loc_i]->Get(mctrackhits);
		if(mctrackhits.empty())
			continue;
		if(!mctrackhits[0]->primary)
			continue;

		int itrack = mctrackhits[0]->itrack;
		if(locHitMatches.find(itrack) == locHitMatches.end())
			continue;
		++locHitMatches[itrack];
	}

	// Find DMCThrown::myid with most wires hit
	map<int, int>::iterator locIterator = locHitMatches.begin();
	int locBestMyID = -1;
	int locBestNumHits = 0;
	for(; locIterator != locHitMatches.end(); ++locIterator)
	{
		if(locIterator->second <= locBestNumHits)
			continue;
		locBestNumHits = locIterator->second;
		locBestMyID = locIterator->first;
	}

	// total fraction of reconstructed hits that match truth hits
	f = 1.0*locBestNumHits/locTotalNumHits;
	if(DEBUG_HISTS)hitMatchFOM->Fill(f);

	return locBestMyID;
}


// Create a list of start (vertex) times from various sources, ordered by 
// uncertainty.
void DTrackTimeBased_factory
  ::CreateStartTimeList(const DTrackWireBased *track,
			vector<const DSCHit*>&sc_hits,
			vector<const DTOFPoint*>&tof_points,
			vector<const DBCALShower*>&bcal_showers,	
			vector<const DFCALShower*>&fcal_showers,
			vector<const DFCALHit*>&fcal_hits,
			vector<DTrackTimeBased::DStartTime_t>&start_times){
  DTrackTimeBased::DStartTime_t start_time;
   
  // Match to the start counter and the outer detectors
  double locStartTimeVariance = 0.0;
  double track_t0=track->t0();
  double locStartTime = track_t0;  // initial guess from tracking
 
  // Get start time estimate from Start Counter
  if (pid_algorithm->Get_StartTime(track->extrapolations.at(SYS_START),sc_hits,locStartTime)){
    start_time.t0=locStartTime;
    //    start_time.t0_sigma=sqrt(locTimeVariance); //uncomment when ready
    start_time.t0_sigma=sqrt(locStartTimeVariance);
    start_time.system=SYS_START;
    start_times.push_back(start_time);
  }
  // Get start time estimate from TOF
  locStartTime = track_t0;  // initial guess from tracking
  if (pid_algorithm->Get_StartTime(track->extrapolations.at(SYS_TOF),tof_points,locStartTime)){
    // Fill in the start time vector
    start_time.t0=locStartTime;
    start_time.t0_sigma=sqrt(locStartTimeVariance);
    //    start_time.t0_sigma=sqrt(locTimeVariance); //uncomment when ready
    start_time.system=SYS_TOF;
    start_times.push_back(start_time); 
  }
  // Get start time estimate from FCAL
  locStartTime = track_t0;  // initial guess from tracking
  if (pid_algorithm->Get_StartTime(track->extrapolations.at(SYS_FCAL),fcal_showers,locStartTime)){
    // Fill in the start time vector
    start_time.t0=locStartTime;
    start_time.t0_sigma=sqrt(locStartTimeVariance);
    //    start_time.t0_sigma=sqrt(locTimeVariance); //uncomment when ready
    start_time.system=SYS_FCAL;
    start_times.push_back(start_time); 
  }
  // look for matches to single FCAL hits
  else if (pid_algorithm->Get_StartTime(track->extrapolations.at(SYS_FCAL),fcal_hits,locStartTime)){
    // Fill in the start time vector
    start_time.t0=locStartTime;
    start_time.t0_sigma=sqrt(locStartTimeVariance);
    //    start_time.t0_sigma=sqrt(locTimeVariance); //uncomment when ready
    start_time.system=SYS_FCAL;
    start_times.push_back(start_time); 

  }
  // Get start time estimate from BCAL
  locStartTime=track_t0;
  if (pid_algorithm->Get_StartTime(track->extrapolations.at(SYS_BCAL),bcal_showers,locStartTime)){
    // Fill in the start time vector
    start_time.t0=locStartTime;
    start_time.t0_sigma=0.5;
    //    start_time.t0_sigma=sqrt(locTimeVariance); //uncomment when ready
    start_time.system=SYS_BCAL;
    start_times.push_back(start_time);    
  }
  // Add the t0 estimate from the tracking 
  start_time.t0=track_t0;
  start_time.t0_sigma=5.;
  start_time.system=track->t0_detector();
  start_times.push_back(start_time);

  // Set t0 for the fit to the first entry in the list. Usually this will be
  // from the start counter.
  mStartTime=start_times[0].t0;
  mStartDetector=start_times[0].system;

  //  _DBG_ << mStartDetector << " " << mStartTime << endl;

}

// Create a list of start times and do the fit for a particular mass hypothesis
bool DTrackTimeBased_factory::DoFit(const DTrackWireBased *track,
				    vector<DTrackTimeBased::DStartTime_t>&start_times,
				    JEventLoop *loop,
				    double mass){  
  if(DEBUG_LEVEL>1){_DBG__;_DBG_<<"---- Starting time based fit with mass: "<<mass<<endl;}
  // Get the hits from the wire-based track
  vector<const DFDCPseudo*>myfdchits;
  track->GetT(myfdchits);
  vector<const DCDCTrackHit *>mycdchits;
  track->GetT(mycdchits);

  // Do the fit
  DTrackFitter::fit_status_t status = DTrackFitter::kFitNotDone;
  if (USE_HITS_FROM_WIREBASED_FIT) {
    fitter->Reset();
    fitter->SetFitType(DTrackFitter::kTimeBased);	
    
    fitter->AddHits(myfdchits);
    fitter->AddHits(mycdchits);

    status=fitter->FitTrack(track->position(),track->momentum(),
			    track->charge(),mass,mStartTime,mStartDetector);
  }   
  else{   
    fitter->Reset();
    fitter->SetFitType(DTrackFitter::kTimeBased);    
    status = fitter->FindHitsAndFitTrack(*track, track->extrapolations,loop, 
					 mass,
					 mycdchits.size()+2*myfdchits.size(),
					 mStartTime,mStartDetector);
    
    // If the status is kFitNotDone, then not enough hits were attached to this
    // track using the hit-gathering algorithm.  In this case get the hits 
    // from the wire-based track
    if (status==DTrackFitter::kFitNotDone){
      //_DBG_ << " Using wire-based hits " << endl;
      fitter->Reset();
      fitter->SetFitType(DTrackFitter::kTimeBased);   
      fitter->AddHits(myfdchits);
      fitter->AddHits(mycdchits);
      
      status=fitter->FitTrack(track->position(),track->momentum(),
			      track->charge(),mass,mStartTime,mStartDetector);
    }

  }

  // if the fit returns chisq=-1, something went terribly wrong.  We may still 
  // have a usable track from the wire-based pass.  In this case set 
  // kFitNoImprovement so we can save the wire-based results.
  if (fitter->GetChisq()<0){
    status=DTrackFitter::kFitNoImprovement;
  }
  
  // In the transition region between the CDC and the FDC where the track 
  // contains both CDC and FDC hits, sometimes too many hits are discarded in 
  // the time-based phase and the time-based fit result does not improve on the 
  // wire-based fit result.  In this case set the status word to 
  // kFitNoImprovement and copy the wire-based parameters into the time-based
  // class.
  if (myfdchits.size()>3 && mycdchits.size()>3){
    unsigned int ndof=fitter->GetNdof();
    if (TMath::Prob(track->chisq,track->Ndof)>
	TMath::Prob(fitter->GetChisq(),ndof)&&ndof<5)
      status=DTrackFitter::kFitNoImprovement;
  }
      
  // Check the status value from the fit
  switch(status){
  case DTrackFitter::kFitNotDone:
    //_DBG_<<"Fitter returned kFitNotDone. This should never happen!!"<<endl;
  case DTrackFitter::kFitFailed:
    break;
  case DTrackFitter::kFitNoImprovement:
    {
      // Create a new time-based track object
      DTrackTimeBased *timebased_track = new DTrackTimeBased();
      *static_cast<DTrackingData*>(timebased_track) = *static_cast<const DTrackingData*>(track);

      timebased_track->chisq = track->chisq;
      timebased_track->Ndof = track->Ndof;
      timebased_track->pulls = track->pulls; 
      timebased_track->extrapolations = track->extrapolations;
      timebased_track->trackid = track->id;
      timebased_track->candidateid=track->candidateid;
      timebased_track->FOM=track->FOM;
      timebased_track->flags=DTrackTimeBased::FLAG__USED_WIREBASED_FIT;
   
      // add the list of start times
      timebased_track->start_times.assign(start_times.begin(),
					  start_times.end());

      for(unsigned int m=0; m<myfdchits.size(); m++)
	timebased_track->AddAssociatedObject(myfdchits[m]); 
      for(unsigned int m=0; m<mycdchits.size(); m++)
	timebased_track->AddAssociatedObject(mycdchits[m]);

      timebased_track->measured_cdc_hits_on_track = mycdchits.size();
      timebased_track->measured_fdc_hits_on_track = myfdchits.size();

      // dEdx
      double locdEdx_FDC, locdx_FDC, locdEdx_CDC, locdEdx_CDC_amp;
      double locdx_CDC_amp,locdx_CDC;
      unsigned int locNumHitsUsedFordEdx_FDC, locNumHitsUsedFordEdx_CDC;
      pid_algorithm->CalcDCdEdx(timebased_track, locdEdx_FDC, locdx_FDC, locdEdx_CDC, locdEdx_CDC_amp, locdx_CDC, locdx_CDC_amp,locNumHitsUsedFordEdx_FDC, locNumHitsUsedFordEdx_CDC);
  
      timebased_track->ddEdx_FDC = locdEdx_FDC;
      timebased_track->ddx_FDC = locdx_FDC;
      timebased_track->dNumHitsUsedFordEdx_FDC = locNumHitsUsedFordEdx_FDC;
      timebased_track->ddEdx_CDC = locdEdx_CDC; 
      timebased_track->ddEdx_CDC_amp = locdEdx_CDC_amp;
      timebased_track->ddx_CDC = locdx_CDC; 
      timebased_track->ddx_CDC_amp = locdx_CDC_amp;
      timebased_track->dNumHitsUsedFordEdx_CDC = locNumHitsUsedFordEdx_CDC;

      timebased_track->AddAssociatedObject(track);
      _data.push_back(timebased_track);
      
      return true;
      break;
    }
  case DTrackFitter::kFitSuccess:
    {
      // Create a new time-based track object
      DTrackTimeBased *timebased_track = new DTrackTimeBased();
      *static_cast<DTrackingData*>(timebased_track) = fitter->GetFitParameters();

      timebased_track->setTime(mStartTime);
      timebased_track->chisq = fitter->GetChisq();
      timebased_track->Ndof = fitter->GetNdof();
      timebased_track->pulls = std::move(fitter->GetPulls());  
      timebased_track->extrapolations=std::move(fitter->GetExtrapolations());
      timebased_track->IsSmoothed = fitter->GetIsSmoothed();
      timebased_track->trackid = track->id;
      timebased_track->candidateid=track->candidateid;
      timebased_track->flags=DTrackTimeBased::FLAG__GOODFIT;
      
      // Set the start time and add the list of start times
      timebased_track->setT0(mStartTime,start_times[0].t0_sigma, mStartDetector);
      timebased_track->start_times.assign(start_times.begin(), start_times.end());
	  
      if (DEBUG_HISTS){
	int id=0;
	if (mStartDetector==SYS_CDC) id=1;
	else if (mStartDetector==SYS_FDC) id=2;
	else if (mStartDetector==SYS_BCAL) id=3;
	else if (mStartDetector==SYS_FCAL) id=4;
	else if (mStartDetector==SYS_TOF) id=5;

	Hstart_time->Fill(start_times[0].t0,id);
      }
      
      
      // Add hits used as associated objects
      const vector<const DCDCTrackHit*> &cdchits = fitter->GetCDCFitHits();
      const vector<const DFDCPseudo*> &fdchits = fitter->GetFDCFitHits();
      
      for(unsigned int m=0; m<cdchits.size(); m++)
	timebased_track->AddAssociatedObject(cdchits[m]);
      for(unsigned int m=0; m<fdchits.size(); m++)
	timebased_track->AddAssociatedObject(fdchits[m]);
      
      timebased_track->measured_cdc_hits_on_track = cdchits.size();
      timebased_track->measured_fdc_hits_on_track = fdchits.size();

      // dEdx
      double locdEdx_FDC, locdx_FDC, locdEdx_CDC, locdEdx_CDC_amp;
      double locdx_CDC,locdx_CDC_amp;
      unsigned int locNumHitsUsedFordEdx_FDC, locNumHitsUsedFordEdx_CDC;
      pid_algorithm->CalcDCdEdx(timebased_track, locdEdx_FDC, locdx_FDC, locdEdx_CDC, locdEdx_CDC_amp,locdx_CDC,locdx_CDC_amp, locNumHitsUsedFordEdx_FDC, locNumHitsUsedFordEdx_CDC);
      
      timebased_track->ddEdx_FDC = locdEdx_FDC;
      timebased_track->ddx_FDC = locdx_FDC;
      timebased_track->dNumHitsUsedFordEdx_FDC = locNumHitsUsedFordEdx_FDC;
      timebased_track->ddEdx_CDC = locdEdx_CDC;
      timebased_track->ddEdx_CDC_amp= locdEdx_CDC_amp;
      timebased_track->ddx_CDC = locdx_CDC;
      timebased_track->ddx_CDC_amp= locdx_CDC_amp;
      timebased_track->dNumHitsUsedFordEdx_CDC = locNumHitsUsedFordEdx_CDC;

      // Set CDC ring & FDC plane hit patterns before candidate and wirebased tracks are associated
      vector<const DCDCTrackHit*> tempCDCTrackHits;
      vector<const DFDCPseudo*> tempFDCPseudos;
      timebased_track->Get(tempCDCTrackHits);
      timebased_track->Get(tempFDCPseudos);
      timebased_track->dCDCRings = pid_algorithm->Get_CDCRingBitPattern(tempCDCTrackHits);
      timebased_track->dFDCPlanes = pid_algorithm->Get_FDCPlaneBitPattern(tempFDCPseudos);

      // Add DTrack object as associate object
      timebased_track->AddAssociatedObject(track);
    
      // Compute the figure-of-merit based on tracking
      timebased_track->FOM = TMath::Prob(timebased_track->chisq, timebased_track->Ndof);
      //_DBG_<< "FOM:   " << timebased_track->FOM << endl;

      _data.push_back(timebased_track);
     
      return true;
      break;
	  
    }
  default:
    break;
  }
  return false;
}


// Create a track with a mass hypothesis that was not present in the list of 
// fitted tracks from an existing fitted track.
void DTrackTimeBased_factory::AddMissingTrackHypothesis(vector<DTrackTimeBased*>&tracks_to_add,
				      const DTrackTimeBased *src_track,
							double my_mass,
							double q,
							JEventLoop *loop){

  // Create a new time-based track object
  DTrackTimeBased *timebased_track = new DTrackTimeBased();
  *static_cast<DTrackingData*>(timebased_track) = *static_cast<const DTrackingData*>(src_track);

  // Get the hits used in the fit  
  vector<const DCDCTrackHit *>src_cdchits;
  src_track->GetT(src_cdchits);
  vector<const DFDCPseudo *>src_fdchits;
  src_track->GetT(src_fdchits);

  // Copy over DKinematicData part from the result of a successful fit
  timebased_track->setPID(IDTrack(q, my_mass));
  timebased_track->chisq = src_track->chisq;
  timebased_track->Ndof = src_track->Ndof;
  timebased_track->pulls = src_track->pulls;
  timebased_track->extrapolations = src_track->extrapolations;
  timebased_track->trackid = src_track->id;
  timebased_track->candidateid=src_track->candidateid;
  timebased_track->FOM=src_track->FOM;
  timebased_track->flags=DTrackTimeBased::FLAG__USED_OTHER_HYPOTHESIS;
  
  // Add list of start times
  timebased_track->start_times.assign(src_track->start_times.begin(),  
				      src_track->start_times.end());
  // Set the start time we used
  timebased_track->setT0(timebased_track->start_times[0].t0,
			 timebased_track->start_times[0].t0_sigma, 
			 timebased_track->start_times[0].system);

  // Add DTrack object as associate object
  vector<const DTrackWireBased*>wire_based_track;
  src_track->GetT(wire_based_track);
  timebased_track->AddAssociatedObject(wire_based_track[0]);

  // (Partially) compensate for the difference in energy loss between the 
  // source track and a particle of mass my_mass 
  DVector3 position,momentum;
  if (timebased_track->extrapolations.at(SYS_CDC).size()>0){
    unsigned int index=timebased_track->extrapolations.at(SYS_CDC).size()-1;
    position=timebased_track->extrapolations[SYS_CDC][index].position;
    momentum=timebased_track->extrapolations[SYS_CDC][index].momentum;
  }
  else if (timebased_track->extrapolations.at(SYS_FDC).size()>0){
    unsigned int index=timebased_track->extrapolations.at(SYS_FDC).size()-1;
    position=timebased_track->extrapolations[SYS_FDC][index].position;
    momentum=timebased_track->extrapolations[SYS_FDC][index].momentum;
  }

  DTrackFitter::fit_status_t status=DTrackFitter::kFitNotDone;
  if (momentum.Mag()>0.){
    CorrectForELoss(position,momentum,q,my_mass);  
    timebased_track->setMomentum(momentum);
    timebased_track->setPosition(position);
    // Redo the fit with the new position and momentum as initial guesses
    fitter->Reset();
    fitter->SetFitType(DTrackFitter::kTimeBased);    
    status = fitter->FindHitsAndFitTrack(*timebased_track,
					 timebased_track->extrapolations,loop, 
					 my_mass,
					 src_cdchits.size()+2*src_fdchits.size(),
					 timebased_track->t0(),
					 timebased_track->t0_detector());
    // if the fit returns chisq=-1, something went terribly wrong.  Do not 
    // update the parameters for the track...
    if (fitter->GetChisq()<0) status=DTrackFitter::kFitFailed;

    // if the fit flips the charge of the track, then this is bad as well
    if(q != fitter->GetFitParameters().charge())
        status=DTrackFitter::kFitFailed; 

    // if we can't refit the track, it is likely of poor quality, so stop here and do not add the hypothesis
    if(status == DTrackFitter::kFitFailed) {
        delete timebased_track;
        return;
    }

    if (status==DTrackFitter::kFitSuccess){
      timebased_track->chisq = fitter->GetChisq();
      timebased_track->Ndof = fitter->GetNdof();
      timebased_track->pulls = std::move(fitter->GetPulls());  
      timebased_track->extrapolations=std::move(fitter->GetExtrapolations());
      timebased_track->IsSmoothed = fitter->GetIsSmoothed();  
      *static_cast<DTrackingData*>(timebased_track) = fitter->GetFitParameters();
      timebased_track->flags=DTrackTimeBased::FLAG__GOODFIT;
      
      timebased_track->setTime(timebased_track->start_times[0].t0);

      // Add hits used as associated objects
      const vector<const DCDCTrackHit*> &cdchits = fitter->GetCDCFitHits();
      const vector<const DFDCPseudo*> &fdchits = fitter->GetFDCFitHits();
       
      for(unsigned int m=0; m<cdchits.size(); m++)
	timebased_track->AddAssociatedObject(cdchits[m]);
      for(unsigned int m=0; m<fdchits.size(); m++)
	timebased_track->AddAssociatedObject(fdchits[m]); 
       
      timebased_track->measured_cdc_hits_on_track = cdchits.size();
      timebased_track->measured_fdc_hits_on_track = fdchits.size();
       
      // Compute the figure-of-merit based on tracking
      timebased_track->FOM = TMath::Prob(timebased_track->chisq, timebased_track->Ndof);
      
    }
  }

  if (status!=DTrackFitter::kFitSuccess){ 
    for(unsigned int m=0; m<src_fdchits.size(); m++)
      timebased_track->AddAssociatedObject(src_fdchits[m]); 
    for(unsigned int m=0; m<src_cdchits.size(); m++)
      timebased_track->AddAssociatedObject(src_cdchits[m]);
    
    timebased_track->measured_cdc_hits_on_track = src_cdchits.size();
    timebased_track->measured_fdc_hits_on_track = src_fdchits.size();
  }

  // dEdx
  double locdEdx_FDC, locdx_FDC, locdEdx_CDC, locdEdx_CDC_amp;
  double locdx_CDC,locdx_CDC_amp;
  unsigned int locNumHitsUsedFordEdx_FDC, locNumHitsUsedFordEdx_CDC;
  pid_algorithm->CalcDCdEdx(timebased_track, locdEdx_FDC, locdx_FDC, locdEdx_CDC, locdEdx_CDC_amp,locdx_CDC,locdx_CDC_amp, locNumHitsUsedFordEdx_FDC, locNumHitsUsedFordEdx_CDC);
  
  timebased_track->ddEdx_FDC = locdEdx_FDC;
  timebased_track->ddx_FDC = locdx_FDC;
  timebased_track->dNumHitsUsedFordEdx_FDC = locNumHitsUsedFordEdx_FDC;
  timebased_track->ddEdx_CDC = locdEdx_CDC;
  timebased_track->ddEdx_CDC_amp = locdEdx_CDC_amp;
  timebased_track->ddx_CDC = locdx_CDC;
  timebased_track->ddx_CDC_amp = locdx_CDC_amp;
  timebased_track->dNumHitsUsedFordEdx_CDC = locNumHitsUsedFordEdx_CDC;

  // The above code has a truncated mean algorithm for dEdx hardwired for the FDC
  // and selectable between on and off for the CDC_amp. Here I save the complete
  // information for a variety of truncation choices, and let the user decide later.

  if (SAVE_TRUNCATED_DEDX) {
    std::vector<DParticleID::dedx_t> locdEdxHits_CDC;
    std::vector<DParticleID::dedx_t> locdEdxHits_FDC;
    jerror_t locReturnStatus = pid_algorithm->GetDCdEdxHits(timebased_track, locdEdxHits_CDC, locdEdxHits_FDC);
    if (locReturnStatus == NOERROR) {
      const int maxtrunc(5);
      sort(locdEdxHits_CDC.begin(),locdEdxHits_CDC.end(), DTrackTimeBased_dedx_cmp);  
      for (int itrunc=0; itrunc <= maxtrunc; ++itrunc) {
        for (int i=itrunc; i < (int)locdEdxHits_CDC.size(); ++i) {
          double dx = locdEdxHits_CDC[i].dx;
          double dE = locdEdxHits_CDC[i].dEdx * dx;
          if (itrunc < (int)timebased_track->ddx_CDC_trunc.size()) {
            timebased_track->ddx_CDC_trunc[itrunc] += dx;
            timebased_track->ddEdx_CDC_trunc[itrunc] += dE;
          }
          else {
            timebased_track->ddx_CDC_trunc.push_back(dx);
            timebased_track->ddEdx_CDC_trunc.push_back(dE);
          }
        }
        if (itrunc < (int)timebased_track->ddx_CDC_trunc.size())
          timebased_track->ddEdx_CDC_trunc[itrunc] /= timebased_track->ddx_CDC_trunc[itrunc] + 1e-99;
      }

      sort(locdEdxHits_CDC.begin(),locdEdxHits_CDC.end(), DTrackTimeBased_dedx_amp_cmp);  
      for (int itrunc=0; itrunc <= maxtrunc; ++itrunc) {
        for (int i=itrunc; i < (int)locdEdxHits_CDC.size(); ++i) {
          double dx = locdEdxHits_CDC[i].dx;
          double dE = locdEdxHits_CDC[i].dEdx_amp * dx;
          if (itrunc < (int)timebased_track->ddx_CDC_amp_trunc.size()) {
            timebased_track->ddx_CDC_amp_trunc[itrunc] += dx;
            timebased_track->ddEdx_CDC_amp_trunc[itrunc] += dE;
          }
          else {
            timebased_track->ddx_CDC_amp_trunc.push_back(dx);
            timebased_track->ddEdx_CDC_amp_trunc.push_back(dE);
          }
        }
        if (itrunc < (int)timebased_track->ddx_CDC_amp_trunc.size())
          timebased_track->ddEdx_CDC_amp_trunc[itrunc] /= timebased_track->ddx_CDC_amp_trunc[itrunc] + 1e-99;
      }

      sort(locdEdxHits_FDC.begin(),locdEdxHits_FDC.end(), DTrackTimeBased_dedx_cmp);  
      for (int itrunc=0; itrunc <= maxtrunc; ++itrunc) {
        for (int i=itrunc; i < (int)locdEdxHits_FDC.size(); ++i) {
          double dx = locdEdxHits_FDC[i].dx;
          double dE = locdEdxHits_FDC[i].dEdx * dx;
          if (itrunc < (int)timebased_track->ddx_FDC_trunc.size()) {
            timebased_track->ddx_FDC_trunc[itrunc] += dx;
            timebased_track->ddEdx_FDC_trunc[itrunc] += dE;
          }
          else {
            timebased_track->ddx_FDC_trunc.push_back(dx);
            timebased_track->ddEdx_FDC_trunc.push_back(dE);
          }
        }
        if (itrunc < (int)timebased_track->ddx_FDC_trunc.size())
          timebased_track->ddEdx_FDC_trunc[itrunc] /= timebased_track->ddx_FDC_trunc[itrunc] + 1e-99;
      }

      sort(locdEdxHits_FDC.begin(),locdEdxHits_FDC.end(), DTrackTimeBased_dedx_amp_cmp);  
      for (int itrunc=0; itrunc <= maxtrunc; ++itrunc) {
        for (int i=itrunc; i < (int)locdEdxHits_FDC.size(); ++i) {
          double dx = locdEdxHits_FDC[i].dx;
          double dE = locdEdxHits_FDC[i].dEdx_amp * dx;
          if (itrunc < (int)timebased_track->ddx_FDC_amp_trunc.size()) {
            timebased_track->ddx_FDC_amp_trunc[itrunc] += dx;
            timebased_track->ddEdx_FDC_amp_trunc[itrunc] += dE;
          }
          else {
            timebased_track->ddx_FDC_amp_trunc.push_back(dx);
            timebased_track->ddEdx_FDC_amp_trunc.push_back(dE);
          }
        }
        if (itrunc < (int)timebased_track->ddx_FDC_amp_trunc.size())
          timebased_track->ddEdx_FDC_amp_trunc[itrunc] /= timebased_track->ddx_FDC_amp_trunc[itrunc] + 1e-99;
      }
    }
  }

  // Set CDC ring & FDC plane hit patterns before candidate and wirebased tracks are associated
  vector<const DCDCTrackHit*> tempCDCTrackHits;
  vector<const DFDCPseudo*> tempFDCPseudos;
  timebased_track->Get(tempCDCTrackHits);
  timebased_track->Get(tempFDCPseudos);
  timebased_track->dCDCRings = pid_algorithm->Get_CDCRingBitPattern(tempCDCTrackHits);
  timebased_track->dFDCPlanes = pid_algorithm->Get_FDCPlaneBitPattern(tempFDCPseudos);
  
  tracks_to_add.push_back(timebased_track);
}


// If the fit failed for certain hypotheses, fill in the gaps using data from
// successful fits for each candidate.
bool DTrackTimeBased_factory::InsertMissingHypotheses(JEventLoop *loop){
  if (_data.size()==0) return false;
  
  // Make sure the tracks are ordered by candidate id
  sort(_data.begin(),_data.end(),DTrackTimeBased_cmp);
 
  JObject::oid_t old_id=_data[0]->candidateid;
  unsigned int mass_bits=0;
  double q=_data[0]->charge();
  bool flipped_charge=false;
  vector<DTrackTimeBased*>myhypotheses;
  vector<DTrackTimeBased*>tracks_to_add;
  for (size_t i=0;i<_data.size();i++){
    if (_data[i]->candidateid!=old_id){
      AddMissingTrackHypotheses(mass_bits,tracks_to_add,myhypotheses,q,
				flipped_charge,loop);

      // Clear the myhypotheses vector for the next track
      myhypotheses.clear();
      // Reset flags and charge 
      q=_data[i]->charge();	
      flipped_charge=false;
      // Set the bit for this mass hypothesis
      mass_bits = 1<<_data[i]->PID();
      
      // Add the data to the myhypotheses vector
      myhypotheses.push_back(_data[i]);
    }
    else{
      myhypotheses.push_back(_data[i]);

      // Set the bit for this mass hypothesis
      mass_bits |= 1<< _data[i]->PID();
      
      // Check if the sign of the charge has flipped
      if (_data[i]->charge()!=q) flipped_charge=true;
    }
    
    old_id=_data[i]->candidateid;
  }
  // Deal with last track candidate	
  AddMissingTrackHypotheses(mass_bits,tracks_to_add,myhypotheses,q,
			    flipped_charge,loop);
    
  // Add the new list of tracks to the output list
  if (tracks_to_add.size()>0){
    for (size_t i=0;i<tracks_to_add.size();i++){
      _data.push_back(tracks_to_add[i]);
    }
    // Make sure the tracks are ordered by candidate id
    sort(_data.begin(),_data.end(),DTrackTimeBased_cmp);
  }

  return true;
}

// Use the FastSwim method in DReferenceTrajectory to propagate back to the 
// POCA to the beam line, adding a bit of energy at each step that would have 
// been lost had the particle emerged from the target.
void DTrackTimeBased_factory::CorrectForELoss(DVector3 &position,DVector3 &momentum,double q,double my_mass){  
  DReferenceTrajectory::swim_step_t dummy_step;
  DReferenceTrajectory rt(fitter->GetDMagneticFieldMap(),q,&dummy_step);
  rt.SetDGeometry(geom);
  rt.SetMass(my_mass);
  rt.SetPLossDirection(DReferenceTrajectory::kBackward);
  DVector3 last_pos,last_mom;
  DVector3 origin(0.,0.,TARGET_Z);
  DVector3 dir(0.,0.,1.);
  rt.FastSwim(position,momentum,last_pos,last_mom,q,origin,dir,300.);   
  position=last_pos;
  momentum=last_mom;   
}

// Fill in all missing hypotheses for a given track candidate
void DTrackTimeBased_factory::AddMissingTrackHypotheses(unsigned int mass_bits,
							vector<DTrackTimeBased*>&tracks_to_add,
							vector<DTrackTimeBased *>&myhypotheses,
							double q,
							bool flipped_charge,
							JEventLoop *loop){ 

  unsigned int last_index=myhypotheses.size()-1;
  unsigned int index=0;
  if (q>0){
    if (flipped_charge){
      /*if ((mass_bits & (1<<AntiProton))==0){
	AddMissingTrackHypothesis(tracks_to_add,myhypotheses[last_index],
				  ParticleMass(Proton),-1.,loop);  
      } 
      */
      for (unsigned int i=0;i<mass_hypotheses_negative.size();i++){
	if ((mass_bits & (1<<mass_hypotheses_negative[i]))==0){
	  if (mass_hypotheses_negative[i]>13) index=last_index;
	  else index=0;
	  AddMissingTrackHypothesis(tracks_to_add,myhypotheses[index],
				    ParticleMass(Particle_t(mass_hypotheses_negative[i])),
				    -1.,loop);  
	} 
      }
    }
    /*   if ((mass_bits & (1<<Proton))==0){
      AddMissingTrackHypothesis(tracks_to_add,myhypotheses[last_index],
				ParticleMass(Proton),+1.,loop);  
				} */
    for (unsigned int i=0;i<mass_hypotheses_positive.size();i++){
      if ((mass_bits & (1<<mass_hypotheses_positive[i]))==0){
	if (mass_hypotheses_positive[i]>13) index=last_index;
	else index=0;
	AddMissingTrackHypothesis(tracks_to_add,myhypotheses[index],
				  ParticleMass(Particle_t(mass_hypotheses_positive[i])),
				  +1.,loop);  
      } 
    }    
  }
  else{
    /*
    if ((mass_bits & (1<<AntiProton))==0){
      AddMissingTrackHypothesis(tracks_to_add,myhypotheses[last_index],
				ParticleMass(Proton),-1.,loop);  
				} */	
    for (unsigned int i=0;i<mass_hypotheses_negative.size();i++){
      if ((mass_bits & (1<<mass_hypotheses_negative[i]))==0){
	if (mass_hypotheses_negative[i]>13) index=last_index;
	else index=0;
	AddMissingTrackHypothesis(tracks_to_add,myhypotheses[index],
				  ParticleMass(Particle_t(mass_hypotheses_negative[i])),
				  -1.,loop);  
      } 
    }
    if (flipped_charge){
      /*if ((mass_bits & (1<<Proton))==0){
	AddMissingTrackHypothesis(tracks_to_add,myhypotheses[last_index],
				  ParticleMass(Proton),+1.,loop);  
				  } */
      for (unsigned int i=0;i<mass_hypotheses_positive.size();i++){
	if ((mass_bits & (1<<mass_hypotheses_positive[i]))==0){
	  if (mass_hypotheses_positive[i]>13) index=last_index;
	  else index=0;
	  AddMissingTrackHypothesis(tracks_to_add,myhypotheses[index],
				    ParticleMass(Particle_t(mass_hypotheses_positive[i])),
				    +1.,loop);  
	} 
      }	
    }
  }
} 
