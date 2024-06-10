/*
 *  File: DCCALShower.cc
 *
 * Created on 11/25/18 by A.S. 
 * use structure similar to FCAL
 */

#include <math.h>
#include "DCCALShower.h"
#include "DCCALGeometry.h"
#include "ccal.h"

DCCALShower::DCCALShower():ExyztCovariance(5)
{
  E        =  0.;
  Esum     =  0.;
  
  x        =  0.;
  y        =  0.;
  x1       =  0.;
  y1       =  0.;
  z        =  0.;
  
  chi2     =  0.;
  sigma_E  =  0.;
  Emax     =  0.;
  time     =  0.;
  sigma_t  =  0.;
  
  dime     =  0;
  idmax    =  0;
  id       =  0;
  type     =  0;
}

DCCALShower::~DCCALShower()
{
  
}
