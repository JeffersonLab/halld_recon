#ifndef _DEventWriterROOT_
#define _DEventWriterROOT_

#include <map>
#include <string>

#include "TClonesArray.h"
#include "TLorentzVector.h"
#include "TVector3.h"
#include "TTree.h"
#include "TFile.h"
#include "TROOT.h"
#include "TMap.h"
#include "TObjString.h"

#include "JANA/JObject.h"
#include <JANA/JEvent.h>

#include "TRIGGER/DTrigger.h"
#include "BCAL/DBCALShower.h"
#include "FCAL/DFCALShower.h"
#include "CCAL/DCCALShower.h"
#include "TRACKING/DMCThrown.h"
#include <TRACKING/DMCTrajectoryPoint.h>
#include "TRACKING/DTrackTimeBased.h"

#include "PID/DVertex.h"
#include "PID/DChargedTrack.h"
#include "PID/DBeamPhoton.h"
#include "PID/DChargedTrackHypothesis.h"
#include "PID/DNeutralParticleHypothesis.h"
#include "PID/DNeutralShower.h"
#include "PID/DEventRFBunch.h"
#include "PID/DMCReaction.h"

#include "ANALYSIS/DParticleCombo.h"
#include "ANALYSIS/DReaction.h"
#include "ANALYSIS/DAnalysisResults.h"
#include "ANALYSIS/DAnalysisUtilities.h"
#include "ANALYSIS/DMCThrownMatching.h"
#include "ANALYSIS/DCutActions.h"
#include "ANALYSIS/DTreeInterface.h"
#include "ANALYSIS/DReactionVertexInfo.h"

using namespace std;

class DEventWriterROOT : public JObject
{
	public:
		JOBJECT_PUBLIC(DEventWriterROOT);

		virtual ~DEventWriterROOT(void);
		void Initialize(const std::shared_ptr<const JEvent>& locEvent);
		void Run_Update(const std::shared_ptr<const JEvent>& locEvent);

		void Create_ThrownTree(const std::shared_ptr<const JEvent>& locEvent, string locOutputFileName) const;

		void Fill_DataTrees(const std::shared_ptr<const JEvent>& locEvent, string locDReactionTag) const; //fills all from this factory tag
		void Fill_DataTree(const std::shared_ptr<const JEvent>& locEvent, const DReaction* locReaction, deque<const DParticleCombo*>& locParticleCombos) const;
		void Fill_ThrownTree(const std::shared_ptr<const JEvent>& locEvent) const;

	protected:

		//CUSTOM FUNCTIONS: //Inherit from this class and write custom code in these functions
			//DO NOT: Write any code that requires a lock of ANY KIND. No reading calibration constants, accessing gParams, etc. This can cause deadlock.
		virtual void Create_CustomBranches_ThrownTree(DTreeBranchRegister& locBranchRegister, const std::shared_ptr<const JEvent>& locEvent) const{};
		virtual void Fill_CustomBranches_ThrownTree(DTreeFillData* locTreeFillData, const std::shared_ptr<const JEvent>& locEvent, const DMCReaction* locMCReaction, const vector<const DMCThrown*>& locMCThrowns) const{};
		virtual void Create_CustomBranches_DataTree(DTreeBranchRegister& locBranchRegister, const std::shared_ptr<const JEvent>& locEvent, const DReaction* locReaction, bool locIsMCDataFlag) const{};
		virtual void Fill_CustomBranches_DataTree(DTreeFillData* locTreeFillData, const std::shared_ptr<const JEvent>& locEvent, const DReaction* locReaction, const DMCReaction* locMCReaction, const vector<const DMCThrown*>& locMCThrowns,
				const DMCThrownMatching* locMCThrownMatching, const DDetectorMatches* locDetectorMatches,
				const vector<const DBeamPhoton*>& locBeamPhotons, const vector<const DChargedTrackHypothesis*>& locChargedHypos,
				const vector<const DNeutralParticleHypothesis*>& locNeutralHypos, const deque<const DParticleCombo*>& locParticleCombos) const{};

