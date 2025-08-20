// $Id$
//
//    File: JEventProcessor_field_map_writer.cc
// Created: Wed Aug 20 05:53:12 PM EDT 2025
// Creator: bgrube (on Linux ifarm2402.jlab.org 5.14.0-570.33.2.el9_6.x86_64 x86_64)
//


#include <DANA/DGeometryManager.h>
#include <JANA/JApplication.h>
#include <HDGEOMETRY/DMagneticFieldMap.h>
#include <HDGEOMETRY/DMagneticFieldMapFineMesh.h>

#include "JEventProcessor_field_map_writer.h"


extern "C"{
	void
	InitPlugin(JApplication *app)
	{
		InitJANAPlugin(app);
		app->Add(new JEventProcessor_field_map_writer());
	}
}  // "C"


JEventProcessor_field_map_writer::JEventProcessor_field_map_writer()
{
	SetTypeName(NAME_OF_THIS); // Provide JANA with this class's name
}


JEventProcessor_field_map_writer::~JEventProcessor_field_map_writer()
{ }


void
JEventProcessor_field_map_writer::Init()
{
	auto app = GetApplication();
	app->SetDefaultParameter( "field_map_writer:OUTPUT_BASE_NAME",     OUTPUT_BASE_NAME,     "names of output files with magnetic field maps are '<OUTPUT_BASE_NAME>_{coarse,fine}Mesh_<run_number>.msgpack'");
	app->SetDefaultParameter( "field_map_writer:STOP_AFTER_FIRST_RUN", STOP_AFTER_FIRST_RUN, "stop processing after the first run");
}

void
JEventProcessor_field_map_writer::BeginRun(const std::shared_ptr<const JEvent>& event)
{
	// get magnetic field map and ensure it is of type DMagneticFieldMapFineMesh
	auto app = GetApplication();
	auto geoManager = app->GetService<DGeometryManager>();
	const int runNmb = event->GetRunNumber();
	auto fieldMap = geoManager->GetBfield(runNmb);
	auto fineMeshFieldMap = dynamic_cast<DMagneticFieldMapFineMesh*>(fieldMap);
	if (not fineMeshFieldMap) {
		throw JException("loaded magnetic field map is not a `DMagneticFieldMapFineMesh`");
	}

	// write out field map
	const std::string outputFileNameCoarseMesh = OUTPUT_BASE_NAME + "_coarseMesh_" + std::to_string(runNmb) + ".msgpack";
	const std::string outputFileNameFineMesh   = OUTPUT_BASE_NAME + "_fineMesh_"   + std::to_string(runNmb) + ".msgpack";
	fineMeshFieldMap->WriteMsgpackFileCoarseMesh(outputFileNameCoarseMesh);
	fineMeshFieldMap->WriteMsgpackFileFineMesh  (outputFileNameFineMesh);
}


void
JEventProcessor_field_map_writer::Process(const std::shared_ptr<const JEvent>& event)
{
	// do nothing
}


void
JEventProcessor_field_map_writer::EndRun()
{ }


void
JEventProcessor_field_map_writer::Finish()
{ }
