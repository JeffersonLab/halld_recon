// $Id$
//
//    File: DEventWriterROOT_ReactionEfficiency.cc
// Created: Sat Jun 10 10:20:19 EDT 2023
// Creator: jrsteven (on Linux ifarm1901.jlab.org 3.10.0-1160.90.1.el7.x86_64 x86_64)
//

#include "DEventWriterROOT_ReactionEfficiency.h"

//GLUEX TTREE DOCUMENTATION: https://halldweb.jlab.org/wiki/index.php/Analysis_TTreeFormat

void DEventWriterROOT_ReactionEfficiency::Run_Update_Custom(const std::shared_ptr<const JEvent>& locEvent)
{
	const DParticleID* locParticleID = NULL;
	locEvent->GetSingle(locParticleID);
	dParticleID = locParticleID;
	
	bfield = DEvent::GetBfield(locEvent);
	rt = new DReferenceTrajectory(bfield);
}

void DEventWriterROOT_ReactionEfficiency::Create_CustomBranches_DataTree(DTreeBranchRegister& locBranchRegister, const std::shared_ptr<const JEvent>& locEvent, const DReaction* locReaction, bool locIsMCDataFlag) const
{
	//EXAMPLES: Create a branch to hold an array of fundamental type:
	//If filling for a specific particle, the branch name should match the particle branch name
	//locArraySizeString is the name of the branch whose variable that contains the size of the array for that tree entry
		//To match the default TTree branches, use either: 'NumThrown', 'NumBeam', 'NumChargedHypos', 'NumNeutralHypos', or 'NumCombos', as appropriate
	unsigned int locInitArraySize = 50; //if too small, will auto-increase as needed, but requires new calls //if too large, uses more memory than needed
	locBranchRegister.Register_FundamentalArray<Float_t>("BestMissingMatchDistTOF", "NumCombos", locInitArraySize);
	locBranchRegister.Register_FundamentalArray<Float_t>("BestMissingMatchDistBCAL", "NumCombos", locInitArraySize);
}

void DEventWriterROOT_ReactionEfficiency::Create_CustomBranches_ThrownTree(DTreeBranchRegister& locBranchRegister, const std::shared_ptr<const JEvent>& locEvent) const
{
	//EXAMPLES: See Create_CustomBranches_DataTree
}

void DEventWriterROOT_ReactionEfficiency::Fill_CustomBranches_DataTree(DTreeFillData* locTreeFillData, const std::shared_ptr<const JEvent>& locEvent, const DReaction* locReaction, const DMCReaction* locMCReaction, const vector<const DMCThrown*>& locMCThrowns,
	const DMCThrownMatching* locMCThrownMatching, const DDetectorMatches* locDetectorMatches,
	const vector<const DBeamPhoton*>& locBeamPhotons, const vector<const DChargedTrackHypothesis*>& locChargedHypos,
	const vector<const DNeutralParticleHypothesis*>& locNeutralHypos, const deque<const DParticleCombo*>& locParticleCombos) const
{
	//The array indices of the particles/combos in the main TTree branches match the vectors of objects passed into this function
		//So if you want to add custom data for each (e.g.) charged track, the correspondence to the main arrays is 1 <--> 1

	for(size_t loc_icombo=0; loc_icombo<locParticleCombos.size(); loc_icombo++) {
		const DParticleCombo* locParticleCombo = locParticleCombos[loc_icombo];
		
		// distance of best match to TOF and BCAL
		double locBestMissingMatchDistTOF = 999;
		double locBestMissingMatchDistBCAL = 999;
		
		// get missing particle
		vector<const DKinematicData*> locMissingParticles = locParticleCombo->Get_MissingParticles(locReaction);
		if(locMissingParticles.empty()) return;
		
		const DKinematicData *locMissing = locMissingParticles[0];
		DVector3 locMissingPosition = locMissing->position();
		DVector3 locMissingMomentum = locMissing->momentum();
		double locInputStartTime = locParticleCombo->Get_EventVertex().T();
		
		rt->Reset();
		rt->Swim(locMissingPosition, locMissingMomentum, locMissing->charge());
		
		// compute distance from unused BCAL showers
		vector<const DNeutralShower*> locUnusedNeutralShowers;
		dAnalysisUtilities->Get_UnusedNeutralShowers(locEvent, locParticleCombo, locUnusedNeutralShowers);
		for(size_t loc_i=0; loc_i<locUnusedNeutralShowers.size(); loc_i++) {
			if(locUnusedNeutralShowers[loc_i]->dDetectorSystem != SYS_BCAL) continue;
			
			const DBCALShower *locBCALShower;
			locUnusedNeutralShowers[loc_i]->GetSingle(locBCALShower);
			
			shared_ptr<DBCALShowerMatchParams>locShowerMatchParams;
			DVector3 locOutputProjPos, locOutputProjMom;
			if(dParticleID->Distance_ToTrack(rt, locBCALShower, locInputStartTime, locShowerMatchParams, &locOutputProjPos, &locOutputProjMom)) {
				
				double locDeltaT = locBCALShower->t - locShowerMatchParams->dFlightTime - locInputStartTime;
				double locMissingMatchDist = locShowerMatchParams->Get_DistanceToTrack();
				
				if(fabs(locDeltaT) > 1.0) continue;
				
				// keep best matched BCAL hit
				if(locMissingMatchDist < locBestMissingMatchDistBCAL) 
					locBestMissingMatchDistBCAL = locMissingMatchDist;
			}
		}
		
		vector<const DTOFPoint*> locUnusedTOFPoints;
		dAnalysisUtilities->Get_UnusedTOFPoints(locEvent, locParticleCombo, locUnusedTOFPoints);
		for(size_t loc_i=0; loc_i<locUnusedTOFPoints.size(); loc_i++) {

			shared_ptr<DTOFHitMatchParams>locHitMatchParams;
			DVector3 locOutputProjPos, locOutputProjMom;
		
			////////////////////////////////////////////////////////////////////////
			if(dParticleID->Distance_ToTrack(rt, locUnusedTOFPoints[loc_i], locInputStartTime, locHitMatchParams, &locOutputProjPos, &locOutputProjMom)) {
				
				double locDeltaT = locHitMatchParams->dHitTime - locHitMatchParams->dFlightTime - locInputStartTime;
				double locMissingMatchDist = locHitMatchParams->Get_DistanceToTrack();
				
				if(fabs(locDeltaT) > 0.3) continue;
				
				// keep best matched TOF hit
				if(locMissingMatchDist < locBestMissingMatchDistTOF) 
					locBestMissingMatchDistTOF = locMissingMatchDist;
			}
		}
		
		locTreeFillData->Fill_Array<Float_t>("BestMissingMatchDistTOF", locBestMissingMatchDistTOF, loc_icombo);
		locTreeFillData->Fill_Array<Float_t>("BestMissingMatchDistBCAL", locBestMissingMatchDistBCAL, loc_icombo);
	}

}

void DEventWriterROOT_ReactionEfficiency::Fill_CustomBranches_ThrownTree(DTreeFillData* locTreeFillData, const std::shared_ptr<const JEvent>& locEvent, const DMCReaction* locMCReaction, const vector<const DMCThrown*>& locMCThrowns) const
{
	//EXAMPLES: See Fill_CustomBranches_DataTree
}