		//UTILITY FUNCTIONS
		string Convert_ToBranchName(string locInputName) const;
		string Build_BranchName(string locParticleBranchName, string locVariableName) const;
		ULong64_t Calc_ParticleMultiplexID(Particle_t locPID) const;
		void Get_DecayProductNames(const DReaction* locReaction, size_t locReactionStepIndex, TMap* locPositionToNameMap, TList*& locDecayProductNames, deque<size_t>& locSavedSteps) const;

		const DAnalysisUtilities* dAnalysisUtilities;

	private:

		unsigned int dInitNumThrownArraySize;
		unsigned int dInitNumBeamArraySize;
		unsigned int dInitNumTrackArraySize;
		unsigned int dInitNumNeutralArraySize;
		unsigned int dInitNumComboArraySize;

		double dTargetCenterZ;

		//DEFAULT ACTIONS LISTED SEPARATELY FROM CUSTOM (in case in derived class user does something bizarre)
		map<const DReaction*, DCutAction_ThrownTopology*> dCutActionMap_ThrownTopology;
		map<const DReaction*, DCutAction_TrueCombo*> dCutActionMap_TrueCombo;
		map<const DReaction*, DCutAction_BDTSignalCombo*> dCutActionMap_BDTSignalCombo;

		//add in future: let user execute custom actions (outside of lock): user adds and initializes actions in derived-writer constructor
		//map<const DReaction*, map<string, map<const DParticleCombo*, bool> > > dCustomActionResultsMap; //string is action name

		/****************************************************************************************************************************************/

		//TREE INTERFACES, FILL OBJECTS
		//The non-thrown objects are created during the constructor, and thus the maps can remain const
		//The thrown objects are created later by the user (so they can specify file name), when the object is const, so they are declared mutable
		mutable DTreeInterface* dThrownTreeInterface = nullptr;
		mutable DTreeFillData dThrownTreeFillData;
		map<const DReaction*, DTreeInterface*> dTreeInterfaceMap;
		map<const DReaction*, DTreeFillData*> dTreeFillDataMap;

		map<const DReaction*, const DReactionVertexInfo*> dVertexInfoMap;

		//Functions to name and fill the pull/error matrix branches correctly:
		//Meaning, if is more than one particle of the same species (e.g. gamma),
		//the naming will be consistent with the default naming in the default root-tree,
		//e.g. Photon1__PullPx, Photon2__PullPx, and so on...
		//The same holds for charged particles...
		vector<string> getPIDinReaction(const DReaction* locReaction)const;
		map<Particle_t,int> getNameForParticle(const DReaction* someReaction, vector<string> someVector)const;
		mutable map<Particle_t,int> abundanceMap;
		string assignName(Particle_t someParticle, map<Particle_t,int> someMap)const;

		// More pull-related variables
		mutable map<const DReaction*, bool> writePulls;

		//TREE CREATION:
		void Create_DataTree(const DReaction* locReaction, const std::shared_ptr<const JEvent>& locEvent, bool locIsMCDataFlag);
		TMap* Create_UserInfoMaps(DTreeBranchRegister& locTreeBranchRegister, const std::shared_ptr<const JEvent>& locEvent, const DReaction* locReaction) const;
		void Create_UserTargetInfo(DTreeBranchRegister& locTreeBranchRegister, Particle_t locTargetPID) const;
		void Create_Branches_Thrown(DTreeBranchRegister& locTreeBranchRegister, bool locIsOnlyThrownFlag) const;

		//TREE CREATION: PARTICLE INFO
		void Create_Branches_ThrownParticles(DTreeBranchRegister& locTreeBranchRegister, bool locIsOnlyThrownFlag) const;
		void Create_Branches_Beam(DTreeBranchRegister& locTreeBranchRegister, bool locIsMCDataFlag) const;
		void Create_Branches_NeutralHypotheses(DTreeBranchRegister& locTreeBranchRegister, bool locIsMCDataFlag) const;
		void Create_Branches_ChargedHypotheses(DTreeBranchRegister& locTreeBranchRegister, bool locIsMCDataFlag) const;
		void Create_Branches_KinFitData(DTreeBranchRegister& locBranchRegister, const std::shared_ptr<const JEvent>& locEventLoop, const DReaction* locReaction, bool locIsMCDataFlag) const;

