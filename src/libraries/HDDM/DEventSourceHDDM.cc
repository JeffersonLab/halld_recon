// $Id: DEventSourceHDDM.cc 19023 2015-07-14 20:23:27Z beattite $
//
// Author: David Lawrence  June 24, 2004
//
// changes: Wed Jun 20 17:08:13 EDT 2007 B. Zihlmann
//          modify TOF section to add several new variables incuding the 
//          GEANT particle type to the Truth hits and the hit and track-hit
//          list.
//
// Oct 3, 2012 Yi Qiang: add functions for Cherenkov RICH detector
// Oct 11, 2012 Yi Qiang: complete functions for Cherenkov detector
// Oct 8, 2013 Yi Qiang: added dedicated object for RICH Truth Hit
// July 5, 2014 R.T.Jones: changed over from c to c++ API for hddm
// June 22, 2015 J. Stevens: changed RICH -> DIRC and remove CERE
// May 7, 2017 R. Dzhygadlo: added DDIRCTruthPmtHit DDIRCTruthBarHit
// Oct 20, 2017 A. Somov: Added fields for the DPSHit/DPSCHit
//
// DEventSourceHDDM methods
//

#include <iostream>
#include <iomanip>
#include <cmath>
using namespace std;

#include <JANA/JFactory_base.h>
#include <JANA/JEventLoop.h>
#include <JANA/JEvent.h>
#include <DANA/DStatusBits.h>

#include <JANA/JGeometryXML.h>
#include "BCAL/DBCALGeometry.h"
#include "PAIR_SPECTROMETER/DPSGeometry.h"

#include <DVector2.h>
#include <DEventSourceHDDM.h>
#include <FDC/DFDCGeometry.h>
#include <FCAL/DFCALGeometry.h>
#include <FCAL/DFCALHit.h>
#include <CCAL/DCCALGeometry.h>
#include <CCAL/DCCALHit.h>


//------------------------------------------------------------------
// Binary predicate used to sort hits
//------------------------------------------------------------------
class MCTrackHitSort{
   public:
      bool operator()(DMCTrackHit* const &thit1, 
                      DMCTrackHit* const &thit2) const {
         return thit1->z < thit2->z;
      }
};

bool MCTrackHitSort_C(DMCTrackHit* const &thit1, 
                      DMCTrackHit* const &thit2) {
   return thit1->z < thit2->z;
}


//----------------
// Constructor
//----------------
DEventSourceHDDM::DEventSourceHDDM(const char* source_name)
: JEventSource(source_name)
{
   /// Constructor for DEventSourceHDDM object
   ifs = new ifstream(source_name);
   ifs->get();
   ifs->unget();
   if (ifs->rdbuf()->in_avail() > 30) {
      class nonstd_streambuf: public std::streambuf {
       public: char *pub_gptr() {return gptr();}
      };
      void *buf = (void*)ifs->rdbuf();
      std::string head(((nonstd_streambuf*)buf)->pub_gptr(), 30);
      std::string expected = " class=\"s\" ";
      std::string also_supported = " class=\"mc_s\" ";
      if (head.find(expected) == head.npos && 
          head.find(also_supported) == head.npos)
      {
         std::string msg("Unexpected header found in input HDDM stream: ");
         throw std::runtime_error(msg + head + source_name);
      }
   }

   fin = new hddm_s::istream(*ifs);
   initialized = false;
   dapp = NULL;
   bfield = NULL;
   geom = NULL;
   
   dRunNumber = -1;
	
   if( (!gPARMS->Exists("JANA_CALIB_CONTEXT")) && (getenv("JANA_CALIB_CONTEXT")==NULL) ){
   		cout << "============================================================" << endl;
			cout << " WARNING: JANA_CALIB_CONTEXT not set. " << endl;
			cout << "You are reading from an HDDM file which is most likely" << endl;
			cout << "MC data. In most cases, you will want to set this parameter" << endl;
			cout << "to get proper reconstruction." << endl;
			cout << "(usually something like \"variation=mc\")" << endl;
   		cout << "============================================================" << endl;
	}
}

//----------------
// Destructor
//----------------
DEventSourceHDDM::~DEventSourceHDDM()
{
   if (fin)
      delete fin;
   if (ifs)
      delete ifs;
}

//----------------
// GetEvent
//----------------
jerror_t DEventSourceHDDM::GetEvent(JEvent &event)
{
   /// Implementation of JEventSource virtual function

   if (!fin)
      return EVENT_SOURCE_NOT_OPEN;

   // Each open HDDM file takes up about 1M of memory so it's
   // worthwhile to close it as soon as we can.
   else if (!ifs->good()) {
      delete fin;
      fin = NULL;
      delete ifs;
      ifs = NULL;
      return NO_MORE_EVENTS_IN_SOURCE;
   }
   
   hddm_s::HDDM *record = new hddm_s::HDDM();
   while (record->getPhysicsEvents().size() == 0) {
      if (! (*fin >> *record)) {
         delete fin;
         fin = NULL;
         delete ifs;
         ifs = NULL;
         return NO_MORE_EVENTS_IN_SOURCE;
      }
   }

   ++Nevents_read;

   int event_number = -1;
   int run_number = -1;

   if(!record->getPhysicsEvents().empty()) {
       // Get event/run numbers from HDDM
       hddm_s::PhysicsEvent &pe = record->getPhysicsEvent(0);
       event_number = pe.getEventNo();
       run_number = pe.getRunNo();
   }

   // Copy the reference info into the JEvent object
   event.SetJEventSource(this);
   event.SetEventNumber(event_number);
   event.SetRunNumber(run_number);
   event.SetRef(record);
   event.SetStatusBit(kSTATUS_HDDM);
   event.SetStatusBit(kSTATUS_FROM_FILE);
   event.SetStatusBit(kSTATUS_PHYSICS_EVENT);
 
   return NOERROR;
}

//----------------
// FreeEvent
//----------------
void DEventSourceHDDM::FreeEvent(JEvent &event)
{
   hddm_s::HDDM *record = (hddm_s::HDDM*)event.GetRef();
   delete record;
}

//----------------
// GetObjects
//----------------
jerror_t DEventSourceHDDM::GetObjects(JEvent &event, JFactory_base *factory)
{
   /// This gets called through the virtual method of the
   /// JEventSource base class. It creates the objects of the type
   /// on which factory is based. It uses the hddm_s::HDDM* object
   /// kept in the ref field of the JEvent object passed.

   // We must have a factory to hold the data
   if (!factory)
      throw RESOURCE_UNAVAILABLE;
   
   // HDDM doesn't exactly support tagged factories, but the tag
   // can be used to direct filling of the correct factory.
   string tag = (factory->Tag()==NULL)? "" : factory->Tag();
   
   // The ref field of the JEvent is just the HDDM object pointer.
   hddm_s::HDDM *record = (hddm_s::HDDM*)event.GetRef();
   if (!record)
      throw RESOURCE_UNAVAILABLE;

   // Get pointer to the B-field object and Geometry object
   JEventLoop *loop = event.GetJEventLoop();
   if (initialized == false && loop) {
      initialized = true;
      dRunNumber = event.GetRunNumber();
      dapp = dynamic_cast<DApplication*>(loop->GetJApplication());
      if (dapp) {
         jcalib = dapp->GetJCalibration(event.GetRunNumber());
         // Make sure jcalib is set
         if (!jcalib) {
            _DBG_ << "ERROR - no jcalib set!" <<endl;
            return RESOURCE_UNAVAILABLE;
         }
         // Get constants and do basic check on number of elements
         vector< map<string, float> > tvals;
         if(jcalib->Get("FDC/strip_calib", tvals))
             throw JException("Could not load CCDB table: FDC/strip_calib");
 
         if (tvals.size() != 192) {
            _DBG_ << "ERROR - strip calibration vectors are not the right size!"
                  << endl;
            return VALUE_OUT_OF_RANGE;
         }
         map<string,float>::iterator iter;
         for (iter=tvals[0].begin(); iter!=tvals[0].end(); iter++) {
            // Copy values into tables. We preserve the order since
            // that is how it was originally done in hitFDC.c
            for (unsigned int i=0; i<tvals.size(); i++) {
               map<string, float> &row = tvals[i];
               uscale[i]=row["qru"];
               vscale[i]=row["qrv"];
            }
         }     
      }
      // load BCAL geometry
      vector<const DBCALGeometry *> BCALGeomVec;
      loop->Get(BCALGeomVec);
      if(BCALGeomVec.size() == 0)
	throw JException("Could not load DBCALGeometry object!");
      dBCALGeom = BCALGeomVec[0];
      
      // load PS geometry
      vector<const DPSGeometry*> psGeomVect;
      loop->Get(psGeomVect);
      if (psGeomVect.size() < 1)
	return OBJECT_NOT_AVAILABLE;
      psGeom = psGeomVect[0];
      

   }

   // Warning: This class is not completely thread-safe and can fail if running
   // running in multithreaded mode over files with events from multiple runs
   // It is expected that simulated data will rarely contain events from multiple
   // runs, as this is an intermediate format in the simulation chain, so for 
   // now we just insert a sanity check, and push the problem to the future
   if(dRunNumber != event.GetRunNumber()) {
       jerr << endl
            << "WARNING:  DEventSourceHDDM cannot currently handle HDDM files containing" << endl
            << "events with multiple runs!  If you encounter this error message," << endl
            << "please contact the GlueX Offline Software Group: halld-offline@jlab.org" << endl 
            << endl;
       exit(-1);
   }

   //Get target center
   //multiple reader threads can access this object: need lock
   bool locNewRunNumber = false;
   unsigned int locRunNumber = event.GetRunNumber();
   LockRead();
   {
      locNewRunNumber = (dTargetCenterZMap.find(locRunNumber) == dTargetCenterZMap.end());
   }
   UnlockRead();
   if(locNewRunNumber)
   {
      DApplication* dapp = dynamic_cast<DApplication*>(loop->GetJApplication());
      DGeometry* locGeometry = dapp->GetDGeometry(loop->GetJEvent().GetRunNumber());
      double locTargetCenterZ = 0.0;
      locGeometry->GetTargetZ(locTargetCenterZ);

      JGeometryXML *jgeom = dynamic_cast<JGeometryXML *>(locGeometry);
      hddm_s::GeometryList geolist = record->getGeometrys();
      if (jgeom != 0 && geolist.size() > 0) {
         std::string md5sim = geolist(0).getMd5simulation();
         std::string md5smear = geolist(0).getMd5smear();
         std::string md5recon = jgeom->GetChecksum();
         geolist(0).setMd5reconstruction(md5recon);
         if (md5sim != md5smear) {
            jerr << std::endl
                 << "WARNING: simulation geometry checksum does not match"
                 << " that shown for the mcsmear step."
                 << std::endl;
         }
         else if (md5sim != md5recon) {
            jerr << endl
                 << "WARNING: simulation geometry checksum does not match"
                 << " the geometry being used for reconstruction."
                 << std::endl;
         }
      }

      vector<double> locBeamPeriodVector;
      if(loop->GetCalib("PHOTON_BEAM/RF/beam_period", locBeamPeriodVector))
          throw runtime_error("Could not load CCDB table: PHOTON_BEAM/RF/beam_period");
      double locBeamBunchPeriod = locBeamPeriodVector[0];

      LockRead();
      {
         dTargetCenterZMap[locRunNumber] = locTargetCenterZ;
         dBeamBunchPeriodMap[locRunNumber] = locBeamBunchPeriod;
      }
      UnlockRead();
   }

   // Get name of data class we're trying to extract
   string dataClassName = factory->GetDataClassName();

   if (dataClassName == "DPSHit")
      return Extract_DPSHit(record, 
                     dynamic_cast<JFactory<DPSHit>*>(factory), tag);

   if (dataClassName == "DPSTruthHit")
      return Extract_DPSTruthHit(record, 
                     dynamic_cast<JFactory<DPSTruthHit>*>(factory), tag);

   if (dataClassName == "DPSCHit")
      return Extract_DPSCHit(record, 
                     dynamic_cast<JFactory<DPSCHit>*>(factory), tag);

   if (dataClassName == "DPSCTruthHit")
      return Extract_DPSCTruthHit(record, 
                     dynamic_cast<JFactory<DPSCTruthHit>*>(factory), tag);

   if (dataClassName == "DRFTime")
      return Extract_DRFTime(record, 
                     dynamic_cast<JFactory<DRFTime>*>(factory), loop);

   if (dataClassName == "DTAGMHit")
      return Extract_DTAGMHit(record, 
                     dynamic_cast<JFactory<DTAGMHit>*>(factory), tag);
 
   if (dataClassName == "DTAGHHit")
      return Extract_DTAGHHit(record, 
                     dynamic_cast<JFactory<DTAGHHit>*>(factory), tag);

   if (dataClassName == "DMCTrackHit")
      return Extract_DMCTrackHit(record,
                     dynamic_cast<JFactory<DMCTrackHit>*>(factory), tag);
 
   if (dataClassName == "DMCReaction")
      return Extract_DMCReaction(record,
                     dynamic_cast<JFactory<DMCReaction>*>(factory), tag, loop);
 
   if (dataClassName == "DMCThrown")
      return Extract_DMCThrown(record,
                     dynamic_cast<JFactory<DMCThrown>*>(factory), tag);
 
   if (dataClassName == "DBCALTruthShower")
      return Extract_DBCALTruthShower(record, 
                     dynamic_cast<JFactory<DBCALTruthShower>*>(factory), tag);
 
   if (dataClassName == "DBCALSiPMSpectrum")
      return Extract_DBCALSiPMSpectrum(record,
                     dynamic_cast<JFactory<DBCALSiPMSpectrum>*>(factory), tag);
 
   if (dataClassName == "DBCALTruthCell")
      return Extract_DBCALTruthCell(record,
                     dynamic_cast<JFactory<DBCALTruthCell>*>(factory), tag);
 
   if (dataClassName == "DBCALSiPMHit")
      return Extract_DBCALSiPMHit(record,
                     dynamic_cast<JFactory<DBCALSiPMHit>*>(factory), tag);
 
   if (dataClassName == "DBCALDigiHit")
      return Extract_DBCALDigiHit(record,
                     dynamic_cast<JFactory<DBCALDigiHit>*>(factory), tag);

   if (dataClassName == "DBCALIncidentParticle")
      return Extract_DBCALIncidentParticle(record,
                     dynamic_cast<JFactory<DBCALIncidentParticle>*>(factory), tag);
 
   if (dataClassName == "DBCALTDCDigiHit")
      return Extract_DBCALTDCDigiHit(record,
                     dynamic_cast<JFactory<DBCALTDCDigiHit>*>(factory), tag);
 
   if (dataClassName == "DCDCHit")
      return Extract_DCDCHit(loop, record,
                     dynamic_cast<JFactory<DCDCHit>*>(factory) , tag);
 
   if (dataClassName == "DFDCHit")
      return Extract_DFDCHit(record, 
                     dynamic_cast<JFactory<DFDCHit>*>(factory), tag);
 
   if (dataClassName == "DFCALTruthShower")
      return Extract_DFCALTruthShower(record, 
                     dynamic_cast<JFactory<DFCALTruthShower>*>(factory), tag);
 
   if (dataClassName == "DFCALHit")
      return Extract_DFCALHit(record,
                     dynamic_cast<JFactory<DFCALHit>*>(factory), tag,
                     event.GetJEventLoop());
 
   if (dataClassName == "DCCALTruthShower")
      return Extract_DCCALTruthShower(record,
                     dynamic_cast<JFactory<DCCALTruthShower>*>(factory), tag);
 
   if (dataClassName == "DCCALHit")
      return Extract_DCCALHit(record,
                     dynamic_cast<JFactory<DCCALHit>*>(factory), tag,
                     event.GetJEventLoop());
 
   if (dataClassName == "DMCTrajectoryPoint" && tag == "")
      return Extract_DMCTrajectoryPoint(record,
                     dynamic_cast<JFactory<DMCTrajectoryPoint>*>(factory), tag);
 
   if (dataClassName == "DTOFTruth")
      return Extract_DTOFTruth(record, 
                     dynamic_cast<JFactory<DTOFTruth>*>(factory), tag);
 
   // TOF is a special case: TWO factories are needed at the same time
   // DTOFHit and DTOFHitMC
   if (dataClassName == "DTOFHit") {
      JFactory_base* factory2 = loop->GetFactory("DTOFHitMC", tag.c_str()); 
      return Extract_DTOFHit(record, 
                     dynamic_cast<JFactory<DTOFHit>*>(factory),
                     dynamic_cast<JFactory<DTOFHitMC>*>(factory2), tag);
   }
   if (dataClassName == "DTOFHitMC") {
      JFactory_base* factory2 = loop->GetFactory("DTOFHit", tag.c_str()); 
      return Extract_DTOFHit(record, 
                     dynamic_cast<JFactory<DTOFHit>*>(factory2),
                     dynamic_cast<JFactory<DTOFHitMC>*>(factory), tag);
   }

   if (dataClassName == "DSCHit")
      return Extract_DSCHit(record, 
                     dynamic_cast<JFactory<DSCHit>*>(factory), tag);

   if (dataClassName == "DSCTruthHit")
      return Extract_DSCTruthHit(record, 
                     dynamic_cast<JFactory<DSCTruthHit>*>(factory), tag);

   if (dataClassName == "DFMWPCTruthHit")
      return Extract_DFMWPCTruthHit(record, 
                     dynamic_cast<JFactory<DFMWPCTruthHit>*>(factory), tag);
   
   if (dataClassName == "DFMWPCTruth")
      return Extract_DFMWPCTruth(record, 
                     dynamic_cast<JFactory<DFMWPCTruth>*>(factory), tag);

   if (dataClassName == "DFMWPCHit")
      return Extract_DFMWPCHit(record, 
                     dynamic_cast<JFactory<DFMWPCHit>*>(factory), tag);

   if (dataClassName == "DCTOFTruth")
      return Extract_DCTOFTruth(record, 
                     dynamic_cast<JFactory<DCTOFTruth>*>(factory), tag);
   
   if (dataClassName == "DCTOFHit")
      return Extract_DCTOFHit(record, 
                     dynamic_cast<JFactory<DCTOFHit>*>(factory), tag);

   if (dataClassName == "DDIRCTruthBarHit")
     return Extract_DDIRCTruthBarHit(record,
		     dynamic_cast<JFactory<DDIRCTruthBarHit>*>(factory), tag);

   if (dataClassName == "DDIRCTruthPmtHit")
     return Extract_DDIRCTruthPmtHit(record,
		     dynamic_cast<JFactory<DDIRCTruthPmtHit>*>(factory), tag);
   
   if (dataClassName == "DDIRCPmtHit")
     return Extract_DDIRCPmtHit(record,
		     dynamic_cast<JFactory<DDIRCPmtHit>*>(factory), tag, event.GetJEventLoop());

   // extract CereTruth and CereRichHit hits, yqiang Oct 3, 2012
   // removed CereTruth (merged into MCThrown), added CereHit, yqiang Oct 10 2012
   if (dataClassName == "DCereHit")
      return Extract_DCereHit(record, 
                     dynamic_cast<JFactory<DCereHit>*>(factory), tag);

   if (dataClassName == "DTPOLHit")
      return Extract_DTPOLHit(record,
                     dynamic_cast<JFactory<DTPOLHit>*>(factory), tag);

   if (dataClassName == "DTPOLTruthHit")
      return Extract_DTPOLTruthHit(record,
                     dynamic_cast<JFactory<DTPOLTruthHit>*>(factory), tag);
   
   if (dataClassName == "DGEMTRDTruthHit")
      return Extract_DGEMTRDTruthHit(record, 
                     dynamic_cast<JFactory<DGEMTRDTruthHit>*>(factory), tag);
   
   if (dataClassName == "DGEMTRDTruth")
      return Extract_DGEMTRDTruth(record, 
                     dynamic_cast<JFactory<DGEMTRDTruth>*>(factory), tag);

   if (dataClassName == "DGEMTRDHit")
      return Extract_DGEMTRDHit(record, 
                     dynamic_cast<JFactory<DGEMTRDHit>*>(factory), tag);

   return OBJECT_NOT_AVAILABLE;
}

