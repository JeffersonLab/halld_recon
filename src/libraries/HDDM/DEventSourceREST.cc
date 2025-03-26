//
// Author: Richard Jones  June 29, 2012
//
//
// DEventSourceREST methods
//

#include <iostream>
#include <iomanip>
#include <fstream>
#include <climits>

#include <JANA/JFactory.h>
#include <JANA/JEvent.h>
#include <DANA/DStatusBits.h>
#include <DANA/DEvent.h>

#include <TAGGER/DTAGHHit_factory_Calib.h>
#include <TAGGER/DTAGMHit_factory_Calib.h>

#include <DVector2.h>
#include "DEventSourceREST.h"

//----------------
// Constructor
//----------------
DEventSourceREST::DEventSourceREST(std::string source_name, JApplication* app)
 : JEventSource(source_name)
{
   /// Constructor for DEventSourceREST object
   EnableGetObjects(true);  // Check the source first for existing objects; only invoke the factory to create them if they aren't found in the source.
	EnableFinishEvent(true); // Ensure ::FinishEvent gets called. By default, it is disabled (false).
   SetTypeName("DEventSourceREST");
   ifs = new ifstream(source_name);
   ifs->get();
   ifs->unget();
   if (ifs->rdbuf()->in_avail() > 30) {
      class nonstd_streambuf: public std::streambuf {
       public: char *pub_gptr() {return gptr();}
      };
      void *buf = (void*)ifs->rdbuf();
      std::stringstream sbuf(((nonstd_streambuf*)buf)->pub_gptr());
      std::string head;
      std::getline(sbuf, head);
      std::string expected = " class=\"r\" ";
      if (head.find(expected) == head.npos) {
         std::string msg("Unexpected header found in input REST stream: ");
         throw std::runtime_error(msg + head + source_name);
      }
   }

   fin = new hddm_r::istream(*ifs);
   
   PRUNE_DUPLICATE_TRACKS = true;
   app->SetDefaultParameter("REST:PRUNE_DUPLICATE_TRACKS", PRUNE_DUPLICATE_TRACKS,
   								"Turn on/off cleaning up multiple tracks with the same hypothesis from the same candidate. Set to \"0\" to turn off (it's on by default)");

   RECO_DIRC_CALC_LUT = true;
   app->SetDefaultParameter("REST:DIRC_CALC_LUT", RECO_DIRC_CALC_LUT, "Turn on/off DIRC LUT reconstruction");

   dDIRCMaxChannels = 108*64;

   // any other initialization which needs to happen
   dBCALShowerFactory = nullptr;
   dFCALShowerFactory = nullptr;

   USE_CCDB_BCAL_COVARIANCE = false;
   app->SetDefaultParameter("REST:USE_CCDB_BCAL_COVARIANCE", USE_CCDB_BCAL_COVARIANCE,
   		"Load REST BCAL Shower covariance matrices from CCDB instead of the file.");
   USE_CCDB_FCAL_COVARIANCE = false;
   app->SetDefaultParameter("REST:USE_CCDB_FCAL_COVARIANCE", USE_CCDB_FCAL_COVARIANCE,
   		"Load REST BFAL Shower covariance matrices from CCDB instead of the file.");
   		
   app->SetDefaultParameter("REST:JANACALIBCONTEXT", REST_JANA_CALIB_CONTEXT);
   calib_generator = new JCalibrationGeneratorCCDB;  // keep this around in case we need to use it
}

//----------------
// Destructor
//----------------
DEventSourceREST::~DEventSourceREST()
{  
  if (fin) {
    delete fin;
  }
  if (ifs) {
    delete ifs;
  }
  
  for(auto &entry : dJCalib_olds)
  	delete entry.second;
  for(auto &entry : dTAGHGeoms)
  	delete entry.second;
  for(auto &entry : dTAGMGeoms)
  	delete entry.second;
}

//----------------
// GetEvent
//----------------
void DEventSourceREST::GetEvent(std::shared_ptr<JEvent> event)
{
   /// Implementation of JEventSource virtual function
   auto app = event->GetJApplication();
   if (!fin) {
      throw RETURN_STATUS::kUNKNOWN; // EVENT_SOURCE_NOT_OPEN
   }

   // Each open hddm file takes up about 1M of memory so it's
   // worthwhile to close it as soon as we can.
   if (ifs->eof()) {
      delete fin;
      fin = NULL;
      delete ifs;
      ifs = NULL;

      throw RETURN_STATUS::kNO_MORE_EVENTS;
   }

#if HDDM_SETPOSITION_EXAMPLE
   static std::ifstream fevlist("events.list");
   static int events_to_go = 0;
   if (events_to_go-- == 0 && fevlist.good()) {
      uint64_t start;
      uint32_t offset, status;
      fevlist >> start >> offset >> status >> events_to_go;
      if (fevlist.good())
         fin->setPosition(hddm_r::streamposition(start, offset, status));
   }
#endif

#if HDDM_GETPOSITION_EXAMPLE
   hddm_r::streamposition pos(fin->getPosition());
   // Later on below, if this event passes all of your selection cuts
   // then you might want to write the event position to output, as in
   // std::cout << "interesting event found at " 
   //           << pos.start << "," << pos.offset << "," << pos.status
   //           << std::endl;
#endif

   hddm_r::HDDM *record = new hddm_r::HDDM();
   hddm_r::threads::ID = 1;
   try{
      while (record->getReconstructedPhysicsEvents().size() == 0) {
         if (! (*fin >> *record)) {
            delete fin;
            fin = NULL;
            delete ifs;
            ifs = NULL;
	        throw RETURN_STATUS::kNO_MORE_EVENTS;
         }
      }
   }catch(std::runtime_error &e){
      cerr << "Exception caught while trying to read REST file!" << endl;
	  cerr << e.what() << endl;
	  _DBG__;
	   throw RETURN_STATUS::kNO_MORE_EVENTS;
   }

   // Copy the reference info into the JEvent object
   while (true) {
      hddm_r::ReconstructedPhysicsEvent &re
            = record->getReconstructedPhysicsEvent();
      int runno = re.getRunNo();
      int eventno = re.getEventNo();
      if (runno == 0 && eventno == 0) {
         // found a comment record, print comment strings and continue
         const hddm_r::CommentList &comments = re.getComments();
         hddm_r::CommentList::iterator iter;
         for (iter = comments.begin(); iter != comments.end(); ++iter) {
            std::cout << "   | " << iter->getText() << std::endl;
         }

         //set version string
         const hddm_r::DataVersionStringList& locVersionStrings = re.getDataVersionStrings();
         hddm_r::DataVersionStringList::iterator Versioniter;
         for (Versioniter = locVersionStrings.begin(); Versioniter != locVersionStrings.end(); ++Versioniter) {
        	 string HDDM_DATA_VERSION_STRING = Versioniter->getText();
             if(app->GetJParameterManager()->Exists("REST:DATAVERSIONSTRING"))
                app->GetJParameterManager()->SetParameter("REST:DATAVERSIONSTRING", HDDM_DATA_VERSION_STRING);
	     else
	 	   app->SetDefaultParameter("REST:DATAVERSIONSTRING", HDDM_DATA_VERSION_STRING);
	     break;
         }

         // set REST calib context - use this to load calibration constants that were used
         // to create the REST files, if needed, but let this be overridden by command-line options
         if( REST_JANA_CALIB_CONTEXT == "" ) {
			 const hddm_r::CcdbContextList& locContextStrings = re.getCcdbContexts();
			 hddm_r::CcdbContextList::iterator Contextiter;
			 for (Contextiter = locContextStrings.begin(); Contextiter != locContextStrings.end(); ++Contextiter) {
				 REST_JANA_CALIB_CONTEXT = Contextiter->getText();
				 jout << " REST file next CCDB context = " << REST_JANA_CALIB_CONTEXT << endl;  // DEBUG?
			 }
		 }

         record->clear();
         while (record->getReconstructedPhysicsEvents().size() == 0) {
            if (! (*fin >> *record)) {
               delete fin;
               fin = NULL;
               delete ifs;
               ifs = NULL;
	           throw RETURN_STATUS::kNO_MORE_EVENTS;
            }
         }

         continue;
      }
      event->SetEventNumber(re.getEventNo());
      event->SetRunNumber(re.getRunNo());
      event->SetJEventSource(this);
      event->Insert(record);

      auto statusBits = new DStatusBits;
      statusBits->SetStatusBit(kSTATUS_REST);
      statusBits->SetStatusBit(kSTATUS_FROM_FILE);
	  statusBits->SetStatusBit(kSTATUS_PHYSICS_EVENT);
	  event->Insert(statusBits);

	  // ++Nevents_read; // TODO: NWB: This is going away. Verify this is fine.
      break;
   }
}

//----------------
// FinishEvent
//----------------
void DEventSourceREST::FinishEvent(JEvent &event)
{
	// Deletion of REST record is handled by the JEvent. No additional cleanup needed.
}

