// $Id$
//
//    File: JEventProcessor_CalCal2.cc
// Created: Tue Feb 10 11:29:02 AM EST 2026
// Creator: staylor (on Linux ifarm2402.jlab.org 5.14.0-611.20.1.el9_7.x86_64 x86_64)
//

/// For more information on the syntax changes between JANA1 and JANA2, visit: https://jeffersonlab.github.io/JANA2/#/jana1to2/jana1-to-jana2

#include "JEventProcessor_CalCal2.h"
#include <TDirectory.h>

// Routine used to create our JEventProcessor
#include <JANA/JApplication.h>
extern "C"{
void InitPlugin(JApplication *app){
  InitJANAPlugin(app);
  app->Add(new JEventProcessor_CalCal2());
}
} // "C"


//------------------
// JEventProcessor_CalCal2 (Constructor)
//------------------
JEventProcessor_CalCal2::JEventProcessor_CalCal2()
{
  // Parameters and Services should be accessed from Init() instead of here!
}

//------------------
// ~JEventProcessor_CalCal2 (Destructor)
//------------------
JEventProcessor_CalCal2::~JEventProcessor_CalCal2()
{
  SetTypeName(NAME_OF_THIS); // Provide JANA with this class's name
}

//------------------
// Init
//------------------
void JEventProcessor_CalCal2::Init()
{
  auto app = GetApplication();
  lockService = app->GetService<JLockService>();
  lockService->RootWriteLock();

  gDirectory->mkdir("CalCal")->cd();
  gDirectory->mkdir("ECAL")->cd();
  {
    hProtonECALGammaTimeDiff=new TH1F("hProtonECALGammaTimeDiff",";t_{0}-t(#gamma,ECAL) [ns]",200,-10,10);
    h2GammaMassECAL=new TH1F("h2GammaMassECAL",";m(2#gamma, ECAL only) [GeV]",1000,0,1.);
    hEcalDt=new TH2F("hEcalDt",";channel;t(RF)-t(ECAL) [ns]",1600,-0.5,1599.5,100,-2,2);
    hEcalGains=new TH1F("hEcalGains",";ECAL gain distribution",100,0.8,1.2);
    hEcalGains2D=new TH2F("hEcalGains2D","ECAL gains;column;row",42,-1.5,40.5,
			  42,-1.5,40.5);
  }
  gDirectory->cd("../");
  gDirectory->mkdir("FCAL")->cd();
  {
    hProtonFCALGammaTimeDiff=new TH1F("hProtonFCALGammaTimeDiff",";t_{0}-t(#gamma,FCAL) [ns]",200,-10,10);
    h2GammaMassFCAL=new TH1F("h2GammaMassFCAL",";m(2#gamma, FCAL only) [GeV]",1000,0,1.);
    hFcalDt=new TH2F("hFcalDt",";channel;t(RF)-t(FCAL) [ns]",2800,-0.5,1599.5,100,-2,2);
    hFcalGains=new TH1F("hFcalGains",";FCAL gain distribution",100,0.8,1.2);
    hFcalGains2D=new TH2F("hFcalGains2D","FCAL gains;column;row",61,-1.5,59.5,
			  61,-1.5,59.5);
  }
  gDirectory->cd("../");
  gDirectory->mkdir("ECALFCAL")->cd();
  {
    h2GammaMassECALFCAL=new TH1F("h2GammaMassECALFCAL",";m(2#gamma, ECAL+FCAL) [GeV]",1000,0,1.);
  }
  gDirectory->cd("../..");

  lockService->RootUnLock();
 
  TRACK_CL_CUT=0.001;
  app->SetDefaultParameter("CalCal:TRACK_CL_CUT",TRACK_CL_CUT);
  PI0_LO_CUT=0.1; // GeV
  app->SetDefaultParameter("CalCal:PI0_LO_CUT",PI0_LO_CUT);
  PI0_HI_CUT=0.16; // GeV
  app->SetDefaultParameter("CalCal:PI0_HI_CUT",PI0_HI_CUT);
  ETA_LO_CUT=0.5; // GeV
  app->SetDefaultParameter("CalCal:ETA_LO_CUT",ETA_LO_CUT);
  ETA_HI_CUT=0.6; // GeV
  app->SetDefaultParameter("CalCal:ETA_HI_CUT",ETA_HI_CUT);
  ESHOWER_MIN=1.; // GeV
  app->SetDefaultParameter("CalCal:ESHOWER_MIN",ESHOWER_MIN);
  EHIT_MIN=0.25; // GeV
  app->SetDefaultParameter("CalCal:EHIT_MIN",EHIT_MIN);
  TIME_CUT=2.;
  app->SetDefaultParameter("CalCal:TIME_CUT",TIME_CUT);

  // ECAL hit resolution parameters
  ECAL_VAR_E_PAR0=7.7e-5;
  ECAL_VAR_E_PAR1=-6.9e-5;
  ECAL_VAR_E_PAR2=2.3e-5;
  app->SetDefaultParameter("CalCal:ECAL_VAR_E_PAR0",ECAL_VAR_E_PAR0);
  app->SetDefaultParameter("CalCal:ECAL_VAR_E_PAR1",ECAL_VAR_E_PAR1);
  app->SetDefaultParameter("CalCal:ECAL_VAR_E_PAR2",ECAL_VAR_E_PAR2);

  // FCAL hit resolution parameters
  FCAL_VAR_E_PAR0=5e-4;
  FCAL_VAR_E_PAR1=2.5e-3;
  FCAL_VAR_E_PAR2=9e-4;
  app->SetDefaultParameter("CalCal:FCAL_VAR_E_PAR0",FCAL_VAR_E_PAR0);
  app->SetDefaultParameter("CalCal:FCAL_VAR_E_PAR1",FCAL_VAR_E_PAR1);
  app->SetDefaultParameter("CalCal:FCAL_VAR_E_PAR2",FCAL_VAR_E_PAR2);
  
}

