//
//    File: DFCALShower_factory.cc
// Created: Tue May 17 11:57:50 EST 2005
// Creator: remitche (on Linux mantrid00 2.4.20-18.8smp i686)

#include <thread>
#include <math.h>
#include <DVector3.h>
#include "TH2F.h"
#include "TROOT.h"
#include "TDirectory.h"
using namespace std;

#include <JANA/JEvent.h>
#include <JANA/Calibrations/JCalibrationManager.h>
#include <JANA/Services/JLockService.h>
#include <DANA/DEvent.h>

#include "FCAL/DFCALShower_factory.h"
#include "FCAL/DFCALGeometry.h"
#include "FCAL/DFCALCluster.h"
#include "FCAL/DFCALHit.h"
#include "TRACKING/DTrackWireBased.h"
#include "DANA/DGeometryManager.h"
#include "HDGEOMETRY/DGeometry.h"

//----------------
// Init
//----------------
void DFCALShower_factory::Init()
{
  auto app = GetApplication();

  //debug_level=1;
  // should we use CCDB constants?
  LOAD_NONLIN_CCDB = true;
  LOAD_TIMING_CCDB = true;
  // 29/03/2020 ijaegle@jlab.org decouple non linear and timing correction
  app->SetDefaultParameter("FCAL:LOAD_NONLIN_CCDB", LOAD_NONLIN_CCDB);
  app->SetDefaultParameter("FCAL:LOAD_TIMING_CCDB", LOAD_TIMING_CCDB);
  // Should we use the PrimeX-D energy correction?
  USE_RING_E_CORRECTION_V1=false;
  app->SetDefaultParameter("FCAL:USE_RING_E_CORRECTION_V1",USE_RING_E_CORRECTION_V1);

  USE_RING_E_CORRECTION_V2=false;
  app->SetDefaultParameter("FCAL:USE_RING_E_CORRECTION_V2",USE_RING_E_CORRECTION_V2);

  USE_CPP_E_CORRECTION=false;
  app->SetDefaultParameter("FCAL:USE_CPP_CORRECTION",USE_CPP_E_CORRECTION);
  
  SHOWER_ENERGY_THRESHOLD = 50*k_MeV;
  app->SetDefaultParameter("FCAL:SHOWER_ENERGY_THRESHOLD", SHOWER_ENERGY_THRESHOLD);

  SHOWER_POSITION_LOG = false;
  app->SetDefaultParameter("FCAL:SHOWER_POSITION_LOG", SHOWER_POSITION_LOG);
  // these need to come from database to ensure accuracy
  // remove default value which might be close to the right solution,
  // but not quite correct -- allow command line tuning

  cutoff_energy= 0;
  linfit_slope = 0;
  linfit_intercept = 0;
  expfit_param1 = 1.10358;
  expfit_param2 = 0.31385;
  expfit_param3 = -2.02585;

  timeConst0 = 0;
  timeConst1 = 0; 
  timeConst2 = 0;
  timeConst3 = 0; 
  timeConst4 = 0;

  app->SetDefaultParameter("FCAL:cutoff_energy", cutoff_energy);
  app->SetDefaultParameter("FCAL:linfit_slope", linfit_slope);
  app->SetDefaultParameter("FCAL:linfit_intercept", linfit_intercept);
  app->SetDefaultParameter("FCAL:expfit_param1", expfit_param1);
  app->SetDefaultParameter("FCAL:expfit_param2", expfit_param2);
  app->SetDefaultParameter("FCAL:expfit_param3", expfit_param3);
  
  USE_NONLINEAR_CORRECTION_TYPE = -1;
  app->SetDefaultParameter("FCAL:USE_NONLINEAR_CORRECTION_TYPE",USE_NONLINEAR_CORRECTION_TYPE);
  
  if (USE_NONLINEAR_CORRECTION_TYPE == 0) {
  } else if (USE_NONLINEAR_CORRECTION_TYPE == 1) {
    expfit_param1 = 2;
    expfit_param1 = 0;
    expfit_param1 = 0;
    SHOWER_POSITION_LOG = true;
    USE_RING_E_CORRECTION_V1 = true;
    USE_RING_E_CORRECTION_V2 = false;
    USE_CPP_E_CORRECTION = false;
  } else if (USE_NONLINEAR_CORRECTION_TYPE == 2) {
    expfit_param1 = 2;
    expfit_param1 = 0;
    expfit_param1 = 0;
    SHOWER_POSITION_LOG = true;
    USE_RING_E_CORRECTION_V1 = false;
    USE_RING_E_CORRECTION_V2 = true;
    USE_CPP_E_CORRECTION = false;
  } else if (USE_NONLINEAR_CORRECTION_TYPE == 3) {
    SHOWER_POSITION_LOG = true;
    USE_RING_E_CORRECTION_V1 = false;
    USE_RING_E_CORRECTION_V2 = false;
    USE_CPP_E_CORRECTION = true;
  }

  app->SetDefaultParameter("FCAL:P0", timeConst0);
  app->SetDefaultParameter("FCAL:P1", timeConst1);
  app->SetDefaultParameter("FCAL:P2", timeConst2);
  app->SetDefaultParameter("FCAL:P3", timeConst3);
  app->SetDefaultParameter("FCAL:P4", timeConst4);

  // Parameters to make shower-depth correction taken from Radphi, 
  // slightly modifed to match photon-polar angle
  FCAL_RADIATION_LENGTH = 0;
  FCAL_CRITICAL_ENERGY = 0;
  FCAL_SHOWER_OFFSET = 0;
	
  app->SetDefaultParameter("FCAL:FCAL_RADIATION_LENGTH", FCAL_RADIATION_LENGTH);
  app->SetDefaultParameter("FCAL:FCAL_CRITICAL_ENERGY", FCAL_CRITICAL_ENERGY);
  app->SetDefaultParameter("FCAL:FCAL_SHOWER_OFFSET", FCAL_SHOWER_OFFSET);

  VERBOSE = 0;              ///< >0 once off info ; >2 event by event ; >3 everything
  COVARIANCEFILENAME = "";  ///<  Setting the filename will take precidence over the CCDB.  Files must end in ij.txt, where i and j are integers corresponding to the element of the matrix
  app->SetDefaultParameter("DFCALShower:VERBOSE", VERBOSE, "Verbosity level for DFCALShower objects and factories");
  app->SetDefaultParameter("DFCALShower:COVARIANCEFILENAME", COVARIANCEFILENAME, "File name for covariance files");
  
  
  log_position_const = 4.2;
  app->SetDefaultParameter("FCAL:log_position_const", log_position_const);
}

