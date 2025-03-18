// $Id$
//
//    File: DTrackUtils_factory.h
//

#ifndef _DTrackUtils_factory_
#define _DTrackUtils_factory_

#include <JANA/JFactory.h>
#include "DTrackUtils.h"

class DTrackUtils_factory : public jana::JFactory<DTrackUtils>
{
 public:
  DTrackUtils_factory(){trackUtils=NULL;};
  ~DTrackUtils_factory(){};
  
  //------------------
  // brun
  //------------------
  jerror_t brun(JEventLoop *loop, int32_t runnumber)
  {
    SetFactoryFlag(NOT_OBJECT_OWNER);
    ClearFactoryFlag(WRITE_TO_OUTPUT);
    
    if( trackUtils ) delete trackUtils;
    
    trackUtils = new DTrackUtils(loop);
    
    return NOERROR;
  }
  
  //------------------
  // evnt
  //------------------
  jerror_t evnt(JEventLoop *loop, uint64_t eventnumber)
  {
    // Reuse existing DTrackUtils object.
    if( trackUtils ) _data.push_back( trackUtils );
    
    return NOERROR;
  }
  
  //------------------
  // erun
  //------------------
  jerror_t erun(void)
  {
    if( trackUtils ) delete trackUtils;
    trackUtils = NULL;
    
    return NOERROR;
  }
  
  DTrackUtils *trackUtils;
};

#endif // _DTrackUtils_factory_

