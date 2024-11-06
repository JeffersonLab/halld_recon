// $Id$
//
/*! **File**: DTOFPoint_factory.cc
 *+ Created: Tue Oct 18 09:50:52 EST 2005
 *+ Creator: remitche (on Linux mantrid00 2.4.20-18.8smp i686)
 *+ Purpose: Implementation of the DTOFPoint_factory creating DTOFHitPoints based on 
 * DTOFPaddleHit objects from both planes (vertical and horizontal) forming space points.
*/

/// \addtogroup TOFDetector

// Modified: Wed Feb 12 13:23:42 EST 2014 B. Zihlamnn
//					 use new TOF geometry with narrow long paddles
//					 and short paddles #22 and #23 for both north and south
//					 

#include <cassert>
#include <cmath>
using namespace std;

#include <JANA/JEvent.h>
#include <JANA/Calibrations/JCalibrationManager.h>

#include "DTOFPoint_factory.h"
#include <DANA/DApplication.h>
#include <HDGEOMETRY/DGeometry.h>

bool Compare_TOFSpacetimeHitMatches_Distance(const DTOFPoint_factory::tof_spacetimehitmatch_t& locTOFSpacetimeHitMatch1, const DTOFPoint_factory::tof_spacetimehitmatch_t& locTOFSpacetimeHitMatch2)
{
	if(locTOFSpacetimeHitMatch2.dBothPositionsWellDefinedFlag != locTOFSpacetimeHitMatch1.dBothPositionsWellDefinedFlag)
		return locTOFSpacetimeHitMatch1.dBothPositionsWellDefinedFlag; //one hit position is well defined and the other is not
	return (locTOFSpacetimeHitMatch1.delta_r < locTOFSpacetimeHitMatch2.delta_r);
};

bool Compare_TOFPoint_Time(const DTOFPoint *locTOFPoint1, const DTOFPoint *locTOFPoint2) {
  return locTOFPoint1->t < locTOFPoint2->t;
}

//------------------
// BeginRun
//------------------
void DTOFPoint_factory::BeginRun(const std::shared_ptr<const JEvent>& event)
{
  DApplication* dapp = dynamic_cast<DApplication*>(loop->GetJApplication());
  const DGeometry *geom = dapp->GetDGeometry(runnumber);

  // load values from geometry 
  vector<double> YWIDTH;  ///> y (perpendicular) bar width per bar number	
  geom->GetTOFPaddlePerpPositions(YPOS, YWIDTH);
  geom->GetTOFZ(CenterVertPlane,CenterHorizPlane,CenterMidPlane);

  map<string,double> paddle_params;
  geom->GetTOFPaddleParameters(paddle_params);
 
  FirstShortBar = paddle_params["FIRSTSHORTBAR"];
  LastShortBar = paddle_params["LASTSHORTBAR"]; 
  HALFPADDLE = paddle_params["HALFLONGBARLENGTH"];
  HALFPADDLE_ONESIDED = paddle_params["HALFSHORTBARLENGTH"];
  double locBeamHoleWidth = paddle_params["LONGBARLENGTH"]
    - 2.0*paddle_params["SHORTBARLENGTH"];
  ONESIDED_PADDLE_MIDPOINT_MAG = HALFPADDLE_ONESIDED + locBeamHoleWidth/2.0;
  NUM_BARS = paddle_params["NLONGBARS"]+paddle_params["NSHORTBARS"]/2;

  string ccdb_directory_name;
  if (NUM_BARS == 46) {
    ccdb_directory_name="TOF2";
  } else {
    ccdb_directory_name="TOF";
  }

  map<string, double> tofparms;
  string locTOFParmsTable = ccdb_directory_name + "/tof_parms";
  if( !loop->GetCalib(locTOFParmsTable.c_str(), tofparms))
    {
      //cout<<"DTOFPoint_factory: loading values from TOF data base"<<endl;
      //HALFPADDLE = tofparms["TOF_HALFPADDLE"];
      E_THRESHOLD = tofparms["TOF_E_THRESHOLD"];
      ATTEN_LENGTH = tofparms["TOF_ATTEN_LENGTH"];
    }
  else
    {
      cout << "DTOFPoint_factory: Error loading values from TOF data base" <<endl;
      //HALFPADDLE = 126; // set to some reasonable value
      E_THRESHOLD = 0.0005;
      ATTEN_LENGTH = 400.;
    }
  
  string locTOFPropSpeedTable = ccdb_directory_name + "/propagation_speed";
  if(eventLoop->GetCalib(locTOFPropSpeedTable.c_str(), propagation_speed))
    jout << "Error loading " << locTOFPropSpeedTable << " !" << endl;
  string locTOFPaddleResolTable = ccdb_directory_name + "/paddle_resolutions";
  if(eventLoop->GetCalib(locTOFPaddleResolTable.c_str(), paddle_resolutions))
    jout << "Error loading " << locTOFPaddleResolTable << " !" << endl;
 
  // for applying attentuation to half lenfgth paddles
  string locTOFAttenLengthTable = ccdb_directory_name + "/attenuation_lengths";
  if(eventLoop->GetCalib(locTOFAttenLengthTable.c_str(), AttenuationLengths))
    jout << "Error loading " << locTOFAttenLengthTable << " !" << endl;
    
  dPositionMatchCut_DoubleEnded = 9.0; //1.5*BARWIDTH
  //	dTimeMatchCut_PositionWellDefined = 1.0;
  dTimeMatchCut_PositionWellDefined = 10.0;
  dTimeMatchCut_PositionNotWellDefined = 10.0;
}

