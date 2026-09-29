// $Id$
//
//    File: JEventProcessor_npp_ana.cc
// Created: Mon Sep 29 02:32:05 PM EDT 2025
// Creator: ilarin (on Linux ifarm2401.jlab.org 5.14.0-570.33.2.el9_6.x86_64 x86_64)
//

/// For more information on the syntax changes between JANA1 and JANA2, visit: https://jeffersonlab.github.io/JANA2/#/jana1to2/jana1-to-jana2

#include "JEventProcessor_npp_ana.h"

#define _POSITION_METHOD_ getPosition_log

// Routine used to create our JEventProcessor
#include <JANA/JApplication.h>

extern "C"{
void InitPlugin(JApplication *app){
    InitJANAPlugin(app);
    app->Add(new JEventProcessor_npp_ana());
}
} // "C"


//------------------
// JEventProcessor_npp_ana (Constructor)
//------------------
JEventProcessor_npp_ana::JEventProcessor_npp_ana()
{
	SetTypeName(NAME_OF_THIS); // Provide JANA with this class's name
}

//------------------
// ~JEventProcessor_npp_ana (Destructor)
//------------------
JEventProcessor_npp_ana::~JEventProcessor_npp_ana()
{
}

//------------------
// Init
//------------------
void JEventProcessor_npp_ana::Init()
{
    auto app = GetApplication();
    lockService   = app->GetService<JLockService>();

    h_u           = new TH1D("h_u","h_u", 20, -0.5, 19.5);
    h_trk_dt      = new TH1D("h_trk_dt","h_trk_dt", 100, -5., 5.);

    h_de[0]       = new TH1D("h_de0","h_de0", 50, -2.0, 2.0);
    h_de[1]       = new TH1D("h_de1","h_de1", 50, -2.0, 2.0);
    h_de_acc[0]   = new TH1D("h_de_acc0","h_de_acc0", 50, -2.0, 2.0);
    h_de_acc[1]   = new TH1D("h_de_acc1","h_de_acc1", 50, -2.0, 2.0);

    tree  = new TTree( "ptree", "ptree" );

    tree->Branch("run",      &nt_run,      "run/I");
    tree->Branch("event",    &nt_event,    "event/I");
    tree->Branch("tgt",      &nt_tgt,      "tgt/I");
    tree->Branch("nchtrk",   &nt_nchtrk,   "nchtrk/I");
    tree->Branch("nfcal",    &nt_nfcal,    "nfcal/I");
    tree->Branch("nbcal",    &nt_nbcal,    "nbcal/I");
    tree->Branch("nneu",     &nt_nneu,     "nneu/I");
    tree->Branch("cbeam",    &nt_cbeam,    "cbeam/I");
    tree->Branch("det1",     &nt_det1,     "det1/I");
    tree->Branch("det2",     &nt_det2,     "det2/I");
    tree->Branch("det3",     &nt_det3,     "det3/I");
    tree->Branch("det4",     &nt_det4,     "det4/I");
    tree->Branch("ovlp1",    &nt_ovlp1,    "ovlp1/I");
    tree->Branch("ovlp2",    &nt_ovlp2,    "ovlp2/I");
    tree->Branch("ovlp3",    &nt_ovlp3,    "ovlp3/I");
    tree->Branch("ovlp4",    &nt_ovlp4,    "ovlp4/I");
    tree->Branch("nbpair",   &nt_nbpair,   "nbpair/I");
    tree->Branch("rankpair", &nt_rankpair, "rankpair/I");

    tree->Branch("rftime",   &nt_rftime,   "rftime/F");
    tree->Branch("rftime2",  &nt_rftime2,  "rftime2/F");
    tree->Branch("eneu",     &nt_eneu,     "eneu/F");
    tree->Branch("eneu5",    &nt_eneu5,    "eneu5/F");
    tree->Branch("tbeam",    &nt_tbeam,    "tbeam/F");
    tree->Branch("ebeam",    &nt_ebeam,    "ebeam/F");
    tree->Branch("eg1",      &nt_eg1,      "eg1/F");
    tree->Branch("eg2",      &nt_eg2,      "eg2/F");
    tree->Branch("eg3",      &nt_eg3,      "eg3/F");
    tree->Branch("eg4",      &nt_eg4,      "eg4/F");
    tree->Branch("epi01",    &nt_epi01,    "epi01/F");
    tree->Branch("epi02",    &nt_epi02,    "epi02/F");
    tree->Branch("dmpi01",   &nt_dmpi01,   "dmpi01/F");
    tree->Branch("dmpi02",   &nt_dmpi02,   "dmpi02/F");
    tree->Branch("tpi01",    &nt_tpi01,    "tpi01/F");
    tree->Branch("tpi02",    &nt_tpi02,    "tpi02/F");
    tree->Branch("epair",    &nt_epair,    "epair/F");
    tree->Branch("ecpair",   &nt_ecpair,   "ecpair/F");
    tree->Branch("mpair",    &nt_mpair,    "mpair/F");
    tree->Branch("cpair",    &nt_mcpair,   "mcpair/F");
    tree->Branch("tcpair",   &nt_tcpair,   "tcpair/F");
    tree->Branch("ccpair",   &nt_ccpair,   "ccpair/F");
    tree->Branch("ebpair",   &nt_ebpair,   "ebpair/F");
    tree->Branch("efpair",   &nt_efpair,   "efpair/F");
    tree->Branch("thpair",   &nt_thpair,   "thpair/F");
    tree->Branch("phipair",  &nt_phipair,  "phipair/F");

    tree->Branch("mbeam",    &nt_mbeam,    "mbeam/O");

    minuit = new TMinuit(1);   // allocate once
    minuit->SetPrintLevel(-1); // suppress any output

}

