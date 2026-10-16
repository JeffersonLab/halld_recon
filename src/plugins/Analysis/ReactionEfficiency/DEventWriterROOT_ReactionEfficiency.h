// $Id$
//
//    File: DEventWriterROOT_ReactionEfficiency.h
// Created: Sat Jun 10 10:20:19 EDT 2023
// Creator: jrsteven (on Linux ifarm1901.jlab.org 3.10.0-1160.90.1.el7.x86_64 x86_64)
//

#ifndef _DEventWriterROOT_ReactionEfficiency_
#define _DEventWriterROOT_ReactionEfficiency_

#include <map>
#include <string>

#include <ANALYSIS/DEventWriterROOT.h>

using namespace std;

class DEventWriterROOT_ReactionEfficiency : public DEventWriterROOT
{
	public:
		virtual ~DEventWriterROOT_ReactionEfficiency(void){};
		void Run_Update_Custom(const std::shared_ptr<const JEvent>& locEvent);

	protected:

		//CUSTOM FUNCTIONS: //Inherit from this class and write custom code in these functions

		virtual void Create_CustomBranches_ThrownTree(DTreeBranchRegister& locBranchRegister, const std::shared_ptr<const JEvent>& locEvent) const;
		virtual void Fill_CustomBranches_ThrownTree(DTreeFillData* locTreeFillData, const std::shared_ptr<const JEvent>& locEvent, const DMCReaction* locMCReaction, const vector<const DMCThrown*>& locMCThrowns) const;

		virtual void Create_CustomBranches_DataTree(DTreeBranchRegister& locBranchRegister, const std::shared_ptr<const JEvent>& locEvent, const DReaction* locReaction, bool locIsMCDataFlag) const;
		virtual void Fill_CustomBranches_DataTree(DTreeFillData* locTreeFillData, const std::shared_ptr<const JEvent>& locEvent, const DReaction* locReaction, const DMCReaction* locMCReaction, const vector<const DMCThrown*>& locMCThrowns,
				const DMCThrownMatching* locMCThrownMatching, const DDetectorMatches* locDetectorMatches,
				const vector<const DBeamPhoton*>& locBeamPhotons, const vector<const DChargedTrackHypothesis*>& locChargedHypos,
				const vector<const DNeutralParticleHypothesis*>& locNeutralHypos, const deque<const DParticleCombo*>& locParticleCombos) const;		

	private:

		const DParticleID* dParticleID;
		const DMagneticFieldMap *bfield;
		DReferenceTrajectory *rt;
};

#endif //_DEventWriterROOT_ReactionEfficiency_
