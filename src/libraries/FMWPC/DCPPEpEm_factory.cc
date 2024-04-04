// $Id$
//
//    File: DCPPEpEm_factory.cc
// Created: Thu Mar 17 14:49:42 EDT 2022
// Creator: staylor (on Linux ifarm1801.jlab.org 3.10.0-1160.11.1.el7.x86_64 x86_64)
//
/// This factory is intended to aid in the identification of e+/e- events 
/// coming from the Pb target.  For events with one positive and one negative
/// track, it performs kinematic fits for the hypotheses that the pair of tracks
/// is either e+/e-, pi+/pi-, or K+K- and saves the four-vectors for the tracks
/// after the fits along with the FCAL showers associated with the tracks.

#include <iostream>
#include <iomanip>
#include "DEPIClassifierMLPMinus.h"
#include "DEPIClassifierMLPPlus.h"
using namespace std;

#include "DCPPEpEm_factory.h"



//------------------
// Init
//------------------
void DCPPEpEm_factory::Init()
{
  auto app = GetApplication();
  
  vector< string > varsMinus( inputVarsMinus, inputVarsMinus + sizeof( inputVarsMinus )/sizeof( char* ) );
  dEPIClassifierMinus = new ReadMLPMinus( varsMinus );
  vector< string > varsPlus( inputVarsPlus, inputVarsPlus + sizeof( inputVarsPlus )/sizeof( char* ) );
  dEPIClassifierPlus = new ReadMLPPlus( varsPlus );
  
  SPLIT_CUT=0.5;
  app->SetDefaultParameter("CPPAnalysis:SPLIT_CUT",SPLIT_CUT); 
  FCAL_THRESHOLD=0.1;
  app->SetDefaultParameter("CPPAnalysis:FCAL_THRESHOLD",FCAL_THRESHOLD);
  BCAL_THRESHOLD=0.05;
  app->SetDefaultParameter("CPPAnalysis:BCAL_THRESHOLD",BCAL_THRESHOLD);
  GAMMA_DT_CUT=2.; 

  // TODO: The following needs to be replaced by a JANA resource!
  PIMU_MODEL_FILE = "/gapps/tensorflow/example_model.tflite";
  app->SetDefaultParameter("CPPAnalysis:PIMU_MODEL_FILE",PIMU_MODEL_FILE, "TFLite model file for pi/mu classification");
  
  VERBOSE=1;
  app->SetDefaultParameter("CPPAnalysis:VERBOSE", VERBOSE);

#ifdef HAVE_TENSORFLOWLITE
  // Load pi/mu classification model
  jout << "Loading pi/mu classification ML model from: " << PIMU_MODEL_FILE << endl;
  pimu_model = tflite::FlatBufferModel::BuildFromFile(PIMU_MODEL_FILE.c_str());
  if(pimu_model == nullptr) throw JException("Unable to load pi/mu tensorflow-lite model file");

  // Build the interpreter with the InterpreterBuilder.
  tflite::ops::builtin::BuiltinOpResolver resolver;
  tflite::InterpreterBuilder builder(*pimu_model, resolver);
  builder(&pimu_interpreter);
  if(pimu_interpreter == nullptr)throw JException("Error building TFLite interpretor for pi/mu model");
  
  // Allocate tensor buffers.
  if(pimu_interpreter->AllocateTensors() != kTfLiteOk) throw JException("Error allocating tensors for pi/mu model");
  if( VERBOSE >= 2 ) tflite::PrintInterpreterState(pimu_interpreter.get());

  // Get list of indexes for inputs and outputs
  auto inputs  = pimu_interpreter->inputs();
  auto outputs = pimu_interpreter->outputs();
  if( VERBOSE >= 1 ){
	 cout << "Model Inputs:" << endl;
	 for( uint32_t idx=0; idx<inputs.size(); idx++ ){
   	cout << "  " << idx << " : " << pimu_interpreter->GetInputName( idx ) << endl;
	 }
	 cout << "Model Outputs:" << endl;
	 for( uint32_t idx=0; idx<outputs.size(); idx++ ){
   	cout << "  " << idx << " : " << pimu_interpreter->GetOutputName( idx ) << endl;
	 }
  }
  if( VERBOSE >= 2 ){
	 cout << "All layer names:" << endl;
	 for( int idx=0; idx<=outputs[0]; idx++ ){
   	auto mytensor = pimu_interpreter->tensor( idx );
   	cout << "  " << idx << " : " << mytensor->name << endl;
	 }
  }
  
  // Get pointers to input and output buffers
  pimu_input  = pimu_interpreter->typed_input_tensor<float>(0);
  pimu_output = pimu_interpreter->typed_output_tensor<float>(0);
#endif // HAVE_TENSORFLOWLITE

  return;
}