//------------------
// BeginRun
//------------------
void JEventProcessor_CalCal2::BeginRun(const std::shared_ptr<const JEvent> &event)
{
  auto runnumber = event->GetRunNumber();
  auto app = event->GetJApplication();
  auto geo_manager = app->GetService<DGeometryManager>();
  auto dgeom = geo_manager->GetDGeometry(runnumber);
  if (dgeom->HaveInsert()){
    event->GetSingle(dECALGeom);
    ifstream infile("ecal_gains.dat");
    for (size_t i=0;i<1600;i++){
      double dummy;
      infile >> dummy;
      ecal_gains.push_back(dummy);
      ecal_counts.push_back(1.);
    }
    DEvent::GetCalib(event, "/ECAL/gains", old_ecal_gains);
    DEvent::GetCalib(event, "/ECAL/timing_offsets", ecal_times);
  }
  DEvent::GetCalib(event, "/FCAL/timing_offsets", fcal_times);
  DEvent::GetCalib(event, "/FCAL/gains", old_fcal_gains);
  ifstream infile("fcal_gains.dat");
  for (size_t i=0;i<old_fcal_gains.size();i++){
    double dummy;
    infile >> dummy;
    fcal_gains.push_back(dummy);
    fcal_counts.push_back(1.);
  }
  event->GetSingle(dFCALGeom);
}

