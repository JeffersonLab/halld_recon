// $Id$
//
//    File: DECALShower_factory.cc
// Created: Tue Mar 25 10:43:23 AM EDT 2025
// Creator: staylor (on Linux ifarm2401.jlab.org 5.14.0-503.19.1.el9_5.x86_64 x86_64)
//

/// For more information on the syntax changes between JANA1 and JANA2, visit: https://jeffersonlab.github.io/JANA2/#/jana1to2/jana1-to-jana2

#include <iostream>
#include <iomanip>

#include "DECALShower_factory.h"
#include <JANA/JEvent.h>
#include <DANA/DGeometryManager.h>
#include <HDGEOMETRY/DGeometry.h>
#include <units.h>

//------------------
// Init
//------------------
void DECALShower_factory::Init()
{
  auto app = GetApplication();
 
  POS_RES1=0.168;
  POS_RES2=0.0636;
  E_VAR1=6.49300e-04;
  E_VAR2=1.24109e-04;
  E_VAR3=4.70327e-05;
  app->SetDefaultParameter("ECAL:POS_RES1",POS_RES1);
  app->SetDefaultParameter("ECAL:POS_RES2",POS_RES2);
  app->SetDefaultParameter("ECAL:E_VAR2",E_VAR2);
  app->SetDefaultParameter("ECAL:E_VAR3",E_VAR3);
  app->SetDefaultParameter("ECAL:E_VAR1",E_VAR1);

  SHOWER_ENERGY_THRESHOLD = 50*k_MeV;
  app->SetDefaultParameter("ECAL:SHOWER_ENERGY_THRESHOLD", SHOWER_ENERGY_THRESHOLD);

  E_CORRECTION_PAR1=0.016;
  E_CORRECTION_PAR2=0.864;
  E_CORRECTION_PAR3=1.055;
  E_CORRECTION_PAR4=2.0;
  app->SetDefaultParameter("ECAL:E_CORRECTION_PAR1",E_CORRECTION_PAR1);
  app->SetDefaultParameter("ECAL:E_CORRECTION_PAR2",E_CORRECTION_PAR2);
  app->SetDefaultParameter("ECAL:E_CORRECTION_PAR3",E_CORRECTION_PAR3);
  app->SetDefaultParameter("ECAL:E_CORRECTION_PAR4",E_CORRECTION_PAR4);

  bool ENABLE_ENERGY_CORRECTION=true;
  app->SetDefaultParameter("ECAL:ENABLE_ENERGY_CORRECTION",ENABLE_ENERGY_CORRECTION);
}

//------------------
// BeginRun
//------------------
void DECALShower_factory::BeginRun(const std::shared_ptr<const JEvent>& event)
{
  auto runnumber = event->GetRunNumber();
  auto app = event->GetJApplication();
  auto geo_manager = app->GetService<DGeometryManager>();
  auto geom = geo_manager->GetDGeometry(runnumber);
  geom->GetECALZ(mECALz);
}

//------------------
// Process
//------------------
void DECALShower_factory::Process(const std::shared_ptr<const JEvent>& event)
{
  vector<const DECALCluster*>clusters;
  event->Get(clusters);

  for (size_t i=0;i<clusters.size();i++){
    const DECALCluster *cluster=clusters[i];

    double E=cluster->E;
    if (ENABLE_ENERGY_CORRECTION){
      E=GetCorrectedEnergy(cluster->E);
    }
    if (E>SHOWER_ENERGY_THRESHOLD){
      DECALShower *shower=new DECALShower;
      shower->nBlocks=cluster->nBlocks;
      shower->isNearBorder=cluster->isNearBorder;
      shower->E=E;
      shower->E1E9=cluster->E1E9;
      shower->E9E25=cluster->E9E25;
      shower->t=cluster->t;

      double z=GetCorrectedZ(E);
      shower->pos=DVector3(cluster->x,cluster->y,z);

      // Guess for covariance matrix
      TMatrixFSym cov(5);
      double sigx=POS_RES1/sqrt(E)+POS_RES2;
      double sigy=sigx;
      const double radiation_length=0.89*k_cm;
      double X0_over_E=radiation_length/E;
      cov(1,1)=sigx*sigx;
      cov(2,2)=sigy*sigy;
      cov(0,0)=E*E_VAR1+E_VAR2+E_VAR3*E*E;
      cov(3,3)=X0_over_E*X0_over_E*cov(0,0);
      cov(3,0)=cov(0,3)=X0_over_E*cov(0,0);
      shower->ExyztCovariance.ResizeTo(5,5);
      shower->ExyztCovariance=cov;
      
      shower->AddAssociatedObject(cluster);
	
      Insert(shower);
    } // Check on mininum energy
  } // loop over clusters
}

//------------------
// EndRun
//------------------
void DECALShower_factory::EndRun()
{
}

//------------------
// Finish
//------------------
void DECALShower_factory::Finish()
{
}

// Correct the measured energy to account for the unmeasured part of of the
// shower
double DECALShower_factory::GetCorrectedEnergy(double Ecluster) const {
  double Ecorr=0.;
  if (Ecluster<E_CORRECTION_PAR4){
    Ecorr=Ecluster/(E_CORRECTION_PAR1*Ecluster+E_CORRECTION_PAR2);
  }
  else {
    Ecorr=E_CORRECTION_PAR4
      /(E_CORRECTION_PAR1*E_CORRECTION_PAR4+E_CORRECTION_PAR2)
      +E_CORRECTION_PAR3*(Ecluster-E_CORRECTION_PAR4);
  }
  return Ecorr;
}

// Correct the z-position of the shower to account for the shower depth
double DECALShower_factory::GetCorrectedZ(double E) const {
  // Crystal material properties
  const double radiation_length=0.89*k_cm;
  const double critical_energy=0.00964*k_GeV;
  // Assume photon-induced shower
  double dZmax=radiation_length*(0.5+log(E/critical_energy));

  return mECALz+dZmax;
}