		//TREE CREATION: COMBO INFO
			//TMap is locPositionToNameMap
		void Create_Branches_Combo(DTreeBranchRegister& locTreeBranchRegister, const DReaction* locReaction, bool locIsMCDataFlag, TMap* locPositionToNameMap) const;
		void Create_Branches_BeamComboParticle(DTreeBranchRegister& locTreeBranchRegister, Particle_t locBeamPID, DKinFitType locKinFitType) const;
		void Create_Branches_ComboTrack(DTreeBranchRegister& locTreeBranchRegister, string locParticleBranchName, DKinFitType locKinFitType) const;
		void Create_Branches_ComboNeutral(DTreeBranchRegister& locTreeBranchRegister, string locParticleBranchName, DKinFitType locKinFitType) const;

		//TREE FILLING: THROWN INFO
		void Compute_ThrownPIDInfo(const vector<const DMCThrown*>& locMCThrowns_FinalState, const vector<const DMCThrown*>& locMCThrowns_Decaying,
				ULong64_t& locNumPIDThrown_FinalState, ULong64_t& locPIDThrown_Decaying) const;
		void Group_ThrownParticles(const vector<const DMCThrown*>& locMCThrowns_FinalState, const vector<const DMCThrown*>& locMCThrowns_Decaying,
				vector<const DMCThrown*>& locMCThrownsToSave, map<const DMCThrown*, unsigned int>& locThrownIndexMap) const;
		void Fill_ThrownInfo(DTreeFillData* locTreeFillData, const DMCReaction* locMCReaction, const DBeamPhoton* locTaggedMCGenBeam, const vector<const DMCThrown*>& locMCThrowns,
				const map<const DMCThrown*, unsigned int>& locThrownIndexMap, ULong64_t locNumPIDThrown_FinalState, ULong64_t locPIDThrown_Decaying,  const vector<const DMCTrajectoryPoint*> locDMCTrajectoryPoints,
				const DMCThrownMatching* locMCThrownMatching = NULL) const;
		void Fill_ThrownParticleData(DTreeFillData* locTreeFillData, unsigned int locArrayIndex, const DMCThrown* locMCThrown, 
				const map<const DMCThrown*, unsigned int>& locThrownIndexMap, const DMCThrownMatching* locMCThrownMatching) const;
		void Fill_ThrownParticleTrajectoryInfo(DTreeFillData* locTreeFillData, const vector<const DMCTrajectoryPoint*> locDMCTrajectoryPoints) const;

		//TREE FILLING: GET HYPOTHESES/BEAM
		vector<const DBeamPhoton*> Get_BeamPhotons(const deque<const DParticleCombo*>& locParticleCombos) const;
		vector<const DChargedTrackHypothesis*> Get_ChargedHypotheses(const std::shared_ptr<const JEvent>& locEvent) const;
		vector<const DChargedTrackHypothesis*> Get_ChargedHypotheses_Used(const std::shared_ptr<const JEvent>& locEvent, const DReaction* locReaction, const deque<const DParticleCombo*>& locParticleCombos) const;
		vector<const DNeutralParticleHypothesis*> Get_NeutralHypotheses(const std::shared_ptr<const JEvent>& locEvent, const set<Particle_t>& locReactionPIDs) const;
		vector<const DNeutralParticleHypothesis*> Get_NeutralHypotheses_Used(const std::shared_ptr<const JEvent>& locEvent, const DReaction* locReaction, const set<Particle_t>& locReactionPIDs, const deque<const DParticleCombo*>& locParticleCombos) const;

		//TREE FILLING: INDEPENDENT PARTICLES
		void Fill_BeamData(DTreeFillData* locTreeFillData, unsigned int locArrayIndex, const DBeamPhoton* locBeamPhoton, const DVertex* locVertex, const DMCThrownMatching* locMCThrownMatching) const;
		void Fill_ChargedHypo(DTreeFillData* locTreeFillData, unsigned int locArrayIndex, const DChargedTrackHypothesis* locChargedTrackHypothesis, const DMCThrownMatching* locMCThrownMatching,
				const map<const DMCThrown*, unsigned int>& locThrownIndexMap, const DDetectorMatches* locDetectorMatches) const;
		void Fill_NeutralHypo(DTreeFillData* locTreeFillData, unsigned int locArrayIndex, const DNeutralParticleHypothesis* locPhotonHypothesis, const DMCThrownMatching* locMCThrownMatching,
				const map<const DMCThrown*, unsigned int>& locThrownIndexMap, const DDetectorMatches* locDetectorMatches) const;

