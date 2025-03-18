
// Copyright 2020, Jefferson Science Associates, LLC.
// Subject to the terms in the LICENSE file found in the top-level directory.


#ifndef HALLD_RECON_DGEOMETRYMANAGER_H
#define HALLD_RECON_DGEOMETRYMANAGER_H

#include <JANA/JApplication.h>
#include <JANA/Services/JServiceLocator.h>

#include <mutex>

class DMagneticFieldMap;
class DLorentzDeflections;
class DGeometry;
class DRootGeom;
class DDIRCLutReader;

class DGeometryManager: public JService {

public:
	explicit DGeometryManager(JApplication* app);
	~DGeometryManager() override;

	DMagneticFieldMap* GetBfield(unsigned int run_number=1);
	DLorentzDeflections *GetLorentzDeflections(unsigned int run_number=1);
	DGeometry* GetDGeometry(unsigned int run_number);
	DRootGeom *GetRootGeom(unsigned int run_number);
	DDIRCLutReader *GetDIRCLut(unsigned int run_number);

private:
	JApplication* m_app;

	std::map<unsigned int, DMagneticFieldMap*> m_bfields;
	std::map<unsigned int, DLorentzDeflections*> m_lorentz_defs;
	std::map<unsigned int, DRootGeom*> m_root_geoms;
	std::map<unsigned int, DGeometry*> m_dgeometries;
	std::map<unsigned int, DDIRCLutReader*> m_dirclut_readers;

	std::mutex m_bfield_mutex;
	std::mutex m_lorentz_mutex;
	std::mutex m_rootgeoms_mutex;
	std::mutex m_dgeoms_mutex;
	std::mutex m_dirclut_mutex;
};


#endif //HALLD_RECON_DGEOMETRYMANAGER_H
