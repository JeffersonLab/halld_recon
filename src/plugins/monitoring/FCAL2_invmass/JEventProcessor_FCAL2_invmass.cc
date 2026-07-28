// $Id$
//
//    File: JEventProcessor_FCAL2_invmass.cc
// Created: Thu Apr  9 02:26:57 PM EDT 2026
// Creator: staylor (on Linux ifarm2402.jlab.org 5.14.0-611.38.1.el9_7.x86_64 x86_64)
//

/// For more information on the syntax changes between JANA1 and JANA2, visit: https://jeffersonlab.github.io/JANA2/#/jana1to2/jana1-to-jana2

#include "JEventProcessor_FCAL2_invmass.h"
#include <HDGEOMETRY/DGeometry.h>
#include <PID/DVertex.h>
#include <PID/DNeutralParticle.h>
#include <DANA/DEvent.h>
#include <ECAL/DECALCluster.h>
#include <ECAL/DECALShower.h>
#include <FCAL/DFCALCluster.h>
#include <FCAL/DFCALShower.h>
#include <TDirectory.h>

// Routine used to create our JEventProcessor
#include <JANA/JApplication.h>
extern "C"{
void InitPlugin(JApplication *app){
    InitJANAPlugin(app);
    app->Add(new JEventProcessor_FCAL2_invmass());
}
} // "C"


//------------------
// JEventProcessor_FCAL2_invmass (Constructor)
//------------------
JEventProcessor_FCAL2_invmass::JEventProcessor_FCAL2_invmass()
{
    // Parameters and Services should be accessed from Init() instead of here!
}

//------------------
// ~JEventProcessor_FCAL2_invmass (Destructor)
//------------------
JEventProcessor_FCAL2_invmass::~JEventProcessor_FCAL2_invmass()
{
	SetTypeName(NAME_OF_THIS); // Provide JANA with this class's name
}

//------------------
// Init
//------------------
void JEventProcessor_FCAL2_invmass::Init()
{
  auto app = GetApplication();
  lockService = app->GetService<JLockService>();
  lockService->RootWriteLock();
  
  gDirectory->mkdir("FCAL2_invmass")->cd();
  h_2gamma_ECAL_ECAL=new TH1F("h_2gamma_ECAL_ECAL","ECAL+ECAL;m(2#gamma) [GeV]",
			      500,0,1);
  h_2gamma_ECAL_ECAL_vs_E=new TH2F("h_2gamma_ECAL_ECAL_vs_E","ECAL+ECAL;E [GeV];m(2#gamma) [GeV]",
			      30,-0.1,5.9,500,0,1);
  h_Epred_ECAL_ECAL_vs_E=new TH2F("h_Epred_ECAL_ECAL_vs_E","ECAL+ECAL;E [GeV];E{{pred}(2#gamma) [GeV]",
				  240,0.0,6.,240,0.,6.);
  h_2gamma_ECAL_ECAL_vs_ch=new TH2F("h_2gamma_ECAL_ECAL_vs_ch","ECAL+ECAL;channel;m(2#gamma) [GeV]",
				    1600,-0.5,1599.5,100,0.08,0.18);
  h_2gamma_ECAL_FCAL=new TH1F("h_2gamma_ECAL_FCAL","ECAL+FCAL;m(2#gamma) [GeV]",
			      500,0,1);
  h_2gamma_ECAL_FCAL_vs_E=new TH2F("h_2gamma_ECAL_FCAL_vs_E","ECAL+FCAL;E [GeV];m(2#gamma) [GeV]",
			      30,-0.1,5.9,500,0,1);
 
  h_2gamma_FCAL_FCAL=new TH1F("h_2gamma_FCAL_FCAL","FCAL+FCAL;m(2#gamma) [GeV]",
			      500,0,1);
  h_2gamma_FCAL_FCAL_vs_E=new TH2F("h_2gamma_FCAL_FCAL_vs_E","FCAL+FCAL;E [GeV];m(2#gamma) [GeV]",
			      30,-0.1,5.9,500,0,1);
  h_Epred_FCAL_FCAL_vs_E=new TH2F("h_Epred_FCAL_FCAL_vs_E","FCAL+FCAL;E [GeV];E{{pred}(2#gamma) [GeV]",
				  240,0.0,6.,240,0.,6.);
  h_2gamma_FCAL_FCAL_vs_ch=new TH2F("h_2gamma_FCAL_FCAL_vs_ch","FCAL+FCAL;channel;m(2#gamma) [GeV]",
				    2800,-0.5,2799.5,100,0.08,0.18);
  h_2gamma_BCAL_BCAL=new TH1F("h_2gamma_BCAL_BCAL","BCAL+BCAL;m(2#gamma) [GeV]",
			      500,0,1);
  h_2gamma_BCAL_ECAL=new TH1F("h_2gamma_BCAL_ECAL","BCAL+ECAL;m(2#gamma) [GeV]",
			      500,0,1);
  h_2gamma_BCAL_FCAL=new TH1F("h_2gamma_BCAL_FCAL","BCAL+FCAL;m(2#gamma) [GeV]",
			      500,0,1);
  gDirectory->cd("../");

  lockService->RootUnLock();

  SHOWER_THRESHOLD=0.5;
  app->SetDefaultParameter("FCAL2_INVMASS:SHOWER_THRESHOLD",SHOWER_THRESHOLD);
  ECAL_POS_CUT=4.0;
  app->SetDefaultParameter("FCAL2_INVMASS:ECAL_POS_CUT",ECAL_POS_CUT);
  DELTA_E_CUT=0.1;
  app->SetDefaultParameter("FCAL2_INVMASS:DELTA_E_CUT",DELTA_E_CUT);
}