//----------------
// GetObjects
//----------------
bool DEventSourceREST::GetObjects(const std::shared_ptr<const JEvent> &event, JFactory *factory)
{
   /// This gets called through the virtual method of the
   /// JEventSource base class. It creates the objects of the type
   /// on which factory is based.  It uses the HDDM_Element object
   /// kept in the ref field of the JEvent object passed.

   // We must have a factory to hold the data
   if (!factory) {
      throw RESOURCE_UNAVAILABLE;
   }

   hddm_r::HDDM *record = const_cast<hddm_r::HDDM*>(event->GetSingleStrict<hddm_r::HDDM>());
   hddm_r::threads::ID = 1;
   // TODO: NWB: Remove const cast if possible

   string dataClassName = factory->GetObjectName();
   JCalibration *jcalib = DEvent::GetJCalibration(event);

	//Get target center
	//multiple reader threads can access this object: need lock
	bool locNewRunNumber = false;
	unsigned int locRunNumber = event->GetRunNumber();
	{
		std::lock_guard<std::mutex> lock(readMutex);
		locNewRunNumber = (dTargetCenterZMap.find(locRunNumber) == dTargetCenterZMap.end());
	}
	if(locNewRunNumber)
	{
		DGeometry* locGeometry = DEvent::GetDGeometry(event);
		double locTargetCenterZ = 0.0;
		locGeometry->GetTargetZ(locTargetCenterZ);
		
		map<string, double> beam_vals;
		if (DEvent::GetCalib(event, "PHOTON_BEAM/beam_spot",beam_vals))
		  throw JException("Could not load CCDB table: PHOTON_BEAM/beam_spot");
	
		vector<double> locBeamPeriodVector;
		if(DEvent::GetCalib(event, "PHOTON_BEAM/RF/beam_period", locBeamPeriodVector))
			throw JException("Could not load CCDB table: PHOTON_BEAM/RF/beam_period");
		double locBeamBunchPeriod = locBeamPeriodVector[0];

		vector< vector <int> > locDIRCChannelStatus;
		vector<int> new_dirc_status(dDIRCMaxChannels);
		locDIRCChannelStatus.push_back(new_dirc_status); 
		locDIRCChannelStatus.push_back(new_dirc_status);
		if(RECO_DIRC_CALC_LUT) { // get DIRC channel status from DB
			if (DEvent::GetCalib(event, "/DIRC/North/channel_status", locDIRCChannelStatus[0]))
				jout << "Error loading /DIRC/North/channel_status !" << endl;
			if (DEvent::GetCalib(event, "/DIRC/South/channel_status", locDIRCChannelStatus[1]))
				jout << "Error loading /DIRC/South/channel_status !" << endl;
		}
		

		{
			std::lock_guard<std::mutex> lock(readMutex);
			dTargetCenterZMap[locRunNumber] = locTargetCenterZ;
			dBeamBunchPeriodMap[locRunNumber] = locBeamBunchPeriod;
			dDIRCChannelStatusMap[locRunNumber] = locDIRCChannelStatus;

			dBeamCenterMap[locRunNumber].Set(beam_vals["x"],
							 beam_vals["y"]);
			dBeamDirMap[locRunNumber].Set(beam_vals["dxdz"],
						      beam_vals["dydz"]);
			dBeamZ0Map[locRunNumber]=beam_vals["z"];
			
			jout << "Run " << locRunNumber << " beam spot:"
			     << " x=" << dBeamCenterMap[locRunNumber].X()
			     << " y=" << dBeamCenterMap[locRunNumber].Y()
			     << " z=" << dBeamZ0Map[locRunNumber]
			     << " dx/dz=" << dBeamDirMap[locRunNumber].X() 
			     << " dy/dz=" << dBeamDirMap[locRunNumber].Y() 
			     << endl;

			// load tagger quality tables
// 			dTAGHCounterQualities[locRunNumber] = new double[TAGH_MAX_COUNTER+1];
// 			dTAGMFiberQualities[locRunNumber] = new double*[TAGM_MAX_ROW+1];
// 			for(int i; i<TAGM_MAX_ROW+1; i++)
// 				dTAGMFiberQualities[locRunNumber][i] = new double[TAGM_MAX_COLUMN+1];

			if(!DTAGHHit_factory_Calib::load_ccdb_constants(jcalib, "counter_quality", "code", dTAGHCounterQualities[locRunNumber])) {
				jerr << "Error loading /PHOTON_BEAM/hodoscope/counter_quality in DEventSourceREST::GetObjects() ... " << endl;
			}
			if(!DTAGMHit_factory_Calib::load_ccdb_constants(jcalib, "fiber_quality", "code", dTAGMFiberQualities[locRunNumber])) {
				jerr << "Error loading /PHOTON_BEAM/microscope/fiber_quality in DEventSourceREST::GetObjects() ... " << endl;
			}

			// tagger related configs for reverse mapping tagger energy to counter number
			if( REST_JANA_CALIB_CONTEXT != "" ) {
				JCalibration *jcalib_old = calib_generator->MakeJCalibration(jcalib->GetURL(), locRunNumber, REST_JANA_CALIB_CONTEXT );
				dTAGHGeoms[locRunNumber] = new DTAGHGeometry(jcalib_old, locRunNumber);
				dTAGMGeoms[locRunNumber] = new DTAGMGeometry(jcalib_old, locRunNumber);
				dJCalib_olds[locRunNumber] = jcalib_old;
			}
		}

		// do multiple things to limit the number of locks
		// make sure that we have a handle to the FCAL shower factory
		if(USE_CCDB_FCAL_COVARIANCE) {
            if(dFCALShowerFactory==nullptr) {
                dFCALShowerFactory = static_cast<DFCALShower_factory*>(event->GetFactory("DFCALShower", ""));
                if(dFCALShowerFactory==nullptr)
                    throw JException("Couldn't find DFCALShower_factory???");
            }
			dFCALShowerFactory->LoadCovarianceLookupTables(event);
        }

        // same with BCAL
		if(USE_CCDB_BCAL_COVARIANCE) { 
            if(dBCALShowerFactory==nullptr) {
                dBCALShowerFactory = static_cast<DBCALShower_factory_IU*>(event->GetFactory("DBCALShower", "IU"));
                if(dBCALShowerFactory==nullptr)
                    throw JException("Couldn't find DBCALShower_factory???");
            }
			dBCALShowerFactory->LoadCovarianceLookupTables(event);
		} 
		
	}

   if (dataClassName =="DMCReaction") {
      return (Extract_DMCReaction(record,
                     dynamic_cast<JFactoryT<DMCReaction>*>(factory), event));
   }
   if (dataClassName =="DRFTime") {
      return (Extract_DRFTime(record,
                     dynamic_cast<JFactoryT<DRFTime>*>(factory), event));
   }
   if (dataClassName =="DBeamPhoton") {
      return (Extract_DBeamPhoton(record,
                     dynamic_cast<JFactoryT<DBeamPhoton>*>(factory), event));
   }
   if (dataClassName =="DMCThrown") {
      return (Extract_DMCThrown(record,
                     dynamic_cast<JFactoryT<DMCThrown>*>(factory)));
   }
   if (dataClassName =="DTOFPoint") {
      return (Extract_DTOFPoint(record,
                     dynamic_cast<JFactoryT<DTOFPoint>*>(factory)));
   }
   if (dataClassName =="DCTOFPoint") {
      return (Extract_DCTOFPoint(record,
                     dynamic_cast<JFactoryT<DCTOFPoint>*>(factory)));
   }
   if (dataClassName =="DSCHit") {
      return (Extract_DSCHit(record,
                     dynamic_cast<JFactoryT<DSCHit>*>(factory)));
   }
   if (dataClassName =="DFCALShower") {
      return (Extract_DFCALShower(record,
                     dynamic_cast<JFactoryT<DFCALShower>*>(factory)));
   }
   if (dataClassName =="DBCALShower") {
      return (Extract_DBCALShower(record,
                     dynamic_cast<JFactoryT<DBCALShower>*>(factory)));
   }
   if (dataClassName =="DCCALShower") {
      return (Extract_DCCALShower(record,
                     dynamic_cast<JFactoryT<DCCALShower>*>(factory)));
   }
   if (dataClassName =="DTrackTimeBased") {
      return (Extract_DTrackTimeBased(record,
                     dynamic_cast<JFactoryT<DTrackTimeBased>*>(factory), event));
   }
   if (dataClassName =="DTrigger") {
      return (Extract_DTrigger(record,
                     dynamic_cast<JFactoryT<DTrigger>*>(factory)) );
   }
   if (dataClassName =="DDIRCPmtHit") {      
      return (Extract_DDIRCPmtHit(record,
		     dynamic_cast<JFactoryT<DDIRCPmtHit>*>(factory), event));
   }
   if (dataClassName =="DFMWPCHit") {
      return (Extract_DFMWPCHit(record,
		     dynamic_cast<JFactoryT<DFMWPCHit>*>(factory), event));
   }
   if (dataClassName =="DFCALHit") {
      return Extract_DFCALHit(record,
		     dynamic_cast<JFactoryT<DFCALHit>*>(factory), event);
   }
   if (dataClassName =="DDetectorMatches") {
      return (Extract_DDetectorMatches(event, record,
                     dynamic_cast<JFactoryT<DDetectorMatches>*>(factory)));
   }
   if (dataClassName =="DEventHitStatistics") {
      return (Extract_DEventHitStatistics(record,
                     dynamic_cast<JFactoryT<DEventHitStatistics>*>(factory)));
   }
   if (dataClassName =="DBeamHelicity") {
      return (Extract_DBeamHelicity(record,
                     dynamic_cast<JFactoryT<DBeamHelicity>*>(factory)));
   }

   return false; //OBJECT_NOT_AVAILABLE
}

//------------------
// Extract_DMCReaction
//------------------
bool DEventSourceREST::Extract_DMCReaction(hddm_r::HDDM *record,
                                   JFactoryT<DMCReaction> *factory, const std::shared_ptr<const JEvent>& locEvent)
{
   /// Copies the data from the Reaction hddm class. This is called
   /// from JEventSourceREST::GetObjects. If factory is NULL, this
   /// returns OBJECT_NOT_AVAILABLE immediately.
   
   if (factory==NULL) {
      return false; //OBJECT_NOT_AVAILABLE
   }
   std::string tag = factory->GetTag();

	double locTargetCenterZ = 0.0;
	int locRunNumber = locEvent->GetRunNumber();
	{
		std::lock_guard<std::mutex> lock(readMutex);
		locTargetCenterZ = dTargetCenterZMap[locRunNumber];
	}
	DVector3 locPosition(0.0, 0.0, locTargetCenterZ);

   vector<DMCReaction*> dmcreactions;

   // loop over reaction records
   const hddm_r::ReactionList &reactions = record->getReactions();
   hddm_r::ReactionList::iterator iter;
   for (iter = reactions.begin(); iter != reactions.end(); ++iter) {
      if (iter->getJtag() != tag) {
         continue;
      }
      DMCReaction *mcreaction = new DMCReaction;
      dmcreactions.push_back(mcreaction);
      mcreaction->type = iter->getType();
      mcreaction->weight = iter->getWeight();
      double Ebeam = iter->getEbeam();

      hddm_r::Origin &origin = iter->getVertex().getOrigin();
      double torig = origin.getT();
      double zorig = origin.getVz();

      mcreaction->beam.setPosition(locPosition);
      mcreaction->beam.setMomentum(DVector3(0.0, 0.0, Ebeam));
      mcreaction->beam.setTime(torig - (zorig - locTargetCenterZ)/29.9792458);
      mcreaction->beam.setPID(Gamma);

      mcreaction->target.setPosition(locPosition);
      Particle_t ttype = iter->getTargetType();
      mcreaction->target.setPID((Particle_t)ttype);
      mcreaction->target.setTime(torig - (zorig - locTargetCenterZ)/29.9792458);
   }
   
   // Copy into factories
   factory->Set(dmcreactions);

   return true; //NOERROR;
}

//------------------
// Extract_DRFTime
//------------------
bool DEventSourceREST::Extract_DRFTime(hddm_r::HDDM *record,
                                   JFactoryT<DRFTime> *factory, const std::shared_ptr<const JEvent>& locEvent)
{
   if (factory==NULL)
      return false; //OBJECT_NOT_AVAILABLE
   string tag = factory->GetTag();

   vector<DRFTime*> locRFTimes;

   // loop over RF-time records
   const hddm_r::RFtimeList &rftimes = record->getRFtimes();
   hddm_r::RFtimeList::iterator iter;
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
		factory->Set(locRFTimes);
		return true; //NOERROR;
	}

	//Not found in the file, so either:
		//Experimental data & it's missing: bail
		//MC data: generate it

	vector<const DBeamPhoton*> locMCGENPhotons;
	locEvent->Get(locMCGENPhotons, "MCGEN");
	if(locMCGENPhotons.empty())
		return false; //OBJECT_NOT_AVAILABLE: Experimental data & it's missing: bail

	//Is MC data. Either:
		//No tag: return photon_time propagated by +/- n*locBeamBunchPeriod to get close to 0.0
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
		int locRunNumber = locEvent->GetRunNumber();
		{
			std::lock_guard<std::mutex> lock(readMutex);
			locBeamBunchPeriod = dBeamBunchPeriodMap[locRunNumber];
		}

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
   factory->Set(locRFTimes);

   return true; //NOERROR;
}