//------------------
// Extract_DRFTime
//------------------
jerror_t DEventSourceHDDM::Extract_DRFTime(hddm_s::HDDM *record,
                                   JFactory<DRFTime> *factory, JEventLoop* locEventLoop)
{
   if (factory==NULL)
      return OBJECT_NOT_AVAILABLE;
   string tag = (factory->Tag())? factory->Tag() : "";

   vector<DRFTime*> locRFTimes;

   // loop over RF-time records
   const hddm_s::RFtimeList &rftimes = record->getRFtimes();
   hddm_s::RFtimeList::iterator iter;
   for (iter = rftimes.begin(); iter != rftimes.end(); ++iter)
   {
      if (iter->getJtag() != tag)
         continue;
      DRFTime *locRFTime = new DRFTime;
      locRFTime->dTime = iter->getTsync();
      locRFTime->dTimeVariance = 0.0; //SET ME!!
      locRFTimes.push_back(locRFTime);
   }

	if(!locRFTimes.empty())
	{
		//found in the file, copy into factory and return
		factory->CopyTo(locRFTimes);
		return NOERROR;
	}

	//Not found in the file, so either:
		//Experimental data & it's missing: bail
		//MC data: generate it

	vector<const DBeamPhoton*> locMCGENPhotons;
	locEventLoop->Get(locMCGENPhotons, "MCGEN");
	if(locMCGENPhotons.empty())
		return OBJECT_NOT_AVAILABLE; //Experimental data & it's missing: bail

	//Is MC data. Either:
		//No tag: return t = 0.0, but true t is 0.0 +/- n*locBeamBunchPeriod: must select the correct beam bunch
		//TRUTH tag: get exact t from DBeamPhoton tag MCGEN

	if(tag == "TRUTH")
	{
		DRFTime *locRFTime = new DRFTime;
		locRFTime->dTime = locMCGENPhotons[0]->time();
		locRFTime->dTimeVariance = 0.0;
		locRFTimes.push_back(locRFTime);
	}
	else
	{
		double locBeamBunchPeriod = 0.0;
		int locRunNumber = locEventLoop->GetJEvent().GetRunNumber();
		LockRead();
		{
			locBeamBunchPeriod = dBeamBunchPeriodMap[locRunNumber];
		}
		UnlockRead();

		//start with true RF time, increment/decrement by multiples of locBeamBunchPeriod ns until closest to 0
		double locTime = locMCGENPhotons[0]->time();
		int locNumRFBuckets = int(locTime/locBeamBunchPeriod);
		locTime -= double(locNumRFBuckets)*locBeamBunchPeriod;
		while(locTime > 0.5*locBeamBunchPeriod)
			locTime -= locBeamBunchPeriod;
		while(locTime < -0.5*locBeamBunchPeriod)
			locTime += locBeamBunchPeriod;

		DRFTime *locRFTime = new DRFTime;
		locRFTime->dTime = locTime;
		locRFTime->dTimeVariance = 0.0;
		locRFTimes.push_back(locRFTime);
	}

	// Copy into factories
	factory->CopyTo(locRFTimes);

   return NOERROR;
}

//------------------
// Extract_DMCTrackHit
//------------------
jerror_t DEventSourceHDDM::Extract_DMCTrackHit(hddm_s::HDDM *record,
                                   JFactory<DMCTrackHit> *factory, string tag)
{
   /// Copies the data from the given hddm_s structure. This is called
   /// from JEventSourceHDDM::GetObjects. If factory is NULL, this
   /// returns OBJECT_NOT_AVAILABLE immediately.
   
   if (factory == NULL)
      return OBJECT_NOT_AVAILABLE;
   if (tag != "")
      return OBJECT_NOT_AVAILABLE;
   
   // The following routines will create DMCTrackHit objects and add them
   // to data.
   vector<DMCTrackHit*> data;
   GetCDCTruthHits(record, data);
   GetFDCTruthHits(record, data);
   GetBCALTruthHits(record, data);
   GetTOFTruthHits(record, data);
   GetCherenkovTruthHits(record, data);
   GetFCALTruthHits(record, data);
   GetSCTruthHits(record, data);

   // It has happened that some CDC hits have "nan" for the drift time
   // in a peculiar event Alex Somov came across. This ultimately caused
   // a seg. fault in MCTrackHitSort_C. I hate doing this since it
   // is treating the symptom rather than the cause, but nonetheless,
   // it patches up the problem for now until there is time to revisit
   // it later.
   for (unsigned int i=0; i < data.size(); i++)
      if (!isfinite(data[i]->z))
         data[i]->z = -1000.0;
   
   // sort hits by z
   sort(data.begin(), data.end(), MCTrackHitSort_C);
   
   // Some systems will use negative phis. Force them all to
   // be in the 0 to 2pi range
   for (unsigned int i=0; i < data.size(); i++) {
      DMCTrackHit *mctrackhit = data[i];
      if (mctrackhit->phi < 0.0)
         mctrackhit->phi += 2.0*M_PI;
   }
   
   // Copy into factory
   factory->CopyTo(data);

   return NOERROR;
}

//-------------------
// GetCDCTruthHits
//-------------------
jerror_t DEventSourceHDDM::GetCDCTruthHits(hddm_s::HDDM *record, 
                                           vector<DMCTrackHit*>& data)
{
   const hddm_s::CdcTruthPointList &points = record->getCdcTruthPoints();
   hddm_s::CdcTruthPointList::iterator iter;
   for (iter = points.begin(); iter != points.end(); ++iter) {
      DMCTrackHit *mctrackhit = new DMCTrackHit;
      mctrackhit->r       = iter->getR();
      mctrackhit->phi     = iter->getPhi();
      mctrackhit->z       = iter->getZ();
      mctrackhit->track   = iter->getTrack();
      mctrackhit->primary = iter->getPrimary();
      mctrackhit->ptype   = iter->getPtype();
      mctrackhit->system  = SYS_CDC;
      const hddm_s::TrackIDList &ids = iter->getTrackIDs();
      mctrackhit->itrack = (ids.size())? ids.begin()->getItrack() : 0;
      data.push_back(mctrackhit);
   }
      
   return NOERROR;
}

//-------------------
// GetFDCTruthHits
//-------------------
jerror_t DEventSourceHDDM::GetFDCTruthHits(hddm_s::HDDM *record,
                                           vector<DMCTrackHit*>& data)
{
   const hddm_s::FdcTruthPointList &points = record->getFdcTruthPoints();
   hddm_s::FdcTruthPointList::iterator iter;
   for (iter = points.begin(); iter != points.end(); ++iter) {
      float x = iter->getX();
      float y = iter->getY();
      DMCTrackHit *mctrackhit = new DMCTrackHit;
      mctrackhit->r       = sqrt(x*x + y*y);
      mctrackhit->phi     = atan2(y,x);
      mctrackhit->z       = iter->getZ();
      mctrackhit->track   = iter->getTrack();
      mctrackhit->primary = iter->getPrimary();
      mctrackhit->ptype   = iter->getPtype();
      mctrackhit->system  = SYS_FDC;
      const hddm_s::TrackIDList &ids = iter->getTrackIDs();
      mctrackhit->itrack = (ids.size())? ids.begin()->getItrack() : 0;
      data.push_back(mctrackhit);
   }

   return NOERROR;
}

//-------------------
// GetBCALTruthHits
//-------------------
jerror_t DEventSourceHDDM::GetBCALTruthHits(hddm_s::HDDM *record,
                                            vector<DMCTrackHit*>& data)
{
   const hddm_s::BcalTruthShowerList &showers = record->getBcalTruthShowers();
   hddm_s::BcalTruthShowerList::iterator iter;
   for (iter = showers.begin(); iter != showers.end(); ++iter) {
      DMCTrackHit *mctrackhit = new DMCTrackHit;
      mctrackhit->r       = iter->getR();
      mctrackhit->phi     = iter->getPhi();
      mctrackhit->z       = iter->getZ();
      mctrackhit->track   = iter->getTrack();
      mctrackhit->primary = iter->getPrimary();
      mctrackhit->ptype   = iter->getPtype();
      mctrackhit->system  = SYS_BCAL;
      const hddm_s::TrackIDList &ids = iter->getTrackIDs();
      mctrackhit->itrack = (ids.size())? ids.begin()->getItrack() : 0;
      data.push_back(mctrackhit);
   }

   return NOERROR;
}

//-------------------
// GetTOFTruthHits
//-------------------
jerror_t DEventSourceHDDM::GetTOFTruthHits(hddm_s::HDDM *record,
                                           vector<DMCTrackHit*>& data)
{
   const hddm_s::FtofTruthPointList &points = record->getFtofTruthPoints();
   hddm_s::FtofTruthPointList::iterator iter;
   for (iter = points.begin(); iter != points.end(); ++iter) {
      float x = iter->getX();
      float y = iter->getY();
      DMCTrackHit *mctrackhit = new DMCTrackHit;
      mctrackhit->r       = sqrt(x*x + y*y);
      mctrackhit->phi     = atan2(y,x);
      mctrackhit->z       = iter->getZ();
      mctrackhit->track   = iter->getTrack();
      mctrackhit->primary = iter->getPrimary();
      mctrackhit->ptype   = iter->getPtype(); // save GEANT particle type 
      mctrackhit->system  = SYS_TOF;
      const hddm_s::TrackIDList &ids = iter->getTrackIDs();
      mctrackhit->itrack = (ids.size())? ids.begin()->getItrack() : 0;
      data.push_back(mctrackhit);
   }
      
   return NOERROR;
}

//-------------------
// GetCherenkovTruthHits
// modified by yqiang, Oct 10 2012
//-------------------
jerror_t DEventSourceHDDM::GetCherenkovTruthHits(hddm_s::HDDM *record,
                                                 vector<DMCTrackHit*>& data)
{
   const hddm_s::CereTruthPointList &points = record->getCereTruthPoints();
   hddm_s::CereTruthPointList::iterator iter;
   for (iter = points.begin(); iter != points.end(); ++iter) {
      float x = iter->getX();
      float y = iter->getY();
      DMCTrackHit *mctrackhit = new DMCTrackHit;
      mctrackhit->r       = sqrt(x*x + y*y);
      mctrackhit->phi     = atan2(y,x);
      mctrackhit->z       = iter->getZ();
      mctrackhit->track   = iter->getTrack();
      mctrackhit->primary = iter->getPrimary();
      mctrackhit->ptype   = iter->getPtype();    // save GEANT particle typ()e
      mctrackhit->system  = SYS_CHERENKOV;
      const hddm_s::TrackIDList &ids = iter->getTrackIDs();
      mctrackhit->itrack = (ids.size())? ids.begin()->getItrack() : 0;
      data.push_back(mctrackhit);
   }

   return NOERROR;
}

