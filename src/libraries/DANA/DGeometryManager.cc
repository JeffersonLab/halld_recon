
// Copyright 2020, Jefferson Science Associates, LLC.
// Subject to the terms in the LICENSE file found in the top-level directory.

#include "DGeometryManager.h"

#include <HDGEOMETRY/DMagneticFieldMapCalibDB.h>
#include <HDGEOMETRY/DMagneticFieldMapFineMesh.h>
#include <HDGEOMETRY/DMagneticFieldMapConst.h>
#include <HDGEOMETRY/DMagneticFieldMapNoField.h>
#include <HDGEOMETRY/DMagneticFieldMapSpoiled.h>
#include <HDGEOMETRY/DMagneticFieldMapParameterized.h>
#include <HDGEOMETRY/DLorentzMapCalibDB.h>
#include <HDGEOMETRY/DRootGeom.h>
#include <HDGEOMETRY/DGeometry.h>

#include <DIRC/DDIRCLutReader.h>

#include <JANA/Calibrations/JCalibrationManager.h>
#include <JANA/Compatibility/JGeometryManager.h>


DGeometryManager::DGeometryManager(JApplication* app) : m_app(app) {}


DGeometryManager::~DGeometryManager() {

	for (auto pair : m_bfields) {
		delete pair.second;
	}
	for (auto pair : m_lorentz_defs) {
		delete pair.second;
	}
	for (auto pair : m_root_geoms) {
		delete pair.second;
	}
	for (auto pair : m_dgeometries) {
		delete pair.second;
	}
	for (auto pair : m_dirclut_readers) {
		delete pair.second;
	}
}


DGeometry *DGeometryManager::GetDGeometry(unsigned int run_number) {
	/// Get the DGeometry object for the specified run number.
	/// The DGeometry class is Hall-D specific. It uses the
	/// JGeometry class from JANA to access values in the HDDS
	/// XML files. However, it supplies some useful and more
	/// user friendly methods for getting at some of the values.
	///
	/// This will first look for the DGeometry object in a list
	/// kept internal to DApplication and return a pointer to the
	/// object if found there. If it is not found there, then
	/// a new DGeometry object will be created and added to the
	/// internal list before returning a pointer to it.
	///
	/// Note that since this method can change internal data
	/// members, a mutex is locked to ensure integrity. This
	/// means that it is <b>NOT</b> efficient to call this
	/// method for every event. The pointer should be obtained
	/// in a brun() method and kept in a local variable if
	/// needed outside of brun().

	// At this point in time, only simulation exists with geometry coming
	// from a JGeometryXML object. The run range for these objects is
	// always set to include only the run number requested so if multiple
	// places in the code ask for different run numbers (as happens) a
	// second DGeometry object is created unecessarily. Here, we look to
	// see if a sole DGeometry object already exists and if so, if it is
	// built on a JGeometryFile object. If so, simply return it under the
	// assumption we are still doing development with simulated data and
	// a single set of geometry files.
	//
	// This isn't a good assumption anymore, disabling... [sdobbs, 1 June 2020]
	//
	//Lock();
	//if(geometries.size()==1 && string("JGeometryXML")==geometries[0]->GetJGeometry()->className()){
	//	Unlock();
	//	return geometries[0];
	//}
	//Unlock();

	// First, get the JGeometry object using our JApplication
	// base class. Then, use that to find the correct DGeometry
	// object if it exists.
	std::lock_guard<std::mutex> lock(m_dgeoms_mutex);
	auto it = m_dgeometries.find(run_number);
	if (it != m_dgeometries.end()) {
		return it->second;
	}

	JGeometry *jgeom = m_app->GetService<JGeometryManager>()->GetJGeometry(run_number);
	if(!jgeom){
		_DBG_<<"ERROR: Unable get geometry for run "<<run_number<<"!"<<endl;
		_DBG_<<"Make sure you JANA_GEOMETRY_URL environment variable is set."<<endl;
		_DBG_<<"It should be set to something like:"<<endl;
		_DBG_<<endl;
		_DBG_<<"    xmlfile://${HALLD_RECON_HOME}/src/programs/Simulation/hdds/main_HDDS.xml"<<endl;
		_DBG_<<endl;
		_DBG_<<"Exiting now."<<endl;
		m_app->Quit();
		exit(-1);
	}

	jout<<"Creating DGeometry:"<<endl;
	jout<<"  Run requested:"<<jgeom->GetRunRequested()<<"  found:"<<jgeom->GetRunFound()<<endl;
	jout<<"  Run validity range: "<<jgeom->GetRunMin()<<"-"<<jgeom->GetRunMax()<<endl;
	jout<<"  URL=\""<<jgeom->GetURL()<<"\""<<"  context=\""<<jgeom->GetContext()<<"\""<<endl;
	jout<<"  Type=\""<<jgeom->className()<<"\""<<endl;

	// Couldn't find a DGeometry object that uses this JGeometry object.
	// Create one and add it to the list.
	DGeometry *dgeom = new DGeometry(jgeom, this, m_app, run_number);

	m_dgeometries[run_number] = dgeom;
	return dgeom;
}