//------------------
// BeginRun
//------------------
void JEventProcessor_npp_ana::BeginRun(const std::shared_ptr<const JEvent> &event)
{
  double targetZ = 0.;
  DGeometry *geom = DEvent::GetDGeometry(event);
  if(!geom) throw JException("Geometry service not available in BeginRun");

//if(geom->GetTargetZ(targetZ) != NOERROR)  throw JException("Failed to get target Z from geometry");
  geom->GetTargetZ(targetZ);

  if(fabs(targetZ-1.)>0.1) {
    jerr << "Wrong target Z = " << targetZ 
         << " Check geometry DB settings." << endl;
    throw JException("Target Z mismatch");
  }

  target_position.SetXYZ(0.,0.,targetZ);

  DEvent::GetCalib(event,"FCAL/block_to_square", block_to_square);
  if(block_to_square.size() != 2800) {
   jerr << "block_to_square.size()=" << block_to_square.size() 
        << "; expected 2800" << endl;
   throw JException("Calibration table size mismatch");
 }

  runtype = -1;
  if( run_is_empty(int(event->GetRunNumber())) ) runtype = 0;
  if(  run_is_lead(int(event->GetRunNumber())) ) runtype = 1;
  if(runtype==-1) throw JException("Unknown Run type");

}

//------------------
// Process
//------------------
void JEventProcessor_npp_ana::Process(const std::shared_ptr<const JEvent> &event)
{

#ifdef SKIM_MODE
 	const DEventWriterEVIO* locEventWriterEVIO = event->GetSingle<DEventWriterEVIO>(); 
  if(locEventWriterEVIO == NULL) {
    cerr << "locEventWriterEVIO is not available" << endl;
    exit(1);
  }
#endif

  lockService->RootFillLock(this);
  h_u->Fill(0);
  lockService->RootFillUnLock(this);

#ifdef MCMODE

	vector<const DMCThrown*> locDMCThrown;
	event->Get(locDMCThrown);

  DVector3 p1mc, p2mc, pgmc[4];
  double e1mc, e2mc;
  int ngmc = 0;

  bool p1mc_read = false, p2mc_read = false;

	if(locDMCThrown.size() == 0) {
    lockService->RootFillLock(this);
    h_u->Fill(1);
    lockService->RootFillUnLock(this);
    return;
  }

	for(unsigned int i = 0; i < locDMCThrown.size(); i++) {
		const DMCThrown *mcthrown = locDMCThrown[i];

    int pid     = mcthrown->type;
    int ppid    = mcthrown->parentid;

    if(pid==7 && ppid==0 && !p1mc_read) {
      p1mc      = mcthrown->momentum();
      p1mc_read = true;
      e1mc      = hypot(mcthrown->momentum().Mag(),mpi0);
    } else {
      if(pid==7 && ppid==0 && p1mc_read) {
        p2mc    = mcthrown->momentum();
        p2mc_read = true;
        e2mc    = hypot(mcthrown->momentum().Mag(),mpi0);
      }
    }

    if(pid==1 && (ppid==1 || ppid ==2)) {
      if(ngmc<4) pgmc[ngmc] = mcthrown->momentum();
      ++ngmc;
    }
  }

  if(!p1mc_read || !p2mc_read) {
    lockService->RootFillLock(this);
    h_u->Fill(1);
    lockService->RootFillUnLock(this);
    return;
  }

  DLorentzVector p1mc4(p1mc,e1mc), p2mc4(p2mc,e2mc);

#else

  vector<const DL1Trigger*> l1trig;
  event->Get(l1trig);

  bool fcaltrigger = false;
  if(l1trig.size()) {
    if( (l1trig[0]->trig_mask & (1 << (1-1))) ) fcaltrigger = true;
  }
  if(!fcaltrigger) {
    lockService->RootFillLock(this);
    h_u->Fill(1);
    lockService->RootFillUnLock(this);
    return;
  }

#endif

  vector<const DEventRFBunch*> rf;
  event->Get(rf);

  if(rf.size() == 0) {
    lockService->RootFillLock(this);
    h_u->Fill(2);
    lockService->RootFillUnLock(this);
    return;
  }

  const DEventRFBunch *locRFBunch = NULL;
  try {
    event->GetSingle(locRFBunch, "CalorimeterOnly");
  } catch (...) {
    lockService->RootFillLock(this);
    h_u->Fill(3);
    lockService->RootFillUnLock(this);
    return;
  }

  if(locRFBunch->dNumParticleVotes<2) {
    lockService->RootFillLock(this);
    h_u->Fill(4);
    lockService->RootFillUnLock(this);
    return;
  }

  double rftime = locRFBunch->dTime;

	vector<const DChargedTrack*> locChargedTracks;
	event->Get(locChargedTracks, "PreSelect");
  if(locChargedTracks.size()>3) {
    lockService->RootFillLock(this);
    h_u->Fill(5);
    lockService->RootFillUnLock(this);
    return;
  }

  int nchtrkintime = 0;
  for(unsigned int itrk = 0; itrk < locChargedTracks.size(); ++itrk) {
    const DChargedTrack *ch_track = locChargedTracks[itrk];
    const DChargedTrackHypothesis *hyp_best = ch_track->Get_BestFOM();
    if(hyp_best == NULL) continue;
    const DTrackTimeBased *track = hyp_best->Get_TrackTimeBased();
    if(track == NULL) continue;
    const double p = track->momentum().Mag();
    if(p<0.1 || p>12.0) continue;

    double m    = hyp_best->mass();
    double one_over_beta  = sqrt(1.+m*m/p/p);
    double t    = track->time() - one_over_beta * (track->position()-target_position).Mag() / clight;
    lockService->RootFillLock(this);
    h_trk_dt->Fill(t-rftime);
    lockService->RootFillUnLock(this);
    if(fabs(t-rftime)*ACCEL_FREQ<0.5) ++nchtrkintime;
  }

  if(nchtrkintime>0)  {
    lockService->RootFillLock(this);
    h_u->Fill(6);
    lockService->RootFillUnLock(this);
    return;
  }

  vector<const DFCALHit*> fcal_hits;
  event->Get(fcal_hits);
  if(fcal_hits.size()>150) {
    lockService->RootFillLock(this);
    h_u->Fill(7);
    lockService->RootFillUnLock(this);
    return;
  }

	vector<const DFCALShower*> fcal_showers;
	event->Get(fcal_showers);
	vector<const DBCALShower*> locBCALShowers;
	event->Get(locBCALShowers);

  if(locBCALShowers.size()+fcal_showers.size()>mshowers) {
    lockService->RootFillLock(this);
    h_u->Fill(8);
    lockService->RootFillUnLock(this);
    return;
  }

  std::vector<ShowerInfo> selectedShowers;  //  create vector for selected showers
  selectedShowers.clear();

  for(unsigned int i=0; i<fcal_showers.size(); ++i) {
  	const DFCALShower *s = fcal_showers[i];
	  const DFCALCluster* locAssociatedCluster = NULL;
	  s->GetSingle(locAssociatedCluster);
	  if(locAssociatedCluster == NULL)  continue;

    double eg  = s->getEnergy();
    if(eg<0.3)  continue;
    DVector3 posg = s->_POSITION_METHOD_();
    double tg = s->getTime() - (posg-target_position).Mag() / clight;
    if(fabs(tg-rftime)*ACCEL_FREQ>2.) continue;
    int dim = s->getNumBlocks();
    int center_id = locAssociatedCluster->getChannelEmax();

    ShowerInfo si{center_id,dim,0,SYS_FCAL,posg,eg,tg};
    selectedShowers.push_back(si);
  }

  checkOverlaps(selectedShowers,SHOWER_TO_SHOWER_DIST); //  find out number of overlaps within the cut value
  selectedShowers.erase(                                //  discard if overlapped more than once
    std::remove_if(selectedShowers.begin(), selectedShowers.end(),
                   [](const ShowerInfo& s){return (s.ovlp>1);}), selectedShowers.end());

  selectedShowers.erase(                                //  discard showers with centers in the inner or 3 outer layers
    std::remove_if(selectedShowers.begin(), selectedShowers.end(),
                   [&](const ShowerInfo& s) {
                      int id = s.center_id;
                      if(id < 0 || id >= (int)block_to_square.size()) {
                        jerr << "got bad block id " << id << endl;
                        throw JException("Bad block center id");
                      }
                      int blk = block_to_square[id];
                      return (blk == 10 || blk == 11 || blk == 1 || blk == 2 || blk == 3);
                    }), selectedShowers.end());

// discard overlapped with track to FCAL projections

	for(unsigned int itrk = 0; itrk < locChargedTracks.size(); ++itrk) {
	  const DChargedTrack *ch_track = locChargedTracks[itrk];
    const DChargedTrackHypothesis *hyp_best = ch_track->Get_BestFOM();
    if(hyp_best == NULL) continue;
    const DTrackTimeBased *track = hyp_best->Get_TrackTimeBased();
    if(track == NULL) continue;
    const double p = track->momentum().Mag();
    if(p<0.1 || p>12.0) continue;

    DVector3 xtrap;
    if(track->extrapolations.size()>0) {
      vector<DTrackFitter::Extrapolation_t>fcal_extraps=track->extrapolations.at(SYS_FCAL);
      if(fcal_extraps.size()>0) {
        xtrap = fcal_extraps[0].position;
      } else {continue;}
    } else {continue;}

    selectedShowers.erase(  //  discard overlapped with tracks here
    std::remove_if(selectedShowers.begin(), selectedShowers.end(),
                   [&](const ShowerInfo& s) {
                    return ((s.pos-xtrap).Pt()<SHOWER_TO_TRACK_DIST);
                  }), selectedShowers.end());

  }

  vector<const DTOFPoint*> locTOFHitVector;
  event->Get(locTOFHitVector);

// discard selectedShowers matched with TOFPOINT objects:

  for(unsigned int itp=0; itp<locTOFHitVector.size(); ++itp) {
    const DTOFPoint *tp = locTOFHitVector[itp];

    selectedShowers.erase(  //  discard overlapped with TOFpoints here
    std::remove_if(selectedShowers.begin(), selectedShowers.end(),
                   [&](const ShowerInfo& s) {
      const double scale  = (tp->pos.Z()-target_position.Z()) / (s.pos.Z()-target_position.Z());
      const double dist   = (tp->pos - scale * s.pos).Pt();
      const double dt     = s.t + tp->pos.Mag()/clight - tp->t;
      return (dist<SHOWER_TO_TOFPOINT_DIST && fabs(dt)<SHOWER_TO_TOFPOINT_TIME);
    }), selectedShowers.end());

  }

	vector<const DNeutralShower*> locNeutralShowers;
	event->Get(locNeutralShowers);

//  Add BCAL showers from NeutralShower bank to the selected shower vector

  double ebcalneu   = 0., efcalneu  = 0.;
  double ebcalneu1  = 0., efcalneu1 = 0.;
  double ebcalneu5  = 0., efcalneu5 = 0.;

  for(unsigned int i=0; i<locNeutralShowers.size(); ++i) {
    const DNeutralShower *ns1 = locNeutralShowers[i];

    if(ns1->dDetectorSystem == SYS_FCAL)  {
      const DFCALShower* labs = NULL;
	    ns1->GetSingle(labs);
      if(labs==NULL) continue;
      double eg  = labs->getEnergy();
      if(eg<0.3) continue;
      DVector3 posg = labs->_POSITION_METHOD_();
      double tg = labs->getTime() - (posg-target_position).Mag() / clight;
      if(fabs(tg-rftime)*ACCEL_FREQ<0.5) efcalneu1 += eg;
      if(fabs(tg-rftime)*ACCEL_FREQ<2.5) efcalneu5 += eg;
      efcalneu += eg;
      continue;
    }

    if(ns1->dDetectorSystem != SYS_BCAL) continue;
  	const DBCALShower* labs = NULL;
	  ns1->GetSingle(labs);
    if(labs==NULL) continue;
    double eg  = labs->E;
    if(eg<0.1) continue;
    DVector3 posg(labs->x,labs->y,labs->z);
    double tg  = labs->t - (posg-target_position).Mag() / clight;

    if(fabs(tg-rftime)*ACCEL_FREQ<0.5) ebcalneu1 += eg;
    if(fabs(tg-rftime)*ACCEL_FREQ<2.5) ebcalneu5 += eg;
    ebcalneu += eg;

    if(fabs(tg-rftime)*ACCEL_FREQ>2.) continue;

    ShowerInfo si{0,0,0,SYS_BCAL,posg,eg,tg};
    selectedShowers.push_back(si);
  }

  double eneu5 = ebcalneu5 + efcalneu5;

  if(selectedShowers.size()<4) {
    lockService->RootFillLock(this);
    h_u->Fill(9);
    lockService->RootFillUnLock(this);
    return;
  }

  std::vector<ShowerPair> showerPairs;
  showerPairs.clear();

  for(size_t i = 0; i < selectedShowers.size(); ++i) {
      const auto& s1 = selectedShowers[i];
      DVector3 s1mom = ShowerMomentum(s1);
      double   s1en  = s1.e;

    for(size_t j = i+1; j < selectedShowers.size(); ++j) {
        const auto& s2 = selectedShowers[j];
        DVector3 s2mom = ShowerMomentum(s2);
        double   s2en  = s2.e;

        if(s1en+s2en<0.5) continue;
        if(s1.ovlp+s2.ovlp>1) continue;
        if(fabs(s1.t-s2.t)*ACCEL_FREQ>2.)  continue;

        double m = TwoShowersInvariantMass(s1,s2);
        if(fabs(m-mpi0)>PI0_MASS_CUT) continue;

        ShowerPair p;
        p.i = i;
        p.j = j;
        p.energy    = s1en + s2en;
        p.time      = 0.5*(s1.t + s2.t);
        p.mass      = m;
        p.momentum  = s1mom + s2mom;

        const double sig1 = caleres(s1en,s1.det);
        const double sig2 = caleres(s2en,s2.det);
        double ec1, ec2;
        int ist;

        fitm01(s1en,s2en,sig1,sig2,m,ec1,ec2,ist);  //  do pi0 mass constrain

        p.energyc   = ec1 + ec2;
        p.momentumc = ec1/s1en * s1mom + ec2/s2en * s2mom;
        p.chi2      = (ec1-s1en)*(ec1-s1en)/sig1/sig1 + (ec2-s2en)*(ec2-s2en)/sig2/sig2;

        p.nbcal     = 0;
        p.ebcal     = 0.;
        p.efcal     = 0.;
        if(s1.det==SYS_FCAL) {
          p.efcal += s1en;
        } else {
          p.ebcal += s1en;
          p.nbcal += 1;
        }
        if(s2.det==SYS_FCAL) {
          p.efcal += s2en;
        } else {
          p.ebcal += s2en;
          p.nbcal += 1;
        }

        showerPairs.push_back(p);
    }
  }

  if(showerPairs.size()<2) {
    lockService->RootFillLock(this);
    h_u->Fill(10);
    lockService->RootFillUnLock(this);
    return;
  }

  std::vector<Pi0Pair> Pi0Pairs;
  Pi0Pairs.clear();

  for(size_t i = 0; i < showerPairs.size(); ++i) {
      const auto& p1 = showerPairs[i];
    for(size_t j = i+1; j < showerPairs.size(); ++j) {
        const auto& p2 = showerPairs[j];

        if(p1.i == p2.i || p1.i == p2.j || p1.j == p2.i || p1.j == p2.j) continue;

        if(p1.energy+p2.energy<3.0 || p1.energy+p2.energy>13.0 || fabs(p1.time-p2.time)*ACCEL_FREQ>2.0) continue;

        double de =  p1.energy + p2.energy - eneu5;
        if(fabs(de)>0.5) continue;

        Pi0Pair pi0p;
        pi0p.i = i;
        pi0p.j = j;
        pi0p.energy   = p1.energy    + p2.energy;
        pi0p.energyc  = p1.energyc   + p2.energyc;
        pi0p.momentum = p1.momentum  + p2.momentum;
        pi0p.momentumc= p1.momentumc + p2.momentumc;
        pi0p.chi2     = p1.chi2      + p2.chi2;
        pi0p.time     = 0.5*(p1.time + p2.time);
        pi0p.nbcal    = p1.nbcal     + p2.nbcal;
        pi0p.ebcal    = p1.ebcal     + p2.ebcal;
        pi0p.efcal    = p1.efcal     + p2.efcal;

        DLorentzVector pairp4(pi0p.momentum,pi0p.energy);
        DLorentzVector pairp4c(pi0p.momentumc,pi0p.energyc);
        pi0p.mass     = pairp4.M();
        pi0p.massc    = pairp4c.M();

        Pi0Pairs.push_back(pi0p);

    }
  }

  if(Pi0Pairs.size()==0) {
    lockService->RootFillLock(this);
    h_u->Fill(11);
    lockService->RootFillUnLock(this);
    return;
  }

// --- Group Pi0Pairs that use the same 4 ShowerInfo indices ---
  map<std::set<size_t>, std::vector<size_t>> duplicate_groups;
  duplicate_groups.clear();

  for(size_t idx = 0; idx < Pi0Pairs.size(); ++idx) {
    set<size_t> key = {
      showerPairs[Pi0Pairs[idx].i].i,
      showerPairs[Pi0Pairs[idx].i].j,
      showerPairs[Pi0Pairs[idx].j].i,
      showerPairs[Pi0Pairs[idx].j].j
    };
    duplicate_groups[key].push_back(idx);
  }

// --- Assign ranks inside each group ---
  for(auto& kv : duplicate_groups) {
    auto& indices = kv.second;
    if(indices.size() <= 1) continue; // unique, leave rank = 0

// Sort by chi2 within this duplicate group
    std::sort(indices.begin(), indices.end(),
              [&](size_t a, size_t b) {
                  return Pi0Pairs[a].chi2 < Pi0Pairs[b].chi2;
              });

    int rank = 1;
    for(size_t idx : indices) {Pi0Pairs[idx].duplication_rank = rank++;}

  }

  std::sort(Pi0Pairs.begin(), Pi0Pairs.end(),
          [](const Pi0Pair& a, const Pi0Pair& b) {return a.chi2 < b.chi2;});  //  Sort by increasing chi2

/*
  if(dbgflag) {
// --- Debug printout of Pi0Pairs ---
  jout << "Found " << Pi0Pairs.size() << " Pi0Pairs:" << endl;
  for (size_t k = 0; k < Pi0Pairs.size(); ++k) {
    const auto& p = Pi0Pairs[k];
    jout << endl << endl << "Pi0Pair " << k
         << " chi2=" << p.chi2
         << " E=" << p.energy
         << " Ec=" << p.energyc
         << " time=" << p.time-rftime
         << " EBcal=" << p.ebcal
         << " EFcal=" << p.efcal
         << " rank=" << p.duplication_rank
         << " showers=("
         << showerPairs[p.i].i << "," << showerPairs[p.i].j
         << " + " 
         << showerPairs[p.j].i << "," << showerPairs[p.j].j
         << ")"
         << endl << endl;
  }}
*/

	vector<const DBeamPhoton*> beam_photons;
	event->Get(beam_photons);

  if(beam_photons.size()<1 || beam_photons.size()>mbeam)  {
    lockService->RootFillLock(this);
    h_u->Fill(12);
    lockService->RootFillUnLock(this);
    return;
  }

  vector<BeamPi0Pair> CoupledPi0s;
  CoupledPi0s.clear();

  for(vector< const DBeamPhoton* >::const_iterator bph  = beam_photons.begin();
      bph != beam_photons.end(); ++bph) {   // beam candidates

    const DLorentzVector beam = (*bph)->lorentzMomentum();
    const double eb = beam.E();
    const double tb = (*bph)->time();

    const int bunch = int(fabs(tb-rftime)*ACCEL_FREQ)+1;
    if(bunch>7) continue;

    for(size_t ip = 0; ip < Pi0Pairs.size(); ++ip) {
      const auto& pair = Pi0Pairs[ip];
      double de = pair.energyc - eb;
      if(fabs(de)>2.0) continue;

      BeamPi0Pair match;
      match.pi0_index   = ip;
      match.chi2        = pair.chi2;
      match.de          = de;

      match.ebeam       = eb;
      match.tbeam       = tb;
      match.cbeam       = (*bph)->dCounter;
      match.mbeam       = ((*bph)->dSystem == SYS_TAGM);

      CoupledPi0s.push_back(match);

      if( fabs(pair.energy-eneu5)<0.5  && locBCALShowers.size()+fcal_showers.size()<=5 && locChargedTracks.size() <= 1 ) {

        lockService->RootFillLock(this);

        nt_run      = event->GetRunNumber();
        nt_event    = event->GetEventNumber();
        nt_tgt      = runtype;

        nt_rftime   = rftime;
        nt_rftime2  = rf[0]->dTime;
        nt_nchtrk   = locChargedTracks.size();
        nt_nfcal    = fcal_showers.size();
        nt_nbcal    = locBCALShowers.size();
        nt_nneu     = locNeutralShowers.size();
        nt_eneu     = ebcalneu+efcalneu;
        nt_eneu5    = ebcalneu5+efcalneu5;

        nt_tbeam    = tb;
        nt_ebeam    = eb;
        nt_cbeam    = (*bph)->dCounter;
        nt_mbeam    = match.mbeam;

        nt_eg1      = selectedShowers[showerPairs[pair.i].i].e;
        nt_eg2      = selectedShowers[showerPairs[pair.i].j].e;
        nt_eg3      = selectedShowers[showerPairs[pair.j].i].e;
        nt_eg4      = selectedShowers[showerPairs[pair.j].j].e;

        nt_det1     = selectedShowers[showerPairs[pair.i].i].det == SYS_FCAL ? 1 : 0;
        nt_det2     = selectedShowers[showerPairs[pair.i].j].det == SYS_FCAL ? 1 : 0;
        nt_det3     = selectedShowers[showerPairs[pair.j].i].det == SYS_FCAL ? 1 : 0;
        nt_det4     = selectedShowers[showerPairs[pair.j].j].det == SYS_FCAL ? 1 : 0;

        nt_ovlp1    = selectedShowers[showerPairs[pair.i].i].ovlp;
        nt_ovlp2    = selectedShowers[showerPairs[pair.i].j].ovlp;
        nt_ovlp3    = selectedShowers[showerPairs[pair.j].i].ovlp;
        nt_ovlp4    = selectedShowers[showerPairs[pair.j].j].ovlp;

        nt_epi01    = showerPairs[pair.i].energyc;
        nt_epi02    = showerPairs[pair.j].energyc;
        nt_dmpi01   = showerPairs[pair.i].mass - mpi0;
        nt_dmpi02   = showerPairs[pair.j].mass - mpi0;
        nt_tpi01    = showerPairs[pair.i].time;
        nt_tpi02    = showerPairs[pair.j].time;

        nt_epair    = pair.energy;
        nt_ecpair   = pair.energyc;
        nt_mpair    = pair.mass;
        nt_mcpair   = pair.massc;
        nt_tcpair   = pair.time;
        nt_ccpair   = pair.chi2;
        nt_nbpair   = pair.nbcal;
        nt_ebpair   = pair.ebcal;
        nt_efpair   = pair.efcal;
        nt_rankpair = pair.duplication_rank;
        nt_thpair   = pair.momentum.Theta()*TMath::RadToDeg();
        nt_phipair  = pair.momentum.Phi()  *TMath::RadToDeg();

        tree->Fill();
        lockService->RootFillUnLock(this);

      }


      if(pair.massc<0.6 && pair.momentum.Theta()*TMath::RadToDeg()<0.5 && match.mbeam &&
         fabs(showerPairs[pair.i].mass-mpi0)<0.0175 &&  fabs(showerPairs[pair.j].mass-mpi0)<0.0175 && fabs(pair.energy-eneu5)<0.1 &&
         pair.nbcal<=2 && locBCALShowers.size()+fcal_showers.size()<=5 && locChargedTracks.size() <= 1 && pair.duplication_rank<=1 &&
         fabs(rf[0]->dTime-rftime)<1.) {

        lockService->RootFillLock(this);
        if(bunch==1) h_de[runtype]->Fill(de);
        else  h_de_acc[runtype]->Fill(de);
        lockService->RootFillUnLock(this);
      }

    }
  }

  if(CoupledPi0s.size()==0) {
    lockService->RootFillLock(this);
    h_u->Fill(13);
    lockService->RootFillUnLock(this);
    return;
  }

  lockService->RootFillLock(this);
  h_u->Fill(19);
  lockService->RootFillUnLock(this);

#ifdef SKIM_MODE
  locEventWriterEVIO->Write_EVIOEvent(event,"2pi0");
#endif

}

//------------------
// EndRun
//------------------
void JEventProcessor_npp_ana::EndRun()
{
    // This is called whenever the run number changes, before it is
    // changed to give you a chance to clean up before processing
    // events from the next run number.
}

//------------------
// Finish
//------------------
void JEventProcessor_npp_ana::Finish()
{
    // Called before program exit after event processing is finished.
  delete minuit; // cleanup
}

