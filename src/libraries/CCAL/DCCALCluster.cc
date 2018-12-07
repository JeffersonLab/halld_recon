/*
 *  File: DCCALCluster.cc
 *
 * Created on 11/25/18 by A.S. 
 * use structure similar to FCAL
 */

#include <math.h>
#include "DCCALCluster.h"
#include "DCCALGeometry.h"

DCCALCluster::DCCALCluster()
{
   fEnergy  =  0;
   fTime    =  0;
   fCentroid.SetXYZ( 0., 0., 0.);
}

DCCALCluster::~DCCALCluster()
{

}
