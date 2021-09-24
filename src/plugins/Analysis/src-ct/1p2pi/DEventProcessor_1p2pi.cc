// rpi	
// $Id$
//
//    File: DEventProcessor_1p2pi.cc
// Created: May 2021
// Creator: S. N. Santiesteban
//

#include <map>
using namespace std;

#include "DEventProcessor_1p2pi.h"
#include <DANA/DApplication.h>
#include <FCAL/DFCALShower.h>
#include <PID/DChargedTrack.h>
#include <PID/DBeamPhoton.h>
#include <PID/DChargedTrackHypothesis.h>

Int_t n1 = 80;
Int_t eventNumber, nPhotonCandidates, nChargedTracks;
Int_t nHyp;
Double_t bmE[80], bmtime[80];

Double_t pX_protcand[10], pY_protcand[10], pZ_protcand[10], X_protcand[10], Y_protcand[10], Z_protcand[10],E_protcand[10], time_protcand[10], t0_protcand[10];
Double_t prot_dedx_cdc[10], prot_dedx_fdc[10], prot_dedx_dc_ChiSq[10];
unsigned int prot_dedx_dc_NDF[10];
Double_t CLKinFit[10], ChiSqFit[10], NDF[10];
Double_t X_vertex[10], Y_vertex[10], Z_vertex[10], T_vertex[10];
Double_t prot_bcaltof[10], prot_fcaltof[10], dt_protcand[10];

Double_t X_piminuscand[10], Y_piminuscand[10], Z_piminuscand[10], dt_piminuscand[10];
Double_t pX_piminuscand[10], pY_piminuscand[10], pZ_piminuscand[10], E_piminuscand[10], time_piminuscand[10], t0_piminuscand[10];
Double_t  piminus_dedx_fdc[10], piminus_dedx_cdc[10], piminus_dedx_dc_ChiSq[10];
Double_t piminus_bcaltof[10], piminus_fcaltof[10];
Double_t X_pipluscand[10], Y_pipluscand[10], Z_pipluscand[10];
Double_t pX_pipluscand[10], pY_pipluscand[10], pZ_pipluscand[10], E_pipluscand[10], time_pipluscand[10], t0_pipluscand[10];
Double_t dt_pipluscand[10];
Double_t  piplus_dedx_fdc[10], piplus_dedx_cdc[10], piplus_dedx_dc_ChiSq[10];
Double_t piplus_bcaltof[10], piplus_fcaltof[10], mom_piplus[10];
Double_t mom_prot[10], mom_piminus[10], t0_rf; 
Int_t N_thrown, type_thrown[19], nShower;
Double_t beamEnergy_thrown, pX_thrown[19], pY_thrown[19], pZ_thrown[19], E_thrown[19];
Double_t HitTime_protcand[10], HitTime_pipluscand[10], HitTime_piminuscand[10];
Double_t Beta_Timing_protcand[10], Beta_Timing_pipluscand[10], Beta_Timing_piminuscand[10];
Double_t ChiSq_Timing_protcand[10], ChiSq_Timing_pipluscand[10], ChiSq_Timing_piminuscand[10];
Double_t NDF_Timing_protcand[10], NDF_Timing_pipluscand[10], NDF_Timing_piminuscand[10];
Double_t dEdx_TOF_protcand[10], dEdx_TOF_pipluscand[10], dEdx_TOF_piminuscand[10];
Double_t dEdx_ST_protcand[10], dEdx_ST_pipluscand[10], dEdx_ST_piminuscand[10];
Double_t Energy_BCAL_protcand[10], Energy_BCAL_pipluscand[10], Energy_BCAL_piminuscand[10];
Double_t Energy_BCALPreshower_protcand[10], Energy_BCALPreshower_pipluscand[10], Energy_BCALPreshower_piminuscand[10];
Double_t SigLong_BCAL_protcand[10], SigLong_BCAL_pipluscand[10], SigLong_BCAL_piminuscand[10];
Double_t SigTheta_BCAL_protcand[10], SigTheta_BCAL_pipluscand[10], SigTheta_BCAL_piminuscand[10];
Double_t SigTrans_BCAL_protcand[10], SigTrans_BCAL_pipluscand[10], SigTrans_BCAL_piminuscand[10];
Double_t RMSTime_BCAL_protcand[10], RMSTime_BCAL_pipluscand[10], RMSTime_BCAL_piminuscand[10];

Double_t Energy_FCAL_protcand[10], Energy_FCAL_pipluscand[10], Energy_FCAL_piminuscand[10];
Double_t E1E9_FCAL_protcand[10], E1E9_FCAL_pipluscand[10], E1E9_FCAL_piminuscand[10];
Double_t E9E25_FCAL_protcand[10], E9E25_FCAL_pipluscand[10], E9E25_FCAL_piminuscand[10];
Double_t SumU_FCAL_protcand[10], SumU_FCAL_pipluscand[10], SumU_FCAL_piminuscand[10];
Double_t SumV_FCAL_protcand[10], SumV_FCAL_pipluscand[10], SumV_FCAL_piminuscand[10];

Double_t TrackBCAL_DeltaPhi_piminuscand[10], TrackBCAL_DeltaZ_piminuscand[10], TrackFCAL_DOCA_piminuscand[10];
Double_t TrackBCAL_DeltaPhi_protcand[10], TrackBCAL_DeltaZ_protcand[10], TrackFCAL_DOCA_protcand[10];
Double_t TrackBCAL_DeltaPhi_pipluscand[10], TrackBCAL_DeltaZ_pipluscand[10], TrackFCAL_DOCA_pipluscand[10];


Double_t X_protonkinfit[10], Y_protonkinfit[10], Z_protonkinfit[10], pX_protonkinfit[10], pY_protonkinfit[10], pZ_protonkinfit[10], E_protonkinfit[10];
Double_t X_pipluskinfit[10], Y_pipluskinfit[10], Z_pipluskinfit[10], pX_pipluskinfit[10], pY_pipluskinfit[10], pZ_pipluskinfit[10], E_pipluskinfit[10];
Double_t X_piminuskinfit[10], Y_piminuskinfit[10], Z_piminuskinfit[10], pX_piminuskinfit[10], pY_piminuskinfit[10], pZ_piminuskinfit[10], E_piminuskinfit[10];
Double_t time_piminuskinfit[10], time_pipluskinfit[10], time_protonkinfit[10], Common_Time[10];
Double_t FOM_pipluscand[10], FOM_piminuscand[10], FOM_protcand[10], NDF_pipluscand[10], NDF_piminuscand[10], NDF_protcand[10], ChiSq_pipluscand[10], ChiSq_piminuscand[10], ChiSq_protcand[10]; 

Double_t RFTime_Measured;

unsigned int piplus_dedx_dc_NDF[10], piminus_dedx_dc_NDF[10];
UInt_t  RunNumber;
UInt_t  L1TriggerBits;
ULong64_t EventNumber;

extern "C"{
void InitPlugin(JApplication *app){
	InitJANAPlugin(app);
	app->AddProcessor(new DEventProcessor_1p2pi());
}
} // "C"


