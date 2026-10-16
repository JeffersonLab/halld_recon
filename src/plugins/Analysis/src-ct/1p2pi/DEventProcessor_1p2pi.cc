#include <map>
using namespace std;

#include "DEventProcessor_1p2pi.h"

#include <DANA/DApplication.h>



// Routine used to create our DEventProcessor
extern "C"{
  void InitPlugin(JApplication *app){
    InitJANAPlugin(app);
    app->Add(new DEventProcessor_1p2pi());
  }
} // "C"

thread_local DTreeFillData DEventProcessor_1p2pi::dTreeFillData;

//------------------
// init
///------------------

void DEventProcessor_1p2pi::Init()
{
  //TTREE INTERFACE
  //MUST DELETE WHEN FINISHED: OR ELSE DATA WON'T BE SAVED!!!

  auto app = GetApplication();

  string treeName = "tree_1p2pi";
  string treeFile = "tree_1p2pi.root";
  app->SetDefaultParameter("SRC_1P2PI:TREENAME", treeName);
  app->SetDefaultParameter("SRC_1P2PI:TREEFILE", treeFile);
  dTreeInterface = DTreeInterface::Create_DTreeInterface(treeName, treeFile);

  //TTREE BRANCHES
  DTreeBranchRegister locTreeBranchRegister;

  locTreeBranchRegister.Register_Single<Int_t>("eventNumber");
  locTreeBranchRegister.Register_Single<Int_t>("L1TriggerBits");

  locTreeBranchRegister.Register_Single<Int_t>("nPhotonCandidates");
  locTreeBranchRegister.Register_FundamentalArray<Double_t>("bmE",    "nPhotonCandidates");
  locTreeBranchRegister.Register_FundamentalArray<Double_t>("bmtime", "nPhotonCandidates");

  //locTreeBranchRegister.Register_FundamentalArray<Double_t>("RFTime_Measured", "RFTime_Measured");

  locTreeBranchRegister.Register_Single<Int_t>("nShower");
  locTreeBranchRegister.Register_Single<Int_t>("nHyp");
  locTreeBranchRegister.Register_FundamentalArray<Double_t>("pX_piminus", "nHyp");
  locTreeBranchRegister.Register_FundamentalArray<Double_t>("pY_piminus", "nHyp");
  locTreeBranchRegister.Register_FundamentalArray<Double_t>("pZ_piminus", "nHyp");
  locTreeBranchRegister.Register_FundamentalArray<Double_t>("E_piminus", "nHyp");
  locTreeBranchRegister.Register_FundamentalArray<Double_t>("t_piminus", "nHyp");

  locTreeBranchRegister.Register_FundamentalArray<Double_t>("pX_piplus", "nHyp");
  locTreeBranchRegister.Register_FundamentalArray<Double_t>("pY_piplus", "nHyp");
  locTreeBranchRegister.Register_FundamentalArray<Double_t>("pZ_piplus", "nHyp");
  locTreeBranchRegister.Register_FundamentalArray<Double_t>("E_piplus", "nHyp");
  locTreeBranchRegister.Register_FundamentalArray<Double_t>("t_piplus", "nHyp");

  locTreeBranchRegister.Register_FundamentalArray<Double_t>("pX_proton", "nHyp");
  locTreeBranchRegister.Register_FundamentalArray<Double_t>("pY_proton", "nHyp");
  locTreeBranchRegister.Register_FundamentalArray<Double_t>("pZ_proton", "nHyp");
  locTreeBranchRegister.Register_FundamentalArray<Double_t>("E_proton", "nHyp");
  locTreeBranchRegister.Register_FundamentalArray<Double_t>("t_proton", "nHyp");

  locTreeBranchRegister.Register_FundamentalArray<Double_t>("X_vertex", "nHyp");
  locTreeBranchRegister.Register_FundamentalArray<Double_t>("Y_vertex", "nHyp");
  locTreeBranchRegister.Register_FundamentalArray<Double_t>("Z_vertex", "nHyp");
  locTreeBranchRegister.Register_FundamentalArray<Double_t>("T_vertex", "nHyp");
  
  locTreeBranchRegister.Register_FundamentalArray<Double_t>("CLKinFit", "nHyp");
  locTreeBranchRegister.Register_FundamentalArray<Double_t>("NDF", "nHyp");
  locTreeBranchRegister.Register_FundamentalArray<Double_t>("ChiSqFit", "nHyp");
  locTreeBranchRegister.Register_FundamentalArray<Double_t>("Common_Time", "nHyp");

  //REGISTER BRANCHES
  dTreeInterface->Create_Branches(locTreeBranchRegister);
}

//------------------ 
// BeginRun
//------------------  
void DEventProcessor_1p2pi::BeginRun(const std::shared_ptr<const JEvent> &locEvent)
{
  dKinFitUtils = new DKinFitUtils_GlueX(locEvent);
  dKinFitter = new DKinFitter(dKinFitUtils);

  locEvent->GetSingle(dAnalysisUtilities);
}

//------------------
// Process
//------------------