//------------------
// BeginRun
//------------------
void DCPPEpEm_factory::BeginRun(const std::shared_ptr<const JEvent>& event)
{
  return;
}

//------------------
// Process
//------------------
void DCPPEpEm_factory::Process(const std::shared_ptr<const JEvent>& event)
{
  vector<const DBeamPhoton*>beamphotons;
  event->Get(beamphotons);
  if (beamphotons.size()==0) return; // RESOURCE_UNAVAILABLE;

  vector<const DChargedTrack*>tracks; 
  event->Get(tracks);
  if (tracks.size()!=2) return; // RESOURCE_UNAVAILABLE;

  // Return if we do not have 2 oppositely-charged tracks
  double q1=tracks[0]->Get_Charge(),q2=tracks[1]->Get_Charge();
  if (q1*q2>0) return; // VALUE_OUT_OF_RANGE;

  // Check that we have fitted tracks for e+/e- and pi+/pi- mass hypotheses
  const DChargedTrackHypothesis *hyp=NULL;
  unsigned int ip=(q1>q2)?0:1;
  unsigned int in=(q1>q2)?1:0;
  
  const DChargedTrackHypothesis *PiPhyp=tracks[ip]->Get_Hypothesis(PiPlus);
  if (PiPhyp==NULL) return RESOURCE_UNAVAILABLE;
  const DTrackTimeBased *piplus=PiPhyp->Get_TrackTimeBased();

  const DChargedTrackHypothesis *PiMhyp=tracks[in]->Get_Hypothesis(PiMinus);
  if (PiMhyp==NULL) return RESOURCE_UNAVAILABLE;
  const DTrackTimeBased *piminus=PiMhyp->Get_TrackTimeBased();

  hyp=tracks[ip]->Get_Hypothesis(KPlus);
  if (hyp==NULL) return; // RESOURCE_UNAVAILABLE;
  const DTrackTimeBased *kplus=hyp->Get_TrackTimeBased();

  hyp=tracks[in]->Get_Hypothesis(KMinus);
  if (hyp==NULL) return; // RESOURCE_UNAVAILABLE;
  const DTrackTimeBased *kminus=hyp->Get_TrackTimeBased();

  hyp=tracks[ip]->Get_Hypothesis(Positron);
  if (hyp==NULL) return; // RESOURCE_UNAVAILABLE;
  const DTrackTimeBased *positron=hyp->Get_TrackTimeBased();
 
   // FCAL shower associated with the e+ track
  shared_ptr<const DFCALShowerMatchParams>fcalparms=hyp->Get_FCALShowerMatchParams();
  const DFCALShower *PositronShower=NULL;
  double PositronTrackDoca=999.;
  if (fcalparms!=NULL){
    PositronTrackDoca=fcalparms->dDOCAToShower;
    PositronShower=fcalparms->dFCALShower;
  }
    
  hyp=tracks[in]->Get_Hypothesis(Electron);
  if (hyp==NULL) return; // RESOURCE_UNAVAILABLE;
  const DTrackTimeBased *electron=hyp->Get_TrackTimeBased();
  
  // FCAL shower associated with the e- track
  fcalparms=hyp->Get_FCALShowerMatchParams();
  const DFCALShower *ElectronShower=NULL;
  double ElectronTrackDoca=999.;
  if (fcalparms!=NULL){
    ElectronTrackDoca=fcalparms->dDOCAToShower;
    ElectronShower=fcalparms->dFCALShower;
  }

  vector<const DNeutralParticle*>neutrals;
  event->Get(neutrals);
  // Find t0 (at "vertex") for event
  double t0_rf=tracks[0]->Get_BestTrackingFOM()->t0();
  DVector3 vertex=tracks[0]->Get_BestTrackingFOM()->position();

  // Veto events that have non-splitoff gammas in time with the tracks
  if (VetoNeutrals(t0_rf,vertex,neutrals)==false){
    // Setup for performing kinematic fits
    DAnalysisUtilities *dAnalysisUtilities=new DAnalysisUtilities(event);
    DKinFitUtils_GlueX *dKinFitUtils = new DKinFitUtils_GlueX(event);
    DKinFitter *dKinFitter = new DKinFitter(dKinFitUtils);   
    dKinFitUtils->Reset_NewEvent();
    dKinFitter->Reset_NewEvent();
    
    // Loop over all beam photons
    for (unsigned int i=0;i<beamphotons.size();i++){
      double dt_rf=beamphotons[i]->time()-t0_rf;
      
      double weight=1.;
      bool got_beam_photon=false;
      if (fabs(dt_rf)>6.012&&fabs(dt_rf)<18.036){
	weight=-1./6.;
	got_beam_photon=true;
      }
      if (fabs(dt_rf)<2.004){
	got_beam_photon=true;
      }
      if (got_beam_photon==false) continue;

      // Create a new CPPEpEm object
      DCPPEpEm *myCPPEpEm=new DCPPEpEm;
      myCPPEpEm->Ebeam=beamphotons[i]->lorentzMomentum().E();
      myCPPEpEm->weight=weight;
      myCPPEpEm->ElectronShower=ElectronShower;
      myCPPEpEm->PositronShower=PositronShower;

      //--------------------------------
      // Kinematic fit for pi+/pi- case
      //--------------------------------
      dKinFitter->Reset_NewFit();
      DoKinematicFit(beamphotons[i],piminus,piplus,dKinFitUtils,dKinFitter,
		     dAnalysisUtilities);
      // fit quality
      myCPPEpEm->pippim_chisq=dKinFitter->Get_ChiSq();

      // Get the fitted 4-vectors
      set<shared_ptr<DKinFitParticle>>myParticles=dKinFitter->Get_KinFitParticles();
      set<shared_ptr<DKinFitParticle>>::iterator locParticleIterator=myParticles.begin();
      for(; locParticleIterator != myParticles.end(); ++locParticleIterator){
	if ((*locParticleIterator)->Get_KinFitParticleType()==d_DetectedParticle){
	  if ((*locParticleIterator)->Get_PID()==211){
	    myCPPEpEm->pip_v4=(*locParticleIterator)->Get_P4();
	  }
	  else{
	    myCPPEpEm->pim_v4=(*locParticleIterator)->Get_P4();
    	  }
	}
      }

       //--------------------------------
      // Kinematic fit for K+/K- case
      //--------------------------------
      dKinFitter->Reset_NewFit();
      DoKinematicFit(beamphotons[i],kminus,kplus,dKinFitUtils,dKinFitter,
		     dAnalysisUtilities);
        // fit quality
      myCPPEpEm->kpkm_chisq=dKinFitter->Get_ChiSq();

      // Get the fitted 4-vectors
      myParticles=dKinFitter->Get_KinFitParticles();
      locParticleIterator=myParticles.begin();
      for(; locParticleIterator != myParticles.end(); ++locParticleIterator){
	if ((*locParticleIterator)->Get_KinFitParticleType()==d_DetectedParticle){
	  if ((*locParticleIterator)->Get_PID()==-321){
	    myCPPEpEm->km_v4=(*locParticleIterator)->Get_P4();
	  }
	  else{
	    myCPPEpEm->kp_v4=(*locParticleIterator)->Get_P4();
    	  }
	}
      }

      //--------------------------------
      // Kinematic fit for e+/e- case
      //--------------------------------
      dKinFitter->Reset_NewFit();
      DoKinematicFit(beamphotons[i],electron,positron,dKinFitUtils,dKinFitter,
		     dAnalysisUtilities);
        // fit quality
      myCPPEpEm->epem_chisq=dKinFitter->Get_ChiSq();

      // Get the fitted 4-vectors
      myParticles=dKinFitter->Get_KinFitParticles();
      locParticleIterator=myParticles.begin();
      for(; locParticleIterator != myParticles.end(); ++locParticleIterator){
	if ((*locParticleIterator)->Get_KinFitParticleType()==d_DetectedParticle){
	  if ((*locParticleIterator)->Get_PID()==11){
	    myCPPEpEm->em_v4=(*locParticleIterator)->Get_P4();
	  }
	  else{
	    myCPPEpEm->ep_v4=(*locParticleIterator)->Get_P4();
    	  }
	}
      }
      
      //--------------------------------
      // Use ML model for pi/mu classification 
      //--------------------------------
      myCPPEpEm->pimu_ML_classifier = -1; // Initialize to "no info" in case anything below fails
#ifdef HAVE_TENSORFLOWLITE
      
      // Is this needed? We are creating a new interpreter for every
      // thread so in principle it is not
      //const std::lock_guard<std::mutex> pimu_lock(pimu_model_mutex);
      
      // Fill in all model features
      // n.b. if we need to use a mutex then we should pass a local
      // array for "input" and the lock the mutex just for the copy
      // to the tflite tensor.
      if( PiMuFillFeatures(loop, tracks.size(), PiPhyp, PiMhyp, pimu_input) ){

      	// Run inference
      	if( pimu_interpreter->Invoke() == kTfLiteOk){
      	  if( pimu_output ) myCPPEpEm->pimu_ML_classifier = pimu_output[0];
      	}
      }
#endif // HAVE_TENSORFLOWLITE

       //--------------------------------
      // Use ML model for pi/e classification 
      //--------------------------------
      myCPPEpEm->pipep_ML_classifier=-1;
      myCPPEpEm->pimem_ML_classifier=-1;
      if (ElectronShower){
	double EOverP=ElectronShower->getEnergy()/myCPPEpEm->em_v4.P();
	double E9E25=ElectronShower->getE9E25();
	myCPPEpEm->pipep_ML_classifier=getEPIClassifierMinus(EOverP,
							     ElectronTrackDoca,
							     E9E25);
      }
      if (PositronShower){
	double EOverP=PositronShower->getEnergy()/myCPPEpEm->ep_v4.P();
	double E9E25=PositronShower->getE9E25();
	myCPPEpEm->pimem_ML_classifier=getEPIClassifierPlus(EOverP,
							    PositronTrackDoca,
							    E9E25);
      }

      Insert(myCPPEpEm);
    }
      
    delete dAnalysisUtilities;
    delete dKinFitter;
    delete dKinFitUtils;
 
  }

  return;
}