		//TREE FILLING: COMBO
		void Fill_ComboData(DTreeFillData* locTreeFillData, const DReaction* locReaction, const DParticleCombo* locParticleCombo, unsigned int locComboIndex, const map<pair<oid_t, Particle_t>, size_t>& locObjectToArrayIndexMap) const;
		void Fill_ComboStepData(DTreeFillData* locTreeFillData, const DReaction* locReaction, const DParticleCombo* locParticleCombo, unsigned int locStepIndex, unsigned int locComboIndex,
				DKinFitType locKinFitType, const map<pair<oid_t, Particle_t>, size_t>& locObjectToArrayIndexMap) const;
		void Fill_KinFitData(DTreeFillData* locTreeFillData, const std::shared_ptr<const JEvent>& locEvent, const DReaction* locReaction, const DMCReaction* locMCReaction, const vector<const DMCThrown*>& locMCThrowns,
				const DMCThrownMatching* locMCThrownMatching, const DDetectorMatches* locDetectorMatches,
				const vector<const DBeamPhoton*>& locBeamPhotons, const vector<const DChargedTrackHypothesis*>& locChargedHypos,
				const vector<const DNeutralParticleHypothesis*>& locNeutralHypos, const deque<const DParticleCombo*>& locParticleCombos) const;   // FIX

		//TREE FILLING: COMBO PARTICLES
		void Fill_ComboBeamData(DTreeFillData* locTreeFillData, unsigned int locComboIndex, const DBeamPhoton* locBeamPhoton, size_t locBeamIndex, DKinFitType locKinFitType) const;
		void Fill_ComboChargedData(DTreeFillData* locTreeFillData, unsigned int locComboIndex, string locParticleBranchName, const DChargedTrackHypothesis* locMeasuredChargedHypo,
				const DChargedTrackHypothesis* locChargedHypo, size_t locChargedIndex, DKinFitType locKinFitType) const;
		void Fill_ComboNeutralData(DTreeFillData* locTreeFillData, unsigned int locComboIndex, string locParticleBranchName, const DNeutralParticleHypothesis* locMeasuredNeutralHypo,
				const DNeutralParticleHypothesis* locNeutralHypo, size_t locNeutralIndex, DKinFitType locKinFitType) const;


		//****************************************************************************************************************

		//GET PULLS FROM KINEMATIC FITTER:  Px, Py, pz, Xx, Xy, Xz
		mutable map< const JObject*, map<DKinFitPullType, double> > myPullsMap;
		void Get_PullsFromFit(const DParticleCombo* particleCombos)const;
		map<DKinFitPullType, double> getPulls(const JObject* particle, const JObject* shower, DKinFitType yourFitType) const;
	
		//Cosmetics to set and get the pulls-name/-type
		//Set the tree branches.
		void setTreePullBranches(DTreeBranchRegister& locBranchRegister,string yourBranchName,DKinFitType yourFitType, int yourNCombos, bool isNeutral) const;
		//Fill the branches:
		void fillTreePullBranches(DTreeFillData* locTreeFillData,string yourBranchName,DKinFitType yourFitType,map<DKinFitPullType, double> yourPullsMap, int yourIndex, bool isNeutral) const;

		//Check, if pull-information is available (preserves crashes)
		void setPullFlag(const DReaction* currentReaction, bool myFlag) const;
		bool getPullFlag(const DReaction* currentReaction) const;
	
		//Store the error martices for the kinematic fit:  (only one triangle, since they are symmetric matrices)
		int nEntriesParticleCov = 7+6+5+4+3+2+1; //--> (7X7) Particle Matrix is symmetric, therefore no need to look at all elements
		int nEntriesShowerCov = 5+4+3+2+1; //--> (5X5) Shower Matrix is symmetric, therefore no need to look at all elements
		int nEntriesDecayCov = 11+10+9+8+7+6+5+4+3+2+1; //--> (11x11) Decaying Particle Matrix is symmetric, therefore no need to look at all elements

