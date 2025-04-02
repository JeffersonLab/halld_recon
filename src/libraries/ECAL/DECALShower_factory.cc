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

    DECALShower *shower=new DECALShower;
    shower->E=cluster->E;
    shower->t=cluster->t;
    shower->pos=DVector3(cluster->x,cluster->y,mECALz);

    // Guess for covariance matrix
    double sigx=POS_RES1/sqrt(shower->E)+POS_RES2;
    double sigy=sigx;
    shower->ExyztCovariance(1,1)=sigx*sigx;
    shower->ExyztCovariance(2,2)=sigy*sigy;
    shower->ExyztCovariance(3,3)=1.; // fix this!
    shower->ExyztCovariance(0,0)
      =shower->E*shower->E*(E_VAR1/shower->E
			    + E_VAR2/(shower->E*shower->E)
			    + E_VAR3);
    // Make sure off-diagonal elements are zero, for now...
    for (unsigned int i=0;i<5;i++){
      for(unsigned int j=0;j<5;j++){
	if (i!=j) shower->ExyztCovariance(i,j)=0.;
      }
    }

    Insert(shower);
  }

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

