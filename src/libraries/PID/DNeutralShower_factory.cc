// $Id$
//
//    File: DNeutralShower_factory.cc
// Created: Tue Aug  9 14:29:24 EST 2011
// Creator: pmatt (on Linux ifarml6 2.6.18-128.el5 x86_64)
//

#include "DNeutralShower_factory.h"
#include "DEventRFBunch.h"
#include "DANA/DEvent.h"

inline bool DNeutralShower_SortByEnergy(const DNeutralShower* locNeutralShower1, const DNeutralShower* locNeutralShower2)
{
  // truncate the shower energies: in units of MeV, ignore all digits that are 10s-place and above
  // then sort by increasing energy: pseudo-random

  //guard against NaN: necessary since casting to int
  bool locFirstIsNaN = (!(locNeutralShower1->dEnergy > -1.0) && !(locNeutralShower1->dEnergy < 1.0));
  bool locSecondIsNaN = (!(locNeutralShower2->dEnergy > -1.0) && !(locNeutralShower2->dEnergy < 1.0));
  if(locFirstIsNaN)
    return false;
  if(locSecondIsNaN)
    return true;
  double locE1 = locNeutralShower1->dEnergy - double(int(locNeutralShower1->dEnergy*100.0))/100.0;
  double locE2 = locNeutralShower2->dEnergy - double(int(locNeutralShower2->dEnergy*100.0))/100.0;

  return (locE1 < locE2);
}

// constructor

DNeutralShower_factory::DNeutralShower_factory()
{

  vector< string > vars( inputVars, inputVars + sizeof( inputVars )/sizeof( char* ) );
  dFCALClassifier = new DNeutralShower_FCALQualityMLP( vars );
  
  dResourcePool_TMatrixFSym = std::make_shared<DResourcePool<TMatrixFSym>>(); 

  TOF_RF_CUT = 6.5;
  japp->SetDefaultParameter("NeutralShower:TOF_RF_CUT", TOF_RF_CUT);
  
  SC_RF_CUT_MIN = 1.0;
  SC_RF_CUT_MAX = 7.0;
  japp->SetDefaultParameter("NeutralShower:SC_RF_CUT_MIN", SC_RF_CUT_MIN);
  japp->SetDefaultParameter("NeutralShower:SC_RF_CUT_MAX", SC_RF_CUT_MAX);

  SC_Energy_CUT = 0.2;
  japp->SetDefaultParameter("NeutralShower:SC_Energy_CUT", SC_Energy_CUT);

  ECAL_FCAL_CUT = 80.;
  japp->SetDefaultParameter("NeutralShower:ECAL_FCAL_CUT", ECAL_FCAL_CUT);
}


//------------------
// Init
//------------------
void DNeutralShower_factory::Init()
{
  dResourcePool_TMatrixFSym->Set_ControlParams(20, 20, 20);
}

//------------------
// BeginRun
//------------------
void DNeutralShower_factory::BeginRun(const std::shared_ptr<const JEvent>& event)
{
  DGeometry* locGeometry = DEvent::GetDGeometry(event);

  JCalibration *jcalib = DEvent::GetJCalibration(event);
  double locTargetCenterZ;
  locGeometry->GetTargetZ(locTargetCenterZ);
  dTargetCenter.SetXYZ(0.0, 0.0, locTargetCenterZ);
  
  // Get start counter geometry;
  locGeometry->GetStartCounterGeom(sc_pos,sc_norm);

  // Get FCAL geometry
  event->GetSingle(dFCALGeometry);
  
  std::map<string, float> beam_spot;
  jcalib->Get("PHOTON_BEAM/beam_spot", beam_spot);
  m_beamSpotX = beam_spot.at("x");
  m_beamSpotY = beam_spot.at("y");
  
  RunNumber = event->GetRunNumber();
}