//------------------
// BeginRun
//------------------
void DFCALShower_factory::BeginRun(const std::shared_ptr<const JEvent>& event)
{
  auto runnumber = event->GetRunNumber();
  auto app = event->GetJApplication();
  auto jcalib = app->GetService<JCalibrationManager>()->GetJCalibration(runnumber);
  auto root_lock = app->GetService<JLockService>();
  auto geo_manager = app->GetService<DGeometryManager>();
  auto geom = geo_manager->GetDGeometry(runnumber);

      map<string, double> depth_correction_params;
      if(jcalib->Get("FCAL/depth_correction_params", depth_correction_params)) {
         jerr << "Problem loading FCAL/depth_correction_params from CCDB!" << endl;
      } else {
         FCAL_RADIATION_LENGTH   = depth_correction_params["radiation_length"];
         FCAL_CRITICAL_ENERGY  = depth_correction_params["critical_energy"];
         FCAL_SHOWER_OFFSET = depth_correction_params["shower_offset"];
      }

	
  // Get calibration constants
  map<string, double> fcal_parms;
  jcalib->Get("FCAL/fcal_parms", fcal_parms);
  if (fcal_parms.find("FCAL_C_EFFECTIVE")!=fcal_parms.end()){
    FCAL_C_EFFECTIVE = fcal_parms["FCAL_C_EFFECTIVE"];
    if(debug_level>0)jout<<"FCAL_C_EFFECTIVE = "<<FCAL_C_EFFECTIVE<<endl;
  } else {
    jerr<<"Unable to get FCAL_C_EFFECTIVE from FCAL/fcal_parms in Calib database!"<<endl;
  }

  if (geom) {
    geom->GetTargetZ(m_zTarget);
    event->GetSingle(fcalGeom);
    m_FCALfront=fcalGeom->fcalFrontZ();
    haveInsert=geom->HaveInsert();
  }
  else{
      
    cerr << "No geometry accessible." << endl;
    return; // RESOURCE_UNAVAILABLE;
  }
  // 29/03/2020 ijaegle@jlab.org add x,y
  std::map<string, float> beam_spot;
  jcalib->Get("PHOTON_BEAM/beam_spot", beam_spot);
  
  // Look in CCDB which non-linear correction version should be used
  nonlinear_correction_type.clear();
  DEvent::GetCalib(event, "FCAL/nonlinear_correction_type", nonlinear_correction_type);
  if (nonlinear_correction_type.size() > 0 && USE_NONLINEAR_CORRECTION_TYPE < 0) {
    if (debug_level > 0) {
      TString str_coef[] = {"A"};
      jout << Form(" %s", str_coef[0].Data()) << nonlinear_correction_type[0]; 
      jout << endl;
    }
    
    if (nonlinear_correction_type[0] == 0) {
      LOAD_NONLIN_CCDB = true;
    } else if (nonlinear_correction_type[0] == 1) {
      LOAD_NONLIN_CCDB = true;
      expfit_param1 = 2;
      expfit_param1 = 0;
      expfit_param1 = 0;
      SHOWER_POSITION_LOG = true;
      USE_RING_E_CORRECTION_V1 = true;	
      USE_RING_E_CORRECTION_V2 = false;
      USE_CPP_E_CORRECTION = false;
    } else if (nonlinear_correction_type[0] == 2) {
      LOAD_NONLIN_CCDB = true;
      expfit_param1 = 2;
      expfit_param1 = 0;
      expfit_param1 = 0;
      SHOWER_POSITION_LOG = true;
      USE_RING_E_CORRECTION_V1 = false;
      USE_RING_E_CORRECTION_V2 = true;
      USE_CPP_E_CORRECTION = false;
    } else if (nonlinear_correction_type[0] == 3) {
      LOAD_NONLIN_CCDB = true;
      SHOWER_POSITION_LOG = true;
      USE_RING_E_CORRECTION_V1 = false;
      USE_RING_E_CORRECTION_V2 = false;
      USE_CPP_E_CORRECTION = true;
    }
  }
  
  // but allow these to be overridden by command line parameters
  energy_dependence_correction_vs_ring.clear();
  nonlinear_correction.clear();
  nonlinear_correction_cpp.clear();
  block_to_square.clear();
  if(LOAD_NONLIN_CCDB) {
    map<string, double> shower_calib_piecewise;
    jcalib->Get("FCAL/shower_calib_piecewise", shower_calib_piecewise);
    cutoff_energy = shower_calib_piecewise["cutoff_energy"];
    linfit_slope = shower_calib_piecewise["linfit_slope"];
    linfit_intercept = shower_calib_piecewise["linfit_intercept"];
    expfit_param1 = shower_calib_piecewise["expfit_param1"];
    expfit_param2 = shower_calib_piecewise["expfit_param2"];
    expfit_param3 = shower_calib_piecewise["expfit_param3"];
    m_beamSpotX = 0;
    m_beamSpotY = 0;
    //expfit_param1 = 1.10358;
    //expfit_param2 = 0.31385;
    //expfit_param3 = -2.02585;
    if(debug_level>0) {
      jout << "cutoff_energy = " << cutoff_energy << endl;
      jout << "linfit_slope = " << linfit_slope << endl;
      jout << "linfit_intercept = " << linfit_intercept << endl;
      jout << "expfit_param1 = " << expfit_param1 << endl;
      jout << "expfit_param2 = " << expfit_param2<< endl;
      jout << "expfit_param3 = " << expfit_param3 << endl;
    }
    jcalib->Get("FCAL/energy_dependence_correction_vs_ring", energy_dependence_correction_vs_ring);
    if (energy_dependence_correction_vs_ring.size() > 0 && energy_dependence_correction_vs_ring[0][0] != 0) {
      m_beamSpotX = beam_spot.at("x");
      m_beamSpotY = beam_spot.at("y");
      if (debug_level > 0) {
	TString str_coef[] = {"A", "B", "C", "D", "E", "F"};
	for (int i = 0; i < 24; i ++) {
	  //for (int j = 0; j < 6; j ++) {
	  for (int j = 0; j < 3; j ++) {
	    jout << "Ring # " << i << Form(" %s", str_coef[j].Data()) << energy_dependence_correction_vs_ring[i][j]; 
	  }
	  jout << endl;
	}
      }
    }
    DEvent::GetCalib(event, "FCAL/nonlinear_correction", nonlinear_correction);
    if (nonlinear_correction.size() > 0) {
      m_beamSpotX = beam_spot.at("x");
      m_beamSpotY = beam_spot.at("y");
      if (debug_level > 0) {
	TString str_coef[] = {"A", "B", "C", "D", "E", "F", "G", "H", "I"};
	for (int i = 0; i < 5; i ++) {
	  //for (int j = 0; j < 6; j ++) {
	  for (int j = 0; j < 9; j ++) {
	    jout << "Ring # " << i << Form(" %s", str_coef[j].Data()) << nonlinear_correction[i][j]; 
	  }
	  jout << endl;
	}
      }
    }

    DEvent::GetCalib(event, "FCAL/block_to_square", block_to_square);
    if (block_to_square.size() > 0) {
      if (debug_level > 0) {
	for (int i = 0; i < (int) block_to_square.size(); i ++) {
	  jout << block_to_square[i];
	}
	jout << endl;
      }
    }
    
    DEvent::GetCalib(event, "FCAL/nonlinear_correction_cpp", nonlinear_correction_cpp);
    if (nonlinear_correction_cpp.size() > 0) {
      m_beamSpotX = beam_spot.at("x");
      m_beamSpotY = beam_spot.at("y");
      if (debug_level > 0) {
	for (int i = 0; i < (int) nonlinear_correction_cpp.size(); i ++) {
	  jout << nonlinear_correction_cpp[i];
	}
	jout << endl;
      }
    }
  }
  if (LOAD_TIMING_CCDB) {
    // Get timing correction polynomial, J. Mirabelli 10/31/17
    map<string,double> timing_correction;
    jcalib->Get("FCAL/shower_timing_correction", timing_correction);
    timeConst0 = timing_correction["P0"];
    timeConst1 = timing_correction["P1"];     
    timeConst2 = timing_correction["P2"];
    timeConst3 = timing_correction["P3"];
    timeConst4 = timing_correction["P4"];

    if(debug_level>0) {
      jout << "timeConst0 = " << timeConst0 << endl;
      jout << "timeConst1 = " << timeConst1 << endl;
      jout << "timeConst2 = " << timeConst2 << endl;
      jout << "timeConst3 = " << timeConst3 << endl;
      jout << "timeConst4 = " << timeConst4 << endl;
    }
  }
  
  jerror_t result = LoadCovarianceLookupTables(event);
  if (result!=NOERROR) return; // result; // TODO: Consider throwing if result is bad
}


