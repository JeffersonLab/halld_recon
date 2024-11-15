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
#include <JANA/Calibrations/JCalibrationGeneratorCCDB.h>

#include "hddm_r.hpp"

#include <PID/DMCReaction.h>
#include <PID/DBeamPhoton.h>
#include <PID/DDetectorMatches.h>
#include "TRACKING/DMCThrown.h"
#include <TRACKING/DTrackTimeBased.h>
#include <FCAL/DFCALShower.h>
#include <FCAL/DFCALShower_factory.h>
#include <FCAL/DFCALHit.h>
#include <CCAL/DCCALShower.h>
#include <BCAL/DBCALShower.h>
#include <BCAL/DBCALShower_factory_IU.h>
#include <START_COUNTER/DSCHit.h>
#include <TOF/DTOFPoint.h>
#include <FMWPC/DCTOFPoint.h>
#include <FMWPC/DFMWPCHit.h>
#include <TRIGGER/DTrigger.h>
#include <DANA/DEvent.h>
#include <RF/DRFTime.h>
#include <DIRC/DDIRCPmtHit.h>
#include <DIRC/DDIRCTruthBarHit.h>
#include <PID/DParticleID.h>
#include <TAGGER/DTAGMHit.h>
#include <TAGGER/DTAGHHit.h>
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
   DEventSourceREST(std::string source_name, JApplication* app);
   virtual ~DEventSourceREST();

		void GetEvent(std::shared_ptr<JEvent> event) override;
		bool GetObjects(const std::shared_ptr<const JEvent> &event, JFactory *factory) override;
		void FinishEvent(JEvent &event) override;
		
   bool Extract_DMCReaction(hddm_r::HDDM *record,
                    JFactoryT<DMCReaction> *factory, const std::shared_ptr<const JEvent>& locEvent);
   bool Extract_DRFTime(hddm_r::HDDM *record,
                    JFactoryT<DRFTime> *factory, const std::shared_ptr<const JEvent>& locEvent);
   bool Extract_DBeamPhoton(hddm_r::HDDM *record,
                    JFactoryT<DBeamPhoton> *factory,
                    const std::shared_ptr<const JEvent>& locEvent);
   bool Extract_DMCThrown(hddm_r::HDDM *record,
                    JFactoryT<DMCThrown> *factory);
   bool Extract_DSCHit(hddm_r::HDDM *record,
                    JFactoryT<DSCHit>* factory);
   bool Extract_DTOFPoint(hddm_r::HDDM *record,
                    JFactoryT<DTOFPoint>* factory); 
   bool Extract_DCTOFPoint(hddm_r::HDDM *record,
                    JFactoryT<DCTOFPoint>* factory);
   bool Extract_DFCALShower(hddm_r::HDDM *record,
                    JFactoryT<DFCALShower>* factory);
   bool Extract_DBCALShower(hddm_r::HDDM *record,
                    JFactoryT<DBCALShower>* factory);
   bool Extract_DCCALShower(hddm_r::HDDM *record,
                    JFactoryT<DCCALShower>* factory);
   bool Extract_DTrackTimeBased(hddm_r::HDDM *record,
                    JFactoryT<DTrackTimeBased>* factory, const std::shared_ptr<const JEvent>& locEvent);
   bool Extract_DTrigger(hddm_r::HDDM *record,
                    JFactoryT<DTrigger>* factory);
   bool Extract_DDetectorMatches(const std::shared_ptr<const JEvent>& locEvent, hddm_r::HDDM *record,
                    JFactoryT<DDetectorMatches>* factory);
#if 0
   bool Extract_DRFTime(hddm_r::HDDM *record,
                    JFactoryT<DRFTime>* factory);
#endif
   bool Extract_DDIRCPmtHit(hddm_r::HDDM *record,
                    JFactoryT<DDIRCPmtHit>* factory, const std::shared_ptr<const JEvent>& locEvent);
   bool Extract_DFMWPCHit(hddm_r::HDDM *record,
   JFactoryT<DFMWPCHit>* factory, const std::shared_ptr<const JEvent>& locEvent);
   bool Extract_DFCALHit(hddm_r::HDDM *record,
   JFactoryT<DFCALHit>* factory, const std::shared_ptr<const JEvent>& locEvent);
   bool Extract_DEventHitStatistics(hddm_r::HDDM *record,
                    JFactoryT<DEventHitStatistics> *factory);

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
   
   string REST_JANA_CALIB_CONTEXT = "";
   JCalibrationGeneratorCCDB *calib_generator;
   
   	map<unsigned int, JCalibration *> dJCalib_olds; //unsigned int is run number
   	map<unsigned int, DTAGHGeometry *> dTAGHGeoms; //unsigned int is run number
   	map<unsigned int, DTAGMGeometry *> dTAGMGeoms; //unsigned int is run number

	//map<unsigned int, double *> dTAGHCounterQualities;
	map<unsigned int, double [TAGH_MAX_COUNTER+1]> dTAGHCounterQualities;
	//map<unsigned int, double **> dTAGMFiberQualities;
	map<unsigned int, double [TAGM_MAX_ROW+1][TAGM_MAX_COLUMN+1]> dTAGMFiberQualities;
	
	

   	std::mutex readMutex;

};

#endif //_JEVENT_SOURCEREST_H_
