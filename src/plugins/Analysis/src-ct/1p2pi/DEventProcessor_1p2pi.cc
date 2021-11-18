#include <map>
using namespace std;

#include "DEventProcessor_1p2pi.h"

#include <DANA/DApplication.h>



// Routine used to create our DEventProcessor
extern "C"{
  void InitPlugin(JApplication *app){
    InitJANAPlugin(app);
    app->AddProcessor(new DEventProcessor_1p2pi());
  }
} // "C"

thread_local DTreeFillData DEventProcessor_1p2pi::dTreeFillData;

//------------------
// init
///------------------

jerror_t DEventProcessor_1p2pi::init(void)
{
  //TTREE INTERFACE
  //MUST DELETE WHEN FINISHED: OR ELSE DATA WON'T BE SAVED!!!
  
  string treeName = "tree_1p2pi";
  string treeFile = "tree_1p2pi.root";
  gPARMS->SetDefaultParameter("SRC_1P2PI:TREENAME", treeName);
  gPARMS->SetDefaultParameter("SRC_1P2PI:TREEFILE", treeFile);
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
  locTreeBranchRegister.Register_FundamentalArray<Double_t>("FOM_piminuscand", "nHyp");
  locTreeBranchRegister.Register_FundamentalArray<Double_t>("NDF_piminuscand", "nHyp");
  locTreeBranchRegister.Register_FundamentalArray<Double_t>("ChiSq_piminuscand", "nHyp");
  locTreeBranchRegister.Register_FundamentalArray<Double_t>("piminus_dedx_dc_NDF", "nHyp");
  locTreeBranchRegister.Register_FundamentalArray<Double_t>("piminus_dedx_dc_ChiSq", "nHyp");
   locTreeBranchRegister.Register_FundamentalArray<Double_t>("Beta_Timing_piminuscand", "nHyp");
locTreeBranchRegister.Register_FundamentalArray<Double_t>("NDF_Timing_piminuscand", "nHyp");
locTreeBranchRegister.Register_FundamentalArray<Double_t>("ChiSq_Timing_piminuscand", "nHyp");
locTreeBranchRegister.Register_FundamentalArray<Double_t>("dEdx_TOF_piminuscand", "nHyp");
locTreeBranchRegister.Register_FundamentalArray<Double_t>("Energy_BCAL_piminuscand", "nHyp");
locTreeBranchRegister.Register_FundamentalArray<Double_t>("Energy_BCALPreshower_piminuscand", "nHyp");
locTreeBranchRegister.Register_FundamentalArray<Double_t>("SigLong_BCAL_piminuscand", "nHyp");
locTreeBranchRegister.Register_FundamentalArray<Double_t>("SigTheta_BCAL_piminuscand", "nHyp");
locTreeBranchRegister.Register_FundamentalArray<Double_t>("SigTrans_BCAL_piminuscand", "nHyp");
locTreeBranchRegister.Register_FundamentalArray<Double_t>("RMSTime_BCAL_piminuscand", "nHyp");
 locTreeBranchRegister.Register_FundamentalArray<Double_t>("E_BCAL_piminuscand", "nHyp");
 locTreeBranchRegister.Register_FundamentalArray<Double_t>("x_BCAL_piminuscand", "nHyp");
 locTreeBranchRegister.Register_FundamentalArray<Double_t>("y_BCAL_piminuscand", "nHyp");
 locTreeBranchRegister.Register_FundamentalArray<Double_t>("z_BCAL_piminuscand", "nHyp");
 locTreeBranchRegister.Register_FundamentalArray<Double_t>("t_BCAL_piminuscand", "nHyp");
locTreeBranchRegister.Register_FundamentalArray<Double_t>("Energy_FCAL_piminuscand", "nHyp");
locTreeBranchRegister.Register_FundamentalArray<Double_t>("E1E9_FCAL_piminuscand", "nHyp");
locTreeBranchRegister.Register_FundamentalArray<Double_t>("E9E25_FCAL_piminuscand", "nHyp");
locTreeBranchRegister.Register_FundamentalArray<Double_t>("SumU_FCAL_piminuscand", "nHyp");
locTreeBranchRegister.Register_FundamentalArray<Double_t>("SumV_FCAL_piminuscand", "nHyp");
locTreeBranchRegister.Register_FundamentalArray<Double_t>("TrackBCAL_DeltaPhi_piminuscand", "nHyp");
locTreeBranchRegister.Register_FundamentalArray<Double_t>("TrackBCAL_DeltaZ_piminuscand", "nHyp");
locTreeBranchRegister.Register_FundamentalArray<Double_t>("TrackFCAL_DOCA_piminuscand", "nHyp");
locTreeBranchRegister.Register_FundamentalArray<Double_t>("dEdx_ST_piminuscand", "nHyp");
 locTreeBranchRegister.Register_FundamentalArray<Double_t>("dEdx_CDC_piminuscand", "nHyp");
 locTreeBranchRegister.Register_FundamentalArray<Double_t>("dEdx_FDC_piminuscand", "nHyp");

  locTreeBranchRegister.Register_FundamentalArray<Double_t>("pX_piplus", "nHyp");
  locTreeBranchRegister.Register_FundamentalArray<Double_t>("pY_piplus", "nHyp");
  locTreeBranchRegister.Register_FundamentalArray<Double_t>("pZ_piplus", "nHyp");
  locTreeBranchRegister.Register_FundamentalArray<Double_t>("E_piplus", "nHyp");
  locTreeBranchRegister.Register_FundamentalArray<Double_t>("t_piplus", "nHyp");
	locTreeBranchRegister.Register_FundamentalArray<Double_t>("FOM_pipluscand", "nHyp");
	locTreeBranchRegister.Register_FundamentalArray<Double_t>("NDF_pipluscand", "nHyp");
	locTreeBranchRegister.Register_FundamentalArray<Double_t>("ChiSq_pipluscand", "nHyp");
	locTreeBranchRegister.Register_FundamentalArray<Double_t>("piplus_dedx_dc_ChiSq", "nHyp");
	locTreeBranchRegister.Register_FundamentalArray<Double_t>("ChiSq_Timing_pipluscand", "nHyp");
	locTreeBranchRegister.Register_FundamentalArray<Double_t>("dEdx_TOF_pipluscand", "nHyp");
	locTreeBranchRegister.Register_FundamentalArray<Double_t>("dEdx_TOF_pipluscand", "nHyp");
	locTreeBranchRegister.Register_FundamentalArray<Double_t>("dEdx_ST_pipluscand", "nHyp");
	locTreeBranchRegister.Register_FundamentalArray<Double_t>("Energy_BCAL_pipluscand", "nHyp");
	locTreeBranchRegister.Register_FundamentalArray<Double_t>("Energy_BCALPreshower_pipluscand", "nHyp");
	locTreeBranchRegister.Register_FundamentalArray<Double_t>("SigLong_BCAL_pipluscand", "nHyp");
	locTreeBranchRegister.Register_FundamentalArray<Double_t>("SigTheta_BCAL_pipluscand", "nHyp");
	locTreeBranchRegister.Register_FundamentalArray<Double_t>("SigTrans_BCAL_pipluscand", "nHyp");
	locTreeBranchRegister.Register_FundamentalArray<Double_t>("Energy_FCAL_pipluscand", "nHyp");
	locTreeBranchRegister.Register_FundamentalArray<Double_t>("E1E9_FCAL_pipluscand", "nHyp");
	locTreeBranchRegister.Register_FundamentalArray<Double_t>("E9E25_FCAL_pipluscand", "nHyp");
	locTreeBranchRegister.Register_FundamentalArray<Double_t>("SumU_FCAL_pipluscand", "nHyp");
	locTreeBranchRegister.Register_FundamentalArray<Double_t>("TrackBCAL_DeltaPhi_pipluscand", "nHyp");
    locTreeBranchRegister.Register_FundamentalArray<Double_t>("TrackBCAL_DeltaZ_pipluscand", "nHyp");
 locTreeBranchRegister.Register_FundamentalArray<Double_t>("Beta_Timing_pipluscand", "nHyp");
  locTreeBranchRegister.Register_FundamentalArray<Double_t>("NDF_Timing_pipluscand", "nHyp");
   locTreeBranchRegister.Register_FundamentalArray<Double_t>("RMSTime_BCAL_pipluscand", "nHyp");
   locTreeBranchRegister.Register_FundamentalArray<Double_t>("E_BCAL_pipluscand", "nHyp");
   locTreeBranchRegister.Register_FundamentalArray<Double_t>("x_BCAL_pipluscand", "nHyp");
   locTreeBranchRegister.Register_FundamentalArray<Double_t>("y_BCAL_pipluscand", "nHyp");
   locTreeBranchRegister.Register_FundamentalArray<Double_t>("z_BCAL_pipluscand", "nHyp");
   locTreeBranchRegister.Register_FundamentalArray<Double_t>("t_BCAL_pipluscand", "nHyp");
    locTreeBranchRegister.Register_FundamentalArray<Double_t>("SumV_FCAL_pipluscand", "nHyp");
     locTreeBranchRegister.Register_FundamentalArray<Double_t>("TrackFCAL_DOCA_pipluscand", "nHyp");         
         locTreeBranchRegister.Register_FundamentalArray<Double_t>("piplus_dedx_dc_NDF", "nHyp");
	 locTreeBranchRegister.Register_FundamentalArray<Double_t>("dEdx_CDC_pipluscand", "nHyp");
	 locTreeBranchRegister.Register_FundamentalArray<Double_t>("dEdx_FDC_pipluscand", "nHyp");


  locTreeBranchRegister.Register_FundamentalArray<Double_t>("pX_proton", "nHyp");
  locTreeBranchRegister.Register_FundamentalArray<Double_t>("pY_proton", "nHyp");
  locTreeBranchRegister.Register_FundamentalArray<Double_t>("pZ_proton", "nHyp");
  locTreeBranchRegister.Register_FundamentalArray<Double_t>("E_proton", "nHyp");
  locTreeBranchRegister.Register_FundamentalArray<Double_t>("t_proton", "nHyp");
locTreeBranchRegister.Register_FundamentalArray<Double_t>("FOM_protcand", "nHyp");
locTreeBranchRegister.Register_FundamentalArray<Double_t>("NDF_protcand", "nHyp");
locTreeBranchRegister.Register_FundamentalArray<Double_t>("ChiSq_protcand", "nHyp");
locTreeBranchRegister.Register_FundamentalArray<Double_t>("prot_dedx_dc_NDF", "nHyp");
locTreeBranchRegister.Register_FundamentalArray<Double_t>("prot_dedx_dc_ChiSq", "nHyp");
locTreeBranchRegister.Register_FundamentalArray<Double_t>("Beta_Timing_protcand", "nHyp");
locTreeBranchRegister.Register_FundamentalArray<Double_t>("NDF_Timing_protcand", "nHyp");
locTreeBranchRegister.Register_FundamentalArray<Double_t>("ChiSq_Timing_protcan", "nHyp");
locTreeBranchRegister.Register_FundamentalArray<Double_t>("dEdx_TOF_protcand", "nHyp");
locTreeBranchRegister.Register_FundamentalArray<Double_t>("Energy_BCAL_protcand", "nHyp");
locTreeBranchRegister.Register_FundamentalArray<Double_t>("Energy_BCALPreshower_protcand", "nHyp");
locTreeBranchRegister.Register_FundamentalArray<Double_t>("SigLong_BCAL_protcand", "nHyp");
locTreeBranchRegister.Register_FundamentalArray<Double_t>("SigTheta_BCAL_protcand", "nHyp");
locTreeBranchRegister.Register_FundamentalArray<Double_t>("SigTrans_BCAL_protcand", "nHyp");
locTreeBranchRegister.Register_FundamentalArray<Double_t>("RMSTime_BCAL_protcand", "nHyp");
 locTreeBranchRegister.Register_FundamentalArray<Double_t>("E_BCAL_protcand", "nHyp");
 locTreeBranchRegister.Register_FundamentalArray<Double_t>("x_BCAL_protcand", "nHyp");
 locTreeBranchRegister.Register_FundamentalArray<Double_t>("y_BCAL_protcand", "nHyp");
 locTreeBranchRegister.Register_FundamentalArray<Double_t>("z_BCAL_protcand", "nHyp");
 locTreeBranchRegister.Register_FundamentalArray<Double_t>("t_BCAL_protcand", "nHyp");
locTreeBranchRegister.Register_FundamentalArray<Double_t>("SigTrans_BCAL_protcand", "nHyp");
locTreeBranchRegister.Register_FundamentalArray<Double_t>("Energy_FCAL_protcand", "nHyp");
locTreeBranchRegister.Register_FundamentalArray<Double_t>("SigTrans_BCAL_protcand", "nHyp");
locTreeBranchRegister.Register_FundamentalArray<Double_t>("E1E9_FCAL_protcand", "nHyp");
locTreeBranchRegister.Register_FundamentalArray<Double_t>("E9E25_FCAL_protcand", "nHyp");
locTreeBranchRegister.Register_FundamentalArray<Double_t>("SumU_FCAL_protcand", "nHyp");
locTreeBranchRegister.Register_FundamentalArray<Double_t>("SumV_FCAL_protcand", "nHyp");
locTreeBranchRegister.Register_FundamentalArray<Double_t>("dEdx_ST_protcand", "nHyp");
 locTreeBranchRegister.Register_FundamentalArray<Double_t>("dEdx_CDC_protcand", "nHyp");
 locTreeBranchRegister.Register_FundamentalArray<Double_t>("dEdx_FDC_protcand", "nHyp");

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

  return NOERROR;
}