//------------------
// Extract_DBeamPhoton
//------------------
bool DEventSourceREST::Extract_DBeamPhoton(hddm_r::HDDM *record,
                                   JFactoryT<DBeamPhoton> *factory,
                                   const std::shared_ptr<const JEvent>& eventLoop)
{
   /// This is called from JEventSourceREST::GetObjects. If factory is NULL,
   /// return OBJECT_NOT_AVAILABLE immediately. If factory tag="MCGEN" then
   /// copy the beam photon data from the Reaction hddm class.

   if (factory==NULL)
      return false; //OBJECT_NOT_AVAILABLE
   string tag = factory->GetTag();

	vector<DBeamPhoton*> dbeam_photons;

	// extract the TAGH geometry
   vector<const DTAGHGeometry*> taghGeomVect;
   eventLoop->Get(taghGeomVect);
   if (taghGeomVect.empty())
      return false; //OBJECT_NOT_AVAILABLE
   const DTAGHGeometry* taghGeom = taghGeomVect[0];

   // extract the TAGM geometry
   vector<const DTAGMGeometry*> tagmGeomVect;
   eventLoop->Get(tagmGeomVect);
   if (tagmGeomVect.empty())
      return false; //OBJECT_NOT_AVAILABLE
   const DTAGMGeometry* tagmGeom = tagmGeomVect[0];

	if(tag == "MCGEN")
	{
		vector<const DMCReaction*> dmcreactions;
		eventLoop->Get(dmcreactions);

		for(size_t loc_i = 0; loc_i < dmcreactions.size(); ++loc_i)
		{
			DBeamPhoton *beamphoton = new DBeamPhoton;
			*(DKinematicData*)beamphoton = dmcreactions[loc_i]->beam;
			if(tagmGeom->E_to_column(beamphoton->energy(), beamphoton->dCounter))
				beamphoton->dSystem = SYS_TAGM;
			else if(taghGeom->E_to_counter(beamphoton->energy(), beamphoton->dCounter))
				beamphoton->dSystem = SYS_TAGH;
			else
				beamphoton->dSystem = SYS_NULL;
			dbeam_photons.push_back(beamphoton);
		}

		// Copy into factories
		factory->Set(dbeam_photons);

		return true; //NOERROR;
	}

	double locTargetCenterZ = 0.0;
	int locRunNumber = eventLoop->GetRunNumber();
	{
		std::lock_guard<std::mutex> lock(readMutex);
		locTargetCenterZ = dTargetCenterZMap[locRunNumber];
	}

	DVector3 pos(0.0, 0.0, locTargetCenterZ);

	//now get the objects
   const hddm_r::TagmBeamPhotonList &locTagmBeamPhotonList = record->getTagmBeamPhotons();
   hddm_r::TagmBeamPhotonList::iterator locTAGMiter;
   for(locTAGMiter = locTagmBeamPhotonList.begin(); locTAGMiter != locTagmBeamPhotonList.end(); ++locTAGMiter)
   {
		if (locTAGMiter->getJtag() != tag)
		 continue;
		
		// load the counter number (if it exists) and set the energy based on the counter
		unsigned int column = 0;
		hddm_r::TagmChannelList &locTagmChannelList = locTAGMiter->getTagmChannels();
		if (locTagmChannelList.size() > 0) {
			// it's easy if the column is already set 
			column = locTagmChannelList().getColumn();
		} else {
			// if the TAGM column isn't saved in the REST file, then we do one of two things
			//   1) if there's no special CCDB context associated with the file, we can just
			//      reverse engineer the counter, assuming the latest CCDB
			//   2) If there is a special CCDB context specified, then use that instead
			if (dJCalib_olds[locRunNumber] == nullptr) {
				if (!tagmGeom->E_to_column(locTAGMiter->getE(), column)) {
					column = 0;
				}
			} else {
				if (!dTAGMGeoms[locRunNumber]->E_to_column(locTAGMiter->getE(), column)) {
					column = 0;
				}
			}

			if(column == 0)
				std::cerr << "Error in DEventSourceREST - tagger microscope could not look up column for energy "
				    << locTAGMiter->getE() << std::endl;
		}
		
		// sometimes the simulation will set photons that miss the tagger counters to have a column of zero - skip these
		if(column == 0) {
			continue;
		}
		// throw away hits from bad or noisy counters
		int quality = dTAGMFiberQualities[locRunNumber][0][column];  // I think this works for the row? - we are generally not worrying about the quality of individual fibers
		if (quality != DTAGMHit_factory_Calib::k_fiber_good )
			continue;
         
		DBeamPhoton* gamma = new DBeamPhoton();

		double Elo_tagm = tagmGeom->getElow(column);
		double Ehi_tagm = tagmGeom->getEhigh(column);
		double Ebeam_tagm = (Elo_tagm + Ehi_tagm)/2.;

		// read the rest of the data from the REST file
		DVector3 mom(0.0, 0.0, Ebeam_tagm);
		gamma->setPID(Gamma);
		gamma->setMomentum(mom);
		gamma->setPosition(pos);
		gamma->setTime(locTAGMiter->getT());
		gamma->dSystem = SYS_TAGM;
		gamma->dCounter = column;

		auto locCovarianceMatrix = dResourcePool_TMatrixFSym->Get_SharedResource();
		locCovarianceMatrix->ResizeTo(7, 7);
		locCovarianceMatrix->Zero();
		gamma->setErrorMatrix(locCovarianceMatrix);
      
      	dbeam_photons.push_back(gamma);
   }

   const hddm_r::TaghBeamPhotonList &locTaghBeamPhotonList = record->getTaghBeamPhotons();
   hddm_r::TaghBeamPhotonList::iterator locTAGHiter;
   for(locTAGHiter = locTaghBeamPhotonList.begin(); locTAGHiter != locTaghBeamPhotonList.end(); ++locTAGHiter)
   {
      if (locTAGHiter->getJtag() != tag)
         continue;

		// load the counter number (if it exists) and set the energy based on the counter
		unsigned int counter = 0;
		hddm_r::TaghChannelList &locTaghChannelList = locTAGHiter->getTaghChannels();
		if (locTaghChannelList.size() > 0) {
			// it's easy if the column is already set 
			counter = locTaghChannelList().getCounter();
		} else {
			// if the TAGH column isn't saved in the REST file, then we do one of two things
			//   1) if there's no special CCDB context associated with the file, we can just
			//      reverse engineer the counter, assuming the latest CCDB
			//   2) If there is a special CCDB context specified, then use that instead
			if (dJCalib_olds[locRunNumber] == nullptr) {
				if (!taghGeom->E_to_counter(locTAGHiter->getE(), counter)) {
					counter = 0;
				}
			} else {
				if (!dTAGHGeoms[locRunNumber]->E_to_counter(locTAGHiter->getE(), counter)) {
					counter = 0;
				}
			}

			if(counter == 0)
				std::cerr << "Error in DEventSourceREST - tagger hodoscope could not look up counter for energy "
				    << locTAGHiter->getE() << std::endl;
		}

		// sometimes the simulation will set photons that miss the tagger counters to have a column of zero - skip these
		if(counter == 0) {
			continue;
		}
         	
		// throw away hits from bad or noisy counters
		int quality = dTAGHCounterQualities[locRunNumber][counter];
		if (quality != DTAGHHit_factory_Calib::k_counter_good )
			continue;
         
      	DBeamPhoton* gamma = new DBeamPhoton();

		double Elo_tagh = taghGeom->getElow(counter);
		double Ehi_tagh = taghGeom->getEhigh(counter);
		double Ebeam_tagh = (Elo_tagh + Ehi_tagh)/2.;

		DVector3 mom(0.0, 0.0, Ebeam_tagh);
		gamma->setPID(Gamma);
		gamma->setMomentum(mom);
		gamma->setPosition(pos);
		gamma->setTime(locTAGHiter->getT());
		gamma->dSystem = SYS_TAGH;
		gamma->dCounter = counter;

	      auto locCovarianceMatrix = dResourcePool_TMatrixFSym->Get_SharedResource();
	      locCovarianceMatrix->ResizeTo(7, 7);
	      locCovarianceMatrix->Zero();
		gamma->setErrorMatrix(locCovarianceMatrix);

      dbeam_photons.push_back(gamma);
   }

	if((tag == "TAGGEDMCGEN") && dbeam_photons.empty())
		return false; //OBJECT_NOT_AVAILABLE - EITHER: didn't hit a tagger counter //OR: old MC data (pre-saving TAGGEDMCGEN): try using TAGGEDMCGEN factory

	// Copy into factories
	factory->Set(dbeam_photons);

   return true; //NOERROR;
}

//------------------
// Extract_DMCThrown
//------------------
bool DEventSourceREST::Extract_DMCThrown(hddm_r::HDDM *record,
                                   JFactoryT<DMCThrown> *factory)
{
   /// Copies the data from the hddm vertex records. This is called
   /// from JEventSourceREST::GetObjects. If factory is NULL, this
   /// returns OBJECT_NOT_AVAILABLE immediately.

   if (factory==NULL) {
      return false; //OBJECT_NOT_AVAILABLE
   }
   string tag = factory->GetTag();

   vector<DMCThrown*> data;

   // loop over vertex records
   hddm_r::VertexList vertices = record->getVertices();
   hddm_r::VertexList::iterator iter;
   for (iter = vertices.begin(); iter != vertices.end(); ++iter) {
      if (iter->getJtag() != tag) {
         continue;
      }
      const hddm_r::Origin &orig = iter->getOrigin();
      double vx = orig.getVx();
      double vy = orig.getVy();
      double vz = orig.getVz();
      double vt = orig.getT();
      const hddm_r::ProductList &products = iter->getProducts();
      hddm_r::ProductList::iterator piter;
      for (piter = products.begin(); piter != products.end(); ++piter) {
         double E  = piter->getMomentum().getE();
         double px = piter->getMomentum().getPx();
         double py = piter->getMomentum().getPy();
         double pz = piter->getMomentum().getPz();
         double mass = sqrt(E*E - (px*px + py*py + pz*pz));
         if (!isfinite(mass)) {
            mass = 0.0;
         }
         DMCThrown *mcthrown = new DMCThrown;
         int pdgtype = piter->getPdgtype();
         Particle_t ptype = PDGtoPType(pdgtype);
         mcthrown->type = ptype;
         mcthrown->pdgtype = pdgtype;
         mcthrown->myid = piter->getId();
         mcthrown->parentid = piter->getParentId();
         mcthrown->mech = 0;
         mcthrown->setPID(ptype);
         mcthrown->setMomentum(DVector3(px, py, pz));
         mcthrown->setPosition(DVector3(vx, vy, vz));
         mcthrown->setTime(vt);
         data.push_back(mcthrown);
      }
   }

   // Copy into factory
   factory->Set(data);

   return true; //NOERROR;
}

//------------------
// Extract_DTOFPoint
//------------------
bool DEventSourceREST::Extract_DTOFPoint(hddm_r::HDDM *record,
                                   JFactoryT<DTOFPoint>* factory)
{
   /// Copies the data from the tofPoint hddm record. This is called
   /// from JEventSourceREST::GetObjects. If factory is NULL, this
   /// returns OBJECT_NOT_AVAILABLE immediately.

   if (factory==NULL) {
      return false; //OBJECT_NOT_AVAILABLE
   }
   string tag = factory->GetTag();

   vector<DTOFPoint*> data;

   // loop over tofPoint records
   const hddm_r::TofPointList &tofs = record->getTofPoints();
   hddm_r::TofPointList::iterator iter;
   for (iter = tofs.begin(); iter != tofs.end(); ++iter) {
      if (iter->getJtag() != tag) {
         continue;
      }
      DTOFPoint *tofpoint = new DTOFPoint();
      tofpoint->pos = DVector3(iter->getX(),iter->getY(),iter->getZ());
      tofpoint->t = iter->getT();
      tofpoint->dE = iter->getDE();
      tofpoint->tErr = iter->getTerr();

      //Status
      const hddm_r::TofStatusList& locTofStatusList = iter->getTofStatuses();
      hddm_r::TofStatusList::iterator locStatusIterator = locTofStatusList.begin();
      if(locStatusIterator == locTofStatusList.end())
	{
	  tofpoint->dHorizontalBar = 0;
	  tofpoint->dVerticalBar = 0;
	  tofpoint->dHorizontalBarStatus = 3;
	  tofpoint->dVerticalBarStatus = 3;
	}
      else //should only be 1
	{
	  for(; locStatusIterator != locTofStatusList.end(); ++locStatusIterator)
	    {
	      int locStatus = locStatusIterator->getStatus(); //horizontal_bar + 45*vertical_bar + 45*45*horizontal_status + 45*45*4*vertical_status
	      tofpoint->dVerticalBarStatus = locStatus/(45*45*4);
	      locStatus %= 45*45*4; //Assume compiler optimizes multiplication
	      tofpoint->dHorizontalBarStatus = locStatus/(45*45);
	      locStatus %= 45*45;
	      tofpoint->dVerticalBar = locStatus/45;
	      tofpoint->dHorizontalBar = locStatus % 45;
	    }
	}
      // Energy deposition
      const hddm_r::TofEnergyDepositionList& locTofEnergyDepositionList = iter->getTofEnergyDepositions();
      hddm_r::TofEnergyDepositionList::iterator locEnergyDepositionIterator = locTofEnergyDepositionList.begin();
      if(locEnergyDepositionIterator == locTofEnergyDepositionList.end())
	{
	  tofpoint->dE1 = 0.;
	  tofpoint->dE2 = 0.;
	}
      else //should only be 1
	{
	  for(; locEnergyDepositionIterator != locTofEnergyDepositionList.end(); ++locEnergyDepositionIterator)
	    {
	      tofpoint->dE1 = locEnergyDepositionIterator->getDE1();
	      tofpoint->dE2 = locEnergyDepositionIterator->getDE2();
	    }
	}
      
      data.push_back(tofpoint);
   }

   // Copy into factory
   factory->Set(data);

   return true; //NOERROR;
}

//------------------
// Extract_DCTOFPoint
//------------------
bool DEventSourceREST::Extract_DCTOFPoint(hddm_r::HDDM *record,
                                   JFactoryT<DCTOFPoint>* factory)
{
   /// Copies the data from the ctofPoint hddm record. This is called
   /// from JEventSourceREST::GetObjects. If factory is NULL, this
   /// returns OBJECT_NOT_AVAILABLE immediately.

   if (factory==NULL) {
      return false; //OBJECT_NOT_AVAILABLE
   }
   string tag = factory->GetTag();

   vector<DCTOFPoint*> data;

   // loop over ctofPoint records
   const hddm_r::CtofPointList &ctofs = record->getCtofPoints();
   hddm_r::CtofPointList::iterator iter;
   for (iter = ctofs.begin(); iter != ctofs.end(); ++iter) {
      if (iter->getJtag() != tag) {
         continue;
      }
      DCTOFPoint *ctofpoint = new DCTOFPoint();
      ctofpoint->bar = iter->getBar();
      ctofpoint->pos = DVector3(iter->getX(),iter->getY(),iter->getZ());
      ctofpoint->t = iter->getT();
      ctofpoint->dE = iter->getDE();
      
      data.push_back(ctofpoint);
   }

   factory->Set(data);


   return true; //NOERROR;
}