//-------------------
// GetFCALTruthHits
//-------------------
jerror_t DEventSourceHDDM::GetFCALTruthHits(hddm_s::HDDM *record,
                                            vector<DMCTrackHit*>& data)
{
   const hddm_s::FcalTruthShowerList &points = record->getFcalTruthShowers();
   hddm_s::FcalTruthShowerList::iterator iter;
   for (iter = points.begin(); iter != points.end(); ++iter) {
      float x = iter->getX();
      float y = iter->getY();
      DMCTrackHit *mctrackhit = new DMCTrackHit;
      mctrackhit->r       = sqrt(x*x + y*y);
      mctrackhit->phi     = atan2(y,x);
      mctrackhit->z       = iter->getZ();
      mctrackhit->track   = iter->getTrack();
      mctrackhit->primary = iter->getPrimary();
      mctrackhit->ptype   = iter->getPtype();
      mctrackhit->system  = SYS_FCAL;
      const hddm_s::TrackIDList &ids = iter->getTrackIDs();
      mctrackhit->itrack = (ids.size())? ids.begin()->getItrack() : 0;
      data.push_back(mctrackhit);
   }

   return NOERROR;
}

//-------------------
// GetCCALTruthHits
//-------------------
jerror_t DEventSourceHDDM::GetCCALTruthHits(hddm_s::HDDM *record,
                                            vector<DMCTrackHit*>& data)
{
   const hddm_s::CcalTruthShowerList &points = record->getCcalTruthShowers();
   hddm_s::CcalTruthShowerList::iterator iter;
   for (iter = points.begin(); iter != points.end(); ++iter) {
      float x = iter->getX();
      float y = iter->getY();
      DMCTrackHit *mctrackhit = new DMCTrackHit;
      mctrackhit->r       = sqrt(x*x + y*y);
      mctrackhit->phi     = atan2(y,x);
      mctrackhit->z       = iter->getZ();
      mctrackhit->track   = iter->getTrack();
      mctrackhit->primary = iter->getPrimary();
      mctrackhit->ptype   = iter->getPtype();
      mctrackhit->system  = SYS_CCAL;
      const hddm_s::TrackIDList &ids = iter->getTrackIDs();
      mctrackhit->itrack = (ids.size())? ids.begin()->getItrack() : 0;
      data.push_back(mctrackhit);
   }

   return NOERROR;
}


//-------------------
// GetSCTruthHits
//-------------------
jerror_t DEventSourceHDDM::GetSCTruthHits(hddm_s::HDDM *record,
                                          vector<DMCTrackHit*>& data)
{
   const hddm_s::StcTruthPointList &points = record->getStcTruthPoints();
   hddm_s::StcTruthPointList::iterator iter;
   for (iter = points.begin(); iter != points.end(); ++iter) {
      DMCTrackHit *mctrackhit = new DMCTrackHit;
      mctrackhit->r       = iter->getR();
      mctrackhit->phi     = iter->getPhi();
      mctrackhit->z       = iter->getZ();
      mctrackhit->track   = iter->getTrack();
      mctrackhit->primary = iter->getPrimary();
      mctrackhit->ptype   = iter->getPtype();    // save GEANT particle type
      mctrackhit->system  = SYS_START;
      const hddm_s::TrackIDList &ids = iter->getTrackIDs();
      mctrackhit->itrack = (ids.size())? ids.begin()->getItrack() : 0;
      data.push_back(mctrackhit);
   }
      
   return NOERROR;
}

//------------------
// Extract_DBCALSiPMHit
//------------------
jerror_t DEventSourceHDDM::Extract_DBCALSiPMHit(hddm_s::HDDM *record,
                                   JFactory<DBCALSiPMHit> *factory, string tag)
{
   /// Copies the data from the given hddm_s structure. This is called
   /// from JEventSourceHDDM::GetObjects. If factory is NULL, this
   /// returns OBJECT_NOT_AVAILABLE immediately.
   
   if (factory == NULL)
      return OBJECT_NOT_AVAILABLE;
   if (tag != "")
      return OBJECT_NOT_AVAILABLE;
   
   vector<DBCALSiPMHit*> data;

   const hddm_s::BcalSiPMUpHitList &uphits = record->getBcalSiPMUpHits();
   hddm_s::BcalSiPMUpHitList::iterator uiter;
   for (uiter = uphits.begin(); uiter != uphits.end(); ++uiter) {
      DBCALSiPMHit *response = new DBCALSiPMHit;
      response->module = uiter->getModule();
      response->layer  = uiter->getLayer();
      response->sector = uiter->getSector();
      response->E      = uiter->getE();
      response->t      = uiter->getT();
      response->end    = DBCALGeometry::kUpstream;
      response->cellId = dBCALGeom->cellId(uiter->getModule(),
                                           uiter->getLayer(), 
                                           uiter->getSector());
      data.push_back(response);
   }
         
   const hddm_s::BcalSiPMDownHitList &downhits = record->getBcalSiPMDownHits();
   hddm_s::BcalSiPMDownHitList::iterator diter;
   for (diter = downhits.begin(); diter != downhits.end(); ++diter) {
      DBCALSiPMHit *response = new DBCALSiPMHit;
      response->module = diter->getModule();
      response->layer  = diter->getLayer();
      response->sector = diter->getSector();
      response->E      = diter->getE();
      response->t      = diter->getT();
      response->end    = DBCALGeometry::kDownstream;
      response->cellId = dBCALGeom->cellId(diter->getModule(),
                                           diter->getLayer(), 
                                           diter->getSector());
      data.push_back(response);
   }
   
   // Copy into factory
   factory->CopyTo(data);

   return NOERROR;
}

//------------------
// Extract_DBCALDigiHit
//------------------
jerror_t DEventSourceHDDM::Extract_DBCALDigiHit(hddm_s::HDDM *record,
                                   JFactory<DBCALDigiHit> *factory, string tag)
{
   /// Copies the data from the given hddm_s structure. This is called
   /// from JEventSourceHDDM::GetObjects. If factory is NULL, this
   /// returns OBJECT_NOT_AVAILABLE immediately.
   
   if (factory == NULL)
      return OBJECT_NOT_AVAILABLE;
   if (tag != "")
      return OBJECT_NOT_AVAILABLE;
   
   vector<DBCALDigiHit*> data;

   const hddm_s::BcalfADCDigiHitList &digihits = record->getBcalfADCDigiHits();
   hddm_s::BcalfADCDigiHitList::iterator iter;
   for (iter = digihits.begin(); iter != digihits.end(); ++iter) {
      DBCALDigiHit *response = new DBCALDigiHit;
      response->module            = iter->getModule();
      response->layer             = iter->getLayer();
      response->sector            = iter->getSector();
      response->pulse_integral    = (uint32_t)iter->getPulse_integral();
      response->pulse_peak        = 0;
      if(iter->getBcalfADCPeaks().size() > 0) {
          response->pulse_peak    = iter->getBcalfADCPeak().getPeakAmp();
      }
      response->pulse_time        = (uint32_t)iter->getPulse_time();
      response->pedestal          = 1;
      response->QF                = 1;
      response->nsamples_integral = 1;
      response->nsamples_pedestal = 1;
      response->datasource        = 3;
      response->end               = (iter->getEnd() == 0)? DBCALGeometry::kUpstream :
                                                DBCALGeometry::kDownstream;
      data.push_back(response);
   }

   // Copy into factory
   factory->CopyTo(data);

   return NOERROR;
}

//------------------
// Extract_DBCALIncidentParticle
//------------------
jerror_t DEventSourceHDDM::Extract_DBCALIncidentParticle(hddm_s::HDDM *record,
                                   JFactory<DBCALIncidentParticle> *factory,
                                   string tag)
{
   /// Copies the data from the given hddm_s structure. This is called
   /// from JEventSourceHDDM::GetObjects. If factory is NULL, this
   /// returns OBJECT_NOT_AVAILABLE immediately.
  
   if (factory == NULL)
      return OBJECT_NOT_AVAILABLE;
   if (tag != "")
      return OBJECT_NOT_AVAILABLE;
  
   vector<DBCALIncidentParticle*> data;

   const hddm_s::BcalTruthIncidentParticleList &plist =
                          record->getBcalTruthIncidentParticles();
   hddm_s::BcalTruthIncidentParticleList::iterator iter;
   for (iter = plist.begin(); iter != plist.end(); ++iter) {
      DBCALIncidentParticle *part = new DBCALIncidentParticle;
      part->ptype = iter->getPtype();
      part->px    = iter->getPx();
      part->py    = iter->getPy();
      part->pz    = iter->getPz();
      part->x     = iter->getX();
      part->y     = iter->getY();
      part->z     = iter->getZ();
      data.push_back(part);
   }
  
   factory->CopyTo(data);
  
   return NOERROR;
}

//------------------
// Extract_DBCALSiPMSpectrum
//------------------
jerror_t DEventSourceHDDM::Extract_DBCALSiPMSpectrum(hddm_s::HDDM *record,
                                   JFactory<DBCALSiPMSpectrum> *factory,
                                   string tag)
{
   /// Copies the data from the given hddm_s structure. This is called
   /// from JEventSourceHDDM::GetObjects. If factory is NULL, this
   /// returns OBJECT_NOT_AVAILABLE immediately.
   
   if (factory == NULL)
      return OBJECT_NOT_AVAILABLE;
   if (tag != "" && tag != "TRUTH")
      return OBJECT_NOT_AVAILABLE;
   
   vector<DBCALSiPMSpectrum*> data;

   const hddm_s::BcalSiPMSpectrumList &specs = record->getBcalSiPMSpectrums();
   hddm_s::BcalSiPMSpectrumList::iterator iter;
   for (iter = specs.begin(); iter != specs.end(); ++iter) {
      DBCALSiPMSpectrum *dana_spectrum = new DBCALSiPMSpectrum;
      dana_spectrum->module      = iter->getModule();
      dana_spectrum->layer       = iter->getLayer();
      dana_spectrum->sector      = iter->getSector();
      dana_spectrum->end         = (iter->getEnd()==0)?
                                   DBCALGeometry::kUpstream :
                                   DBCALGeometry::kDownstream;
      if (tag == "") 
         dana_spectrum->incident_id = 0;
      else if (tag == "TRUTH") {
         const hddm_s::BcalSiPMTruthList &truths = iter->getBcalSiPMTruths();
         if (truths.size() > 0)
            dana_spectrum->incident_id = truths.begin()->getIncident_id();
         else
            dana_spectrum->incident_id = 0;
      }

      double t = iter->getTstart();
      double bin_width = iter->getBin_width();
      stringstream ss(iter->getVals());

      // Extract values and use them to fill histo
      string entry;
      while (ss >> entry) {
              if (entry[0] == 'X') {
                      // get rid of the X, the rest of the entry is the number of zeroes to add
                      stringstream sss(entry.substr(1)); 
                      int num_zeros;
                      sss >> num_zeros;
                      
                      for(int i=0; i<num_zeros; i++) {
                              dana_spectrum->spectrum.Fill(t, 0.0);
                              t += bin_width;                         
                      }
              } else {
                      stringstream sss(entry);
                      double dE;
                      sss >> dE;
                      dana_spectrum->spectrum.Fill(t, dE);
                      t += bin_width;
              }
      }

      data.push_back(dana_spectrum);
   }
         
   // Copy into factory
   factory->CopyTo(data);

   return NOERROR;
}

//------------------
// Extract_DBCALTDCDigiHit
//------------------
jerror_t DEventSourceHDDM::Extract_DBCALTDCDigiHit(hddm_s::HDDM *record,
                                   JFactory<DBCALTDCDigiHit> *factory, string tag)
{
   /// Copies the data from the given hddm_s structure. This is called
   /// from JEventSourceHDDM::GetObjects. If factory is NULL, this
   /// returns OBJECT_NOT_AVAILABLE immediately.
   
   if (factory == NULL)
      return OBJECT_NOT_AVAILABLE;
   if (tag != "")
      return OBJECT_NOT_AVAILABLE;
   
   vector<DBCALTDCDigiHit*> data;

   const hddm_s::BcalTDCDigiHitList &hits = record->getBcalTDCDigiHits();
   hddm_s::BcalTDCDigiHitList::iterator iter;
   for (iter = hits.begin(); iter != hits.end(); ++iter) {
      DBCALTDCDigiHit *bcaltdchit = new DBCALTDCDigiHit;
      bcaltdchit->module = iter->getModule();
      bcaltdchit->layer  = iter->getLayer();
      bcaltdchit->sector = iter->getSector();
      bcaltdchit->end    = (iter->getEnd() == 0)? DBCALGeometry::kUpstream :
                                                DBCALGeometry::kDownstream;
      bcaltdchit->time   = (uint32_t)iter->getTime();
      data.push_back(bcaltdchit);
   }
         
   // Copy into factory
   factory->CopyTo(data);

   return NOERROR;
}

//------------------
// Extract_DMCReaction
//------------------
jerror_t DEventSourceHDDM::Extract_DMCReaction(hddm_s::HDDM *record,
                                   JFactory<DMCReaction> *factory, string tag,
                                   JEventLoop *loop)
{
   /// Copies the data from the given hddm_s structure. This is called
   /// from JEventSourceHDDM::GetObjects. If factory is NULL, this
   /// returns OBJECT_NOT_AVAILABLE immediately.
   
   if (tag != "")
      return OBJECT_NOT_AVAILABLE;

   if (factory == NULL)
      return OBJECT_NOT_AVAILABLE;
   
   double locTargetCenterZ = 0.0;
   int locRunNumber = loop->GetJEvent().GetRunNumber();
   LockRead();
   {
      locTargetCenterZ = dTargetCenterZMap[locRunNumber];
   }
   UnlockRead();
   DVector3 locPosition(0.0, 0.0, locTargetCenterZ);

   vector<DMCReaction*> dmcreactions;

   const hddm_s::ReactionList &reacts = record->getReactions();
   hddm_s::ReactionList::iterator iter;
   for (iter = reacts.begin(); iter != reacts.end(); ++iter) {
      DMCReaction *mcreaction = new DMCReaction;
      dmcreactions.push_back(mcreaction);
      mcreaction->type = iter->getType();
      mcreaction->weight = iter->getWeight();
      hddm_s::Origin &origin = iter->getVertex().getOrigin();
      double torig = origin.getT();
      double zorig = origin.getVz();
      
      const hddm_s::BeamList &beams = record->getBeams();
      if (beams.size() > 0) {
         hddm_s::Beam &beam = iter->getBeam();
         DVector3 mom(beam.getMomentum().getPx(),
                      beam.getMomentum().getPy(),
                      beam.getMomentum().getPz());
         mcreaction->beam.setPosition(locPosition);
         mcreaction->beam.setMomentum(mom);
         mcreaction->beam.setPID(Gamma);
         mcreaction->target.setPID(IDTrack(mcreaction->beam.charge(),
                                           mcreaction->beam.mass()));
         mcreaction->beam.setTime(torig - (zorig - locTargetCenterZ)/29.9792458);
      }
      else {
         // fake values for DMCReaction
         mcreaction->beam.setPosition(locPosition);
         mcreaction->beam.setPID(Gamma);
      }

      const hddm_s::TargetList &targets = record->getTargets();
      if (targets.size() > 0) {
         hddm_s::Target &target = iter->getTarget();
         DKinematicData target_kd;
         DVector3 mom(target.getMomentum().getPx(),
                      target.getMomentum().getPy(),
                      target.getMomentum().getPz());
         mcreaction->target.setPosition(locPosition);
         mcreaction->target.setMomentum(mom);
         mcreaction->target.setPID(IDTrack(target.getProperties().getCharge(),
        		 target.getProperties().getMass()));
         mcreaction->target.setTime(torig - (zorig - locTargetCenterZ)/29.9792458);
      }
      else {
         // fake values for DMCReaction
         mcreaction->target.setPosition(locPosition);
      }
   }
   
   // Copy into factories
   //_DBG_<<"Creating "<<dmcreactions.size()<<" DMCReaction objects"<<endl;

   factory->CopyTo(dmcreactions);

   return NOERROR;
}

