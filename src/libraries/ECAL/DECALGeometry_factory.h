// $Id$
/*
 *  File: DECALHit_factory.h
 *
 *  Created on 01/16/2024 by A.S.  
 */

#ifndef _DECALGeometry_factory_
#define _DECALGeometry_factory_

#include <JANA/JFactory.h>
#include "DECALGeometry.h"

class DECALGeometry_factory:public jana::JFactory<DECALGeometry>{
 public:
  DECALGeometry_factory(){};
  ~DECALGeometry_factory(){};
  
  DECALGeometry *ecalgeometry = nullptr;
  
  //------------------
  // brun
  //------------------
  jerror_t brun(JEventLoop *loop, int32_t runnumber)
  {
    DApplication *dapp = dynamic_cast<DApplication*>(loop->GetJApplication());
    const DGeometry *geom = dapp->GetDGeometry(runnumber);

    flags = PERSISTANT;
    _data.push_back( new DECALGeometry(geom) );
    
    return NOERROR;
  }
  
  //------------------
  // erun
  //------------------
  jerror_t erun(void)
  {
    for(unsigned int i=0; i<_data.size(); i++)delete _data[i];
    _data.clear();
    
    return NOERROR;
  }
};

#endif // _DECALGeometry_factory_

