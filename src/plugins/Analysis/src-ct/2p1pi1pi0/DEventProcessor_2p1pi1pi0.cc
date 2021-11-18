// 
//
//    File: DEventProcessor_2p1pi1pi0.cc
// Created: Mon Apr  3 11:38:03 EDT 2006
// Creator: davidl (on Darwin swire-b241.jlab.org 8.4.0 powerpc)
//

#include <map>
using namespace std;

#include "DEventProcessor_2p1pi1pi0.h"

#include <DANA/DApplication.h>

// Routine used to create our DEventProcessor
extern "C"{
  void InitPlugin(JApplication *app){
    InitJANAPlugin(app);
    app->AddProcessor(new DEventProcessor_2p1pi1pi0());
  }
} // "C"

//define static local variable //declared in header file
thread_local DTreeFillData DEventProcessor_2p1pi1pi0::dTreeFillData;

//------------------
// init
//------------------
jerror_t DEventProcessor_2p1pi1pi0::init(void)
{
 
  //TTREE INTERFACE
  //MUST DELETE WHEN FINISHED: OR ELSE DATA WON'T BE SAVED!!!
  string treeName = "tree_2p1pi1pi0";
  string treeFile = "tree_2p1pi1pi0.root";
  gPARMS->SetDefaultParameter("SRC_2P1PI1PI0:TREENAME", treeName);
  gPARMS->SetDefaultParameter("SRC_2P1PI1PI0:TREEFILE", treeFile);
  dTreeInterface = DTreeInterface::Create_DTreeInterface(treeName, treeFile);

  //TTREE BRANCHES
  DTreeBranchRegister locTreeBranchRegister;

  locTreeBranchRegister.Register_Single<Int_t>("eventNumber");
  locTreeBranchRegister.Register_Single<Int_t>("nPhotonCandidates");
  locTreeBranchRegister.Register_FundamentalArray<Double_t>("beamEnergy", "nPhotonCandidates");
  locTreeBranchRegister.Register_FundamentalArray<Double_t>("beamT", "nPhotonCandidates");

  locTreeBranchRegister.Register_Single<Double_t>("pX_Proton0");
  locTreeBranchRegister.Register_Single<Double_t>("pY_Proton0");
  locTreeBranchRegister.Register_Single<Double_t>("pZ_Proton0");
  locTreeBranchRegister.Register_Single<Double_t>("pX_Proton1");
  locTreeBranchRegister.Register_Single<Double_t>("pY_Proton1");
  locTreeBranchRegister.Register_Single<Double_t>("pZ_Proton1");
  locTreeBranchRegister.Register_Single<Double_t>("pX_PiMinus");
  locTreeBranchRegister.Register_Single<Double_t>("pY_PiMinus");
  locTreeBranchRegister.Register_Single<Double_t>("pZ_PiMinus");
  locTreeBranchRegister.Register_Single<Double_t>("pX_Photon0");
  locTreeBranchRegister.Register_Single<Double_t>("pY_Photon0");
  locTreeBranchRegister.Register_Single<Double_t>("pZ_Photon0");
  locTreeBranchRegister.Register_Single<Double_t>("pX_Photon1");
  locTreeBranchRegister.Register_Single<Double_t>("pY_Photon1");
  locTreeBranchRegister.Register_Single<Double_t>("pZ_Photon1");

  locTreeBranchRegister.Register_Single<Double_t>("x_shower0");
  locTreeBranchRegister.Register_Single<Double_t>("y_shower0");
  locTreeBranchRegister.Register_Single<Double_t>("z_shower0");
  locTreeBranchRegister.Register_Single<Double_t>("E_shower0");
  locTreeBranchRegister.Register_Single<Double_t>("t_shower0");
  locTreeBranchRegister.Register_Single<Bool_t>("BCAL_shower0");
  locTreeBranchRegister.Register_Single<Double_t>("x_shower1");
  locTreeBranchRegister.Register_Single<Double_t>("y_shower1");
  locTreeBranchRegister.Register_Single<Double_t>("z_shower1");
  locTreeBranchRegister.Register_Single<Double_t>("E_shower1");
  locTreeBranchRegister.Register_Single<Double_t>("t_shower1");
  locTreeBranchRegister.Register_Single<Bool_t>("BCAL_shower1");

  locTreeBranchRegister.Register_Single<Double_t>("vertex_X");
  locTreeBranchRegister.Register_Single<Double_t>("vertex_Y");
  locTreeBranchRegister.Register_Single<Double_t>("vertex_Z");
  locTreeBranchRegister.Register_Single<Double_t>("vertex_T");

  locTreeBranchRegister.Register_Single<Int_t>("N_thrown");
  locTreeBranchRegister.Register_Single<Int_t>("beamEnergy_thrown");
  locTreeBranchRegister.Register_FundamentalArray<Double_t>("pX_thrown", "N_thrown");
  locTreeBranchRegister.Register_FundamentalArray<Double_t>("pY_thrown", "N_thrown");
  locTreeBranchRegister.Register_FundamentalArray<Double_t>("pZ_thrown", "N_thrown");
  locTreeBranchRegister.Register_FundamentalArray<Double_t>("E_thrown", "N_thrown");
  locTreeBranchRegister.Register_FundamentalArray<Int_t>("type_thrown", "N_thrown");
  locTreeBranchRegister.Register_FundamentalArray<Int_t>("pdgtype_thrown", "N_thrown");
  locTreeBranchRegister.Register_FundamentalArray<Int_t>("myid_thrown", "N_thrown");
  locTreeBranchRegister.Register_FundamentalArray<Int_t>("parentid_thrown", "N_thrown");

  locTreeBranchRegister.Register_Single<Double_t>("CL");

  locTreeBranchRegister.Register_Single<Double_t>("FOM_piminuscand");
  locTreeBranchRegister.Register_Single<Double_t>("NDF_piminuscand");
  locTreeBranchRegister.Register_Single<Double_t>("ChiSq_piminuscand");
  locTreeBranchRegister.Register_Single<Double_t>("piminus_dedx_dc_NDF");
  locTreeBranchRegister.Register_Single<Double_t>("piminus_dedx_dc_ChiSq");
  locTreeBranchRegister.Register_Single<Double_t>("Beta_Timing_piminuscand");
  locTreeBranchRegister.Register_Single<Double_t>("NDF_Timing_piminuscand");
  locTreeBranchRegister.Register_Single<Double_t>("ChiSq_Timing_piminuscand");
  locTreeBranchRegister.Register_Single<Double_t>("dEdx_TOF_piminuscand");
  locTreeBranchRegister.Register_Single<Double_t>("Energy_BCAL_piminuscand");
  locTreeBranchRegister.Register_Single<Double_t>("Energy_BCALPreshower_piminuscand");
  locTreeBranchRegister.Register_Single<Double_t>("SigLong_BCAL_piminuscand");
  locTreeBranchRegister.Register_Single<Double_t>("SigTheta_BCAL_piminuscand");
  locTreeBranchRegister.Register_Single<Double_t>("SigTrans_BCAL_piminuscand");
  locTreeBranchRegister.Register_Single<Double_t>("RMSTime_BCAL_piminuscand");
  locTreeBranchRegister.Register_Single<Double_t>("E_BCAL_piminuscand");
  locTreeBranchRegister.Register_Single<Double_t>("x_BCAL_piminuscand");
  locTreeBranchRegister.Register_Single<Double_t>("y_BCAL_piminuscand");
  locTreeBranchRegister.Register_Single<Double_t>("z_BCAL_piminuscand");
  locTreeBranchRegister.Register_Single<Double_t>("t_BCAL_piminuscand");
  locTreeBranchRegister.Register_Single<Double_t>("Energy_FCAL_piminuscand");
  locTreeBranchRegister.Register_Single<Double_t>("E1E9_FCAL_piminuscand");
  locTreeBranchRegister.Register_Single<Double_t>("E9E25_FCAL_piminuscand");
  locTreeBranchRegister.Register_Single<Double_t>("SumU_FCAL_piminuscand");
  locTreeBranchRegister.Register_Single<Double_t>("SumV_FCAL_piminuscand");
  locTreeBranchRegister.Register_Single<Double_t>("TrackBCAL_DeltaPhi_piminuscand");
  locTreeBranchRegister.Register_Single<Double_t>("TrackBCAL_DeltaZ_piminuscand");
  locTreeBranchRegister.Register_Single<Double_t>("TrackFCAL_DOCA_piminuscand");
  locTreeBranchRegister.Register_Single<Double_t>("dEdx_ST_piminuscand");
  locTreeBranchRegister.Register_Single<Double_t>("dEdx_CDC_piminuscand");
  locTreeBranchRegister.Register_Single<Double_t>("dEdx_FDC_piminuscand");

  locTreeBranchRegister.Register_Single<Double_t>("FOM_prot0cand");
  locTreeBranchRegister.Register_Single<Double_t>("NDF_prot0cand");
  locTreeBranchRegister.Register_Single<Double_t>("ChiSq_prot0cand");
  locTreeBranchRegister.Register_Single<Double_t>("prot0_dedx_dc_NDF");
  locTreeBranchRegister.Register_Single<Double_t>("prot0_dedx_dc_ChiSq");
  locTreeBranchRegister.Register_Single<Double_t>("Beta_Timing_prot0cand");
  locTreeBranchRegister.Register_Single<Double_t>("NDF_Timing_prot0cand");
  locTreeBranchRegister.Register_Single<Double_t>("ChiSq_Timing_prot0cand");
  locTreeBranchRegister.Register_Single<Double_t>("dEdx_TOF_prot0cand");
  locTreeBranchRegister.Register_Single<Double_t>("Energy_BCAL_prot0cand");
  locTreeBranchRegister.Register_Single<Double_t>("Energy_BCALPreshower_prot0cand");
  locTreeBranchRegister.Register_Single<Double_t>("SigLong_BCAL_prot0cand");
  locTreeBranchRegister.Register_Single<Double_t>("SigTheta_BCAL_prot0cand");
  locTreeBranchRegister.Register_Single<Double_t>("SigTrans_BCAL_prot0cand");
  locTreeBranchRegister.Register_Single<Double_t>("RMSTime_BCAL_prot0cand");
  locTreeBranchRegister.Register_Single<Double_t>("E_BCAL_prot0cand");
  locTreeBranchRegister.Register_Single<Double_t>("x_BCAL_prot0cand");
  locTreeBranchRegister.Register_Single<Double_t>("y_BCAL_prot0cand");
  locTreeBranchRegister.Register_Single<Double_t>("z_BCAL_prot0cand");
  locTreeBranchRegister.Register_Single<Double_t>("t_BCAL_prot0cand");
  locTreeBranchRegister.Register_Single<Double_t>("Energy_FCAL_prot0cand");
  locTreeBranchRegister.Register_Single<Double_t>("E1E9_FCAL_prot0cand");
  locTreeBranchRegister.Register_Single<Double_t>("E9E25_FCAL_prot0cand");
  locTreeBranchRegister.Register_Single<Double_t>("SumU_FCAL_prot0cand");
  locTreeBranchRegister.Register_Single<Double_t>("SumV_FCAL_prot0cand");
  locTreeBranchRegister.Register_Single<Double_t>("TrackBCAL_DeltaPhi_prot0cand");
  locTreeBranchRegister.Register_Single<Double_t>("TrackBCAL_DeltaZ_prot0cand");
  locTreeBranchRegister.Register_Single<Double_t>("TrackFCAL_DOCA_prot0cand");
  locTreeBranchRegister.Register_Single<Double_t>("dEdx_ST_prot0cand");
  locTreeBranchRegister.Register_Single<Double_t>("dEdx_CDC_prot0cand");
  locTreeBranchRegister.Register_Single<Double_t>("dEdx_FDC_prot0cand");

  locTreeBranchRegister.Register_Single<Double_t>("FOM_prot1cand");
  locTreeBranchRegister.Register_Single<Double_t>("NDF_prot1cand");
  locTreeBranchRegister.Register_Single<Double_t>("ChiSq_prot1cand");
  locTreeBranchRegister.Register_Single<Double_t>("prot1_dedx_dc_NDF");
  locTreeBranchRegister.Register_Single<Double_t>("prot1_dedx_dc_ChiSq");
  locTreeBranchRegister.Register_Single<Double_t>("Beta_Timing_prot1cand");
  locTreeBranchRegister.Register_Single<Double_t>("NDF_Timing_prot1cand");
  locTreeBranchRegister.Register_Single<Double_t>("ChiSq_Timing_prot1cand");
  locTreeBranchRegister.Register_Single<Double_t>("dEdx_TOF_prot1cand");
  locTreeBranchRegister.Register_Single<Double_t>("Energy_BCAL_prot1cand");
  locTreeBranchRegister.Register_Single<Double_t>("Energy_BCALPreshower_prot1cand");
  locTreeBranchRegister.Register_Single<Double_t>("SigLong_BCAL_prot1cand");
  locTreeBranchRegister.Register_Single<Double_t>("SigTheta_BCAL_prot1cand");
  locTreeBranchRegister.Register_Single<Double_t>("SigTrans_BCAL_prot1cand");
  locTreeBranchRegister.Register_Single<Double_t>("RMSTime_BCAL_prot1cand");
  locTreeBranchRegister.Register_Single<Double_t>("E_BCAL_prot1cand");
  locTreeBranchRegister.Register_Single<Double_t>("x_BCAL_prot1cand");
  locTreeBranchRegister.Register_Single<Double_t>("y_BCAL_prot1cand");
  locTreeBranchRegister.Register_Single<Double_t>("z_BCAL_prot1cand");
  locTreeBranchRegister.Register_Single<Double_t>("t_BCAL_prot1cand");
  locTreeBranchRegister.Register_Single<Double_t>("Energy_FCAL_prot1cand");
  locTreeBranchRegister.Register_Single<Double_t>("E1E9_FCAL_prot1cand");
  locTreeBranchRegister.Register_Single<Double_t>("E9E25_FCAL_prot1cand");
  locTreeBranchRegister.Register_Single<Double_t>("SumU_FCAL_prot1cand");
  locTreeBranchRegister.Register_Single<Double_t>("SumV_FCAL_prot1cand");
  locTreeBranchRegister.Register_Single<Double_t>("TrackBCAL_DeltaPhi_prot1cand");
  locTreeBranchRegister.Register_Single<Double_t>("TrackBCAL_DeltaZ_prot1cand");
  locTreeBranchRegister.Register_Single<Double_t>("TrackFCAL_DOCA_prot1cand");
  locTreeBranchRegister.Register_Single<Double_t>("dEdx_ST_prot1cand");
  locTreeBranchRegister.Register_Single<Double_t>("dEdx_CDC_prot1cand");
  locTreeBranchRegister.Register_Single<Double_t>("dEdx_FDC_prot1cand");

  //REGISTER BRANCHES
  dTreeInterface->Create_Branches(locTreeBranchRegister);

  // create root folder and cd to it, store main dir
  TDirectory *main = gDirectory;
  gDirectory->mkdir("SRC_2p1pi1pi0")->cd();

  h_BCAL = new TH2D("h_BCAL","BCAL;z [cm];r [cm]",100,-200.,800.,100,0.,200.);
  h_FCAL = new TH2D("h_FCAL","FCAL;z [cm];r [cm]",100,-200.,800.,100,0.,200.);
  h_m2gamma = new TH1D("h_m2gamma",";m2gamma [GeV];",100,0.,1.);
  h_m2pi = new TH1D("h_m2pi",";m2pi [GeV];",250,0.,5.);

  main->cd();

  return NOERROR;
}

