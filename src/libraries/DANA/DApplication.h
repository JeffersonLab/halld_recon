// $Id$
//
//    File: DApplication.h
// Created: Mon Jul  3 21:46:01 EDT 2006
// Creator: davidl (on Darwin Harriet.local 8.6.0 powerpc)
//

#ifndef _DApplication_
#define _DApplication_

#include <deque>
#include <map>
#include <set>
#include <algorithm>

#include "TF1.h"

#include <JANA/jerror.h>
#include <JANA/JApplication.h>
using namespace jana;
using namespace std;

#include "HDGEOMETRY/DGeometry.h"
#include "DIRC/DDIRCLutReader.h"

class DMagneticFieldMap;
class DLorentzDeflections;
class DGeometry;
//class DMaterialMap;
class DRootGeom;
class DDIRCLutReader;

class DApplication:public JApplication{

/// The DApplication class extends the JApplication class
/// by adding the default event source generators and
/// factory generators that are HAll-D specific.

	public:
		DApplication(int narg, char* argv[]);
		virtual ~DApplication();
		virtual const char* className(void){return static_className();}
		static const char* static_className(void){return "DApplication";}
		
		jerror_t Init(void);
		
		DMagneticFieldMap* GetBfield(unsigned int run_number=1);
		DLorentzDeflections *GetLorentzDeflections(unsigned int run_number=1);
		DGeometry* GetDGeometry(unsigned int run_number);
		DRootGeom *GetRootGeom(unsigned int run_number);
		void CopySQLiteToLocalDisk(void);
		DDIRCLutReader *GetDIRCLut(unsigned int run_number);
		
		pthread_rwlock_t* GetReadWriteLock(string &name) {
			return rw_locks.count( name ) == 0 ? nullptr : rw_locks[name];
		}
		pthread_rwlock_t* GetRootReadWriteLock() {
			return root_rw_lock;
		}
		pthread_rwlock_t* GetRootFillLock( JEventProcessor *proc ) {
			return root_fill_rw_lock.count( proc ) == 0 ? nullptr : root_fill_rw_lock[proc];
		}
		
		// Theses methods allow setting of the JApplication protected
		// members that are used to keep track of events read and processed.
		// This is done here since changing the JANA source would require
		// time to propagate whereas this is motivated by changes to
		// JEventSourceEVIOpp and can be propagated with those much easier.
		void SetNEventsRead( uint64_t newval ){ NEvents_read = newval; }
		void SetNEventsProcessed( uint64_t newval ){ NEvents = newval; }
		void AddNEventsRead( uint64_t newval ){ NEvents_read += newval; }
		void AddNEventsProcessed( uint64_t newval ){ NEvents += newval; }

	protected:
	
		DMagneticFieldMap *bfield;
		DLorentzDeflections *lorentz_def;
		JEventSourceGenerator *event_source_generator;
		JFactoryGenerator *factory_generator;
	 	DRootGeom *RootGeom;	
		vector<DGeometry*> geometries;
		DDIRCLutReader *dircLut;

		pthread_mutex_t mutex;
};

#endif // _DApplication_

