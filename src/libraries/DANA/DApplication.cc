// $Id$
//
//    File: DApplication.cc
// Created: Mon Jul  3 21:46:01 EDT 2006
// Creator: davidl (on Darwin Harriet.local 8.6.0 powerpc)
//


#include <iostream>
#include <fstream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "TF1.h"

using std::string;

#include <HDDM/DEventSourceHDDMGenerator.h>
#include <HDDM/DEventSourceRESTGenerator.h>
#include <EVENTSTORE/DEventSourceEventStoreGenerator.h>
#include <DAQ/JEventSourceGenerator_EVIO.h>
#include <DAQ/JEventSourceGenerator_EVIOpp.h>

#include "DANARootErrorHandler.h"
#include "DStatusBits.h"
#include "DFactoryGenerator.h"

#include <DANA/DApplication.h>
#include <DANA/DGeometryManager.h>
#include <JANA/Compatibility/JLockService.h>
#include <JANA/Calibrations/JCalibrationManager.h>
#include <JANA/Calibrations/JCalibrationGeneratorCCDB.h>
#include <JANA/Compatibility/JGeometryManager.h>
#include <JANA/CLI/JMain.h>

/// The DApplication class adds HALL-D specific event source and factory generators to a JApplication
/// factory generators that are HAll-D specific.


DApplication::DApplication(int argc, char* argv[]) {
	m_options = jana::ParseCommandLineOptions(argc, argv);
	m_japp = jana::CreateJApplication(m_options);
	japp = m_japp;
	InitHallDLibraries(m_japp);
}

DApplication::DApplication(JApplication *app) {
	m_japp = new JApplication();
	InitHallDLibraries(m_japp);
}

void DApplication::RootUnLock() {
	if (m_rootlock == nullptr) {
		m_rootlock = m_japp->GetService<JLockService>();
	}
	m_rootlock->RootUnLock();
}

void DApplication::RootWriteLock() {
	if (m_rootlock == nullptr) {
		m_rootlock = m_japp->GetService<JLockService>();
	}
	m_rootlock->RootWriteLock();
}

DMagneticFieldMap* DApplication::GetBfield(uint32_t run_number) {
	return m_japp->GetService<DGeometryManager>()->GetBfield(run_number);
}

