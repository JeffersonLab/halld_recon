// $Id$
//
//    File: JEventProcessor_dumpcandidates.cc
// Created: Tue Feb  4 09:29:35 EST 2014
// Creator: davidl (on Linux ifarm1102 2.6.32-220.7.1.el6.x86_64 x86_64)
//

#include "JEventProcessor_dumpcandidates.h"


#include <TRACKING/DTrackCandidate.h>
#include <TRACKING/DTrackHitSelector.h>
#include <TRACKING/DTrackFitter.h>
#include <CDC/DCDCTrackHit.h>
#include <FDC/DFDCPseudo.h>
#include <DANA/DEvent.h>

extern "C"{
void InitPlugin(JApplication *app){
	InitJANAPlugin(app);
	app->Add(new JEventProcessor_dumpcandidates());
}
} // "C"


//------------------
// JEventProcessor_dumpcandidates (Constructor)
//------------------
JEventProcessor_dumpcandidates::JEventProcessor_dumpcandidates()
{
	SetTypeName("JEventProcessor_dumpcandidates");

	events_written = 0;
	events_discarded = 0;
	
	beam_origin = DVector3(0.0, 0.0, 65.0);
	beam_dir    = DVector3(0.0, 0.0, 1.0);
}

//------------------
// ~JEventProcessor_dumpcandidates (Destructor)
//------------------
JEventProcessor_dumpcandidates::~JEventProcessor_dumpcandidates()
{

}

//------------------
// Init
//------------------
void JEventProcessor_dumpcandidates::Init()
{
	auto app = GetApplication();
	lockService = app->GetService<JLockService>();

	MAX_CANDIDATE_FILTER = 1000;
	app->SetDefaultParameter("MAX_CANDIDATE_FILTER", MAX_CANDIDATE_FILTER, "Maximum number of candidates allowed in event before any are written to file.");

	
	// Open output file
	ofs = new ofstream("gluex_candidates.txt");
}

//------------------
// BeginRun
//------------------
void JEventProcessor_dumpcandidates::BeginRun(const std::shared_ptr<const JEvent>& event)
{
	// Get pointer to geometry object
	dgeom = GetDGeometry(event);
	if(!dgeom){
		jerr << "Couldn't get DGeometry pointer!!" << endl;
		return;
	}

	// Get CDC wires
	vector<vector<DCDCWire *> > cdcwires;
	dgeom->GetCDCWires(cdcwires);

	// Get FDC wires
	vector<vector<DFDCWire *> > fdcwires;
	dgeom->GetFDCWires(fdcwires);
	
	lockService->RootFillLock(this);
	// Generate map keyed by wire object's address
	// (in the form of an unsigned long) and whose
	// value is the index of the wire (i.e. position
	// as produced by dumpwires)
	int index = 0;
	for(unsigned int i=0; i<cdcwires.size(); i++){
		vector<DCDCWire *> &wires = cdcwires[i];
		for(unsigned int j=0; j<wires.size(); j++){

			DCDCWire *w = wires[j];
			wireID[ GetCDCWireID(w) ] = index++;
		}
	}

	for(unsigned int i=0; i<fdcwires.size(); i++){
		vector<DFDCWire *> &wires = fdcwires[i];
		for(unsigned int j=0; j<wires.size(); j++){

			DFDCWire *w = wires[j];
			wireID[ GetFDCWireID(w) ] = index++;
		}
	}
	lockService->RootFillUnLock(this);
}

//------------------
// Process
//------------------
void JEventProcessor_dumpcandidates::Process(const std::shared_ptr<const JEvent>& event)
{
	// Get track candidates
	vector<const DTrackCandidate*> candidates;
	event->Get(candidates);
	if(candidates.size()==0 || candidates.size()>MAX_CANDIDATE_FILTER){
		events_discarded++;
		return;
	}

	// Get pointer to DTrackHitSelector object
	vector<const DTrackHitSelector *> hitselectors;
	event->Get(hitselectors);
	if(hitselectors.size()<1){
		_DBG_<<"Unable to get a DTrackHitSelector object!"<<endl;
		return;
	}
	const DTrackHitSelector * hitselector = hitselectors[0];

	// Get pointer to DTrackFitter object
	vector<const DTrackFitter *> fitters;
	event->Get(fitters);
	if(fitters.size()<1){
		_DBG_<<"Unable to get a DTrackFitter object!"<<endl;
		throw JException("Unable to get a DTrackFitter object!");
	}
	DTrackFitter *fitter = const_cast<DTrackFitter*>(fitters[0]);
	
	for(unsigned int i=0; i<candidates.size(); i++){
		const DTrackCandidate *can = candidates[i];
		
		// Create DReferenceTrajectory for this candidate
		DReferenceTrajectory *rt = new DReferenceTrajectory(fitter->GetDMagneticFieldMap());
		rt->SetDGeometry(dgeom);
      	rt->q = can->charge();
		rt->SetMass(0.1396);
		rt->Swim(can->position(),can->momentum(),can->charge());
		
		// Get hits to be used for the fit
		vector<const DCDCTrackHit*> cdctrackhits;
		vector<const DFDCPseudo*> fdcpseudos;
		event->Get(cdctrackhits);
		event->Get(fdcpseudos);
		hitselector->GetAllHits(DTrackHitSelector::kHelical, rt, cdctrackhits, fdcpseudos, fitter);

		// Get list of wire ids
		vector<int> wire_ids;
		for(unsigned int j=0; j<cdctrackhits.size(); j++){
			unsigned long id = GetCDCWireID( cdctrackhits[j]->wire );
			wire_ids.push_back(GetWireIndex( id ));

		}
		for(unsigned int j=0; j<fdcpseudos.size(); j++){
			unsigned long id = GetFDCWireID( fdcpseudos[j]->wire );
			wire_ids.push_back(GetWireIndex( id ));
		}
		
		// To make it easier to compare to thrown values later,
		// find the parameters of the trajectory point closest
		// to the beamline
		rt->Swim(can->position(),-can->momentum(),-can->charge());
		DKinematicData kd = (*can);
		DVector3 commonpos;
		double doca, var_doca;
		rt->FindPOCAtoLine(beam_origin, beam_dir, NULL, &kd, commonpos, doca, var_doca);

		// Write candidate to string
		stringstream ss;
		ss << can->charge();
		ss << " " << kd.x() << " " << kd.y() << " " << kd.z();
		ss << " " << -kd.px() << " " << -kd.py() << " " << -kd.pz();
		for(unsigned int j=0; j<wire_ids.size(); j++){
			ss << " " << wire_ids[j];
		}
		
		// Write candidate string to file
		(*ofs) << ss.str() << endl;
		events_written++;
	}
}

//------------------
// EndRun
//------------------
void JEventProcessor_dumpcandidates::EndRun()
{
}

//------------------
// Finish
//------------------
void JEventProcessor_dumpcandidates::Finish()
{
	if(ofs){
		ofs->close();
		delete ofs;
		ofs =NULL;
	}
	
	cout << endl;
	cout << "Wrote " << events_written << " candidate events to output file (discarded " << events_discarded << ")" << endl;
	cout << endl;
}