//------------------
// EndRun
//------------------
void DCPPEpEm_factory::EndRun()
{
  return;
}

//------------------
// Finish
//------------------
void DCPPEpEm_factory::Finish()
{
  return;
}

// Run the kinematic fitter requiring energy and momentum conservation and 
// applying a vertex constraint
void DCPPEpEm_factory::DoKinematicFit(const DBeamPhoton *beamphoton,
				      const DTrackTimeBased *negative,
				      const DTrackTimeBased *positive,
				      DKinFitUtils_GlueX *dKinFitUtils,
				      DKinFitter *dKinFitter,
				      DAnalysisUtilities *dAnalysisUtilities) const {
  set<shared_ptr<DKinFitParticle>> InitialParticles, FinalParticles;
  
  shared_ptr<DKinFitParticle>myBeam=dKinFitUtils->Make_BeamParticle(beamphoton);
  shared_ptr<DKinFitParticle>myTarget=dKinFitUtils->Make_TargetParticle(Pb208);
  
  InitialParticles.insert(myBeam);  
  InitialParticles.insert(myTarget);

  shared_ptr<DKinFitParticle>myNegativeParticle=dKinFitUtils->Make_DetectedParticle(negative);
  FinalParticles.insert(myNegativeParticle);
  shared_ptr<DKinFitParticle>myPositiveParticle=dKinFitUtils->Make_DetectedParticle(positive);
  FinalParticles.insert(myPositiveParticle);
  shared_ptr<DKinFitParticle>myRecoil=dKinFitUtils->Make_MissingParticle(Pb208);
  FinalParticles.insert(myRecoil);    
  
  // make energy-momentum constraint
  shared_ptr<DKinFitConstraint_P4>locP4Constraint = dKinFitUtils->Make_P4Constraint(InitialParticles, FinalParticles);
  dKinFitter->Add_Constraint(locP4Constraint);

  // Make vertex constraint
  DVector3 vertex;
  dAnalysisUtilities->Calc_DOCAVertex(myPositiveParticle.get(),
				      myNegativeParticle.get(),vertex);
  set<shared_ptr<DKinFitParticle>> ConstrainedParticles;
  set<shared_ptr<DKinFitParticle>> NonConstrainedParticles;
  ConstrainedParticles.insert(myNegativeParticle);
  ConstrainedParticles.insert(myPositiveParticle);
  NonConstrainedParticles.insert(myBeam);  
  NonConstrainedParticles.insert(myTarget);
  NonConstrainedParticles.insert(myRecoil);
  shared_ptr<DKinFitConstraint_Vertex> locVertexConstraint = dKinFitUtils->Make_VertexConstraint(ConstrainedParticles,NonConstrainedParticles,vertex);
  dKinFitter->Add_Constraint(locVertexConstraint);
  
  // PERFORM THE KINEMATIC FIT
  dKinFitter->Fit_Reaction();
}