//------------------
// Extract_DMCThrown
//------------------
jerror_t DEventSourceHDDM::Extract_DMCThrown(hddm_s::HDDM *record,
                                   JFactory<DMCThrown> *factory, string tag)
{
   /// Copies the data from the given hddm_s structure. This is called
   /// from JEventSourceHDDM::GetObjects. If factory is NULL, this
   /// returns OBJECT_NOT_AVAILABLE immediately.
   
   if (factory == NULL)
      return OBJECT_NOT_AVAILABLE;
   if (tag != "")
      return OBJECT_NOT_AVAILABLE;
   
   vector<DMCThrown*> data;

   const hddm_s::VertexList &verts = record->getVertices();
   hddm_s::VertexList::iterator iter;
   for (iter = verts.begin(); iter != verts.end(); ++iter) {
      const hddm_s::OriginList &origs = iter->getOrigins();
      const hddm_s::ProductList &prods = iter->getProducts();
      double vertex[4] = {0., 0., 0., 0.};
      if (origs.size() > 0) {
         vertex[0] = iter->getOrigin().getT();
         vertex[1] = iter->getOrigin().getVx();
         vertex[2] = iter->getOrigin().getVy();
         vertex[3] = iter->getOrigin().getVz();
      }
      hddm_s::ProductList::iterator piter;
      for (piter = prods.begin(); piter != prods.end(); ++piter) {
         double E  = piter->getMomentum().getE();
         double px = piter->getMomentum().getPx();
         double py = piter->getMomentum().getPy();
         double pz = piter->getMomentum().getPz();
         double mass = sqrt(E*E - (px*px + py*py + pz*pz));
         if (!isfinite(mass))
            mass = 0.0;
         DMCThrown *mcthrown = new DMCThrown;
         mcthrown->type     = piter->getType();
         mcthrown->myid     = piter->getId();
         mcthrown->parentid = piter->getParentid();
         mcthrown->mech     = piter->getMech();
         mcthrown->pdgtype  = piter->getPdgtype();
         mcthrown->setPID((Particle_t)mcthrown->type);
         mcthrown->setMomentum(DVector3(px, py, pz));
         mcthrown->setPosition(DVector3(vertex[1], vertex[2], vertex[3]));
         mcthrown->setTime(vertex[0]);
         data.push_back(mcthrown);
      }
   }
   
   // Copy into factory
   factory->CopyTo(data);

   return NOERROR;
}

//------------------
// Extract_DCDCHit
//------------------
jerror_t DEventSourceHDDM::Extract_DCDCHit(JEventLoop* locEventLoop, hddm_s::HDDM *record,
                                   JFactory<DCDCHit> *factory, string tag)
{
   /// Copies the data from the given hddm_s structure. This is called
   /// from JEventSourceHDDM::GetObjects. If factory is NULL, this
   /// returns OBJECT_NOT_AVAILABLE immediately.
   
   if (factory == NULL)
       return OBJECT_NOT_AVAILABLE;

   // Since we are writing out CDC hits with the new "Calib" tag by default
   // assume that is what we are reading in, so that we don't support the
   // default tag anymore
   // sdobbs -- 3/13/2018
   //if (tag != "" && tag != "TRUTH" && tag != "Calib")
   if (tag != "TRUTH" && tag != "Calib")
      return OBJECT_NOT_AVAILABLE;
   
   vector<DCDCHit*> data;

   if ( tag == "" || tag == "Calib" ) {
      vector<const DCDCHit*> locTruthHits;
      locEventLoop->Get(locTruthHits, "TRUTH");

		//pre-sort truth hits
		map<pair<int, int>, vector<const DCDCHit*>> locTruthHitMap; //key pair: ring, straw
		for(auto& locTruthHit : locTruthHits)
			locTruthHitMap[std::make_pair(locTruthHit->ring, locTruthHit->straw)].push_back(locTruthHit);

        const hddm_s::CdcStrawHitList &hits = record->getCdcStrawHits();
        hddm_s::CdcStrawHitList::iterator iter;
        int locIndex = 0;
        for (iter = hits.begin(); iter != hits.end(); ++iter) {
            DCDCHit *hit = new DCDCHit;
            hit->ring   = iter->getRing();
            hit->straw  = iter->getStraw();
            hit->q      = iter->getQ();
            hit->t      = iter->getT();
            if(iter->getCdcDigihits().size() > 0) {
                hit->amp  = iter->getCdcDigihit().getPeakAmp();
            }
	    else{  
	      // for generated events (not folded-in background events) for which we
	      // have no digi hits return q
	      hit->amp=hit->q;
	    }
            hit->QF     = 0;
            if(iter->getCdcHitQFs().size() > 0) {
                hit->QF  = iter->getCdcHitQF().getQF();
            }            
            hit->d      = 0.; // initialize to zero to avoid any NaN
            hit->itrack = 0;  // track information is in TRUTH tag
            hit->ptype  = 0;  // ditto

			//match hit between truth & recon
			auto& locPotentialTruthHits = locTruthHitMap[std::make_pair(hit->ring, hit->straw)];
			double locBestDeltaT = 9.9E99;
			const DCDCHit* locBestTruthHit = nullptr;
			for(auto& locTruthHit : locPotentialTruthHits)
			{
				auto locDeltaT = fabs(hit->t - locTruthHit->t);
				if(locDeltaT >= locBestDeltaT)
					continue;
				locBestDeltaT = locDeltaT;
				locBestTruthHit = locTruthHit;
			}
			if(locBestTruthHit != nullptr)
           hit->AddAssociatedObject(locBestTruthHit);

         data.push_back(hit);
         ++locIndex;
      }
   }
   else if (tag == "TRUTH") {
      const hddm_s::CdcStrawTruthHitList &thits = record->getCdcStrawTruthHits();
      hddm_s::CdcStrawTruthHitList::iterator iter;
      for (iter = thits.begin(); iter != thits.end(); ++iter) {
         DCDCHit *hit = new DCDCHit;
         hit->ring   = iter->getRing();
         hit->straw  = iter->getStraw();
         hit->q      = iter->getQ();
         hit->t      = iter->getT();
         hit->d      = iter->getD();
         hit->itrack = iter->getItrack();
         hit->ptype  = iter->getPtype();
         data.push_back(hit);
      }
   }
   
   // Copy into factory
   factory->CopyTo(data);

   return NOERROR;
}


//------------------
// Extract_DFDCHit
//------------------
jerror_t DEventSourceHDDM::Extract_DFDCHit(hddm_s::HDDM *record,
                                   JFactory<DFDCHit> *factory, string tag)
{
   /// Copies the data from the given hddm_s structure. This is called
   /// from JEventSourceHDDM::GetObjects. If factory is NULL, this
   /// returns OBJECT_NOT_AVAILABLE immediately.
   
   if (factory == NULL)
      return OBJECT_NOT_AVAILABLE;
   if (tag != "" && tag != "TRUTH" && tag != "CALIB")
      return OBJECT_NOT_AVAILABLE;
   
   vector<DFDCHit*> data;

   if (tag == "") {
      const hddm_s::FdcAnodeHitList &ahits = record->getFdcAnodeHits();
      hddm_s::FdcAnodeHitList::iterator ahiter;
      for (ahiter = ahits.begin(); ahiter != ahits.end(); ++ahiter) {
         DFDCHit* newHit = new DFDCHit();
         newHit->layer   = ahiter->getLayer();
         newHit->module  = ahiter->getModule();
         newHit->element = ahiter->getWire();
         newHit->q       = ahiter->getDE();
         newHit->pulse_height = 0.;     // not measured
         newHit->t       = ahiter->getT();
         newHit->d       = 0.; // initialize to zero to avoid any NaN
         newHit->itrack  = 0;  // track information is in TRUTH tag
         newHit->ptype   = 0;  // ditto
         newHit->plane   = 2;
         newHit->type    = 0;
         newHit->gPlane  = DFDCGeometry::gPlane(newHit);
         newHit->gLayer  = DFDCGeometry::gLayer(newHit);
         newHit->r       = DFDCGeometry::getWireR(newHit);
         data.push_back(newHit);
      }

      // Ditto for the cathodes.
      const hddm_s::FdcCathodeHitList &chits = record->getFdcCathodeHits();
      hddm_s::FdcCathodeHitList::iterator chiter;
      for (chiter = chits.begin(); chiter != chits.end(); ++chiter) {
         DFDCHit* newHit = new DFDCHit();
         newHit->layer   = chiter->getLayer();
         newHit->module  = chiter->getModule();
         newHit->element = chiter->getStrip();
         if (newHit->element > 1000)
            newHit->element -= 1000;
         newHit->plane   = chiter->getPlane();
         newHit->q       = chiter->getQ();
         newHit->pulse_height = newHit->q;
         if(chiter->getFdcDigihits().size() > 0) {
             newHit->pulse_height  = chiter->getFdcDigihit().getPeakAmp();
         }
         newHit->t       = chiter->getT();
         newHit->d       = 0.; // initialize to zero to avoid any NaN
         newHit->itrack  = 0;  // track information is in TRUTH tag
         newHit->ptype   = 0;  // ditto
         newHit->type    = 1;
         newHit->gPlane  = DFDCGeometry::gPlane(newHit);    
         newHit->gLayer  = DFDCGeometry::gLayer(newHit);
         newHit->r       = DFDCGeometry::getStripR(newHit);
         data.push_back(newHit);
      }
   }

   else if (tag == "TRUTH"){
      const hddm_s::FdcAnodeTruthHitList &aths = record->getFdcAnodeTruthHits();
      hddm_s::FdcAnodeTruthHitList::iterator atiter;
      for (atiter = aths.begin(); atiter != aths.end(); ++atiter) {
         DFDCHit* newHit = new DFDCHit();
         newHit->layer   = atiter->getLayer();
         newHit->module  = atiter->getModule();
         newHit->element = atiter->getWire();
         newHit->q       = atiter->getDE();
	 newHit->pulse_height=0.; // not measured
         newHit->t       = atiter->getT();
         newHit->d       = atiter->getD();
         newHit->itrack  = atiter->getItrack();
         newHit->ptype   = atiter->getPtype();
         newHit->plane   = 2;
         newHit->type    = 0;
         newHit->gPlane  = DFDCGeometry::gPlane(newHit);
         newHit->gLayer  = DFDCGeometry::gLayer(newHit);
         newHit->r       = DFDCGeometry::getWireR(newHit);
         data.push_back(newHit);
      }
          
      // Ditto for the cathodes.
      const hddm_s::FdcCathodeTruthHitList &cths = 
                                           record->getFdcCathodeTruthHits();
      hddm_s::FdcCathodeTruthHitList::iterator ctiter;
      for (ctiter = cths.begin(); ctiter != cths.end(); ++ctiter) {
         DFDCHit* newHit = new DFDCHit();
         newHit->layer   = ctiter->getLayer();
         newHit->module  = ctiter->getModule();
         newHit->element = ctiter->getStrip();
         if (newHit->element > 1000)
            newHit->element -= 1000;
         newHit->plane   = ctiter->getPlane();
         newHit->q       = ctiter->getQ();
	 newHit->pulse_height = newHit->q;
         newHit->t       = ctiter->getT();
         newHit->d       = 0.; // initialize to zero to avoid any NaN
         newHit->itrack  = ctiter->getItrack();
         newHit->ptype   = ctiter->getPtype();
         newHit->type    = 1;
         newHit->gPlane  = DFDCGeometry::gPlane(newHit);    
         newHit->gLayer  = DFDCGeometry::gLayer(newHit);
         newHit->r       = DFDCGeometry::getStripR(newHit);
         data.push_back(newHit);
      }
   }

   else if (tag == "CALIB") {
      // Deal with the wires
      const hddm_s::FdcAnodeHitList &ahits = record->getFdcAnodeHits();
      hddm_s::FdcAnodeHitList::iterator ahiter;
      for (ahiter = ahits.begin(); ahiter != ahits.end(); ++ahiter) {
         DFDCHit* newHit = new DFDCHit();
         newHit->layer   = ahiter->getLayer();
         newHit->module  = ahiter->getModule();
         newHit->element = ahiter->getWire();
         newHit->q       = ahiter->getDE();
         newHit->t       = ahiter->getT();
         newHit->d       = 0.; // initialize to zero to avoid any NaN
         newHit->itrack  = 0;  // track information is in TRUTH tag
         newHit->ptype   = 0;  // ditto
         newHit->plane   = 2;
         newHit->type    = 0;
         newHit->gPlane  = DFDCGeometry::gPlane(newHit);
         newHit->gLayer  = DFDCGeometry::gLayer(newHit);
         newHit->r       = DFDCGeometry::getWireR(newHit);
         data.push_back(newHit);
      }
          
      // Ditto for the cathodes.
      const hddm_s::FdcCathodeHitList &chits = record->getFdcCathodeHits();
      hddm_s::FdcCathodeHitList::iterator chiter;
      for (chiter = chits.begin(); chiter != chits.end(); ++chiter) {
         DFDCHit* newHit = new DFDCHit();
         newHit->layer   = chiter->getLayer();
         newHit->module  = chiter->getModule();
         newHit->element = chiter->getStrip();
         if (newHit->element > 1000)
            newHit->element-=1000;
         newHit->plane   = chiter->getPlane();
         if (newHit->plane == 1) // v 
            newHit->q = chiter->getQ()*vscale[newHit->element-1];
         else  // u
            newHit->q = chiter->getQ()*uscale[newHit->element-1];
         newHit->t       = chiter->getT();
         newHit->d       = 0.; // initialize to zero to avoid any NaN
         newHit->itrack  = 0;  // track information is in TRUTH tag
         newHit->ptype   = 0;  // ditto
         newHit->type    = 1;
         newHit->gPlane  = DFDCGeometry::gPlane(newHit);    
         newHit->gLayer  = DFDCGeometry::gLayer(newHit);
         newHit->r       = DFDCGeometry::getStripR(newHit);
         data.push_back(newHit);
      }
   }
   
   // Copy into factory
   factory->CopyTo(data);
   
   return NOERROR;
}

//------------------
// Extract_DBCALTruthShower
//------------------
jerror_t DEventSourceHDDM::Extract_DBCALTruthShower(hddm_s::HDDM *record,
                                   JFactory<DBCALTruthShower> *factory,
                                   string tag)
{
   /// Copies the data from the given hddm_s structure. This is called
   /// from JEventSourceHDDM::GetObjects. If factory is NULL, this
   /// returns OBJECT_NOT_AVAILABLE immediately.

   if (factory == NULL)
      return OBJECT_NOT_AVAILABLE;
   if (tag != "")
      return OBJECT_NOT_AVAILABLE;

   vector<DBCALTruthShower*> data;

   const hddm_s::BcalTruthShowerList &shows = record->getBcalTruthShowers();
   hddm_s::BcalTruthShowerList::iterator iter;
   for (iter = shows.begin(); iter != shows.end(); ++iter) {
      DBCALTruthShower *bcaltruth = new DBCALTruthShower;
      bcaltruth->track   = iter->getTrack();
      bcaltruth->ptype   = iter->getPtype();
      bcaltruth->primary = (iter->getPrimary())? 1 : 0;
      bcaltruth->phi     = iter->getPhi();
      bcaltruth->r       = iter->getR();
      bcaltruth->z       = iter->getZ();
      bcaltruth->t       = iter->getT();
      bcaltruth->E       = iter->getE();
      bcaltruth->px      = iter->getPx();
      bcaltruth->py      = iter->getPy();
      bcaltruth->pz      = iter->getPz();
      const hddm_s::TrackIDList &ids = iter->getTrackIDs();
      bcaltruth->itrack = (ids.size())? ids.begin()->getItrack() : 0;
      data.push_back(bcaltruth);
   }

   // Copy into factory
   factory->CopyTo(data);

   return NOERROR;
}

