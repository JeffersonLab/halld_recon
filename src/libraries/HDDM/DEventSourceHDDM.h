// Author: David Lawrence  June 24, 2004
//
//
// DEventSourceHDDM
//
/// Implements JEventSource for HDDM files
//
// Changes:   Oct 3, 2012 Yi Qiang: add classes for Cerenkov detector
//         OCt 10, 2012 Yi Qiang: modifed Cerenkov classes with general Cere hits
//         Oct 8, 2013 Yi Qiang: added dedicated object for RICH Truth Hit
//         June 22, 2015 J. Stevens: changed RICH -> DIRC and remove CERE
//         May 7, 2017 R. Dzhygadlo: added DDIRCTruthPmtHit DDIRCTruthBarHit
//         Jan 16, 2024 A. Somov: added functions for ECAL

#ifndef _JEVENT_SOURCEHDDM_H_
#define _JEVENT_SOURCEHDDM_H_

#include <vector>
#include <string>
using namespace std;

#include <pthread.h>

#include <JANA/JEventSource.h>
#include <JANA/Compatibility/jerror.h>
#include <JANA/Calibrations/JCalibration.h>

#include <HDDM/hddm_s.hpp>

#include "TRACKING/DMCTrackHit.h"
#include "TRACKING/DMCThrown.h"
#include "TRACKING/DMCTrajectoryPoint.h"
#include "BCAL/DBCALSiPMHit.h"
#include "BCAL/DBCALSiPMSpectrum.h"
#include "BCAL/DBCALDigiHit.h"
#include "BCAL/DBCALIncidentParticle.h"
#include "BCAL/DBCALTDCDigiHit.h"
#include "BCAL/DBCALTruthShower.h"
#include "BCAL/DBCALTruthCell.h"
#include "BCAL/DBCALGeometry.h"
#include "CDC/DCDCHit.h"
#include "FDC/DFDCHit.h"
#include "FCAL/DFCALTruthShower.h"
#include "FCAL/DFCALHit.h"
#include "ECAL/DECALTruthShower.h"
#include "ECAL/DECALHit.h"
#include "CCAL/DCCALTruthShower.h"
#include "CCAL/DCCALHit.h"
#include "TOF/DTOFTruth.h"
#include "TOF/DTOFHit.h"
#include "TOF/DTOFHitMC.h"
#include "START_COUNTER/DSCTruthHit.h"
#include "START_COUNTER/DSCHit.h"
#include <PID/DMCReaction.h>
#include <PID/DBeamPhoton.h>
#include <TRACKING/DTrackTimeBased.h>
#include <TAGGER/DTAGMHit.h>
#include <TAGGER/DTAGHHit.h>
#include <TAGGER/DTAGMGeometry.h>
#include <TAGGER/DTAGHGeometry.h>
// load CERE headers, yqiang Oct 3, 2012
// modified by yqiang, Oct 10 2012
// added RichTruthHit object, yqiang, Oct 7, 2013
#include <CERE/DCereHit.h>
#include "DIRC/DDIRCTruthBarHit.h"
#include "DIRC/DDIRCTruthPmtHit.h"
#include "DIRC/DDIRCPmtHit.h"
#include <RF/DRFTime.h>
#include "PAIR_SPECTROMETER/DPSHit.h"
#include "PAIR_SPECTROMETER/DPSCHit.h"
#include "PAIR_SPECTROMETER/DPSTruthHit.h"
#include "PAIR_SPECTROMETER/DPSCTruthHit.h"
#include "FMWPC/DFMWPCTruthHit.h"
#include "FMWPC/DFMWPCHit.h"
#include "FMWPC/DFMWPCTruth.h"
#include "FMWPC/DCTOFHit.h"
#include "FMWPC/DCTOFHit.h"
#include "FMWPC/DCTOFTruth.h"
#include "PAIR_SPECTROMETER/DPSGeometry.h"
#include "TPOL/DTPOLHit.h"
#include "TPOL/DTPOLTruthHit.h"
#include "DResourcePool.h"

class DEventSourceHDDM:public JEventSource
{
   public:
      DEventSourceHDDM(const char* source_name);
      virtual ~DEventSourceHDDM();      
      virtual const char* className(void){return static_className();}
      static const char* static_className(void){return "DEventSourceHDDM";}

      void GetEvent(std::shared_ptr<JEvent> event) override;
      void FinishEvent(JEvent &event) override;
      bool GetObjects(const std::shared_ptr<const JEvent> &event, JFactory *factory) override;
      