//------------------
// Process
//------------------
void JEventProcessor_CalCal2::Process(const std::shared_ptr<const JEvent> &event)
{
  // Look for events with energy in the forward calorimeters and one track
  // Use only forward triggers
  auto triggers=event->Get<DTrigger>();
  if (triggers.size()==0) return;
  if (!(triggers[0]->Get_L1TriggerBits()&0x1)) return;

  // Look for a minimum number of showers
  auto ecalshowers=event->Get<DECALShower>();
  auto fcalshowers=event->Get<DFCALShower>();
  if (fcalshowers.size()+ecalshowers.size()<2) return;

  // Check quality of tracks
  auto tracks=event->Get<DChargedTrack>();
  if (tracks.size()!=1) return;
  auto hyp=tracks[0]->Get_BestTrackingFOM();
  auto tb_track=hyp->Get_TrackTimeBased();
  if (tb_track->FOM<TRACK_CL_CUT) return;
  double dEdx=1e6*hyp->Get_dEdx_CDC_amp();
  if (dEdx<0.01) return; // Need to have some hits in CDC
  
  // Acquire root fill lock
  lockService->RootFillLock(this); 

  // Get the position and time at the vertex
  DVector3 vertex=hyp->position();
  double t0=hyp->t0();

  // Collect vectors of ECAL and FCAL hits that satisfy a time cut and some
  // basic selection criteria for the clusters
  vector<pair<DVector3,vector<const DECALHit*>>>ecalclusters;
  vector<pair<DVector3,vector<const DFCALHit*>>>fcalclusters;
  for (size_t i=0;i<ecalshowers.size();i++){
    // Apply minimum shower energy cut
    if (ecalshowers[i]->E<ESHOWER_MIN) continue;
    
    // Check the timing with respect to the "vertex" time at the target
    auto diff=ecalshowers[i]->pos-vertex;
    double dt=t0-(ecalshowers[i]->t-diff.Mag()/29.98);
    hProtonECALGammaTimeDiff->Fill(dt);
    if (dt>TIME_CUT) continue;
    
    // Get the cluster object associated with this shower
    auto cluster=ecalshowers[i]->GetSingle<DECALCluster>();
    if (cluster->status!=1) continue; // Single fitted peak?
      
    // Get the channel with the maximum energy
    int channel_Emax=cluster->channel_Emax;
    hEcalDt->Fill(channel_Emax,dt);

    // Skip if block with maximum energy is at the ECAL/FCAL border or near the
    // beam hole
    int row=dECALGeom->row(channel_Emax);
    int col=dECALGeom->column(channel_Emax);
    if (row==0 || col==0 || row==39 || col==39) continue;
    if (row>=19 && row<=22 && col>=19 && col<=22) continue;
    
    auto dir=diff.Unit();
    auto hits=cluster->Get<DECALHit>();
    ecalclusters.push_back(make_pair(dir,hits));
  }
  for (size_t i=0;i<fcalshowers.size();i++){
    // Apply minimum shower energy cut
    if (fcalshowers[i]->getEnergy()<ESHOWER_MIN) continue;
    
    // Check the timing with respect to the "vertex" time at the target
    auto diff=fcalshowers[i]->getPosition()-vertex;
    double dt=t0-(fcalshowers[i]->getTime()-diff.Mag()/29.98);
    hProtonFCALGammaTimeDiff->Fill(dt);
    if (fabs(dt)>TIME_CUT) continue;
    
    // Get the cluster object associated with this shower
    auto cluster=fcalshowers[i]->GetSingle<DFCALCluster>();
 
    // Get the channel with the maximum energy
    int channel_Emax=cluster->getChannelEmax();

    // Time difference corresponding to this channel
    double dt_Emax=t0-(fcalshowers[i]->getTimeMaxE()-diff.Mag()/29.98);
    hFcalDt->Fill(channel_Emax,dt_Emax);

    // Skip if block with maximum energy is at the ECAL/FCAL border
    int row=dFCALGeom->row(channel_Emax);
    int col=dFCALGeom->column(channel_Emax);
    if (row>=18 && row<=40 && col>=18 && col<=40) continue;
    
    auto dir=diff.Unit();
    auto hits=cluster->Get<DFCALHit>();
    fcalclusters.push_back(make_pair(dir,hits));
  }
    
  // Two photon final states
  if (ecalclusters.size()==2 && fcalclusters.size()==0){
    EcalAnalysis(ecalclusters[0],ecalclusters[1]);
  }
  if (fcalclusters.size()==2 && ecalclusters.size()==0){
    FcalAnalysis(fcalclusters[0],fcalclusters[1]);
  }
  if (fcalclusters.size()==1 && ecalclusters.size()==1){
    EcalFcalAnalysis(ecalclusters[0],fcalclusters[0]);
  }
    
  // Release root fill lock
  lockService->RootFillUnLock(this);
}

//------------------
// EndRun
//------------------
void JEventProcessor_CalCal2::EndRun()
{

}