//------------------
// Extract_DBCALTruthCell
//------------------
jerror_t DEventSourceHDDM::Extract_DBCALTruthCell(hddm_s::HDDM *record,
                                   JFactory<DBCALTruthCell> *factory, 
                                   string tag)
{
   /// Copies the data from the given hddm_s structure. This is called
   /// from JEventSourceHDDM::GetObjects. If factory is NULL, this
   /// returns OBJECT_NOT_AVAILABLE immediately.

   if (factory == NULL)
      return OBJECT_NOT_AVAILABLE;
   if (tag != "")
      return OBJECT_NOT_AVAILABLE;

   vector<DBCALTruthCell*> data;

   const hddm_s::BcalTruthHitList &hits = record->getBcalTruthHits();
   hddm_s::BcalTruthHitList::iterator hiter;
   for (hiter = hits.begin(); hiter != hits.end(); ++hiter) {
      DBCALTruthCell *truthcell = new DBCALTruthCell();
      truthcell->module = hiter->getModule();
      truthcell->layer  = hiter->getLayer();
      truthcell->sector = hiter->getSector();
      truthcell->E      = hiter->getE();
      truthcell->t      = hiter->getT();
      truthcell->zLocal = hiter->getZLocal();
      data.push_back(truthcell);
   }

   // Copy into factory
   factory->CopyTo(data);

   return NOERROR;
}

//------------------
// Extract_DFCALTruthShower
//------------------
jerror_t DEventSourceHDDM::Extract_DFCALTruthShower(hddm_s::HDDM *record,
                                   JFactory<DFCALTruthShower> *factory,
                                   string tag)
{
   /// Copies the data from the given hddm_s structure. This is called
   /// from JEventSourceHDDM::GetObjects. If factory is NULL, this
   /// returns OBJECT_NOT_AVAILABLE immediately.
   
   if (factory == NULL)
      return OBJECT_NOT_AVAILABLE;
   if (tag != "")
      return OBJECT_NOT_AVAILABLE;
   
   vector<DFCALTruthShower*> data;
   JObject::oid_t id=1;

   const hddm_s::FcalTruthShowerList &shows = record->getFcalTruthShowers();
   hddm_s::FcalTruthShowerList::iterator iter;
   for (iter = shows.begin(); iter != shows.end(); ++iter) {
      const hddm_s::TrackIDList &ids = iter->getTrackIDs();
      int itrack = (ids.size())? ids.begin()->getItrack() : 0;
      DFCALTruthShower *dfcaltruthshower = new DFCALTruthShower(
            id++,
            iter->getX(),
            iter->getY(),
            iter->getZ(),
            iter->getPx(),
            iter->getPy(),
            iter->getPz(),
            iter->getE(),
            iter->getT(),
            iter->getPrimary(),
            iter->getTrack(),
            iter->getPtype(),
            itrack
            );
      data.push_back(dfcaltruthshower);
   }

   // Copy into factory
   factory->CopyTo(data);

   return NOERROR;
}

//------------------
// Extract_DFCALHit
//------------------
jerror_t DEventSourceHDDM::Extract_DFCALHit(hddm_s::HDDM *record,
                                   JFactory<DFCALHit> *factory, string tag,
                                   JEventLoop* eventLoop)
{
  /// Copies the data from the given hddm_s structure. This is called
  /// from JEventSourceHDDM::GetObjects. If factory is NULL, this
  /// returs OBJECT_NOT_AVAILABLE immediately.
   
  if (factory == NULL)
     return OBJECT_NOT_AVAILABLE;
   if (tag != "" && tag != "TRUTH")
      return OBJECT_NOT_AVAILABLE;

   // extract the FCAL Geometry (for isBlockActive() and positionOnFace())
   vector<const DFCALGeometry*> fcalGeomVect;
   eventLoop->Get( fcalGeomVect );
   if (fcalGeomVect.size() < 1)
      return OBJECT_NOT_AVAILABLE;
   const DFCALGeometry& fcalGeom = *(fcalGeomVect[0]);

   vector<DFCALHit*> data;

   if (tag == "") {
      const hddm_s::FcalHitList &hits = record->getFcalHits();
      hddm_s::FcalHitList::iterator iter;
      for (iter = hits.begin(); iter != hits.end(); ++iter) {
         int row = iter->getRow();
         int column = iter->getColumn();
 
         // Filter out non-physical blocks here
         if (!fcalGeom.isBlockActive(row, column))
            continue;

         // Get position of blocks on front face. (This should really come from
         // hdgeant directly so the positions can be shifted in mcsmear.)
         DVector2 pos = fcalGeom.positionOnFace(row, column);

         DFCALHit *mchit = new DFCALHit();
         mchit->row    = row;
         mchit->column = column;
         mchit->x      = pos.X();
         mchit->y      = pos.Y();
         mchit->E      = iter->getE();
         mchit->t      = iter->getT();
         mchit->intOverPeak = 6.;
         if(iter->getFcalDigihits().size() > 0) {
             mchit->intOverPeak  = iter->getFcalDigihit().getIntegralOverPeak();
         }
         data.push_back(mchit);
       }
    }
    else if (tag == "TRUTH") {
      const hddm_s::FcalTruthHitList &hits = record->getFcalTruthHits();
      hddm_s::FcalTruthHitList::iterator iter;
      for (iter = hits.begin(); iter != hits.end(); ++iter) {
         int row = iter->getRow();
         int column = iter->getColumn();
 
         // Filter out non-physical blocks here
         if (!fcalGeom.isBlockActive(row, column))
            continue;

         // Get position of blocks on front face. (This should really come from
         // hdgeant directly so the positions can be shifted in mcsmear.)
         DVector2 pos = fcalGeom.positionOnFace(row, column);

         DFCALHit *mchit = new DFCALHit();
         mchit->row    = row;
         mchit->column = column;
         mchit->x      = pos.X();
         mchit->y      = pos.Y();
         mchit->E      = iter->getE();
         mchit->t      = iter->getT();
	 mchit->intOverPeak = 6.;
         data.push_back(mchit);
      }
   }
  
  // Copy into factory
  factory->CopyTo(data);
  
  return NOERROR;
}

//------------------
// Extract_DCCALTruthShower
//------------------
jerror_t DEventSourceHDDM::Extract_DCCALTruthShower(hddm_s::HDDM *record,
                                   JFactory<DCCALTruthShower> *factory,
                                   string tag)
{
   /// Copies the data from the given hddm_s structure. This is called
   /// from JEventSourceHDDM::GetObjects. If factory is NULL, this
   /// returns OBJECT_NOT_AVAILABLE immediately.
   
   if (factory == NULL)
      return OBJECT_NOT_AVAILABLE;
   if (tag != "")
      return OBJECT_NOT_AVAILABLE;
   
   vector<DCCALTruthShower*> data;
   JObject::oid_t id=1;

   const hddm_s::CcalTruthShowerList &shows = record->getCcalTruthShowers();
   hddm_s::CcalTruthShowerList::iterator iter;
   for (iter = shows.begin(); iter != shows.end(); ++iter) {
      const hddm_s::TrackIDList &ids = iter->getTrackIDs();
      int itrack = (ids.size())? ids.begin()->getItrack() : 0;
      DCCALTruthShower *dccaltruthshower = new DCCALTruthShower(
            id++,
            iter->getX(),
            iter->getY(),
            iter->getZ(),
            iter->getPx(),
            iter->getPy(),
            iter->getPz(),
            iter->getE(),
            iter->getT(),
            iter->getPrimary(),
            iter->getTrack(),
            iter->getPtype(),
            itrack
            );
      data.push_back(dccaltruthshower);
   }

   // Copy into factory
   factory->CopyTo(data);

   return NOERROR;
}

//------------------
// Extract_DCCALHit
//------------------
jerror_t DEventSourceHDDM::Extract_DCCALHit(hddm_s::HDDM *record,
                                   JFactory<DCCALHit> *factory, string tag,
                                   JEventLoop* eventLoop)
{
   /// Copies the data from the given hddm_s structure. This is called
   /// from JEventSourceHDDM::GetObjects. If factory is NULL, this
   /// returs OBJECT_NOT_AVAILABLE immediately.

   if (factory == NULL)
      return OBJECT_NOT_AVAILABLE;
   if (tag != "" && tag != "TRUTH")
      return OBJECT_NOT_AVAILABLE;

   // extract the CCAL Geometry (for isBlockActive() and positionOnFace())
   vector<const DCCALGeometry*> ccalGeomVect;
   eventLoop->Get( ccalGeomVect );
   if (ccalGeomVect.size() < 1)
      return OBJECT_NOT_AVAILABLE;
   const DCCALGeometry& ccalGeom = *(ccalGeomVect[0]);

   vector<DCCALHit*> data;
   int hitId = 0;

   if (tag == "") {
      const hddm_s::CcalHitList &hits = record->getCcalHits();
      hddm_s::CcalHitList::iterator iter;
      for (iter = hits.begin(); iter != hits.end(); ++iter) {
         int row = iter->getRow();
         int column = iter->getColumn();
 
         // Filter out non-physical blocks here
         if (!ccalGeom.isBlockActive(row, column))
            continue;

         // Get position of blocks on front face. (This should really come from
         // hdgeant directly so the poisitions can be shifted in mcsmear.)
         DVector2 pos = ccalGeom.positionOnFace(row, column);

         DCCALHit *mchit = new DCCALHit();
         mchit->row    = row;
         mchit->column = column;
         mchit->x      = pos.X();
         mchit->y      = pos.Y();
         mchit->E      = iter->getE();
         mchit->t      = iter->getT();
         mchit->id     = hitId++;
         data.push_back(mchit);
      }
   }

   else if (tag == "TRUTH") {
      const hddm_s::CcalTruthHitList &hits = record->getCcalTruthHits();
      hddm_s::CcalTruthHitList::iterator iter;
      for (iter = hits.begin(); iter != hits.end(); ++iter) {
         int row = iter->getRow();
         int column = iter->getColumn();
 
         // Filter out non-physical blocks here
         if (!ccalGeom.isBlockActive(row, column))
            continue;

         // Get position of blocks on front face. (This should really come from
         // hdgeant directly so the poisitions can be shifted in mcsmear.)
         DVector2 pos = ccalGeom.positionOnFace(row, column);

         DCCALHit *mchit = new DCCALHit();
         mchit->row    = row;
         mchit->column = column;
         mchit->x      = pos.X();
         mchit->y      = pos.Y();
         mchit->E      = iter->getE();
         mchit->t      = iter->getT();
         mchit->id     = hitId++;
         data.push_back(mchit);
      }
   }
  
  // Copy into factory
  factory->CopyTo(data);
  
  return NOERROR;
}

//------------------
// Extract_DMCTrajectoryPoint
//------------------
jerror_t DEventSourceHDDM::Extract_DMCTrajectoryPoint(hddm_s::HDDM *record,
                                   JFactory<DMCTrajectoryPoint> *factory, 
                                   string tag)
{
   /// Copies the data from the given hddm_s structure. This is called
   /// from JEventSourceHDDM::GetObjects. If factory is NULL, this
   /// returns OBJECT_NOT_AVAILABLE immediately.
   
   if (factory == NULL)
      return OBJECT_NOT_AVAILABLE;
   if (tag != "")
      return OBJECT_NOT_AVAILABLE;
   
   vector<DMCTrajectoryPoint*> data;

   const hddm_s::McTrajectoryPointList &pts = record->getMcTrajectoryPoints();
   hddm_s::McTrajectoryPointList::iterator iter;
   for (iter = pts.begin(); iter != pts.end(); ++iter) {
      DMCTrajectoryPoint *p = new DMCTrajectoryPoint;
      p->x             = iter->getX();
      p->y             = iter->getY();
      p->z             = iter->getZ();
      p->t             = iter->getT();
      p->px            = iter->getPx();
      p->py            = iter->getPy();
      p->pz            = iter->getPz();
      p->E             = iter->getE();
      p->dE            = iter->getDE();
      p->primary_track = iter->getPrimary_track();
      p->track         = iter->getTrack();
      p->part          = iter->getPart();
      p->radlen        = iter->getRadlen();
      p->step          = iter->getStep();
      p->mech          = iter->getMech();
      data.push_back(p);
   }
   
   // Copy into factory
   factory->CopyTo(data);

   return NOERROR;
}

//------------------
// Extract_DTOFTruth
//------------------
jerror_t DEventSourceHDDM::Extract_DTOFTruth(hddm_s::HDDM *record,
                                   JFactory<DTOFTruth>* factory, string tag)
{
   /// Copies the data from the given hddm_s structure. This is called
   /// from JEventSourceHDDM::GetObjects. If factory is NULL, this
   /// returns OBJECT_NOT_AVAILABLE immediately.

   if (factory == NULL)
      return OBJECT_NOT_AVAILABLE;
   if (tag != "")
      return OBJECT_NOT_AVAILABLE;
  
   vector<DTOFTruth*> data;

   const hddm_s::FtofTruthPointList &points = record->getFtofTruthPoints();
   hddm_s::FtofTruthPointList::iterator iter;
   for (iter = points.begin(); iter != points.end(); ++iter) {
      DTOFTruth *toftruth = new DTOFTruth;
      toftruth->primary = iter->getPrimary();
      toftruth->track   = iter->getTrack();
      toftruth->x       = iter->getX();
      toftruth->y       = iter->getY();
      toftruth->z       = iter->getZ();
      toftruth->t       = iter->getT();
      toftruth->px      = iter->getPx();
      toftruth->py      = iter->getPy();
      toftruth->pz      = iter->getPz();
      toftruth->E       = iter->getE();
      toftruth->ptype   = iter->getPtype();
      const hddm_s::TrackIDList &ids = iter->getTrackIDs();
      toftruth->itrack = (ids.size())? ids.begin()->getItrack() : 0;
      data.push_back(toftruth);
   }

   // Copy into factory
   factory->CopyTo(data);

   return NOERROR;
}

