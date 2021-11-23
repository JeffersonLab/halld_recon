// $Id$
//
//    File: DFactory_1p2pi.cc
// Created: Wed Mar 11 20:34:14 EDT 2015
// Creator: jrsteven (on Linux halldw1.jlab.org 2.6.32-504.8.1.el6.x86_64 x86_64)
//

#include "DFactory_1p2pi.h"

//------------------
// brun
//------------------
jerror_t DFactory_1p2pi::brun(JEventLoop* locEventLoop, int32_t locRunNumber)
{
	vector<double> locBeamPeriodVector;
	locEventLoop->GetCalib("PHOTON_BEAM/RF/beam_period", locBeamPeriodVector);
	dBeamBunchPeriod = locBeamPeriodVector[0];

	dKinFitUtils = new DKinFitUtils_GlueX(eventLoop);
	dKinFitter = new DKinFitter(dKinFitUtils);

	eventLoop->GetSingle(dAnalysisUtilities);

	return NOERROR;
}

//------------------
// init
//------------------
jerror_t DFactory_1p2pi::evnt(JEventLoop* locEventLoop, uint64_t locEventNumber)
{
  vector<const DChargedTrack*>ch_tracks;
  vector<const DBeamPhoton*> beam_ph;
  vector<const DNeutralShower*> showers;
  const DTrigger* Trigger = NULL;

  locEventLoop->Get(ch_tracks);
  locEventLoop->Get(beam_ph);
  locEventLoop->Get(showers);
  locEventLoop->GetSingle(Trigger);

  // some event selections
  if(!Trigger->Get_IsPhysicsEvent()) return NOERROR;
  if (ch_tracks.size()!=3) return NOERROR;


  map<Particle_t, int> targetParticles = {
	{Proton,1},
        {PiPlus,1},
        {PiMinus,1}
  };

   map<Particle_t, vector<const DChargedTrackHypothesis*> > emptyHypothesis;
   vector<map<Particle_t, vector<const DChargedTrackHypothesis*> > > hypothesisList;
   GetHypotheses(ch_tracks,targetParticles,emptyHypothesis,hypothesisList);
  

   Int_t _nHyp = hypothesisList.size();
    
   if(_nHyp == 0)  return NOERROR;

   //=== Photon Informatiion 
   Int_t  _nPhotonCandidates = beam_ph.size();

  //==========================
  
	// if we have at least one reaction, let's save some data
	D1p2piData *event_data = new D1p2piData;

	event_data->nHyp = _nHyp;
	event_data->nHyp_fitted = 0;
	event_data->nPhotonCandidates = _nPhotonCandidates;
	event_data->eventNumber = locEventNumber;
	event_data->L1TriggerBits = Trigger->Get_L1TriggerBits();

	if(_nPhotonCandidates>0){
	 for(Int_t ii = 0; ii < _nPhotonCandidates; ii++){
		event_data->bmE.push_back(beam_ph[ii]->momentum().Mag());
		event_data->bmtime.push_back(beam_ph[ii]->time());
	 }
	}

  dKinFitUtils->Reset_NewEvent();
  dKinFitter->Reset_NewEvent();

  for (Int_t j = 0; j < _nHyp; j++){
  
     map<Particle_t, vector<const DChargedTrackHypothesis*> > thisHyp = hypothesisList[j];

     //============ PiMinus Hypothesis ===========================================================//
     const DChargedTrackHypothesis *hyp_pi_min   = thisHyp[PiMinus][0];
     const DTrackTimeBased *pi_min_track = hyp_pi_min->Get_TrackTimeBased();            	    
     //============ PiPlus Hypothesis ===========================================================//
     const DChargedTrackHypothesis *hyp_pi_plus   = thisHyp[PiPlus][0];
     const DTrackTimeBased *pi_plus_track = hyp_pi_plus->Get_TrackTimeBased();
     //============ Proton Hypothesis ===========================================================//
     const DChargedTrackHypothesis *hyp_proton   = thisHyp[Proton][0];
     const DTrackTimeBased *proton_track = hyp_proton->Get_TrackTimeBased();	    

	 vector<const DTrackTimeBased*> locTrackTimeBasedVectorForVertexing;
	 locTrackTimeBasedVectorForVertexing.push_back(pi_min_track);
	 locTrackTimeBasedVectorForVertexing.push_back(pi_plus_track);
	 locTrackTimeBasedVectorForVertexing.push_back(proton_track);
	 DVector3 locRoughPosition = dAnalysisUtilities->Calc_CrudeVertex(locTrackTimeBasedVectorForVertexing);
	 TVector3 locTRoughPosition(locRoughPosition.X(), locRoughPosition.Y(), locRoughPosition.Z());


      //--------------------------------
      // Kinematic fit
      //--------------------------------
      dKinFitter->Reset_NewFit();
      set<shared_ptr<DKinFitParticle>> FinalParticles, NoParticles;

      shared_ptr<DKinFitParticle>myProton=dKinFitUtils->Make_DetectedParticle(proton_track);
      shared_ptr<DKinFitParticle>myPiMinus=dKinFitUtils->Make_DetectedParticle(pi_min_track);
      shared_ptr<DKinFitParticle>myPiPlus=dKinFitUtils->Make_DetectedParticle(pi_plus_track);

      FinalParticles.insert(myProton);
      FinalParticles.insert(myPiMinus);
      FinalParticles.insert(myPiPlus);
     
      //  Production Vertex constraint
      set<shared_ptr<DKinFitParticle>> locFullConstrainParticles;
      locFullConstrainParticles.insert(myPiPlus);
      locFullConstrainParticles.insert(myPiMinus);
      locFullConstrainParticles.insert(myProton);
      
      //shared_ptr<DKinFitConstraint_Vertex> locProductionVertexConstraint =  dKinFitUtils->Make_VertexConstraint(locFullConstrainParticles, NoParticles, proton_track->position());
	  // maybe use a better vertex guess
      shared_ptr<DKinFitConstraint_Vertex> locProductionVertexConstraint =  dKinFitUtils->Make_VertexConstraint(locFullConstrainParticles, NoParticles, locTRoughPosition);

      dKinFitter->Add_Constraint(locProductionVertexConstraint);

      // PERFORM THE KINEMATIC FIT
      bool locFitStatus = dKinFitter->Fit_Reaction();
      if(!locFitStatus) continue;

      //GET THE FIT RESULTS
      double _CL = dKinFitter->Get_ConfidenceLevel();

      if (_CL>0){
      
      	event_data->nHyp_fitted++;
      

		 TVector3 vertex_kf;
		 set<shared_ptr<DKinFitParticle>>myParticles=dKinFitter->Get_KinFitParticles();
		 set<shared_ptr<DKinFitParticle>>::iterator locParticleIterator=myParticles.begin();

		//============= Fit Values ========================================================
		shared_ptr<DKinFitParticle> fitProton = NULL;
		shared_ptr<DKinFitParticle> fitPiPlus = NULL;
		shared_ptr<DKinFitParticle> fitPiMinus = NULL;

		for(; locParticleIterator != myParticles.end(); ++locParticleIterator){
			 if ((*locParticleIterator)->Get_KinFitParticleType()==d_DetectedParticle) {
				 vertex_kf=(*locParticleIterator)->Get_Position();
				 event_data->T_vertex.push_back((*locParticleIterator)->Get_Time());
			 }

			 int pid = (*locParticleIterator)->Get_PID();

			 switch (pid){
			 case 2212:
				 fitProton = (*locParticleIterator);
				 break;
			 case 211:
				 fitPiPlus = (*locParticleIterator);
				 break;
			 case -211:
				 fitPiMinus = (*locParticleIterator);
				 break;
			 default:
				 break;
			 }
		}
		
		event_data->X_vertex.push_back(vertex_kf[0]);
		event_data->Y_vertex.push_back(vertex_kf[1]);
		event_data->Z_vertex.push_back(vertex_kf[2]);
		event_data->CLKinFit.push_back(_CL);
		event_data->NDF.push_back(dKinFitter->Get_NDF());
		event_data->ChiSqFit.push_back(dKinFitter->Get_ChiSq());
		event_data->Common_Time.push_back(fitPiPlus->Get_CommonTime());

		event_data->pX_piminus.push_back(fitPiMinus->Get_Momentum().X());
		event_data->pY_piminus.push_back(fitPiMinus->Get_Momentum().Y());
		event_data->pZ_piminus.push_back(fitPiMinus->Get_Momentum().Z());
		event_data->E_piminus.push_back(fitPiMinus->Get_Energy());
		event_data->t_piminus.push_back(fitPiMinus->Get_Time());

		event_data->pX_piplus.push_back(fitPiPlus->Get_Momentum().X());
		event_data->pY_piplus.push_back(fitPiPlus->Get_Momentum().Y());
		event_data->pZ_piplus.push_back(fitPiPlus->Get_Momentum().Z());
		event_data->E_piplus.push_back(fitPiPlus->Get_Energy());
		event_data->t_piplus.push_back(fitPiPlus->Get_Time());

		event_data->pX_proton.push_back(fitProton->Get_Momentum().X());
		event_data->pY_proton.push_back(fitProton->Get_Momentum().Y());
		event_data->pZ_proton.push_back(fitProton->Get_Momentum().Z());
		event_data->E_proton.push_back(fitProton->Get_Energy());
		event_data->t_proton.push_back(fitProton->Get_Time());
		
      }
	}
	
	_data.push_back(event_data);

	return NOERROR;
}