//------------------
// Finish
//------------------
void JEventProcessor_CalCal2::Finish()
{
  ofstream ecalgainfile("ecal_gains.dat");
  for (size_t i=0;i<ecal_gains.size();i++){
    ecalgainfile << ecal_gains[i] << endl;
    int row=dECALGeom->row(i);
    int col=dECALGeom->column(i);
    if (dECALGeom->isBlockActive(row,col)){
      hEcalGains->Fill(ecal_gains[i]);
      hEcalGains2D->Fill(col,row,ecal_gains[i]);
    }
  }
  ofstream fcalgainfile("fcal_gains.dat");
  for (size_t i=0;i<fcal_gains.size();i++){
    fcalgainfile << fcal_gains[i] << endl;
    int row=dFCALGeom->row(i);
    int col=dFCALGeom->column(i);
    if (dFCALGeom->isBlockActive(row,col)){
      hFcalGains->Fill(fcal_gains[i]);
      hFcalGains2D->Fill(col,row,fcal_gains[i]);
    }
  }

  if (ecal_times.size()>0){
    hEcalDt->FitSlicesY();
    TH1F *hEcalTimeMeans=(TH1F*)gDirectory->Get("hEcalDt_2");
    double t_mean=0.;
    int num_active=0;
    for (size_t i=0;i<1600;i++){
      ecal_times[i]+=hEcalTimeMeans->GetBinContent(i+1);
      int row=dECALGeom->row(i);
      int col=dECALGeom->column(i);
      if (dECALGeom->isBlockActive(row,col)){
	num_active++;
	t_mean+=ecal_times[i];
      }
    }
    t_mean/=double(num_active);
    ofstream ecaltimefile("ecal_times.dat");
    for (size_t i=0;i<1600;i++){
      ecaltimefile << ecal_times[i]-t_mean << endl;
    }
  }
  hFcalDt->FitSlicesY();
  TH1F *hFcalTimeMeans=(TH1F*)gDirectory->Get("hFcalDt_2");
  double t_mean=0.;
  int num_active=0;
  for (size_t i=0;i<1600;i++){
    fcal_times[i]+=hFcalTimeMeans->GetBinContent(i+1);
    int row=dFCALGeom->row(i);
    int col=dFCALGeom->column(i);
    if (dFCALGeom->isBlockActive(row,col)){
      num_active++;
      t_mean+=ecal_times[i];
    }
  }
  t_mean/=double(num_active);
  ofstream fcaltimefile("fcal_times.dat");
  for (size_t i=0;i<2800;i++){
    fcaltimefile << fcal_times[i]-t_mean << endl;
  }
  //TFitResultPtr fit1=h2GammaMassFCAL->Fit("gaus","S");
  //cout << "FCAL m=" << fit1->Parameter(1) << " s="<< fit1->Parameter(2) <<endl;
  
  
}

// Apply pi0 mass constraint with the assumption that the error in the opening
// angle is negligible
void JEventProcessor_CalCal2::ApplyConstraint(Particle_t particle,
					      double one_minus_costheta12,
					      double E1S,double V1sum,
					      double E2S,double V2sum,
					      vector<double>&E1vec,
					      vector<double>&V1vec,
					      vector<double>&E2vec,
					      vector<double>&V2vec) const{
  double mass_sq=pow(ParticleMass(particle),2.);
  double E1sum=E1S,E2sum=E2S;
  size_t num_cluster1=E1vec.size();
  size_t num_cluster2=E2vec.size();
  
  // iterate with chi^2 check for convergence
  vector<double>E1Improved(num_cluster1),E2Improved(num_cluster2);
  int iter=0;
  double chisq=1e10,chisq_old;
  do {
    chisq_old=chisq;
    double r=2.*one_minus_costheta12
      *(E2sum*E1S+E1sum*E2S-E1sum*E2sum)-mass_sq;
    double S=4*one_minus_costheta12*one_minus_costheta12
      *(E1sum*E1sum*V2sum+E2sum*E2sum*V1sum);
    // Lagrange multiplier
    double lagrange=r/S;
		      
    for (size_t k=0;k<num_cluster1;k++){
      E1Improved[k]=E1vec[k]-2.*one_minus_costheta12*E2sum*lagrange*V1vec[k];
    }
    for (size_t k=0;k<num_cluster2;k++){
      E2Improved[k]=E2vec[k]-2.*one_minus_costheta12*E1sum*lagrange*V2vec[k];
    }
    E1sum=0.;
    for (size_t k=0;k<num_cluster1;k++){
      E1sum+=E1Improved[k];
    }
    E2sum=0.;
    for (size_t k=0;k<num_cluster2;k++){
      E2sum+=E2Improved[k];
    }
    chisq=lagrange*(S*lagrange+2.*r);
    iter++;
  }
  while (fabs(chisq-chisq_old)>0.01 && iter<20);
  
  // Update energy vectors with improved values
  E1vec=E1Improved;
  E2vec=E2Improved;
}