//------------------
// Extract_DTOFHit
//------------------
jerror_t DEventSourceHDDM::Extract_DTOFHit( hddm_s::HDDM *record,
                                   JFactory<DTOFHit>* factory,
                                   JFactory<DTOFHitMC> *factoryMC,
                                   string tag)
{
   /// Copies the data from the given hddm_s structure. This is called
   /// from JEventSourceHDDM::GetObjects. If factory is NULL, this
   /// returns OBJECT_NOT_AVAILABLE immediately.
   
   if (factory == NULL)
      return OBJECT_NOT_AVAILABLE;
   if (tag != "" && tag != "TRUTH")
      return OBJECT_NOT_AVAILABLE;

   vector<DTOFHit*> data;
   vector<DTOFHitMC*> dataMC;

   const hddm_s::FtofCounterList &ctrs = record->getFtofCounters();
   hddm_s::FtofCounterList::iterator iter;
   for (iter = ctrs.begin(); iter != ctrs.end(); ++iter) {
      if (tag == "") {
         vector<DTOFHit*> north_hits;
         vector<DTOFHit*> south_hits;

         // Loop over north AND south hits
         const hddm_s::FtofHitList &hits = iter->getFtofHits();
         hddm_s::FtofHitList::iterator hiter;
         for (hiter = hits.begin(); hiter != hits.end(); ++hiter) {
            DTOFHit *tofhit = new DTOFHit;
            tofhit->bar   = hiter->getBar();
            tofhit->plane = hiter->getPlane();
            tofhit->end   = hiter->getEnd();
            tofhit->dE    = hiter->getDE();
            tofhit->Amp   = 0.;
            if(hiter->getFtofDigihits().size() > 0) {
                tofhit->Amp  = hiter->getFtofDigihit().getPeakAmp();
            }
            tofhit->t     = hiter->getT();
            tofhit->t_TDC = tofhit->t;
            tofhit->t_fADC= tofhit->t;          
            tofhit->has_TDC=true;
            tofhit->has_fADC=true;
            data.push_back(tofhit);
            if (tofhit->end == 0)
               north_hits.push_back(tofhit);
            else 
               south_hits.push_back(tofhit);
         }

         // return truth hits in a different factory
         const hddm_s::FtofTruthHitList &truths = iter->getFtofTruthHits();
         hddm_s::FtofTruthHitList::iterator titer;
         unsigned int north_mchits = 0;
         unsigned int south_mchits = 0;
         for (titer = truths.begin(); titer != truths.end(); ++titer) {
            DTOFHitMC *tofmchit = new DTOFHitMC;
            tofmchit->bar    = titer->getBar();
            tofmchit->plane  = titer->getPlane();
            tofmchit->end    = titer->getEnd();
            tofmchit->itrack = titer->getFtofTruthExtra(0).getItrack();
            tofmchit->ptype  = titer->getFtofTruthExtra(0).getPtype();
            tofmchit->dist   = titer->getFtofTruthExtra(0).getDist();
            tofmchit->x      = titer->getFtofTruthExtra(0).getX();
            tofmchit->y      = titer->getFtofTruthExtra(0).getY();
            tofmchit->z      = titer->getFtofTruthExtra(0).getZ();
            tofmchit->px     = titer->getFtofTruthExtra(0).getPx();
            tofmchit->py     = titer->getFtofTruthExtra(0).getPy();
            tofmchit->pz     = titer->getFtofTruthExtra(0).getPz();
            tofmchit->E      = titer->getFtofTruthExtra(0).getE();
            dataMC.push_back(tofmchit);

            // best-guess at tofhit-tofMChit association, not exact
            if (tofmchit->end == 0) {
               if (north_mchits < north_hits.size()) {
                  north_hits[north_mchits]->AddAssociatedObject(tofmchit);
               }
               north_mchits++;
            }
            else {
               if (south_mchits < south_hits.size()) {
                  south_hits[south_mchits]->AddAssociatedObject(tofmchit);
               }
               south_mchits++;
            }
         }
     }

     else if (tag == "TRUTH") {
        const hddm_s::FtofTruthHitList &truths = iter->getFtofTruthHits();
        hddm_s::FtofTruthHitList::iterator titer;
        for (titer = truths.begin(); titer != truths.end(); ++titer) {
           DTOFHit *tofhit = new DTOFHit;
           tofhit->bar   = titer->getBar();
           tofhit->plane = titer->getPlane();
           tofhit->end   = titer->getEnd();
           tofhit->dE    = titer->getDE();
           tofhit->t     = titer->getT();
	   tofhit->t_fADC= tofhit->t;
	   tofhit->t_TDC = tofhit->t;
	   tofhit->has_TDC=true;
	   tofhit->has_fADC=true;
           data.push_back(tofhit);

           DTOFHitMC *tofmchit = new DTOFHitMC;
           tofmchit->bar    = tofhit->bar;
           tofmchit->plane  = tofhit->plane;
           tofmchit->end    = tofhit->end;
           tofmchit->itrack = titer->getFtofTruthExtra().getItrack();
           tofmchit->ptype  = titer->getFtofTruthExtra().getPtype();
           tofmchit->dist   = titer->getFtofTruthExtra().getDist();
           tofmchit->x      = titer->getFtofTruthExtra().getX();
           tofmchit->y      = titer->getFtofTruthExtra().getY();
           tofmchit->z      = titer->getFtofTruthExtra().getZ();
           tofmchit->px     = titer->getFtofTruthExtra().getPx();
           tofmchit->py     = titer->getFtofTruthExtra().getPy();
           tofmchit->pz     = titer->getFtofTruthExtra().getPz();
           tofmchit->E      = titer->getFtofTruthExtra().getE();
           dataMC.push_back(tofmchit);
           tofhit->AddAssociatedObject(tofmchit);
        }
     }
   }

   // Copy into factory
   factory->CopyTo(data);
   factoryMC->CopyTo(dataMC);

   return NOERROR;
}

//------------------
// Extract_DSCHit
//------------------
jerror_t DEventSourceHDDM::Extract_DSCHit(hddm_s::HDDM *record,
                                   JFactory<DSCHit>* factory, string tag)
{
   /// Copies the data from the given hddm_s structure. This is called
   /// from JEventSourceHDDM::GetObjects. If factory is NULL, this
   /// returns OBJECT_NOT_AVAILABLE immediately.
   
   if (factory == NULL)
      return OBJECT_NOT_AVAILABLE;
   if (tag != "" && tag != "TRUTH")
      return OBJECT_NOT_AVAILABLE;

   vector<DSCHit*> data;
  
   if (tag == "") {
      const hddm_s::StcHitList &hits = record->getStcHits();
      hddm_s::StcHitList::iterator iter;
      for (iter = hits.begin(); iter != hits.end(); ++iter) {
         DSCHit *hit = new DSCHit;
         hit->sector = iter->getSector();
         hit->dE = iter->getDE();
         hit->t = iter->getT();
         hit->t_TDC=hit->t;
         hit->t_fADC=hit->t;
         hit->pulse_height = 0.;
         if(iter->getStcDigihits().size() > 0) {
             hit->pulse_height  = iter->getStcDigihit().getPeakAmp();
         }
         hit->has_TDC=true;
         hit->has_fADC=true;
         data.push_back(hit);
      }
   }
   else if (tag == "TRUTH") {
      const hddm_s::StcTruthHitList &hits = record->getStcTruthHits();
      hddm_s::StcTruthHitList::iterator iter;
      for (iter = hits.begin(); iter != hits.end(); ++iter) {
         DSCHit *hit = new DSCHit;
         hit->sector = iter->getSector();
         hit->dE = iter->getDE();
         hit->t = iter->getT();
         hit->t_TDC=hit->t;
         hit->t_fADC=hit->t;
         hit->has_TDC=true;
         hit->has_fADC=true;
         data.push_back(hit);
      }
   }

  // Copy into factory
  factory->CopyTo(data);

  return NOERROR;
}

//------------------
// Extract_DSCTruthHit
//------------------
jerror_t DEventSourceHDDM::Extract_DSCTruthHit(hddm_s::HDDM *record,
                                   JFactory<DSCTruthHit>* factory, string tag)
{
   /// Copies the data from the given hddm_s structure. This is called
   /// from JEventSourceHDDM::GetObjects. If factory is NULL, this
   /// returns OBJECT_NOT_AVAILABLE immediately.

   if (factory == NULL)
      return OBJECT_NOT_AVAILABLE;
   if (tag != "")
      return OBJECT_NOT_AVAILABLE;

   vector<DSCTruthHit*> data;

   const hddm_s::StcTruthPointList &points = record->getStcTruthPoints();
   hddm_s::StcTruthPointList::iterator iter;
   for (iter = points.begin(); iter != points.end(); ++iter) {
      DSCTruthHit *hit = new DSCTruthHit;
      hit->dEdx    = iter->getDEdx();
      hit->phi     = iter->getPhi();
      hit->primary = iter->getPrimary();
      hit->ptype   = iter->getPtype();
      hit->r       = iter->getR();
      hit->t       = iter->getT();
      hit->z       = iter->getZ();
      hit->track   = iter->getTrack();
      hit->sector  = iter->getSector();
      const hddm_s::TrackIDList &ids = iter->getTrackIDs();
      hit->itrack = (ids.size())? ids.begin()->getItrack() : 0;
      data.push_back(hit);
   }

   // Copy into factory
   factory->CopyTo(data);

   return NOERROR;
}

//------------------
// Extract_DTrackTimeBased
//------------------
jerror_t DEventSourceHDDM::Extract_DTrackTimeBased(hddm_s::HDDM *record,
                                   JFactory<DTrackTimeBased> *factory, 
                                   string tag, int32_t runnumber, JEventLoop* locEventLoop)
{
   // Note: Since this is a reconstructed factory, we want to generally return OBJECT_NOT_AVAILABLE
   // rather than NOERROR. The reason being that the caller interprets "NOERROR" to mean "yes I
   // usually can provide objects of that type, but this event has none." This will cause it to
   // skip any attempt at reconstruction. On the other hand, a value of "OBJECT_NOT_AVAILABLE" tells
   // it "I cannot provide those type of objects for this event."

   if (factory == NULL)
      return OBJECT_NOT_AVAILABLE;
   if (tag != "")
      return OBJECT_NOT_AVAILABLE;

   vector<DTrackTimeBased*> data;
   vector<DReferenceTrajectory*> rts;

   const hddm_s::TracktimebasedList &ttbs = record->getTracktimebaseds();
   hddm_s::TracktimebasedList::iterator iter;
   for (iter = ttbs.begin(); iter != ttbs.end(); ++iter) {
      DVector3 mom(iter->getMomentum().getPx(),
                   iter->getMomentum().getPy(),
                   iter->getMomentum().getPz());
      DVector3 pos(iter->getOrigin().getVx(),
                   iter->getOrigin().getVy(),
                   iter->getOrigin().getVz());
      DTrackTimeBased *track = new DTrackTimeBased();
      track->setMomentum(mom);
      track->setPosition(pos);
      track->setPID(IDTrack(iter->getProperties().getCharge(),
    		  iter->getProperties().getMass()));
      track->chisq = iter->getChisq();
      track->Ndof = iter->getNdof();
      track->FOM = iter->getFOM();
      track->candidateid = iter->getCandidateid();
      track->id = iter->getId();

      // Reconstitute errorMatrix
      auto locCovarianceMatrix = dResourcePool_TMatrixFSym->Get_SharedResource();
      locCovarianceMatrix->ResizeTo(7, 7);
      string str_vals = iter->getErrorMatrix().getVals();
      StringToTMatrixFSym(str_vals, locCovarianceMatrix.get(),
                          iter->getErrorMatrix().getNrows(),
                          iter->getErrorMatrix().getNcols());
      track->setErrorMatrix(locCovarianceMatrix);

      // Reconstitute TrackingErrorMatrix
      str_vals = iter->getTrackingErrorMatrix().getVals();
      auto TrackingErrorMatrix = dResourcePool_TMatrixFSym->Get_SharedResource();
      TrackingErrorMatrix->ResizeTo(5, 5);
      StringToTMatrixFSym(str_vals, TrackingErrorMatrix.get(),
                          iter->getTrackingErrorMatrix().getNrows(),
                          iter->getTrackingErrorMatrix().getNcols());
      track->setTrackingErrorMatrix(TrackingErrorMatrix);

      data.push_back(track);
   }

   // Copy into factory
   if (ttbs.size() > 0){
      factory->CopyTo(data);

      // If the event had a s_Tracktimebased_t pointer, then report
      // back that we read them in from the file. Otherwise, report
      // OBJECT_NOT_AVAILABLE
      return NOERROR;
   }

   // If we get to here then there was not even a placeholder in the HDDM file.
   // Return OBJECT_NOT_AVAILABLE to indicate reconstruction should be tried.
   return OBJECT_NOT_AVAILABLE;
}


//-------------------------------
// StringToTMatrixFSym
//-------------------------------
string DEventSourceHDDM::StringToTMatrixFSym(string &str_vals, TMatrixFSym* mat,
                                             int Nrows, int Ncols)
{
   /// This is the inverse of the DMatrixDSymToString method in the
   /// danahddm plugin.

   // Convert the given string into a symmetric matrix
   mat->ResizeTo(Nrows, Ncols);
   stringstream ss(str_vals);
   for (int irow=0; irow<mat->GetNrows(); irow++) {
      for (int icol=irow; icol<mat->GetNcols(); icol++) {
         ss >> (*mat)[irow][icol];
         (*mat)[icol][irow] = (*mat)[irow][icol];
      }
   }
   
   return ss.str();
}

//------------------
// Extract_DTAGMHit
//------------------
jerror_t DEventSourceHDDM::Extract_DTAGMHit(hddm_s::HDDM *record,
                                   JFactory<DTAGMHit>* factory,
                                   string tag)
{
   /// Copies the data from the given hddm_s structure. This is called
   /// from JEventSourceHDDM::GetObjects. If factory is NULL, this
   /// returns OBJECT_NOT_AVAILABLE immediately.

   if (factory == NULL)
      return OBJECT_NOT_AVAILABLE;
   if (tag != "" && tag != "TRUTH")
      return OBJECT_NOT_AVAILABLE;

   vector<DTAGMHit*> data;

   // loop over microChannel/taggerHit records
   const hddm_s::MicroChannelList &tags = record->getMicroChannels();
   hddm_s::MicroChannelList::iterator iter;
   for (iter = tags.begin(); iter != tags.end(); ++iter) {
      if (tag == "") {
         const hddm_s::TaggerHitList &hits = iter->getTaggerHits();
         hddm_s::TaggerHitList::iterator hiter;
         for (hiter = hits.begin(); hiter != hits.end(); ++hiter) {
            DTAGMHit *taghit = new DTAGMHit();
            taghit->E = hiter->getE();
            taghit->t = hiter->getT();
            taghit->npix_fadc = hiter->getNpe();
            taghit->time_fadc = hiter->getTADC();
            taghit->column = hiter->getColumn();
            taghit->row = hiter->getRow();
	    taghit->has_fADC = true;
	    taghit->has_TDC = true;         
	    data.push_back(taghit);
         }
      }
      else if (tag == "TRUTH") {
         const hddm_s::TaggerTruthHitList &hits = iter->getTaggerTruthHits();
         hddm_s::TaggerTruthHitList::iterator hiter;
         for (hiter = hits.begin(); hiter != hits.end(); ++hiter) {
            DTAGMHit *taghit = new DTAGMHit();
            taghit->E = hiter->getE();
            taghit->t = hiter->getT();
            taghit->npix_fadc = hiter->getDE() * 1e5; // ~1e5 pixels/GeV
            taghit->time_fadc = hiter->getT();
            taghit->column = hiter->getColumn();
            taghit->row = hiter->getRow();
	    taghit->has_fADC = true;
	    taghit->has_TDC = true;           
	    taghit->bg = hiter->getBg();
	    data.push_back(taghit);
         }
      }
   }

   // Copy into factory
   factory->CopyTo(data);

   return NOERROR;
}