//------------------
// Process
//------------------
void DNeutralShower_factory::Process(const std::shared_ptr<const JEvent>& event)
{
  const DDetectorMatches* locDetectorMatches = NULL;
  event->GetSingle(locDetectorMatches);
    
  vector<const DBCALShower*> locBCALShowers;
  event->Get(locBCALShowers);

  vector<const DFCALShower*> locFCALShowers;
  event->Get(locFCALShowers);

  vector<const DECALShower*> locECALShowers;
  event->Get(locECALShowers);

  vector<const DCCALShower*> locCCALShowers;
  event->Get(locCCALShowers);

  vector< const DEventRFBunch* > eventRFBunches;
  event->Get(eventRFBunches);
  // there should always be one and only one object or else it is a coding error
  assert( eventRFBunches.size() == 1 );
  double rfTime = eventRFBunches[0]->dTime; // this is the RF time at the center of the target

  //-----   TOF veto    -----//
  DVector3 vertex(m_beamSpotX, m_beamSpotY, dTargetCenter.Z());
  vector <const DTOFPoint*> locTOFPoints;
  event->Get(locTOFPoints);
  //-----   SC veto -----//
  vector<const DSCHit*> locSCHits;
  event->Get(locSCHits);
  
  // Loop over all DFCALShowers, create DNeutralShower if didn't match to any tracks
  // The chance of an actual neutral shower matching to a bogus track is very small
  oid_t locShowerID = 0;
  for(size_t loc_i = 0; loc_i < locFCALShowers.size(); ++loc_i)
    {
      if(locDetectorMatches->Get_IsMatchedToTrack(locFCALShowers[loc_i]))
	continue;

      // create DNeutralShower
      DNeutralShower* locNeutralShower = new DNeutralShower();
      locNeutralShower->dBCALFCALShower = static_cast<const JObject*>(locFCALShowers[loc_i]);
      locNeutralShower->dDetectorSystem = SYS_FCAL;
      locNeutralShower->dShowerID = locShowerID;
      ++locShowerID;
      
      locNeutralShower->dEnergy = locFCALShowers[loc_i]->getEnergy();
      locNeutralShower->dSpacetimeVertex.SetVect(locFCALShowers[loc_i]->getPosition());
      locNeutralShower->dSpacetimeVertex.SetT(locFCALShowers[loc_i]->getTime());
      
      locNeutralShower->dQuality = getFCALQuality( locFCALShowers[loc_i], rfTime );
      
      // Check if indeed shower is not related to a non-reconstructed track in SC and/or TOF
      DVector3 position = locFCALShowers[loc_i]->getPosition_log() - vertex;
      double phi_fcal = position.Phi();
      double delta_x_min = 1000.;
      double delta_y_min = 1000.;
      double delta_phi_min = 1000.;
      int sc_match = check_SC_match(phi_fcal, rfTime, locSCHits, delta_phi_min);
      int tof_match = check_TOF_match(position, rfTime, vertex, locTOFPoints, delta_x_min, delta_y_min);
      locNeutralShower->dTOF_FCAL_match = tof_match;
      locNeutralShower->dTOF_FCAL_x_min = (float) delta_x_min;
      locNeutralShower->dTOF_FCAL_y_min = (float) delta_y_min;
      locNeutralShower->dSC_FCAL_match = sc_match;
      locNeutralShower->dSC_FCAL_phi_min = (float) delta_phi_min;
            
      auto locCovMatrix = dResourcePool_TMatrixFSym->Get_SharedResource();
      locCovMatrix->ResizeTo(5, 5);
      *locCovMatrix = locFCALShowers[loc_i]->ExyztCovariance;
      locNeutralShower->dCovarianceMatrix = locCovMatrix;

      locNeutralShower->AddAssociatedObject(locFCALShowers[loc_i]);

      Insert(locNeutralShower);
    }

  // Loop over all DECALShowers, create DNeutralShower if didn't match to any
  // tracks and the cluster did not share energy with an FCAL shower. 
  size_t numShowers=mData.size();
  for(size_t loc_i = 0; loc_i < locECALShowers.size(); ++loc_i){
    if(locDetectorMatches->Get_IsMatchedToTrack(locECALShowers[loc_i]))
      continue;

    // Look for showers near boundary between ECAL and FCAL
    bool gotMatch=false;
    // If there were no FCAL showers, skip to creation of DNeutralShower
    if (numShowers>0){
      // Check if the ECAL shower has hits near the FCAL-ECAL border
      if (locECALShowers[loc_i]->isNearBorder){
	// Compare positions between ECAL and FCAL showers
	DVector3 locEcalPos=locECALShowers[loc_i]->pos;
	for (size_t loc_j=0;loc_j<numShowers;loc_j++){
	  const DFCALShower *locFCALShower=static_cast<const DFCALShower*>(mData[loc_j]->dBCALFCALShower);
	  if (locFCALShower->getIsNearBorder()){
	    DVector3 locFcalPos=mData[loc_j]->dSpacetimeVertex.Vect();
	    double locDx=locEcalPos.X()-locFcalPos.X();
	    double locDy=locEcalPos.Y()-locFcalPos.Y();
	    double locR2=locDx*locDx+locDy*locDy;
	    if (locR2<ECAL_FCAL_CUT){
	      gotMatch=true;
	      
	      // Flag that this shower is a combination of ECAL and FCAL showers
	      mData[loc_j]->dDetectorSystem=SYS_ECAL_FCAL;
	      
	      // Get the combined energy
	      double locE_ECAL=locECALShowers[loc_i]->E;
	      double locE_FCAL=mData[loc_j]->dEnergy;
	      double locEtot=locE_ECAL+locE_FCAL;
	      double ecal_frac=locE_ECAL/locEtot;
	      double fcal_frac=1.-ecal_frac;
	      mData[loc_j]->dEnergy=locEtot;
	    
	      // Energy-weighted time
	      double locTime=ecal_frac*locECALShowers[loc_i]->t
		+fcal_frac*mData[loc_j]->dSpacetimeVertex.T();
	      mData[loc_j]->dSpacetimeVertex.SetT(locTime);
	      
	      // Energy-weighted position
	      DVector3 locPos=ecal_frac*locEcalPos+fcal_frac*locFcalPos;
	      mData[loc_j]->dSpacetimeVertex.SetVect(locPos);
	      
	      // Add the ECAL shower as an associated object
	      mData[loc_j]->AddAssociatedObject(locECALShowers[loc_i]);
	      
	      break;
	    }
	  }
	}
      }
    }
    
    // If we did not find a match between the ECAL and the FCAL, create
    // a DNeutralShower from the ECAL shower
    if (gotMatch==false){
      DNeutralShower* locNeutralShower = new DNeutralShower();
      locNeutralShower->dBCALFCALShower = static_cast<const JObject*>(locECALShowers[loc_i]);
      locNeutralShower->dDetectorSystem = SYS_ECAL;
      locNeutralShower->dShowerID = locShowerID;
      ++locShowerID;
      
      locNeutralShower->dEnergy = locECALShowers[loc_i]->E;
      locNeutralShower->dSpacetimeVertex.SetVect(locECALShowers[loc_i]->pos);
      locNeutralShower->dSpacetimeVertex.SetT(locECALShowers[loc_i]->t);
      locNeutralShower->AddAssociatedObject(locECALShowers[loc_i]);
      
      locNeutralShower->dQuality = 1;
      
      auto locCovMatrix = dResourcePool_TMatrixFSym->Get_SharedResource();
      locCovMatrix->ResizeTo(5, 5);
      *locCovMatrix = locECALShowers[loc_i]->ExyztCovariance;
      locNeutralShower->dCovarianceMatrix = locCovMatrix;
      
      Insert(locNeutralShower);
    }
  }
  
  // Loop over all DBCALShowers, create DNeutralShower if didn't match to any tracks
  // The chance of an actual neutral shower matching to a bogus track is very small
  for(size_t loc_i = 0; loc_i < locBCALShowers.size(); ++loc_i)
    {
      if(locDetectorMatches->Get_IsMatchedToTrack(locBCALShowers[loc_i]))
	continue;

      // create DNeutralShower
      DNeutralShower* locNeutralShower = new DNeutralShower();
      locNeutralShower->dBCALFCALShower = static_cast<const JObject*>(locBCALShowers[loc_i]);
      locNeutralShower->dDetectorSystem = SYS_BCAL;
      locNeutralShower->dShowerID = locShowerID;
      ++locShowerID;
      
      // in the BCAL set the quality variable 1 to avoid eliminating
      // NeutralShowers in future splitoff rejection algorithms
      locNeutralShower->dQuality = 1;
      
      // Check if indeed shower is not related to a non-reconstructed track in SC
      double x = locBCALShowers[loc_i]->x - vertex.X();
      double y = locBCALShowers[loc_i]->y - vertex.Y();
      double z = locBCALShowers[loc_i]->z - vertex.Z();
      DVector3 position(x, y, z);
      double phi_bcal = position.Phi();
      double delta_phi_min = 1000.;
      int sc_match = check_SC_match(phi_bcal, rfTime, locSCHits, delta_phi_min);
      locNeutralShower->dSC_BCAL_match = sc_match;
      locNeutralShower->dSC_BCAL_phi_min = (float) delta_phi_min;
      
      locNeutralShower->dEnergy = locBCALShowers[loc_i]->E;
      locNeutralShower->dSpacetimeVertex.SetXYZT(locBCALShowers[loc_i]->x, locBCALShowers[loc_i]->y, locBCALShowers[loc_i]->z, locBCALShowers[loc_i]->t);
      auto locCovMatrix = dResourcePool_TMatrixFSym->Get_SharedResource();
      locCovMatrix->ResizeTo(5, 5);
      *locCovMatrix = locBCALShowers[loc_i]->ExyztCovariance;
      locNeutralShower->dCovarianceMatrix = locCovMatrix;
      
      locNeutralShower->AddAssociatedObject(locBCALShowers[loc_i]);

      Insert(locNeutralShower);
    }
  
  // Loop over all DCCALShowers, create DNeutralShower if didn't match to any tracks
  // The chance of an actual neutral shower matching to a bogus track is very small
  for(size_t loc_i = 0; loc_i < locCCALShowers.size(); ++loc_i)
    {
      //if(locDetectorMatches->Get_IsMatchedToTrack(locCCALShowers[loc_i]))
      //continue;

      // create DNeutralShower
      DNeutralShower* locNeutralShower = new DNeutralShower();
      locNeutralShower->dBCALFCALShower = static_cast<const JObject*>(locCCALShowers[loc_i]);
      locNeutralShower->dDetectorSystem = SYS_CCAL;
      locNeutralShower->dShowerID = locShowerID;
      ++locShowerID;
      
      locNeutralShower->dQuality = 1;
      
      locNeutralShower->dEnergy = locCCALShowers[loc_i]->E;
      locNeutralShower->dSpacetimeVertex.SetXYZT(locCCALShowers[loc_i]->x, locCCALShowers[loc_i]->y, locCCALShowers[loc_i]->z, locCCALShowers[loc_i]->time);
      
      auto locCovMatrix = dResourcePool_TMatrixFSym->Get_SharedResource();
      locCovMatrix->ResizeTo(5, 5);
      *locCovMatrix = locCCALShowers[loc_i]->ExyztCovariance;
      locNeutralShower->dCovarianceMatrix = locCovMatrix;

      locNeutralShower->AddAssociatedObject(locCCALShowers[loc_i]);
      
      Insert(locNeutralShower);
    }
  
  sort(mData.begin(), mData.end(), DNeutralShower_SortByEnergy);
}

