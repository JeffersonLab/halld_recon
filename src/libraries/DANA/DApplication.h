// $Id$
//
//    File: DApplication.h
// Created: Mon Jul  3 21:46:01 EDT 2006
// Creator: davidl (on Darwin Harriet.local 8.6.0 powerpc)
//

#ifndef _DApplication_
#define _DApplication_

#include <JANA/Compatibility/JLockService.h>
#include <JANA/CLI/JMain.h>

class JApplication;
class JEventSourceGenerator;
class JFactoryGenerator;
class DMagneticFieldMap;

class DApplication {

	JApplication* m_japp = nullptr;
	std::shared_ptr<JLockService> m_rootlock = nullptr;
	jana::UserOptions m_options;

	JEventSourceGenerator *event_source_generator;
	JFactoryGenerator *factory_generator;

	void InitHallDLibraries(JApplication* app);
	void CheckCpuSimdSupport();
	void CopySQLiteToLocalDisk(JApplication *app);

public:
	DApplication(int argc, char* argv[]);
	DApplication(JApplication *app);
	JApplication* GetJApp() { return m_japp; }
	jana::UserOptions& GetUserOptions() { return m_options; }
	void RootUnLock();
	void RootWriteLock();
	DMagneticFieldMap* GetBfield(uint32_t run_number);
};

#endif // _DApplication_

