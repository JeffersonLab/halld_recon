// $Id$
//
//    File: DTrackFinder.h
// Created: Fri Aug 15 09:43:08 EDT 2014
// Creator: staylor (on Linux ifarm1102 2.6.32-220.7.1.el6.x86_64 x86_64)
//

#ifndef _DTrackFinder_
#define _DTrackFinder_

#include "DVector3.h"
#include "TH1F.h"
#include "TH1I.h"
#include "FDC/DFDCPseudo.h"
#include "CDC/DCDCTrackHit.h"
#include "CDC/DCDCWire.h"
#include "DMatrixSIMD.h"

#include <JANA/JApplication.h>

#include <vector>

class DTrackFinder:public JObject{
   public:
      JOBJECT_PUBLIC(DTrackFinder);

      DTrackFinder(JApplication* app);
      virtual ~DTrackFinder();

      enum state_vector{
         state_x,
         state_y,
         state_tx,
         state_ty,
      };

      class fdc_hit_t{
         public:
            fdc_hit_t(const DFDCPseudo *hit=NULL,bool used=false):
               hit(hit),used(used){}
            const DFDCPseudo *hit;
            bool used;
      };

      class fdc_segment_t{
      public:
	fdc_segment_t(vector<const DFDCPseudo *>&input_hits,
		      bool matched=false){
	  for (unsigned int i=0;i<input_hits.size();i++){
	    this->hits.push_back(input_hits[i]);
	  }
	  this->S=FindStateVector();
	  this->matched=matched;
	};
	~fdc_segment_t(){};
	
	DMatrix4x1 FindStateVector(void) const;
	
	bool matched;
	DMatrix4x1 S;
	vector<const DFDCPseudo *>hits;
	
      };

      class cdc_hit_t{
         public:
            cdc_hit_t(const DCDCTrackHit *hit=NULL,bool used=false):
               hit(hit),used(used){}
            const DCDCTrackHit *hit;
            bool used;
      };

      class cdc_segment_t{
         public:
            cdc_segment_t(vector<const DCDCTrackHit *>&input_hits,const DVector3 &dir,
                  bool matched=false){
               for (unsigned int i=0;i<input_hits.size();i++){
                  this->hits.push_back(input_hits[i]);
               }
               this->dir=dir;
               this->matched=matched;
            };
            ~cdc_segment_t(){};

            bool matched;
            DVector3 dir;
            vector<const DCDCTrackHit *>hits;

      };

      class cdc_track_t{
         public:
            cdc_track_t(vector<const DCDCTrackHit *>myhits){
               for (unsigned int i=0;i<myhits.size();i++){
                  if (myhits[i]->is_stereo)this->stereo_hits.push_back(myhits[i]);
                  else this->axial_hits.push_back(myhits[i]);
               }
            };  
            ~cdc_track_t(){};

            jerror_t FindStateVector(bool IsCosmics=false);

            vector<const DCDCTrackHit *>axial_hits; 
            vector<const DCDCTrackHit *>stereo_hits;
            DVector3 dir;
            double z; // z-position at which S is reported
            DMatrix4x1 S;

      };


      void Reset(void);
      void AddHit(const DCDCTrackHit *hit); 
      void AddHit(const DFDCPseudo *hit);
      bool FindFDCSegments(void);
      bool LinkFDCSegments(void);
      bool FindAxialSegments(void);
      bool LinkCDCSegments(void);
      bool MatchCDCHit(const DVector3 &vhat,const DVector3 &pos0,
            const DCDCTrackHit *hit, double cut);
      bool MatchCDCStereoHit(const DVector3 &tdir,const DVector3 &t0,
            const DCDCTrackHit *hit);

      const vector<cdc_track_t>&GetCDCTracks(void) const {return cdc_tracks;};
      const vector<fdc_segment_t>&GetFDCTracks(void) const {return fdc_tracks;};

      double FindDoca(double z,const DMatrix4x1 &S,const DVector3 &wdir,
            const DVector3 &origin,DVector3 *poca=NULL) const;
      double FindDoca(const DVector3 &pos1,const DVector3 &mom1,
            const DVector3 &pos2,const DVector3 &mom2,
            DVector3 *poca=NULL) const;
      bool FindIntersectionsWithCylinder(double R,const DVector3 &dir,
            const DVector3 &pos,DVector3 &out1,
            DVector3 &out2) const;
      bool FindIntersectionWithPlane(const DVector3 &origin,const DVector3 &norm,
            const DVector3 &pos,const DVector3 &dir,
            DVector3 &outpos) const;


   private:

      TH1F *hCDCMatch_Axial, *hCDCMatch_Stereo, *hCDCMatch_PairD;
      TH1I *hFDCLayer, *hFDCLayerRaw, *hFDCLayerFirst;
      vector<cdc_hit_t>axial_hits;
      vector<cdc_hit_t>stereo_hits;
      vector<cdc_segment_t>axial_segments;
      vector<cdc_track_t>cdc_tracks;

      vector<fdc_hit_t>fdc_hits;
      vector<fdc_segment_t>fdc_segments[4];
      vector<fdc_segment_t>fdc_tracks;

      bool COSMICS, DEBUG_HISTS, VERBOSE;
      double CDC_MATCH_RADIUS,CDC_MATCH_PHI,CDC_COSMIC_MATCH_PHI;
      double FDC_MATCH_RADIUS;
};

#endif // _DTrackFinder_