// Loop over pairs of showers in the ECAL
void JEventProcessor_CalCal2::EcalAnalysis(pair<DVector3,vector<const DECALHit*>>&cluster1,pair<DVector3,vector<const DECALHit*>>&cluster2){
  // Store energy and variance for each hit in the clusters
  vector<double>E1vec,V1vec;
  double E1sum=0.,V1sum=0.;
  MakeEcalVectors(cluster1.second,E1sum,V1sum,E1vec,V1vec);
  vector<double>E2vec,V2vec;
  double E2sum=0.,V2sum=0.;
  MakeEcalVectors(cluster2.second,E2sum,V2sum,E2vec,V2vec);
      
  // Angular dependence of invariant mass
  double one_minus_costheta12=1.-cluster1.first.Dot(cluster2.first);
  // 2 photon mass
  double mass=sqrt(2.*E1sum*E2sum*one_minus_costheta12);
  h2GammaMassECAL->Fill(mass);
  
  // Apply the pi0 constraint
  if (mass>PI0_LO_CUT && mass<PI0_HI_CUT){
    ApplyConstraint(Pi0,one_minus_costheta12,E1sum,V1sum,E2sum,V2sum,E1vec,
		    V1vec,E2vec,V2vec);
    
    // Update gain factors
    UpdateGains(E1vec,cluster1.second);
    UpdateGains(E2vec,cluster2.second);
  }
}

// Loop over pairs of showers in the FCAL
void JEventProcessor_CalCal2::FcalAnalysis(pair<DVector3,vector<const DFCALHit*>>&cluster1,pair<DVector3,vector<const DFCALHit*>>&cluster2){
  // Store energy and variance for each hit in the clusters
  vector<double>E1vec,V1vec;
  double E1sum=0.,V1sum=0.;
  MakeFcalVectors(cluster1.second,E1sum,V1sum,E1vec,V1vec);
  vector<double>E2vec,V2vec;
  double E2sum=0,V2sum=0.;
  MakeFcalVectors(cluster2.second,E2sum,V2sum,E2vec,V2vec);
  
  // Angular dependence of invariant mass
  double one_minus_costheta12=1.-cluster1.first.Dot(cluster2.first);
  // 2 photon mass
  double mass=sqrt(2.*E1sum*E2sum*one_minus_costheta12);
  h2GammaMassFCAL->Fill(mass);
  
  // Apply the pi0 constraint
  if (mass>PI0_LO_CUT && mass<PI0_HI_CUT){
    ApplyConstraint(Pi0,one_minus_costheta12,E1sum,V1sum,E2sum,V2sum,
		    E1vec,V1vec,E2vec,V2vec);

    // Update gain factors
    UpdateGains(E1vec,cluster1.second);
    UpdateGains(E2vec,cluster2.second);
  }
}

