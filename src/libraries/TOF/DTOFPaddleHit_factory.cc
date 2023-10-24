// $Id$
//
/*! **File**: DTOFPaddleHit_factory.cc
 *+ Created: Thu Jun  9 10:05:21 EDT 2005
 *+ Creator: davidl (on Darwin wire129.jlab.org 7.8.0 powerpc)
 *+ Purpose: Implementation of DTOFPaddleHit factory creating paddle hits based on matched
 * DTOFHit objects for long paddles that are equiped with PMTs on both ends. For these paddle
 * hits a time-difference and mean-time can be determined. The time difference results in a
 * hit position along the paddle while the mean-time is a measure of the time of flight of the
 * particle from the IP to the paddle.
*/

/// \addtogroup TOFDetector

// Modified: Wed Feb 12 13:19:10 EST 2014 by B. Zihlmann
//           reflect the changes in the TOF geometry with
//           19 LWB, 2 LNB, 2 SB, 2LNB, 19 LWB
//                          2 SB
//           LWB: long wide bars
//           LNB: long narrow bars
//           SB:  short bars
//           the bar numbering goes from 1 all through 46 with
//           bar 22 and 23 are the 4 short bars distinguished by north/south
//

/*! \file DTOFPaddleHit_factory.cc
 * The code file DTOFPaddleHit_factory.cc is the implementation of the factory that generates
 * TOFPaddleHit objects and is based on long paddles with 2-ended readout. 
 * The individual hits from either side come from DTOFHit objects.
 * This factory code is called for every event. The mothod is DTOFPaddleHit_factory::evnt(). 
*/

#include <iostream>
using namespace std;

#include "DTOFPaddleHit_factory.h"
#include "DTOFHit.h"
#include "DTOFHitMC.h"
#include "DTOFPaddleHit.h"
#include <math.h>

//#define NaN std::numeric_limits<double>::quiet_NaN()
#define BuiltInNaN __builtin_nan("")

//------------------
// brun
//------------------
jerror_t DTOFPaddleHit_factory::brun(JEventLoop *loop, int32_t runnumber)
{
  /// Retreive TOF parameters based on the TOF geometry for this run. This includes
  /// values like the number of bars in a plane the length of the bars, the effective
  /// speed of light in the bars and attenuation lengths.  

  DApplication* dapp = dynamic_cast<DApplication*>(loop->GetJApplication());
  const DGeometry *geom = dapp->GetDGeometry(runnumber);
  
  // load values from geometry
  map<string,double> paddle_params;
  geom->GetTOFPaddleParameters(paddle_params);
  TOF_NUM_BARS=paddle_params["NLONGBARS"]+paddle_params["NSHORTBARS"]/2;
  TOF_NUM_PLANES=2; // not likely to change...
  HALFPADDLE=paddle_params["HALFLONGBARLENGTH"];
  FirstShortBar = paddle_params["FIRSTSHORTBAR"];
  LastShortBar = paddle_params["LASTSHORTBAR"];

  map<string, double> tofparms;
  string ccdb_directory_name;
  if(TOF_NUM_BARS == 46) {
    ccdb_directory_name="TOF2";
  } else {
    ccdb_directory_name="TOF";
  }
  string locTOFParmsTable = ccdb_directory_name + "/tof_parms";
  if( !loop->GetCalib(locTOFParmsTable.c_str(), tofparms)) {
    //cout<<"DTOFPaddleHit_factory: loading values from TOF data base"<<endl;

    C_EFFECTIVE    =    tofparms["TOF_C_EFFECTIVE"];
    //HALFPADDLE     =    tofparms["TOF_HALFPADDLE"];
    E_THRESHOLD    =    tofparms["TOF_E_THRESHOLD"];
    ATTEN_LENGTH   =    tofparms["TOF_ATTEN_LENGTH"];
  } else {
    cout << "DTOFPaddleHit_factory: Error loading values from TOF data base" <<endl;

    C_EFFECTIVE = 15.;    // set to some reasonable value
    //HALFPADDLE = 126;     // set to some reasonable value
    E_THRESHOLD = 0.0005; // energy threshold in GeV
    ATTEN_LENGTH = 400.;  // 400cm attenuation length
  }

  TIME_COINCIDENCE_CUT=2.*HALFPADDLE/C_EFFECTIVE;

  string locTOFPropSpeedTable = ccdb_directory_name + "/propagation_speed";
  if(eventLoop->GetCalib(locTOFPropSpeedTable.c_str(), propagation_speed))
    jout << "Error loading " << locTOFPropSpeedTable << " !" << endl;
  string locTOFAttenLengthTable = ccdb_directory_name + "/attenuation_lengths";
  if(eventLoop->GetCalib(locTOFAttenLengthTable.c_str(), AttenuationLengths))
    jout << "Error loading " << locTOFAttenLengthTable << " !" << endl;
  
  return NOERROR;

}