//------------------
// Extract_DTAGHHit
//------------------
jerror_t DEventSourceHDDM::Extract_DTAGHHit( hddm_s::HDDM *record,
                                   JFactory<DTAGHHit>* factory,
                                   string tag)
{
   /// Copies the data from the given hddm_s structure. This is called
   /// from JEventSourceHDDM::GetObjects. If factory is NULL, this
   /// returns OBJECT_NOT_AVAILABLE immediately.

   if (factory == NULL)
      return OBJECT_NOT_AVAILABLE;
   if (tag != "" && tag != "TRUTH")
      return OBJECT_NOT_AVAILABLE;

   vector<DTAGHHit*> data;
  
   // loop over hodoChannel/taggerHit records
   const hddm_s::HodoChannelList &tags = record->getHodoChannels();
   hddm_s::HodoChannelList::iterator iter;
   for (iter = tags.begin(); iter != tags.end(); ++iter) {
      if (tag == "") {
         const hddm_s::TaggerHitList &hits = iter->getTaggerHits();
         hddm_s::TaggerHitList::iterator hiter;
         for (hiter = hits.begin(); hiter != hits.end(); ++hiter) {
            DTAGHHit *taghit = new DTAGHHit();
            taghit->E = hiter->getE();
            taghit->t = hiter->getT();
            taghit->npe_fadc = hiter->getNpe();
            taghit->time_fadc = hiter->getTADC();
            taghit->counter_id = hiter->getCounterId();
	    taghit->has_fADC = true;
	    taghit->has_TDC  = true;
            data.push_back(taghit);
         }
      }
      else if (tag == "TRUTH") {
         const hddm_s::TaggerTruthHitList &hits = iter->getTaggerTruthHits();
         hddm_s::TaggerTruthHitList::iterator hiter;
         for (hiter = hits.begin(); hiter != hits.end(); ++hiter) {
            DTAGHHit *taghit = new DTAGHHit();
            taghit->E = hiter->getE();
            taghit->t = hiter->getT();
            taghit->npe_fadc = hiter->getDE() * 5e5; // ~5e5 pe/GeV
            taghit->time_fadc = hiter->getT();
            taghit->counter_id = hiter->getCounterId();
	    taghit->has_fADC = true;
	    taghit->has_TDC  = true;
	    taghit->bg = hiter->getBg();

            data.push_back(taghit);
         }
      }
   }

   // Copy into factory
   factory->CopyTo(data);

   return NOERROR;
}

//------------------
// Extract_DPSHit
//------------------
jerror_t DEventSourceHDDM::Extract_DPSHit(hddm_s::HDDM *record,
                                   JFactory<DPSHit>* factory, string tag)
{
   /// Copies the data from the given hddm_s structure. This is called
   /// from JEventSourceHDDM::GetObjects. If factory is NULL, this
   /// returns OBJECT_NOT_AVAILABLE immediately.
   
   if (factory == NULL)
      return OBJECT_NOT_AVAILABLE;
   if (tag != "" && tag != "TRUTH")
      return OBJECT_NOT_AVAILABLE;

   vector<DPSHit*> data;
  
   if (tag == "") {
      const hddm_s::PsHitList &hits = record->getPsHits();
      hddm_s::PsHitList::iterator iter;
      for (iter = hits.begin(); iter != hits.end(); ++iter) {
         DPSHit *hit = new DPSHit;
         if(iter->getArm() == 0)
             hit->arm = DPSGeometry::Arm::kNorth;
         else 
             hit->arm = DPSGeometry::Arm::kSouth;
         hit->column = iter->getColumn();
	 double npix_fadc = iter->getDE()*0.5e5; // 100 pixels in 2 MeV
         hit->npix_fadc   = npix_fadc;
         hit->t = iter->getT();
	 
	 hit->E = 0.5*(psGeom->getElow(hit->arm,hit->column) + psGeom->getEhigh(hit->arm,hit->column));
	 hit->pulse_peak = npix_fadc*21;       // 1 pixel 21 fadc counts
	 hit->integral   = npix_fadc*21*5.1;   // integral/peak = 5.1  
         data.push_back(hit);
      }
   }
   else if (tag == "TRUTH") {
      const hddm_s::PsTruthHitList &hits = record->getPsTruthHits();
      hddm_s::PsTruthHitList::iterator iter;
      for (iter = hits.begin(); iter != hits.end(); ++iter) {
         DPSHit *hit = new DPSHit;
         if(iter->getArm() == 0)
             hit->arm = DPSGeometry::Arm::kNorth;
         else 
             hit->arm = DPSGeometry::Arm::kSouth;
         hit->column = iter->getColumn();
         hit->npix_fadc = iter->getDE() * 1e5; // ~1e5 pixels/GeV
         hit->t = iter->getT();
         data.push_back(hit);
      }
   }

  // Copy into factory
  factory->CopyTo(data);

  return NOERROR;
}

//------------------
// Extract_DPSTruthHit
//------------------
jerror_t DEventSourceHDDM::Extract_DPSTruthHit(hddm_s::HDDM *record,
                                   JFactory<DPSTruthHit>* factory, string tag)
{
   /// Copies the data from the given hddm_s structure. This is called
   /// from JEventSourceHDDM::GetObjects. If factory is NULL, this
   /// returns OBJECT_NOT_AVAILABLE immediately.

   if (factory == NULL)
      return OBJECT_NOT_AVAILABLE;
   if (tag != "")
      return OBJECT_NOT_AVAILABLE;

   vector<DPSTruthHit*> data;

   const hddm_s::PsTruthPointList &points = record->getPsTruthPoints();
   hddm_s::PsTruthPointList::iterator iter;
   for (iter = points.begin(); iter != points.end(); ++iter) {
      DPSTruthHit *hit = new DPSTruthHit;
      hit->dEdx    = iter->getDEdx();
      hit->primary = iter->getPrimary();
      hit->ptype   = iter->getPtype();
      hit->t       = iter->getT();
      hit->x       = iter->getX();
      hit->y       = iter->getY();
      hit->z       = iter->getZ();
      hit->track   = iter->getTrack();
      hit->column  = iter->getColumn();
      const hddm_s::TrackIDList &ids = iter->getTrackIDs();
      hit->itrack = (ids.size())? ids.begin()->getItrack() : 0;
      data.push_back(hit);
   }

   // Copy into factory
   factory->CopyTo(data);

   return NOERROR;
}

//------------------
// Extract_DPSCHit
//------------------
jerror_t DEventSourceHDDM::Extract_DPSCHit(hddm_s::HDDM *record,
                                   JFactory<DPSCHit>* factory, string tag)
{
   /// Copies the data from the given hddm_s structure. This is called
   /// from JEventSourceHDDM::GetObjects. If factory is NULL, this
   /// returns OBJECT_NOT_AVAILABLE immediately.
   
   if (factory == NULL)
      return OBJECT_NOT_AVAILABLE;
   if (tag != "" && tag != "TRUTH")
      return OBJECT_NOT_AVAILABLE;

   vector<DPSCHit*> data;
  
   if (tag == "") {
      const hddm_s::PscHitList &hits = record->getPscHits();
      hddm_s::PscHitList::iterator iter;
      for (iter = hits.begin(); iter != hits.end(); ++iter) {
         DPSCHit *hit = new DPSCHit;
         if(iter->getArm() == 0)
             hit->arm = DPSGeometry::Arm::kNorth;
         else 
             hit->arm = DPSGeometry::Arm::kSouth;
         hit->module = iter->getModule();

	 double npe_fadc = iter->getDE()*2.5e5;
	 hit->npe_fadc   = npe_fadc;
	 hit->pulse_peak = npe_fadc*0.4;       //  1000 pe - 400 fadc count
	 hit->integral   = npe_fadc*0.4*3;     // integral/peak = 3.

         hit->t = iter->getT();
	 hit->time_tdc  = iter->getT();
	 hit->time_fadc = iter->getT();

	 hit->has_fADC = true;
	 hit->has_TDC  = true;

         data.push_back(hit);
      }
   }
   else if (tag == "TRUTH") {
      const hddm_s::PscTruthHitList &hits = record->getPscTruthHits();
      hddm_s::PscTruthHitList::iterator iter;
      for (iter = hits.begin(); iter != hits.end(); ++iter) {
         DPSCHit *hit = new DPSCHit;
         if(iter->getArm() == 0)
             hit->arm = DPSGeometry::Arm::kNorth;
         else 
             hit->arm = DPSGeometry::Arm::kSouth;
         hit->module = iter->getModule();
         hit->npe_fadc = iter->getDE() * 5e5; // ~5e5 pe/GeV
         hit->t = iter->getT();
         data.push_back(hit);
      }
   }

  // Copy into factory
  factory->CopyTo(data);

  return NOERROR;
}

//------------------
// Extract_DPSCTruthHit
//------------------
jerror_t DEventSourceHDDM::Extract_DPSCTruthHit(hddm_s::HDDM *record,
                                   JFactory<DPSCTruthHit>* factory, string tag)
{
   /// Copies the data from the given hddm_s structure. This is called
   /// from JEventSourceHDDM::GetObjects. If factory is NULL, this
   /// returns OBJECT_NOT_AVAILABLE immediately.

   if (factory == NULL)
      return OBJECT_NOT_AVAILABLE;
   if (tag != "")
      return OBJECT_NOT_AVAILABLE;

   vector<DPSCTruthHit*> data;

   const hddm_s::PscTruthPointList &points = record->getPscTruthPoints();
   hddm_s::PscTruthPointList::iterator iter;
   for (iter = points.begin(); iter != points.end(); ++iter) {
      DPSCTruthHit *hit = new DPSCTruthHit;
      hit->dEdx    = iter->getDEdx();
      hit->primary = iter->getPrimary();
      hit->ptype   = iter->getPtype();
      hit->t       = iter->getT();
      hit->x       = iter->getX();
      hit->y       = iter->getY();
      hit->z       = iter->getZ();
      hit->track   = iter->getTrack();
      hit->column  = iter->getModule();
      const hddm_s::TrackIDList &ids = iter->getTrackIDs();
      hit->itrack = (ids.size())? ids.begin()->getItrack() : 0;
      data.push_back(hit);
   }

   // Copy into factory
   factory->CopyTo(data);

   return NOERROR;
}

//------------------
// Etract_DTPOLHit
//------------------
jerror_t DEventSourceHDDM::Extract_DTPOLHit(hddm_s::HDDM *record,
                                   JFactory<DTPOLHit>* factory, string tag)
{
   if (factory == NULL)
      return OBJECT_NOT_AVAILABLE;
   if (tag != "")
      return OBJECT_NOT_AVAILABLE;

   vector<DTPOLHit*> data;

   if (tag == "")
   {
      const hddm_s::TpolHitList &hits = record->getTpolHits();
      hddm_s::TpolHitList::iterator iter;
      for (iter = hits.begin(); iter != hits.end(); ++iter)
      {
         DTPOLHit *hit = new DTPOLHit;
         hit->sector = iter->getSector();
         hit->ring = iter->getRing();
         hit->dE = iter->getDE();
         hit->t = iter->getT();

         data.push_back(hit);
      }
   }
   else if (tag == "Truth")
   {
      const hddm_s::TpolTruthHitList &truthHits = record->getTpolTruthHits();
      hddm_s::TpolTruthHitList::iterator iter;
      for (iter = truthHits.begin(); iter != truthHits.end(); ++iter)
      {
         DTPOLHit *hit = new DTPOLHit;
         hit->sector = iter->getSector();
         hit->t = iter->getT();

         data.push_back(hit);
      }
   }

   factory->CopyTo(data);

   return NOERROR;
}

//------------------------
// Extract_DTPOLTruthHit
//------------------------
jerror_t DEventSourceHDDM::Extract_DTPOLTruthHit(hddm_s::HDDM *record,                                                                      JFactory<DTPOLTruthHit>* factory, string tag)
{
   if (factory == NULL)
      return OBJECT_NOT_AVAILABLE;
   if (tag != "")
      return OBJECT_NOT_AVAILABLE;

   vector<DTPOLTruthHit*> data;

   const hddm_s::TpolTruthPointList &points = record->getTpolTruthPoints();
   hddm_s::TpolTruthPointList::iterator iter;
   for (iter = points.begin(); iter != points.end(); ++iter)
   {
      DTPOLTruthHit *hit = new DTPOLTruthHit;
      hit->dEdx = iter->getDEdx();
      hit->primary = iter->getPrimary();
      hit->track = iter->getTrack();
      hit->ptype = iter->getPtype();
      hit->r = iter->getR();
      hit->phi = iter->getPhi();
      hit->t = iter->getT();
      const hddm_s::TrackIDList &ids = iter->getTrackIDs();
      hit->itrack = (ids.size())? ids.begin()->getItrack() : 0;

      data.push_back(hit);
   }

   factory->CopyTo(data);

   return NOERROR;
}

Particle_t DEventSourceHDDM::IDTrack(float locCharge, float locMass) const
{
   float locMassTolerance = 0.010;
   if (locCharge > 0.1) // Positive particles
   {
      if (fabs(locMass - ParticleMass(Proton)) < locMassTolerance) return Proton;
      if (fabs(locMass - ParticleMass(PiPlus)) < locMassTolerance) return PiPlus;
      if (fabs(locMass - ParticleMass(KPlus)) < locMassTolerance) return KPlus;
      if (fabs(locMass - ParticleMass(Positron)) < locMassTolerance) return Positron;
      if (fabs(locMass - ParticleMass(MuonPlus)) < locMassTolerance) return MuonPlus;
   }
   else if(locCharge < -0.1) // Negative particles
   {
      if (fabs(locMass - ParticleMass(PiMinus)) < locMassTolerance) return PiMinus;
      if (fabs(locMass - ParticleMass(KMinus)) < locMassTolerance) return KMinus;
      if (fabs(locMass - ParticleMass(MuonMinus)) < locMassTolerance) return MuonMinus;
      if (fabs(locMass - ParticleMass(Electron)) < locMassTolerance) return Electron;
      if (fabs(locMass - ParticleMass(AntiProton)) < locMassTolerance) return AntiProton;
   }
   else //Neutral Track
   {
      if (fabs(locMass - ParticleMass(Gamma)) < locMassTolerance) return Gamma;
      if (fabs(locMass - ParticleMass(Neutron)) < locMassTolerance) return Neutron;
   }
   return Unknown;
}

//------------------
// Extract_DFMWPCTruthHit
//------------------
jerror_t DEventSourceHDDM::Extract_DFMWPCTruthHit(hddm_s::HDDM *record,  JFactory<DFMWPCTruthHit> *factory, string tag)
{
   /// Copies the data from the given hddm_s record. This is called
   /// from JEventSourceHDDM::GetObjects. If factory is NULL, this
   /// returns OBJECT_NOT_AVAILABLE immediately.

   if (factory == NULL) return OBJECT_NOT_AVAILABLE;
   if (tag != "") return OBJECT_NOT_AVAILABLE;

   vector<DFMWPCTruthHit*> data;

   const hddm_s::FmwpcTruthHitList &points = record->getFmwpcTruthHits();
   hddm_s::FmwpcTruthHitList::iterator iter;
   for (iter = points.begin(); iter != points.end(); ++iter) {
      DFMWPCTruthHit *hit = new DFMWPCTruthHit;
      hit->layer = iter->getLayer();
      hit->wire  = iter->getWire();
      hit->q     = iter->getQ();
      hit->dE    = iter->getDE();
      hit->dx    = iter->getDx();
      hit->t     = iter->getT();
      data.push_back(hit);
   }

   // Copy into factory
   factory->CopyTo(data);

   return NOERROR;
}