void DFCALShower_factory::EndRun() {
  // delete lookup tables to prevent memory leak
  for (int i=0; i<5; i++) {
    for (int j=0; j<=i; j++) {
      delete CovarianceLookupTable[i][j];
      CovarianceLookupTable[i][j] = nullptr;
    }
  }
}


//------------------
// Process
//------------------
void DFCALShower_factory::Process(const std::shared_ptr<const JEvent>& event)
{
  vector<const DFCALCluster*> fcalClusters;
  event->Get(fcalClusters);
  if(fcalClusters.size()<1)return;
 
  // Use the center of the target as an approximation for the vertex position
  // 29/03/2020 ijaegle@jlab.org add beam center in x,y
  DVector3 vertex(m_beamSpotX, m_beamSpotY, m_zTarget);
  
  vector< const DTrackWireBased* > allWBTracks;
  event->Get( allWBTracks );
  vector< const DTrackWireBased* > wbTracks = filterWireBasedTracks( allWBTracks );

  // event over list of DFCALCluster objects and calculate the "Non-linear" corrected
  // energy and position for each. We'll use a logarithmic energy-weighting to 
  // find the final position and error. 
  for( vector< const DFCALCluster* >::const_iterator clItr = fcalClusters.begin();
       clItr != fcalClusters.end();  ++clItr ){
    const DFCALCluster* cluster=*clItr;
    
    // energy weighted time provides better resolution:
    double cTime = cluster->getTimeEWeight();
  
    double zback=m_FCALfront + fcalGeom->blockLength();
    double c_effective=FCAL_C_EFFECTIVE;
    
    int channel = cluster->getChannelEmax();
    DVector2 pos=fcalGeom->positionOnFace(channel);
    
    // Get corrected energy, position, and errZ
    double Ecorrected;
    DVector3 pos_corrected;
    double errZ;
    double radius = pos.Mod();
    int ring_nb = (int) (radius / (5 * k_cm));
    GetCorrectedEnergyAndPosition( cluster, ring_nb , Ecorrected, pos_corrected, errZ, &vertex);
    
    if (Ecorrected>SHOWER_ENERGY_THRESHOLD){
      DVector3 pos_log;
      GetLogWeightedPosition( cluster, pos_log, Ecorrected, &vertex );	

      //up to this point, all times have been times at which light reaches
      //the back of the detector. Here we correct for the time that it 
      //takes the Cherenkov light to reach the back of the detector
      //so that the t reported is roughly the time of the shower at the
      //position pos_corrected	
      cTime -= ( zback - pos_corrected.Z() )/c_effective;

      //Apply time-walk correction/global timing offset
      cTime += ( timeConst0  +  timeConst1 * Ecorrected  +  timeConst2 * TMath::Power( Ecorrected, 2 ) +
		 timeConst3 * TMath::Power( Ecorrected, 3 )  +  timeConst4 * TMath::Power( Ecorrected, 4 ) );

      // Make the DFCALShower object
      DFCALShower* shower = new DFCALShower;
      
      shower->setEnergy( Ecorrected );
      if (!SHOWER_POSITION_LOG)
	shower->setPosition( pos_corrected );
      else
	shower->setPosition( pos_log );
      shower->setPosition_log( pos_log ); 
      shower->setTime ( cTime );

      FillCovarianceMatrix( shower );

      if( VERBOSE > 2 ){
	printf("FCAL shower:  }  E=%f   x=%f   y=%f   z=%f   t=%f\n",
	       shower->getEnergy(),shower->getPosition().X(),shower->getPosition().Y(),shower->getPosition().Z(),shower->getTime());
	printf("FCAL shower:   dE=%f  dx=%f  dy=%f  dz=%f  dt=%f\n",
	       shower->EErr(),shower->xErr(),shower->yErr(),shower->zErr(),shower->tErr());
	printf("FCAL shower:   Ex=%f  Ey=%f  Ez=%f  Et=%f  xy=%f\n",
	       shower->EXcorr(),shower->EYcorr(),shower->EZcorr(),shower->ETcorr(),shower->XYcorr());
	printf("FCAL shower:   xz=%f  xt=%f  yz=%f  yt=%f  zt=%f\n",
	       shower->XZcorr(),shower->XTcorr(),shower->YZcorr(),shower->YTcorr(),shower->ZTcorr());
      }

      // now fill information related to shower shape and nearby
      // tracks -- useful for splitoff rejection later

      double docaTr = 1E6;
      double timeTr = 1E6;
      double xTr = 0;
      double yTr = 0;

      double flightTime;
      DVector3 projPos, projMom;

      // find the closest track to the shower -- here we event over the best FOM
      // wire-based track for every track candidate not just the ones associated
      // with the topology
      for( size_t iTrk = 0; iTrk < wbTracks.size(); ++iTrk ){

	if( !wbTracks[iTrk]->GetProjection( SYS_FCAL, projPos, &projMom, &flightTime ) ) continue;
	
	// need to swim fcalPos to common z for DOCA calculation -- this really
	// shouldn't be in the event if the z-value of projPos doesn't change
	// with each track
	
	DVector3 fcalFacePos = ( shower->getPosition() - vertex );
	//if (SHOWER_POSITION_LOG) fcalFacePos = ( shower->getPosition_log() - vertex );
	fcalFacePos.SetMag( fcalFacePos.Mag() * projPos.Z() / fcalFacePos.Z() );
 
	double distance = ( fcalFacePos - projPos ).Mag();
	
	if( distance < docaTr ){

	  docaTr = distance;
	  // this is the time from the center of the target to the detector -- to compare with
	  // the FCAL time, one needs to have the t0RF at the center of the target.  That
	  // comparison happens at a later stage in the analysis.
	  timeTr = ( wbTracks[iTrk]->position().Z() - vertex.Z() ) / SPEED_OF_LIGHT + flightTime;
	  xTr = projPos.X();
	  yTr = projPos.Y();
	}
      }

      shower->setDocaTrack( docaTr );
      shower->setTimeTrack( timeTr );

      // now compute some variables at the hit level
      unsigned int num_hits=cluster->GetNHits();
      shower->setNumBlocks(num_hits);
      
      // Get (E,x,y) for each hit in the cluster
      const vector<DFCALCluster::DFCALClusterHit_t>hits=cluster->GetHits();

      double e9e25, e1e9;      
      getE1925FromHits(hits, e1e9, e9e25);
      shower->setE1E9( e1e9 );
      shower->setE9E25( e9e25 );

      double sumU = 0;
      double sumV = 0;
      // if there is no nearest track, the defaults for xTr and yTr will result
      // in using the beam axis as the directional axis
      //if (!SHOWER_POSITION_LOG)
      getUVFromHits( sumU, sumV, hits,
		     DVector3( shower->getPosition().X(), shower->getPosition().Y(), 0 ),
		     DVector3( xTr, yTr, 0 ) );
      //else
      //getUVFromHits( sumU, sumV, fcalHits,
      //	       DVector3( shower->getPosition_log().X(), shower->getPosition_log().Y(), 0 ),
      //	       DVector3( xTr, yTr, 0 ) );

      shower->setSumU( sumU );
      shower->setSumV( sumV );
      
      shower->AddAssociatedObject( cluster );

      // If the FCAL-2 insert is installed, flag if any of the hits in the
      // cluster are near the FCAL-ECAL interface.
      shower->setIsNearBorder(false);
      if (haveInsert){
	int min_row=1000,min_col=1000,max_row=0,max_col=0;
	for (size_t j=0;j<hits.size();j++){
	  int row=fcalGeom->row(hits[j].ch);
	  int col=fcalGeom->column(hits[j].ch);
	  if (row<min_row) min_row=row;
	  if (col<min_col) min_col=col;
	  if (row>max_row) max_row=row;
	  if (col>max_col) max_col=col;
	}
	if (max_row>=18 && min_row<=40 && max_col>=18 && min_col<=40){
	  shower->setIsNearBorder(true);
	}
      }

      Insert(shower);
    }
  }
}

