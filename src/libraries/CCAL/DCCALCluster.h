/*
 *  File: DCCALHit_factory.h
 *
 * Created on 11/25/18 by A.S. 
 * use structure similar to FCAL
 */


#ifndef _DCCALCluster_
#define _DCCALCluster_

#include <DVector3.h>
#include "DCCALHit.h"
using namespace std;

#include <JANA/JObject.h>
#include <JANA/JFactory.h>
using namespace jana;

#define CCAL_USER_HITS_MAX  1000
#define MAX_SHOWER_RADIUS   5

class DCCALCluster : public JObject {
   public:
      JOBJECT_PUBLIC(DCCALCluster);
                
      DCCALCluster();
      ~DCCALCluster();

      double   fEnergy;      // total cluster energy (GeV) or 0 if stale
      DVector3 fCentroid;    // cluster centroid position (cm) [z=0 at target center]
      double   fTime;        // cluster time(ns) 


      double getEnergy() const; 
      DVector3 getCentroid() const;
      double getTime() const;

      void toStrings(vector<pair<string,string> > &items) const {
	AddString(items, "x(cm)", "%3.1f", getCentroid().x());
	AddString(items, "y(cm)", "%3.1f", getCentroid().y());
	AddString(items, "z(cm)", "%3.1f", getCentroid().z());
	AddString(items, "E(GeV)", "%2.3f", getEnergy());
	AddString(items, "t(ns)", "%2.3f", getTime());
      }
      


 private:
      

      
};


inline double DCCALCluster::getEnergy() const
{
  return fEnergy;
}

inline DVector3 DCCALCluster::getCentroid() const
{
   return fCentroid;
}

inline double DCCALCluster::getTime() const
{
   return fTime;
}


#endif // _DCCALCluster_