      jerror_t Extract_DMCTrackHit(hddm_s::HDDM *record, JFactoryT<DMCTrackHit> *factory, string tag);
      jerror_t GetCDCTruthHits(hddm_s::HDDM *record, vector<DMCTrackHit*>& data);
      jerror_t GetFDCTruthHits(hddm_s::HDDM *record, vector<DMCTrackHit*>& data);
      jerror_t GetBCALTruthHits(hddm_s::HDDM *record, vector<DMCTrackHit*>& data);
      jerror_t GetTOFTruthHits(hddm_s::HDDM *record, vector<DMCTrackHit*>& data);
      jerror_t GetCherenkovTruthHits(hddm_s::HDDM *record, vector<DMCTrackHit*>& data);
      jerror_t GetFCALTruthHits(hddm_s::HDDM *record, vector<DMCTrackHit*>& data);
      jerror_t GetECALTruthHits(hddm_s::HDDM *record, vector<DMCTrackHit*>& data);
      jerror_t GetCCALTruthHits(hddm_s::HDDM *record, vector<DMCTrackHit*>& data);
      jerror_t GetSCTruthHits(hddm_s::HDDM *record, vector<DMCTrackHit*>& data);

	   jerror_t Extract_DRFTime(hddm_s::HDDM *record, JFactoryT<DRFTime> *factory, const std::shared_ptr<const JEvent>& locEvent);
      jerror_t Extract_DBCALTruthShower(hddm_s::HDDM *record, JFactoryT<DBCALTruthShower> *factory, string tag);
      jerror_t Extract_DBCALTruthCell(hddm_s::HDDM *record, JFactoryT<DBCALTruthCell> *factory, string tag);
      jerror_t Extract_DBCALSiPMHit(hddm_s::HDDM *record, JFactoryT<DBCALSiPMHit> *factory, string tag);
      jerror_t Extract_DBCALSiPMSpectrum(hddm_s::HDDM *record, JFactoryT<DBCALSiPMSpectrum>* factory, string tag);
      jerror_t Extract_DBCALDigiHit(hddm_s::HDDM *record, JFactoryT<DBCALDigiHit> *factory, string tag);
      jerror_t Extract_DBCALIncidentParticle(hddm_s::HDDM *record, JFactoryT<DBCALIncidentParticle> *factory, string tag);
      jerror_t Extract_DBCALTDCDigiHit(hddm_s::HDDM *record, JFactoryT<DBCALTDCDigiHit> *factory, string tag);
      jerror_t Extract_DMCReaction(hddm_s::HDDM *record, JFactoryT<DMCReaction> *factory, string tag, const std::shared_ptr<const JEvent>& loop);
      jerror_t Extract_DMCThrown(hddm_s::HDDM *record, JFactoryT<DMCThrown> *factory, string tag);
      jerror_t Extract_DCDCHit(const std::shared_ptr<const JEvent>& locEvent, hddm_s::HDDM *record, JFactoryT<DCDCHit> *factory, string tag);
      jerror_t Extract_DFDCHit(hddm_s::HDDM *record, JFactoryT<DFDCHit> *factory, string tag);
      jerror_t Extract_DFCALTruthShower(hddm_s::HDDM *record, JFactoryT<DFCALTruthShower> *factory, string tag);
      jerror_t Extract_DECALHit(hddm_s::HDDM *record, JFactoryT<DECALHit> *factory, string tag, const std::shared_ptr<const JEvent>& event);
      jerror_t Extract_DECALTruthShower(hddm_s::HDDM *record, JFactoryT<DECALTruthShower> *factory, string tag);
      jerror_t Extract_DCCALHit(hddm_s::HDDM *record, JFactoryT<DCCALHit> *factory, string tag, const std::shared_ptr<const JEvent>& event);
      jerror_t Extract_DCCALTruthShower(hddm_s::HDDM *record, JFactoryT<DCCALTruthShower> *factory, string tag);
      jerror_t Extract_DFCALHit(hddm_s::HDDM *record, JFactoryT<DFCALHit> *factory, string tag, const std::shared_ptr<const JEvent>& event);
      jerror_t Extract_DMCTrajectoryPoint(hddm_s::HDDM *record, JFactoryT<DMCTrajectoryPoint> *factory, string tag);
      jerror_t Extract_DTOFTruth(hddm_s::HDDM *record,  JFactoryT<DTOFTruth> *factory, string tag);

      jerror_t Extract_DTOFHit( hddm_s::HDDM *record,  JFactoryT<DTOFHit>* factory, JFactoryT<DTOFHitMC>* factoryMC, string tag);
      jerror_t Extract_DTOFHitMC( hddm_s::HDDM *record,  JFactoryT<DTOFHitMC>* factoryMC, JFactoryT<DTOFHit>* factory, string tag);