//--------------------------------
// GetCorrectedEnergyAndPosition
//
// Non-linear and depth corrections should be fixed within DFCALShower member functions
//--------------------------------
  void DFCALShower_factory::GetCorrectedEnergyAndPosition(const DFCALCluster* cluster, int ring_nb, double &Ecorrected, DVector3 &pos_corrected, double &errZ, const DVector3 *vertex)
{
  // Non-linear energy correction are done here
  //int MAXITER = 1000;

  DVector3  posInCal = cluster->getCentroid();
  int block = cluster->getChannelEmax();
  int square_nb = -1;
  if (USE_CPP_E_CORRECTION)
    square_nb = block_to_square[block];
  
  float x0 = posInCal.Px();
  float y0 = posInCal.Py();
  double Eclust = cluster->getEnergy();
  
  double Ecutoff = 0;
  double A = 0;
  double B = 0;
  double C = 0;
  double D = 0;
  double E = 0;
  double F = 0;
  double G = 0;
  double H = 0;
  double I = 0;
  double Egamma = Eclust;
  Ecorrected = 0;

  // block properties
  double radiation_length=FCAL_RADIATION_LENGTH;
  double shower_offset=FCAL_SHOWER_OFFSET;
  double critical_energy=FCAL_CRITICAL_ENERGY;
  double zfront=m_FCALfront;

  // 06/04/2020 ijaegle@jlab.org allows two different energy dependence correction
  if (USE_RING_E_CORRECTION_V1 && energy_dependence_correction_vs_ring.size() > 0) {
    // Method II: PRIMEXD way, correction per ring
    Egamma=Eclust; // Initialize, before correction
    int ring_region = -1;
    if (0 <= ring_nb && ring_nb <= 2)
      ring_region = 0;
    else if (3 <= ring_nb && ring_nb <= 4)
      ring_region = 1;
    else if (ring_nb == 5)
      ring_region = 2;
    else if (6 <= ring_nb && ring_nb <= 7)
      ring_region = 3;
    else if (8 <= ring_nb && ring_nb <= 9)
      ring_region = 4;
    else if (10 <= ring_nb && ring_nb <= 11)
      ring_region = 5;
    else if (12 <= ring_nb && ring_nb <= 17)
	ring_region = 6;
    else if (18 <= ring_nb && ring_nb <= 20)
	ring_region = 7;
    else if (21 <= ring_nb && ring_nb <= 23)
      ring_region = 8;
    if (ring_region != -1) {	
      Egamma = 0;
      A = energy_dependence_correction_vs_ring[ring_region][0];
      B = energy_dependence_correction_vs_ring[ring_region][1];
      C = energy_dependence_correction_vs_ring[ring_region][2];
      //D = energy_dependence_correction_vs_ring[ring_nb][3];
      //E = energy_dependence_correction_vs_ring[ring_nb][4];
      //F = energy_dependence_correction_vs_ring[ring_nb][5];
      //Egamma = Eclust / (A + B * Eclust + C * pow(Eclust, 2) + D * pow(Eclust, 3) + E * pow(Eclust, 4) + F * pow(Eclust, 5)); 
      //Egamma = Eclust / (A + B * Eclust + C * pow(Eclust, 2)); 
      Egamma = Eclust / (A - exp(-B * Eclust + C)); 
    }
  } else if (USE_RING_E_CORRECTION_V2 && nonlinear_correction.size() > 0) {
    // Method III: E/P method, correction per for the first 4 then one correction for ring 5 to 23
    Egamma=Eclust; // Initialize, before correction
    int ring_region = -1;
    if (ring_nb == 1)
      ring_region = 0;
    else if (ring_nb == 2)
      ring_region = 1;
    else if (ring_nb == 3)
      ring_region = 2;
    else if (ring_nb == 4)
      ring_region = 3;
    else if (5 <= ring_nb)
      ring_region = 4;
    if (ring_region != -1) {	
      Egamma = 0;
      A = nonlinear_correction[ring_region][0];
      B = nonlinear_correction[ring_region][1];
      C = nonlinear_correction[ring_region][2];
      D = nonlinear_correction[ring_region][3];
      E = nonlinear_correction[ring_region][4];
      F = nonlinear_correction[ring_region][5];
      G = nonlinear_correction[ring_region][6];
      H = nonlinear_correction[ring_region][7];
      I = nonlinear_correction[ring_region][8];
      //[0]-[1]*exp(-[2]*x+[3]) -[4]/([5]+[6]*exp(-x*[7]+[8])
      Egamma = Eclust / (A - B * exp(-C * Eclust + D) - E / (F + G * exp(-Eclust * H + I))); 
    }
    // End Correction method III     
  } else if (LOAD_NONLIN_CCDB && !USE_RING_E_CORRECTION_V2 && !USE_RING_E_CORRECTION_V1) {
    // Method I: IU way, one overall correction
    Egamma = 0;
    Ecutoff = cutoff_energy;
    A = linfit_slope;
    B = linfit_intercept;
    C = expfit_param1;
    D = expfit_param2;
    E = expfit_param3;
    // 06/02/2016 Shower Non-linearity Correction by Adesh. 
    // 29/03/2020 ijaegle@jlab.org the linear part correction is applied in (some) data/sim. backward comptability?
    if ( Eclust <= Ecutoff ) { 
      
      Egamma = Eclust / (A * Eclust + B); // Linear part
      
    } else {
      // 29/03/2020 ijaegle@jlab.org this correction is always applied if all C=2 & D=E=0 then Egamma = Eclust
      // if all C=D=E=0 by mistake then Egamma = - Eclust
      Egamma = Eclust / (C - exp(-D * Eclust + E)); // Non-linear part
    }
    //cout <<"Eclust " << Eclust << " Egamma " << Egamma << " A " << A << " B " << B << " C " << C << " D " << D << " E " << E << endl;
  } // End Correction method I
  
  if (USE_CPP_E_CORRECTION && !USE_RING_E_CORRECTION_V2 && !USE_RING_E_CORRECTION_V1) {
    double scalef = nonlinear_correction_cpp[0];
    if (square_nb >= 0 && square_nb <= 13) {
      double Eshift = 0;
      if (square_nb == 10) {
	Eshift = atan(nonlinear_correction_cpp[4 + square_nb * 3] * Egamma + nonlinear_correction_cpp[5 + square_nb * 3]);
	Eshift *= nonlinear_correction_cpp[3 + square_nb * 3] * Eshift;
      } else {
	Eshift = nonlinear_correction_cpp[3 + square_nb * 3] * atan(nonlinear_correction_cpp[4 + square_nb * 3] * Egamma + nonlinear_correction_cpp[5 + square_nb * 3]);
      }
      Eshift = scalef * Eshift;
      if (Eshift > 0.) {
	Egamma *= (1. + nonlinear_correction_cpp[1] * 1.e-2 * Egamma + nonlinear_correction_cpp[2] * 1.e-2 * Egamma * Egamma) / Eshift;
      } else {
	if (VERBOSE > 3) jerr << "CPP nonlinear correction has a wrong Eshift" << endl;
      } 
    } else {
      if (VERBOSE > 3) jerr << "CPP nonlinear correction has no square_nb" << endl;
    }
  }
  //End energy dependence correction
  
  if (Egamma <= 0 && Eclust > 0) Egamma = Eclust; 
  
  // then depth corrections 
  if ( Egamma > 0 ) { 
    float dxV = x0-vertex->X();
    float dyV = y0-vertex->Y();
    float zV = vertex->Z();
   
    double z0 = zfront - zV;
    double zMax = radiation_length*(shower_offset+log(Egamma/critical_energy));

    double zed = z0;
    double zed1 = z0 + zMax;

    double r0 = sqrt(dxV*dxV + dyV*dyV );

    int niter;
    for ( niter=0; niter<100; niter++) {
      double tt = r0/zed1;
      zed = z0 + zMax/sqrt( 1 + tt*tt );
      if ( fabs( (zed-zed1) ) < 0.001) {
	break;
      }
      zed1 = zed;
    }
    
    posInCal.SetZ( zed + zV );
    errZ = zed - zed1;
    
  }
  
  Ecorrected = Egamma;
  pos_corrected = posInCal;

}