//------------------
// Extract_DSCHit
//------------------
bool DEventSourceREST::Extract_DSCHit(hddm_r::HDDM *record,
                                   JFactoryT<DSCHit>* factory)
{
   /// Copies the data from the startHit hddm record. This is called
   /// from JEventSourceREST::GetObjects. If factory is NULL, this
   /// returns OBJECT_NOT_AVAILABLE immediately.

   if (factory==NULL) {
      return false; //OBJECT_NOT_AVAILABLE
   }
   string tag = factory->GetTag();

   vector<DSCHit*> data;

   // loop over startHit records
   const hddm_r::StartHitList &starts = record->getStartHits();
   hddm_r::StartHitList::iterator iter;
   for (iter = starts.begin(); iter != starts.end(); ++iter) {
      if (iter->getJtag() != tag) {
         continue;
      }
      DSCHit *start = new DSCHit();
      start->sector = iter->getSector();
      start->dE = iter->getDE();
      start->t = iter->getT();
      data.push_back(start);
   }

   // Copy into factory
   factory->Set(data);

   return true; //NOERROR;
}

//-----------------------
// Extract_DTrigger
//-----------------------
bool DEventSourceREST::Extract_DTrigger(hddm_r::HDDM *record, JFactoryT<DTrigger>* factory)
{
	/// Copies the data from the trigger hddm record. This is
	/// call from JEventSourceREST::GetObjects. If factory is NULL, this
	/// returns OBJECT_NOT_AVAILABLE immediately.

	if (factory==NULL)
		return false; //OBJECT_NOT_AVAILABLE
	string tag = factory->GetTag();

	vector<DTrigger*> data;

	// loop over trigger records
	const hddm_r::TriggerList &triggers = record->getTriggers();
	hddm_r::TriggerList::iterator iter;
	for (iter = triggers.begin(); iter != triggers.end(); ++iter)
	{
		if (iter->getJtag() != tag)
			continue;

		DTrigger *locTrigger = new DTrigger();
		locTrigger->Set_L1TriggerBits(Convert_SignedIntToUnsigned(iter->getL1_trig_bits()));
		locTrigger->Set_L1FrontPanelTriggerBits(Convert_SignedIntToUnsigned(iter->getL1_fp_trig_bits()));
		
		const hddm_r::TriggerEnergySumsList& locTriggerEnergySumsList = iter->getTriggerEnergySumses();
		hddm_r::TriggerEnergySumsList::iterator locTriggerEnergySumsIterator = locTriggerEnergySumsList.begin();
		if(locTriggerEnergySumsIterator == locTriggerEnergySumsList.end()) {
			locTrigger->Set_GTP_BCALEnergy(0);
			locTrigger->Set_GTP_FCALEnergy(0);
		} else { //should only be 1
			for(; locTriggerEnergySumsIterator != locTriggerEnergySumsList.end(); ++locTriggerEnergySumsIterator) {
				locTrigger->Set_GTP_BCALEnergy(locTriggerEnergySumsIterator->getBCALEnergySum());
				locTrigger->Set_GTP_FCALEnergy(locTriggerEnergySumsIterator->getFCALEnergySum());
			}
		}
		
		data.push_back(locTrigger);
	}

	// Copy into factory
	factory->Set(data);

	return true; //NOERROR;
}

//-----------------------
// Extract_DBeamHelicity
//-----------------------
bool DEventSourceREST::Extract_DBeamHelicity(hddm_r::HDDM *record, JFactoryT<DBeamHelicity>* factory)
{
	/// Copies the data from the electron beam hddm record. This is
	/// call from JEventSourceREST::GetObjects. If factory is NULL, this
	/// returns OBJECT_NOT_AVAILABLE immediately.

	if (factory==NULL)
		return false; //OBJECT_NOT_AVAILABLE
	string tag = factory->GetTag();

	vector<DBeamHelicity*> data;

	// loop over the electron beam info records
	const hddm_r::ElectronBeamList &ebeams = record->getElectronBeams();
	hddm_r::ElectronBeamList::iterator iter;
	for (iter = ebeams.begin(); iter != ebeams.end(); ++iter)
	{
 		if (iter->getJtag() != tag)
 			continue;

		// don't make an object if there's no beam helicity
		if(iter->getHelicitydata()&0x01 == 0)  continue;

		DBeamHelicity *locBeamHelicity = new DBeamHelicity();
		locBeamHelicity->helicity = (iter->getHelicitydata()>>1)&0x01;

		locBeamHelicity->pattern_sync = (iter->getHelicitydata()>>2)&0x01;
		locBeamHelicity->t_settle = (iter->getHelicitydata()>>3)&0x01;
		locBeamHelicity->pair_sync = (iter->getHelicitydata()>>4)&0x01;
		locBeamHelicity->ihwp = (iter->getHelicitydata()>>5)&0x01;
		locBeamHelicity->beam_on = (iter->getHelicitydata()>>6)&0x01;
				
		data.push_back(locBeamHelicity);
	}

	// Copy into factory
	factory->Set(data);

	return true; //NOERROR;
}



//-----------------------
// Extract_DFCALShower
//-----------------------
bool DEventSourceREST::Extract_DFCALShower(hddm_r::HDDM *record,
                                   JFactoryT<DFCALShower>* factory)
{
   /// Copies the data from the fcalShower hddm record. This is
   /// call from JEventSourceREST::GetObjects. If factory is NULL, this
   /// returns OBJECT_NOT_AVAILABLE immediately.

   if (factory==NULL) {
      return false; //OBJECT_NOT_AVAILABLE
   }
   string tag = factory->GetTag();

   vector<DFCALShower*> data;

   // loop over fcal shower records
   const hddm_r::FcalShowerList &showers =
                 record->getFcalShowers();
   hddm_r::FcalShowerList::iterator iter;
   for (iter = showers.begin(); iter != showers.end(); ++iter) {
      if (iter->getJtag() != tag)
         continue;

      DFCALShower *shower = new DFCALShower();
      shower->setPosition(DVector3(iter->getX(),iter->getY(),iter->getZ()));
      shower->setEnergy(iter->getE());
      shower->setTime(iter->getT());

	  if(USE_CCDB_FCAL_COVARIANCE) {
	  	 dFCALShowerFactory->FillCovarianceMatrix(shower);
	  } else {
		  TMatrixFSym covariance(5);
		  covariance(0,0) = iter->getEerr()*iter->getEerr();
		  covariance(1,1) = iter->getXerr()*iter->getXerr();
		  covariance(2,2) = iter->getYerr()*iter->getYerr();
		  covariance(3,3) = iter->getZerr()*iter->getZerr();
		  covariance(4,4) = iter->getTerr()*iter->getTerr();
		  covariance(1,2) = covariance(2,1) = iter->getXycorr()*iter->getXerr()*iter->getYerr();
		  covariance(1,3) = covariance(3,1) = iter->getXzcorr()*iter->getXerr()*iter->getZerr();
		  covariance(2,3) = covariance(3,2) = iter->getYzcorr()*iter->getYerr()*iter->getZerr();
		  covariance(0,3) = covariance(3,0) = iter->getEzcorr()*iter->getEerr()*iter->getZerr();
		  covariance(3,4) = covariance(4,3) = iter->getTzcorr()*iter->getTerr()*iter->getZerr();
	  
		  // further correlations (an extension of REST format, so code is different.)
		  const hddm_r::FcalCorrelationsList& locFcalCorrelationsList = iter->getFcalCorrelationses();
		  hddm_r::FcalCorrelationsList::iterator locFcalCorrelationsIterator = locFcalCorrelationsList.begin();
		  if(locFcalCorrelationsIterator != locFcalCorrelationsList.end()) {
			  covariance(0,4) = covariance(4,0) = locFcalCorrelationsIterator->getEtcorr()*iter->getEerr()*iter->getTerr();
			  covariance(0,1) = covariance(1,0) = locFcalCorrelationsIterator->getExcorr()*iter->getEerr()*iter->getXerr();
			  covariance(0,2) = covariance(2,0) = locFcalCorrelationsIterator->getEycorr()*iter->getEerr()*iter->getYerr();
			  covariance(1,4) = covariance(4,1) = locFcalCorrelationsIterator->getTxcorr()*iter->getTerr()*iter->getXerr();
			  covariance(2,4) = covariance(4,2) = locFcalCorrelationsIterator->getTycorr()*iter->getTerr()*iter->getYerr();
		  }
		  shower->ExyztCovariance = covariance;
	  }

      // MVA classifier output - this information is being calculated in DNeutralShower now!
      //const hddm_r::FcalShowerClassificationList& locFcalShowerClassificationList = iter->getFcalShowerClassifications();
      //hddm_r::FcalShowerClassificationList::iterator locFcalShowerClassificationIterator = locFcalShowerClassificationList.begin();
      //if(locFcalShowerClassificationIterator != locFcalShowerClassificationList.end()) {
	  //        shower->setClassifierOutput(locFcalShowerClassificationIterator->getClassifierOuput());
      //}

      // shower shape and other parameters.  used e.g. as input to MVA classifier
      const hddm_r::FcalShowerPropertiesList& locFcalShowerPropertiesList = iter->getFcalShowerPropertiesList();
      hddm_r::FcalShowerPropertiesList::iterator locFcalShowerPropertiesIterator = locFcalShowerPropertiesList.begin();
      if(locFcalShowerPropertiesIterator != locFcalShowerPropertiesList.end()) {
	          shower->setDocaTrack(locFcalShowerPropertiesIterator->getDocaTrack());
	          shower->setTimeTrack(locFcalShowerPropertiesIterator->getTimeTrack());
	          shower->setSumU(locFcalShowerPropertiesIterator->getSumU());
	          shower->setSumV(locFcalShowerPropertiesIterator->getSumV());
	          shower->setE1E9(locFcalShowerPropertiesIterator->getE1E9());
	          shower->setE9E25(locFcalShowerPropertiesIterator->getE9E25());
      }

      const hddm_r::FcalShowerNBlocksList& locFcalShowerNBlocksList = iter->getFcalShowerNBlockses();
      hddm_r::FcalShowerNBlocksList::iterator locFcalShowerNBlocksIterator = locFcalShowerNBlocksList.begin();
      if(locFcalShowerNBlocksIterator != locFcalShowerNBlocksList.end()) {
		  shower->setNumBlocks(locFcalShowerNBlocksIterator->getNumBlocks());
      }      
      data.push_back(shower);
   }

   // Copy into factory
   factory->Set(data);

   return true; //NOERROR;
}