//------------------
// Extract_DFMWPCTruth
//------------------
jerror_t DEventSourceHDDM::Extract_DFMWPCTruth(hddm_s::HDDM *record,
                                   JFactory<DFMWPCTruth>* factory, string tag)
{
   /// Copies the data from the given hddm_s structure. This is called
   /// from JEventSourceHDDM::GetObjects. If factory is NULL, this
   /// returns OBJECT_NOT_AVAILABLE immediately.

   if (factory == NULL)
      return OBJECT_NOT_AVAILABLE;
   if (tag != "")
      return OBJECT_NOT_AVAILABLE;
  
   vector<DFMWPCTruth*> data;

   const hddm_s::FmwpcTruthPointList &points = record->getFmwpcTruthPoints();
   hddm_s::FmwpcTruthPointList::iterator iter;
   for (iter = points.begin(); iter != points.end(); ++iter) {
      DFMWPCTruth *fmwpctruth = new DFMWPCTruth;
      fmwpctruth->primary = iter->getPrimary();
      fmwpctruth->track   = iter->getTrack();
      fmwpctruth->x       = iter->getX();
      fmwpctruth->y       = iter->getY();
      fmwpctruth->z       = iter->getZ();
      fmwpctruth->t       = iter->getT();
      fmwpctruth->px      = iter->getPx();
      fmwpctruth->py      = iter->getPy();
      fmwpctruth->pz      = iter->getPz();
      fmwpctruth->E       = iter->getE();
      fmwpctruth->ptype   = iter->getPtype();
      const hddm_s::TrackIDList &ids = iter->getTrackIDs();
      fmwpctruth->itrack = (ids.size())? ids.begin()->getItrack() : 0;
      data.push_back(fmwpctruth);
   }

   // Copy into factory
   factory->CopyTo(data);

   return NOERROR;
}

//------------------
// Extract_DFMWPCHit
//------------------
jerror_t DEventSourceHDDM::Extract_DFMWPCHit(hddm_s::HDDM *record,  JFactory<DFMWPCHit> *factory, string tag)
{
   /// Copies the data from the given hddm_s record. This is called
   /// from JEventSourceHDDM::GetObjects. If factory is NULL, this
   /// returns OBJECT_NOT_AVAILABLE immediately.

   if (factory == NULL) return OBJECT_NOT_AVAILABLE;
   if (tag != "") return OBJECT_NOT_AVAILABLE;

   vector<DFMWPCHit*> data;

   const hddm_s::FmwpcHitList &points = record->getFmwpcHits();
   hddm_s::FmwpcHitList::iterator iter;
   for (iter = points.begin(); iter != points.end(); ++iter) {
      DFMWPCHit *hit = new DFMWPCHit;
      hit->layer = iter->getLayer();
      hit->wire  = iter->getWire();
      hit->q     = iter->getQ();
      hit->amp   = iter->getAmp(); 
      hit->t     = iter->getT();
      data.push_back(hit);
   }

   // Copy into factory
   factory->CopyTo(data);

   return NOERROR;
}


//------------------
// Extract_DCTOFTruth
//------------------
jerror_t DEventSourceHDDM::Extract_DCTOFTruth(hddm_s::HDDM *record,
                                   JFactory<DCTOFTruth>* factory, string tag)
{
   /// Copies the data from the given hddm_s structure. This is called
   /// from JEventSourceHDDM::GetObjects. If factory is NULL, this
   /// returns OBJECT_NOT_AVAILABLE immediately.

   if (factory == NULL)
      return OBJECT_NOT_AVAILABLE;
   if (tag != "")
      return OBJECT_NOT_AVAILABLE;
  
   vector<DCTOFTruth*> data;

   const hddm_s::CtofTruthPointList &points = record->getCtofTruthPoints();
   hddm_s::CtofTruthPointList::iterator iter;
   for (iter = points.begin(); iter != points.end(); ++iter) {
      DCTOFTruth *ctoftruth = new DCTOFTruth;
      ctoftruth->primary = iter->getPrimary();
      ctoftruth->track   = iter->getTrack();
      ctoftruth->x       = iter->getX();
      ctoftruth->y       = iter->getY();
      ctoftruth->z       = iter->getZ();
      ctoftruth->t       = iter->getT();
      ctoftruth->px      = iter->getPx();
      ctoftruth->py      = iter->getPy();
      ctoftruth->pz      = iter->getPz();
      ctoftruth->E       = iter->getE();
      ctoftruth->ptype   = iter->getPtype();
      const hddm_s::TrackIDList &ids = iter->getTrackIDs();
      ctoftruth->itrack = (ids.size())? ids.begin()->getItrack() : 0;
      data.push_back(ctoftruth);
   }

   // Copy into factory
   factory->CopyTo(data);

   return NOERROR;
}


//------------------
// Extract_DCTOFHit
//------------------
jerror_t DEventSourceHDDM::Extract_DCTOFHit(hddm_s::HDDM *record,  JFactory<DCTOFHit> *factory, string tag)
{
   /// Copies the data from the given hddm_s record. This is called
   /// from JEventSourceHDDM::GetObjects. If factory is NULL, this
   /// returns OBJECT_NOT_AVAILABLE immediately.

   if (factory == NULL) return OBJECT_NOT_AVAILABLE;
   if (tag != "") return OBJECT_NOT_AVAILABLE;

   vector<DCTOFHit*> data;

   const hddm_s::CtofHitList &points = record->getCtofHits();
   hddm_s::CtofHitList::iterator iter;
   for (iter = points.begin(); iter != points.end(); ++iter) {
      DCTOFHit *hit = new DCTOFHit;
      hit->bar = iter->getBar();
      hit->end  = iter->getEnd();
      hit->dE    = iter->getDE();
      hit->t     = iter->getT();
      data.push_back(hit);
   }

   // Copy into factory
   factory->CopyTo(data);

   return NOERROR;
}

//------------------
// Extract_DDIRCPmtHit
//------------------
jerror_t DEventSourceHDDM::Extract_DDIRCPmtHit(hddm_s::HDDM *record,
                                   JFactory<DDIRCPmtHit> *factory, string tag,
                                   JEventLoop* eventLoop)
{
  /// Copies the data from the given hddm_s structure. This is called
  /// from JEventSourceHDDM::GetObjects. If factory is NULL, this
  /// returs OBJECT_NOT_AVAILABLE immediately.
   
  if (factory == NULL)
     return OBJECT_NOT_AVAILABLE;
  if (tag != "")
     return OBJECT_NOT_AVAILABLE;

  vector<DDIRCPmtHit*> data;

  if (tag == "") {
     vector<const DDIRCTruthPmtHit*> locDIRCTruthPmtHit;
     eventLoop->Get(locDIRCTruthPmtHit);

     const hddm_s::DircPmtHitList &hits = record->getDircPmtHits();
     hddm_s::DircPmtHitList::iterator iter;
     for (iter = hits.begin(); iter != hits.end(); ++iter) {
         double time = iter->getT();
         int channel = iter->getCh();

         DDIRCPmtHit *hit = new DDIRCPmtHit();
         hit->t  = time;
         hit->ch = channel;

	 for (auto& iterTruth : locDIRCTruthPmtHit) { //.begin(); iterTruth != locDIRCTruthPmtHit.end(); ++iterTruth) {
		 
		 // must match channel and time
		 if(channel == iterTruth->ch && fabs(time-iterTruth->t) < 5.0) {
			 
			 hit->AddAssociatedObject(iterTruth);

			 break;
		 }
	 }
	 
         data.push_back(hit);
     }
  }
  
  // Copy into factory
  factory->CopyTo(data);
  
  return NOERROR;
}

//------------------
// Extract_DCereHit
// added by yqiang Oct 11, 2012
//------------------
jerror_t DEventSourceHDDM::Extract_DCereHit(hddm_s::HDDM *record,
                                   JFactory<DCereHit>* factory, string tag)
{
   if (factory == NULL)
      return OBJECT_NOT_AVAILABLE;
   if (tag != "" && tag != "TRUTH")
      return OBJECT_NOT_AVAILABLE;

   vector<DCereHit*> data;

   if (tag == "") {
      const hddm_s::CereHitList &hits = record->getCereHits();
      hddm_s::CereHitList::iterator iter;
      for (iter = hits.begin(); iter != hits.end(); ++iter) {
         DCereHit *hit = new DCereHit;
         hit->sector = iter->getSector();
         hit->pe = iter->getPe();
         hit->t = iter->getT();
         data.push_back(hit);
      }
   }
   else if (tag == "TRUTH") {
      const hddm_s::CereTruthHitList &hits = record->getCereTruthHits();
      hddm_s::CereTruthHitList::iterator iter;
      for (iter = hits.begin(); iter != hits.end(); ++iter) {
         DCereHit *hit = new DCereHit;
         hit->sector = iter->getSector();
         hit->pe = iter->getPe();
         hit->t = iter->getT();
         data.push_back(hit);
      }
   }

   // copy into factory
   factory->CopyTo(data);

   return NOERROR;
}

//------------------
// Extract_DDIRCTruthBarHit
//------------------
jerror_t DEventSourceHDDM::Extract_DDIRCTruthBarHit(hddm_s::HDDM *record,
                                   JFactory<DDIRCTruthBarHit>* factory, string tag)
{
   /// Copies the data from the given hddm_s structure. This is called
   /// from JEventSourceHDDM::GetObjects. If factory is NULL, this
   /// returns OBJECT_NOT_AVAILABLE immediately.

   if (factory == NULL)
      return OBJECT_NOT_AVAILABLE;
   if (tag != "")
      return OBJECT_NOT_AVAILABLE;

   vector<DDIRCTruthBarHit*> data;

   const hddm_s::DircTruthBarHitList &hits = record->getDircTruthBarHits();
   hddm_s::DircTruthBarHitList::iterator iter;
   for (iter = hits.begin(); iter != hits.end(); ++iter) {
      DDIRCTruthBarHit *hit = new DDIRCTruthBarHit;
      hit->x = iter->getX();
      hit->y = iter->getY();
      hit->z = iter->getZ();
      hit->px = iter->getPx();
      hit->py = iter->getPy();
      hit->pz = iter->getPz();
      hit->t = iter->getT();
      hit->E = iter->getE();
      hit->pdg = iter->getPdg();
      hit->bar = iter->getBar();
      hit->track = iter->getTrack();
      data.push_back(hit);
   }

  // Copy into factory
  factory->CopyTo(data);

  return NOERROR;
}

//------------------
// Extract_DDIRCTruthPmtHit
//------------------
jerror_t DEventSourceHDDM::Extract_DDIRCTruthPmtHit(hddm_s::HDDM *record,
                                   JFactory<DDIRCTruthPmtHit>* factory, string tag)
{
   /// Copies the data from the given hddm_s structure. This is called
   /// from JEventSourceHDDM::GetObjects. If factory is NULL, this
   /// returns OBJECT_NOT_AVAILABLE immediately.

   if (factory == NULL)
      return OBJECT_NOT_AVAILABLE;
   if (tag != "")
      return OBJECT_NOT_AVAILABLE;

   vector<DDIRCTruthPmtHit*> data;
   

   const hddm_s::DircTruthPmtHitList &hits = record->getDircTruthPmtHits();
   hddm_s::DircTruthPmtHitList::iterator iter;
   for (iter = hits.begin(); iter != hits.end(); ++iter) {
      DDIRCTruthPmtHit *hit = new DDIRCTruthPmtHit;
      hit->x = iter->getX();
      hit->y = iter->getY();
      hit->z = iter->getZ();
      hit->t = iter->getT();
      hit->E = iter->getE();
      hit->ch = iter->getCh();
      hit->key_bar = iter->getKey_bar();
      hddm_s::DircTruthPmtHitExtraList &hitextras = iter->getDircTruthPmtHitExtras();
      if(hitextras.size() > 0) {
      	hit->t_fixed = hitextras(0).getT_fixed();
      	hit->path = hitextras(0).getPath();
      	hit->refl = hitextras(0).getRefl();
      	hit->bbrefl = hitextras(0).getBbrefl();
      }
      data.push_back(hit);
   }

  // Copy into factory
  factory->CopyTo(data);

  return NOERROR;
}

//------------------
// Extract_DGEMTRDTruthHit
//------------------
jerror_t DEventSourceHDDM::Extract_DGEMTRDTruthHit(hddm_s::HDDM *record,  JFactory<DGEMTRDTruthHit> *factory, string tag)
{
   /// Copies the data from the given hddm_s record. This is called
   /// from JEventSourceHDDM::GetObjects. If factory is NULL, this
   /// returns OBJECT_NOT_AVAILABLE immediately.

   if (factory == NULL) return OBJECT_NOT_AVAILABLE;
   if (tag != "") return OBJECT_NOT_AVAILABLE;

   vector<DGEMTRDTruthHit*> data;

   const hddm_s::GemtrdTruthHitList &points = record->getGemtrdTruthHits();
   hddm_s::GemtrdTruthHitList::iterator iter;
   for (iter = points.begin(); iter != points.end(); ++iter) {
      DGEMTRDTruthHit *hit = new DGEMTRDTruthHit;
      hit->layer = iter->getLayer();
      hit->q     = iter->getQ();
      hit->y    = iter->getY();
      hit->x    = iter->getX();
      hit->t     = iter->getT();
      hit->zdrift = iter->getZdrift();
      data.push_back(hit);
   }

   // Copy into factory
   factory->CopyTo(data);

   return NOERROR;
}


//------------------
// Extract_DGEMTRDTruth
//------------------
jerror_t DEventSourceHDDM::Extract_DGEMTRDTruth(hddm_s::HDDM *record,
                                   JFactory<DGEMTRDTruth>* factory, string tag)
{
   /// Copies the data from the given hddm_s structure. This is called
   /// from JEventSourceHDDM::GetObjects. If factory is NULL, this
   /// returns OBJECT_NOT_AVAILABLE immediately.

   if (factory == NULL)
      return OBJECT_NOT_AVAILABLE;
   if (tag != "")
      return OBJECT_NOT_AVAILABLE;
  
   vector<DGEMTRDTruth*> data;

   const hddm_s::GemtrdTruthPointList &points = record->getGemtrdTruthPoints();
   hddm_s::GemtrdTruthPointList::iterator iter;
   for (iter = points.begin(); iter != points.end(); ++iter) {
      DGEMTRDTruth *gemtrdtruth = new DGEMTRDTruth;
      gemtrdtruth->primary = iter->getPrimary();
      gemtrdtruth->track   = iter->getTrack();
      gemtrdtruth->x       = iter->getX();
      gemtrdtruth->y       = iter->getY();
      gemtrdtruth->z       = iter->getZ();
      gemtrdtruth->t       = iter->getT();
      gemtrdtruth->px      = iter->getPx();
      gemtrdtruth->py      = iter->getPy();
      gemtrdtruth->pz      = iter->getPz();
      gemtrdtruth->E       = iter->getE();
      gemtrdtruth->ptype   = iter->getPtype();
      const hddm_s::TrackIDList &ids = iter->getTrackIDs();
      gemtrdtruth->itrack = (ids.size())? ids.begin()->getItrack() : 0;
      data.push_back(gemtrdtruth);
   }

   // Copy into factory
   factory->CopyTo(data);

   return NOERROR;
}

//------------------
// Extract_DGEMTRDHit
//------------------
jerror_t DEventSourceHDDM::Extract_DGEMTRDHit(hddm_s::HDDM *record,  JFactory<DGEMTRDHit> *factory, string tag)
{
   /// Copies the data from the given hddm_s record. This is called
   /// from JEventSourceHDDM::GetObjects. If factory is NULL, this
   /// returns OBJECT_NOT_AVAILABLE immediately.

   if (factory == NULL) return OBJECT_NOT_AVAILABLE;
   if (tag != "") return OBJECT_NOT_AVAILABLE;

   vector<DGEMTRDHit*> data;

   const hddm_s::GemtrdHitList &points = record->getGemtrdHits();
   hddm_s::GemtrdHitList::iterator iter;
   for (iter = points.begin(); iter != points.end(); ++iter) {
      DGEMTRDHit *hit = new DGEMTRDHit;
      hit->layer = iter->getLayer();
      hit->q     = iter->getQ();
      hit->t     = iter->getT();
      hit->x     = iter->getX();
      hit->y     = iter->getY();
      data.push_back(hit);
   }

   // Copy into factory
   factory->CopyTo(data);

   return NOERROR;
}