DTOFPoint_factory::tof_spacetimehit_t* DTOFPoint_factory::Get_TOFSpacetimeHitResource(void)
{
  tof_spacetimehit_t* locTOFSpacetimeHit;
  if(dTOFSpacetimeHitPool_Available.empty())
    {
      locTOFSpacetimeHit = new tof_spacetimehit_t;
      dTOFSpacetimeHitPool_All.push_back(locTOFSpacetimeHit);
    }
  else
    {
      locTOFSpacetimeHit = dTOFSpacetimeHitPool_Available.back();
      dTOFSpacetimeHitPool_Available.pop_back();
    }
  return locTOFSpacetimeHit;
}

//------------------
// Process
//------------------
void DTOFPoint_factory::Process(const std::shared_ptr<const JEvent>& event)
{	
  // delete pool size if too large, preventing memory-leakage-like behavor.
  if(dTOFSpacetimeHitPool_All.size() > MAX_TOFSpacetimeHitPoolSize)
    {
      for(size_t loc_i = MAX_TOFSpacetimeHitPoolSize; loc_i < dTOFSpacetimeHitPool_All.size(); ++loc_i)
	delete dTOFSpacetimeHitPool_All[loc_i];
      dTOFSpacetimeHitPool_All.resize(MAX_TOFSpacetimeHitPoolSize);
    }
  dTOFSpacetimeHitPool_Available = dTOFSpacetimeHitPool_All;
  
  vector<const DTOFPaddleHit*> locTOFHitVector;
  event->Get(locTOFHitVector);
  
  // create the hit spacetime information
  deque<tof_spacetimehit_t*> locTOFSpacetimeHits_Horizontal, locTOFSpacetimeHits_Vertical;
  set<tof_spacetimehit_t*> locUnusedTOFSpacetimeHits;
  for(size_t loc_i = 0; loc_i < locTOFHitVector.size(); ++loc_i)
    {
      const DTOFPaddleHit* locTOFHit = locTOFHitVector[loc_i];
      if(!((locTOFHit->E_north > E_THRESHOLD) || (locTOFHit->E_south > E_THRESHOLD)))
	continue;
      
      if(locTOFHit->orientation) //horizontal
	{
	  tof_spacetimehit_t* locSpacetimeHit = Build_TOFSpacetimeHit_Horizontal(locTOFHit);
	  locTOFSpacetimeHits_Horizontal.push_back(locSpacetimeHit);
	  locUnusedTOFSpacetimeHits.insert(locSpacetimeHit);
	}
      else //vertical
	{
	  tof_spacetimehit_t* locSpacetimeHit = Build_TOFSpacetimeHit_Vertical(locTOFHit);
	  locTOFSpacetimeHits_Vertical.push_back(locSpacetimeHit);
	  locUnusedTOFSpacetimeHits.insert(locSpacetimeHit);
	}
    }
  
  
  //find matches between planes and sort them by delta-r
  deque<tof_spacetimehitmatch_t> locTOFSpacetimeHitMatches; //use list for sorting, vector for resource pool
  for(size_t loc_i = 0; loc_i < locTOFSpacetimeHits_Horizontal.size(); ++loc_i)
    {
      tof_spacetimehit_t* locTOFSpacetimeHit_Horizontal = locTOFSpacetimeHits_Horizontal[loc_i];
      for(size_t loc_j = 0; loc_j < locTOFSpacetimeHits_Vertical.size(); ++loc_j)
	{
	  tof_spacetimehit_t* locTOFSpacetimeHit_Vertical = locTOFSpacetimeHits_Vertical[loc_j];
	  
	  tof_spacetimehitmatch_t locTOFSpacetimeHitMatch;
	  if(!Match_Hits(locTOFSpacetimeHit_Horizontal, locTOFSpacetimeHit_Vertical, locTOFSpacetimeHitMatch))
	    continue; //not a match
	  
	  locTOFSpacetimeHitMatches.push_back(locTOFSpacetimeHitMatch);
	}
    }
  std::sort(locTOFSpacetimeHitMatches.begin(), locTOFSpacetimeHitMatches.end(), Compare_TOFSpacetimeHitMatches_Distance); //sort matches by delta_r
  
  
  // create DTOFPoints, in order of best matches (by delta_r)
  for(size_t loc_i = 0; loc_i < locTOFSpacetimeHitMatches.size(); ++loc_i)
    {
      tof_spacetimehit_t* locTOFSpacetimeHit_Horizontal = locTOFSpacetimeHitMatches[loc_i].dTOFSpacetimeHit_Horizontal;
      if(locUnusedTOFSpacetimeHits.find(locTOFSpacetimeHit_Horizontal) == locUnusedTOFSpacetimeHits.end())
	continue; //hit used in a previous successful match
      
      tof_spacetimehit_t* locTOFSpacetimeHit_Vertical = locTOFSpacetimeHitMatches[loc_i].dTOFSpacetimeHit_Vertical;
      if(locUnusedTOFSpacetimeHits.find(locTOFSpacetimeHit_Vertical) == locUnusedTOFSpacetimeHits.end())
	continue; //hit used in a previous successful match
      
      Create_MatchedTOFPoint(locTOFSpacetimeHit_Horizontal, locTOFSpacetimeHit_Vertical);
      
      //remove used hits from the unused list
      locUnusedTOFSpacetimeHits.erase(locTOFSpacetimeHit_Horizontal);
      locUnusedTOFSpacetimeHits.erase(locTOFSpacetimeHit_Vertical);
    }
  
  // Loop over unused/unmatched TOF Spacetime hits, and create separate DTOFPoint's for them
  set<tof_spacetimehit_t*>::iterator locSetIterator = locUnusedTOFSpacetimeHits.begin();
  for(; locSetIterator != locUnusedTOFSpacetimeHits.end(); ++locSetIterator)
    Create_UnMatchedTOFPoint(*locSetIterator);
  
  // make sure all the hits are sorted by time (why not?)
  // this helps with reproducibiliy problems...
  std::sort(mData.begin(), mData.end(), Compare_TOFPoint_Time);
}