//-----------------------
// Extract_DBCALShower
//-----------------------
bool DEventSourceREST::Extract_DBCALShower(hddm_r::HDDM *record,
                                   JFactoryT<DBCALShower>* factory)
{
   /// Copies the data from the bcalShower hddm record. This is
   /// call from JEventSourceREST::GetObjects. If factory is NULL, this
   /// returns OBJECT_NOT_AVAILABLE immediately.

   if (factory==NULL) {
      return false; //OBJECT_NOT_AVAILABLE
   }
   string tag = factory->GetTag();

   vector<DBCALShower*> data;

   // loop over bcal shower records
   const hddm_r::BcalShowerList &showers =
                 record->getBcalShowers();
   hddm_r::BcalShowerList::iterator iter;
   for (iter = showers.begin(); iter != showers.end(); ++iter) {
      if (iter->getJtag() != tag)
         continue;

	  DBCALShower *shower = new DBCALShower();
	  shower->E = iter->getE();
	  shower->E_raw = -1;
	  shower->x = iter->getX();
	  shower->y = iter->getY();
	  shower->z = iter->getZ();
	  shower->t = iter->getT();
	  shower->Q = 0;    // Fix this to zero for now, can add to REST if it's ever used in higher-level analyses

	  if(USE_CCDB_BCAL_COVARIANCE) {
	  	 dBCALShowerFactory->FillCovarianceMatrix(shower);
	  } else {
		  TMatrixFSym covariance(5);
		  covariance(0,0) = iter->getEerr()*iter->getEerr();
		  covariance(1,1) = iter->getXerr()*iter->getXerr();
		  covariance(2,2) = iter->getYerr()*iter->getYerr();
		  covariance(3,3) = iter->getZerr()*iter->getZerr();
		  covariance(4,4) = iter->getTerr()*iter->getTerr();
		  covariance(1,2) = covariance(2,1) = iter->getXycorr()*iter->getXerr()*iter->getYerr();
		  covariance(1,3) = covariance(3,1) = iter->getXzcorr()*iter->getXerr()*iter->getZerr();
		  covariance(2,3) = covariance(3,2) = iter->getYzcorr()*iter->getYerr()*iter->getZerr();
		  covariance(0,3) = covariance(3,0) = iter->getEzcorr()*iter->getEerr()*iter->getZerr();
		  covariance(3,4) = covariance(4,3) = iter->getTzcorr()*iter->getTerr()*iter->getZerr();

		  // further correlations (an extension of REST format, so code is different.)
		  const hddm_r::BcalCorrelationsList& locBcalCorrelationsList = iter->getBcalCorrelationses();
		  hddm_r::BcalCorrelationsList::iterator locBcalCorrelationsIterator = locBcalCorrelationsList.begin();
		  if(locBcalCorrelationsIterator != locBcalCorrelationsList.end()) {
			  covariance(0,4) = covariance(4,0) = locBcalCorrelationsIterator->getEtcorr()*iter->getEerr()*iter->getTerr();
			  covariance(0,1) = covariance(1,0) = locBcalCorrelationsIterator->getExcorr()*iter->getEerr()*iter->getXerr();
			  covariance(0,2) = covariance(2,0) = locBcalCorrelationsIterator->getEycorr()*iter->getEerr()*iter->getYerr();
			  covariance(1,4) = covariance(4,1) = locBcalCorrelationsIterator->getTxcorr()*iter->getTerr()*iter->getXerr();
			  covariance(2,4) = covariance(4,2) = locBcalCorrelationsIterator->getTycorr()*iter->getTerr()*iter->getYerr();
		  }
		  shower->ExyztCovariance = covariance;
		}

		// preshower
		const hddm_r::PreshowerList& locPreShowerList = iter->getPreshowers();
		hddm_r::PreshowerList::iterator locPreShowerIterator = locPreShowerList.begin();
		if(locPreShowerIterator == locPreShowerList.end())
			shower->E_preshower = 0.0;
		else //should only be 1
		{
			for(; locPreShowerIterator != locPreShowerList.end(); ++locPreShowerIterator)
				shower->E_preshower = locPreShowerIterator->getPreshowerE();
		}

		// width
		const hddm_r::WidthList& locWidthList = iter->getWidths();
		hddm_r::WidthList::iterator locWidthIterator = locWidthList.begin();
		if(locWidthIterator == locWidthList.end()) {
			shower->sigLong = -1.;
			shower->sigTrans = -1.;
			shower->sigTheta = -1.;
		}
		else //should only be 1
		{
			for(; locWidthIterator != locWidthList.end(); ++locWidthIterator) {
				shower->sigLong = locWidthIterator->getSigLong();
				shower->sigTrans = locWidthIterator->getSigTrans();
				shower->sigTheta = locWidthIterator->getSigTheta();
			}
		}

		const hddm_r::BcalClusterList& locBcalClusterList = iter->getBcalClusters();
		hddm_r::BcalClusterList::iterator locBcalClusterIterator = locBcalClusterList.begin();
		if(locBcalClusterIterator == locBcalClusterList.end())
			shower->N_cell = -1;
		else //should only be 1
		{
			for(; locBcalClusterIterator != locBcalClusterList.end(); ++locBcalClusterIterator)
				shower->N_cell = locBcalClusterIterator->getNcell();
		}

		const hddm_r::BcalLayersList& locBcalLayersList = iter->getBcalLayerses();
		hddm_r::BcalLayersList::iterator locBcalLayersIterator = locBcalLayersList.begin();
		if(locBcalLayersIterator == locBcalLayersList.end()) {
		        shower->E_L2 = 0.;
		        shower->E_L3 = 0.;
		        shower->E_L4 = 0.;
			shower->rmsTime = -1;
		}
		else //should only be 1
		{
			for(; locBcalLayersIterator != locBcalLayersList.end(); ++locBcalLayersIterator) {
				shower->rmsTime = locBcalLayersIterator->getRmsTime();
				shower->E_L2 = locBcalLayersIterator->getE_L2();
				shower->E_L3 = locBcalLayersIterator->getE_L3();
				shower->E_L4 = locBcalLayersIterator->getE_L4();
            }
		}

      data.push_back(shower);
   }

   // Copy into factory
   factory->Set(data);

   return true; //NOERROR;
}

//-----------------------
// Extract_DCCALShower
//-----------------------
bool DEventSourceREST::Extract_DCCALShower(hddm_r::HDDM *record,
                                   JFactoryT<DCCALShower>* factory)
{
   /// Copies the data from the ccalShower hddm record. This is
   /// call from JEventSourceREST::GetObjects. If factory is NULL, this
   /// returns OBJECT_NOT_AVAILABLE immediately.

   if (factory==NULL) {
      return false; //OBJECT_NOT_AVAILABLE
   }
   string tag = factory->GetTag();

   vector<DCCALShower*> data;

   // loop over ccal shower records
   const hddm_r::CcalShowerList &showers =
                 record->getCcalShowers();
   hddm_r::CcalShowerList::iterator iter;
   for (iter = showers.begin(); iter != showers.end(); ++iter) {
      if (iter->getJtag() != tag)
         continue;

      DCCALShower *shower = new DCCALShower();
      shower->E = iter->getE();
      shower->x = iter->getX();
      shower->y = iter->getY();
      shower->z = iter->getZ();
      shower->time = iter->getT();
      shower->sigma_t = iter->getTerr();
      shower->sigma_E = iter->getEerr();
      shower->Emax = iter->getEmax();
      shower->x1 = iter->getX1();
      shower->y1 = iter->getY1();
      shower->chi2 = iter->getChi2();
      
      shower->type = iter->getType();
      shower->dime = iter->getDime();
      shower->id = iter->getId();
      shower->idmax = iter->getIdmax();
      
      data.push_back(shower);
   }

   // Copy into factory
   factory->Set(data);

   return true; //NOERROR;
}

