//most of the code was orginally written by Matt Shepherd in
//DBCALCluster_factory.cc

#include <vector>
using namespace std;

#include <JANA/JApplication.h>
#include <JANA/JEvent.h>
#include <JANA/Calibrations/JCalibrationManager.h>
#include "DANA/DGeometryManager.h"
#include "HDGEOMETRY/DGeometry.h"

#include "BCAL/DBCALPoint_factory.h"
#include "BCAL/DBCALHit.h"
#include "BCAL/DBCALGeometry.h"

#include "units.h"


//----------------
// Init
//----------------
void DBCALPoint_factory::Init() {
	PRINTCALIBRATION = false;
	auto app = GetApplication();
	app->SetDefaultParameter("BCALPOINT:PRINTCALIBRATION", PRINTCALIBRATION, "Print the calibration parameters.");
}

//----------------
// BeginRun
//----------------
void DBCALPoint_factory::BeginRun(const std::shared_ptr<const JEvent>& event) {

  auto event_number = event->GetEventNumber();
  auto run_number = event->GetRunNumber();
  auto app = GetApplication();
  auto calibration = app->GetService<JCalibrationManager>()->GetJCalibration(run_number);
  auto geom = app->GetService<DGeometryManager>()->GetDGeometry(run_number);

  // Only print messages for one thread whenever run number changes
  static pthread_mutex_t print_mutex = PTHREAD_MUTEX_INITIALIZER;
  static set<int> runs_announced;
  pthread_mutex_lock(&print_mutex);
  bool print_messages = false;
  if(runs_announced.find(run_number) == runs_announced.end()){
    print_messages = true;
    runs_announced.insert(run_number);
  }
  pthread_mutex_unlock(&print_mutex);

  geom->GetTargetZ(m_z_target_center);

  // load BCAL geometry
  vector<const DBCALGeometry *> BCALGeomVec;
  event->Get(BCALGeomVec);
  if(BCALGeomVec.size() == 0)
	throw JException("Could not load DBCALGeometry object!");
  m_BCALGeom = BCALGeomVec[0];

  if(print_messages) jout << "in DBCALPoint_factory, loading constants ..." << jendl;

  // load attenuation correction parameters 
  attenuation_parameters.clear();

  vector< vector<double> > attenuation_parameters_temp;
  calibration->Get("/BCAL/attenuation_parameters", attenuation_parameters_temp, event_number);

  // avoid potential crash ...
  for (unsigned int i = 0; i < attenuation_parameters_temp.size(); i++){
      attenuation_parameters.push_back(attenuation_parameters_temp.at(i));
  }

   if (PRINTCALIBRATION) {
       int channel = 0;
       for (int module=1; module<=BCAL_NUM_MODULES; module++) {
           for (int layer=1; layer<=BCAL_NUM_LAYERS; layer++) {
               for (int sector=1; sector<=BCAL_NUM_SECTORS; sector++) {
                   printf("%2i  %2i  %2i %12.4f %12.4f %12.4f\n",
                          module,layer,sector,
                          attenuation_parameters[channel][0],
                          attenuation_parameters[channel][1],
                          attenuation_parameters[channel][2]);
               }
               channel++;
           }
       }
   }


  // load effective velocities
  effective_velocities.clear();

  // Passing in the member vector directly was sometimes causing a crash...
  vector <double> effective_velocities_temp;
  calibration->Get("/BCAL/effective_velocities", effective_velocities_temp, event_number);

  for (unsigned int i = 0; i < effective_velocities_temp.size(); i++){
    effective_velocities.push_back(effective_velocities_temp.at(i));
  }

  // load track parameters (the parameters of the quadratic fit in histograms of z_track = f(tUp - tDown)  )
  track_parameters.clear();

  vector< vector<double> > track_parameters_temp;
  calibration->Get("/BCAL/z_track_parms", track_parameters_temp, event_number);

  // Passing in the member vector directly was sometimes causing a crash...
  for (unsigned int i = 0; i < track_parameters_temp.size(); i++){
      track_parameters.push_back(track_parameters_temp.at(i));
  }
}