//------------------
// fini
//------------------
jerror_t DFactory_1p2pi::fini(void)
{
	return NOERROR;
}

// Recursive function for determining possible particle assignments

void DFactory_1p2pi::GetHypotheses(vector<const DChargedTrack *> &tracks,
						map<Particle_t, int> &particles,
						map<Particle_t, vector<const DChargedTrackHypothesis*> > &assignmentHypothesis,
						vector<map<Particle_t, vector<const DChargedTrackHypothesis*> > > &hypothesisList
						) const
{

  const DChargedTrack * firstTrack = tracks.front();
  vector<const DChargedTrack *> otherTracks(tracks);
  otherTracks.erase(otherTracks.begin());
  map<Particle_t, int>::iterator partIt;

  for (partIt = particles.begin(); partIt != particles.end(); partIt++){

      if (partIt->second > 0){

	  Particle_t particle = partIt->first;
	  const DChargedTrackHypothesis *hyp=NULL;
	  
	  if ((hyp = firstTrack->Get_Hypothesis(particle)) != NULL){

	      double prob = TMath::Prob(hyp->Get_ChiSq(),hyp->Get_NDF());
	      if (prob < 0) continue; 
		map<Particle_t, vector<const DChargedTrackHypothesis*> > newHypothesis = assignmentHypothesis;
              
		if (assignmentHypothesis.find(particle) == assignmentHypothesis.end()){
			vector<const DChargedTrackHypothesis*> newVector;
			newHypothesis[particle] = newVector;
		}
		
		newHypothesis[particle].push_back(hyp);
	      
		if (otherTracks.empty()){
			hypothesisList.push_back(newHypothesis);
		} else {
			map<Particle_t, int> otherParticles(particles);
			otherParticles[particle]--;
			GetHypotheses(otherTracks, otherParticles, newHypothesis, hypothesisList); 
                }
        }
      }
  }

  return;

} // end GetHypotheses