//--------------------------------
// Extract_DTrackTimeBased
//--------------------------------
bool DEventSourceREST::Extract_DTrackTimeBased(hddm_r::HDDM *record,
                                   JFactoryT<DTrackTimeBased>* factory, const std::shared_ptr<const JEvent>& locEvent)
{
   /// Copies the data from the chargedTrack hddm record. This is
   /// call from JEventSourceREST::GetObjects. If factory is NULL, this
   /// returns OBJECT_NOT_AVAILABLE immediately.

   if (factory==NULL) {
      return false; //OBJECT_NOT_AVAILABLE
   }
   
   int locRunNumber = locEvent->GetRunNumber();
   DVector2 locBeamCenter,locBeamDir;
   double locBeamZ0=0.;
   {
     std::lock_guard<std::mutex> lock(readMutex);
     locBeamCenter = dBeamCenterMap[locRunNumber];
     locBeamDir = dBeamDirMap[locRunNumber];
     locBeamZ0 = dBeamZ0Map[locRunNumber];
   }

   string tag = factory->GetTag();

   vector<DTrackTimeBased*> data;

   // loop over chargedTrack records
   const hddm_r::ChargedTrackList &tracks = record->getChargedTracks();
   hddm_r::ChargedTrackList::iterator iter;
   for (iter = tracks.begin(); iter != tracks.end(); ++iter) {
      if (iter->getJtag() != tag) {
         continue;
      }
      DTrackTimeBased *tra = new DTrackTimeBased();
      tra->trackid = 0;
      tra->candidateid = iter->getCandidateId();
      Particle_t ptype = iter->getPtype();
      tra->setPID(ptype);

      const hddm_r::TrackFit &fit = iter->getTrackFit();
      tra->Ndof = fit.getNdof();
      tra->chisq = fit.getChisq();
      tra->FOM = TMath::Prob(tra->chisq, tra->Ndof);
      tra->setTime(fit.getT0());
      DVector3 track_pos(fit.getX0(),fit.getY0(),fit.getZ0());
      DVector3 track_mom(fit.getPx(),fit.getPy(),fit.getPz());
      tra->setPosition(track_pos);
      tra->setMomentum(track_mom);

      auto loc5x5ErrorMatrix = dResourcePool_TMatrixFSym->Get_SharedResource();
      loc5x5ErrorMatrix->ResizeTo(5, 5);
      (*loc5x5ErrorMatrix)(0,0) = fit.getE11();
      (*loc5x5ErrorMatrix)(0,1) = (*loc5x5ErrorMatrix)(1,0) = fit.getE12();
      (*loc5x5ErrorMatrix)(0,2) = (*loc5x5ErrorMatrix)(2,0) = fit.getE13();
      (*loc5x5ErrorMatrix)(0,3) = (*loc5x5ErrorMatrix)(3,0) = fit.getE14();
      (*loc5x5ErrorMatrix)(0,4) = (*loc5x5ErrorMatrix)(4,0) = fit.getE15();
      (*loc5x5ErrorMatrix)(1,1) = fit.getE22();
      (*loc5x5ErrorMatrix)(1,2) = (*loc5x5ErrorMatrix)(2,1) = fit.getE23();
      (*loc5x5ErrorMatrix)(1,3) = (*loc5x5ErrorMatrix)(3,1) = fit.getE24();
      (*loc5x5ErrorMatrix)(1,4) = (*loc5x5ErrorMatrix)(4,1) = fit.getE25();
      (*loc5x5ErrorMatrix)(2,2) = fit.getE33();
      (*loc5x5ErrorMatrix)(2,3) = (*loc5x5ErrorMatrix)(3,2) = fit.getE34();
      (*loc5x5ErrorMatrix)(2,4) = (*loc5x5ErrorMatrix)(4,2) = fit.getE35();
      (*loc5x5ErrorMatrix)(3,3) = fit.getE44();
      (*loc5x5ErrorMatrix)(3,4) = (*loc5x5ErrorMatrix)(4,3) = fit.getE45();
      (*loc5x5ErrorMatrix)(4,4) = fit.getE55();
      tra->setTrackingErrorMatrix(loc5x5ErrorMatrix);

      // Convert from cartesian coordinates to the 5x1 state vector corresponding to the tracking error matrix.
      double vect[5];
      DVector2 beam_pos=locBeamCenter+(track_pos.Z()-locBeamZ0)*locBeamDir;
      DVector2 diff(track_pos.X()-beam_pos.X(),track_pos.Y()-beam_pos.Y());
      vect[2]=tan(M_PI_2 - track_mom.Theta());
      vect[1]=track_mom.Phi();
      double sinphi=sin(vect[1]);
      double cosphi=cos(vect[1]);
      vect[0]=tra->charge()/track_mom.Perp();
      vect[4]=track_pos.Z();
      vect[3]=diff.Mod();

      if ((diff.X() > 0 && sinphi>0) || (diff.Y() <0 && cosphi>0) || (diff.Y() >0 && cosphi<0) || (diff.X() <0 && sinphi<0))
        vect[3] *= -1.; 
      tra->setTrackingStateVector(vect[0], vect[1], vect[2], vect[3], vect[4]);

      // Set the 7x7 covariance matrix.
      auto loc7x7ErrorMatrix = dResourcePool_TMatrixFSym->Get_SharedResource();
      loc7x7ErrorMatrix->ResizeTo(7, 7);
	   Get7x7ErrorMatrix(tra->mass(), vect, loc5x5ErrorMatrix.get(), loc7x7ErrorMatrix.get());
      tra->setErrorMatrix(loc7x7ErrorMatrix);
      (*loc7x7ErrorMatrix)(6, 6) = fit.getT0err()*fit.getT0err();

      // Positions at each FDC package
      const hddm_r::FdcTrackPosList locFdcTrackPosList = iter->getFdcTrackPoses();
      hddm_r::FdcTrackPosList::iterator locFdcTrackPosIterator = locFdcTrackPosList.begin();
      if (locFdcTrackPosIterator!=locFdcTrackPosList.end()){
	// Create the extrapolation vector
	vector<DTrackFitter::Extrapolation_t>myvector;
	tra->extrapolations.emplace(SYS_FDC,myvector);
	for(; locFdcTrackPosIterator != locFdcTrackPosList.end(); ++locFdcTrackPosIterator){
	  DVector3 pos(locFdcTrackPosIterator->getX(),
		       locFdcTrackPosIterator->getY(),
		       locFdcTrackPosIterator->getZ());
	  DVector3 mom;
	  tra->extrapolations[SYS_FDC].push_back(DTrackFitter::Extrapolation_t(pos,mom,0.,0.));
	}
      }

      // Track parameters at exit of tracking volume
      const hddm_r::ExitParamsList& locExitParamsList = iter->getExitParamses();
      hddm_r::ExitParamsList::iterator locExitParamsIterator = locExitParamsList.begin();	
      if (locExitParamsIterator!=locExitParamsList.end()){
	// Create the extrapolation vector
	vector<DTrackFitter::Extrapolation_t>myvector;
	tra->extrapolations.emplace(SYS_NULL,myvector);
	
	for(; locExitParamsIterator != locExitParamsList.end(); ++locExitParamsIterator){
	  DVector3 pos(locExitParamsIterator->getX1(),
		       locExitParamsIterator->getY1(),
		       locExitParamsIterator->getZ1());
	  DVector3 mom(locExitParamsIterator->getPx1(),
		     locExitParamsIterator->getPy1(),
		       locExitParamsIterator->getPz1());
	  tra->extrapolations[SYS_NULL].push_back(DTrackFitter::Extrapolation_t(pos,mom,locExitParamsIterator->getT1(),0.));
	}
      }
      // Hit layers
      const hddm_r::ExpectedhitsList& locExpectedhitsList = iter->getExpectedhitses();
	   hddm_r::ExpectedhitsList::iterator locExpectedhitsIterator = locExpectedhitsList.begin();
		if(locExpectedhitsIterator == locExpectedhitsList.end())
		{
			tra->potential_cdc_hits_on_track = 0;
			tra->potential_fdc_hits_on_track = 0;
			tra->measured_cdc_hits_on_track = 0;
			tra->measured_fdc_hits_on_track = 0;
			//tra->cdc_hit_usage.total_hits = 0;
			//tra->fdc_hit_usage.total_hits = 0;
		}
		else //should only be 1
		{
			for(; locExpectedhitsIterator != locExpectedhitsList.end(); ++locExpectedhitsIterator)
			{
				tra->potential_cdc_hits_on_track = locExpectedhitsIterator->getExpectedCDChits();
				tra->potential_fdc_hits_on_track = locExpectedhitsIterator->getExpectedFDChits();
				tra->measured_cdc_hits_on_track = locExpectedhitsIterator->getMeasuredCDChits();
				tra->measured_fdc_hits_on_track = locExpectedhitsIterator->getMeasuredFDChits();
				//tra->cdc_hit_usage.total_hits = locExpectedhitsIterator->getMeasuredCDChits();
				//tra->fdc_hit_usage.total_hits = locExpectedhitsIterator->getMeasuredFDChits();
			}
		}
		
		// Expected number of hits
      const hddm_r::HitlayersList& locHitlayersList = iter->getHitlayerses();
	   hddm_r::HitlayersList::iterator locHitlayersIterator = locHitlayersList.begin();
		if(locHitlayersIterator == locHitlayersList.end())
		{
			tra->dCDCRings = 0;
			tra->dFDCPlanes = 0;
		}
		else //should only be 1
		{
			for(; locHitlayersIterator != locHitlayersList.end(); ++locHitlayersIterator)
			{
				tra->dCDCRings = locHitlayersIterator->getCDCrings();
				tra->dFDCPlanes = locHitlayersIterator->getFDCplanes();
			}
		}

		// MC match hit info
      const hddm_r::McmatchList& locMCMatchesList = iter->getMcmatchs();
	   hddm_r::McmatchList::iterator locMcmatchIterator = locMCMatchesList.begin();
		if(locMcmatchIterator == locMCMatchesList.end())
		{
			tra->dCDCRings = 0;
			tra->dFDCPlanes = 0;
		}
		else //should only be 1
		{
			for(; locMcmatchIterator != locMCMatchesList.end(); ++locMcmatchIterator)
			{
				tra->dMCThrownMatchMyID = locMcmatchIterator->getIthrown();
				tra->dNumHitsMatchedToThrown = locMcmatchIterator->getNumhitsmatch();
			}
		}

      // add the drift chamber dE/dx information
      const hddm_r::DEdxDCList &el = iter->getDEdxDCs();
      hddm_r::DEdxDCList::iterator diter = el.begin();
      tra->ddx_CDC_trunc.clear();
      tra->ddx_CDC_amp_trunc.clear();
      tra->ddEdx_CDC_trunc.clear();
      tra->ddEdx_CDC_amp_trunc.clear();
      tra->ddx_FDC_trunc.clear();
      tra->ddx_FDC_amp_trunc.clear();
      tra->ddEdx_FDC_trunc.clear();
      tra->ddEdx_FDC_amp_trunc.clear();
      if (diter != el.end()) {
         tra->dNumHitsUsedFordEdx_FDC = diter->getNsampleFDC();
         tra->dNumHitsUsedFordEdx_CDC = diter->getNsampleCDC();
         tra->ddEdx_FDC = diter->getDEdxFDC();
         tra->ddEdx_CDC = diter->getDEdxCDC();
         tra->ddx_FDC = diter->getDxFDC();
         tra->ddx_CDC = diter->getDxCDC();  
	 const hddm_r::CDCAmpdEdxList &el2 = diter->getCDCAmpdEdxs();
	 hddm_r::CDCAmpdEdxList::iterator diter2 = el2.begin();
	 if (diter2 != el2.end()){
	   tra->ddx_CDC_amp= diter2->getDxCDCAmp();
	   tra->ddEdx_CDC_amp = diter2->getDEdxCDCAmp();
	 }
	 else{
	   tra->ddx_CDC_amp=tra->ddx_CDC;
	   tra->ddEdx_CDC_amp=tra->ddEdx_CDC;
	 }
         const hddm_r::CDCdEdxTruncList &cdctruncs = diter->getCDCdEdxTruncs();
         hddm_r::CDCdEdxTruncList::iterator itcdc;
         for (itcdc = cdctruncs.begin(); itcdc != cdctruncs.end(); ++itcdc) {
           int ntrunc = itcdc->getNtrunc();
           for (int s=(int)tra->ddx_CDC_trunc.size(); s <= ntrunc; ++s) {
             tra->ddx_CDC_trunc.push_back(0);
             tra->ddx_CDC_amp_trunc.push_back(0);
             tra->ddEdx_CDC_trunc.push_back(0);
             tra->ddEdx_CDC_amp_trunc.push_back(0);
           }
           tra->ddx_CDC_trunc[ntrunc] = itcdc->getDx();
           tra->ddx_CDC_amp_trunc[ntrunc] = itcdc->getDxAmp();
           tra->ddEdx_CDC_trunc[ntrunc] = itcdc->getDEdx();
           tra->ddEdx_CDC_amp_trunc[ntrunc] = itcdc->getDEdxAmp();
         }
         const hddm_r::FDCdEdxTruncList &fdctruncs = diter->getFDCdEdxTruncs();
         hddm_r::FDCdEdxTruncList::iterator itfdc;
         for (itfdc = fdctruncs.begin(); itfdc != fdctruncs.end(); ++itfdc) {
           int ntrunc = itfdc->getNtrunc();
           for (int s=(int)tra->ddx_FDC_trunc.size(); s <= ntrunc; ++s) {
             tra->ddx_FDC_trunc.push_back(0);
             tra->ddx_FDC_amp_trunc.push_back(0);
             tra->ddEdx_FDC_trunc.push_back(0);
             tra->ddEdx_FDC_amp_trunc.push_back(0);
           }
           tra->ddx_FDC_trunc[ntrunc] = itfdc->getDx();
           tra->ddx_FDC_amp_trunc[ntrunc] = itfdc->getDxAmp();
           tra->ddEdx_FDC_trunc[ntrunc] = itfdc->getDEdx();
           tra->ddEdx_FDC_amp_trunc[ntrunc] = itfdc->getDEdxAmp();
         }
      }
      else {
         tra->dNumHitsUsedFordEdx_FDC = 0;
         tra->dNumHitsUsedFordEdx_CDC = 0;
         tra->ddEdx_FDC = 0.0;
         tra->ddEdx_CDC = 0.0;
         tra->ddx_FDC = 0.0;
         tra->ddx_CDC = 0.0; 
	 tra->ddEdx_CDC_amp = 0.0;
         tra->ddx_CDC_amp = 0.0;
      }

      data.push_back(tra);
   }
   
   if( PRUNE_DUPLICATE_TRACKS && (data.size() > 1) ) {
   		vector< int > indices_to_erase;
   		
   		 for( unsigned int i=0; i<data.size()-1; i++ ) {
  			for( unsigned int j=i+1; j<data.size(); j++ ) {
				if(find(indices_to_erase.begin(), indices_to_erase.end(), j) != indices_to_erase.end())
					continue;
					
				// look through the remaining tracks for duplicates
   				// (1) if there is a track with the same candidate/PID and worse chi^2, reject that track
   				// (2) if there is a track with the same candidate/PID and better chi^2, reject this track
				if( (data[i]->candidateid == data[j]->candidateid) 
					&& (data[i]->PID() == data[j]->PID()) ) {  // is a duplicate track
					if(data[i]->chisq < data[j]->chisq) {
						indices_to_erase.push_back(j);
					 } else	{	
						indices_to_erase.push_back(i);	
					}	
				}
			}
   		}
		
		// create the new set of tracks
		vector<DTrackTimeBased*> new_data;
		for( unsigned int i=0; i<data.size(); i++ ) {
			if(find(indices_to_erase.begin(), indices_to_erase.end(), i) != indices_to_erase.end())
				continue;

			new_data.push_back(data[i]);
		}
		data = new_data;   // replace the set of tracks with the pruned one
   }

   // Copy into factory
   factory->Set(data);

   return true; //NOERROR;
}