//------------------
// BeginRun
//------------------
void JEventProcessor_FCAL2_invmass::BeginRun(const std::shared_ptr<const JEvent> &event)
{
 const DGeometry *geom=DEvent::GetDGeometry(event);
 double x0,y0,z0;
 geom->GetFCALPosition(x0,y0,z0);
 m_FCALCenter.SetXYZ(x0,y0,z0);
}

//------------------
// Process
//------------------
void JEventProcessor_FCAL2_invmass::Process(const std::shared_ptr<const JEvent> &event)
{
  auto neutrals=event->Get<DNeutralParticle>();
  if (neutrals.size()!=2) return;
  
  auto myDVertex=event->GetSingle<DVertex>();
  if (myDVertex==NULL) return;

  // Vertex position and time
  auto vertex=myDVertex->dSpacetimeVertex.Vect();
  if (vertex.z()<50. || vertex.z()>80. || vertex.Perp()>1.) return;
  double t0_rf=myDVertex->dSpacetimeVertex.T();

  lockService->RootFillLock(this); 

  for (size_t i=0;i<neutrals.size();i++){
    auto gamma=neutrals[i]->Get_Hypothesis(Gamma);
   
    // Shower energy
    double E1=gamma->lorentzMomentum().E();
    if (E1<SHOWER_THRESHOLD) continue;

    auto shower=gamma->Get_NeutralShower();
    DetectorSystem_t det1=shower->dDetectorSystem;
    if (det1==SYS_ECAL){
      // Avoid the beam hole
      auto pos_rel=shower->dSpacetimeVertex.Vect()-m_FCALCenter;
      if (fabs(pos_rel.X())<ECAL_POS_CUT && fabs(pos_rel.Y())<ECAL_POS_CUT) continue;
    }
    // Compute the lorentz momentum
    auto diff=shower->dSpacetimeVertex.Vect()-vertex;
    auto dir1=(1./diff.Mag())*diff;
    DLorentzVector gamma1_v4(E1*dir1,E1);
    
    // Check time at the vertex
    double dt=shower->dSpacetimeVertex.T()-diff.Mag()/29.98-t0_rf;
    if (fabs(dt)>2.) continue;

    for(size_t j=i+1;j<neutrals.size();j++){
      gamma=neutrals[j]->Get_Hypothesis(Gamma);
 
      // Shower energy
      double E2=gamma->lorentzMomentum().E();
      if (E2<SHOWER_THRESHOLD) continue;

      // Compute the lorentz momentum
      shower=gamma->Get_NeutralShower();
      DetectorSystem_t det2=shower->dDetectorSystem;
      if (det2==SYS_ECAL){
	// Avoid the beam hole
	auto pos_rel=shower->dSpacetimeVertex.Vect()-m_FCALCenter;
	if (fabs(pos_rel.X())<4. && fabs(pos_rel.Y())<4.) continue;
      }
      // Compute the lorentz momentum
      diff=shower->dSpacetimeVertex.Vect()-vertex;
      auto dir2=(1./diff.Mag())*diff;
      DLorentzVector gamma2_v4(E2*dir2,E2);

      // Check time at the vertex
      double dt=shower->dSpacetimeVertex.T()-diff.Mag()/29.98-t0_rf;
      if (fabs(dt)>2.) continue;

      // Two photon invariant mass
      auto twogamma=gamma1_v4+gamma2_v4;
      double mass=twogamma.M();

      // Expected E for E1=E2
      double Epred=ParticleMass(Pi0)/sqrt(2.*(1.-dir1.Dot(dir2)));

      // Fill some mass histograms
      double E_average=0.5*(E1+E2);
      if (det1==SYS_BCAL && det2==SYS_BCAL){
	h_2gamma_BCAL_BCAL->Fill(mass);
      }
      else if ((det1==SYS_BCAL && det2==SYS_FCAL)
	       || (det2==SYS_BCAL && det1==SYS_FCAL)){
	h_2gamma_BCAL_FCAL->Fill(mass);
      }
      else if ((det1==SYS_ECAL && det2==SYS_BCAL)
	       || (det2==SYS_ECAL && det1==SYS_BCAL)){
	h_2gamma_BCAL_ECAL->Fill(mass);
      }
      else if (det1==SYS_ECAL && det2==SYS_ECAL){
	h_2gamma_ECAL_ECAL->Fill(mass);
	if (fabs(E1-E2)<DELTA_E_CUT){
	  h_2gamma_ECAL_ECAL_vs_E->Fill(E_average,mass);
	  if (mass>0.05 && mass<0.16){
	    h_Epred_ECAL_ECAL_vs_E->Fill(E_average,Epred);
	  }
	}
	auto ecal_shower=static_cast<const DECALShower *>(shower->dBCALFCALShower);
	auto ecal_cluster=ecal_shower->GetSingle<DECALCluster>();
	h_2gamma_ECAL_ECAL_vs_ch->Fill(ecal_cluster->channel_Emax,mass);
      }
      else if ((det1==SYS_ECAL && det2==SYS_FCAL)
	       || (det2==SYS_ECAL && det1==SYS_FCAL)){
	h_2gamma_ECAL_FCAL->Fill(mass);
	if (fabs(E1-E2)<0.1){
	  h_2gamma_ECAL_FCAL_vs_E->Fill(E_average,mass);
	}
      }
      else if (det1==SYS_FCAL && det2==SYS_FCAL){
	h_2gamma_FCAL_FCAL->Fill(mass);
	if (fabs(E1-E2)<DELTA_E_CUT){
	  h_2gamma_FCAL_FCAL_vs_E->Fill(E_average,mass);
	  if (mass>0.05 && mass<0.16){
	    h_Epred_FCAL_FCAL_vs_E->Fill(E_average,Epred);
	  }
	}
	auto fcal_shower=static_cast<const DFCALShower *>(shower->dBCALFCALShower);
	auto fcal_cluster=fcal_shower->GetSingle<DFCALCluster>();
	h_2gamma_FCAL_FCAL_vs_ch->Fill(fcal_cluster->getChannelEmax(),mass);
      }   
    }
  }

  lockService->RootFillUnLock(this); 
}

//------------------
// EndRun
//------------------
void JEventProcessor_FCAL2_invmass::EndRun()
{
    // This is called whenever the run number changes, before it is
    // changed to give you a chance to clean up before processing
    // events from the next run number.
}

//------------------
// Finish
//------------------
void JEventProcessor_FCAL2_invmass::Finish()
{
    // Called before program exit after event processing is finished.
}