//------------------
// brun
//------------------
jerror_t DEventProcessor_2p1pi1pi0::brun(JEventLoop *eventLoop, int32_t runnumber)
{
  dKinFitUtils = new DKinFitUtils_GlueX(eventLoop);
  dKinFitter = new DKinFitter(dKinFitUtils);

  return NOERROR;
}

//------------------
// evnt
//------------------
jerror_t DEventProcessor_2p1pi1pi0::evnt(JEventLoop *loop, uint64_t eventnumber)
{

  // Pulling detector information
  vector<const DBeamPhoton*> beam_ph;

  vector<const DChargedTrack*>ch_tracks;

  vector<const DNeutralShower*> showers;

  vector<const DMCThrown*> mcthrown;

  vector<const DMCReaction*> reaction;

  loop->Get(beam_ph);

  loop->Get(ch_tracks);

  loop->Get(showers);

  loop->Get(mcthrown);

  loop->Get(reaction);

  dTreeFillData.Fill_Single<Int_t>("eventNumber", eventnumber);

  double E_shower[2];
  double x_shower[2];
  double y_shower[2];
  double z_shower[2];
  double t_shower[2];
  bool BCAL_shower[2];

  if (mcthrown.size() > 0) {
    dTreeFillData.Fill_Single<Int_t>("N_thrown", mcthrown.size());
    dTreeFillData.Fill_Single<Int_t>("beamEnergy_thrown",reaction[0]->beam.energy());

    for (unsigned int ii = 0; ii < mcthrown.size(); ii++)
      {
	dTreeFillData.Fill_Array<Int_t>("type_thrown", mcthrown[ii]->type, ii);
	dTreeFillData.Fill_Array<Double_t>("pX_thrown", mcthrown[ii]->momentum().X(), ii);
	dTreeFillData.Fill_Array<Double_t>("pY_thrown", mcthrown[ii]->momentum().Y(), ii);
	dTreeFillData.Fill_Array<Double_t>("pZ_thrown", mcthrown[ii]->momentum().Z(), ii);
	dTreeFillData.Fill_Array<Double_t>("E_thrown", mcthrown[ii]->energy(), ii);
	dTreeFillData.Fill_Array<Int_t>("pdgtype_thrown", mcthrown[ii]->pdgtype, ii);
	dTreeFillData.Fill_Array<Int_t>("myid_thrown", mcthrown[ii]->myid, ii);
	dTreeFillData.Fill_Array<Int_t>("parentid_thrown", mcthrown[ii]->parentid, ii);
      }
  }
  
  fcal_ncl    = 0;
  fcal_en_cl  = 0;
  fcal_x_cl   = 0;
  fcal_y_cl   = 0;

  if ((ch_tracks.size() != 3) || (showers.size() != 2))
    return NOERROR;

  map<Particle_t, int> targetParticles = {
    {Proton,2},
    {PiMinus,1}
  };

  map<Particle_t, vector<const DChargedTrackHypothesis*> > emptyHypothesis;
  vector<map<Particle_t, vector<const DChargedTrackHypothesis*> > > hypothesisList;
  
  GetHypotheses(ch_tracks,targetParticles,emptyHypothesis,hypothesisList);
  
  Int_t nHyp = hypothesisList.size();
  
  dTreeFillData.Fill_Single<Int_t>("nPhotonCandidates", beam_ph.size());
  
  if(beam_ph.size() > 0)
    {
      
      for(unsigned int ii = 0; ii < beam_ph.size(); ii++)
	{
	  dTreeFillData.Fill_Array<Double_t>("beamEnergy", beam_ph[ii]->momentum().Mag(), ii);
	  dTreeFillData.Fill_Array<Double_t>("beamT", beam_ph[ii]->time(), ii);
	}
      
    }
  
  for (unsigned int ii = 0; ii<2; ii++)
    {
      
      const DNeutralShower *shower = showers[ii];
      
      E_shower[ii] = shower->dEnergy;
      x_shower[ii] = shower->dSpacetimeVertex.X();
      y_shower[ii] = shower->dSpacetimeVertex.Y();
      z_shower[ii] = shower->dSpacetimeVertex.Z();
      t_shower[ii] = shower->dSpacetimeVertex.T();
      BCAL_shower[ii] = (shower->dDetectorSystem == SYS_BCAL);

      // Filling shower positions
      japp->RootFillLock(this); //ACQUIRE ROOT FILL LOCK
      if (BCAL_shower[ii])
	{
	  h_BCAL->Fill(z_shower[ii],shower->dSpacetimeVertex.Perp());
	}
      else
	{
	  h_FCAL->Fill(z_shower[ii],shower->dSpacetimeVertex.Perp());
	}
      japp->RootFillUnLock(this); //RELEASE ROOT FILL LOCK
      
    }

  if (nHyp != 1)
    return NOERROR;
      
  LockState(); //ACQUIRE PROCESSOR LOCK

  map<Particle_t, vector<const DChargedTrackHypothesis*> > thisHyp = hypothesisList[0];
  const DChargedTrackHypothesis * hyp_pr0 = thisHyp[Proton][0];
  const DChargedTrackHypothesis * hyp_pr1 = thisHyp[Proton][1];
  const DChargedTrackHypothesis * hyp_pim = thisHyp[PiMinus][0];
  

  // Pi Minus Candidate
  double FOM_piminuscand          = hyp_pim->Get_FOM();
  double NDF_piminuscand          = hyp_pim->Get_NDF();
  double ChiSq_piminuscand        = hyp_pim->Get_ChiSq();
  double piminus_dedx_dc_NDF      = hyp_pim->Get_NDF_DCdEdx();
  double piminus_dedx_dc_ChiSq    = hyp_pim->Get_ChiSq_DCdEdx();
  double Beta_Timing_piminuscand  = hyp_pim->measuredBeta();
  double NDF_Timing_piminuscand   = hyp_pim->Get_NDF_Timing();
  double ChiSq_Timing_piminuscand = hyp_pim->Get_ChiSq_Timing();
  double dEdx_TOF_piminuscand     = (hyp_pim->Get_TOFHitMatchParams() != NULL) ? hyp_pim->Get_TOFHitMatchParams()->dEdx : 0.0;
  double dEdx_ST_piminuscand      = (hyp_pim->Get_SCHitMatchParams()  != NULL) ? hyp_pim->Get_SCHitMatchParams()->dEdx : 0.0;
  double dEdx_CDC_piminuscand      = (hyp_pim->Get_TrackTimeBased() != NULL) ? hyp_pim->Get_TrackTimeBased()->ddEdx_CDC : 0.0;
  double dEdx_FDC_piminuscand      = (hyp_pim->Get_TrackTimeBased() != NULL) ? hyp_pim->Get_TrackTimeBased()->ddEdx_FDC : 0.0;
  const DBCALShower* showerpiminus = NULL;
  if (hyp_pim->Get_BCALShowerMatchParams() != NULL) showerpiminus = hyp_pim->Get_BCALShowerMatchParams()->dBCALShower;
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
  if (hyp_pim->Get_FCALShowerMatchParams() != NULL) FCALshowerpiminus = hyp_pim->Get_FCALShowerMatchParams()->dFCALShower;
  double Energy_FCAL_piminuscand = (FCALshowerpiminus != NULL) ? FCALshowerpiminus->getEnergy() : 0.0;
  double E1E9_FCAL_piminuscand   = (FCALshowerpiminus != NULL) ? FCALshowerpiminus->getE1E9() : 0.0;
  double E9E25_FCAL_piminuscand  = (FCALshowerpiminus != NULL) ? FCALshowerpiminus->getE9E25() : 0.0;
  double SumU_FCAL_piminuscand   = (FCALshowerpiminus != NULL) ? FCALshowerpiminus->getSumU() : 0.0;
  double SumV_FCAL_piminuscand   = (FCALshowerpiminus != NULL) ? FCALshowerpiminus->getSumV() : 0.0;

  double TrackBCAL_DeltaPhi_piminuscand = ( hyp_pim->Get_BCALShowerMatchParams()  != NULL) ? hyp_pim->Get_BCALShowerMatchParams()->dDeltaPhiToShower : 999.0;
  double TrackBCAL_DeltaZ_piminuscand   = ( hyp_pim->Get_BCALShowerMatchParams()  != NULL) ? hyp_pim->Get_BCALShowerMatchParams()->dDeltaZToShower : 999.0;
  double TrackFCAL_DOCA_piminuscand     = ( hyp_pim->Get_FCALShowerMatchParams()  != NULL) ? hyp_pim->Get_FCALShowerMatchParams()->dDOCAToShower : 999.0;  

  dTreeFillData.Fill_Single<Double_t>("FOM_piminuscand",FOM_piminuscand);
  dTreeFillData.Fill_Single<Double_t>("NDF_piminuscand",NDF_piminuscand);
  dTreeFillData.Fill_Single<Double_t>("ChiSq_piminuscand",ChiSq_piminuscand);
  dTreeFillData.Fill_Single<Double_t>("piminus_dedx_dc_NDF",piminus_dedx_dc_NDF);
  dTreeFillData.Fill_Single<Double_t>("piminus_dedx_dc_ChiSq",piminus_dedx_dc_ChiSq);
  dTreeFillData.Fill_Single<Double_t>("Beta_Timing_piminuscand",Beta_Timing_piminuscand);
  dTreeFillData.Fill_Single<Double_t>("NDF_Timing_piminuscand",NDF_Timing_piminuscand);
  dTreeFillData.Fill_Single<Double_t>("ChiSq_Timing_piminuscand",ChiSq_Timing_piminuscand);
  dTreeFillData.Fill_Single<Double_t>("ChiSq_Timing_piminuscand",ChiSq_Timing_piminuscand);
  dTreeFillData.Fill_Single<Double_t>("dEdx_TOF_piminuscand",dEdx_TOF_piminuscand);
  dTreeFillData.Fill_Single<Double_t>("dEdx_ST_piminuscand",dEdx_ST_piminuscand);
  dTreeFillData.Fill_Single<Double_t>("dEdx_CDC_piminuscand",dEdx_CDC_piminuscand);
  dTreeFillData.Fill_Single<Double_t>("dEdx_FDC_piminuscand",dEdx_FDC_piminuscand);
  dTreeFillData.Fill_Single<Double_t>("Energy_BCAL_piminuscand",Energy_BCAL_piminuscand);
  dTreeFillData.Fill_Single<Double_t>("Energy_BCALPreshower_piminuscand",Energy_BCALPreshower_piminuscand);
  dTreeFillData.Fill_Single<Double_t>("SigLong_BCAL_piminuscand",SigLong_BCAL_piminuscand); 
  dTreeFillData.Fill_Single<Double_t>("SigTheta_BCAL_piminuscand",SigTheta_BCAL_piminuscand);
  dTreeFillData.Fill_Single<Double_t>("SigTrans_BCAL_piminuscand",SigTrans_BCAL_piminuscand);
  dTreeFillData.Fill_Single<Double_t>("RMSTime_BCAL_piminuscand",RMSTime_BCAL_piminuscand);
  dTreeFillData.Fill_Single<Double_t>("E_BCAL_piminuscand",E_BCAL_piminuscand);
  dTreeFillData.Fill_Single<Double_t>("x_BCAL_piminuscand",x_BCAL_piminuscand);
  dTreeFillData.Fill_Single<Double_t>("y_BCAL_piminuscand",y_BCAL_piminuscand);
  dTreeFillData.Fill_Single<Double_t>("z_BCAL_piminuscand",z_BCAL_piminuscand);
  dTreeFillData.Fill_Single<Double_t>("t_BCAL_piminuscand",t_BCAL_piminuscand);
  dTreeFillData.Fill_Single<Double_t>("Energy_FCAL_piminuscand",Energy_FCAL_piminuscand);
  dTreeFillData.Fill_Single<Double_t>("E1E9_FCAL_piminuscand",E1E9_FCAL_piminuscand);
  dTreeFillData.Fill_Single<Double_t>("E9E25_FCAL_piminuscand",E9E25_FCAL_piminuscand);
  dTreeFillData.Fill_Single<Double_t>("SumU_FCAL_piminuscand",SumU_FCAL_piminuscand);
  dTreeFillData.Fill_Single<Double_t>("SumV_FCAL_piminuscand",SumV_FCAL_piminuscand);
  dTreeFillData.Fill_Single<Double_t>("TrackBCAL_DeltaPhi_piminuscand",TrackBCAL_DeltaPhi_piminuscand);
  dTreeFillData.Fill_Single<Double_t>("TrackBCAL_DeltaZ_piminuscand",TrackBCAL_DeltaZ_piminuscand);
  dTreeFillData.Fill_Single<Double_t>("TrackFCAL_DOCA_piminuscand",TrackFCAL_DOCA_piminuscand);

  // Proton Candidate 0
  double FOM_prot0cand          = hyp_pr0->Get_FOM();
  double NDF_prot0cand          = hyp_pr0->Get_NDF();
  double ChiSq_prot0cand        = hyp_pr0->Get_ChiSq();
  double prot0_dedx_dc_NDF      = hyp_pr0->Get_NDF_DCdEdx();
  double prot0_dedx_dc_ChiSq    = hyp_pr0->Get_ChiSq_DCdEdx();
  double Beta_Timing_prot0cand  = hyp_pr0->measuredBeta();
  double NDF_Timing_prot0cand   = hyp_pr0->Get_NDF_Timing();
  double ChiSq_Timing_prot0cand = hyp_pr0->Get_ChiSq_Timing();
  double dEdx_TOF_prot0cand     = (hyp_pr0->Get_TOFHitMatchParams() != NULL) ? hyp_pr0->Get_TOFHitMatchParams()->dEdx : 0.0;
  double dEdx_ST_prot0cand      = (hyp_pr0->Get_SCHitMatchParams()  != NULL) ? hyp_pr0->Get_SCHitMatchParams()->dEdx : 0.0;
  double dEdx_CDC_prot0cand      = (hyp_pim->Get_TrackTimeBased() != NULL) ? hyp_pim->Get_TrackTimeBased()->ddEdx_CDC : 0.0;
  double dEdx_FDC_prot0cand      = (hyp_pim->Get_TrackTimeBased() != NULL) ? hyp_pim->Get_TrackTimeBased()->ddEdx_FDC : 0.0;
  const DBCALShower* showerprot0 = NULL;
  if (hyp_pr0->Get_BCALShowerMatchParams() != NULL) showerprot0 = hyp_pr0->Get_BCALShowerMatchParams()->dBCALShower;
  double Energy_BCAL_prot0cand  = (showerprot0 != NULL ) ? showerprot0->E : 0.0;
  double Energy_BCALPreshower_prot0cand  = (showerprot0 != NULL ) ? showerprot0->E_preshower : 0.0;
  double SigLong_BCAL_prot0cand = (showerprot0 != NULL ) ? showerprot0->sigLong : 0.0;
  double SigTheta_BCAL_prot0cand= (showerprot0 != NULL ) ? showerprot0->sigTheta : 0.0;
  double SigTrans_BCAL_prot0cand= (showerprot0 != NULL ) ? showerprot0->sigTrans : 0.0;
  double RMSTime_BCAL_prot0cand = (showerprot0 != NULL ) ? showerprot0->rmsTime : 0.0;
  double E_BCAL_prot0cand = (showerprot0 != NULL ) ? showerprot0->E : 0.0;
  double x_BCAL_prot0cand = (showerprot0 != NULL ) ? showerprot0->x : 0.0;
  double y_BCAL_prot0cand = (showerprot0 != NULL ) ? showerprot0->y : 0.0;
  double z_BCAL_prot0cand = (showerprot0 != NULL ) ? showerprot0->z : 0.0;
  double t_BCAL_prot0cand = (showerprot0 != NULL ) ? showerprot0->t : 0.0;

  const DFCALShower* FCALshowerprot0 = NULL;
  if (hyp_pr0->Get_FCALShowerMatchParams() != NULL) FCALshowerprot0 = hyp_pr0->Get_FCALShowerMatchParams()->dFCALShower;
  double Energy_FCAL_prot0cand = (FCALshowerprot0 != NULL) ? FCALshowerprot0->getEnergy() : 0.0;
  double E1E9_FCAL_prot0cand   = (FCALshowerprot0 != NULL) ? FCALshowerprot0->getE1E9() : 0.0;
  double E9E25_FCAL_prot0cand  = (FCALshowerprot0 != NULL) ? FCALshowerprot0->getE9E25() : 0.0;
  double SumU_FCAL_prot0cand   = (FCALshowerprot0 != NULL) ? FCALshowerprot0->getSumU() : 0.0;
  double SumV_FCAL_prot0cand   = (FCALshowerprot0 != NULL) ? FCALshowerprot0->getSumV() : 0.0;

  double TrackBCAL_DeltaPhi_prot0cand = ( hyp_pr0->Get_BCALShowerMatchParams()  != NULL) ? hyp_pr0->Get_BCALShowerMatchParams()->dDeltaPhiToShower : 999.0;
  double TrackBCAL_DeltaZ_prot0cand   = ( hyp_pr0->Get_BCALShowerMatchParams()  != NULL) ? hyp_pr0->Get_BCALShowerMatchParams()->dDeltaZToShower : 999.0;
  double TrackFCAL_DOCA_prot0cand     = ( hyp_pr0->Get_FCALShowerMatchParams()  != NULL) ? hyp_pr0->Get_FCALShowerMatchParams()->dDOCAToShower : 999.0;

  dTreeFillData.Fill_Single<Double_t>("FOM_prot0cand",FOM_prot0cand);
  dTreeFillData.Fill_Single<Double_t>("NDF_prot0cand",NDF_prot0cand);
  dTreeFillData.Fill_Single<Double_t>("ChiSq_prot0cand",ChiSq_prot0cand);
  dTreeFillData.Fill_Single<Double_t>("prot0_dedx_dc_NDF",prot0_dedx_dc_NDF);
  dTreeFillData.Fill_Single<Double_t>("prot0_dedx_dc_ChiSq",prot0_dedx_dc_ChiSq);
  dTreeFillData.Fill_Single<Double_t>("Beta_Timing_prot0cand",Beta_Timing_prot0cand);
  dTreeFillData.Fill_Single<Double_t>("NDF_Timing_prot0cand",NDF_Timing_prot0cand);
  dTreeFillData.Fill_Single<Double_t>("ChiSq_Timing_prot0cand",ChiSq_Timing_prot0cand);
  dTreeFillData.Fill_Single<Double_t>("ChiSq_Timing_prot0cand",ChiSq_Timing_prot0cand);
  dTreeFillData.Fill_Single<Double_t>("dEdx_TOF_prot0cand",dEdx_TOF_prot0cand);
  dTreeFillData.Fill_Single<Double_t>("dEdx_ST_prot0cand",dEdx_ST_prot0cand);
  dTreeFillData.Fill_Single<Double_t>("dEdx_CDC_prot0cand",dEdx_CDC_prot0cand);
  dTreeFillData.Fill_Single<Double_t>("dEdx_FDC_prot0cand",dEdx_FDC_prot0cand);
  dTreeFillData.Fill_Single<Double_t>("Energy_BCAL_prot0cand",Energy_BCAL_prot0cand);
  dTreeFillData.Fill_Single<Double_t>("Energy_BCALPreshower_prot0cand",Energy_BCALPreshower_prot0cand);
  dTreeFillData.Fill_Single<Double_t>("SigLong_BCAL_prot0cand",SigLong_BCAL_prot0cand);
  dTreeFillData.Fill_Single<Double_t>("SigTheta_BCAL_prot0cand",SigTheta_BCAL_prot0cand);
  dTreeFillData.Fill_Single<Double_t>("SigTrans_BCAL_prot0cand",SigTrans_BCAL_prot0cand);
  dTreeFillData.Fill_Single<Double_t>("RMSTime_BCAL_prot0cand",RMSTime_BCAL_prot0cand);
  dTreeFillData.Fill_Single<Double_t>("E_BCAL_prot0cand",E_BCAL_prot0cand);
  dTreeFillData.Fill_Single<Double_t>("x_BCAL_prot0cand",x_BCAL_prot0cand);
  dTreeFillData.Fill_Single<Double_t>("y_BCAL_prot0cand",y_BCAL_prot0cand);
  dTreeFillData.Fill_Single<Double_t>("z_BCAL_prot0cand",z_BCAL_prot0cand);
  dTreeFillData.Fill_Single<Double_t>("t_BCAL_prot0cand",t_BCAL_prot0cand);
  dTreeFillData.Fill_Single<Double_t>("Energy_FCAL_prot0cand",Energy_FCAL_prot0cand);
  dTreeFillData.Fill_Single<Double_t>("E1E9_FCAL_prot0cand",E1E9_FCAL_prot0cand);
  dTreeFillData.Fill_Single<Double_t>("E9E25_FCAL_prot0cand",E9E25_FCAL_prot0cand);
  dTreeFillData.Fill_Single<Double_t>("SumU_FCAL_prot0cand",SumU_FCAL_prot0cand);
  dTreeFillData.Fill_Single<Double_t>("SumV_FCAL_prot0cand",SumV_FCAL_prot0cand);
  dTreeFillData.Fill_Single<Double_t>("TrackBCAL_DeltaPhi_prot0cand",TrackBCAL_DeltaPhi_prot0cand);
  dTreeFillData.Fill_Single<Double_t>("TrackBCAL_DeltaZ_prot0cand",TrackBCAL_DeltaZ_prot0cand);
  dTreeFillData.Fill_Single<Double_t>("TrackFCAL_DOCA_prot0cand",TrackFCAL_DOCA_prot0cand);

  // Proton Candidate 1
  double FOM_prot1cand          = hyp_pr1->Get_FOM();
  double NDF_prot1cand          = hyp_pr1->Get_NDF();
  double ChiSq_prot1cand        = hyp_pr1->Get_ChiSq();
  double prot1_dedx_dc_NDF      = hyp_pr1->Get_NDF_DCdEdx();
  double prot1_dedx_dc_ChiSq    = hyp_pr1->Get_ChiSq_DCdEdx();
  double Beta_Timing_prot1cand  = hyp_pr1->measuredBeta();
  double NDF_Timing_prot1cand   = hyp_pr1->Get_NDF_Timing();
  double ChiSq_Timing_prot1cand = hyp_pr1->Get_ChiSq_Timing();
  double dEdx_TOF_prot1cand     = (hyp_pr1->Get_TOFHitMatchParams() != NULL) ? hyp_pr1->Get_TOFHitMatchParams()->dEdx : 0.0;
  double dEdx_ST_prot1cand      = (hyp_pr1->Get_SCHitMatchParams()  != NULL) ? hyp_pr1->Get_SCHitMatchParams()->dEdx : 0.0;
  double dEdx_CDC_prot1cand      = (hyp_pr1->Get_TrackTimeBased() != NULL) ? hyp_pr1->Get_TrackTimeBased()->ddEdx_CDC : 0.0;
  double dEdx_FDC_prot1cand      = (hyp_pr1->Get_TrackTimeBased() != NULL) ? hyp_pr1->Get_TrackTimeBased()->ddEdx_FDC : 0.0;
  const DBCALShower* showerprot1 = NULL;
  if (hyp_pr1->Get_BCALShowerMatchParams() != NULL) showerprot1 = hyp_pr1->Get_BCALShowerMatchParams()->dBCALShower;
  double Energy_BCAL_prot1cand  = (showerprot1 != NULL ) ? showerprot1->E : 0.0;
  double Energy_BCALPreshower_prot1cand  = (showerprot1 != NULL ) ? showerprot1->E_preshower : 0.0;
  double SigLong_BCAL_prot1cand = (showerprot1 != NULL ) ? showerprot1->sigLong : 0.0;
  double SigTheta_BCAL_prot1cand= (showerprot1 != NULL ) ? showerprot1->sigTheta : 0.0;
  double SigTrans_BCAL_prot1cand= (showerprot1 != NULL ) ? showerprot1->sigTrans : 0.0;
  double RMSTime_BCAL_prot1cand = (showerprot1 != NULL ) ? showerprot1->rmsTime : 0.0;
  double E_BCAL_prot1cand = (showerprot1 != NULL ) ? showerprot1->E : 0.0;
  double x_BCAL_prot1cand = (showerprot1 != NULL ) ? showerprot1->x : 0.0;
  double y_BCAL_prot1cand = (showerprot1 != NULL ) ? showerprot1->y : 0.0;
  double z_BCAL_prot1cand = (showerprot1 != NULL ) ? showerprot1->z : 0.0;
  double t_BCAL_prot1cand = (showerprot1 != NULL ) ? showerprot1->t : 0.0;

  const DFCALShower* FCALshowerprot1 = NULL;
  if (hyp_pr1->Get_FCALShowerMatchParams() != NULL) FCALshowerprot1 = hyp_pr1->Get_FCALShowerMatchParams()->dFCALShower;
  double Energy_FCAL_prot1cand = (FCALshowerprot1 != NULL) ? FCALshowerprot1->getEnergy() : 0.0;
  double E1E9_FCAL_prot1cand   = (FCALshowerprot1 != NULL) ? FCALshowerprot1->getE1E9() : 0.0;
  double E9E25_FCAL_prot1cand  = (FCALshowerprot1 != NULL) ? FCALshowerprot1->getE9E25() : 0.0;
  double SumU_FCAL_prot1cand   = (FCALshowerprot1 != NULL) ? FCALshowerprot1->getSumU() : 0.0;
  double SumV_FCAL_prot1cand   = (FCALshowerprot1 != NULL) ? FCALshowerprot1->getSumV() : 0.0;

  double TrackBCAL_DeltaPhi_prot1cand = ( hyp_pr1->Get_BCALShowerMatchParams()  != NULL) ? hyp_pr1->Get_BCALShowerMatchParams()->dDeltaPhiToShower : 999.0;
  double TrackBCAL_DeltaZ_prot1cand   = ( hyp_pr1->Get_BCALShowerMatchParams()  != NULL) ? hyp_pr1->Get_BCALShowerMatchParams()->dDeltaZToShower : 999.0;
  double TrackFCAL_DOCA_prot1cand     = ( hyp_pr1->Get_FCALShowerMatchParams()  != NULL) ? hyp_pr1->Get_FCALShowerMatchParams()->dDOCAToShower : 999.0;

  dTreeFillData.Fill_Single<Double_t>("FOM_prot1cand",FOM_prot1cand);
  dTreeFillData.Fill_Single<Double_t>("NDF_prot1cand",NDF_prot1cand);
  dTreeFillData.Fill_Single<Double_t>("ChiSq_prot1cand",ChiSq_prot1cand);
  dTreeFillData.Fill_Single<Double_t>("prot1_dedx_dc_NDF",prot1_dedx_dc_NDF);
  dTreeFillData.Fill_Single<Double_t>("prot1_dedx_dc_ChiSq",prot1_dedx_dc_ChiSq);
  dTreeFillData.Fill_Single<Double_t>("Beta_Timing_prot1cand",Beta_Timing_prot1cand);
  dTreeFillData.Fill_Single<Double_t>("NDF_Timing_prot1cand",NDF_Timing_prot1cand);
  dTreeFillData.Fill_Single<Double_t>("ChiSq_Timing_prot1cand",ChiSq_Timing_prot1cand);
  dTreeFillData.Fill_Single<Double_t>("ChiSq_Timing_prot1cand",ChiSq_Timing_prot1cand);
  dTreeFillData.Fill_Single<Double_t>("dEdx_TOF_prot1cand",dEdx_TOF_prot1cand);
  dTreeFillData.Fill_Single<Double_t>("dEdx_ST_prot1cand",dEdx_ST_prot1cand);
  dTreeFillData.Fill_Single<Double_t>("dEdx_CDC_prot1cand",dEdx_CDC_prot1cand);
  dTreeFillData.Fill_Single<Double_t>("dEdx_FDC_prot1cand",dEdx_FDC_prot1cand);
  dTreeFillData.Fill_Single<Double_t>("Energy_BCAL_prot1cand",Energy_BCAL_prot1cand);
  dTreeFillData.Fill_Single<Double_t>("Energy_BCALPreshower_prot1cand",Energy_BCALPreshower_prot1cand);
  dTreeFillData.Fill_Single<Double_t>("SigLong_BCAL_prot1cand",SigLong_BCAL_prot1cand);
  dTreeFillData.Fill_Single<Double_t>("SigTheta_BCAL_prot1cand",SigTheta_BCAL_prot1cand);
  dTreeFillData.Fill_Single<Double_t>("SigTrans_BCAL_prot1cand",SigTrans_BCAL_prot1cand);
  dTreeFillData.Fill_Single<Double_t>("RMSTime_BCAL_prot1cand",RMSTime_BCAL_prot1cand);
  dTreeFillData.Fill_Single<Double_t>("E_BCAL_prot1cand",E_BCAL_prot1cand);
  dTreeFillData.Fill_Single<Double_t>("x_BCAL_prot1cand",x_BCAL_prot1cand);
  dTreeFillData.Fill_Single<Double_t>("y_BCAL_prot1cand",y_BCAL_prot1cand);
  dTreeFillData.Fill_Single<Double_t>("z_BCAL_prot1cand",z_BCAL_prot1cand);
  dTreeFillData.Fill_Single<Double_t>("t_BCAL_prot1cand",t_BCAL_prot1cand);
  dTreeFillData.Fill_Single<Double_t>("Energy_FCAL_prot1cand",Energy_FCAL_prot1cand);
  dTreeFillData.Fill_Single<Double_t>("E1E9_FCAL_prot1cand",E1E9_FCAL_prot1cand);
  dTreeFillData.Fill_Single<Double_t>("E9E25_FCAL_prot1cand",E9E25_FCAL_prot1cand);
  dTreeFillData.Fill_Single<Double_t>("SumU_FCAL_prot1cand",SumU_FCAL_prot1cand);
  dTreeFillData.Fill_Single<Double_t>("SumV_FCAL_prot1cand",SumV_FCAL_prot1cand);
  dTreeFillData.Fill_Single<Double_t>("TrackBCAL_DeltaPhi_prot1cand",TrackBCAL_DeltaPhi_prot1cand);
  dTreeFillData.Fill_Single<Double_t>("TrackBCAL_DeltaZ_prot1cand",TrackBCAL_DeltaZ_prot1cand);
  dTreeFillData.Fill_Single<Double_t>("TrackFCAL_DOCA_prot1cand",TrackFCAL_DOCA_prot1cand);

  dKinFitUtils->Reset_NewEvent();
  dKinFitter->Reset_NewEvent();
  dKinFitter->Reset_NewFit();
  
  set<shared_ptr<DKinFitParticle> > FinalParticles, FinalPhotons, NoParticles;
  
  shared_ptr<DKinFitParticle> myProton0 = dKinFitUtils->Make_DetectedParticle(hyp_pr0->Get_TrackTimeBased());
  shared_ptr<DKinFitParticle> myProton1 = dKinFitUtils->Make_DetectedParticle(hyp_pr1->Get_TrackTimeBased());
  shared_ptr<DKinFitParticle> myPiMinus = dKinFitUtils->Make_DetectedParticle(hyp_pim->Get_TrackTimeBased());
  FinalParticles.insert(myProton0);
  FinalParticles.insert(myProton1);
  FinalParticles.insert(myPiMinus);
  
  shared_ptr<DKinFitParticle> myPhoton0 = dKinFitUtils->Make_DetectedShower(showers[0],Gamma);
  shared_ptr<DKinFitParticle> myPhoton1 = dKinFitUtils->Make_DetectedShower(showers[1],Gamma);
  FinalParticles.insert(myPhoton0);
  FinalParticles.insert(myPhoton1);
  FinalPhotons.insert(myPhoton0);
  FinalPhotons.insert(myPhoton1);
  
  shared_ptr<DKinFitParticle> myPion = dKinFitUtils->Make_DecayingParticle(Pi0,NoParticles,FinalPhotons);
  
  shared_ptr<DKinFitConstraint_Vertex> locProductionVertexConstraint = dKinFitUtils->Make_VertexConstraint(FinalParticles,NoParticles,hyp_pr0->Get_TrackTimeBased()->position());
  dKinFitter->Add_Constraint(locProductionVertexConstraint);
  
  shared_ptr<DKinFitConstraint_Mass> pionMassConstraint = dKinFitUtils->Make_MassConstraint(myPion);
  dKinFitter->Add_Constraint(pionMassConstraint);
  
  dKinFitter->Fit_Reaction();
  
  dTreeFillData.Fill_Single<Double_t>("CL",dKinFitter->Get_ConfidenceLevel());
  if (dKinFitter->Get_ConfidenceLevel() == 0){
    UnlockState(); //RELEASE PROCESSOR LOCK
    return NOERROR;
  }
  
  TVector3 vertex;
  double vertex_x = 0;
  double vertex_y = 0;
  double vertex_z = 0;
  double vertex_t = 0;
  
  shared_ptr<DKinFitParticle> fitProton0 = NULL;
  shared_ptr<DKinFitParticle> fitProton1 = NULL;
  shared_ptr<DKinFitParticle> fitPiMinus = NULL;
  shared_ptr<DKinFitParticle> fitPhoton0 = NULL;
  shared_ptr<DKinFitParticle> fitPhoton1 = NULL;
  
  // Loop over particles
  set<shared_ptr<DKinFitParticle> >myParticles = dKinFitter->Get_KinFitParticles();
  set<shared_ptr<DKinFitParticle> >::iterator locParticleIterator=myParticles.begin();
  for(; locParticleIterator != myParticles.end(); ++locParticleIterator)
    {
      
      int pid = (*locParticleIterator)->Get_PID();
      
      switch (pid)
	{
	case 2212:
	  if (fitProton0 == NULL)
	    {
	      fitProton0 = (*locParticleIterator);
	      vertex = fitProton0->Get_Position();
	      vertex_x = vertex.X();
	      vertex_y = vertex.Y();
	      vertex_z = vertex.Z();
	      vertex_t = fitProton0->Get_Time();
	    }
	  else
	    {
	      fitProton1 = (*locParticleIterator);
	    }
	  break;
	case -211:
	  fitPiMinus = (*locParticleIterator);
	  break;
	case 22:
	  if (fitPhoton0 == NULL)
	    {
	      fitPhoton0 = (*locParticleIterator);
	    }
	  else
	    {
	      fitPhoton1 = (*locParticleIterator);
	    }
	  break;
	case 111:
	  break;
	default:
	  cout << pid << "\n\n";
	  break;
	}
      
    }
  
  TVector3 vertexPos(vertex_x,vertex_y,vertex_z);
  
  dTreeFillData.Fill_Single<Double_t>("vertex_X", vertex_x);
  dTreeFillData.Fill_Single<Double_t>("vertex_Y", vertex_y);
  dTreeFillData.Fill_Single<Double_t>("vertex_Z", vertex_z);
  dTreeFillData.Fill_Single<Double_t>("vertex_T", vertex_t);
  
  // Showers using fit vertex
  TVector3 p_shower0(x_shower[0]-vertex_x,y_shower[0]-vertex_y,z_shower[0]-vertex_z);
  p_shower0 = p_shower0.Unit()*E_shower[0];
  
  TVector3 p_shower1(x_shower[1]-vertex_x,y_shower[1]-vertex_y,z_shower[1]-vertex_z);
  p_shower1 = p_shower1.Unit()*E_shower[1];
  
  // Filling invariant mass without fitting
  double m2gammaSq = 2.*(p_shower1.Mag()*p_shower0.Mag() - p_shower1.Dot(p_shower0));
  
  TLorentzVector vProton0(fitProton0->Get_Momentum(),sqrt(fitProton0->Get_Momentum().Mag2() + mN*mN));
  TLorentzVector vProton1(fitProton1->Get_Momentum(),sqrt(fitProton1->Get_Momentum().Mag2() + mN*mN));
  TLorentzVector vPiMinus(fitPiMinus->Get_Momentum(),sqrt(fitPiMinus->Get_Momentum().Mag2() + mpip*mpip));
  TLorentzVector vPhoton1(fitPhoton1->Get_Momentum(),fitPhoton1->Get_Momentum().Mag());
  TLorentzVector vPhoton0(fitPhoton0->Get_Momentum(),fitPhoton0->Get_Momentum().Mag());
  TLorentzVector vPi0 = vPhoton0 + vPhoton1;
  
  japp->RootFillLock(this); //ACQUIRE ROOT FILL LOCK
  h_m2gamma->Fill(sqrt(m2gammaSq));
  h_m2pi->Fill((vPi0+vPiMinus).M());
  japp->RootFillUnLock(this); //RELEASE ROOT FILL LOCK
  
  dTreeFillData.Fill_Single<Double_t>("pX_Proton0", vProton0.X());
  dTreeFillData.Fill_Single<Double_t>("pY_Proton0", vProton0.Y());
  dTreeFillData.Fill_Single<Double_t>("pZ_Proton0", vProton0.Z());
  dTreeFillData.Fill_Single<Double_t>("pX_Proton1", vProton1.X());
  dTreeFillData.Fill_Single<Double_t>("pY_Proton1", vProton1.Y());
  dTreeFillData.Fill_Single<Double_t>("pZ_Proton1", vProton1.Z());
  dTreeFillData.Fill_Single<Double_t>("pX_PiMinus", vPiMinus.X());
  dTreeFillData.Fill_Single<Double_t>("pY_PiMinus", vPiMinus.Y());
  dTreeFillData.Fill_Single<Double_t>("pZ_PiMinus", vPiMinus.Z());
  dTreeFillData.Fill_Single<Double_t>("pX_Photon0", vPhoton0.X());
  dTreeFillData.Fill_Single<Double_t>("pY_Photon0", vPhoton0.Y());
  dTreeFillData.Fill_Single<Double_t>("pZ_Photon0", vPhoton0.Z());
  dTreeFillData.Fill_Single<Double_t>("pX_Photon1", vPhoton1.X());
  dTreeFillData.Fill_Single<Double_t>("pY_Photon1", vPhoton1.Y());
  dTreeFillData.Fill_Single<Double_t>("pZ_Photon1", vPhoton1.Z());
  
  dTreeFillData.Fill_Single<Double_t>("E_shower0", E_shower[0]);
  dTreeFillData.Fill_Single<Double_t>("x_shower0", x_shower[0]);
  dTreeFillData.Fill_Single<Double_t>("y_shower0", y_shower[0]);
  dTreeFillData.Fill_Single<Double_t>("z_shower0", z_shower[0]);
  dTreeFillData.Fill_Single<Double_t>("t_shower0", t_shower[0]);
  dTreeFillData.Fill_Single<Bool_t>("BCAL_shower0", BCAL_shower[0]);
  dTreeFillData.Fill_Single<Double_t>("E_shower1", E_shower[1]);
  dTreeFillData.Fill_Single<Double_t>("x_shower1", x_shower[1]);
  dTreeFillData.Fill_Single<Double_t>("y_shower1", y_shower[1]);
  dTreeFillData.Fill_Single<Double_t>("z_shower1", z_shower[1]);
  dTreeFillData.Fill_Single<Double_t>("t_shower1", t_shower[1]);
  dTreeFillData.Fill_Single<Bool_t>("BCAL_shower1", BCAL_shower[1]);
  
  //FILL TTREE
  dTreeInterface->Fill(dTreeFillData);
  
  UnlockState(); //RELEASE PROCESSOR LOCK

  return NOERROR;

}