//----------------
// Process
//----------------
void DBCALPoint_factory::Process(const std::shared_ptr<const JEvent>& event) {

  vector<const DBCALUnifiedHit*> hits;
  event->Get(hits);
  if (hits.size() <= 0) return;

  // first arrange the list of hits so they are grouped by cell
  map< int, cellHits > cellHitMap;
  for( vector< const DBCALUnifiedHit* >::const_iterator hitPtr = hits.begin();
       hitPtr != hits.end();
       ++hitPtr ){
    
    const DBCALUnifiedHit& hit = (**hitPtr);
    
    int id = m_BCALGeom->cellId( hit.module, hit.layer, hit.sector );

    // Add hit to appropriate list for this cell
    if(hit.end == m_BCALGeom->kUpstream){
      cellHitMap[id].uphits.push_back( *hitPtr );
    }else{
      cellHitMap[id].dnhits.push_back( *hitPtr );
    }
  }

  // now go through this list and group hits into BCAL points
  // this combines information from both ends
  for( map< int, cellHits >::const_iterator mapItr = cellHitMap.begin();
       mapItr != cellHitMap.end();
       ++mapItr ){
    
    const vector<const DBCALUnifiedHit*> &uphits = mapItr->second.uphits;
    const vector<const DBCALUnifiedHit*> &dnhits = mapItr->second.dnhits;

    //require double-ended hits
    if(uphits.size()==0 || dnhits.size()==0) continue;

    // Each SiPM sum can have multiple hits, some caused purely by
    // dark hits. A more sophisticated algorithm may be needed here
    // to decipher the multi-hit events. For now, we just take the
    // most energetic hit from each end. (Single ended hits are
    // ignored.

    const DBCALUnifiedHit *uphit=uphits[0];
    const DBCALUnifiedHit *dnhit=dnhits[0];

    for(unsigned int i=1; i<uphits.size(); i++){
      if(uphits[i]->E > uphit->E) uphit = uphits[i];
    }

    for(unsigned int i=1; i<dnhits.size(); i++){
      if(dnhits[i]->E > dnhit->E) dnhit = dnhits[i];
    }

    // first check that the hits don't have absurd timing information

    //int id = m_BCALGeom->cellId( uphit->module, uphit->layer, uphit->sector );  // key the cell identification off of the upstream cell
    int table_id = GetCalibIndex( uphit->module, uphit->layer, uphit->sector );  // key the cell identification off of the upstream cell

    // float fibLen = m_BCALGeom->GetBCAL_length();
    //float cEff = m_BCALGeom->C_EFFECTIVE;    
    float cEff = GetEffectiveVelocity(table_id);

    // get the position with respect to the center of the module -- positive
    // z in the downstream direction
    // double zLocal = 0.5 * cEff * ( uphit->t - dnhit->t );

    // if the timing information indicates that the z position is more than 60 cm outside the BCAL, likely the hit is contamined by noise or entirely noise, skip this cell
    // double tol = 60*k_cm;

    // comment out this section and move any checks on z position (i.e. timing) to clusterizer. Elton 5/3/2017
    // if (zLocal > (0.5*fibLen + tol) || zLocal < (-0.5*fibLen - tol)) continue;

    // pass attenuation length parameters to the DBCALPoint constructor, since
    // many of the calculations are implemented there
    double attenuation_length = m_BCALGeom->GetBCAL_attenutation_length();
    double attenuation_L1=-1., attenuation_L2=-1.;  // these parameters are ignored for now
    GetAttenuationParameters(table_id, attenuation_length, attenuation_L1, attenuation_L2);
    // if (GetAttenuationParameters(id, attenuation_length, attenuation_L1, attenuation_L2)) {
    //   printf("got new att length %f\n",attenuation_length);
    // } else {
    //   printf("default att length %f\n",attenuation_length);
    // }

    // pass track parameters to the DBCALPoint constructor, since
    // many of the calculations are implemented there. This should change promptly
    double track_p0 = -1.0; // will be updated from GetTrackParameters (dimensions: cm)
    double track_p1 = -1.0; // will be updated from GetTrackParameters (dimensions: cm/ns)
    double track_p2 = -100.0; // will be updated from GetTrackParameters (dimensions: cm/ns^2)
    GetTrackParameters(table_id, track_p0, track_p1, track_p2);

    DBCALPoint *point = new DBCALPoint(*uphit,*dnhit,m_z_target_center,attenuation_length,cEff,track_p0,track_p1,track_p2,m_BCALGeom);

    point->AddAssociatedObject(uphit);
    point->AddAssociatedObject(dnhit);

    Insert(point);
  }

  //Possibly we should also construct points from single-ended hits here.
  //The code for this is currently (commented out) in
  //DBCALCluster_factory.cc
}

bool DBCALPoint_factory::GetAttenuationParameters(int id, double &attenuation_length, 
						    double &attenuation_L1, double &attenuation_L2)
{
	vector<double> &parms = attenuation_parameters.at(id);

	attenuation_length = parms[0];
	attenuation_L1 = parms[1];
	attenuation_L2 = parms[2];

	return true;
}

double DBCALPoint_factory::GetEffectiveVelocity(int id)
{
	return effective_velocities.at(id);
}

bool DBCALPoint_factory::GetTrackParameters(int id, double &track_p0, 
						    double &track_p1, double &track_p2)
{
	vector<double> &z_parms = track_parameters.at(id);

	if(!z_parms.empty()){
		track_p0 = z_parms[0];
		track_p1 = z_parms[1];
		track_p2 = z_parms[2];
		return true;
	}
  	else{
  		jerr<<"Failed to retrieve the z_track parameters from CCDB!!!" << jendl;
  		exit(-1);
  	}
}