DTOFPoint_factory::tof_spacetimehit_t* DTOFPoint_factory::Build_TOFSpacetimeHit_Horizontal(const DTOFPaddleHit* locTOFHit)
{
  tof_spacetimehit_t* locTOFSpacetimeHit = Get_TOFSpacetimeHitResource();
  locTOFSpacetimeHit->TOFHit = locTOFHit;
  
  int bar = locTOFHit->bar;
  int id = NUM_BARS + locTOFHit->bar - 1;
  double v = propagation_speed[id];
  
  if((locTOFHit->bar < FirstShortBar) || (locTOFHit->bar >LastShortBar)) //double-ended bars
    {
      locTOFSpacetimeHit->dIsDoubleEndedBar = true;
      locTOFSpacetimeHit->dIsSingleEndedNorthPaddle = false;
      locTOFSpacetimeHit->y = bar2y(bar);
      if(locTOFHit->meantime != locTOFHit->meantime)
	{
	  //NaN: only one energy hit above threshold on the double-ended bar
	  locTOFSpacetimeHit->dPositionWellDefinedFlag = false;
	  locTOFSpacetimeHit->x = 0.0;
	  if(locTOFHit->E_north > E_THRESHOLD)
	    locTOFSpacetimeHit->t = locTOFHit->t_north - HALFPADDLE/v;
	  else
	    locTOFSpacetimeHit->t = locTOFHit->t_south - HALFPADDLE/v;
	  
	  locTOFSpacetimeHit->pos_cut = 1000.0;
	  locTOFSpacetimeHit->t_cut = dTimeMatchCut_PositionNotWellDefined;
	}
      else
	{
	  locTOFSpacetimeHit->dPositionWellDefinedFlag = true;
	  locTOFSpacetimeHit->x = locTOFHit->pos;
	  locTOFSpacetimeHit->t = locTOFHit->meantime;
	  locTOFSpacetimeHit->pos_cut = dPositionMatchCut_DoubleEnded;
	  locTOFSpacetimeHit->t_cut = dTimeMatchCut_PositionWellDefined;
	}
      
      //printf("h: x %f y %f\n",locTOFSpacetimeHit->x,locTOFSpacetimeHit->y);
      return locTOFSpacetimeHit;
    }
  
  //single-ended bars
  locTOFSpacetimeHit->dIsDoubleEndedBar = false;
  locTOFSpacetimeHit->dPositionWellDefinedFlag = false;
  locTOFSpacetimeHit->pos_cut = 1000.0;
  locTOFSpacetimeHit->t_cut = dTimeMatchCut_PositionNotWellDefined;
  
  if(locTOFHit->t_south != 0.)
    {
      locTOFSpacetimeHit->dIsSingleEndedNorthPaddle = false;
      locTOFSpacetimeHit->y = bar2y(bar,1);
      locTOFSpacetimeHit->x = -1.0*ONESIDED_PADDLE_MIDPOINT_MAG;
      locTOFSpacetimeHit->t = locTOFHit->t_south - HALFPADDLE_ONESIDED/v;
    }
  else
    {
      locTOFSpacetimeHit->dIsSingleEndedNorthPaddle = true;
      locTOFSpacetimeHit->y = bar2y(bar,0);
      locTOFSpacetimeHit->x = ONESIDED_PADDLE_MIDPOINT_MAG;
      locTOFSpacetimeHit->t = locTOFHit->t_north - HALFPADDLE_ONESIDED/v;
    }
  
  //printf("h: x %f y %f\n",locTOFSpacetimeHit->x,locTOFSpacetimeHit->y);
  return locTOFSpacetimeHit;
}

