// $Id$
//
//    File: DNeutralShower_factory.cc
// Created: Tue Aug  9 14:29:24 EST 2011
// Creator: pmatt (on Linux ifarml6 2.6.18-128.el5 x86_64)
//

#include "DNeutralShower_factory.h"
#include "DEventRFBunch.h"

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
  gPARMS->SetDefaultParameter("NeutralShower:TOF_RF_CUT", TOF_RF_CUT);

  SC_RF_CUT_MIN = 1.0;
  SC_RF_CUT_MAX = 7.0;
  gPARMS->SetDefaultParameter("NeutralShower:SC_RF_CUT_MIN", SC_RF_CUT_MIN);
  gPARMS->SetDefaultParameter("NeutralShower:SC_RF_CUT_MAX", SC_RF_CUT_MAX);

  SC_Energy_CUT = 0.2;
  gPARMS->SetDefaultParameter("NeutralShower:SC_Energy_CUT", SC_Energy_CUT);

  //TOF_FCAL_THETA_CUT = 10;
  //gPARMS->SetDefaultParameter("NeutralShower:TOF_FCAL_THETA_CUT", TOF_FCAL_THETA_CUT);

  //TOF_FCAL_PHI_CUT = 10;
  //gPARMS->SetDefaultParameter("NeutralShower:TOF_FCAL_PHI_CUT", TOF_FCAL_PHI_CUT);

  TOF_FCAL_x_match_CUT = 10;
  TOF_FCAL_y_match_CUT = 10;
  gPARMS->SetDefaultParameter("NeutralShower:TOF_FCAL_x_match_CUT", TOF_FCAL_x_match_CUT);
  gPARMS->SetDefaultParameter("NeutralShower:TOF_FCAL_y_match_CUT", TOF_FCAL_y_match_CUT);

  SC_FCAL_PHI_CUT = 25;
  gPARMS->SetDefaultParameter("NeutralShower:SC_FCAL_PHI_CUT", SC_FCAL_PHI_CUT);

  SC_BCAL_PHI_CUT = 15;
  gPARMS->SetDefaultParameter("NeutralShower:SC_BCAL_PHI_CUT", SC_BCAL_PHI_CUT);
}


//------------------
// init
//------------------
jerror_t DNeutralShower_factory::init(void)
{
  dResourcePool_TMatrixFSym->Set_ControlParams(20, 20, 20);
  return NOERROR;
}

//------------------
// brun
//------------------
jerror_t DNeutralShower_factory::brun(jana::JEventLoop *locEventLoop, int32_t runnumber)
{

  DApplication* locApplication = dynamic_cast<DApplication*>(locEventLoop->GetJApplication());
  DGeometry* locGeometry = locApplication->GetDGeometry(runnumber);

  jana::JCalibration *jcalib = japp->GetJCalibration(runnumber);
  double locTargetCenterZ;
  locGeometry->GetTargetZ(locTargetCenterZ);
  dTargetCenter.SetXYZ(0.0, 0.0, locTargetCenterZ);
  

  std::map<string, float> beam_spot;
  jcalib->Get("PHOTON_BEAM/beam_spot", beam_spot);
  m_beamSpotX = beam_spot.at("x");
  m_beamSpotY = beam_spot.at("y");
  
  RunNumber = runnumber;

  return NOERROR;
}