//------------------
// EndRun
//------------------
void DNeutralShower_factory::EndRun()
{
}

//------------------
// Finish
//------------------
void DNeutralShower_factory::Finish()
{
}

double DNeutralShower_factory::getFCALQuality( const DFCALShower* fcalShower, double rfTime ) const {

  double flightDistance = ( fcalShower->getPosition() - dTargetCenter ).Mag();
  double flightTime = fcalShower->getTime() - rfTime;
  
  vector< double > mvaInputs( 8 );
  mvaInputs[0] = fcalShower->getNumBlocks();
  mvaInputs[1] = fcalShower->getE9E25();
  mvaInputs[2] = fcalShower->getE1E9();
  mvaInputs[3] = fcalShower->getSumU();
  mvaInputs[4] = fcalShower->getSumV();
  mvaInputs[5] = ( mvaInputs[3] - mvaInputs[4] ) / ( mvaInputs[3] + mvaInputs[4] );
  mvaInputs[6] = flightDistance / flightTime;
  mvaInputs[7] = fcalShower->getTime() - ( rfTime + fcalShower->getTimeTrack() );
  
  return dFCALClassifier->GetMvaValue( mvaInputs );
}

int DNeutralShower_factory::check_TOF_match(DVector3 fcalpos, double rfTime, DVector3 vertex, vector< const DTOFPoint* > locTOFPoints, double &dx_min, double &dy_min) 
{
  int global_tof_match = 0;
  dx_min = 1000;
  dy_min = 1000;
  for (vector< const DTOFPoint* >::const_iterator tof = locTOFPoints.begin(); tof != locTOFPoints.end(); tof++) {
    
    double xt = (*tof)->pos.X() - vertex.X();
    double yt = (*tof)->pos.Y() - vertex.Y();
    double zt = (*tof)->pos.Z() - vertex.Z();
    double rt = sqrt(xt*xt + yt*yt + zt*zt);
    double tt = (*tof)->t - (rt / TMath::C() * 1e7);
    double dt = tt - rfTime;
    xt *= fcalpos.Z() / zt;
    yt *= fcalpos.Z() / zt;
    double dx = fcalpos.X() - xt;
    double dy = fcalpos.Y() - yt;
    if (fabs(dt) < TOF_RF_CUT) {
      if (dx*dx + dy*dy < dx_min*dx_min + dy_min*dy_min) {
	dx_min = fabs(dx);
	dy_min = fabs(dy);
      }
      global_tof_match ++;
    }
  }
  
  return global_tof_match;
}

int DNeutralShower_factory::check_SC_match(double phi, double rfTime, vector< const DSCHit* > locSCHits, double &dphi_min) 
{
  int global_sc_match = 0;
  dphi_min = 1000.;
  for (unsigned int i = 0; i < locSCHits.size(); i ++) {
    
    const DSCHit *schits = locSCHits[i];
    double t = schits->t;
    double e = schits->dE;
    double s = schits->sector - 1;
    double diff_t = t - rfTime;
    double phi_sc = sc_pos[s][0].Phi();
    double dphi = phi - phi_sc;
    if (dphi < -TMath::Pi()) 
      dphi += 2.0 * TMath::Pi();
    if (dphi > TMath::Pi()) 
      dphi -= 2.0 * TMath::Pi();
    if (fabs(dphi) < dphi_min)
      dphi_min = fabs(dphi);
    if ((SC_RF_CUT_MIN < diff_t) && (diff_t < SC_RF_CUT_MAX) && ((e * 1e3) > SC_Energy_CUT)) {
      global_sc_match ++;
    }
  } 
  
  return global_sc_match;
}