DTOFPoint_factory::tof_spacetimehit_t* DTOFPoint_factory::Build_TOFSpacetimeHit_Vertical(const DTOFPaddleHit* locTOFHit)
{
  tof_spacetimehit_t* locTOFSpacetimeHit = Get_TOFSpacetimeHitResource();
  locTOFSpacetimeHit->TOFHit = locTOFHit;
  
  int bar = locTOFHit->bar;
  int id = locTOFHit->bar - 1;
  double v = propagation_speed[id];
  
  if((locTOFHit->bar < FirstShortBar) || (locTOFHit->bar > LastShortBar))
    {
      //double-ended bars
      locTOFSpacetimeHit->dIsDoubleEndedBar = true;
      locTOFSpacetimeHit->dIsSingleEndedNorthPaddle = false;
      locTOFSpacetimeHit->x = bar2y(bar);
      if(locTOFHit->meantime != locTOFHit->meantime)
	{
	  //NaN: only one energy hit above threshold on the double-ended bar
	  locTOFSpacetimeHit->dPositionWellDefinedFlag = false;
	  locTOFSpacetimeHit->y = 0.0;
	  if(locTOFHit->E_north > E_THRESHOLD)
	    locTOFSpacetimeHit->t = locTOFHit->t_north - HALFPADDLE/v;
	  else
	    locTOFSpacetimeHit->t = locTOFHit->t_south - HALFPADDLE/v;
	  locTOFSpacetimeHit->pos_cut = 1000.0;
	  locTOFSpacetimeHit->t_cut = dTimeMatchCut_PositionNotWellDefined;
	}
      else
	{
	  locTOFSpacetimeHit->dPositionWellDefinedFlag = true;
	  locTOFSpacetimeHit->y = locTOFHit->pos;
	  locTOFSpacetimeHit->t = locTOFHit->meantime;
	  locTOFSpacetimeHit->pos_cut = dPositionMatchCut_DoubleEnded;
	  locTOFSpacetimeHit->t_cut = dTimeMatchCut_PositionWellDefined;
	}
      
      //printf("h: x %f y %f\n",locTOFSpacetimeHit->x,locTOFSpacetimeHit->y);
      return locTOFSpacetimeHit;
    }
  
  //single-ended bars
  locTOFSpacetimeHit->dIsDoubleEndedBar = false;
  locTOFSpacetimeHit->dPositionWellDefinedFlag = false;
  locTOFSpacetimeHit->pos_cut = 1000.0;
  locTOFSpacetimeHit->t_cut = dTimeMatchCut_PositionNotWellDefined;
  if(locTOFHit->t_south != 0.)
    {
      locTOFSpacetimeHit->dIsSingleEndedNorthPaddle = false;
      locTOFSpacetimeHit->x = bar2y(bar,0);
      locTOFSpacetimeHit->y = -1.0*ONESIDED_PADDLE_MIDPOINT_MAG;
      locTOFSpacetimeHit->t = locTOFHit->t_south - HALFPADDLE_ONESIDED/v;
    }
  else
    {
      locTOFSpacetimeHit->dIsSingleEndedNorthPaddle = true;
      locTOFSpacetimeHit->x = bar2y(bar,1);
      locTOFSpacetimeHit->y = ONESIDED_PADDLE_MIDPOINT_MAG;
      locTOFSpacetimeHit->t = locTOFHit->t_north - HALFPADDLE_ONESIDED/v;
    }
  
  //printf("h: x %f y %f\n",locTOFSpacetimeHit->x,locTOFSpacetimeHit->y);
  return locTOFSpacetimeHit;
}