      jerror_t Extract_DSCHit(hddm_s::HDDM *record,  JFactoryT<DSCHit> *factory, string tag);
      jerror_t Extract_DSCTruthHit(hddm_s::HDDM *record,  JFactoryT<DSCTruthHit> *factory, string tag);

      jerror_t Extract_DTrackTimeBased(hddm_s::HDDM *record,  JFactoryT<DTrackTimeBased> *factory, string tag, int32_t runnumber, const std::shared_ptr<const JEvent>& locEvent);
      string StringToTMatrixFSym(string &str_vals, TMatrixFSym* mat, int Nrows, int Ncols);
      
      jerror_t Extract_DTAGMHit( hddm_s::HDDM *record,  JFactoryT<DTAGMHit>* factory, string tag);
      jerror_t Extract_DTAGHHit( hddm_s::HDDM *record,  JFactoryT<DTAGHHit>* factory, string tag);

      jerror_t Extract_DPSHit(hddm_s::HDDM *record,JFactoryT<DPSHit>* factory, 
			      string tag);
      jerror_t Extract_DPSCHit(hddm_s::HDDM *record,JFactoryT<DPSCHit>* factory, 
			       string tag);
      jerror_t Extract_DPSTruthHit(hddm_s::HDDM *record,JFactoryT<DPSTruthHit>* factory, 
			      string tag);
      jerror_t Extract_DPSCTruthHit(hddm_s::HDDM *record,JFactoryT<DPSCTruthHit>* factory, 
			       string tag);
      jerror_t Extract_DFMWPCTruthHit(hddm_s::HDDM *record,  JFactoryT<DFMWPCTruthHit> *factory, string tag);
      jerror_t Extract_DFMWPCHit(hddm_s::HDDM *record,  JFactoryT<DFMWPCHit> *factory, string tag);
      jerror_t Extract_DFMWPCTruth(hddm_s::HDDM *record,  JFactoryT<DFMWPCTruth> *factory, string tag);
      jerror_t Extract_DCTOFHit(hddm_s::HDDM *record,  JFactoryT<DCTOFHit> *factory, string tag);
      jerror_t Extract_DCTOFTruth(hddm_s::HDDM *record,  JFactoryT<DCTOFTruth> *factory, string tag);

      jerror_t Extract_DTPOLHit(hddm_s::HDDM *record, JFactoryT<DTPOLHit>* factory, string tag);
      jerror_t Extract_DTPOLTruthHit(hddm_s::HDDM *record, JFactoryT<DTPOLTruthHit>* factory, string tag);
      Particle_t IDTrack(float locCharge, float locMass) const;

      // add RICH hit and Truth, yqiang Oct 3, 2012
      // modifed by yqiang, Oct 10 2012 now include both truth hits in DMCThrown
      // Oct 8, 2013, added dedicated object for RICH truth hit
      jerror_t Extract_DCereHit(hddm_s::HDDM *record, JFactoryT<DCereHit> *factory, string tag);
      jerror_t Extract_DDIRCTruthBarHit(hddm_s::HDDM *record, JFactoryT<DDIRCTruthBarHit> *factory, string tag);
      jerror_t Extract_DDIRCTruthPmtHit(hddm_s::HDDM *record, JFactoryT<DDIRCTruthPmtHit> *factory, string tag);
      jerror_t Extract_DDIRCPmtHit(hddm_s::HDDM *record, JFactoryT<DDIRCPmtHit> *factory, string tag, const std::shared_ptr<const JEvent>& event);

      std::ifstream *ifs;
      hddm_s::istream *fin;
      const DMagneticFieldMap *bfield;
      const DGeometry *geom;

   private:
      bool initialized;
      int dRunNumber;
      static thread_local shared_ptr<DResourcePool<TMatrixFSym>> dResourcePool_TMatrixFSym;

      map<unsigned int, double> dTargetCenterZMap; //unsigned int is run number
      map<unsigned int, double> dBeamBunchPeriodMap; //unsigned int is run number

      const DBCALGeometry *dBCALGeom;

      const DPSGeometry * psGeom;
      

      JCalibration *jcalib;
      float uscale[192],vscale[192];

	  double tagh_counter_quality[TAGH_MAX_COUNTER+1];
	  double tagm_fiber_quality[TAGM_MAX_ROW+1][TAGM_MAX_COLUMN+1];


      std::mutex read_mutex;
      // TODO: NWB: JANA2 doesn't have separate mutexes for
      //       reading vs in-progress. The read mutex is now
      //       the users' responsibility, although we should
      //       verify this isn't going to break anything.
};

#endif //_JEVENT_SOURCEHDDM_H_