void DEventProcessor_1p2pi::Process(const std::shared_ptr<const JEvent> &locEvent)
{

  vector<const DChargedTrack*>ch_tracks;
  vector<const DBeamPhoton*> beam_ph;
  vector<const DNeutralShower*> showers;
  const DTrigger* Trigger = NULL;

  locEvent->Get(ch_tracks);
  locEvent->Get(beam_ph);
  locEvent->Get(showers);
  locEvent->GetSingle(Trigger);

  if(!Trigger->Get_IsPhysicsEvent()) return; // NOERROR;
  if (ch_tracks.size()!=3) return; // NOERROR;


  map<Particle_t, int> targetParticles = {
	{Proton,1},
        {PiPlus,1},
        {PiMinus,1}
  };

   map<Particle_t, vector<const DChargedTrackHypothesis*> > emptyHypothesis;
   vector<map<Particle_t, vector<const DChargedTrackHypothesis*> > > hypothesisList;
   GetHypotheses(ch_tracks,targetParticles,emptyHypothesis,hypothesisList);
  

   Int_t _nHyp = hypothesisList.size();
    
   if(_nHyp == 0)  return; // NOERROR;

   std::lock_guard<std::mutex> guard(m_mutex); // ACQUIRE PROCESSOR LOCK

   dTreeFillData.Fill_Single<Int_t>("nHyp", _nHyp);

   //=== Photon Informatiion 
   Int_t  _nPhotonCandidates = beam_ph.size();

  dTreeFillData.Fill_Single<Int_t>("nPhotonCandidates", _nPhotonCandidates);
  
  if(_nPhotonCandidates>0){
     for(Int_t ii = 0; ii < _nPhotonCandidates; ii++){
        dTreeFillData.Fill_Array<Double_t>("bmE", beam_ph[ii]->momentum().Mag(), ii);
        dTreeFillData.Fill_Array<Double_t>("bmtime", beam_ph[ii]->time(), ii);
     }
  }
  //==========================
  //
  dTreeFillData.Fill_Single<Int_t>("eventNumber", locEvent->GetEventNumber());
  dTreeFillData.Fill_Single<Double_t>("L1TriggerBits", Trigger->Get_L1TriggerBits());


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
				 dTreeFillData.Fill_Array<Double_t>("T_vertex",  (*locParticleIterator)->Get_Time(),j);
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


		 //cout << hex << "0x" << fitProton << "  0x" << fitPiPlus << "  0x" << fitPiMinus << endl;
	 
		dTreeFillData.Fill_Array<Double_t>("X_vertex",vertex_kf[0], j);
		dTreeFillData.Fill_Array<Double_t>("Y_vertex",vertex_kf[1], j);
		dTreeFillData.Fill_Array<Double_t>("Z_vertex",vertex_kf[2], j);
		dTreeFillData.Fill_Array<Double_t>("CLKinFit",_CL, j);
		dTreeFillData.Fill_Array<Double_t>("NDF", dKinFitter->Get_NDF(), j);
		dTreeFillData.Fill_Array<Double_t>("ChiSqFit", dKinFitter->Get_ChiSq(), j);
		dTreeFillData.Fill_Array<Double_t>("Common_Time", fitPiPlus->Get_CommonTime(), j);

		dTreeFillData.Fill_Array<Double_t>("pX_piminus", fitPiMinus->Get_Momentum().X(),j);
		dTreeFillData.Fill_Array<Double_t>("pY_piminus", fitPiMinus->Get_Momentum().Y(),j);
		dTreeFillData.Fill_Array<Double_t>("pZ_piminus", fitPiMinus->Get_Momentum().Z(),j);
		dTreeFillData.Fill_Array<Double_t>("E_piminus",  fitPiMinus->Get_Energy(),j);
		dTreeFillData.Fill_Array<Double_t>("t_piminus",  fitPiMinus->Get_Time(),j);

		dTreeFillData.Fill_Array<Double_t>("pX_piplus", fitPiPlus->Get_Momentum().X(),j);
		dTreeFillData.Fill_Array<Double_t>("pY_piplus", fitPiPlus->Get_Momentum().Y(),j);
		dTreeFillData.Fill_Array<Double_t>("pZ_piplus", fitPiPlus->Get_Momentum().Z(),j);
		dTreeFillData.Fill_Array<Double_t>("E_piplus",  fitPiPlus->Get_Energy(),j);
		dTreeFillData.Fill_Array<Double_t>("t_piplus",  fitPiPlus->Get_Time(),j);

		dTreeFillData.Fill_Array<Double_t>("pX_proton", fitProton->Get_Momentum().X(),j);
		dTreeFillData.Fill_Array<Double_t>("pY_proton", fitProton->Get_Momentum().Y(),j); 
		dTreeFillData.Fill_Array<Double_t>("pZ_proton", fitProton->Get_Momentum().Z(),j);
		dTreeFillData.Fill_Array<Double_t>("E_proton",  fitProton->Get_Energy(),j);
		dTreeFillData.Fill_Array<Double_t>("t_proton",  fitProton->Get_Time(),j);

		dTreeInterface->Fill(dTreeFillData);
  

      }//CL

  }// for hyp

  //RELEASE PROCESSOR LOCK
}

//------------------
void DEventProcessor_1p2pi::EndRun()
{
	// Any final calculations on histograms (like dividing them)
	// should be done here. This may get called more than once.
}

//------------------
// Finish
//------------------

void DEventProcessor_1p2pi::Finish()
{
	delete dTreeInterface; //saves trees to file, closes file
}

// Recursive function for determining possible particle assignments

void DEventProcessor_1p2pi::GetHypotheses(vector<const DChargedTrack *> &tracks,
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

} // end GetHypotheses