//--------------------------------
// Extract_DDetectorMatches
//--------------------------------
bool DEventSourceREST::Extract_DDetectorMatches(const std::shared_ptr<const JEvent>& locEvent, hddm_r::HDDM *record, JFactoryT<DDetectorMatches>* factory)
{
   /// Copies the data from the detectorMatches hddm record. This is
   /// called from JEventSourceREST::GetObjects. If factory is NULL, this
   /// returns OBJECT_NOT_AVAILABLE immediately.

   if(factory==NULL)
     return false; //OBJECT_NOT_AVAILABLE

   string tag = factory->GetTag();
   vector<DDetectorMatches*> data;

   vector<const DTrackTimeBased*> locTrackTimeBasedVector;
   locEvent->Get(locTrackTimeBasedVector);

   vector<const DSCHit*> locSCHits;
   locEvent->Get(locSCHits);

   vector<const DTOFPoint*> locTOFPoints;
   locEvent->Get(locTOFPoints); 

   vector<const DCTOFPoint*> locCTOFPoints;
   locEvent->Get(locCTOFPoints);

   vector<const DBCALShower*> locBCALShowers;
   locEvent->Get(locBCALShowers);

   vector<const DFCALShower*> locFCALShowers;
   locEvent->Get(locFCALShowers);

   const DParticleID* locParticleID = NULL;
   vector<const DDIRCPmtHit*> locDIRCHits;
   vector<const DDIRCTruthBarHit*> locDIRCBarHits;
   if(RECO_DIRC_CALC_LUT) {
	   locEvent->GetSingle(locParticleID);
	   locEvent->Get(locDIRCHits);
	   locEvent->Get(locDIRCBarHits);
   }

   const hddm_r::DetectorMatchesList &detectormatches = record->getDetectorMatcheses();

   // loop over chargedTrack records
   hddm_r::DetectorMatchesList::iterator iter;
   for(iter = detectormatches.begin(); iter != detectormatches.end(); ++iter)
	{
      if(iter->getJtag() != tag)
         continue;

      DDetectorMatches *locDetectorMatches = new DDetectorMatches();

      const hddm_r::DircMatchParamsList &dircList = iter->getDircMatchParamses(); 
      hddm_r::DircMatchParamsList::iterator dircIter = dircList.begin();
      const hddm_r::DircMatchHitList &dircMatchHitList = iter->getDircMatchHits(); 
      
      for(; dircIter != dircList.end(); ++dircIter)
      {
	      size_t locTrackIndex = dircIter->getTrack();
	      if(locTrackIndex >= locTrackTimeBasedVector.size()) continue;

	      auto locTrackTimeBased = locTrackTimeBasedVector[locTrackIndex];
	      if( !locTrackTimeBased ) continue;

	      auto locDIRCMatchParams = std::make_shared<DDIRCMatchParams>();
	      map<shared_ptr<const DDIRCMatchParams> ,vector<const DDIRCPmtHit*> > locDIRCTrackMatchParams;
	      locDetectorMatches->Get_DIRCTrackMatchParamsMap(locDIRCTrackMatchParams);

	      if(RECO_DIRC_CALC_LUT) {
		      TVector3 locProjPos(dircIter->getX(),dircIter->getY(),dircIter->getZ());
		      TVector3 locProjMom(dircIter->getPx(),dircIter->getPy(),dircIter->getPz());
		      double locFlightTime = dircIter->getT();

		      if( locParticleID->Get_DIRCLut()->CalcLUT(locProjPos, locProjMom, locDIRCHits, locFlightTime, locTrackTimeBased->mass(), locDIRCMatchParams, locDIRCBarHits, locDIRCTrackMatchParams) )
			  locDetectorMatches->Add_Match(locTrackTimeBasedVector[locTrackIndex], std::const_pointer_cast<const DDIRCMatchParams>(locDIRCMatchParams));
	      }
	      else {
		      // add hits to match list
		      hddm_r::DircMatchHitList::iterator dircMatchHitIter = dircMatchHitList.begin();
		      for(; dircMatchHitIter != dircMatchHitList.end(); ++dircMatchHitIter) {
			      size_t locMatchHitTrackIndex = dircMatchHitIter->getTrack();
			      if(locMatchHitTrackIndex == locTrackIndex) {
				      size_t locMatchHitIndex = dircMatchHitIter->getHit();
				      locDIRCTrackMatchParams[locDIRCMatchParams].push_back(locDIRCHits[locMatchHitIndex]);
			      }
		      }

		      locDIRCMatchParams->dExtrapolatedPos = DVector3(dircIter->getX(),dircIter->getY(),dircIter->getZ());
		      locDIRCMatchParams->dExtrapolatedMom = DVector3(dircIter->getPx(),dircIter->getPy(),dircIter->getPz());
		      locDIRCMatchParams->dExtrapolatedTime = dircIter->getT();
		      locDIRCMatchParams->dExpectedThetaC = dircIter->getExpectthetac();
		      locDIRCMatchParams->dThetaC = dircIter->getThetac();
		      locDIRCMatchParams->dDeltaT = dircIter->getDeltat();
		      locDIRCMatchParams->dLikelihoodElectron = dircIter->getLele();
		      locDIRCMatchParams->dLikelihoodPion = dircIter->getLpi();
		      locDIRCMatchParams->dLikelihoodKaon = dircIter->getLk();
		      locDIRCMatchParams->dLikelihoodProton = dircIter->getLp();
		      locDIRCMatchParams->dNPhotons = dircIter->getNphotons();
		      locDetectorMatches->Add_Match(locTrackTimeBasedVector[locTrackIndex], std::const_pointer_cast<const DDIRCMatchParams>(locDIRCMatchParams));
	      }
      }

      const hddm_r::BcalMatchParamsList &bcalList = iter->getBcalMatchParamses();
      hddm_r::BcalMatchParamsList::iterator bcalIter = bcalList.begin();
      for(; bcalIter != bcalList.end(); ++bcalIter)
      {
         size_t locShowerIndex = bcalIter->getShower();
         size_t locTrackIndex = bcalIter->getTrack();

         auto locShowerMatchParams = std::make_shared<DBCALShowerMatchParams>();
         locShowerMatchParams->dBCALShower = locBCALShowers[locShowerIndex];
         locShowerMatchParams->dx = bcalIter->getDx();
         locShowerMatchParams->dFlightTime = bcalIter->getTflight();
         locShowerMatchParams->dFlightTimeVariance = bcalIter->getTflightvar();
         locShowerMatchParams->dPathLength = bcalIter->getPathlength();
         locShowerMatchParams->dDeltaPhiToShower = bcalIter->getDeltaphi();
         locShowerMatchParams->dDeltaZToShower = bcalIter->getDeltaz();

         locDetectorMatches->Add_Match(locTrackTimeBasedVector[locTrackIndex], locBCALShowers[locShowerIndex], std::const_pointer_cast<const DBCALShowerMatchParams>(locShowerMatchParams));
      }

      const hddm_r::FcalMatchParamsList &fcalList = iter->getFcalMatchParamses();
      hddm_r::FcalMatchParamsList::iterator fcalIter = fcalList.begin();
      for(; fcalIter != fcalList.end(); ++fcalIter)
      {
         size_t locShowerIndex = fcalIter->getShower();
         size_t locTrackIndex = fcalIter->getTrack();

         auto locShowerMatchParams = std::make_shared<DFCALShowerMatchParams>();
         locShowerMatchParams->dFCALShower = locFCALShowers[locShowerIndex];
         locShowerMatchParams->dx = fcalIter->getDx();
         locShowerMatchParams->dFlightTime = fcalIter->getTflight();
         locShowerMatchParams->dFlightTimeVariance = fcalIter->getTflightvar();
         locShowerMatchParams->dPathLength = fcalIter->getPathlength();
         locShowerMatchParams->dDOCAToShower = fcalIter->getDoca();
	 locShowerMatchParams->dEcenter=0.;
	 locShowerMatchParams->dE3x3=0.;
	 locShowerMatchParams->dE5x5=0.;
	 const hddm_r::FcalEnergyParamsList &fcalEnergyList = fcalIter->getFcalEnergyParamses();
	 hddm_r::FcalEnergyParamsList::iterator fcalEnergyIter = fcalEnergyList.begin();
	 for(; fcalEnergyIter != fcalEnergyList.end(); ++fcalEnergyIter){
	   locShowerMatchParams->dEcenter=fcalEnergyIter->getEcenter();
	   locShowerMatchParams->dE3x3=fcalEnergyIter->getE3x3();
	   locShowerMatchParams->dE5x5=fcalEnergyIter->getE5x5();
	 }
         locDetectorMatches->Add_Match(locTrackTimeBasedVector[locTrackIndex], locFCALShowers[locShowerIndex], std::const_pointer_cast<const DFCALShowerMatchParams>(locShowerMatchParams));
      }

      const hddm_r::FcalSingleHitMatchParamsList &fcalSingleHitList = iter->getFcalSingleHitMatchParamses();
      hddm_r::FcalSingleHitMatchParamsList::iterator fcalSingleHitIter = fcalSingleHitList.begin();
      for(; fcalSingleHitIter != fcalSingleHitList.end(); ++fcalSingleHitIter)
      {
         size_t locTrackIndex = fcalSingleHitIter->getTrack();

         auto locSingleHitMatchParams = std::make_shared<DFCALSingleHitMatchParams>();
         locSingleHitMatchParams->dEHit = fcalSingleHitIter->getEhit();
	 locSingleHitMatchParams->dTHit = fcalSingleHitIter->getThit();
         locSingleHitMatchParams->dx = fcalSingleHitIter->getDx();
         locSingleHitMatchParams->dFlightTime = fcalSingleHitIter->getTflight();
         locSingleHitMatchParams->dFlightTimeVariance = fcalSingleHitIter->getTflightvar();
         locSingleHitMatchParams->dPathLength = fcalSingleHitIter->getPathlength();
         locSingleHitMatchParams->dDOCAToHit = fcalSingleHitIter->getDoca();

         locDetectorMatches->Add_Match(locTrackTimeBasedVector[locTrackIndex],std::const_pointer_cast<const DFCALSingleHitMatchParams>(locSingleHitMatchParams));
      }


      const hddm_r::ScMatchParamsList &scList = iter->getScMatchParamses();
      hddm_r::ScMatchParamsList::iterator scIter = scList.begin();
      for(; scIter != scList.end(); ++scIter)
      {
         size_t locHitIndex = scIter->getHit();
         size_t locTrackIndex = scIter->getTrack();

         auto locSCHitMatchParams = std::make_shared<DSCHitMatchParams>();
         locSCHitMatchParams->dSCHit = locSCHits[locHitIndex];
         locSCHitMatchParams->dEdx = scIter->getDEdx();
         locSCHitMatchParams->dHitTime = scIter->getThit();
         locSCHitMatchParams->dHitTimeVariance = scIter->getThitvar();
         locSCHitMatchParams->dHitEnergy = scIter->getEhit();
         locSCHitMatchParams->dFlightTime = scIter->getTflight();
         locSCHitMatchParams->dFlightTimeVariance = scIter->getTflightvar();
         locSCHitMatchParams->dPathLength = scIter->getPathlength();
         locSCHitMatchParams->dDeltaPhiToHit = scIter->getDeltaphi();

         locDetectorMatches->Add_Match(locTrackTimeBasedVector[locTrackIndex], locSCHits[locHitIndex], std::const_pointer_cast<const DSCHitMatchParams>(locSCHitMatchParams));
      }

      const hddm_r::TofMatchParamsList &tofList = iter->getTofMatchParamses();
      hddm_r::TofMatchParamsList::iterator tofIter = tofList.begin();
      for(; tofIter != tofList.end(); ++tofIter)
      {
         size_t locHitIndex = tofIter->getHit();
         size_t locTrackIndex = tofIter->getTrack();

         auto locTOFHitMatchParams = std::make_shared<DTOFHitMatchParams>();
         locTOFHitMatchParams->dTOFPoint = locTOFPoints[locHitIndex];

         locTOFHitMatchParams->dHitTime = tofIter->getThit();
         locTOFHitMatchParams->dHitTimeVariance = tofIter->getThitvar();
         locTOFHitMatchParams->dHitEnergy = tofIter->getEhit();

         locTOFHitMatchParams->dEdx = tofIter->getDEdx();
         locTOFHitMatchParams->dFlightTime = tofIter->getTflight();
         locTOFHitMatchParams->dFlightTimeVariance = tofIter->getTflightvar();
         locTOFHitMatchParams->dPathLength = tofIter->getPathlength();
         locTOFHitMatchParams->dDeltaXToHit = tofIter->getDeltax();
         locTOFHitMatchParams->dDeltaYToHit = tofIter->getDeltay();

         locDetectorMatches->Add_Match(locTrackTimeBasedVector[locTrackIndex], locTOFPoints[locHitIndex], std::const_pointer_cast<const DTOFHitMatchParams>(locTOFHitMatchParams));
	 
	 // dE/dx per plane
	 const hddm_r::TofDedxList& locTofDedxList = tofIter->getTofDedxs();
	 hddm_r::TofDedxList::iterator locTofDedxIterator = locTofDedxList.begin();
	 if(locTofDedxIterator == locTofDedxList.end())
	   {
	     locTOFHitMatchParams->dEdx1 = 0.;
	     locTOFHitMatchParams->dEdx2 = 0.;
	   }
	 else //should only be 1
	   {
	     for(; locTofDedxIterator != locTofDedxList.end(); ++locTofDedxIterator)
	       {
		 locTOFHitMatchParams->dEdx1 = locTofDedxIterator->getDEdx1();
		 locTOFHitMatchParams->dEdx2 = locTofDedxIterator->getDEdx2();

		 // check if already have average dE/dx
		 if(locTOFHitMatchParams->dEdx > 0) continue;
		 
		 // average dE/dx is missing: take average if hits in both planes, otherwise use single plane value
		 if(locTOFHitMatchParams->dEdx1>0 && locTOFHitMatchParams->dEdx2>0)
			 locTOFHitMatchParams->dEdx = (locTOFHitMatchParams->dEdx1 + locTOFHitMatchParams->dEdx2) / 2.0; 
		 else if(locTOFHitMatchParams->dEdx1>0)
			 locTOFHitMatchParams->dEdx = locTOFHitMatchParams->dEdx1;
		 else if(locTOFHitMatchParams->dEdx2>0) 
			 locTOFHitMatchParams->dEdx = locTOFHitMatchParams->dEdx2;
	       }
	   }	 
      }

      // Extract track matching data for FMPWCs
      const hddm_r::FmwpcMatchParamsList &fmwpcList = iter->getFmwpcMatchParamses();
      hddm_r::FmwpcMatchParamsList::iterator fmwpcIter = fmwpcList.begin();
      for(; fmwpcIter != fmwpcList.end(); ++fmwpcIter)
      {
         size_t locTrackIndex = fmwpcIter->getTrack();
	 const hddm_r::FmwpcDataList &fmwpcDataList = fmwpcIter->getFmwpcDatas();
	 hddm_r::FmwpcDataList::iterator fmwpcDataIter = fmwpcDataList.begin();

         auto locFMWPCMatchParams = std::make_shared<DFMWPCMatchParams>();
	 for(; fmwpcDataIter != fmwpcDataList.end(); ++fmwpcDataIter){
	   locFMWPCMatchParams->dLayers.push_back(fmwpcDataIter->getLayer());
	   locFMWPCMatchParams->dNhits.push_back(fmwpcDataIter->getNhits());
	   locFMWPCMatchParams->dDists.push_back(fmwpcDataIter->getDist());
	   locFMWPCMatchParams->dClosestWires.push_back(fmwpcDataIter->getClosestwire());
	 }
	 locDetectorMatches->Add_Match(locTrackTimeBasedVector[locTrackIndex], std::const_pointer_cast<const DFMWPCMatchParams>(locFMWPCMatchParams));
      }

      // Extract track matching data for CTOF
      const hddm_r::CtofMatchParamsList &ctofList = iter->getCtofMatchParamses();
      hddm_r::CtofMatchParamsList::iterator ctofIter = ctofList.begin();
      for(; ctofIter != ctofList.end(); ++ctofIter)
      {
         size_t locHitIndex = ctofIter->getHit();
         size_t locTrackIndex = ctofIter->getTrack();

         auto locCTOFHitMatchParams = std::make_shared<DCTOFHitMatchParams>();
         locCTOFHitMatchParams->dCTOFPoint = locCTOFPoints[locHitIndex];
         locCTOFHitMatchParams->dEdx = ctofIter->getDEdx();
         locCTOFHitMatchParams->dFlightTime = ctofIter->getTflight();
         locCTOFHitMatchParams->dDeltaXToHit = ctofIter->getDeltax();
         locCTOFHitMatchParams->dDeltaYToHit = ctofIter->getDeltay();

         locDetectorMatches->Add_Match(locTrackTimeBasedVector[locTrackIndex], locCTOFPoints[locHitIndex], std::const_pointer_cast<const DCTOFHitMatchParams>(locCTOFHitMatchParams));
      }

      const hddm_r::BcalDOCAtoTrackList &bcaldocaList = iter->getBcalDOCAtoTracks();
      hddm_r::BcalDOCAtoTrackList::iterator bcaldocaIter = bcaldocaList.begin();
      for(; bcaldocaIter != bcaldocaList.end(); ++bcaldocaIter)
      {
         size_t locShowerIndex = bcaldocaIter->getShower();
         double locDeltaPhi = bcaldocaIter->getDeltaphi();
         double locDeltaZ = bcaldocaIter->getDeltaz();
         locDetectorMatches->Set_DistanceToNearestTrack(locBCALShowers[locShowerIndex], locDeltaPhi, locDeltaZ);
      }

      const hddm_r::FcalDOCAtoTrackList &fcaldocaList = iter->getFcalDOCAtoTracks();
      hddm_r::FcalDOCAtoTrackList::iterator fcaldocaIter = fcaldocaList.begin();
      for(; fcaldocaIter != fcaldocaList.end(); ++fcaldocaIter)
      {
         size_t locShowerIndex = fcaldocaIter->getShower();
         double locDOCA = fcaldocaIter->getDoca();
         locDetectorMatches->Set_DistanceToNearestTrack(locFCALShowers[locShowerIndex], locDOCA);
      }

      const hddm_r::TflightPCorrelationList &correlationList = iter->getTflightPCorrelations();
      hddm_r::TflightPCorrelationList::iterator correlationIter = correlationList.begin();
      for(; correlationIter != correlationList.end(); ++correlationIter)
      {
         size_t locTrackIndex = correlationIter->getTrack();
         DetectorSystem_t locDetectorSystem = (DetectorSystem_t)correlationIter->getSystem();
         double locCorrelation = correlationIter->getCorrelation();
         locDetectorMatches->Set_FlightTimePCorrelation(locTrackTimeBasedVector[locTrackIndex], locDetectorSystem, locCorrelation);
      }

      data.push_back(locDetectorMatches);
   }

   // Copy data to factory
   factory->Set(data);

   return true; //NOERROR;
}