//------------------
// erun
//------------------
jerror_t DEventProcessor_2p1pi1pi0::erun(void)
{
  // Any final calculations on histograms (like dividing them)
  // should be done here. This may get called more than once.
  return NOERROR;
}

//------------------
// fini
//------------------
jerror_t DEventProcessor_2p1pi1pi0::fini(void)
{

  delete dTreeInterface; //saves trees to file, closes file

  return NOERROR;
}

// Recursive function for determining possible particle assignments
void DEventProcessor_2p1pi1pi0::GetHypotheses(vector<const DChargedTrack *> &tracks,
						map<Particle_t, int> &particles,
						map<Particle_t, vector<const DChargedTrackHypothesis*> > &assignmentHypothesis,
						vector<map<Particle_t, vector<const DChargedTrackHypothesis*> > > &hypothesisList
						) const
{

  const DChargedTrack * firstTrack = tracks.front();
  vector<const DChargedTrack *> otherTracks(tracks);
  otherTracks.erase(otherTracks.begin());
  map<Particle_t, int>::iterator partIt;
  
  for (partIt = particles.begin(); partIt != particles.end(); partIt++)
    {
      if (partIt->second == 0)
	continue;
      
      Particle_t particle = partIt->first;
      const DChargedTrackHypothesis *hyp=NULL;
      
      if ((hyp = firstTrack->Get_Hypothesis(particle)) == NULL)
	continue;
      
      double prob = TMath::Prob(hyp->Get_ChiSq_DCdEdx(),hyp->Get_NDF_DCdEdx());
      
      if (prob <= 0)
	continue;
      
      map<Particle_t, vector<const DChargedTrackHypothesis*> > newHypothesis = assignmentHypothesis;
      
      if (assignmentHypothesis.find(particle) == assignmentHypothesis.end())
	{
	  vector<const DChargedTrackHypothesis*> newVector;
	  newHypothesis[particle] = newVector;
	}
      
      newHypothesis[particle].push_back(hyp);		    
      
      if (otherTracks.empty())
	{
	  hypothesisList.push_back(newHypothesis);
	}
      else
	{
	  map<Particle_t, int> otherParticles(particles);
	  otherParticles[particle]--;
	  GetHypotheses(otherTracks,
			otherParticles,
			newHypothesis,
			hypothesisList
			);
	  
	}    
      
    }
  
}