bool DTOFPoint_factory::Match_Hits(tof_spacetimehit_t* locTOFSpacetimeHit_Horizontal, tof_spacetimehit_t* locTOFSpacetimeHit_Vertical, tof_spacetimehitmatch_t& locTOFSpacetimeHitMatch)
{
  //make sure that single-ended paddles don't match each other
  if((!locTOFSpacetimeHit_Vertical->dIsDoubleEndedBar) && (!locTOFSpacetimeHit_Horizontal->dIsDoubleEndedBar))
    return false; //unphysical
  
  //make sure that (e.g. horizontal) single-ended paddles don't match (e.g. vertical) paddles on the opposite side
  if(!locTOFSpacetimeHit_Horizontal->dIsDoubleEndedBar)
    {
      //horizontal is single-ended
      if(locTOFSpacetimeHit_Horizontal->dIsSingleEndedNorthPaddle)
	{
	  //horizontal is on north (+x) side
	  if(locTOFSpacetimeHit_Vertical->TOFHit->bar < FirstShortBar)
	    return false; //vertical is on south (-x) side: CANNOT MATCH
	}
      else
	{
	  //horizontal is on south (-x) side
	  if(locTOFSpacetimeHit_Vertical->TOFHit->bar > LastShortBar)
	    return false; //vertical is on north (+x) side: CANNOT MATCH
	}
    }
  else if(!locTOFSpacetimeHit_Vertical->dIsDoubleEndedBar)
    {
      //vertical is single-ended
      if(locTOFSpacetimeHit_Vertical->dIsSingleEndedNorthPaddle)
	{
	  //vertical is on north (+y) side
	  if(locTOFSpacetimeHit_Horizontal->TOFHit->bar < FirstShortBar)
	    return false; //horizontal is on south (-y) side: CANNOT MATCH
	}
      else
	{
	  //vertical is on south (-y) side
	  if(locTOFSpacetimeHit_Horizontal->TOFHit->bar > LastShortBar)
	    return false; //horizontal is on north (+y) side: CANNOT MATCH
	}
    }
  
  //If the position along BOTH paddles is not well defined, cannot tell whether these hits match or not
  //If the hit multiplicity was low, could just generate all matches
  //However, the hit multiplicity is generally very high due to hits near the beamline: would have TOF hits everywhere
  //Therefore, don't keep as match: register separately, let track / tof matching salvage the situation
  if((!locTOFSpacetimeHit_Horizontal->dPositionWellDefinedFlag) && (!locTOFSpacetimeHit_Vertical->dPositionWellDefinedFlag))
    return false; //have no idea whether these hits go together: assume they don't
  
  float locDeltaX = locTOFSpacetimeHit_Horizontal->x - locTOFSpacetimeHit_Vertical->x;
  if(fabs(locDeltaX) > locTOFSpacetimeHit_Horizontal->pos_cut)
    return false;
  
  float locDeltaY = locTOFSpacetimeHit_Horizontal->y - locTOFSpacetimeHit_Vertical->y;
  if(fabs(locDeltaY) > locTOFSpacetimeHit_Vertical->pos_cut)
    return false;
  
  float locDeltaT = locTOFSpacetimeHit_Horizontal->t - locTOFSpacetimeHit_Vertical->t;
  float locTimeCut = (locTOFSpacetimeHit_Horizontal->t_cut > locTOFSpacetimeHit_Vertical->t_cut) ? locTOFSpacetimeHit_Horizontal->t_cut : locTOFSpacetimeHit_Vertical->t_cut;
  if(fabs(locDeltaT) > locTimeCut)
    return false;
  
  locTOFSpacetimeHitMatch.delta_t = locDeltaT;
  locTOFSpacetimeHitMatch.delta_r = sqrt(locDeltaX*locDeltaX + locDeltaY*locDeltaY);
  locTOFSpacetimeHitMatch.dTOFSpacetimeHit_Horizontal = locTOFSpacetimeHit_Horizontal;
  locTOFSpacetimeHitMatch.dTOFSpacetimeHit_Vertical = locTOFSpacetimeHit_Vertical;
  locTOFSpacetimeHitMatch.dBothPositionsWellDefinedFlag = (locTOFSpacetimeHit_Horizontal->dPositionWellDefinedFlag == locTOFSpacetimeHit_Vertical->dPositionWellDefinedFlag);
  
  return true;
}

