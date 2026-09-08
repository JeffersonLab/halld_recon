// $Id$
 //
//    File: JEventProcessor_thetaCrossTree.cc
// Created: Fri Aug  8 03:02:21 PM EDT 2025
// Creator: shannen (on Linux ifarm2401.jlab.org 5.14.0-503.19.1.el9_5.x86_64 x86_64)
//

/// For more information on the syntax changes between JANA1 and JANA2, visit: https://jeffersonlab.github.io/JANA2/#/jana1to2/jana1-to-jana2

#include "JEventProcessor_thetaCrossTree2.h"


// Routine used to create our JEventProcessor
#include <JANA/JApplication.h>
extern "C"{
void InitPlugin(JApplication *app){
    InitJANAPlugin(app);
    app->Add(new JEventProcessor_thetaCrossTree2());
}
} // "C"


//------------------
// JEventProcessor_thetaCrossTree (Constructor)
//------------------
JEventProcessor_thetaCrossTree2::JEventProcessor_thetaCrossTree2()
{
	SetTypeName(NAME_OF_THIS); // Provide JANA with this class's name

    // Parameters and Services should be accessed from Init() instead of here!
}

//------------------
// ~JEventProcessor_thetaCrossTree (Destructor)
//------------------
JEventProcessor_thetaCrossTree2::~JEventProcessor_thetaCrossTree2()
{
}

//------------------
// Init
//------------------
void JEventProcessor_thetaCrossTree2::Init()
{
    // This is called once at program startup. 
    
    auto app = GetApplication();
    // lockService should be initialized here like this
    lockService = app->GetService<JLockService>();

    h = new TH1I("h", "cuts", 10, -1, 9);
    WH = new TH1D("W", "W tot", 100, .1, .2);
    Bunch_time_hist = new TH1D("BunchTime", "Bunch Time", 500, -30, 30);
    FCal_centroid = new TH2D("FCal_centroid", "FCal centroid", 250, -75, 75, 250, -75, 75);
    ThetaD = new TH1D("theta", "Theta", 5000, 0, 100);
    ED = new TH1D("E", "E", 5000, 0, 5);
    EThetaD = new TH2D("ETheta", "Energy Theta", 5000, 0, 100, 5000, 0, 5);
    ThetaDFCAL = new TH1D("thetaFCAL", "Theta FCAL", 5000, 0, 100);
    EDFCAL = new TH1D("EFCAL", "E FCAL", 5000, 0, 5);
    EThetaDFCAL = new TH2D("EThetaFCAL", "Energy Theta FCAL", 5000, 0, 100, 5000, 0, 5);
    ThetaDBCAL = new TH1D("thetaBCAL", "Theta BCAL", 5000, 0, 100);
    EDBCAL = new TH1D("EBCAL", "E BCAL", 5000, 0, 5);
    EThetaDBCAL = new TH2D("EThetaBCAL", "Energy Theta BCAL", 5000, 0, 100, 5000, 0, 5);
    
    ofile = new TFile("pi0crossTree.root","RECREATE");
    tree1 = new TTree( "pi", "pi" );

    //tree1->Branch("allInfo", &allInfo);
    tree1->Branch("runNum", &runNum_fill);
    tree1->Branch("evtNum", &evtNum_fill);
    tree1->Branch("E1", &E1_fill);
    tree1->Branch("E2", &E2_fill);
    tree1->Branch("E1s", &E1s_fill);
    tree1->Branch("E2s", &E2s_fill);
    tree1->Branch("ri", &ri_fill);
    tree1->Branch("rj", &rj_fill);
    tree1->Branch("beamE", &beamE_fill);
    tree1->Branch("w", &w_fill);
    tree1->Branch("E1t", &E1t);
    tree1->Branch("E2t", &E2t);

    /*
    tree1->Branch("runNum_BCAL", &runNum_fill_BCAL);
    tree1->Branch("evtNum_BCAL", &evtNum_fill_BCAL);
    tree1->Branch("E1_BCAL", &E1_fill_BCAL);
    tree1->Branch("E2_BCAL_BCAL", &E2_fill_BCAL);
    tree1->Branch("E1s_BCAL", &E1s_fill_BCAL);
    tree1->Branch("E2s_BCAL", &E2s_fill_BCAL);
    tree1->Branch("ri_BCAL", &ri_fill_BCAL);
    tree1->Branch("rj_BCAL", &rj_fill_BCAL);
    tree1->Branch("beamE_BCAL", &beamE_fill_BCAL);
    tree1->Branch("w_BCAL", &w_fill_BCAL);
    tree1->Branch("E1t_BCAL", &E1t_BCAL);
    tree1->Branch("E2t_BCAL", &E2t_BCAL);
    

    tree1->Branch("runNum_TOF", &runNum_fill_TOF);
    tree1->Branch("evtNum_TOF", &evtNum_fill_TOF);
    tree1->Branch("E1_TOF", &E1_fill_TOF);
    tree1->Branch("E2_TOF", &E2_fill_TOF);
    tree1->Branch("E1s_TOF", &E1s_fill_TOF);
    tree1->Branch("E2s_TOF", &E2s_fill_TOF);
    tree1->Branch("ri_TOF", &ri_fill_TOF);
    tree1->Branch("rj_TOF", &rj_fill_TOF);
    tree1->Branch("beamE_TOF", &beamE_fill_TOF);
    tree1->Branch("w_TOF", &w_fill_TOF);
    tree1->Branch("E1t_TOF", &E1t_TOF);
    tree1->Branch("E2t_TOF", &E2t_TOF);

    */
}