// Transform the 5x5 tracking error matrix into a 7x7 error matrix in cartesian
// coordinates.
// This was copied and transformed from DKinFit.cc
void DEventSourceREST::Get7x7ErrorMatrix(double mass, const double vec[5], const TMatrixFSym* C5x5, TMatrixFSym* loc7x7ErrorMatrix)
{
  TMatrixF J(7,5);

  // State vector
  double q_over_pt=vec[0];
  double phi=vec[1];
  double tanl=vec[2];
  double D=vec[3];

  double pt=1./fabs(q_over_pt);
  double pt_sq=pt*pt;
  double cosphi=cos(phi);
  double sinphi=sin(phi);
  double q=(q_over_pt>0)?1.:-1.;

  J(0, 0)=-q*pt_sq*cosphi;
  J(0, 1)=-pt*sinphi;
  
  J(1, 0)=-q*pt_sq*sinphi;
  J(1, 1)=pt*cosphi;
  
  J(2, 0)=-q*pt_sq*tanl;
  J(2, 2)=pt;
  
  J(3, 1)=-D*cosphi;
  J(3, 3)=-sinphi;
  
  J(4, 1)=-D*sinphi;
  J(4, 3)=cosphi;
  
  J(5, 4)=1.;

  // C'= JCJ^T
  TMatrixFSym locTempMatrix(*C5x5);
  *loc7x7ErrorMatrix=locTempMatrix.Similarity(J);
}

uint32_t DEventSourceREST::Convert_SignedIntToUnsigned(int32_t locSignedInt) const
{
	//Convert uint32_t to int32_t
	//Reverse scheme (from DEventWriterREST):
		//If is >= 0, then the int32_t is the same as the uint32_t (last bit not set)
		//If is the minimum int: bit 32 is 1, and all of the other bits are zero
		//Else, bit 32 is 1, then the uint32_t is -1 * int32_t, + add the last bit
	if(locSignedInt >= 0)
		return uint32_t(locSignedInt); //bit 32 is zero
	else if(locSignedInt == numeric_limits<int32_t>::min()) // -(2^31)
		return uint32_t(0x80000000); //bit 32 is 1, all others are 0
	return uint32_t(-1*locSignedInt) + uint32_t(0x80000000); //bit 32 is 1, all others are negative of signed int (which was negative)
}

//-----------------------
// Extract_DDIRCPmtHit
//-----------------------
bool DEventSourceREST::Extract_DDIRCPmtHit(hddm_r::HDDM *record,
                                   JFactoryT<DDIRCPmtHit>* factory, const std::shared_ptr<const JEvent>& locEvent)
{
   /// Copies the data from the fcalShower hddm record. This is
   /// call from JEventSourceREST::GetObjects. If factory is NULL, this
   /// returns OBJECT_NOT_AVAILABLE immediately.

   if (factory==NULL) {
      return false; //OBJECT_NOT_AVAILABLE
   }
   string tag = factory->GetTag();

   vector<DDIRCPmtHit*> data;

   // loop over fcal shower records
   const hddm_r::DircHitList &hits =
                 record->getDircHits();
   hddm_r::DircHitList::iterator iter;
   for (iter = hits.begin(); iter != hits.end(); ++iter) {
      if (iter->getJtag() != tag)
         continue;

      // throw away hits from bad or noisy channels (after REST reconstruction)
      int locRunNumber = locEvent->GetRunNumber();
      int box = (iter->getCh() < dDIRCMaxChannels) ? 1 : 0;
      int channel = iter->getCh() % dDIRCMaxChannels;
      dirc_status_state status = static_cast<dirc_status_state>(dDIRCChannelStatusMap[locRunNumber][box][channel]);
      if ( (status==BAD) || (status==NOISY) ) {
	      continue;
      }

      DDIRCPmtHit *hit = new DDIRCPmtHit();
      hit->setChannel(iter->getCh());
      hit->setTime(iter->getT());
      hit->setTOT(iter->getTot());

      data.push_back(hit);
   }

   // Copy into factory
   factory->Set(data);
   
   return true; //NOERROR;
}

//-----------------------
// Extract_DFMWPCHit
//-----------------------
bool DEventSourceREST::Extract_DFMWPCHit(hddm_r::HDDM *record,
                    JFactoryT<DFMWPCHit>* factory, const std::shared_ptr<const JEvent>& locEvent)
{
   /// Copies the data from the fmwpc hit hddm record. This is
   /// call from JEventSourceREST::GetObjects. If factory is NULL, this
   /// returns OBJECT_NOT_AVAILABLE immediately.

   if (factory==NULL) {
      return false; //OBJECT_NOT_AVAILABLE
   }
   string tag = factory->GetTag();

   vector<DFMWPCHit*> data;

   // loop over data hit records
   const hddm_r::FmwpcHitList &hits =
                 record->getFmwpcHits();
   hddm_r::FmwpcHitList::iterator iter;
   for (iter = hits.begin(); iter != hits.end(); ++iter) {
      if (iter->getJtag() != tag)
         continue;

      DFMWPCHit *hit = new DFMWPCHit();
      hit->layer = iter->getLayer();
      hit->wire = iter->getWire();
      hit->q = iter->getQ();
      hit->amp = iter->getAmp();
      hit->t = iter->getT();
      hit->QF = iter->getQf();
      hit->ped = iter->getPed();

      data.push_back(hit);
   }
   // Copy into factory
   factory->Set(data);

   return true; //NOERROR;
}

//-----------------------
// Extract_DFCALHit
//-----------------------
bool DEventSourceREST::Extract_DFCALHit(hddm_r::HDDM *record,
                                   JFactoryT<DFCALHit>* factory, const std::shared_ptr<const JEvent>& locEvent)
{
   /// Copies the data from the fcal hit hddm record. This is
   /// call from JEventSourceREST::GetObjects. If factory is NULL, this
   /// returns OBJECT_NOT_AVAILABLE immediately.

   if (factory==NULL) {
      return false; //OBJECT_NOT_AVAILABLE
   }
   string tag = factory->GetTag();
   vector<DFCALHit*> data;

   // loop over fmwpc hit records
   const hddm_r::FcalHitList &hits =
                 record->getFcalHits();
   hddm_r::FcalHitList::iterator iter;
   for (iter = hits.begin(); iter != hits.end(); ++iter) {
      if (iter->getJtag() != tag)
         continue;

      DFCALHit *hit = new DFCALHit();
      hit->row = iter->getRow();
      hit->column = iter->getColumn();
      hit->x = iter->getX();
      hit->y = iter->getY();
      hit->E = iter->getE();
      hit->t = iter->getT();
      hit->intOverPeak = iter->getIntOverPeak();

      data.push_back(hit);
   }

   // Copy into factory
   factory->Set(data);

   return true; //NOERROR;
}

//----------------------------
// Extract_DEventHitStatistics
//----------------------------
bool DEventSourceREST::Extract_DEventHitStatistics(hddm_r::HDDM *record,
                                   JFactoryT<DEventHitStatistics>* factory)
{
   /// Copies the data from the hitStatistics hddm record. This is
   /// call from JEventSourceREST::GetObjects. If factory is NULL, this
   /// returns OBJECT_NOT_AVAILABLE immediately.

   if (factory==NULL) {
      return false; //OBJECT_NOT_AVAILABLE
   }
   string tag = factory->GetTag();

   vector<DEventHitStatistics*> data;

   hddm_r::HitStatisticsList slist = (hddm_r::HitStatisticsList)record->getHitStatisticses();
   if (slist.size() != 0 && slist().getJtag() == tag) {
      DEventHitStatistics *stats = new DEventHitStatistics;
      hddm_r::StartCountersList starts = slist().getStartCounterses();
      stats->start_counters = (starts.size() > 0)? starts().getCount() : 0;
      hddm_r::CdcStrawsList straws = slist().getCdcStrawses();
      stats->cdc_straws = (straws.size() > 0)? straws().getCount() : 0;
      hddm_r::FdcPseudosList pseudos = slist().getFdcPseudoses();
      stats->fdc_pseudos = (pseudos.size() > 0)? pseudos().getCount() : 0;
      hddm_r::BcalCellsList cells = slist().getBcalCellses();
      stats->bcal_cells = (cells.size() > 0)? cells().getCount() : 0;
      hddm_r::FcalBlocksList blocks = slist().getFcalBlockses();
      stats->fcal_blocks = (blocks.size() > 0)? blocks().getCount() : 0;
      hddm_r::CcalBlocksList bloccs = slist().getCcalBlockses();
      stats->ccal_blocks = (bloccs.size() > 0)? bloccs().getCount() : 0;
      hddm_r::TofPaddlesList paddles = slist().getTofPaddleses();
      stats->tof_paddles = (paddles.size() > 0)? paddles().getCount() : 0;
      hddm_r::DircPMTsList pmts = slist().getDircPMTses();
      stats->dirc_PMTs = (pmts.size() > 0)? pmts().getCount() : 0;

      data.push_back(stats);
   }

   // Copy into factory
   factory->Set(data);
   
   return true; //NOERROR;
}