//------------------
// init
//------------------
jerror_t DEventProcessor_1p2pi::init(void)
{
  
  T = new TTree( "T", "T" );
  T->Branch("EventNumber",&EventNumber,"EventNumber/l");
  T->Branch("L1TriggerBits",&L1TriggerBits,"L1TriggerBits/i");
  
  T->Branch("nHyp",&nHyp,"nHyp/I");
  T->Branch("nChargedTracks",&nChargedTracks,"nChargedTracks/I");
  T->Branch("nShower",&nShower,"nShower/I");

  T->Branch("X_piminuscand",X_piminuscand,"X_piminuscand[nHyp]/D");
  T->Branch("Y_piminuscand",Y_piminuscand,"Y_piminuscand[nHyp]/D");
  T->Branch("Z_piminuscand",Z_piminuscand,"Z_piminuscand[nHyp]/D");
  T->Branch("pX_piminuscand",pX_piminuscand,"pX_piminuscand[nHyp]/D");
  T->Branch("pY_piminuscand",pY_piminuscand,"pY_piminuscand[nHyp]/D");
  T->Branch("pZ_piminuscand",pZ_piminuscand,"pZ_piminuscand[nHyp]/D");
  T->Branch("E_piminuscand",E_piminuscand,"E_piminuscand[nHyp]/D");
  T->Branch("time_piminuscand",time_piminuscand,"time_piminuscand[nHyp]/D");
  T->Branch("t0_piminuscand",t0_piminuscand,"t0_piminuscand[nHyp]/D");
  T->Branch("dt_piminuscand",dt_piminuscand,"dt_piminuscand[nHyp]/D");
  T->Branch("piminus_dedx_cdc",piminus_dedx_cdc,"piminus_dedx_cdc[nHyp]/D");
  T->Branch("piminus_dedx_fdc",piminus_dedx_fdc,"piminus_dedx_fdc[nHyp]/D");
  T->Branch("piminus_dedx_dc_NDF",piminus_dedx_dc_NDF,"piminus_dedx_dc_NDF[nHyp]/i");
  T->Branch("piminus_dedx_dc_ChiSq",piminus_dedx_dc_ChiSq,"piminus_dedx_dc_ChiSq[nHyp]/D");
  T->Branch("piminus_bcaltof",piminus_bcaltof,"piminus_bcaltof[nHyp]/D");
  T->Branch("piminus_fcaltof",piminus_fcaltof,"piminus_fcaltof[nHyp]/D");
  T->Branch("FOM_piminuscand",FOM_piminuscand,"FOM_piminuscand[nHyp]/D");
  T->Branch("NDF_piminuscand",NDF_piminuscand,"NDF_piminuscand[nHyp]/D");
  T->Branch("ChiSq_piminuscand",ChiSq_piminuscand,"ChiSq_piminuscand[nHyp]/D");
  T->Branch("HitTime_piminuscand",HitTime_piminuscand,"HitTime_piminuscand[nHyp]/D");
  T->Branch("Beta_Timing_piminuscand",Beta_Timing_piminuscand,"Beta_Timing_piminuscand[nHyp]/D");
  T->Branch("NDF_Timing_piminuscand",NDF_Timing_piminuscand,"NDF_Timing_piminuscand[nHyp]/D");
  T->Branch("ChiSq_Timing_piminuscand",ChiSq_Timing_piminuscand,"ChiSq_Timing_piminuscand[nHyp]/D");
  T->Branch("dEdx_ST_piminuscand",dEdx_ST_piminuscand,"dEdx_ST_piminuscand[nHyp]/D");
  T->Branch("dEdx_TOF_piminuscand",dEdx_TOF_piminuscand,"dEdx_TOF_piminuscand[nHyp]/D");
  T->Branch("Energy_BCAL_piminuscand",Energy_BCAL_piminuscand,"Energy_BCAL_piminuscand[nHyp]/D");
  T->Branch("Energy_BCALPreshower_piminuscand",Energy_BCALPreshower_piminuscand,"Energy_BCALPreshower_piminuscand[nHyp]/D");
  T->Branch("SigLong_BCAL_piminuscand",SigLong_BCAL_piminuscand,"SigLong_BCAL_piminuscand[nHyp]/D");
  T->Branch("SigTheta_BCAL_piminuscand",SigTheta_BCAL_piminuscand,"SigTheta_BCAL_piminuscand[nHyp]/D");
  T->Branch("SigTrans_BCAL_piminuscand",SigTrans_BCAL_piminuscand,"SigTrans_BCAL_piminuscand[nHyp]/D");
  T->Branch("RMSTime_BCAL_piminuscand",RMSTime_BCAL_piminuscand,"RMSTime_BCAL_piminuscand[nHyp]/D");
  T->Branch("Energy_FCAL_piminuscand",Energy_FCAL_piminuscand,"Energy_FCAL_piminuscand[nHyp]/D");
  T->Branch("E1E9_FCAL_piminuscand",E1E9_FCAL_piminuscand,"E1E9_FCAL_piminuscand[nHyp]/D");
  T->Branch("E9E25_FCAL_piminuscand",E9E25_FCAL_piminuscand,"E9E25_FCAL_piminuscand[nHyp]/D");
  T->Branch("SumU_FCAL_piminuscand",SumU_FCAL_piminuscand,"SumU_FCAL_piminuscand[nHyp]/D");
  T->Branch("SumV_FCAL_piminuscand",SumV_FCAL_piminuscand,"SumV_FCAL_piminuscand[nHyp]/D");
  T->Branch("TrackBCAL_DeltaPhi_piminuscand",TrackBCAL_DeltaPhi_piminuscand,"TrackBCAL_DeltaPhi_piminuscand[nHyp]/D");
  T->Branch("TrackBCAL_DeltaZ_piminuscand",TrackBCAL_DeltaZ_piminuscand,"TrackBCAL_DeltaZ_piminuscand[nHyp]/D");
  T->Branch("TrackFCAL_DOCA_piminuscand",TrackFCAL_DOCA_piminuscand,"TrackFCAL_DOCA_piminuscand[nHyp]/D");
 // T->Branch("",,"[nHyp]/D");

  T->Branch("X_protcand",X_protcand,"X_protcand[nHyp]/D");
  T->Branch("Y_protcand",Y_protcand,"Y_protcand[nHyp]/D");
  T->Branch("Z_protcand",Z_protcand,"Z_protcand[nHyp]/D");
  T->Branch("pX_protcand",pX_protcand,"pX_protcand[nHyp]/D");
  T->Branch("pY_protcand",pY_protcand,"pY_protcand[nHyp]/D");
  T->Branch("pZ_protcand",pZ_protcand,"pZ_protcand[nHyp]/D");
  T->Branch("E_protcand",E_protcand,"E_protcand[nHyp]/D");
  T->Branch("time_protcand",time_protcand,"time_protcand[nHyp]/D");
  T->Branch("t0_protcand",t0_protcand,"t0_protcand[nHyp]/D");
  T->Branch("dt_protcand",dt_protcand,"dt_protcand[nHyp]/D");
  T->Branch("prot_dedx_cdc",prot_dedx_cdc,"prot_dedx_cdc[nHyp]/D");
  T->Branch("prot_dedx_fdc",prot_dedx_fdc,"prot_dedx_fdc[nHyp]/D");
  T->Branch("prot_dedx_dc_NDF",prot_dedx_dc_NDF,"prot_dedx_dc_NDF[nHyp]/i");
  T->Branch("prot_dedx_dc_ChiSq",prot_dedx_dc_ChiSq,"prot_dedx_dc_ChiSq[nHyp]/D");	  
  T->Branch("prot_bcaltof",prot_bcaltof,"prot_bcaltof[nHyp]/D");
  T->Branch("prot_fcaltof",prot_fcaltof,"prot_fcaltof[nHyp]/D");
  T->Branch("FOM_protcand",FOM_protcand,"FOM_protcand[nHyp]/D");
  T->Branch("NDF_protcand",NDF_protcand,"NDF_protcand[nHyp]/D");
  T->Branch("ChiSq_protcand",ChiSq_protcand,"ChiSq_protcand[nHyp]/D");
  T->Branch("HitTime_protcand",HitTime_protcand,"HitTime_protcand[nHyp]/D");
  T->Branch("Beta_Timing_protcand",Beta_Timing_protcand,"Beta_Timing_protcand[nHyp]/D");
  T->Branch("NDF_Timing_protcand",NDF_Timing_protcand,"NDF_Timing_protcand[nHyp]/D");
  T->Branch("ChiSq_Timing_protcand",ChiSq_Timing_protcand,"ChiSq_Timing_protcand[nHyp]/D");
  T->Branch("dEdx_ST_protcand",dEdx_ST_protcand,"dEdx_ST_protcand[nHyp]/D");
  T->Branch("dEdx_TOF_protcand",dEdx_TOF_protcand,"dEdx_TOF_protcand[nHyp]/D");
  T->Branch("Energy_BCAL_protcand",Energy_BCAL_protcand,"Energy_BCAL_protcand[nHyp]/D");
  T->Branch("Energy_BCALPreshower_protcand",Energy_BCALPreshower_protcand,"Energy_BCALPreshower_protcand[nHyp]/D");
  T->Branch("SigLong_BCAL_protcand",SigLong_BCAL_protcand,"SigLong_BCAL_protcand[nHyp]/D");
  T->Branch("SigTheta_BCAL_protcand",SigTheta_BCAL_protcand,"SigTheta_BCAL_protcand[nHyp]/D");
  T->Branch("SigTrans_BCAL_protcand",SigTrans_BCAL_protcand,"SigTrans_BCAL_protcand[nHyp]/D");
  T->Branch("RMSTime_BCAL_protcand",RMSTime_BCAL_protcand,"RMSTime_BCAL_protcand[nHyp]/D");
  T->Branch("Energy_FCAL_protcand",Energy_FCAL_piminuscand,"Energy_FCAL_piminuscand[nHyp]/D");
  T->Branch("E1E9_FCAL_protcand",E1E9_FCAL_protcand,"E1E9_FCAL_protcand[nHyp]/D");
  T->Branch("E9E25_FCAL_protcand",E9E25_FCAL_protcand,"E9E25_FCAL_protcand[nHyp]/D");
  T->Branch("SumU_FCAL_protcand",SumU_FCAL_protcand,"SumU_FCAL_protcand[nHyp]/D");
  T->Branch("SumV_FCAL_protcand",SumV_FCAL_protcand,"SumV_FCAL_protcand[nHyp]/D");
  T->Branch("TrackBCAL_DeltaPhi_protcand",TrackBCAL_DeltaPhi_protcand,"TrackBCAL_DeltaPhi_protcand[nHyp]/D");
  T->Branch("TrackBCAL_DeltaZ_protcand",TrackBCAL_DeltaZ_protcand,"TrackBCAL_DeltaZ_protcand[nHyp]/D");
  T->Branch("TrackFCAL_DOCA_protcand",TrackFCAL_DOCA_protcand,"TrackFCAL_DOCA_protcand[nHyp]/D");

 // T->Branch("",&,"");

  T->Branch("X_pipluscand",X_pipluscand,"X_pipluscand[nHyp]/D");
  T->Branch("Y_pipluscand",Y_pipluscand,"Y_pipluscand[nHyp]/D");
  T->Branch("Z_pipluscand",Z_pipluscand,"Z_pipluscand[nHyp]/D");
  T->Branch("pX_pipluscand",pX_pipluscand,"pX_pipluscand[nHyp]/D");
  T->Branch("pY_pipluscand",pY_pipluscand,"pY_pipluscand[nHyp]/D");
  T->Branch("pZ_pipluscand",pZ_pipluscand,"pZ_pipluscand[nHyp]/D");
  T->Branch("E_pipluscand",E_pipluscand,"E_pipluscand[nHyp]/D");
  T->Branch("time_pipluscand",time_pipluscand,"time_pipluscand[nHyp]/D");
  T->Branch("t0_pipluscand",t0_pipluscand,"t0_pipluscand[nHyp]/D");
  T->Branch("dt_pipluscand",dt_pipluscand,"dt_pipluscand[nHyp]/D");
  T->Branch("piplus_dedx_cdc",piplus_dedx_cdc,"piplus_dedx_cdc[nHyp]/D");
  T->Branch("piplus_dedx_fdc",piplus_dedx_fdc,"piplus_dedx_fdc[nHyp]/D");
  T->Branch("piplus_dedx_dc_NDF",piplus_dedx_dc_NDF,"piplus_dedx_dc_NDF[nHyp]/i");
  T->Branch("piplus_dedx_dc_ChiSq",piplus_dedx_dc_ChiSq,"piplus_dedx_dc_ChiSq[nHyp]/D");
  T->Branch("piplus_bcaltof",piplus_bcaltof,"piplus_bcaltof[nHyp]/D");
  T->Branch("piplus_fcaltof",piplus_fcaltof,"piplus_fcaltof[nHyp]/D");
  T->Branch("FOM_pipluscand",FOM_pipluscand,"FOM_pipluscand[nHyp]/D");
  T->Branch("NDF_pipluscand",NDF_pipluscand,"NDF_pipluscand[nHyp]/D");
  T->Branch("ChiSq_pipluscand",ChiSq_pipluscand,"ChiSq_pipluscand[nHyp]/D");
  T->Branch("HitTime_pipluscand",HitTime_pipluscand,"HitTime_pipluscand[nHyp]/D");
  T->Branch("Beta_Timing_pipluscand",Beta_Timing_pipluscand,"Beta_Timing_pipluscand[nHyp]/D");
  T->Branch("NDF_Timing_pipluscand",NDF_Timing_pipluscand,"NDF_Timing_pipluscand[nHyp]/D");
  T->Branch("ChiSq_Timing_pipluscand",ChiSq_Timing_pipluscand,"ChiSq_Timing_pipluscand[nHyp]/D");
  T->Branch("dEdx_ST_pipluscand",dEdx_ST_pipluscand,"dEdx_ST_pipluscand[nHyp]/D");
  T->Branch("dEdx_TOF_pipluscand",dEdx_TOF_pipluscand,"dEdx_TOF_pipluscand[nHyp]/D");
  T->Branch("Energy_BCAL_pipluscand",Energy_BCAL_pipluscand,"Energy_BCAL_pipluscand[nHyp]/D");
  T->Branch("Energy_BCALPreshower_pipluscand",Energy_BCALPreshower_pipluscand,"Energy_BCALPreshower_pipluscand[nHyp]/D");
  T->Branch("SigLong_BCAL_pipluscand",SigLong_BCAL_pipluscand,"SigLong_BCAL_pipluscand[nHyp]/D");
  T->Branch("SigTheta_BCAL_pipluscand",SigTheta_BCAL_pipluscand,"SigTheta_BCAL_pipluscand[nHyp]/D");
  T->Branch("SigTrans_BCAL_pipluscand",SigTrans_BCAL_pipluscand,"SigTrans_BCAL_pipluscand[nHyp]/D");
  T->Branch("RMSTime_BCAL_pipluscand",RMSTime_BCAL_pipluscand,"RMSTime_BCAL_pipluscand[nHyp]/D");
  T->Branch("Energy_FCAL_pipluscand",Energy_FCAL_pipluscand,"Energy_FCAL_pipluscand[nHyp]/D");
  T->Branch("E1E9_FCAL_pipluscand",E1E9_FCAL_pipluscand,"E1E9_FCAL_pipluscand[nHyp]/D");
  T->Branch("E9E25_FCAL_pipluscand",E9E25_FCAL_pipluscand,"E9E25_FCAL_pipluscand[nHyp]/D");
  T->Branch("SumU_FCAL_pipluscand",SumU_FCAL_pipluscand,"SumU_FCAL_pipluscand[nHyp]/D");
  T->Branch("SumV_FCAL_pipluscand",SumV_FCAL_pipluscand,"SumV_FCAL_pipluscand[nHyp]/D");
  T->Branch("TrackBCAL_DeltaPhi_pipluscand",TrackBCAL_DeltaPhi_pipluscand,"TrackBCAL_DeltaPhi_pipluscand[nHyp]/D");
  T->Branch("TrackBCAL_DeltaZ_pipluscand",TrackBCAL_DeltaZ_pipluscand,"TrackBCAL_DeltaZ_pipluscand[nHyp]/D");
  T->Branch("TrackFCAL_DOCA_pipluscand",TrackFCAL_DOCA_pipluscand,"TrackFCAL_DOCA_pipluscand[nHyp]/D");

  T->Branch("mom_piminug",mom_piminus,"mom_piminus[nHyp]/D");
  T->Branch("mom_piplus",mom_piplus,"mom_piplus[nHyp]/D");
  T->Branch("mom_prot",mom_prot,"mom_prot[nHyp]/D");

  T->Branch("t0_rf",&t0_rf,"t0_rf/D");
  T->Branch("RFTime_Measured",&RFTime_Measured,"RFTime_Measured/I");

  //=======================================================================//
  
  T->Branch("X_piminuskinfit",X_piminuskinfit,"X_piminuskinfit[nHyp]/D");
  T->Branch("Y_piminuskinfit",Y_piminuskinfit,"Y_piminuskinfit[nHyp]/D");
  T->Branch("Z_piminuskinfit",Z_piminuskinfit,"Z_piminuskinfit[nHyp]/D");
  T->Branch("pX_piminuskinfit",pX_piminuskinfit,"pX_piminuskinfit[nHyp]/D");
  T->Branch("pY_piminuskinfit",pY_piminuskinfit,"pY_piminuskinfit[nHyp]/D");
  T->Branch("pZ_piminuskinfit",pZ_piminuskinfit,"pZ_piminuskinfit[nHyp]/D");
  T->Branch("E_piminuskinfit",E_piminuskinfit,"E_piminuskinfit[nHyp]/D");
  T->Branch("time_piminuskinfit",time_piminuskinfit,"time_piminuskinfit[nHyp]/D");

  T->Branch("X_pipluskinfit",X_pipluskinfit,"X_pipluskinfit[nHyp]/D");
  T->Branch("Y_pipluskinfit",Y_pipluskinfit,"Y_pipluskinfit[nHyp]/D");
  T->Branch("Z_pipluskinfit",Z_pipluskinfit,"Z_pipluskinfit[nHyp]/D");
  T->Branch("pX_pipluskinfit",pX_pipluskinfit,"pX_pipluskinfit[nHyp]/D");
  T->Branch("pY_pipluskinfit",pY_pipluskinfit,"pY_pipluskinfit[nHyp]/D");
  T->Branch("pZ_pipluskinfit",pZ_pipluskinfit,"pZ_pipluskinfit[nHyp]/D");
  T->Branch("E_pipluskinfit",E_pipluskinfit,"E_pipluskinfit[nHyp]/D");
  T->Branch("time_pipluskinfit",time_pipluskinfit,"time_pipluskinfit[nHyp]/D");

  T->Branch("X_protonkinfit",X_protonkinfit,"X_protonkinfit[nHyp]/D");
  T->Branch("Y_protonkinfit",Y_protonkinfit,"Y_protonkinfit[nHyp]/D");
  T->Branch("Z_protonkinfit",Z_protonkinfit,"Z_protonkinfit[nHyp]/D");
  T->Branch("pX_protonkinfit",pX_protonkinfit,"pX_protonkinfit[nHyp]/D");
  T->Branch("pY_protonkinfit",pY_protonkinfit,"pY_protonkinfit[nHyp]/D");
  T->Branch("pZ_protonkinfit",pZ_protonkinfit,"pZ_protonkinfit[nHyp]/D");
  T->Branch("E_protonkinfit",E_protonkinfit,"E_protonkinfit[nHyp]/D");
  T->Branch("time_protonkinfit",time_protonkinfit,"time_protonkinfit[nHyp]/D");
  T->Branch("Common_Time",Common_Time,"Common_Time[nHyp]/D");

  //==========================================================================//
  T->Branch("nPhotonCandidates",&nPhotonCandidates,"nPhotonCandidates/I");
 
  T->Branch("X_vertex",X_vertex,"X_vertex[nHyp]/D");
  T->Branch("Y_vertex",Y_vertex,"Y_vertex[nHyp]/D");
  T->Branch("Z_vertex",Z_vertex,"Z_vertex[nHyp]/D");
  T->Branch("T_vertex",T_vertex,"T_vertex[nHyp]/D");
  T->Branch("CLKinFit",CLKinFit,"CLKinFit[nHyp]/D");
  T->Branch("NDF",NDF,"NDF[nHyp][80]/D");
  T->Branch("ChiSqFit",ChiSqFit,"ChiSqFit[nHyp]/D");

  //T->Branch("",,"[nPhotonCandidates]/D");
   
  T->Branch("bmE",bmE,"bmE[nPhotonCandidates]/D");
  T->Branch("bmtime",bmtime,"bmtime[nPhotonCandidates]/D");

  RunInformation = new TTree( "RunInformation", "RunInformation" );
  RunInformation->Branch("RunNumber",&RunNumber,"RunNumber/I");

  return NOERROR;
}