//------------------
// BeginRun
//------------------
void JEventProcessor_thetaCrossTree2::BeginRun(const std::shared_ptr<const JEvent> &event)
{
    // This is called whenever the run number changes
}

//------------------
// Process
//------------------
void JEventProcessor_thetaCrossTree2::Process(const std::shared_ptr<const JEvent> &event)
{
    // This is called for every event. Use of common resources like writing
    // to a file or filling a histogram should be mutex protected. Using
    // event->Get(...) to get reconstructed objects (and thereby activating the
    // reconstruction algorithm) should be done outside of any mutex lock
    // since multiple threads may call this method at the same time.
    // Here's an example:
    //
    // vector<const MyDataClass*> mydataclasses;
    // event->Get(mydataclasses);
    //
    // If you have lockService initialized in Init() then you can acquire locks like this
    // lockService->RootFillLock(this);
    //  ... fill historgrams or trees ...
    // lockService->RootFillUnLock(this);

  //std::cout << "test/n";

  lockService->RootFillLock(this);
  h->Fill(0);
  lockService->RootFillUnLock(this);
  
  vector<const DChargedTrack*> locChargedTracks;
  event->Get(locChargedTracks, "PreSelect");
  if(locChargedTracks.size() != 0) return;

  lockService->RootFillLock(this);
  h->Fill(1);
  lockService->RootFillUnLock(this);

  //vector<const DL1Trigger*> trig;
  //event->Get(trig);

  vector<const DTrigger*> trig;
  event->Get(trig);

  if(trig.empty()){
    //cout << "Trig 1 empty " << endl;
    return;
  }
  //if(trig2.empty()){
  //  cout << "Trig 2 empty " << endl;
  //  return;
    //}

  //cout << "trig bit test: " <<  trig[0]->trig_mask << endl;
  //cout << "trig bit test2: " << trig2[0]->Get_L1TriggerBits() << endl;
  //cout << endl;
  //return;
  //if(trig.empty() || trig2.empty()) return;
  
  
  //std::cout << "Trig: " << trig[0]->trig_mask << std::endl;
  //std::cout << "boolTest: " << !(trig[0]->trig_mask & 1) << std::endl;
  //std::cout << "boolTest2: " << !(trig[0]->trig_mask & 4) << std::endl;
  //std::cout << "boolTest3: " << (trig[0]->trig_mask & 5) << std::endl;
  //std::cout << "Bool test: " << (((trig[0]->trig_mask & 0x1) && (trig[0]->trig_mask & 0x3)) && (trig[0]->trig_mask ^ 0x5)) << std::endl;
  
  
  //if(!(trig[0]->trig_mask & 1) && !(trig[0]->trig_mask & 4)) return;

  lockService->RootFillLock(this);
  h->Fill(2);
  lockService->RootFillUnLock(this);
  //cout << "Wasn't rejected!\n";
  //trig_mask instead of Getter function for DL1Trigger
  //if(((trig[0]->trig_mask & 0x1) && (trig[0]->trig_mask & 0x3)) && (trig[0]->trig_mask ^ 0x5)) return;
  if(trig[0]->Get_L1TriggerBits() != 1 && trig[0]->Get_L1TriggerBits() != 5) return;
  //cout << "Got past trig!" 
  //if(((trig[0]->Get_L1TriggerBits()  & 0x1) && (trig[0]->Get_L1TriggerBits()  & 0x3)) && (trig[0]->Get_L1TriggerBits()  ^ 0x5)) return;
  //std::cout << "Above declaration\n";                                                                                                                                                                                                                                      
  vector<const DBeamPhoton*> locBeamPhoton;
  event->Get(locBeamPhoton);
  if(locBeamPhoton.size() == 0) return;

  lockService->RootFillLock(this);
  h->Fill(3);
  lockService->RootFillUnLock(this);
  //std::cout << "Below declaration\n";                                                                                                                                        

  vector<const DEventRFBunch*> rf;
  event->Get(rf);
  if(rf.size() == 0) return;

  double locRFTime = rf[0]->dTime;
  if(isnan(locRFTime)) return;

  lockService->RootFillLock(this);
  h->Fill(4);
  lockService->RootFillUnLock(this);
  //lockService->RootFillLock(this);
  //RFTime_hist->Fill(locRFTime);
  //for(unsigned int i = 0; i < locBeamPhoton.size(); i++){                                                                                           
    //std::cout << "got here!\n";                                                                                                                                 
    //Bunch_time_hist->Fill(locBeamPhoton[i]->time() - locRFTime);                                                                                                                             
    //std::cout << "bunch time: " << locBeamPhoton[i]->time() - locRFTime << std::endl;                                                                         
  //}                                                                                                                     
  //cut_hist->Fill(0);
  //lockService->RootFillUnLock(this);

  vector<const DNeutralShower*> locNeutralShowers;
  event->Get(locNeutralShowers);
  //if(locNeutralShowers.size()<2||locNeutralShowers.size()>5) return;                                                        
  if(locNeutralShowers.size()!=2) return;

  lockService->RootFillLock(this);
  h->Fill(5);
  lockService->RootFillUnLock(this);

  bool TOF_veto = false;//, TOF_veto_compl = false;//, BCAL_veto = false;

  vector<const DTOFPoint*> locTOFHitVector;
  event->Get(locTOFHitVector);
  //if(locTOFHitVector.size()!=0) TOF_veto = true;

  //lockService->RootFillLock(this);
  //h->Fill(2);
  //lockService->RootFillUnLock(this);

  /*
  vector<const DMCThrown*> locThrown;
  event->Get(locThrown);
  vector<const DMCTrackHit*> locPoint;
  event->Get(locPoint);
  std::cout << "THrown size: " << locThrown.size() << std::endl;
  std::cout << "Point size: " << locPoint.size() << std::endl;
  for(int i = 0; i < locThrown.size(); i++){
    const DMCThrown *thrown = locThrown[i];

    int ptype = thrown->type;
    int pmech = thrown->mech;
    int id = thrown->myid;
    //float dist = track->z;
    std::cout << "type: " << ptype << std::endl;
    std::cout << "id: " << id << std::endl;
    std::cout << "mech: " << pmech << std::endl;
  }
  for(int i = 0; i < locPoint.size(); i++){
    const DMCTrackHit *track = locPoint[i];

    int type = track->ptype;
    std::cout << "track type: " << type << std::endl;
  }
  */

  DVector3 vtx(0,0,1);

  vector<int> block_to_square;
  DEvent::GetCalib(event, "FCAL/block_to_square", block_to_square);

  //std::cout << "Block to square size: " << block_to_square.size() << std::endl;

  vector<const DFCALShower*> locFCALShowers;
  event->Get(locFCALShowers);

  bool inner_FCal = false, outer_FCal = false;
  
  //std::cout << "Shower size: " << locFCALShowers.size() << std::endl;
  
  for(int s = 0; s < locFCALShowers.size(); s++){
    const DFCALShower *s1 = locFCALShowers[s];

    int center_id;

    
    
    const DFCALCluster* locAssociatedCluster = NULL;
    s1->GetSingle(locAssociatedCluster);
    if(locAssociatedCluster == NULL) center_id = -1;
    else center_id = locAssociatedCluster->getChannelEmax();

    //std::cout << "center_id: " << center_id << std::endl;
    //std::cout << "block to square: " << block_to_square[center_id] << std::endl;
    
    if(block_to_square[center_id]==10 || block_to_square[center_id]==11) inner_FCal = true;  //  skip inner square                                                                    
    if(block_to_square[center_id]==1  || block_to_square[center_id]==2 ||
       block_to_square[center_id]==3) outer_FCal = true;                               //  skip outer part 
    
  }

  cout << "got here!" << endl;
  
  for(int i = 0; i < locNeutralShowers.size(); i ++){
    for (int j = 0; j < locNeutralShowers.size(); j++){
      if(i == j) {
        continue;
      }
      if(i < j) continue;
      
      if(inner_FCal || outer_FCal){
	//continue;
	//std::cout << "Skipped for inner\n";
	  continue;
      }
      //else {std::cout << "Didnlt' skip for inner\n";}
      lockService->RootFillLock(this);
      h->Fill(5);
      lockService->RootFillUnLock(this);
      
      const DNeutralShower *nsi = locNeutralShowers[i];
      const DNeutralShower *nsj = locNeutralShowers[j];

      const bool isFcali = (nsi->dDetectorSystem==SYS_FCAL);
      const double Egi = nsi->dEnergy;
      const double xi = nsi->dSpacetimeVertex.Vect().X();      
      const double yi = nsi->dSpacetimeVertex.Vect().Y();                     
      const DVector3 ri =  nsi->dSpacetimeVertex.Vect() - vtx;

      vector<float> ri_vec;
      //ri_vec.push_back(ri.T());
      ri_vec.push_back(ri.X());
      ri_vec.push_back(ri.Y());
      ri_vec.push_back(ri.Z());
      
      const bool isFcalj = (nsj->dDetectorSystem==SYS_FCAL);
      const double Egj = nsj->dEnergy;
      const double xj = nsj->dSpacetimeVertex.Vect().X();                                           
      const double yj = nsj->dSpacetimeVertex.Vect().Y();
      const DVector3 rj =  nsj->dSpacetimeVertex.Vect() - vtx;

      //std::cout << "isFCali: " << isFcali << std::endl;
      //std::cout << "isFcalj: " << isFcalj << std::endl;
      
      vector<float> rj_vec;
      //rj_vec.push_back(rj.T()); 
      rj_vec.push_back(rj.X());
      rj_vec.push_back(rj.Y()); 
      rj_vec.push_back(rj.Z()); 

      const DVector3 gi_p3 = Egi/ri.Mag()*ri;
      const DLorentzVector gi_p4(gi_p3, Egi);
      const DVector3 gj_p3 = Egj/rj.Mag()*rj;
      const DLorentzVector gj_p4(gj_p3, Egj);

      double theta = (gi_p3 + gj_p3).Theta();

      double W = (gi_p4 + gj_p4).M();
      cout << "W: " << (gi_p4 + gj_p4).M() << endl << endl;
      /*******
      for(unsigned int t = 0; t < locTOFHitVector.size(); t++){
	//std::cout << "Got to TOF loop\n";

	const DTOFPoint *tofPoint = locTOFHitVector[t];

	double DeltaXi = tofPoint->Is_XPositionWellDefined() ? tofPoint->pos.X() - xi : -1;
	double DeltaYi = tofPoint->Is_YPositionWellDefined() ? tofPoint->pos.Y() - yi : -1;
	double DeltaXj = tofPoint->Is_XPositionWellDefined() ? tofPoint->pos.X() - xj : -1;
	double DeltaYj = tofPoint->Is_YPositionWellDefined() ? tofPoint->pos.Y() - yj : -1;
	double TOF_CUT_PAR1 = 1.1, TOF_CUT_PAR2 = 1.5, TOF_CUT_PAR3 = 6.15;
	double locMatchCut_2Di = exp(-1*TOF_CUT_PAR1*gi_p3.Mag() + TOF_CUT_PAR2) + TOF_CUT_PAR3;
	double locMatchCut_2Dj = exp(-1*TOF_CUT_PAR1*gj_p3.Mag() + TOF_CUT_PAR2) + TOF_CUT_PAR3;
	double locMatchCut_1Di = locMatchCut_2Di;
	double locMatchCut_1Dj = locMatchCut_2Dj;
	
	if(!tofPoint->Is_XPositionWellDefined()){
	  //std::cout << "Delta Yi: " << fabs(DeltaYi) << std::endl;
	  //std::cout << "Delta Yj: " << fabs(DeltaYj) << std::endl;
	  //std::cout << "locMatchCut_1Di: " << locMatchCut_1Di << std::endl;
	  //std::cout << "locMatchCut_1Dj: " << locMatchCut_1Dj << std::endl;
	  if((fabs(DeltaYi) > locMatchCut_1Di) && (fabs(DeltaYj) > locMatchCut_1Dj)) continue;
	  //if(fabs(DeltaYj) > locMatchCut_1Dj) continue;
	}
	else if(!tofPoint->Is_YPositionWellDefined()){
	  //std::cout << "Delta Xi: " << fabs(DeltaXi) << std::endl;
          //std::cout << "Delta Xj: " << fabs(DeltaXj) << std::endl;
	  //std::cout << "locMatchCut_1Di: " << locMatchCut_1Di << std::endl;
          //std::cout << "locMatchCut_1Dj: " << locMatchCut_1Dj << std::endl;
	  if((fabs(DeltaXi) > locMatchCut_1Di) && (fabs(DeltaXj) > locMatchCut_1Dj)) continue;
	  //if(fabs(DeltaXj) > locMatchCut_1Dj) continue;
	}
	else{
	  double locDistancei = sqrt(DeltaXi*DeltaXi + DeltaYi*DeltaYi);
	  double locDistancej = sqrt(DeltaXj*DeltaXj + DeltaYj*DeltaYj);
	  //std::cout << "Distancei: " << locDistancei << std::endl;
	  //std::cout << "Distancej: " << locDistancej << std::endl;
	  //std::cout << "locMatchCut_1Di: " << locMatchCut_1Di << std::endl;
          //std::cout << "locMatchCut_1Dj: " << locMatchCut_1Dj << std::endl;
	  if((locDistancei > locMatchCut_2Di) && (locDistancej > locMatchCut_2Dj)) continue;
	  //if(locDistance > locMatchCut_2D) continue;
	}

	//std::cout << "TOF cut test!\n";
	TOF_veto = true;
      }
      *////////////
      //if(TOF_veto) continue;
      lockService->RootFillLock(this);
      h->Fill(6);
      lockService->RootFillUnLock(this);
      double Eij = Egi + Egj;
      
      double sc1, sc2;

      lockService->RootFillLock(this);
      get_pi0_momentum_and_mass(gi_p4, isFcali, gj_p4, isFcalj, sc1, sc2);
      lockService->RootFillUnLock(this);

      double Esi = sc1*Egi;
      double Esj = sc2*Egj;

      if(isFcali){
        //if(Egi < 0.4) continue;
	lockService->RootFillLock(this);
	FCal_centroid->Fill(xi, yi);
	lockService->RootFillUnLock(this);
      }
      else{
	//if(Egi < 0.2) continue;
      }
      if(isFcalj){
        //if(Egj < 0.4) continue;
	lockService->RootFillLock(this);
        FCal_centroid->Fill(xj, yj);
        lockService->RootFillUnLock(this);
      }
      else{
        //if(Egj < 0.2) continue;
      }

      lockService->RootFillLock(this);
      h->Fill(7);
      lockService->RootFillUnLock(this);
      
      double vtxTimei = nsi->dSpacetimeVertex.T() - ri.Mag()/clight;
      double vtxTimej = nsj->dSpacetimeVertex.T() - rj.Mag()/clight;
      if(abs(vtxTimei - locRFTime)>1 || abs(vtxTimej - locRFTime)>1) continue;

      lockService->RootFillLock(this);
      h->Fill(8);
      lockService->RootFillLock(this);
      
      //if(Eij < 3) continue;

      lockService->RootFillLock(this);
      h->Fill(9);
      lockService->RootFillUnLock(this);
      
      //vector<float> Nums;
      double runNum = event->GetRunNumber();
      //Nums.push_back(runNument);
      //vector<double> eventNum;
      double eventNum = event->GetEventNumber();
      //Nums.push_back(eventNument);

      //vector<float> Energies;
      //Energies.push_back(Egi);
      //Energies.push_back(Egj);
      //Energies.push_back(Esi);
      //Energies.push_back(Esj);

      vector<float> beamE;
      vector<float> w_vec;
      vector<float> scaledE1;
      vector<float> scaledE2;
      for(unsigned int b = 0; b < locBeamPhoton.size(); b++){
        //std::cout << "photon energy test: " << locBeamPhoton[b]->momentum().Mag() << std::endl;;                                                            
	float tagE = locBeamPhoton[b]->momentum().Mag();
	beamE.push_back(tagE);

	double st1, st2;
	get_pi0_energyScaled(Egi, Egj, tagE, st1, st2, isFcali, isFcalj);
	float Et1 = st1*Egi;
	float Et2 = st2*Egj;

	scaledE1.push_back(Et1);
	scaledE2.push_back(Et2);
	
	double dt = locBeamPhoton[b]->time() - locRFTime;
        int N = 4;
        double bunchTime = 4.008;
        double w = 0;
        if(abs(dt) < 0.8){
          w = 1;
        }
        for (int n=2; n<2+N; n++){
          if (abs(dt-n*bunchTime)<0.8){
              w = -1./(2.*N);  // side peaks number 2 to 5                                                                                                                                                                                                                    
            }
          if (abs(dt+n*bunchTime)<0.8){
              w = -1./(2.*N);  // side peaks number -2 to -5                                                                                                                                                                                                                  
            }
        }

	Bunch_time_hist->Fill(bunchTime);
	w_vec.push_back(w);

      }
      //std::cout << "Filling vector\n";
      lockService->RootFillLock(this);
      ThetaD->Fill(gi_p3.Theta()*180/TMath::Pi());
      ED->Fill(Egi);
      ThetaD->Fill(gj_p3.Theta()*180/TMath::Pi());
      ED->Fill(Egj);
      EThetaD->Fill(gi_p3.Theta()*180/TMath::Pi(), Egi);
      EThetaD->Fill(gj_p3.Theta()*180/TMath::Pi(), Egj);
      if(isFcali){
	ThetaDFCAL->Fill(gi_p3.Theta()*180/TMath::Pi());
	EDFCAL->Fill(Egi);
	//ThetaDFCAL->Fill(gj_p3.Theta()*180/TMath::Pi());
	//EDFCAL->Fill(Egj);
	EThetaDFCAL->Fill(gi_p3.Theta()*180/TMath::Pi(), Egi);
	//EThetaDFCAL->Fill(gj_p3.Theta()*180/TMath::Pi(), Egj);
      }
      else{
	ThetaDBCAL->Fill(gi_p3.Theta()*180/TMath::Pi());
	EDBCAL->Fill(Egi);
	//ThetaDBCAL->Fill(gj_p3.Theta()*180/TMath::Pi());
	//EDBCAL->Fill(Egj);
	EThetaDBCAL->Fill(gi_p3.Theta()*180/TMath::Pi(), Egi);
	//EThetaDBCAL->Fill(gj_p3.Theta()*180/TMath::Pi(), Egj);
      }

      if(isFcalj){
        ThetaDFCAL->Fill(gj_p3.Theta()*180/TMath::Pi());
        EDFCAL->Fill(Egj);
        //ThetaDFCAL->Fill(gj_p3.Theta()*180/TMath::Pi())
        //EDFCAL->Fill(Egj);        
        EThetaDFCAL->Fill(gj_p3.Theta()*180/TMath::Pi(), Egj);
        //EThetaDFCAL->Fill(gj_p3.Theta()*180/TMath::Pi(), Egj);         
      }
      else{
        ThetaDBCAL->Fill(gj_p3.Theta()*180/TMath::Pi());
        EDBCAL->Fill(Egj);
        //ThetaDBCAL->Fill(gj_p3.Theta()*180/TMath::Pi());   
        //EDBCAL->Fill(Egj); 
        EThetaDBCAL->Fill(gj_p3.Theta()*180/TMath::Pi(), Egj);
        //EThetaDBCAL->Fill(gj_p3.Theta()*180/TMath::Pi(), Egj);   
      }
      
      WH->Fill(W);
      //if(!TOF_veto){
	//cout << "finishted!" << endl;
	runNum_fill = runNum;
	evtNum_fill = eventNum;
	E1_fill = Egi;
	E2_fill = Egj;
	E1s_fill = Esi;
	E2s_fill = Esj;
	ri_fill = ri_vec;
	rj_fill = rj_vec;
	beamE_fill = beamE;
	w_fill = w_vec;
	E1t = scaledE1;
	E2t = scaledE2;
	//}

      /*
      if(!BCAL_veto){
        runNum_fill_BCAL = runNum;
        evtNum_fill_BCAL = eventNum;
        E1_fill_BCAL = Egi;
        E2_fill_BCAL = Egj;
        E1s_fill_BCAL = Esi;
        E2s_fill_BCAL = Esj;
        ri_fill_BCAL = ri_vec;
        rj_fill_BCAL = rj_vec;
        beamE_fill_BCAL = beamE;
        w_fill_BCAL = w_vec;
        E1t_BCAL = scaledE1;
        E2t_BCAL = scaledE2;
      }

      
      if(!TOF_veto_compl){
	runNum_fill_TOF = runNum;
        evtNum_fill_TOF = eventNum;
        E1_fill_TOF = Egi;
        E2_fill_TOF = Egj;
        E1s_fill_TOF = Esi;
        E2s_fill_TOF = Esj;
        ri_fill_TOF = ri_vec;
        rj_fill_TOF = rj_vec;
        beamE_fill_TOF = beamE;
        w_fill_TOF = w_vec; 
        E1t_TOF = scaledE1; 
        E2t_TOF = scaledE2;
      }
      //allInfo.push_back(beamE);
      //allInfo.push_back(w_vec);
      //allInfo.push_back(scaledE1);
      //allInfo.push_back(scaledE2);
tree1->Fill();      */
      
      tree1->Fill();
      lockService->RootFillUnLock(this);
      
    }
  }
  
}

//------------------
// EndRun
//------------------
void JEventProcessor_thetaCrossTree2::EndRun()
{
    // This is called whenever the run number changes, before it is
    // changed to give you a chance to clean up before processing
    // events from the next run number.
}

//------------------
// Finish
//------------------
void JEventProcessor_thetaCrossTree2::Finish()
{
    // Called before program exit after event processing is finished.
  lockService->RootWriteLock();
  ofile->Write();
  ofile->Close();
  lockService->RootUnLock();
}