//------------------
// evnt
//------------------
jerror_t DNeutralShower_factory::evnt(jana::JEventLoop *locEventLoop, uint64_t eventnumber)
{
  const DDetectorMatches* locDetectorMatches = NULL;
  locEventLoop->GetSingle(locDetectorMatches);
    
  vector<const DBCALShower*> locBCALShowers;
  locEventLoop->Get(locBCALShowers);

  vector<const DFCALShower*> locFCALShowers;
  locEventLoop->Get(locFCALShowers);

  vector<const DCCALShower*> locCCALShowers;
  locEventLoop->Get(locCCALShowers);

  vector< const DEventRFBunch* > eventRFBunches;
  locEventLoop->Get(eventRFBunches);
  // there should always be one and only one object or else it is a coding error
  assert( eventRFBunches.size() == 1 );
  double rfTime = eventRFBunches[0]->dTime; // this is the RF time at the center of the target
  /*if (locCCALShowers.size() > 0) {
    std::cout <<"size bcal " << locBCALShowers.size() << std::endl; 
    std::cout <<"size fcal " << locFCALShowers.size() << std::endl; 
    std::cout <<"size ccal " << locCCALShowers.size() << std::endl;
    }*/ 
  // Loop over all DBCALShowers, create DNeutralShower if didn't match to any tracks
  // The chance of an actual neutral shower matching to a bogus track is very small

  //-----   TOF veto    -----//
  DVector3 vertex(m_beamSpotX, m_beamSpotY, dTargetCenter.Z());
  vector <const DTOFPoint*> locTOFPoints;
  locEventLoop->Get(locTOFPoints);
  int n_locTOFPoints = 0;
  for (vector<const DTOFPoint*>::const_iterator tof = locTOFPoints.begin(); tof != locTOFPoints.end(); tof++) {
    double xt = (*tof)->pos.X() - vertex.X();
    double yt = (*tof)->pos.Y() - vertex.Y();
    double zt = (*tof)->pos.Z() - vertex.Z();
    double rt = sqrt(xt*xt + yt*yt + zt*zt);
    double tt = (*tof)->t - (rt / TMath::C() * 1e7);
    double dt = tt - rfTime;
    if (fabs(dt) < TOF_RF_CUT)
      n_locTOFPoints ++;
  }
  
  //-----   SC veto -----//
  vector<const DSCHit*> locSCHits;
  locEventLoop->Get(locSCHits);
  int n_locSCHits = 0;
  for (unsigned int i = 0; i < locSCHits.size(); i ++) {
    const DSCHit *schits = locSCHits[i];
    double t = schits->t;
    double e = schits->dE;
    //double s = schits->sector;
    double diff_t = t - rfTime;
    //double phi = 6.0 * (s + 1) - 11.25;
    if (SC_RF_CUT_MIN < diff_t && diff_t < SC_RF_CUT_MAX) {
      if ((e * 1e3) > SC_Energy_CUT)
	n_locSCHits ++;
    }
  }


  JObject::oid_t locShowerID = 0;
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

      double x = locBCALShowers[loc_i]->x - vertex.X();
      double y = locBCALShowers[loc_i]->y - vertex.Y();
      double z = locBCALShowers[loc_i]->z - vertex.Z();
      double t = locBCALShowers[loc_i]->t;
      DVector3 position(x, y, z);
      double r = position.Mag();
      t -= (r / TMath::C() * 1e7);
      double phi_bcal = position.Phi();
      //double theta_bcal = position.Theta();
      
      bool Charge_Hit_in_BCAL = false;
      for (unsigned int i = 0; i < locSCHits.size(); i ++) {
	const DSCHit *schits = locSCHits[i];
	double t = schits->t;
	double e = schits->dE;
	double s = schits->sector;
	double diff_t = t - rfTime;
	double phi_sc = 6.0 * (s + 1) - 11.25 - 45.0 * TMath::DegToRad();
	if (SC_RF_CUT_MIN < diff_t && diff_t < SC_RF_CUT_MAX)
	  if ((e * 1e3) > SC_Energy_CUT)
	    if ((fabs(phi_bcal - phi_sc) * TMath::RadToDeg()) < SC_BCAL_PHI_CUT)
	      Charge_Hit_in_BCAL = true;
      }
      
      //locNeutralShower->dTOF_FCAL_match = 0;
      if (Charge_Hit_in_BCAL)
	locNeutralShower->dSC_BCAL_match = 1;
      else
	locNeutralShower->dSC_BCAL_match = 0;
      locNeutralShower->dTOFVeto = n_locTOFPoints;
      locNeutralShower->dSCVeto = n_locSCHits;

      locNeutralShower->dEnergy = locBCALShowers[loc_i]->E;
      locNeutralShower->dSpacetimeVertex.SetXYZT(locBCALShowers[loc_i]->x, locBCALShowers[loc_i]->y, locBCALShowers[loc_i]->z, locBCALShowers[loc_i]->t);
      auto locCovMatrix = dResourcePool_TMatrixFSym->Get_SharedResource();
      locCovMatrix->ResizeTo(5, 5);
      *locCovMatrix = locBCALShowers[loc_i]->ExyztCovariance;
      locNeutralShower->dCovarianceMatrix = locCovMatrix;
      
      locNeutralShower->AddAssociatedObject(locBCALShowers[loc_i]);

      _data.push_back(locNeutralShower);
    }

  // Loop over all DFCALShowers, create DNeutralShower if didn't match to any tracks
  // The chance of an actual neutral shower matching to a bogus track is very small
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

      DVector3 position = locFCALShowers[loc_i]->getPosition_log() - vertex;
      double t = locFCALShowers[loc_i]->getTime();
      double r = position.Mag();
      t -= (r / TMath::C() * 1e7);
      double phi_fcal = position.Phi();
      //double theta_fcal = position.Theta();
      
      bool Charge_Hit_in_FCAL = false;
      
      /*
      for (vector<const DTOFPoint*>::const_iterator tof = locTOFPoints.begin(); tof != locTOFPoints.end(); tof++) {
	double xt = (*tof)->pos.X() - vertex.X();
	double yt = (*tof)->pos.Y() - vertex.Y();
	double zt = (*tof)->pos.Z() - vertex.Z();
	double rt = sqrt(xt*xt + yt*yt + zt*zt);
	double tt = (*tof)->t - (rt / TMath::C() * 1e7);
	double dt = tt - rfTime;
	DVector3 tof_position(xt, yt, zt);
	double phi_tof = tof_position.Phi();
	double theta_tof = tof_position.Theta();
	if (fabs(dt) < TOF_RF_CUT && 
	    fabs(theta_fcal - theta_tof) * TMath::RadToDeg() < TOF_FCAL_THETA_CUT && 
	    fabs(phi_fcal - phi_tof) * TMath::RadToDeg() < TOF_FCAL_PHI_CUT) {
	  Charge_Hit_in_FCAL = true;
	}
      }
      */
      
      int tof_match = check_TOF_match(position, rfTime, vertex, locTOFPoints);
      
      if (tof_match)
	locNeutralShower->dTOF_FCAL_match = 1;
      else
	locNeutralShower->dTOF_FCAL_match = 0;
      
      Charge_Hit_in_FCAL = false;
      for (unsigned int i = 0; i < locSCHits.size(); i ++) {
	const DSCHit *schits = locSCHits[i];
	double t = schits->t;
	double e = schits->dE;
	double s = schits->sector;
	double diff_t = t - rfTime;
	double phi_sc = 6.0 * (s + 1) - 11.25 - 45.0 * TMath::DegToRad();
	if (SC_RF_CUT_MIN < diff_t && diff_t < SC_RF_CUT_MAX) 
	  if ((e * 1e3) > SC_Energy_CUT) 
	    if ((fabs(phi_fcal - phi_sc) * TMath::RadToDeg()) < SC_FCAL_PHI_CUT)
	      Charge_Hit_in_FCAL = true;
      }
      
      if (Charge_Hit_in_FCAL)
	locNeutralShower->dSC_FCAL_match = 1;
      else
        locNeutralShower->dSC_FCAL_match = 0;
      locNeutralShower->dTOFVeto = n_locTOFPoints;
      locNeutralShower->dSCVeto = n_locSCHits;

      auto locCovMatrix = dResourcePool_TMatrixFSym->Get_SharedResource();
      locCovMatrix->ResizeTo(5, 5);
      *locCovMatrix = locFCALShowers[loc_i]->ExyztCovariance;
      locNeutralShower->dCovarianceMatrix = locCovMatrix;

      locNeutralShower->AddAssociatedObject(locFCALShowers[loc_i]);

      _data.push_back(locNeutralShower);
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
      
      //locNeutralShower->dTOF_FCAL_match = 0;
      locNeutralShower->dTOFVeto = n_locTOFPoints;
      locNeutralShower->dSCVeto = n_locSCHits;

      locNeutralShower->dEnergy = locCCALShowers[loc_i]->E;
      locNeutralShower->dSpacetimeVertex.SetXYZT(locCCALShowers[loc_i]->x, locCCALShowers[loc_i]->y, locCCALShowers[loc_i]->z, locCCALShowers[loc_i]->time);
      //std::cout << "CCAL MERDE Energy " <<  locCCALShowers[loc_i]->E << " time " << locCCALShowers[loc_i]->time << std::endl;
      auto locCovMatrix = dResourcePool_TMatrixFSym->Get_SharedResource();
      locCovMatrix->ResizeTo(5, 5);
      *locCovMatrix = locCCALShowers[loc_i]->ExyztCovariance;
      locNeutralShower->dCovarianceMatrix = locCovMatrix;

      locNeutralShower->AddAssociatedObject(locCCALShowers[loc_i]);
      
      _data.push_back(locNeutralShower);
    }
  
  sort(_data.begin(), _data.end(), DNeutralShower_SortByEnergy);

  return NOERROR;
}

//------------------
// erun
//------------------
jerror_t DNeutralShower_factory::erun(void)
{
  return NOERROR;
}

//------------------
// fini
//------------------
jerror_t DNeutralShower_factory::fini(void)
{
  return NOERROR;
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

int DNeutralShower_factory::check_TOF_match(DVector3 fcalpos, double rfTime, DVector3 vertex, vector< const DTOFPoint* > locTOFPoints) 
{
	
  int global_tof_match = 0;
  
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
    
    if (fabs(dx) < TOF_FCAL_x_match_CUT && fabs(dy) < TOF_FCAL_y_match_CUT) {
      if (fabs(dt) < TOF_RF_CUT) 
	global_tof_match ++;
    }
  }

  return global_tof_match;
}