jerror_t
DFCALShower_factory::FillCovarianceMatrix(DFCALShower *shower){
  /// This function takes a FCALShower object and using the internal variables
  /// overwrites any existing covaraince matrix using lookup tables.

  // Get edges of lookup table histograms (assume that all histograms have the same limits.)
  TAxis *xaxis = CovarianceLookupTable[0][0]->GetXaxis();
  TAxis *yaxis = CovarianceLookupTable[0][0]->GetYaxis();
  float minElookup = xaxis->GetBinLowEdge(1);
  float maxElookup = xaxis->GetBinUpEdge(xaxis->GetNbins());
  float minthlookup = yaxis->GetBinLowEdge(1);
  float maxthlookup = yaxis->GetBinUpEdge(yaxis->GetNbins());

  float shower_E = shower->getEnergy();
  float shower_x = shower->getPosition().X();
  float shower_y = shower->getPosition().Y();
  float shower_z = shower->getPosition().Z();
  //if (SHOWER_POSITION_LOG) {
  //shower_x = shower->getPosition_log().X();
  //shower_y = shower->getPosition_log().Y();
  //shower_z = shower->getPosition_log().Z();
  //}
  float shower_r = sqrt(shower_x*shower_x + shower_y*shower_y);
  float shower_theta = atan2(shower_r,shower_z);
  float thlookup = shower_theta/3.14159265*180;
  float Elookup = shower_E;

  // Adjust values: in order to use Interpolate() must be within histogram range
  if (Elookup<minElookup) Elookup=minElookup;
  if (Elookup>maxElookup) Elookup=maxElookup-0.0001; // move below edge, on edge doesn't work.
  if (thlookup<minthlookup) thlookup=minthlookup;
  if (thlookup>maxthlookup) thlookup=maxthlookup-0.0001;
  if (VERBOSE>3) printf("(%f,%F)    limits (%f,%f)  (%f,%f)\n",Elookup,thlookup,minElookup,maxElookup,minthlookup,maxthlookup);

  DMatrixDSym ErphiztCovariance(5);
  for (int i=0; i<5; i++) {
    for (int j=0; j<=i; j++) {
      float val = CovarianceLookupTable[i][j]->Interpolate(Elookup, thlookup);
      if (i==0 && j==0) val *= shower_E; // E variance is divided by energy in CCDB
      ErphiztCovariance(i,j) = ErphiztCovariance(j,i) = val;
    }
  }

  float shower_phi = atan2(shower_y,shower_x);
  float cosPhi = cos(shower_phi);
  float sinPhi = sin(shower_phi);
  DMatrix rotationmatrix(5,5);
  rotationmatrix(0,0) = 1;
  rotationmatrix(3,3) = 1;
  rotationmatrix(4,4) = 1;
  rotationmatrix(1,1) = cosPhi;
  rotationmatrix(1,2) = -sinPhi;
  rotationmatrix(2,1) = sinPhi;
  rotationmatrix(2,2) = cosPhi;

  if (VERBOSE>3) {printf("(E,r,phi,z,t)  "); ErphiztCovariance.Print(); }
  DMatrixDSym &D = ErphiztCovariance.Similarity(rotationmatrix);
  for (int i=0; i<5; i++) {
    for (int j=0; j<5; j++)
      shower->ExyztCovariance(i, j) = D(i, j);
  }
  if (VERBOSE>2) {printf("(E,x,y,z,t)    "); shower->ExyztCovariance.Print(); }

  return NOERROR;
}