DMagneticFieldMap *DGeometryManager::GetBfield(unsigned int run_number) {
	const char *ccdb_help =
			" \n"
			" Could not load the solenoid field map from the CCDB!\n"
			" Please specify the solenoid field map to use on the command line, e.g.:\n"
			" \n"
			"   -PBFIELD_MAP=Magnets/Solenoid/solenoid_1200A_poisson_20140520\n"
			" or\n"
			"   -PBFIELD_TYPE=NoField\n";


	std::lock_guard<std::mutex> lock(m_bfield_mutex);

	// If field map already exists, return it immediately
	auto it = m_bfields.find(run_number);
	if (it != m_bfields.end()) {
		return it->second;
	}

	DMagneticFieldMap* bfield;

	// Create magnetic field object for use by everyone
	// Allow a trivial homogeneous map to be used if
	// specified on the command line
	string bfield_type = "FineMesh";
	string bfield_map = "";
	auto params = m_app->GetJParameterManager();
	params->SetDefaultParameter("BFIELD_TYPE", bfield_type);
	if (params->Exists("BFIELD_MAP")) {
		bfield_map = params->GetParameterValue<std::string>("BFIELD_MAP");
	}
	if (bfield_type == "CalibDB" || bfield_type == "FineMesh") {
		// if the magnetic field map got passed in on the command line, then use that value instead of the CCDB values
		if (bfield_map != "") {
			bfield = new DMagneticFieldMapFineMesh(m_app, run_number, bfield_map);
		}
		else {
			// otherwise, we load some default map
			// see if we can load the name of the magnetic field map to use from the calib DB
			JCalibration *jcalib = m_app->GetService<JCalibrationManager>()->GetJCalibration(run_number);
			map<string, string> bfield_map_name;
			if (jcalib->GetCalib("/Magnets/Solenoid/solenoid_map", bfield_map_name)) {
				// if we can't find information in the CCDB, then quit with an error message
				jerr << ccdb_help << endl;
				exit(-1);
			}
			else {
				if (bfield_map_name.find("map_name") != bfield_map_name.end()) {
					if (bfield_map_name["map_name"] == "NoField") {
						// special case for no magnetic field
						bfield = new DMagneticFieldMapNoField(m_app);
					}
					else {
						// pass along the name of the magnetic field map to load
						bfield = new DMagneticFieldMapFineMesh(m_app, run_number, bfield_map_name["map_name"]);
					}
				}
				else {
					// if we can't find information in the CCDB, then quit with an error message
					jerr << ccdb_help << endl;
					exit(-1);
				}
			}
		}
		string subclass = "<none>";
		if (dynamic_cast<DMagneticFieldMapFineMesh *>(bfield)) subclass = "DMagneticFieldMapFineMesh";
		if (dynamic_cast<DMagneticFieldMapNoField *>(bfield)) subclass = "DMagneticFieldMapNoField";
		jout << "Created Magnetic field map of type " << subclass << endl;
	} else if (bfield_type == "Const") {
		bfield = new DMagneticFieldMapConst(0.0, 0.0, 1.9);
		jout << "Created Magnetic field map of type DMagneticFieldMapConst." << endl;
		//}else if(bfield_type=="Spoiled"){
		// bfield = new DMagneticFieldMapSpoiled(this);
		// jout<<"Created Magnetic field map of type DMagneticFieldMapSpoiled."<<endl;
		//}else if(bfield_type=="Parameterized"){
		// bfield = new DMagneticFieldMapParameterized(this);
		// jout<<"Created Magnetic field map of type DMagneticFieldMapParameterized."<<endl;
		//}
	} else if (bfield_type == "NoField") {
		bfield = new DMagneticFieldMapNoField(m_app);
		jout << "Created Magnetic field map with B=(0,0,0) everywhere." << endl;
	} else {
		_DBG_ << " Unknown DMagneticFieldMap subclass \"DMagneticFieldMap" << bfield_type << "\" !!" << endl;
		exit(-1);
	}
	m_bfields[run_number] = bfield;
	return bfield;
}


DLorentzDeflections *DGeometryManager::GetLorentzDeflections(unsigned int run_number) {

	std::lock_guard<std::mutex> lock(m_lorentz_mutex);
	auto it = m_lorentz_defs.find(run_number);
	if (it != m_lorentz_defs.end()) {
		return it->second;
	}

	auto lorentz_def = new DLorentzMapCalibDB(m_app, run_number);
	m_lorentz_defs[run_number] = lorentz_def;
	return lorentz_def;
}


DRootGeom *DGeometryManager::GetRootGeom(unsigned int run_number) {

	std::lock_guard<std::mutex> lock(m_rootgeoms_mutex);
	auto it = m_root_geoms.find(run_number);
	if (it != m_root_geoms.end()) {
		return it->second;
	}

	auto rootGeom = new DRootGeom(m_app, run_number);
	m_root_geoms[run_number] = rootGeom;
	return rootGeom;
}


DDIRCLutReader *DGeometryManager::GetDIRCLut(unsigned int run_number) {

	std::lock_guard<std::mutex> lock(m_dirclut_mutex);
	auto it = m_dirclut_readers.find(run_number);
	if (it != m_dirclut_readers.end()) {
		return it->second;
	}

	auto dircLut = new DDIRCLutReader(m_app, run_number);
	m_dirclut_readers[run_number] = dircLut;
	return dircLut;
}
