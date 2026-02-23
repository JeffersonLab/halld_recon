// $Id$
/*
 *  File: DECALGeometry_factory.h
 *
 *  Created on 03/26/2025 by S.T.
 */

#ifndef _DECALGeometry_factory_
#define _DECALGeometry_factory_

#include <JANA/JFactoryT.h>
#include "DECALGeometry.h"
#include <HDGEOMETRY/DGeometry.h>
#include <DANA/DGeometryManager.h>
#include <JANA/Calibrations/JCalibrationManager.h>

class DECALGeometry_factory:public JFactoryT<DECALGeometry>{
public:
  DECALGeometry_factory() = default;
  ~DECALGeometry_factory() override = default;
private:
  //------------------
  // BeginRun
  //------------------
  void BeginRun(const std::shared_ptr<const JEvent>& event){
    SetFactoryFlag(PERSISTENT);
    auto runnumber = event->GetRunNumber();
    auto app = event->GetJApplication();
    auto geo_manager = app->GetService<DGeometryManager>();
    auto dgeom = geo_manager->GetDGeometry(runnumber);
    if (dgeom->HaveInsert()){
      auto jcalib =  app->GetService<JCalibrationManager>()->GetJCalibration(runnumber);
      Insert(new DECALGeometry(dgeom,jcalib));
    }
  } 
  //------------------
  // EndRun
  //------------------
  void EndRun(){
    // We have to manually clear and delete the contents of mData because PERSISTENT flag was set.
    for (auto geom : mData) {
      delete geom;
    }
    mData.clear();
  }
};

#endif // _DECALGeometry_factory_