		//Save the elements of the error matrix to the tree: 
		void fillTreeErrMBranches(DTreeFillData* locTreeFillData,string yourBranchName,DKinFitType yourFitType, const DKinematicData* particle, const DNeutralShower* shower, bool isNeutral) const;
	
		//Fill tree for 7x7-Matrices:
		void fillTreeParticleErrMBranches(DTreeFillData* locTreeFillData,string yourBranchName, const DKinematicData* particle) const;
			
		 //Fill tree for 5x5-Matrices (showers):
		void fillTreeShowerErrMBranches(DTreeFillData* locTreeFillData,string yourBranchName, const DNeutralShower* shower) const;
	
		//GET TRACKING PULLS FROM KINEMATIC FITTER:q/pt, phi, tan(lambda), z, D
			
		//Get the difference in pull errors for the position
		vector< vector<double> > getSquaredErrX(const DKinematicData* particle, const DKinematicData* particleFit, map<DKinFitPullType, double> yourPullsMap) const;

		//Get the difference in pull errors for the momentum
		vector< vector<double> > getSquaredErrP(const DKinematicData* particle, const DKinematicData* particleFit, map<DKinFitPullType, double> yourPullsMap) const;

		//Get difference based on the momentum component:
		double getDiffSquaredErrP_Component(double yourRecComponent,double yourFitComponent,double yourPull) const;
	
		//Calculate the phi-pull:
		//Calculate the error (rec / fitted) first:
		double getPhiError(const DKinematicData* particle, vector< vector<double> > yourErrorMatrix, int isFitted) const;
	
		//--------------------------------------------------------
	
		//Now calculate the pull:
		double getPhiPull(const DKinematicData* particle, const DKinematicData* particleFit, vector< vector<double> > yourErrorMatrix) const;
	
		//Calculate tan(lambda)  pull:
		//Again, start with the error (i.e. handling derivatives) first:
		double getLambdaError(const DKinematicData* particle, vector< vector<double> > yourErrorMatrix, int isFitted) const;
	
		//--------------------------------------------------------
	
		//Get the pull:
		double getTanLambdaPull(const DKinematicData* particle, const DKinematicData* particleFit, vector< vector<double> > yourErrorMatrix) const;
	
		//Calculatze the q/pt pull:
		//Get the error from q/pt:
		double getQPTError(const DKinematicData* particle, vector< vector<double> > yourErrorMatrix, int isFitted) const;
		//Get the pull:
		double getQPTPull(const DKinematicData* particle, const DKinematicData* particleFit, vector< vector<double> > yourErrorMatrix) const;

		//Calulate the D pull
		//Get the error for D:
		double getDError(const DKinematicData* particle, vector< vector<double> > yourErrorMatrix, int isFitted) const;
		//Get pull:
		double getDPull(const DKinematicData* particle, const DKinematicData* particleFit, vector< vector<double> > yourErrorMatrix) const;

		//Collect the tracking pulls at once:
		vector<double> collectTrackingPulls(const DKinematicData* particle, const DKinematicData* particleFit, map<DKinFitPullType, double> yourPullsMap) const;

		//Fill the tracking pulls into the tree:
		void fillTreeTrackPullBranches(DTreeFillData* locTreeFillData,string yourBranchName,DKinFitType yourFitType,map<DKinFitPullType, double> yourPullsMap, int yourIndex, bool isNeutral, const DKinematicData* particle, const DKinematicData* particleFit) const;

};

inline string DEventWriterROOT::Convert_ToBranchName(string locInputName) const
{
	TString locTString(locInputName);
	locTString.ReplaceAll("*", "Star");
	locTString.ReplaceAll("(", "_");
	locTString.ReplaceAll(")", "_");
	locTString.ReplaceAll("+", "Plus");
	locTString.ReplaceAll("-", "Minus");
	locTString.ReplaceAll("'", "Prime");
	return (string)((const char*)locTString);
}

inline string DEventWriterROOT::Build_BranchName(string locParticleBranchName, string locVariableName) const
{
	return ((locParticleBranchName != "") ? locParticleBranchName + string("__") + locVariableName : locVariableName);
}

#endif //_DEventWriterROOT_