jerror_t
DFCALShower_factory::LoadCovarianceLookupTables(const std::shared_ptr<const JEvent>& event){
  auto runnumber = event->GetRunNumber();
  auto app = event->GetJApplication();
  auto calibration = app->GetService<JCalibrationManager>()->GetJCalibration(runnumber);
  auto root_lock = app->GetService<JLockService>();

  std::thread::id this_id = std::this_thread::get_id();
  stringstream idstring;
  idstring << this_id;
  if (VERBOSE>0) printf("DFCALShower_factory::LoadCovarianceLookupTables():  Thread %s\n",idstring.str().c_str());

  bool USECCDB=0;
  bool DUMMYTABLES=0;
  // if filename specified try to use filename else get info from CCDB
  if (COVARIANCEFILENAME == "") USECCDB=1;

  map<string,string> covariance_data;
  if (USECCDB) {
    // load information for covariance matrix
    if (calibration->Get("/FCAL/shower_covariance", covariance_data)) {
      jerr << "Error loading /FCAL/shower_covariance !" << endl;
      DUMMYTABLES=1;
    }
    if (covariance_data.size() == 15)  {  // there are 15 elements in the covariance matrix
      // for example, print it all out
      if (VERBOSE>0) {
	for(auto element : covariance_data) {
	  cout << "\nTEST:   " << element.first << " = " << element.second << endl;
	}
      }
    } else {
      jerr << "Wrong number of elements /FCAL/shower_covariance !" << endl;
      DUMMYTABLES=1;
    }
  }

  for (int i=0; i<5; i++) {
    for (int j=0; j<=i; j++) {

      root_lock->RootWriteLock();
      // change directory to memory so that histograms are not saved to file
      TDirectory *savedir = gDirectory;

      char histname[255];
      sprintf(histname,"covariance_%i%i_thread%s",i,j,idstring.str().c_str());
      // Read in string
      ifstream ifs;
      string line;
      stringstream ss;
      if (USECCDB) {
	stringstream matrixname;
	matrixname << "covmatrix_" << i << j;
	if (VERBOSE>1) cout << "Using CCDB \"" << matrixname.str() << "\"  " << covariance_data[matrixname.str()] << endl;
	ss.str(covariance_data[matrixname.str()]);
      } else {
	char filename[255];
	sprintf(filename,"%s%i%i.txt",COVARIANCEFILENAME.c_str(),i,j);
	if (VERBOSE>0) cout  << filename << std::endl;
	ifs.open(filename);
	if (! ifs.is_open()) {
	  jerr << " Error: Cannot open file! " << filename << std::endl;
	  DUMMYTABLES=1;
	} else {
	  getline(ifs, line, '\n');
	  ss.str(line);
	  if (VERBOSE>1) cout << filename << " dump: " <<line<<endl;
	}
      }
      if (DUMMYTABLES) {
	// create dummy histogram since something went wrong
	CovarianceLookupTable[i][j] = new TH2F(histname,"Covariance histogram",10,0,12,10,0,12);
	CovarianceLookupTable[i][j]->SetDirectory(nullptr);
      } else {
	// Parse string
	int nxbins, nybins;
	ss>>nxbins;
	ss>>nybins;
	if (VERBOSE>1) printf("parsed dump: bins (%i,%i)\n",nxbins,nybins);
	Float_t xbins[nxbins+1];
	Float_t ybins[nybins+1];
	for (int count=0; count<=nxbins; count++) {
	  ss>>xbins[count];
	  if (VERBOSE>1) printf("(%i,%f)  ",count,xbins[count]);
	}
	if (VERBOSE>1) printf("\n");
	for (int count=0; count<=nybins; count++) {
	  ss>>ybins[count];
	  if (VERBOSE>1) printf("(%i,%f)  ",count,ybins[count]);
	}
	if (VERBOSE>1) printf("\n");
	int xbin=1;
	double cont;
	int ybin=1;
	// create histogram
	CovarianceLookupTable[i][j] = new TH2F(histname,"Covariance histogram",nxbins,xbins,nybins,ybins);
	CovarianceLookupTable[i][j]->SetDirectory(nullptr);
	// fill histogram
	while(ss>>cont){
	  if (VERBOSE>1) printf("(%i,%i) (%i,%i) %e  ",i,j,xbin,ybin,cont);
	  CovarianceLookupTable[i][j]->SetBinContent(xbin,ybin,cont);
	  ybin++;
	  if (ybin>nybins) { xbin++; ybin=1; }
	}
	if (VERBOSE>1) printf("\n");
	// Close file
	ifs.close();
      }
      savedir->cd();
      root_lock->RootUnLock();
    }
  }
  return NOERROR;
}