void DTOFPoint_factory::Create_MatchedTOFPoint(const tof_spacetimehit_t* locTOFSpacetimeHit_Horizontal, const tof_spacetimehit_t* locTOFSpacetimeHit_Vertical)
{
  const DTOFPaddleHit* locTOFHit_Horizontal = locTOFSpacetimeHit_Horizontal->TOFHit;
  const DTOFPaddleHit* locTOFHit_Vertical = locTOFSpacetimeHit_Vertical->TOFHit;
  
  int id_vert = locTOFHit_Vertical->bar - 1;
  int id_horiz = NUM_BARS + locTOFHit_Horizontal->bar - 1;
  double locVMatchTErr = paddle_resolutions[id_vert];   // paddle time resolutions
  double locHMatchTErr = paddle_resolutions[id_horiz];
  double locMatchTErr = 0.;    
  
  //reconstruct TOF hit information, using information from the best bar: one or both of the bars may have a PMT signal below threshold
  float locMatchX, locMatchY, locMatchZ, locMatchdE, locMatchT, locMatchdE1, locMatchdE2;
  if(locTOFSpacetimeHit_Horizontal->dPositionWellDefinedFlag && locTOFSpacetimeHit_Vertical->dPositionWellDefinedFlag)
    {
      //both bars have both PMT signals above threshold
      //is x/y resolution from energy calibration better than x/y resolution from paddle edges?
      locMatchX = locTOFSpacetimeHit_Horizontal->x;
      locMatchY = locTOFSpacetimeHit_Vertical->y;
      locMatchZ = CenterMidPlane; //z: midpoint between tof planes
      locMatchT = 0.5*(locTOFSpacetimeHit_Horizontal->t + locTOFSpacetimeHit_Vertical->t);
      locMatchdE = 0.5*(locTOFHit_Horizontal->dE + locTOFHit_Vertical->dE);
      locMatchTErr = 0.5 * sqrt(locVMatchTErr*locVMatchTErr + locHMatchTErr*locHMatchTErr);
      locMatchdE1 = locTOFHit_Vertical->dE;
      locMatchdE2 = locTOFHit_Horizontal->dE;
      //cout<<"dE / dE1  /dE2 == "<<locMatchdE<<" / "<<locMatchdE1<<" / "<<locMatchdE2<<endl;

    }
  else if(locTOFSpacetimeHit_Horizontal->dPositionWellDefinedFlag)
    {
      //the vertical position from the vertical paddle is not well defined
      locMatchX = locTOFSpacetimeHit_Horizontal->x;
      locMatchY = locTOFSpacetimeHit_Horizontal->y;
      locMatchT = locTOFSpacetimeHit_Horizontal->t;
      locMatchZ = CenterHorizPlane; //z: center of horizontal plane
      locMatchdE = locTOFHit_Horizontal->dE;
      locMatchTErr = locHMatchTErr;
      locMatchdE2 = locTOFHit_Horizontal->dE;

      std::vector<const DTOFHit*> loctofhits;
      locTOFHit_Vertical->Get(loctofhits);
      const DTOFHit* loctofhit = 0;
      for (auto hit : loctofhits) {
        if ( (locMatchY > 0 && hit->end == 0) ||
             (locMatchY < 0 && hit->end == 1) )
        {
           loctofhit = hit;
        }
      }
      if (loctofhit) {
        int id = loctofhit->plane*NUM_BARS*2 + loctofhit->bar-1 + loctofhit->end*NUM_BARS;
        float atten1 = AttenuationLengths[id][0];
        float atten2 = AttenuationLengths[id][1];
        float locd = TMath::Abs(locMatchY) - 15.; // only attenuate close to end of paddle not to center.
        float corr = 1;
        if (locd>0){
          corr = (TMath::Exp(-locd/atten1) + TMath::Exp(-locd/atten2))/2.;
        }
        locMatchdE1 = loctofhit->dE * corr;
        //cout<<"Vertical: dE1 "<<locMatchdE1<<" = "<<loctofhit->dE<<"  *  "<<corr<<endl;
      }
      else {
         locMatchdE1 = 0;
      }
    }
  else if(locTOFSpacetimeHit_Vertical->dPositionWellDefinedFlag)
    {
      //the horizontal position from the horizontal paddle is not well defined
      locMatchX = locTOFSpacetimeHit_Vertical->x;
      locMatchY = locTOFSpacetimeHit_Vertical->y;
      locMatchT = locTOFSpacetimeHit_Vertical->t;
      locMatchZ = CenterVertPlane; //z: center of vertical plane
      locMatchdE = locTOFHit_Vertical->dE;
      locMatchTErr = locVMatchTErr;
      locMatchdE1 = locTOFHit_Vertical->dE;

      std::vector<const DTOFHit*> loctofhits;
      locTOFHit_Horizontal->Get(loctofhits);
      const DTOFHit* loctofhit = 0;
      for (auto hit : loctofhits) {
        if ( (locMatchX > 0 && hit->end == 0) ||
             (locMatchX < 0 && hit->end == 1) )
        {
           loctofhit = hit;
        }
      }
      if (loctofhit) {
        int id = loctofhit->plane*NUM_BARS*2 + loctofhit->bar-1 + loctofhit->end*NUM_BARS;
        float atten1 = AttenuationLengths[id][0];
        float atten2 = AttenuationLengths[id][1];
        float locd = TMath::Abs(locMatchX) - 15.; // only attenuate close to end of paddle not to center.
        float corr = 1;
        if (locd>0){
          corr = (TMath::Exp(-locd/atten1) + TMath::Exp(-locd/atten2))/2.;
        }
        locMatchdE2 = loctofhit->dE * corr;
        //cout<<"Horizontal: dE2 "<<locMatchdE2<<" = "<<loctofhit->dE<<"  *  "<<corr<<endl;
      }
      else {
         locMatchdE2= 0;
      }
    }
  else {
    return;
  }
  
  DTOFPoint* locTOFPoint = new DTOFPoint;
  locTOFPoint->AddAssociatedObject(locTOFHit_Horizontal);
  locTOFPoint->AddAssociatedObject(locTOFHit_Vertical);
  locTOFPoint->pos.SetXYZ(locMatchX, locMatchY, locMatchZ);
  locTOFPoint->t = locMatchT;
  locTOFPoint->tErr = locMatchTErr;  
  locTOFPoint->dE = locMatchdE;
  locTOFPoint->dE1 = locMatchdE1;
  locTOFPoint->dE2 = locMatchdE2;
  
  locTOFPoint->dHorizontalBar = locTOFHit_Horizontal->bar;
  locTOFPoint->dVerticalBar = locTOFHit_Vertical->bar;
  
  //Status: 0 if no hit (or none above threshold), 1 if only North hit above threshold, 2 if only South hit above threshold, 3 if both hits above threshold
  locTOFPoint->dHorizontalBarStatus = int(locTOFHit_Horizontal->E_north > E_THRESHOLD) + 2*int(locTOFHit_Horizontal->E_south > E_THRESHOLD);
  locTOFPoint->dVerticalBarStatus = int(locTOFHit_Vertical->E_north > E_THRESHOLD) + 2*int(locTOFHit_Vertical->E_south > E_THRESHOLD);
  
  mData.push_back(locTOFPoint);
}