//------------------
// evnt
//------------------
jerror_t DTOFPaddleHit_factory::evnt(JEventLoop *loop, uint64_t eventnumber)
{
  /// General Purpose:
  /// Based on the list of DTOFHit objects this methodes finds hits for paddles
  /// with PMTs on both ends and forms paddles hits. The timing information from
  /// both ends of a paddle can be used to determine the hit position along the paddle
  /// as well as the mean time which is a measure of Time-Of-Flight for the particle 
  /// from the vertex to the detector impact location.
  /// With a hit location the energy depositon can be corrected for attenuation.
  /// For a mathced hit on both ends both ends energy has to be above an energy threshold
  /// E_THRESHOLD (default 0.5MeV) and within a time window TIME_COINCIDENCE_CUT given by
  /// the time for a signal to traverse a full length paddle.

  vector<const DTOFHit*> hits;
  loop->Get(hits,TOF_POINT_TAG.c_str());

  vector<const DTOFHit*> P1hitsL;
  vector<const DTOFHit*> P1hitsR;
  vector<const DTOFHit*> P2hitsL;
  vector<const DTOFHit*> P2hitsR;

  //int P1L[100];
  //int P1R[100];
  //int P2L[100];
  //int P2R[100];

  //int c1l = 0;
  //int c1r = 0;
  //int c2l = 0;
  //int c2r = 0;

  // sort the tof hits into left and right PMTs for both planes


  /// First Loop over DTOFHits and sort them into lists accroding to Left and Right 
  /// for both planes individually. This results in four separate lists of DTOFHit objects.

  for (unsigned int i = 0; i < hits.size(); i++){
    const DTOFHit *hit = hits[i];
    if (hit->has_fADC && hit->has_TDC){ // good hits have both ADC and TDC info
      if (hit->plane){
	if (hit->end){
	  P2hitsR.push_back(hit);
	  //P2R[c2r++] = i;
	} else {
	  P2hitsL.push_back(hit);	
	  //P2L[c2l++] = i;
	}
      } else {
	if (hit->end){
	  P1hitsR.push_back(hit);
	  //P1R[c1r++] = i;
	} else {
	  P1hitsL.push_back(hit);
	  //P1L[c1l++] = i;
	}
      }
    }
  }

  // find matching Up/Down TOFHits
  ///
  /// Now loop over the hits of "TOP" PMT hits which are the hits in the plane
  /// with vertically oriented paddles the PMTs at the top. All short paddles will
  /// be ignored.
  /// For each found hit a match is seeked in the list of "BOTTOM" PMT hits with the
  /// requirement that both ends have a signal over threhold (default is 0.5MeV) and 
  /// both hits have a timing not further appart than a value given by TOF_COINCIDENCE_CUT
  /// which is deduced by the total length of the paddle and the effective speed of light.
  for (unsigned int i=0; i<P1hitsL.size(); i++){

    int bar = P1hitsL[i]->bar;

    if ((bar < FirstShortBar) || (bar > LastShortBar)) {
      
      // we are dealing with double ended readout paddles:
      for (unsigned int j=0; j<P1hitsR.size(); j++){      
	if (bar==P1hitsR[j]->bar 
	    && fabs(P1hitsR[j]->t-P1hitsL[i]->t)<TIME_COINCIDENCE_CUT
	    && (P1hitsL[i]->dE>E_THRESHOLD || P1hitsR[j]->dE>E_THRESHOLD)){
	  DTOFPaddleHit *hit = new DTOFPaddleHit;
	  hit->bar = bar;
	  hit->orientation   = P1hitsL[i]->plane;
	  hit->E_north = P1hitsL[i]->dE;
	  hit->t_north = P1hitsL[i]->t;
	  hit->AddAssociatedObject(P1hitsL[i]);
	  hit->E_south = P1hitsR[j]->dE;
	  hit->t_south = P1hitsR[j]->t;      
	  hit->AddAssociatedObject(P1hitsR[j]);  

	  _data.push_back(hit);
	}
      }
    } 
  }
  

  // find Full length bar with not corresponding hit on the right side
  ///
  /// In case no match is found a DTOFPaddleHit object is created with only one
  /// end having its information loaded while the values of other end are initialized to zero.
  /// An energy threhold for the signal is still required (default 0.5MeV)
  for (unsigned int i=0; i<P1hitsL.size(); i++){ 
    int bar = P1hitsL[i]->bar;
    int found = 0;
    
    if ((bar < FirstShortBar) || (bar > LastShortBar)) {
      for (unsigned int j=0; j<P1hitsR.size(); j++){      
	if (bar==P1hitsR[j]->bar){
	  found = 1;
	}
      }
    }

    if (!found){
      if (P1hitsL[i]->dE>E_THRESHOLD){
	DTOFPaddleHit *hit = new DTOFPaddleHit;
	hit->bar = bar;
	hit->orientation   = P1hitsL[i]->plane;
	hit->E_north = P1hitsL[i]->dE;
	hit->t_north = P1hitsL[i]->t;
	hit->E_south = 0.;
	hit->t_south = 0.;  
	hit->AddAssociatedObject(P1hitsL[i]);

	_data.push_back(hit);
      }
    }
  }

  // find full length bar with no corresponding hit on the left side
  for (unsigned int i=0; i<P1hitsR.size(); i++){   
    int bar = P1hitsR[i]->bar;
    int found = 0;

    if ((bar < FirstShortBar) || (bar > LastShortBar)) {
      for (unsigned int j=0; j<P1hitsL.size(); j++){      
	if (bar==P1hitsL[j]->bar){
	  found = 1;
	}
      }
    }

    if (!found){
      if (P1hitsR[i]->dE>E_THRESHOLD){
	DTOFPaddleHit *hit = new DTOFPaddleHit;
	hit->bar = bar;
	hit->orientation   = P1hitsR[i]->plane;
	hit->E_south = P1hitsR[i]->dE;
	hit->t_south = P1hitsR[i]->t;
	hit->E_north = 0.;
	hit->t_north = 0.;      
	hit->AddAssociatedObject(P1hitsR[i]);
	
	_data.push_back(hit);
      }
    }
  }
  

  // now the same thing for plane 2
  for (unsigned int i=0; i<P2hitsL.size(); i++){
    int bar = P2hitsL[i]->bar; 
    if ((bar <  FirstShortBar) || (bar > LastShortBar )){
      for (unsigned int j=0; j<P2hitsR.size(); j++){      
	if (bar==P2hitsR[j]->bar 
	    && fabs(P2hitsR[j]->t-P2hitsL[i]->t)<TIME_COINCIDENCE_CUT
	    && (P2hitsL[i]->dE>E_THRESHOLD || P2hitsR[j]->dE>E_THRESHOLD)){
	  DTOFPaddleHit *hit = new DTOFPaddleHit;
	  hit->bar = bar;
	  hit->orientation   = P2hitsL[i]->plane;
	  hit->E_north = P2hitsL[i]->dE;
	  hit->t_north = P2hitsL[i]->t;
	  hit->AddAssociatedObject(P2hitsL[i]);
	  hit->E_south = P2hitsR[j]->dE;
	  hit->t_south = P2hitsR[j]->t;      
	  hit->AddAssociatedObject(P2hitsR[j]);
	  
	  _data.push_back(hit);
	}
      }
    }
  }
  


  // Plane 2 full length paddles with hit only on the left
  for (unsigned int i=0; i<P2hitsL.size(); i++){   
    int bar = P2hitsL[i]->bar;
    int found = 0;
    
    if ((bar < FirstShortBar) || (bar > LastShortBar)) {
      for (unsigned int j=0; j<P2hitsR.size(); j++){      
	if (bar==P2hitsR[j]->bar){
	  found = 1;
	}
      }
    }
    
    if (!found){
      if (P2hitsL[i]->dE>E_THRESHOLD){
	DTOFPaddleHit *hit = new DTOFPaddleHit;
	hit->bar = bar;
	hit->orientation   = P2hitsL[i]->plane;
	hit->E_north = P2hitsL[i]->dE;
	hit->t_north = P2hitsL[i]->t;
	hit->E_south = 0.;
	hit->t_south = 0.;      
	hit->AddAssociatedObject(P2hitsL[i]);

	_data.push_back(hit);
      }
    }
  }

  // Plane 2 with full length paddle and hit only on the right.
  for (unsigned int i=0; i<P2hitsR.size(); i++){   
    int bar = P2hitsR[i]->bar;
    int found = 0;

    if ((bar < FirstShortBar) || (bar > LastShortBar)) {
      for (unsigned int j=0; j<P2hitsL.size(); j++){      
	if (bar==P2hitsL[j]->bar){
	  found = 1;
	}
      }
    }

    if (!found){
       if (P2hitsR[i]->dE>E_THRESHOLD){
	DTOFPaddleHit *hit = new DTOFPaddleHit;
	hit->bar = bar;
	hit->orientation   = P2hitsR[i]->plane;
	hit->E_south = P2hitsR[i]->dE;
	hit->t_south = P2hitsR[i]->t;
	hit->E_north = 0.;
	hit->t_north = 0.;      
	hit->AddAssociatedObject(P2hitsR[i]);

	_data.push_back(hit);
      }
    }
  }


  for (int i=0;i<(int)_data.size(); i++) {
    
    DTOFPaddleHit *hit = _data[i];
    
    int check = -1;
    if (hit->E_north > E_THRESHOLD) {
      check++;
    }
    if (hit->E_south > E_THRESHOLD) {
      check++;
    }
    
    if (check > 0 ){

      int id=TOF_NUM_BARS*hit->orientation+hit->bar-1;
      double v=propagation_speed[id];
      hit->meantime = (hit->t_north+hit->t_south)/2. - HALFPADDLE/v;
      hit->timediff = (hit->t_south - hit->t_north)/2.;
      float pos = hit->timediff * v;  
      hit->pos = pos;
      hit->dpos      = 2.;  // manually/artificially set to 2cm. 
      
      float xl =  pos; // distance to left PMT 
      float xr =  pos; // distance to right PMT
      int idl = hit->orientation*TOF_NUM_PLANES*TOF_NUM_BARS + hit->bar-1;
      int idr = idl+TOF_NUM_BARS;
      float d1 = AttenuationLengths[idl][0];
      float d2 = AttenuationLengths[idl][1];

      // reference distance is 144cm from PMT
      // if pos==0 (center) no change in dE is required => att_i = 1.
      // so change ref. distance to HALFPADDLE
      //float att_left = ( TMath::Exp(-144./d1) +  TMath::Exp(-144./d2)) / 
      //( TMath::Exp(-xl/d1) +  TMath::Exp(-xl/d2));
      // adc2E values in DTOFHit_factory are for pos=0

      float att_left = (TMath::Exp(-xl/d1) +  TMath::Exp(-xl/d2))/2.;

      d1 = AttenuationLengths[idr][0];
      d2 = AttenuationLengths[idr][1];
      //float att_right = ( TMath::Exp(-144./d1) +  TMath::Exp(-144./d2)) / 
      //( TMath::Exp(-xr/d1) +  TMath::Exp(-xr/d2));

      float att_right = ( TMath::Exp(xr/d1) +  TMath::Exp(xr/d2))/2.;

      hit->dE = (hit->E_north*att_left + hit->E_south*att_right)/2.;
    } else {
      hit->meantime = BuiltInNaN;
      hit->timediff = BuiltInNaN;
      hit->pos = BuiltInNaN;
      hit->dpos = BuiltInNaN;
      hit->dE = BuiltInNaN;
   }

  }
  
  return NOERROR;
}

