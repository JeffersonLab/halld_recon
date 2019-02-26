/*
 *  File: DCCALShower.cc
 *
 * Created on 11/25/18 by A.S. 
 * use structure similar to FCAL
 */

#include <math.h>
#include "DCCALShower.h"
#include "DCCALGeometry.h"
#include "hycal.h"

DCCALShower::DCCALShower()
{

  type     =  0;
  dime     =  0;    
  status   =  0;
  id       =  0;
  idmax    =  0;
  E        =  0;
  x        =  0;
  y        =  0;
  z        =  0;
  x1       =  0;
  y1       =  0;
  chi2     =  0;
  sigma_E  =  0;
  Emax     =  0;
  time     =  0;
  sigma_t  =  0;
  
  for(int icell=0; icell<MAX_CC; icell++) {
    id_storage[icell] = 0;
    en_storage[icell] = 0;
  }
  
}

DCCALShower::~DCCALShower()
{

}
