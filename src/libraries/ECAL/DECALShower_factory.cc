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

