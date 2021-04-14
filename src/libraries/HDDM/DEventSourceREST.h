//
// Author: Richard Jones  June 29, 2012
//
// DEventSourceREST
//
/// Implements JEventSource for REST files

#ifndef _JEVENT_SOURCEREST_H_
#define _JEVENT_SOURCEREST_H_

#include <vector>
#include <string>

#include <pthread.h>

#include <JANA/JEventSource.h>
#include <JANA/jerror.h>
#include <JANA/JCalibration.h>

#include "hddm_r.hpp"

#include <PID/DMCReaction.h>
#include <PID/DBeamPhoton.h>
#include <PID/DDetectorMatches.h>
#include "TRACKING/DMCThrown.h"
#include <TRACKING/DTrackTimeBased.h>
#include <FCAL/DFCALShower.h>
#include <FCAL/DFCALShower_factory.h>
#include <CCAL/DCCALShower.h>
#include <BCAL/DBCALShower.h>
#include <BCAL/DBCALShower_factory_IU.h>
#include <START_COUNTER/DSCHit.h>
#include <TOF/DTOFPoint.h>
#include <TRIGGER/DTrigger.h>
#include <DANA/DApplication.h>
#include <RF/DRFTime.h>
#include <DIRC/DDIRCPmtHit.h>
#include <DIRC/DDIRCTruthBarHit.h>
#include <PID/DParticleID.h>
#include <TAGGER/DTAGMGeometry.h>
#include <TAGGER/DTAGHGeometry.h>
#include <HDDM/DEventHitStatistics.h>
#include "DResourcePool.h"

#include <TMatrixF.h>
#include <DMatrix.h>
#include <TMath.h>

class DEventSourceREST:public JEventSource
{
 public:
   DEventSourceREST(const char* source_name);
   virtual ~DEventSourceREST();		
   virtual const char* className(void) {
      return DEventSourceREST::static_className();
   }
   static const char* static_className(void) {
      return "DEventSourceREST";
   }

   jerror_t GetEvent(JEvent &event);
   void FreeEvent(JEvent &event);
   jerror_t GetObjects(JEvent &event, JFactory_base *factory);
		
   jerror_t Extract_DMCReaction(hddm_r::HDDM *record,
                    JFactory<DMCReaction> *factory, JEventLoop* locEventLoop);
   jerror_t Extract_DRFTime(hddm_r::HDDM *record,
                    JFactory<DRFTime> *factory, JEventLoop* locEventLoop);
   jerror_t Extract_DBeamPhoton(hddm_r::HDDM *record,
                    JFactory<DBeamPhoton> *factory,
                    JEventLoop *eventLoop);
   jerror_t Extract_DMCThrown(hddm_r::HDDM *record,
                    JFactory<DMCThrown> *factory);
   jerror_t Extract_DSCHit(hddm_r::HDDM *record,
                    JFactory<DSCHit>* factory);
   jerror_t Extract_DTOFPoint(hddm_r::HDDM *record,
                    JFactory<DTOFPoint>* factory);
   jerror_t Extract_DFCALShower(hddm_r::HDDM *record,
                    JFactory<DFCALShower>* factory);
   jerror_t Extract_DBCALShower(hddm_r::HDDM *record,
                    JFactory<DBCALShower>* factory);
   jerror_t Extract_DCCALShower(hddm_r::HDDM *record,
                    JFactory<DCCALShower>* factory);
   jerror_t Extract_DTrackTimeBased(hddm_r::HDDM *record,
                    JFactory<DTrackTimeBased>* factory, JEventLoop* locEventLoop);
   jerror_t Extract_DTrigger(hddm_r::HDDM *record,
                    JFactory<DTrigger>* factory);
   jerror_t Extract_DDetectorMatches(JEventLoop* locEventLoop, hddm_r::HDDM *record,
                    JFactory<DDetectorMatches>* factory);
#if 0
   jerror_t Extract_DRFTime(hddm_r::HDDM *record,
                    JFactory<DRFTime>* factory);
#endif
   jerror_t Extract_DDIRCPmtHit(hddm_r::HDDM *record,
                    JFactory<DDIRCPmtHit>* factory, JEventLoop* locEventLoop);
   jerror_t Extract_DEventHitStatistics(hddm_r::HDDM *record,
                    JFactory<DEventHitStatistics> *factory);

   void Get7x7ErrorMatrix(double mass, const double vec[5], const TMatrixFSym* C5x5, TMatrixFSym* loc7x7ErrorMatrix);
 private:
   // Warning: Class JEventSource methods must be re-entrant, so do not
   // store any data here that might change from event to event.

	uint32_t Convert_SignedIntToUnsigned(int32_t locSignedInt) const;

	bool USE_CCDB_BCAL_COVARIANCE;
	bool USE_CCDB_FCAL_COVARIANCE;
	
	bool PRUNE_DUPLICATE_TRACKS;
	bool RECO_DIRC_CALC_LUT;
	int dDIRCMaxChannels;
	enum dirc_status_state {GOOD, BAD, NOISY};
	map<unsigned int, vector<vector<int>>> dDIRCChannelStatusMap; //unsigned int is run number
	
	map<unsigned int, DVector2> dBeamCenterMap,dBeamDirMap;
	map<unsigned int, double> dBeamZ0Map;

	DFCALShower_factory *dFCALShowerFactory;
	DBCALShower_factory_IU *dBCALShowerFactory;

	map<unsigned int, double> dTargetCenterZMap; //unsigned int is run number
	map<unsigned int, double> dBeamBunchPeriodMap; //unsigned int is run number
    static thread_local shared_ptr<DResourcePool<TMatrixFSym>> dResourcePool_TMatrixFSym;

   std::ifstream *ifs;		// input hddm file ifstream
   hddm_r::istream *fin;	// provides hddm layer on top of ifstream
};

#endif //_JEVENT_SOURCEREST_H_