unsigned int
DFCALShower_factory::getMaxHit(int chan_Emax, const vector< const DFCALHit* >& hitVec ) const {
  
  unsigned int maxIndex = 0;
  
  for( vector< const DFCALHit* >::const_iterator hit = hitVec.begin();
       hit != hitVec.end(); ++hit ){
    if (fcalGeom->channel((**hit).row,(**hit).column)==chan_Emax){
      maxIndex = hit - hitVec.begin();
      break;
    }
  }
  return maxIndex;
}


unsigned int
DFCALShower_factory::getMaxHit( const vector< const DFCALHit* >& hitVec ) const {
  
  unsigned int maxIndex = 0;
  
  double eMaxSh = 0;
  
  for( vector< const DFCALHit* >::const_iterator hit = hitVec.begin();
       hit != hitVec.end(); ++hit ){

    if( (**hit).E > eMaxSh ){

      eMaxSh = (**hit).E;
      maxIndex = hit - hitVec.begin();
    }
  }

  return maxIndex;
}

void
DFCALShower_factory::getUVFromHits( double& sumUSh, double& sumVSh, 
				    const vector<DFCALCluster::DFCALClusterHit_t>& hits,
				    const DVector3& showerVec,
				    const DVector3& trackVec ) const {

  // This method forms an axis pointing from the shower to nearest track
  // and computes the energy-weighted second moment of the shower along
  // and perpendicular to this axis.  True photons are fairly symmetric
  // and have similar values of sumU and sumV whereas splitoffs tend
  // to be asymmetric in these variables.

  DVector3 u = ( showerVec - trackVec ).Unit();
  DVector3 z( 0, 0, 1 );
  DVector3 v = u.Cross( z );

  DVector3 hitLoc( 0, 0, 0 );

  sumUSh = 0;
  sumVSh = 0;

  double sumE = 0;
  
  for( vector<DFCALCluster::DFCALClusterHit_t>::const_iterator hit = hits.begin();
       hit != hits.end(); ++hit ){

    hitLoc.SetX( (*hit).x - showerVec.X() );
    hitLoc.SetY( (*hit).y - showerVec.Y() );

    sumUSh += (*hit).E * pow( u.Dot( hitLoc ), 2 );
    sumVSh += (*hit).E * pow( v.Dot( hitLoc ), 2 );

    sumE += (*hit).E;
  }

  sumUSh /= sumE;
  sumVSh /= sumE;
}