void DApplication::InitHallDLibraries(JApplication* app) {

	// Add services
	app->ProvideService(std::make_shared<JLockService>());
	auto calib_man = make_shared<JCalibrationManager>();
	calib_man->AddCalibrationGenerator(new JCalibrationGeneratorCCDB);
	app->ProvideService(calib_man);
	app->ProvideService(std::make_shared<DGeometryManager>(app));
	app->ProvideService(std::make_shared<JGeometryManager>());

	// Disable inherently (and horrorifically)-unsafe registration of EVERY TObject with the global TObjectTable //multithreading!!
	// Simply setting/checking a bool is not thread-safe due to cache non-coherence and operation re-shuffling by the compiler
	TObject::SetObjectStat(kFALSE);
	//Similar: This adds functions to the global gROOT.  We don't want this, because different threads tend to have their own functions with the same name.
	TF1::DefaultAddToGlobalList(kFALSE);

	// Add plugin paths to Hall-D specific binary directories
	const char *bms_osname = getenv("BMS_OSNAME");
	string sbms_osname(bms_osname == NULL ? "" : bms_osname);

	if (const char *ptr = getenv("DANA_PLUGIN_PATH")) {
		app->AddPluginPath(string(ptr));
	}
	if (const char *ptr = getenv("HALLD_MY")) {
		app->AddPluginPath(string(ptr) + "/" + sbms_osname + "/plugins");  // SBMS
		app->AddPluginPath(string(ptr) + "/lib/" + sbms_osname);     // BMS
	}
	if (const char *ptr = getenv("HALLD_RECON_HOME")) {
		app->AddPluginPath(string(ptr) + "/" + sbms_osname + "/plugins");  // SBMS
		app->AddPluginPath(string(ptr) + "/lib/" + sbms_osname);     // BMS
	}

	// Since we defer reading in some tables until they are requested (likely while processing the first event)
	// that time gets counted against the thread as being non-reponsive. The default timeout of 8 seconds is
	// therefore too small. Change it to 30 here, unless the user has set it explicitly on the command line.

	// TODO: NWB: Verify this does the same thing as the old version
	string thread_timeout; // = "30 seconds";
	auto thread_timeout_param = app->GetParameter("THREAD_TIMEOUT", thread_timeout);
	if (thread_timeout_param && thread_timeout_param->IsDefault()) {
		app->SetParameterValue("THREAD_TIMEOUT", "30 seconds");
	}

	CheckCpuSimdSupport();

	// Optionally copy SQLite CCDB file to local disk
	CopySQLiteToLocalDisk(app);

	/// Add DEventSourceHDDMGenerator and DFactoryGenerator, which adds the default list of Hall-D factories
	bool HDDM_ENABLE = true;
	app->SetDefaultParameter("HDDM:ENABLE", HDDM_ENABLE,
	                         "Enable the HDDM source readers. If set to 0, input files are assumed to never be of an HDDM format.");
	if (HDDM_ENABLE) {
		event_source_generator = new DEventSourceHDDMGenerator();
		app->Add(event_source_generator);
		app->Add(new DEventSourceRESTGenerator());
		app->Add(new JEventSourceGenerator_EVIOpp());
		app->Add(new JEventSourceGenerator_EVIO());
		app->Add(new DEventSourceEventStoreGenerator());
	}
	factory_generator = new DFactoryGenerator();
	app->Add(factory_generator);
	// Only check SSE capabilities if we're going to use the variables
	// below so as to avoid compiler warnings.

	// Install our own error handler for ROOT message
	int ROOT_ERROR_LEVEL_SUPRESS = 10000;
	app->SetDefaultParameter("ROOT_ERROR_LEVEL_SUPRESS", ROOT_ERROR_LEVEL_SUPRESS);
	InitDANARootErrorHandler(ROOT_ERROR_LEVEL_SUPRESS);

	// Define base set of status bits
	DStatusBits::SetStatusBitDescriptions();
}


//---------------------------------
// CheckCpuSimdSupport
//---------------------------------
void DApplication::CheckCpuSimdSupport() {
#if USE_SIMD || USE_SSE2 || USE_SSE3
	// Check if running on a cpu that supports the instruction set
	// extensions that were assumed when this application was built
	unsigned int cpeinfo;
	unsigned int cpsse3;
	//	unsigned int amdinfo;
	asm("mov $0x01, %%eax\ncpuid\n"
	  : "=d" (cpeinfo), "=c" (cpsse3)
	);
	// asm("mov $0x80000001, %%eax\ncpuid\n"
	//     : "=d" (amdinfo)
	// );
	//
	// int mmx = ((cpeinfo >> 23) & 0x1 );
	// int sse = ((cpeinfo >> 25) & 0x1 );
	// int sse2 = ((cpeinfo >> 26) & 0x1 );
	// int sse3 = ((cpsse3       ) & 0x1 );
	// int ssse3 = ((cpsse3 >>  9) & 0x1 );
	// int sse4_1 = ((cpsse3 >> 19) & 0x1 );
	// int sse4_2 = ((cpsse3 >> 20) & 0x1 );
	// int sse4a = ((amdinfo >>  6) & 0x1 );
#endif // USE_SIMD || USE_SSE2 || USE_SSE3

#if USE_SIMD
	int sse = ((cpeinfo >> 25) & 0x1 );
	if (sse == 0) {
		jerr<<"DApplication::Init error - application was built to run only on machines" << endl
			<<"supporting the SSE processor extensions. Please run on a processor that" << endl
			<<"supports SSE, or rebuild with DISABLE_SIMD=yes. " << endl;
		exit(-1);
	}
#endif

#if USE_SSE2
	int sse2 = ((cpeinfo >> 26) & 0x1 );
	if (sse2 == 0) {
		jerr<<"DApplication::Init error - application was built to run only on machines" << endl
			<<"supporting the SSE2 processor extensions. Please run on a processor that" << endl
			<<"supports SSE2, or rebuild with DISABLE_SSE2=yes." << endl;
		exit(-1);
	}
#endif

#if USE_SSE3
	int sse3 = ((cpsse3       ) & 0x1 );
	if (sse3 == 0) {
		jerr<<"DApplication::Init error - application was built to run only on machines" << endl
			<<"supporting the SSE3 processor extensions. Please run on a processor that" << endl
			<<"supports SSE3, or rebuild with DISABLE_SSE3=yes."
			<< endl;
		exit(-1);
	}
#endif

}