// Veto events that have neutral particles in time with the tracks that are not
// considered to be hadronic splitoffs
bool DCPPEpEm_factory::VetoNeutrals(double t0_rf,const DVector3 &vect,
				    vector<const DNeutralParticle*>&neutrals) const{
  
  for (unsigned int i=0;i<neutrals.size();i++){
    const DNeutralParticleHypothesis *gamHyp=neutrals[i]->Get_Hypothesis(Gamma);
    double tdiff=gamHyp->time()-t0_rf;
    if (fabs(tdiff)<GAMMA_DT_CUT){
      // Shower energy
      double E=gamHyp->lorentzMomentum().E(); 

      // Get shower info corresponding to this hypothesis
      const DNeutralShower *shower=gamHyp->Get_NeutralShower();
      if (shower->dDetectorSystem==SYS_FCAL){
	if (shower->dQuality<SPLIT_CUT) continue;
	if (E>FCAL_THRESHOLD) return true;
      }
      else if (shower->dDetectorSystem==SYS_BCAL){
	if (E>BCAL_THRESHOLD) return true;
      }
    }
  }
  
  return false;
}

// Fill the features array with values for this event.
// Return true if values are valid, false otherwise.
// e.g. return false if there is not at least 1 pi+
// and 1 pi- candidate.
bool DCPPEpEm_factory::PiMuFillFeatures(jana::JEventLoop *loop, unsigned int nChargedTracks,const DChargedTrackHypothesis *PiPhyp, const DChargedTrackHypothesis *PiMhyp, float *features){
  memset(features,0,47*sizeof(float));

  // Features list is the following:
  //  0  nChargedTracks
  //  1  nFCALShowers
  //  2  nFCALhits
  //  3  nMWPChits
  //  4  nFMWPCMatchedTracks
  //  5  FCAL_E_center_8
  //  6  FCAL_E_3x3_8
  //  7  FCAL_E_5x5_8
  //  8  FMWPC_closest_wire1_8
  //  9  FMWPC_dist_closest_wire1_8
  // 10  FMWPC_Nhits_cluster1_8
  // 11  FMWPC_closest_wire2_8
  // 12  FMWPC_dist_closest_wire2_8
  // 13  FMWPC_Nhits_cluster2_8
  // 14  FMWPC_closest_wire3_8
  // 15  FMWPC_dist_closest_wire3_8
  // 16  FMWPC_Nhits_cluster3_8
  // 17  FMWPC_closest_wire4_8
  // 18  FMWPC_dist_closest_wire4_8
  // 19  FMWPC_Nhits_cluster4_8
  // 20  FMWPC_closest_wire5_8
  // 21  FMWPC_dist_closest_wire5_8
  // 22  FMWPC_Nhits_cluster5_8
  // 23  FMWPC_closest_wire6_8
  // 24  FMWPC_dist_closest_wire6_8
  // 25  FMWPC_Nhits_cluster6_8
  // 26  FCAL_E_center_9
  // 27  FCAL_E_3x3_9
  // 28  FCAL_E_5x5_9
  // 29  FMWPC_closest_wire1_9
  // 30  FMWPC_dist_closest_wire1_9
  // 31  FMWPC_Nhits_cluster1_9
  // 32  FMWPC_closest_wire2_9
  // 33  FMWPC_dist_closest_wire2_9
  // 34  FMWPC_Nhits_cluster2_9
  // 35  FMWPC_closest_wire3_9
  // 36  FMWPC_dist_closest_wire3_9
  // 37  FMWPC_Nhits_cluster3_9
  // 38  FMWPC_closest_wire4_9
  // 39  FMWPC_dist_closest_wire4_9
  // 40  FMWPC_Nhits_cluster4_9
  // 41  FMWPC_closest_wire5_9
  // 42  FMWPC_dist_closest_wire5_9
  // 43  FMWPC_Nhits_cluster5_9
  // 44  FMWPC_closest_wire6_9
  // 45  FMWPC_dist_closest_wire6_9
  // 46  FMWPC_Nhits_cluster6_9
  
  vector<const DFCALShower*  > fcalshowers;
  vector<const DFCALHit*     > fcalhits;
  vector<const DFMWPCHit*    > fmwpchits;
  vector<const DEventHitStatistics*>stats;
  loop->Get( stats         );
  loop->Get( fcalshowers   );
  loop->Get( fcalhits      );
  loop->Get( fmwpchits     );

  features[ 0] = nChargedTracks;
  features[ 1] = fcalshowers.size();
  features[ 2] = (stats.size()>0) ? stats[0]->fcal_blocks : fcalhits.size();
  features[ 3] = fmwpchits.size();
  features[ 4] = 4; // lepton and pion tracks found 

  // Match to FCAL for pi+ hypothesis
  shared_ptr<const DFCALShowerMatchParams>fcalparms=PiPhyp->Get_FCALShowerMatchParams();
  if (fcalparms!=nullptr){
    features[ 5] = fcalparms->dEcenter;
    features[ 6] = fcalparms->dE3x3;
    features[ 7] = fcalparms->dE5x5;
  }
  // Match to FMWPCs for pi+ hypothesis
  shared_ptr<const DFMWPCMatchParams>fmwpcparms=PiPhyp->Get_FMWPCMatchParams();
  // Before training the model, Nikhil's code replaced feature values
  // where the distance to the closest wire was >30 with values used
  // to indicate no wire hit. 
  for (int ilayer=0; ilayer<6; ilayer++){
    features[ 8+3*ilayer] = -1000.0;
    features[ 9+3*ilayer] = 1000000;
    features[10+3*ilayer] = 0;
  }
  if (fmwpcparms!=nullptr){
    for (unsigned int i=0;i<fmwpcparms->dLayers.size();i++){
      if (fmwpcparms->dDists[i]<30){
	int ilayer=fmwpcparms->dLayers[i]-1;
	features[ 8+3*ilayer] = fmwpcparms->dClosestWires[i];
	features[ 9+3*ilayer] = fmwpcparms->dDists[i];
	features[10+3*ilayer] = fmwpcparms->dNhits[i];
      }
    }
  }

  // Match to FCAL for pi- hypothesis
  fcalparms=PiMhyp->Get_FCALShowerMatchParams();
  if (fcalparms!=nullptr){
    features[26] = fcalparms->dEcenter;
    features[27] = fcalparms->dE3x3;
    features[28] = fcalparms->dE5x5;
  }
  // Match to FMWPCs for pi- hypothesis
  fmwpcparms=PiMhyp->Get_FMWPCMatchParams();
  // Before training the model, Nikhil's code replaced feature values
  // where the distance to the closest wire was >30 with values used
  // to indicate no wire hit. 
  for (int ilayer=0; ilayer<6; ilayer++){
    features[29+3*ilayer] = -1000.0;
    features[30+3*ilayer] = 1000000;
    features[31+3*ilayer] = 0;
  }
  if (fmwpcparms!=nullptr){
    for (unsigned int i=0;i<fmwpcparms->dLayers.size();i++){
      if (fmwpcparms->dDists[i]<30){
	int ilayer=fmwpcparms->dLayers[i]-1;
	features[29+3*ilayer] = fmwpcparms->dClosestWires[i];
	features[30+3*ilayer] = fmwpcparms->dDists[i];
	features[31+3*ilayer] = fmwpcparms->dNhits[i];
      }
    }
  }
  
  // These are values Nikhil sent that were used for normalizing the
  // features before training the model.  
  static const float feature_min[] = {2.0,0.0,2.0,0.0,2.0,0.0,0.0,0.0,-1000.0,0.0,0.0,-1000.0,0.0,0.0,-1000.0,0.0,0.0,-1000.0,0.0,0.0,-1000.0,0.0,0.0,-1000.0,0.0,0.0,0.0,0.0,0.0,-1000.0,0.0,0.0,-1000.0,0.0,0.0,-1000.0,0.0,0.0,-1000.0,0.0,0.0,-1000.0,0.0,0.0,-1000.0,0.0,0.0,0.0};
  static const float feature_max[] = {6.0,10.0,20.0,94.0,8.0,3.924656391143799,5.177245497703552,5.349521217867732,144.0,1000000.0,39.0,144.0,1000000.0,17.0,144.0,1000000.0,12.0,144.0,1000000.0,11.0,144.0,1000000.0,8.0,144.0,1000000.0,7.0,4.154212951660156,5.578885164111853,5.9553504548966885,144.0,1000000.0,39.0,144.0,1000000.0,32.0,144.0,1000000.0,14.0,144.0,1000000.0,35.0,144.0,1000000.0,7.0,144.0,1000000.0,11.0,1.0};
  for(int i=0; i<47; i++){
    features[i] = (features[i] - feature_min[i])/(feature_max[i]-feature_min[i]);
  }

  return true;   
}