void DFCALShower_factory::getE1925FromHits(const vector<DFCALCluster::DFCALClusterHit_t>&hits,
					   double& e1e9Sh, double& e9e25Sh) const {
  unsigned int maxIndex=0;
  double maxE=0;
  for (unsigned int i=0;i<hits.size();i++){
    if (hits[i].E>maxE){
      maxE=hits[i].E;
      maxIndex=i;
    }
  }
  
  double E9 = 0;
  double E25 = 0;
  double E9cut=4.5,E25cut=8.5;

  const DFCALCluster::DFCALClusterHit_t maxHit = hits[maxIndex];
  for( vector<DFCALCluster::DFCALClusterHit_t>::const_iterator hit = hits.begin();
       hit != hits.end(); ++hit ){  
    if(fabs((*hit).x - maxHit.x) < E9cut && fabs((*hit).y - maxHit.y) < E9cut )
      E9 += (*hit).E;      
    if(fabs((*hit).x - maxHit.x) < E25cut && fabs((*hit).y - maxHit.y) < E25cut)
      E25 += (*hit).E;
  }

  e1e9Sh = maxE/E9;
  e9e25Sh = E9/E25;
  
}

vector< const DTrackWireBased* >
DFCALShower_factory::filterWireBasedTracks( vector< const DTrackWireBased* >& wbTracks ) const {

  vector< const DTrackWireBased* > finalTracks;
  map< unsigned int, vector< const DTrackWireBased* > > sortedTracks;

  // first sort the wire based tracks into lists with a common candidate id
  // this means that they all come from the same track in the detector
  
  for( unsigned int i = 0; i < wbTracks.size(); ++i ){

    unsigned int id = wbTracks[i]->candidateid;

    if( sortedTracks.find( id ) == sortedTracks.end() ){
      
      sortedTracks[id] = vector< const DTrackWireBased* >();
    }

    sortedTracks[id].push_back( wbTracks[i] );
  }

  // now event through that list of unique tracks and for each set
  // of wire based tracks, choose the one with the highest FOM
  // (this is choosing among different particle hypotheses)
  
  for( map< unsigned int, vector< const DTrackWireBased* > >::const_iterator
	 anId = sortedTracks.begin();
       anId != sortedTracks.end(); ++anId ){

    double maxFOM = 0;
    unsigned int bestIndex = 0;

    for( unsigned int i = 0; i < anId->second.size(); ++i ){

      if( anId->second[i]->Ndof < 15 ) continue;
      
      if( anId->second[i]->FOM > maxFOM ){

	maxFOM = anId->second[i]->FOM;
	bestIndex = i;
      }
    }

    finalTracks.push_back( anId->second[bestIndex] );
  }
  
  return finalTracks;
}


void DFCALShower_factory::GetLogWeightedPosition( const DFCALCluster* cluster, DVector3 &pos_log, double Egamma, const DVector3 *vertex )
{
  
  DVector3  posInCal = cluster->getCentroid();
  
  vector<DFCALCluster::DFCALClusterHit_t> locHitVector=cluster->GetHits();
    
  //------   Loop over hits   ------//
  
  double sW    =  0.0;
  double xpos  =  0.0;
  double ypos  =  0.0;
  double W;
  
  double ecluster = cluster->getEnergy();
  
  for( int ih = 0; ih < (int)locHitVector.size(); ih++ ) {
	
	double xcell = locHitVector[ih].x;
	double ycell = locHitVector[ih].y;
	double ecell = locHitVector[ih].E;
	
	W  =  log_position_const + log( ecell / ecluster );
	if( W > 0. ) {
		sW    +=  W;
		xpos  +=  xcell * W;
		ypos  +=  ycell * W;
	}
  }
  
  double x1, y1;
  if( sW ) {
  	x1  =  xpos / sW;
	y1  =  ypos / sW;
  } else {
  	cout << "\nBad Cluster Logged in DFCALShower_factory::GetLogWeightedPosition" << endl;
	x1  =  0.;
	y1  =  0.;
  }
  
  
  // Shower Depth Corrections (copied from GetCorrectedEnergyAndPosition function)

  if ( Egamma > 0 ) { 
    float dxV   = x1 - vertex->X();
    float dyV   = y1 - vertex->Y();
    float  zV   = vertex->Z();
    
    double z0   = m_FCALfront - zV;
    double zMax = FCAL_RADIATION_LENGTH*(FCAL_SHOWER_OFFSET 
					 + log(Egamma/FCAL_CRITICAL_ENERGY));
    
    double zed  = z0;
    double zed1 = z0 + zMax;
    
    double r0 = sqrt( dxV*dxV + dyV*dyV );
    
    int niter;
    for ( niter=0; niter<100; niter++) {
      double tt = r0/zed1;
      zed = z0 + zMax/sqrt( 1 + tt*tt );
      if ( fabs( (zed-zed1) ) < 0.001) {
	break;
      }
      zed1 = zed;
    }
    
    posInCal.SetZ( zed + zV );
  }
  
  posInCal.SetX( x1 );
  posInCal.SetY( y1 );
  
  
  pos_log = posInCal;
  
  
  return;
  
}