void DTOFPoint_factory::Create_UnMatchedTOFPoint(const tof_spacetimehit_t* locTOFSpacetimeHit)
{
  const DTOFPaddleHit* locPaddleHit = locTOFSpacetimeHit->TOFHit;
  bool locIsHorizontalBarFlag = (locPaddleHit->orientation == 1);
  float locPointZ = locIsHorizontalBarFlag ? CenterHorizPlane : CenterVertPlane;
  
  int id_vert = locPaddleHit->bar - 1;
  int id_horiz = NUM_BARS + locPaddleHit->bar - 1;
  double locVTErr = paddle_resolutions[id_vert];   // paddle time resolutions
  double locHTErr = paddle_resolutions[id_horiz];
  
  double locTErr = locIsHorizontalBarFlag ? locHTErr : locVTErr;
  
  if(locTOFSpacetimeHit->dPositionWellDefinedFlag)
    {
      //Position is well defined
      DTOFPoint* locTOFPoint = new DTOFPoint;
      locTOFPoint->AddAssociatedObject(locPaddleHit);
      
      locTOFPoint->pos.SetXYZ(locTOFSpacetimeHit->x, locTOFSpacetimeHit->y, locPointZ);
      locTOFPoint->t = locTOFSpacetimeHit->t;
      locTOFPoint->tErr = locTErr;
      locTOFPoint->dE = locPaddleHit->dE;
      
      locTOFPoint->dHorizontalBar = locIsHorizontalBarFlag ? locPaddleHit->bar : 0;
      locTOFPoint->dVerticalBar = locIsHorizontalBarFlag ? 0 : locPaddleHit->bar;
      
      //Status: 0 if no hit (or none above threshold), 1 if only North hit above threshold, 2 if only South hit above threshold, 3 if both hits above threshold
      locTOFPoint->dHorizontalBarStatus = locIsHorizontalBarFlag ? 3 : 0;
      locTOFPoint->dVerticalBarStatus = locIsHorizontalBarFlag ? 0 : 3;

      if (locTOFPoint->dHorizontalBar){
	locTOFPoint->dE2 = locPaddleHit->dE;
	locTOFPoint->dE1 = 0.;
      } else{
	locTOFPoint->dE2 = 0.;
	locTOFPoint->dE1 = locPaddleHit->dE;
      }
      
      mData.push_back(locTOFPoint);
    }
  else
    {
     return; 

     //position not well defined: save anyway:
      //Will use track matching to define position in the other direction
      //Then, will update the hit energy and time based on that position
     /*
      DTOFPoint* locTOFPoint = new DTOFPoint;
      locTOFPoint->AddAssociatedObject(locPaddleHit);
      
      float locPointX = locTOFSpacetimeHit->x;
      float locPointY = locTOFSpacetimeHit->y;
      locTOFPoint->pos.SetXYZ(locPointX, locPointY, locPointZ);
      locTOFPoint->t = locTOFSpacetimeHit->t;
      locTOFPoint->tErr = locTErr; 
      
      bool locNorthAboveThresholdFlag = (locPaddleHit->E_north > E_THRESHOLD);
      
      locTOFPoint->dHorizontalBar = locIsHorizontalBarFlag ? locPaddleHit->bar : 0;
      locTOFPoint->dVerticalBar = locIsHorizontalBarFlag ? 0 : locPaddleHit->bar;
      
      int locBarStatus = locNorthAboveThresholdFlag ? 1 : 2;
      locTOFPoint->dHorizontalBarStatus = locIsHorizontalBarFlag ? locBarStatus : 0;
      locTOFPoint->dVerticalBarStatus = locIsHorizontalBarFlag ? 0 : locBarStatus;
      
      //Energy: Propagate to paddle mid-point
      double locDeltaXToMidPoint = locTOFSpacetimeHit->dIsDoubleEndedBar ? HALFPADDLE : HALFPADDLE_ONESIDED;
      float locEnergy = locNorthAboveThresholdFlag ? locPaddleHit->E_north : locPaddleHit->E_south;
      locEnergy *= exp(locDeltaXToMidPoint/ATTEN_LENGTH);
      locTOFPoint->dE = locEnergy;

      // IGNORE NOW
      return;
      
      _data.push_back(locTOFPoint);
     */
    }
}

void DTOFPoint_factory::Finish()
{
  for(size_t loc_i = 0; loc_i < dTOFSpacetimeHitPool_All.size(); ++loc_i)
    delete dTOFSpacetimeHitPool_All[loc_i];
}