// Deal with one shower in ECAL and one shower in FCAL 
void JEventProcessor_CalCal2::EcalFcalAnalysis(pair<DVector3,vector<const DECALHit*>>&ecluster,pair<DVector3,vector<const DFCALHit*>>&fcluster){
  // Store energy and variance for each hit in these clusters
  vector<double>E1vec,V1vec;
  double E1sum=0,V1sum=0.;
  MakeEcalVectors(ecluster.second,E1sum,V1sum,E1vec,V1vec);
  vector<double>E2vec,V2vec;
  double E2sum=0,V2sum=0.;
  MakeFcalVectors(fcluster.second,E2sum,V2sum,E2vec,V2vec);
  
  // Angular dependence of invariant mass
  double one_minus_costheta12=1.-ecluster.first.Dot(fcluster.first);
  // 2 photon mass
  double mass=sqrt(2.*E1sum*E2sum*one_minus_costheta12);
  h2GammaMassECALFCAL->Fill(mass);
  
  // Apply the pi0 constraint
  bool update_gains=false;
  if (mass>PI0_LO_CUT && mass<PI0_HI_CUT){
    ApplyConstraint(Pi0,one_minus_costheta12,E1sum,V2sum,E2sum,V2sum,E1vec,
		    V1vec,E2vec,V2vec);
  }
  // or eta constraint
  if (mass>ETA_LO_CUT && mass<ETA_HI_CUT){
    ApplyConstraint(Eta,one_minus_costheta12,E1sum,V1sum,E2sum,V2sum,E1vec,
		    V1vec,E2vec,V2vec);
    update_gains=true;
  }
    
  // Update gain factors
  if (update_gains){
    UpdateGains(E1vec,ecluster.second);
    UpdateGains(E2vec,fcluster.second);
  }
}

// Find updated gains and gain variances for ECAL
void JEventProcessor_CalCal2::UpdateGains(vector<double>&Evec,
					  vector<const DECALHit*>&hits){
  for (size_t k=0;k<hits.size();k++){
    const DECALHit *hit=hits[k];
    int channel=dECALGeom->channel(hit->row,hit->column);
    double Ek=hit->E/old_ecal_gains[channel];
    if (Ek>EHIT_MIN){  
      double new_gain=Evec[k]/Ek;
      if (new_gain>0.8 && new_gain<1.2){
	// Running average
	double N=ecal_counts[channel];
	ecal_gains[channel]=(N*ecal_gains[channel]+new_gain)/(N+1.);
	ecal_counts[channel]+=1.;
      }
    }
  }
}

// Find updated gains and gain variances for FCAL
void JEventProcessor_CalCal2::UpdateGains(vector<double>&Evec,
					  vector<const DFCALHit*>&hits){
  for (size_t k=0;k<hits.size();k++){
    const DFCALHit *hit=hits[k];
    int channel=dFCALGeom->channel(hit->row,hit->column);
    double Ek=hit->E/old_fcal_gains[channel];
    if (Ek>EHIT_MIN){  
      double new_gain=Evec[k]/Ek;
      if (new_gain>0.8 && new_gain<1.2){
	// Running average
	double N=fcal_counts[channel];
	fcal_gains[channel]=(N*fcal_gains[channel]+new_gain)/(N+1.);
	fcal_counts[channel]+=1.;
      }
    }
  }
}

// Fill ECAL data needed for constrained fit
void JEventProcessor_CalCal2::MakeEcalVectors(vector<const DECALHit *>&hits,
					      double &Esum,double &Vsum,
					      vector<double>&Evec,
					      vector<double>&Vvec) const {
  for (size_t k=0;k<hits.size();k++){
    const DECALHit *hit=hits[k];
    int channel=dECALGeom->channel(hit->row,hit->column);
    double E=ecal_gains[channel]*hit->E/old_ecal_gains[channel];
    Evec.push_back(E);
    Esum+=E;
    double Vtemp=ECAL_VAR_E_PAR0+ECAL_VAR_E_PAR1*E+ECAL_VAR_E_PAR2*E*E;
    Vvec.push_back(Vtemp);
    Vsum+=Vtemp;
  }
}

// Fill FCAL data needed for constrained fit
void JEventProcessor_CalCal2::MakeFcalVectors(vector<const DFCALHit *>&hits,
					      double &Esum,double &Vsum,
					      vector<double>&Evec,
					      vector<double>&Vvec) const {
  for (size_t k=0;k<hits.size();k++){
    const DFCALHit *hit=hits[k];
    int channel=dFCALGeom->channel(hit->row,hit->column);
    double E=fcal_gains[channel]*hit->E/old_fcal_gains[channel];
    Evec.push_back(E);
    Esum+=E;
    double Vtemp=FCAL_VAR_E_PAR0+FCAL_VAR_E_PAR1*E+FCAL_VAR_E_PAR2*E*E;
    Vvec.push_back(Vtemp);
    Vsum+=Vtemp;
  }
}