//------------------ 
// brun
//------------------  
jerror_t DEventProcessor_1p2pi::brun(JEventLoop *eventLoop, int32_t runnumber)
{
  dKinFitUtils = new DKinFitUtils_GlueX(eventLoop);
  dKinFitter = new DKinFitter(dKinFitUtils);

  eventLoop->GetSingle(dAnalysisUtilities);

  return NOERROR;
}

//------------------
// evnt
//------------------

jerror_t DEventProcessor_1p2pi::evnt(JEventLoop *loop, uint64_t eventnumber)
{

  vector<const DChargedTrack*>ch_tracks;
  vector<const DBeamPhoton*> beam_ph;
  vector<const DNeutralShower*> showers;
  const DTrigger* Trigger = NULL;

  loop->Get(ch_tracks);
  loop->Get(beam_ph);
  loop->Get(showers);
  loop->GetSingle(Trigger);

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

   LockState(); //ACQUIRE PROCESSOR LOCK

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
  dTreeFillData.Fill_Single<Int_t>("eventNumber", eventnumber); 
  dTreeFillData.Fill_Single<Double_t>("L1TriggerBits", Trigger->Get_L1TriggerBits());
  dTreeFillData.Fill_Single<Int_t>("nShower",showers.size());


  dKinFitUtils->Reset_NewEvent();
  dKinFitter->Reset_NewEvent();


  dKinFitUtils->Reset_NewEvent();
  dKinFitter->Reset_NewEvent();

  for (Int_t j = 0; j < _nHyp; j++){
  
     map<Particle_t, vector<const DChargedTrackHypothesis*> > thisHyp = hypothesisList[j];

     //============ PiMinus Hypothesis ===========================================================//
     const DChargedTrackHypothesis *hyp_pi_min   = thisHyp[PiMinus][0];
     const DTrackTimeBased *pi_min_track = hyp_pi_min->Get_TrackTimeBased();            	    
     double FOM_piminuscand          = hyp_pi_min->Get_FOM();
     double NDF_piminuscand          = hyp_pi_min->Get_NDF();
     double ChiSq_piminuscand        = hyp_pi_min->Get_ChiSq();
     double piminus_dedx_dc_NDF      = hyp_pi_min->Get_NDF_DCdEdx();
     double piminus_dedx_dc_ChiSq    = hyp_pi_min->Get_ChiSq_DCdEdx();
     double Beta_Timing_piminuscand  = hyp_pi_min->measuredBeta();
     double NDF_Timing_piminuscand   = hyp_pi_min->Get_NDF_Timing();
     double ChiSq_Timing_piminuscand = hyp_pi_min->Get_ChiSq_Timing();
     double dEdx_TOF_piminuscand     = (hyp_pi_min->Get_TOFHitMatchParams() != NULL) ? hyp_pi_min->Get_TOFHitMatchParams()->dEdx : 0.0;
     double dEdx_ST_piminuscand      = (hyp_pi_min->Get_SCHitMatchParams()  != NULL) ? hyp_pi_min->Get_SCHitMatchParams()->dEdx : 0.0;
     double dEdx_CDC_piminuscand      = (hyp_pi_min->Get_TrackTimeBased() != NULL) ? hyp_pi_min->Get_TrackTimeBased()->ddEdx_CDC : 0.0;
     double dEdx_FDC_piminuscand      = (hyp_pi_min->Get_TrackTimeBased() != NULL) ? hyp_pi_min->Get_TrackTimeBased()->ddEdx_FDC : 0.0;
     const DBCALShower* showerpiminus = NULL;
     if (hyp_pi_min->Get_BCALShowerMatchParams() != NULL) showerpiminus = hyp_pi_min->Get_BCALShowerMatchParams()->dBCALShower;
     double Energy_BCAL_piminuscand  = (showerpiminus != NULL ) ? showerpiminus->E : 0.0;
     double Energy_BCALPreshower_piminuscand  = (showerpiminus != NULL ) ? showerpiminus->E_preshower : 0.0;
     double SigLong_BCAL_piminuscand = (showerpiminus != NULL ) ? showerpiminus->sigLong : 0.0;
     double SigTheta_BCAL_piminuscand= (showerpiminus != NULL ) ? showerpiminus->sigTheta : 0.0;
     double SigTrans_BCAL_piminuscand= (showerpiminus != NULL ) ? showerpiminus->sigTrans : 0.0;
     double RMSTime_BCAL_piminuscand = (showerpiminus != NULL ) ? showerpiminus->rmsTime : 0.0;
     double E_BCAL_piminuscand = (showerpiminus != NULL ) ? showerpiminus->E : 0.0;
     double x_BCAL_piminuscand = (showerpiminus != NULL ) ? showerpiminus->x : 0.0;
     double y_BCAL_piminuscand = (showerpiminus != NULL ) ? showerpiminus->y : 0.0;
     double z_BCAL_piminuscand = (showerpiminus != NULL ) ? showerpiminus->z : 0.0;
     double t_BCAL_piminuscand = (showerpiminus != NULL ) ? showerpiminus->t : 0.0;

     const DFCALShower* FCALshowerpiminus = NULL;
     if (hyp_pi_min->Get_FCALShowerMatchParams() != NULL) FCALshowerpiminus = hyp_pi_min->Get_FCALShowerMatchParams()->dFCALShower;
     double Energy_FCAL_piminuscand = (FCALshowerpiminus != NULL) ? FCALshowerpiminus->getEnergy() : 0.0;
     double E1E9_FCAL_piminuscand   = (FCALshowerpiminus != NULL) ? FCALshowerpiminus->getE1E9() : 0.0;
     double E9E25_FCAL_piminuscand  = (FCALshowerpiminus != NULL) ? FCALshowerpiminus->getE9E25() : 0.0;
     double SumU_FCAL_piminuscand   = (FCALshowerpiminus != NULL) ? FCALshowerpiminus->getSumU() : 0.0;
     double SumV_FCAL_piminuscand   = (FCALshowerpiminus != NULL) ? FCALshowerpiminus->getSumV() : 0.0;

     double TrackBCAL_DeltaPhi_piminuscand = ( hyp_pi_min->Get_BCALShowerMatchParams()  != NULL) ? hyp_pi_min->Get_BCALShowerMatchParams()->dDeltaPhiToShower : 999.0;
     double TrackBCAL_DeltaZ_piminuscand   = ( hyp_pi_min->Get_BCALShowerMatchParams()  != NULL) ? hyp_pi_min->Get_BCALShowerMatchParams()->dDeltaZToShower : 999.0;
     double TrackFCAL_DOCA_piminuscand     = ( hyp_pi_min->Get_FCALShowerMatchParams()  != NULL) ? hyp_pi_min->Get_FCALShowerMatchParams()->dDOCAToShower : 999.0;  

     dTreeFillData.Fill_Array<Double_t>("FOM_piminuscand",FOM_piminuscand, j);
     dTreeFillData.Fill_Array<Double_t>("NDF_piminuscand",NDF_piminuscand, j);
     dTreeFillData.Fill_Array<Double_t>("ChiSq_piminuscand",ChiSq_piminuscand, j);
     dTreeFillData.Fill_Array<Double_t>("piminus_dedx_dc_NDF",piminus_dedx_dc_NDF, j);
     dTreeFillData.Fill_Array<Double_t>("piminus_dedx_dc_ChiSq",piminus_dedx_dc_ChiSq, j);
     dTreeFillData.Fill_Array<Double_t>("Beta_Timing_piminuscand",Beta_Timing_piminuscand, j);
     dTreeFillData.Fill_Array<Double_t>("NDF_Timing_piminuscand",NDF_Timing_piminuscand, j);
     dTreeFillData.Fill_Array<Double_t>("ChiSq_Timing_piminuscand",ChiSq_Timing_piminuscand, j);
     dTreeFillData.Fill_Array<Double_t>("ChiSq_Timing_piminuscand",ChiSq_Timing_piminuscand, j);
     dTreeFillData.Fill_Array<Double_t>("dEdx_TOF_piminuscand",dEdx_TOF_piminuscand, j);
     dTreeFillData.Fill_Array<Double_t>("dEdx_ST_piminuscand",dEdx_ST_piminuscand, j);	
     dTreeFillData.Fill_Array<Double_t>("dEdx_CDC_piminuscand",dEdx_CDC_piminuscand, j);
     dTreeFillData.Fill_Array<Double_t>("dEdx_FDC_piminuscand",dEdx_FDC_piminuscand, j);
     dTreeFillData.Fill_Array<Double_t>("Energy_BCAL_piminuscand",Energy_BCAL_piminuscand, j);
     dTreeFillData.Fill_Array<Double_t>("Energy_BCALPreshower_piminuscand",Energy_BCALPreshower_piminuscand, j);
     dTreeFillData.Fill_Array<Double_t>("SigLong_BCAL_piminuscand",SigLong_BCAL_piminuscand, j); 
     dTreeFillData.Fill_Array<Double_t>("SigTheta_BCAL_piminuscand",SigTheta_BCAL_piminuscand, j);
     dTreeFillData.Fill_Array<Double_t>("SigTrans_BCAL_piminuscand",SigTrans_BCAL_piminuscand, j);
     dTreeFillData.Fill_Array<Double_t>("RMSTime_BCAL_piminuscand",RMSTime_BCAL_piminuscand, j);
     dTreeFillData.Fill_Array<Double_t>("E_BCAL_piminuscand",E_BCAL_piminuscand, j);
     dTreeFillData.Fill_Array<Double_t>("x_BCAL_piminuscand",x_BCAL_piminuscand, j);
     dTreeFillData.Fill_Array<Double_t>("y_BCAL_piminuscand",y_BCAL_piminuscand, j);
     dTreeFillData.Fill_Array<Double_t>("z_BCAL_piminuscand",z_BCAL_piminuscand, j);
     dTreeFillData.Fill_Array<Double_t>("t_BCAL_piminuscand",t_BCAL_piminuscand, j);
     dTreeFillData.Fill_Array<Double_t>("Energy_FCAL_piminuscand",Energy_FCAL_piminuscand, j);
     dTreeFillData.Fill_Array<Double_t>("E1E9_FCAL_piminuscand",E1E9_FCAL_piminuscand, j);
     dTreeFillData.Fill_Array<Double_t>("E9E25_FCAL_piminuscand",E9E25_FCAL_piminuscand, j);
     dTreeFillData.Fill_Array<Double_t>("SumU_FCAL_piminuscand",SumU_FCAL_piminuscand, j);
     dTreeFillData.Fill_Array<Double_t>("SumV_FCAL_piminuscand",SumV_FCAL_piminuscand, j);
     dTreeFillData.Fill_Array<Double_t>("TrackBCAL_DeltaPhi_piminuscand",TrackBCAL_DeltaPhi_piminuscand, j);
     dTreeFillData.Fill_Array<Double_t>("TrackBCAL_DeltaZ_piminuscand",TrackBCAL_DeltaZ_piminuscand, j);
     dTreeFillData.Fill_Array<Double_t>("TrackFCAL_DOCA_piminuscand",TrackFCAL_DOCA_piminuscand, j);

    //============ PiPlus Hypothesis ===========================================================//
    const DChargedTrackHypothesis *hyp_pi_plus   = thisHyp[PiPlus][0];
    const DTrackTimeBased *pi_plus_track = hyp_pi_plus->Get_TrackTimeBased();
    double FOM_pipluscand = hyp_pi_plus->Get_FOM();
    double NDF_pipluscand = hyp_pi_plus->Get_NDF();
    double ChiSq_pipluscand = hyp_pi_plus->Get_ChiSq();
    double piplus_dedx_dc_NDF = hyp_pi_plus->Get_NDF_DCdEdx();
    double piplus_dedx_dc_ChiSq  = hyp_pi_plus->Get_ChiSq_DCdEdx();
    double Beta_Timing_pipluscand = hyp_pi_plus->measuredBeta();
    double NDF_Timing_pipluscand   = hyp_pi_plus->Get_NDF_Timing();
    double ChiSq_Timing_pipluscand = hyp_pi_plus->Get_ChiSq_Timing();
    double dEdx_TOF_pipluscand = (hyp_pi_plus->Get_TOFHitMatchParams() != NULL) ? hyp_pi_plus->Get_TOFHitMatchParams()->dEdx : 0.0;
    double dEdx_ST_pipluscand  = (hyp_pi_plus->Get_SCHitMatchParams()  != NULL) ? hyp_pi_plus->Get_SCHitMatchParams()->dEdx : 0.0;
    double dEdx_CDC_pipluscand      = (hyp_pi_plus->Get_TrackTimeBased() != NULL) ? hyp_pi_plus->Get_TrackTimeBased()->ddEdx_CDC : 0.0;
    double dEdx_FDC_pipluscand      = (hyp_pi_plus->Get_TrackTimeBased() != NULL) ? hyp_pi_plus->Get_TrackTimeBased()->ddEdx_FDC : 0.0;
    const DBCALShower* showerpiplus = NULL;
    if (hyp_pi_plus->Get_BCALShowerMatchParams() != NULL) showerpiplus = hyp_pi_plus->Get_BCALShowerMatchParams()->dBCALShower;
    double Energy_BCAL_pipluscand  = (showerpiplus != NULL ) ? showerpiplus->E : 0.0;
    double Energy_BCALPreshower_pipluscand  = (showerpiplus != NULL ) ? showerpiplus->E_preshower : 0.0;
    double SigLong_BCAL_pipluscand = (showerpiplus != NULL ) ? showerpiplus->sigLong : 0.0;
    double SigTheta_BCAL_pipluscand = (showerpiplus != NULL ) ? showerpiplus->sigTheta : 0.0;
    double SigTrans_BCAL_pipluscand = (showerpiplus != NULL ) ? showerpiplus->sigTrans : 0.0;
    double RMSTime_BCAL_pipluscand = (showerpiplus != NULL ) ? showerpiplus->rmsTime : 0.0;
    double E_BCAL_pipluscand = (showerpiplus != NULL ) ? showerpiplus->E : 0.0;
    double x_BCAL_pipluscand = (showerpiplus != NULL ) ? showerpiplus->x : 0.0;
    double y_BCAL_pipluscand = (showerpiplus != NULL ) ? showerpiplus->y : 0.0;
    double z_BCAL_pipluscand = (showerpiplus != NULL ) ? showerpiplus->z : 0.0;
    double t_BCAL_pipluscand = (showerpiplus != NULL ) ? showerpiplus->t : 0.0;

    const DFCALShower* FCALshowerpiplus = NULL;
    if (hyp_pi_plus->Get_FCALShowerMatchParams() != NULL) FCALshowerpiplus = hyp_pi_plus->Get_FCALShowerMatchParams()->dFCALShower;
    double Energy_FCAL_pipluscand = (FCALshowerpiplus != NULL) ? FCALshowerpiplus->getEnergy() : 0.0;
    double E1E9_FCAL_pipluscand = (FCALshowerpiplus != NULL) ? FCALshowerpiplus->getE1E9() : 0.0;
    double E9E25_FCAL_pipluscand = (FCALshowerpiplus != NULL) ? FCALshowerpiplus->getE9E25() : 0.0;
    double SumU_FCAL_pipluscand = (FCALshowerpiplus != NULL) ? FCALshowerpiplus->getSumU() : 0.0;
    double SumV_FCAL_pipluscand = (FCALshowerpiplus != NULL) ? FCALshowerpiplus->getSumV() : 0.0;

    double TrackBCAL_DeltaPhi_pipluscand = ( hyp_pi_plus->Get_BCALShowerMatchParams()  != NULL) ? hyp_pi_plus->Get_BCALShowerMatchParams()->dDeltaPhiToShower : 999.0;
    double TrackBCAL_DeltaZ_pipluscand   = ( hyp_pi_plus->Get_BCALShowerMatchParams()  != NULL) ? hyp_pi_plus->Get_BCALShowerMatchParams()->dDeltaZToShower : 999.0;
    double TrackFCAL_DOCA_pipluscand     = ( hyp_pi_plus->Get_FCALShowerMatchParams()  != NULL) ? hyp_pi_plus->Get_FCALShowerMatchParams()->dDOCAToShower : 999.0;

    dTreeFillData.Fill_Array<Double_t>("FOM_pipluscand",FOM_pipluscand, j);
    dTreeFillData.Fill_Array<Double_t>("NDF_pipluscand",NDF_pipluscand, j);
    dTreeFillData.Fill_Array<Double_t>("ChiSq_pipluscand",ChiSq_pipluscand, j);
    dTreeFillData.Fill_Array<Double_t>("piplus_dedx_dc_NDF",piplus_dedx_dc_NDF, j);
    dTreeFillData.Fill_Array<Double_t>("piplus_dedx_dc_ChiSq",piplus_dedx_dc_ChiSq, j);
    dTreeFillData.Fill_Array<Double_t>("Beta_Timing_pipluscand",Beta_Timing_pipluscand, j);
    dTreeFillData.Fill_Array<Double_t>("NDF_Timing_pipluscand",NDF_Timing_pipluscand, j);
    dTreeFillData.Fill_Array<Double_t>("ChiSq_Timing_pipluscand",ChiSq_Timing_pipluscand, j);
    dTreeFillData.Fill_Array<Double_t>("dEdx_TOF_pipluscand",dEdx_TOF_pipluscand, j);
    dTreeFillData.Fill_Array<Double_t>("dEdx_ST_pipluscand",dEdx_ST_pipluscand, j);
    dTreeFillData.Fill_Array<Double_t>("dEdx_CDC_pipluscand",dEdx_CDC_pipluscand, j);
    dTreeFillData.Fill_Array<Double_t>("dEdx_FDC_pipluscand",dEdx_FDC_pipluscand, j);
    dTreeFillData.Fill_Array<Double_t>("Energy_BCAL_pipluscand",Energy_BCAL_pipluscand, j);
    dTreeFillData.Fill_Array<Double_t>("Energy_BCALPreshower_pipluscand",Energy_BCALPreshower_pipluscand, j);
    dTreeFillData.Fill_Array<Double_t>("SigLong_BCAL_pipluscand",SigLong_BCAL_pipluscand, j);
    dTreeFillData.Fill_Array<Double_t>("SigTheta_BCAL_pipluscand",SigTheta_BCAL_pipluscand, j);
    dTreeFillData.Fill_Array<Double_t>("SigTrans_BCAL_pipluscand",SigTrans_BCAL_pipluscand, j);
    dTreeFillData.Fill_Array<Double_t>("RMSTime_BCAL_pipluscand",RMSTime_BCAL_pipluscand, j);
    dTreeFillData.Fill_Array<Double_t>("E_BCAL_pipluscand",E_BCAL_pipluscand, j);
    dTreeFillData.Fill_Array<Double_t>("x_BCAL_pipluscand",x_BCAL_pipluscand, j);
    dTreeFillData.Fill_Array<Double_t>("y_BCAL_pipluscand",y_BCAL_pipluscand, j);
    dTreeFillData.Fill_Array<Double_t>("z_BCAL_pipluscand",z_BCAL_pipluscand, j);
    dTreeFillData.Fill_Array<Double_t>("t_BCAL_pipluscand",t_BCAL_pipluscand, j);
    dTreeFillData.Fill_Array<Double_t>("Energy_FCAL_pipluscand",Energy_FCAL_pipluscand, j);
    dTreeFillData.Fill_Array<Double_t>("E1E9_FCAL_pipluscand",E1E9_FCAL_pipluscand, j);
    dTreeFillData.Fill_Array<Double_t>("E9E25_FCAL_pipluscand",E9E25_FCAL_pipluscand, j);
    dTreeFillData.Fill_Array<Double_t>("SumU_FCAL_pipluscand",SumU_FCAL_pipluscand, j);
    dTreeFillData.Fill_Array<Double_t>("SumV_FCAL_pipluscand",SumV_FCAL_pipluscand, j);
    dTreeFillData.Fill_Array<Double_t>("TrackBCAL_DeltaPhi_pipluscand",TrackBCAL_DeltaPhi_pipluscand, j);
    dTreeFillData.Fill_Array<Double_t>("TrackBCAL_DeltaZ_pipluscand",TrackBCAL_DeltaZ_pipluscand, j);
    dTreeFillData.Fill_Array<Double_t>("TrackFCAL_DOCA_pipluscand",TrackFCAL_DOCA_pipluscand, j);
     //============ Proton Hypothesis ===========================================================//
     const DChargedTrackHypothesis *hyp_proton   = thisHyp[Proton][0];
     const DTrackTimeBased *proton_track = hyp_proton->Get_TrackTimeBased();	    
    
   double FOM_protcand = hyp_proton->Get_FOM();
   double NDF_protcand = hyp_proton->Get_NDF();
   double ChiSq_protcand = hyp_proton->Get_ChiSq();
   double prot_dedx_dc_NDF  = hyp_proton->Get_NDF_DCdEdx();
   double prot_dedx_dc_ChiSq  = hyp_proton->Get_ChiSq_DCdEdx();
   double Beta_Timing_protcand = hyp_proton->measuredBeta();
   double NDF_Timing_protcand   = hyp_proton->Get_NDF_Timing();
   double ChiSq_Timing_protcan = hyp_proton->Get_ChiSq_Timing();
   double dEdx_TOF_protcand = (hyp_proton->Get_TOFHitMatchParams() != NULL) ? hyp_proton->Get_TOFHitMatchParams()->dEdx : 0.0;
   double dEdx_ST_protcand  = (hyp_proton->Get_SCHitMatchParams()  != NULL) ? hyp_proton->Get_SCHitMatchParams()->dEdx : 0.0;
   double dEdx_CDC_protcand      = (hyp_proton->Get_TrackTimeBased() != NULL) ? hyp_proton->Get_TrackTimeBased()->ddEdx_CDC : 0.0;
   double dEdx_FDC_protcand      = (hyp_proton->Get_TrackTimeBased() != NULL) ? hyp_proton->Get_TrackTimeBased()->ddEdx_FDC : 0.0;
   const DBCALShower* showerproton = NULL;
   if (hyp_proton->Get_BCALShowerMatchParams() != NULL) showerproton = hyp_proton->Get_BCALShowerMatchParams()->dBCALShower;
   double Energy_BCAL_protcand  = (showerproton != NULL ) ? showerproton->E : 0.0;
   double Energy_BCALPreshower_protcand  = (showerproton != NULL ) ? showerproton->E_preshower : 0.0;
   double SigLong_BCAL_protcand = (showerproton != NULL ) ? showerproton->sigLong : 0.0;
   double SigTheta_BCAL_protcand = (showerproton != NULL ) ? showerproton->sigTheta : 0.0;
   double SigTrans_BCAL_protcand = (showerproton != NULL ) ? showerproton->sigTrans : 0.0;
   double RMSTime_BCAL_protcand  = (showerproton != NULL ) ? showerproton->rmsTime : 0.0;
   double E_BCAL_protcand = (showerproton != NULL ) ? showerproton->E : 0.0;
   double x_BCAL_protcand = (showerproton != NULL ) ? showerproton->x : 0.0;
   double y_BCAL_protcand = (showerproton != NULL ) ? showerproton->y : 0.0;
   double z_BCAL_protcand = (showerproton != NULL ) ? showerproton->z : 0.0;
   double t_BCAL_protcand = (showerproton != NULL ) ? showerproton->t : 0.0;

   const DFCALShower* FCALshowerproton = NULL;
   if (hyp_proton->Get_FCALShowerMatchParams() != NULL) FCALshowerproton = hyp_proton->Get_FCALShowerMatchParams()->dFCALShower;
   double Energy_FCAL_protcand = (FCALshowerproton != NULL) ? FCALshowerproton->getEnergy() : 0.0;
   double E1E9_FCAL_protcand = (FCALshowerproton != NULL) ? FCALshowerproton->getE1E9() : 0.0;
   double E9E25_FCAL_protcand = (FCALshowerproton != NULL) ? FCALshowerproton->getE9E25() : 0.0;
   double SumU_FCAL_protcand = (FCALshowerproton != NULL) ? FCALshowerproton->getSumU() : 0.0;
   double SumV_FCAL_protcand = (FCALshowerproton != NULL) ? FCALshowerproton->getSumV() : 0.0;

  dTreeFillData.Fill_Array<Double_t>("FOM_protcand",FOM_protcand, j);
dTreeFillData.Fill_Array<Double_t>("NDF_protcand",NDF_protcand, j);
dTreeFillData.Fill_Array<Double_t>("ChiSq_protcand",ChiSq_protcand, j);
dTreeFillData.Fill_Array<Double_t>("prot_dedx_dc_NDF",prot_dedx_dc_NDF, j);
dTreeFillData.Fill_Array<Double_t>("prot_dedx_dc_ChiSq",prot_dedx_dc_ChiSq, j);
dTreeFillData.Fill_Array<Double_t>("Beta_Timing_protcand",Beta_Timing_protcand, j);
dTreeFillData.Fill_Array<Double_t>("NDF_Timing_protcand",NDF_Timing_protcand, j);
dTreeFillData.Fill_Array<Double_t>("ChiSq_Timing_protcan",ChiSq_Timing_protcan, j);
dTreeFillData.Fill_Array<Double_t>("dEdx_TOF_protcand",dEdx_TOF_protcand, j);
dTreeFillData.Fill_Array<Double_t>("dEdx_ST_protcand",dEdx_ST_protcand, j);
 dTreeFillData.Fill_Array<Double_t>("dEdx_CDC_protcand",dEdx_CDC_protcand, j);
 dTreeFillData.Fill_Array<Double_t>("dEdx_FDC_protcand",dEdx_FDC_protcand, j);
dTreeFillData.Fill_Array<Double_t>("Energy_BCAL_protcand",Energy_BCAL_protcand, j);
dTreeFillData.Fill_Array<Double_t>("Energy_BCALPreshower_protcand",Energy_BCALPreshower_protcand, j);
dTreeFillData.Fill_Array<Double_t>("SigLong_BCAL_protcand",SigLong_BCAL_protcand, j);
dTreeFillData.Fill_Array<Double_t>("SigTheta_BCAL_protcand",SigTheta_BCAL_protcand, j);
dTreeFillData.Fill_Array<Double_t>("SigTrans_BCAL_protcand",SigTrans_BCAL_protcand, j);
dTreeFillData.Fill_Array<Double_t>("RMSTime_BCAL_protcand",RMSTime_BCAL_protcand, j);
 dTreeFillData.Fill_Array<Double_t>("E_BCAL_protcand",E_BCAL_protcand, j);
 dTreeFillData.Fill_Array<Double_t>("x_BCAL_protcand",x_BCAL_protcand, j);
 dTreeFillData.Fill_Array<Double_t>("y_BCAL_protcand",y_BCAL_protcand, j);
 dTreeFillData.Fill_Array<Double_t>("z_BCAL_protcand",z_BCAL_protcand, j);
 dTreeFillData.Fill_Array<Double_t>("t_BCAL_protcand",t_BCAL_protcand, j);
dTreeFillData.Fill_Array<Double_t>("Energy_FCAL_protcand",Energy_FCAL_protcand, j);
dTreeFillData.Fill_Array<Double_t>("E1E9_FCAL_protcand",E1E9_FCAL_protcand, j);
dTreeFillData.Fill_Array<Double_t>("E9E25_FCAL_protcand",E9E25_FCAL_protcand, j);
dTreeFillData.Fill_Array<Double_t>("SumU_FCAL_protcand",SumU_FCAL_protcand, j);
dTreeFillData.Fill_Array<Double_t>("SumV_FCAL_protcand",SumV_FCAL_protcand, j);
 

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

  UnlockState(); //RELEASE PROCESSOR LOCK

  return NOERROR;

}

//------------------
jerror_t DEventProcessor_1p2pi::erun(void)
{
	// Any final calculations on histograms (like dividing them)
	// should be done here. This may get called more than once.
	return NOERROR;
}

//------------------
// fini
//------------------

jerror_t DEventProcessor_1p2pi::fini(void)
{
	delete dTreeInterface; //saves trees to file, closes file
	return NOERROR;
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

  return;

} // end GetHypotheses



