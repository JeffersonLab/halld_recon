//********************************************************************************************
// DTRDSegment.h : definition for a track segment built from points - derived from DFDCSegment
//********************************************************************************************

#ifndef DTRDSEGMENT_H
#define DTRDSEGMENT_H

#include <JANA/JObject.h>
using namespace jana;

#include "DTRDHit.h"
#include "DTRDPoint.h"

#include <DMatrix.h>
#include <sstream>

// Residuals and arc length
struct fdc_track_t{
  unsigned int hit_id;
  double sign; // Sign of left-right ambiguity resolution for this hit
  double dx,dy; //residuals
  double chi2; // chi2 contribution of this point
  double s; // path length
};


///
/// class DTRDSegment: definition for a track segment in the TRD
/// 
class DTRDSegment : public JObject {
	public :
		JOBJECT_PUBLIC(DTRDSegment);			/// DANA identifier
		
		/// 
		/// DFDCSegment::DFDCSegment():
		/// Default constructor
		///
		DTRDSegment(){}
		DTRDSegment(const DTRDSegment &segment){
		  this->xc=segment.xc;
		  this->yc=segment.yc;
		  this->rc=segment.rc;
		  this->tanl=segment.tanl;
		  this->q=segment.q;
		  this->D=segment.D;
		  this->z_vertex=segment.z_vertex;
		  this->phi0=segment.phi0;
		  this->Phi1=segment.Phi1;
		  this->package=segment.package;
		  this->chisq=segment.chisq;
		  this->Ndof=segment.Ndof;
		}

		double chisq;
		int Ndof;

		// circle parameters
		double xc,yc,rc;  
		// azimuthal angle of the intersection of the helical path to 
		// the most downstream plane in a given package containing a
		// hit
		double Phi1;		               
		// "vertex" z position and the phi angle there
		double z_vertex,phi0;
		// tangent of the dip angle
		double tanl;
		// charge (+/-1)
		double q;
		// Distance of closest approach to the beam line
		double D;
		// Package number
		unsigned int package;
		
		// List of points belonging to this track segment
		vector<const DTRDPoint *>hits;	

      void Summarize(JObjectSummary& summary) const override {
         summary.add(xc, "xc", "%3.2f");
         summary.add(yc, "yc", "%3.2f");
         summary.add(rc, "rc", "%3.2f");
         summary.add(57.296*phi0, "phi0(deg)", "%3.2f");
         summary.add(Phi1, "Phi1(rad)", "%3.2f");
         summary.add(D, "D", "%3.1f");
         summary.add(q, "q", "%d");
         summary.add(hits.size(), "Nhits", "%3.1f");
      }

};

#endif //DTRDSEGMENT_H