//---------------------------------
// CopySQLiteToLocalDisk
//---------------------------------
void DApplication::CopySQLiteToLocalDisk(JApplication *app) {
	// Check if parameters are set and consistent
	JParameterManager *pm = app->GetJParameterManager();
	if (!pm->Exists("SQLITE_TO_LOCAL")) return;

	string tmp;
	pm->SetDefaultParameter("SQLITE_TO_LOCAL", tmp);

	string JANA_CALIB_URL;
	if (pm->Exists("JANA_CALIB_URL")) {
		pm->GetParameter("JANA_CALIB_URL", JANA_CALIB_URL);
	} else {
		auto url_env = getenv("JANA_CALIB_URL");
		if (url_env != NULL) JANA_CALIB_URL = url_env;
	}
	if (JANA_CALIB_URL.empty()) return;

	if (JANA_CALIB_URL.find("sqlite://") != 0) {
		jout << "WARNING: SQLITE_TO_LOCAL specified but JANA_CALIB_URL does not specify a" << endl;
		jout << "         SQLite source. SQLITE_TO_LOCAL will be ignored." << endl;
		return;
	}

	// Get destination and source file names
	string dest;
	pm->GetParameter("SQLITE_TO_LOCAL", dest);
	string src = JANA_CALIB_URL.substr(10);

	// Check if source exists
	if (access(src.c_str(), R_OK) == -1) {
		jout << "WARNING: SQLITE_TO_LOCAL specified but sqlite file specified in JANA_CALIB_URL:" << endl;
		jout << "         " << src << "  does not exist. SQLITE_TO_LOCAL will be ignored." << endl;
		return;
	}

	// Check if destination exists
	bool copy_sqlite_file = true;
	if (access(dest.c_str(), R_OK) != -1) {

		// Check if file sizes are identical
		ifstream ifsrc(src.c_str());
		ifsrc.seekg(0, ifsrc.end);
		auto size_src = ifsrc.tellg();
		ifsrc.close();
		ifstream ifdest(dest.c_str());
		ifdest.seekg(0, ifdest.end);
		auto size_dest = ifdest.tellg();
		ifdest.close();

		if (size_src == size_dest) {

			// File sizes are identical. Check if source is also older than dest.
			struct stat src_result;
			struct stat dest_result;
			stat(src.c_str(), &src_result);
			stat(dest.c_str(), &dest_result);
			if (src_result.st_mtime < dest_result.st_mtime) {
				jout << "Modification time of " << src << " is older than " << dest << endl;
				jout << "so file will not be copied" << endl;
				copy_sqlite_file = false;
			}
		}
	}

	// Copy sqlite file to specified location if needed
	if (copy_sqlite_file) {
		jout << "Copying " << src << " -> " << dest << endl;
		unlink(dest.c_str());
		std::ifstream ifs(src.c_str(), std::ios::binary);
		std::ofstream ofs(dest.c_str(), std::ios::binary);
		ofs << ifs.rdbuf();
	}

	// Overwrite the JANA_CALIB_URL config. parameter with the new destination
	JANA_CALIB_URL = "sqlite:///" + dest;
	jout << "Overwriting JANA_CALIB_URL with: " << JANA_CALIB_URL << endl;
	pm->SetParameter("JANA_CALIB_URL", JANA_CALIB_URL);
}