//------------------
// brun
//------------------
jerror_t DEventProcessor_1p2pi::brun(JEventLoop *eventLoop, int32_t runnumber)
{
        //void DKinFitUtils_GlueX::Reset_NewEvent(void);
        //void DKinFitter::Reset_NewEvent(void);
	return NOERROR;
}

//------------------
// evnt
//------------------
jerror_t DEventProcessor_1p2pi::evnt(JEventLoop *loop, uint64_t eventnumber)
{


	vector<const DFCALShower*> fcal_showers;
        vector<const DChargedTrack*>ch_tracks;
	vector<const DBeamPhoton*> beam_ph;
        vector<const DNeutralShower*> showers;
	
	loop->Get(fcal_showers);
	loop->Get(ch_tracks);
	loop->Get(beam_ph);
        loop->Get(showers);
	
	japp->RootWriteLock();

	const DTrigger* Trigger = NULL;
	loop->GetSingle(Trigger);

	if (loop->GetJEvent().GetEventNumber() == 1){
		RunNumber = loop->GetJEvent().GetRunNumber();
		RunInformation->Fill();
	}

	if(!Trigger->Get_IsPhysicsEvent()) {
		japp->RootUnLock();
		return NOERROR;
	}
	const DEventRFBunch* RF_bunch = NULL;
	loop->GetSingle(RF_bunch);
	 
	//=============================================//
   

	if (ch_tracks.size()!=3){
		 japp->RootUnLock();
		 return NOERROR;
        }
	//=== Get Hypothesis ===========///
       
//	vector<Particle_t> targetParticles = {Proton, PiPlus, PiMinus};

	map<Particle_t, int> targetParticles = {
                {Proton,1},
                {PiPlus,1},
                {PiMinus,1}
        };

        map<Particle_t, vector<const DChargedTrackHypothesis*> > emptyHypothesis;
        vector<map<Particle_t, vector<const DChargedTrackHypothesis*> > > hypothesisList;
        GetHypotheses(ch_tracks,targetParticles,emptyHypothesis,hypothesisList);

	//cout << showers.size() << endl;

	if (hypothesisList.size()  == 0) {
		japp->RootUnLock();
		return NOERROR;
	}
        //cout << "Number of Charged particles: "<< ch_tracks.size() << " Total number of  Hypothesis: " << hypothesisList.size()  << "  " << showers.size() << endl;
	

	EventNumber = loop->GetJEvent().GetEventNumber(); 
	L1TriggerBits = Trigger->Get_L1TriggerBits();
        nChargedTracks = ch_tracks.size();
	nHyp = hypothesisList.size();	
	nPhotonCandidates = beam_ph.size();
	RFTime_Measured = RF_bunch->dTime;
	nShower = showers.size() ;
	for (Int_t j = 0; j < nHyp; j++){

	   X_vertex[j] =  Y_vertex[j] =  Z_vertex[j] = T_vertex[j] = -100;
           ChiSqFit[j]=-100;
           NDF[j]=-100;
	   FOM_protcand[j] = FOM_pipluscand[j] = FOM_piminuscand[j] = -100;
	   NDF_protcand[j] = NDF_pipluscand[j] = NDF_piminuscand[j] = -100;
	   pX_protcand[j] = pY_protcand[j] = pZ_protcand[j] = -100;
           X_protcand[j] = Y_protcand[j] = Z_protcand[j] = E_protcand[j] = time_protcand[j] = t0_protcand[j] = ChiSq_protcand[j] = -100;
           dt_protcand[j] = dt_pipluscand[j] = -100;
           prot_dedx_fdc[j] = prot_dedx_cdc[j] = prot_dedx_dc_NDF[j] = prot_dedx_dc_ChiSq[j]  = -100;
           prot_bcaltof[j] = prot_fcaltof[j] = -100;
	   X_pipluscand[j] = Y_pipluscand[j] = Z_pipluscand[j] = -100;
	   pX_pipluscand[j] = pY_pipluscand[j] = pZ_pipluscand[j] = E_pipluscand[j] = time_pipluscand[j] = t0_pipluscand[j] = ChiSq_pipluscand[j] = -100;
           dt_pipluscand[j] = piplus_dedx_fdc[j] = piplus_dedx_cdc[j] = piplus_dedx_dc_NDF[j] = piplus_dedx_dc_ChiSq[j] = -100;
           piplus_bcaltof[j] = piplus_fcaltof[j] = mom_piplus[j] = -100;
	   X_piminuscand[j] = Y_piminuscand[j] = Z_piminuscand[j] = piminus_bcaltof[j] = -100;
           pX_piminuscand[j] = pY_piminuscand[j] = pZ_piminuscand[j] = E_piminuscand[j] = time_piminuscand[j] = t0_piminuscand[j] = ChiSq_piminuscand[j] = -1000;
           dt_piminuscand[j] = -100;
           piminus_dedx_fdc[j] = piminus_dedx_cdc[j] = piminus_dedx_dc_NDF[j] = piminus_dedx_dc_ChiSq[j] = -100;
           piminus_fcaltof[j] = -100;
	   X_protonkinfit[j] = Y_protonkinfit[j] = Z_protonkinfit[j] = pX_protonkinfit[j] = pY_protonkinfit[j] = pZ_protonkinfit[j] = E_protonkinfit[j] = -100;
	   X_pipluskinfit[j] = Y_pipluskinfit[j] = Z_pipluskinfit[j] = pX_pipluskinfit[j] = pY_pipluskinfit[j] = pZ_pipluskinfit[j] = E_pipluskinfit[j] = -100;
	   X_piminuskinfit[j] = Y_piminuskinfit[j] = Z_piminuskinfit[j] = pX_piminuskinfit[j] = pY_piminuskinfit[j] = pZ_piminuskinfit[j] = E_piminuskinfit[j] = -100; 
	   time_piminuskinfit[j] = time_pipluskinfit[j] = time_protonkinfit[j] = -100;
	   Common_Time[j] = -100;  
	   HitTime_protcand[j] = HitTime_piminuscand[j] = HitTime_pipluscand[j] = -100;
	   Beta_Timing_protcand[j] = Beta_Timing_pipluscand[j] = Beta_Timing_piminuscand[j] = -100;
	   ChiSq_Timing_protcand[j] = ChiSq_Timing_pipluscand[j] = ChiSq_Timing_piminuscand[j] = -100;
	   NDF_Timing_protcand[j]   = NDF_Timing_pipluscand[j]   = NDF_Timing_piminuscand[j] = -100;
	   dEdx_TOF_protcand[j] = dEdx_TOF_pipluscand[j] = dEdx_TOF_piminuscand[j] = -100;
	   dEdx_ST_protcand[j] = dEdx_ST_pipluscand[j] = dEdx_ST_piminuscand[j] = -100;
	   Energy_BCAL_protcand[j] = Energy_BCAL_pipluscand[j] = Energy_BCAL_piminuscand[j] = -100;
           Energy_BCALPreshower_protcand[j] = Energy_BCALPreshower_pipluscand[j] = Energy_BCALPreshower_piminuscand[j] = -100;
	   SigLong_BCAL_protcand[j] = SigLong_BCAL_pipluscand[j] = SigLong_BCAL_piminuscand[j] = -100;
	   SigTheta_BCAL_protcand[j] = SigTheta_BCAL_pipluscand[j] = SigTheta_BCAL_piminuscand[j] = -100;
	   SigTrans_BCAL_protcand[j] = SigTrans_BCAL_pipluscand[j] = SigTrans_BCAL_piminuscand[j] =-100;
	   RMSTime_BCAL_protcand[j] = RMSTime_BCAL_pipluscand[j] = RMSTime_BCAL_piminuscand[j] =-100;
	   Energy_FCAL_protcand[j] =  Energy_FCAL_pipluscand[j] =  Energy_FCAL_piminuscand[j] = -100;
	   E1E9_FCAL_protcand[j]   =  E1E9_FCAL_pipluscand[j]   =  E1E9_FCAL_piminuscand[j]   = -100;
	   E9E25_FCAL_protcand[j]  =  E9E25_FCAL_pipluscand[j]  =  E9E25_FCAL_piminuscand[j]  = -100;
	   SumU_FCAL_protcand[j]   =  SumU_FCAL_pipluscand[j]   =  SumU_FCAL_piminuscand[j]   = -100;
	   SumV_FCAL_protcand[j]   =  SumV_FCAL_pipluscand[j]   =  SumV_FCAL_piminuscand[j]   = -100;
	   TrackBCAL_DeltaPhi_protcand[j]    = TrackBCAL_DeltaZ_protcand[j]    = TrackFCAL_DOCA_protcand[j]    = -100;
	   TrackBCAL_DeltaPhi_pipluscand[j]  = TrackBCAL_DeltaZ_pipluscand[j]  = TrackFCAL_DOCA_pipluscand[j]  = -100;
	   TrackBCAL_DeltaPhi_piminuscand[j] = TrackBCAL_DeltaZ_piminuscand[j] = TrackFCAL_DOCA_piminuscand[j] = -100;



    }


       for(Int_t ii = 0; ii < nPhotonCandidates; ii++){
	      bmE[ii] = beam_ph[ii]->momentum().Mag();
	      bmtime[ii] = beam_ph[ii]->time();
       }



	t0_rf = -10000;
 	
	for ( Int_t ii = 0; ii < nHyp; ii++){
	    
	    map<Particle_t, vector<const DChargedTrackHypothesis*> > thisHyp = hypothesisList[ii];
	    t0_rf = ch_tracks[0]->Get_BestTrackingFOM()->t0();
	    
            //============ PiMinus Hypothesis ===========================================================//
            const DChargedTrackHypothesis *hyp_pi_min   = thisHyp[PiMinus][0];
	    const DTrackTimeBased *pi_min_track = hyp_pi_min->Get_TrackTimeBased();
	    FOM_piminuscand[ii] = hyp_pi_min->Get_FOM(); 
	    NDF_piminuscand[ii] = hyp_pi_min->Get_NDF(); 
	    ChiSq_piminuscand[ii] = hyp_pi_min->Get_ChiSq(); 
	    piminus_dedx_dc_NDF[ii]  = hyp_pi_min->Get_NDF_DCdEdx();
            piminus_dedx_dc_ChiSq[ii]  = hyp_pi_min->Get_ChiSq_DCdEdx();
	    Beta_Timing_piminuscand[ii] = hyp_pi_min->measuredBeta();
	    NDF_Timing_piminuscand[ii]   = hyp_pi_min->Get_NDF_Timing();	    
	    ChiSq_Timing_piminuscand[ii] = hyp_pi_min->Get_ChiSq_Timing();	    
	    dEdx_TOF_piminuscand[ii] = (hyp_pi_min->Get_TOFHitMatchParams() != NULL) ? hyp_pi_min->Get_TOFHitMatchParams()->dEdx : 0.0;
	    dEdx_ST_piminuscand[ii]  = (hyp_pi_min->Get_SCHitMatchParams()  != NULL) ? hyp_pi_min->Get_SCHitMatchParams()->dEdx : 0.0;
	    const DBCALShower* showerpiminus = NULL;
	    if (hyp_pi_min->Get_BCALShowerMatchParams() != NULL) showerpiminus = hyp_pi_min->Get_BCALShowerMatchParams()->dBCALShower;
	    Energy_BCAL_piminuscand[ii]  = (showerpiminus != NULL ) ? showerpiminus->E : 0.0;
	    Energy_BCALPreshower_piminuscand[ii]  = (showerpiminus != NULL ) ? showerpiminus->E_preshower : 0.0;
	    SigLong_BCAL_piminuscand[ii] = (showerpiminus != NULL ) ? showerpiminus->sigLong : 0.0;
	    SigTheta_BCAL_piminuscand[ii]= (showerpiminus != NULL ) ? showerpiminus->sigTheta : 0.0;
	    SigTrans_BCAL_piminuscand[ii] = (showerpiminus != NULL ) ? showerpiminus->sigTrans : 0.0;
	    RMSTime_BCAL_piminuscand[ii] = (showerpiminus != NULL ) ? showerpiminus->rmsTime : 0.0;
            
	    const DFCALShower* FCALshowerpiminus = NULL; 
	    if (hyp_pi_min->Get_FCALShowerMatchParams() != NULL) FCALshowerpiminus = hyp_pi_min->Get_FCALShowerMatchParams()->dFCALShower;
	    Energy_FCAL_piminuscand[ii] = (FCALshowerpiminus != NULL) ? FCALshowerpiminus->getEnergy() : 0.0; 
	    E1E9_FCAL_piminuscand[ii] = (FCALshowerpiminus != NULL) ? FCALshowerpiminus->getE1E9() : 0.0; 
	    E9E25_FCAL_piminuscand[ii] = (FCALshowerpiminus != NULL) ? FCALshowerpiminus->getE9E25() : 0.0; 
	    SumU_FCAL_piminuscand[ii] = (FCALshowerpiminus != NULL) ? FCALshowerpiminus->getSumU() : 0.0; 
	    SumV_FCAL_piminuscand[ii] = (FCALshowerpiminus != NULL) ? FCALshowerpiminus->getSumV() : 0.0;
 
            TrackBCAL_DeltaPhi_piminuscand[ii] = ( hyp_pi_min->Get_BCALShowerMatchParams()  != NULL) ? hyp_pi_min->Get_BCALShowerMatchParams()->dDeltaPhiToShower : 999.0;
            TrackBCAL_DeltaZ_piminuscand[ii]   = ( hyp_pi_min->Get_BCALShowerMatchParams()  != NULL) ? hyp_pi_min->Get_BCALShowerMatchParams()->dDeltaZToShower : 999.0;
	    TrackFCAL_DOCA_piminuscand[ii]     = ( hyp_pi_min->Get_FCALShowerMatchParams()  != NULL) ? hyp_pi_min->Get_FCALShowerMatchParams()->dDOCAToShower : 999.0;  

	    //=============== Position ===================================================//
	    X_piminuscand[ii] = pi_min_track->position().X();
            Y_piminuscand[ii] = pi_min_track->position().Y();
            Z_piminuscand[ii] = pi_min_track->position().Z();
	    //========== Energy and Momentum ============================================//
            DLorentzVector pi_minus_rec = pi_min_track->lorentzMomentum();
	    E_piminuscand[ii]  = pi_minus_rec.E();
            pX_piminuscand[ii] = pi_minus_rec.X();
            pY_piminuscand[ii] = pi_minus_rec.Y();
            pZ_piminuscand[ii] = pi_minus_rec.Z(); 
	    mom_piminus[ii]    = pi_min_track->momentum().Mag();
	    //========== Other information ============================================//
            time_piminuscand[ii]  = pi_min_track->time();
            t0_piminuscand[ii]    = pi_min_track->t0();
	    HitTime_piminuscand[ii]    = hyp_pi_min->t1();
            piminus_dedx_cdc[ii]  = 1e6*pi_min_track->ddEdx_CDC;
            piminus_dedx_fdc[ii]  = 1e6*pi_min_track->ddEdx_FDC;
	    shared_ptr<const DBCALShowerMatchParams> piminus_BCAL_TOF = hyp_pi_min->Get_BCALShowerMatchParams();
            shared_ptr<const DFCALShowerMatchParams> piminus_FCAL_TOF = hyp_pi_min->Get_FCALShowerMatchParams();
            if (piminus_BCAL_TOF!=NULL) piminus_bcaltof[ii]  = piminus_BCAL_TOF->dFlightTime;
            if (piminus_FCAL_TOF!=NULL) piminus_fcaltof[ii]  = piminus_FCAL_TOF->dFlightTime;	    
	   
	    //============ PiPlus Hypothesis ===========================================================//
	    const DChargedTrackHypothesis *hyp_pi_plus   = thisHyp[PiPlus][0];
	    const DTrackTimeBased *pi_plus_track = hyp_pi_plus->Get_TrackTimeBased();
	    FOM_pipluscand[ii] = hyp_pi_plus->Get_FOM();
	    NDF_pipluscand[ii] = hyp_pi_plus->Get_NDF();
	    ChiSq_pipluscand[ii] = hyp_pi_plus->Get_ChiSq();
	    piplus_dedx_dc_NDF[ii]  = hyp_pi_plus->Get_NDF_DCdEdx();
            piplus_dedx_dc_ChiSq[ii]  = hyp_pi_plus->Get_ChiSq_DCdEdx();
	    Beta_Timing_pipluscand[ii] = hyp_pi_plus->measuredBeta();
	    NDF_Timing_pipluscand[ii]   = hyp_pi_plus->Get_NDF_Timing();
            ChiSq_Timing_pipluscand[ii] = hyp_pi_plus->Get_ChiSq_Timing();
	    dEdx_TOF_pipluscand[ii] = (hyp_pi_plus->Get_TOFHitMatchParams() != NULL) ? hyp_pi_plus->Get_TOFHitMatchParams()->dEdx : 0.0;
            dEdx_ST_pipluscand[ii]  = (hyp_pi_plus->Get_SCHitMatchParams()  != NULL) ? hyp_pi_plus->Get_SCHitMatchParams()->dEdx : 0.0;
	    const DBCALShower* showerpiplus = NULL;
            if (hyp_pi_plus->Get_BCALShowerMatchParams() != NULL) showerpiplus = hyp_pi_plus->Get_BCALShowerMatchParams()->dBCALShower;
            Energy_BCAL_pipluscand[ii]  = (showerpiplus != NULL ) ? showerpiplus->E : 0.0;
            Energy_BCALPreshower_pipluscand[ii]  = (showerpiplus != NULL ) ? showerpiplus->E_preshower : 0.0;
	    SigLong_BCAL_pipluscand[ii] = (showerpiplus != NULL ) ? showerpiplus->sigLong : 0.0;
            SigTheta_BCAL_pipluscand[ii] = (showerpiplus != NULL ) ? showerpiplus->sigTheta : 0.0;
            SigTrans_BCAL_pipluscand[ii] = (showerpiplus != NULL ) ? showerpiplus->sigTrans : 0.0;
            RMSTime_BCAL_pipluscand[ii] = (showerpiplus != NULL ) ? showerpiplus->rmsTime : 0.0;

	    const DFCALShower* FCALshowerpiplus = NULL;
            if (hyp_pi_plus->Get_FCALShowerMatchParams() != NULL) FCALshowerpiplus = hyp_pi_plus->Get_FCALShowerMatchParams()->dFCALShower;
            Energy_FCAL_pipluscand[ii] = (FCALshowerpiplus != NULL) ? FCALshowerpiplus->getEnergy() : 0.0;
            E1E9_FCAL_pipluscand[ii] = (FCALshowerpiplus != NULL) ? FCALshowerpiplus->getE1E9() : 0.0;    
            E9E25_FCAL_pipluscand[ii] = (FCALshowerpiplus != NULL) ? FCALshowerpiplus->getE9E25() : 0.0;
            SumU_FCAL_pipluscand[ii] = (FCALshowerpiplus != NULL) ? FCALshowerpiplus->getSumU() : 0.0;  
            SumV_FCAL_pipluscand[ii] = (FCALshowerpiplus != NULL) ? FCALshowerpiplus->getSumV() : 0.0; 

	    TrackBCAL_DeltaPhi_pipluscand[ii] = ( hyp_pi_plus->Get_BCALShowerMatchParams()  != NULL) ? hyp_pi_plus->Get_BCALShowerMatchParams()->dDeltaPhiToShower : 999.0;
            TrackBCAL_DeltaZ_pipluscand[ii]   = ( hyp_pi_plus->Get_BCALShowerMatchParams()  != NULL) ? hyp_pi_plus->Get_BCALShowerMatchParams()->dDeltaZToShower : 999.0;
            TrackFCAL_DOCA_pipluscand[ii]     = ( hyp_pi_plus->Get_FCALShowerMatchParams()  != NULL) ? hyp_pi_plus->Get_FCALShowerMatchParams()->dDOCAToShower : 999.0;

	    //=============== Position ===================================================//
	    X_pipluscand[ii] = pi_plus_track->position().X();
            Y_pipluscand[ii] = pi_plus_track->position().Y();
            Z_pipluscand[ii] = pi_plus_track->position().Z(); 
	    //========== Energy and Momentum ============================================//
	    DLorentzVector pi_plus_rec = pi_plus_track->lorentzMomentum();
            E_pipluscand[ii]  = pi_plus_rec.E();
            pX_pipluscand[ii] = pi_plus_rec.X();
            pY_pipluscand[ii] = pi_plus_rec.Y();
            pZ_pipluscand[ii] = pi_plus_rec.Z();
            mom_piplus[ii]    = pi_plus_track->momentum().Mag();
	    //========== Other information ============================================//
	    time_pipluscand[ii]  = pi_plus_track->time();
            t0_pipluscand[ii]    = pi_plus_track->t0();
	    HitTime_pipluscand[ii] = hyp_pi_plus->t1();
            piplus_dedx_cdc[ii]  = 1e6*pi_plus_track->ddEdx_CDC;
            piplus_dedx_fdc[ii]  = 1e6*pi_plus_track->ddEdx_FDC;
            shared_ptr<const DBCALShowerMatchParams> piplus_BCAL_TOF = hyp_pi_plus->Get_BCALShowerMatchParams();
            shared_ptr<const DFCALShowerMatchParams> piplus_FCAL_TOF = hyp_pi_plus->Get_FCALShowerMatchParams();
            if (piplus_BCAL_TOF!=NULL) piplus_bcaltof[ii]  = piplus_BCAL_TOF->dFlightTime;
            if (piplus_FCAL_TOF!=NULL) piplus_fcaltof[ii]  = piplus_FCAL_TOF->dFlightTime;
	
	    //============ Proton Hypothesis ===========================================================//
	    const DChargedTrackHypothesis *hyp_proton   = thisHyp[Proton][0];
            const DTrackTimeBased *proton_track = hyp_proton->Get_TrackTimeBased();
	    FOM_protcand[ii] = hyp_proton->Get_FOM();
	    NDF_protcand[ii] = hyp_proton->Get_NDF();
	    ChiSq_protcand[ii] = hyp_proton->Get_ChiSq();
	    prot_dedx_dc_NDF[ii]  = hyp_proton->Get_NDF_DCdEdx();
            prot_dedx_dc_ChiSq[ii]  = hyp_proton->Get_ChiSq_DCdEdx();
	    Beta_Timing_protcand[ii] = hyp_proton->measuredBeta();
	    NDF_Timing_protcand[ii]   = hyp_proton->Get_NDF_Timing();
            ChiSq_Timing_protcand[ii] = hyp_proton->Get_ChiSq_Timing();
            dEdx_TOF_protcand[ii] = (hyp_proton->Get_TOFHitMatchParams() != NULL) ? hyp_proton->Get_TOFHitMatchParams()->dEdx : 0.0;
            dEdx_ST_protcand[ii]  = (hyp_proton->Get_SCHitMatchParams()  != NULL) ? hyp_proton->Get_SCHitMatchParams()->dEdx : 0.0;
	    const DBCALShower* showerproton = NULL;
            if (hyp_proton->Get_BCALShowerMatchParams() != NULL) showerproton = hyp_proton->Get_BCALShowerMatchParams()->dBCALShower;
            Energy_BCAL_protcand[ii]  = (showerproton != NULL ) ? showerproton->E : 0.0;
            Energy_BCALPreshower_protcand[ii]  = (showerproton != NULL ) ? showerproton->E_preshower : 0.0;
	    SigLong_BCAL_protcand[ii] = (showerproton != NULL ) ? showerproton->sigLong : 0.0;
            SigTheta_BCAL_protcand[ii] = (showerproton != NULL ) ? showerproton->sigTheta : 0.0;
            SigTrans_BCAL_protcand[ii] = (showerproton != NULL ) ? showerproton->sigTrans : 0.0;
            RMSTime_BCAL_protcand[ii]  = (showerproton != NULL ) ? showerproton->rmsTime : 0.0;

	    const DFCALShower* FCALshowerproton = NULL;
            if (hyp_proton->Get_FCALShowerMatchParams() != NULL) FCALshowerproton = hyp_proton->Get_FCALShowerMatchParams()->dFCALShower;
            Energy_FCAL_protcand[ii] = (FCALshowerproton != NULL) ? FCALshowerproton->getEnergy() : 0.0;
            E1E9_FCAL_protcand[ii] = (FCALshowerproton != NULL) ? FCALshowerproton->getE1E9() : 0.0;
            E9E25_FCAL_protcand[ii] = (FCALshowerproton != NULL) ? FCALshowerproton->getE9E25() : 0.0;
            SumU_FCAL_protcand[ii] = (FCALshowerproton != NULL) ? FCALshowerproton->getSumU() : 0.0;
            SumV_FCAL_protcand[ii] = (FCALshowerproton != NULL) ? FCALshowerproton->getSumV() : 0.0;

            TrackBCAL_DeltaPhi_protcand[ii] = ( hyp_proton->Get_BCALShowerMatchParams()  != NULL) ? hyp_proton->Get_BCALShowerMatchParams()->dDeltaPhiToShower : 999.0;
            TrackBCAL_DeltaZ_protcand[ii]   = ( hyp_proton->Get_BCALShowerMatchParams()  != NULL) ? hyp_proton->Get_BCALShowerMatchParams()->dDeltaZToShower : 999.0;
            TrackFCAL_DOCA_protcand[ii]     = ( hyp_proton->Get_FCALShowerMatchParams()  != NULL) ? hyp_proton->Get_FCALShowerMatchParams()->dDOCAToShower : 999.0;

	    //=============== Position ===================================================//
	    X_protcand[ii] = proton_track->position().X();
            Y_protcand[ii] = proton_track->position().Y();
            Z_protcand[ii] = proton_track->position().Z();
	    //========== Energy and Momentum ============================================//
	    DLorentzVector proton_rec = proton_track->lorentzMomentum();
            E_protcand[ii]  = proton_rec.E();
            pX_protcand[ii] = proton_rec.X();
            pY_protcand[ii] = proton_rec.Y();
            pZ_protcand[ii] = proton_rec.Z();
            mom_prot[ii] = proton_track->momentum().Mag();
	    //========== Other information ============================================//
	    time_protcand[ii]  = proton_track->time();
            t0_protcand[ii]    = proton_track->t0();
	    HitTime_protcand[ii] = hyp_proton->t1();
            prot_dedx_cdc[ii]  = 1e6*proton_track->ddEdx_CDC;
            prot_dedx_fdc[ii]  = 1e6*proton_track->ddEdx_FDC;
            shared_ptr<const DBCALShowerMatchParams> proton_BCAL_TOF = hyp_proton->Get_BCALShowerMatchParams();
            shared_ptr<const DFCALShowerMatchParams> proton_FCAL_TOF = hyp_proton->Get_FCALShowerMatchParams();
            if (proton_BCAL_TOF!=NULL) prot_bcaltof[ii]  = proton_BCAL_TOF->dFlightTime;
            if (proton_FCAL_TOF!=NULL) prot_fcaltof[ii]  = proton_FCAL_TOF->dFlightTime;
          
            //--------------------------------
            // Kinematic fit
	    //--------------------------------
	    DKinFitUtils_GlueX *dKinFitUtils = new DKinFitUtils_GlueX(loop);
            DKinFitter *dKinFitter = new DKinFitter(dKinFitUtils);

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

            shared_ptr<DKinFitConstraint_Vertex> locProductionVertexConstraint =  dKinFitUtils->Make_VertexConstraint(locFullConstrainParticles, NoParticles, proton_track->position());
	    dKinFitter->Add_Constraint(locProductionVertexConstraint);
            // PERFORM THE KINEMATIC FIT
	    dKinFitter->Fit_Reaction();
	    //GET THE FIT RESULTS
            double CL = dKinFitter->Get_ConfidenceLevel();
            
            if (CL>0){
              
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
			T_vertex[ii] = (*locParticleIterator)->Get_Time();
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


		X_vertex[ii] = vertex_kf[0];
                Y_vertex[ii] = vertex_kf[1];
                Z_vertex[ii] = vertex_kf[2]; 
                CLKinFit[ii] = CL;
                NDF[ii]      = dKinFitter->Get_NDF();
                ChiSqFit[ii] = dKinFitter->Get_ChiSq();
		
		Common_Time[ii] = fitPiPlus->Get_CommonTime();
		E_protonkinfit[ii]  = fitProton->Get_Energy(); time_protonkinfit[ii]  = fitProton->Get_Time();
                pX_protonkinfit[ii] = fitProton->Get_Momentum().X();  pY_protonkinfit[ii] = fitProton->Get_Momentum().Y(); pZ_protonkinfit[ii] = fitProton->Get_Momentum().Z();
                X_protonkinfit[ii] = fitProton->Get_Position().X(); Y_protonkinfit[ii] = fitProton->Get_Position().Y(); Z_protonkinfit[ii] = fitProton->Get_Position().Z();     
		E_pipluskinfit[ii]  = fitPiPlus->Get_Energy(); time_pipluskinfit[ii]  = fitPiPlus->Get_Time();
                pX_pipluskinfit[ii] = fitPiPlus->Get_Momentum().X();  pY_pipluskinfit[ii] = fitPiPlus->Get_Momentum().Y(); pZ_pipluskinfit[ii] = fitPiPlus->Get_Momentum().Z();
                X_pipluskinfit[ii] = fitPiPlus->Get_Position().X(); Y_pipluskinfit[ii] = fitPiPlus->Get_Position().Y(); Z_pipluskinfit[ii] = fitPiPlus->Get_Position().Z();
		
		E_piminuskinfit[ii]  = fitPiMinus->Get_Energy(); time_piminuskinfit[ii]  = fitPiMinus->Get_Time();
                pX_piminuskinfit[ii] = fitPiMinus->Get_Momentum().X();  pY_piminuskinfit[ii] = fitPiMinus->Get_Momentum().Y(); pZ_piminuskinfit[ii] = fitPiMinus->Get_Momentum().Z();
                X_piminuskinfit[ii] = fitPiMinus->Get_Position().X(); Y_piminuskinfit[ii] = fitPiMinus->Get_Position().Y(); Z_piminuskinfit[ii] = fitPiMinus->Get_Position().Z();    
				

           } //CL 
	} //for hyp 

	
	T->Fill();	
	japp->RootUnLock();
	return NOERROR;
}


//Recursive function for determining possible particle assignments 
// -- J. Pybus

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
	return NOERROR;
}