double DCPPEpEm_factory::getEPIClassifierMinus(double EoverP_minus, double FCAL_DOCA_minus, double FCAL_E9E25_minus){
  vector<double> mvaInputsMinus(3);
  //  mvaInputsMinus[0] = (ElectronShower->getEnergy())/(em_v4.P()); 
  //define EoverP_minus in loop. Call this function within loop.
  //Classifier instantiated in constructor at top of the code (is this not the correct way to do it?)
  mvaInputsMinus[0] = EoverP_minus;
  mvaInputsMinus[1] = FCAL_DOCA_minus;
  mvaInputsMinus[2] = FCAL_E9E25_minus;
  double epiMVAminus = dEPIClassifierMinus->GetMvaValue( mvaInputsMinus );
  return epiMVAminus;
}


double DCPPEpEm_factory::getEPIClassifierPlus(double EoverP_plus, double FCAL_DOCA_plus, double FCAL_E9E25_plus){
  vector<double> mvaInputsPlus(3);
  mvaInputsPlus[0] = EoverP_plus;
  mvaInputsPlus[1] = FCAL_DOCA_plus;
  mvaInputsPlus[2] = FCAL_E9E25_plus;
  double epiMVAplus = dEPIClassifierPlus->GetMvaValue( mvaInputsPlus );
  return epiMVAplus;
}

