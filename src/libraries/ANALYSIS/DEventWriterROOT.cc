#include "DEventWriterROOT.h"
#include "TObjString.h"

#include <TObjString.h>

static bool BCAL_VERBOSE_OUTPUT = false;
static bool FCAL_VERBOSE_OUTPUT = false;
static bool CCAL_VERBOSE_OUTPUT = false;
static bool DIRC_OUTPUT = true;
static bool FDC_VERBOSE_OUTPUT = true;

static bool STORE_PULL_INFO = false;
static bool STORE_ERROR_MATRIX_INFO = false;
static bool STORE_MC_TRAJECTORIES = false;

static bool STORE_SC_VETO_INFO = false;
static bool STORE_THROWN_DECAYING_PARTICLES = true;

void DEventWriterROOT::Initialize(const std::shared_ptr<const JEvent>& locEvent)
{
	dInitNumThrownArraySize = 20;
	dInitNumBeamArraySize = 20;
	dInitNumTrackArraySize = 50;
	dInitNumNeutralArraySize = 15;
	dInitNumComboArraySize = 100;
	dThrownTreeInterface = NULL;

	locEvent->GetSingle(dAnalysisUtilities);

	auto locReactions = DAnalysis::Get_Reactions(locEvent);

	//CREATE & INITIALIZE ANALYSIS ACTIONS
	for(size_t loc_i = 0; loc_i < locReactions.size(); ++loc_i)
	{
		if(!locReactions[loc_i]->Get_EnableTTreeOutputFlag())
			continue;

		dCutActionMap_ThrownTopology[locReactions[loc_i]] = new DCutAction_ThrownTopology(locReactions[loc_i], true);
		dCutActionMap_ThrownTopology[locReactions[loc_i]]->Initialize(locEvent);

		dCutActionMap_TrueCombo[locReactions[loc_i]] = new DCutAction_TrueCombo(locReactions[loc_i], -1.0, true);
		dCutActionMap_TrueCombo[locReactions[loc_i]]->Initialize(locEvent);

		dCutActionMap_BDTSignalCombo[locReactions[loc_i]] = new DCutAction_BDTSignalCombo(locReactions[loc_i], 5.73303E-7, true, true); //+/- 5sigma
		dCutActionMap_BDTSignalCombo[locReactions[loc_i]]->Initialize(locEvent);
	}

	//CREATE TREES
	vector<const DMCThrown*> locMCThrowns;
	locEvent->Get(locMCThrowns);

	vector<const DReactionVertexInfo*> locVertexInfos;
	locEvent->Get(locVertexInfos);

	//Get Target Center Z
	DGeometry* locGeometry = DEvent::GetDGeometry(locEvent);
	dTargetCenterZ = 65.0;
	locGeometry->GetTargetZ(dTargetCenterZ);

	// Get upstream positions of each FDC package
	vector<double>fdc_z_wires;
	locGeometry->GetFDCZ(fdc_z_wires);
	dFdcPackages[0]=fdc_z_wires[0]-1.; // just upstream
	dFdcPackages[1]=fdc_z_wires[6]-1.;
	dFdcPackages[2]=fdc_z_wires[12]-1.;
	dFdcPackages[3]=fdc_z_wires[18]-1.;

	//CREATE TTREES
	for(auto& locVertexInfo : locVertexInfos)
	{
		auto locVertexReactions = locVertexInfo->Get_Reactions();
		for(auto& locReaction : locVertexReactions)
		{
			dVertexInfoMap.emplace(locReaction, locVertexInfo);
			if(locReaction->Get_EnableTTreeOutputFlag())
				Create_DataTree(locReaction, locEvent, !locMCThrowns.empty());
		}
	}
}

void DEventWriterROOT::Run_Update(const std::shared_ptr<const JEvent>& locEvent)
{
	locEvent->GetSingle(dAnalysisUtilities);

	//Get Target Center Z
	DGeometry* locGeometry = DEvent::GetDGeometry(locEvent);
	dTargetCenterZ = 65.0;
	locGeometry->GetTargetZ(dTargetCenterZ);

	// update run-dependent info/objects
	for(auto& locMapPair : dCutActionMap_ThrownTopology)
		locMapPair.second->Run_Update(locEvent);
	for(auto& locMapPair : dCutActionMap_TrueCombo)
		locMapPair.second->Run_Update(locEvent);
	for(auto& locMapPair : dCutActionMap_BDTSignalCombo)
		locMapPair.second->Run_Update(locEvent);
}

DEventWriterROOT::~DEventWriterROOT(void)
{
	//Delete tree interface objects
	for(auto& locMapPair : dTreeInterfaceMap)
		delete locMapPair.second;
	if(dThrownTreeInterface != NULL)
		delete dThrownTreeInterface;

	for(auto& locMapPair : dTreeFillDataMap)
		delete locMapPair.second;

	//Delete actions
	for(auto& locMapPair : dCutActionMap_ThrownTopology)
		delete locMapPair.second;
	for(auto& locMapPair : dCutActionMap_TrueCombo)
		delete locMapPair.second;
	for(auto& locMapPair : dCutActionMap_BDTSignalCombo)
		delete locMapPair.second;
}

void DEventWriterROOT::Create_ThrownTree(const std::shared_ptr<const JEvent>& locEvent, string locOutputFileName) const
{
	if(dThrownTreeInterface != nullptr)
		return; //Already setup for this thread!
	dThrownTreeInterface = DTreeInterface::Create_DTreeInterface("Thrown_Tree", locOutputFileName); //set up this thread
	if(dThrownTreeInterface->Get_BranchesCreatedFlag())
		return; //branches already created: return

	// set parameters specifically for thrown trees
	// if(japp->Exists("ANALYSIS:STORE_THROWN_DECAYING_PARTICLES"))
	japp->GetParameter("ANALYSIS:STORE_THROWN_DECAYING_PARTICLES",STORE_THROWN_DECAYING_PARTICLES); cout << "ANALYSIS:STORE_THROWN_DECAYING_PARTICLES set to " << STORE_THROWN_DECAYING_PARTICLES << ", IGNORE the \"<-- NO DEFAULT! (TYPO?)\" message " << endl;

	//TTREE BRANCHES
	DTreeBranchRegister locBranchRegister;

	//Get target PID
	const DMCReaction* locMCReaction = NULL;
	locEvent->GetSingle(locMCReaction);
	Particle_t locTargetPID = locMCReaction->target.PID();

	//setup target info
	Create_UserTargetInfo(locBranchRegister, locTargetPID);

	//create basic/misc. tree branches (run#, event#, etc.)
	locBranchRegister.Register_Single<UInt_t>("RunNumber");
	locBranchRegister.Register_Single<ULong64_t>("EventNumber");

	//Thrown Data
	Create_Branches_Thrown(locBranchRegister, true);

	//CUSTOM
	Create_CustomBranches_ThrownTree(locBranchRegister, locEvent);

	//CREATE BRANCHES
	dThrownTreeInterface->Create_Branches(locBranchRegister);
	dThrownTreeInterface->Set_TreeIndexBranchNames("RunNumber", "EventNumber");
}

void DEventWriterROOT::Create_DataTree(const DReaction* locReaction, const std::shared_ptr<const JEvent>& locEvent, bool locIsMCDataFlag)
{
	string locReactionName = locReaction->Get_ReactionName();
	string locOutputFileName = locReaction->Get_TTreeOutputFileName();
	string locTreeName = locReactionName + string("_Tree");

	//create fill object
	dTreeFillDataMap[locReaction] = new DTreeFillData();

	//create tree interface
	DTreeInterface* locTreeInterface = DTreeInterface::Create_DTreeInterface(locTreeName, locOutputFileName);
	dTreeInterfaceMap[locReaction] = locTreeInterface;
	if(locTreeInterface->Get_BranchesCreatedFlag())
		return; //branches already created, then return

	//Branch register
	DTreeBranchRegister locBranchRegister;

	//fill maps
	TMap* locPositionToNameMap = Create_UserInfoMaps(locBranchRegister, locEvent, locReaction);

/******************************************************************** Create Branches ********************************************************************/

	//create basic/misc. tree branches (run#, event#, etc.)
	locBranchRegister.Register_Single<UInt_t>("RunNumber");
	locBranchRegister.Register_Single<ULong64_t>("EventNumber");
	locBranchRegister.Register_Single<UInt_t>("L1TriggerBits");
	locBranchRegister.Register_Single<Double_t>("L1BCALEnergy");
	locBranchRegister.Register_Single<Double_t>("L1FCALEnergy");

	//create X4_Production
	locBranchRegister.Register_Single<TLorentzVector>("X4_Production");

	//create thrown branches
	if(locIsMCDataFlag)
	{
		Create_Branches_Thrown(locBranchRegister, false);
		locBranchRegister.Register_Single<Bool_t>("IsThrownTopology");
	}

	bool locBeamUsedFlag = DAnalysis::Get_IsFirstStepBeam(locReaction);

	//create branches for final-state particle hypotheses
	if(locBeamUsedFlag)
		Create_Branches_Beam(locBranchRegister, locIsMCDataFlag);
	Create_Branches_NeutralHypotheses(locBranchRegister, locIsMCDataFlag);
	Create_Branches_ChargedHypotheses(locBranchRegister, locIsMCDataFlag);

	//create branches for combos
	locBranchRegister.Register_Single<UChar_t>("NumUnusedTracks");
	Create_Branches_Combo(locBranchRegister, locReaction, locIsMCDataFlag, locPositionToNameMap);

	//Kinematic fit data (pulls and covariance matrices)
	Create_Branches_KinFitData(locBranchRegister, locEvent, locReaction, locIsMCDataFlag);

	//Custom branches
	Create_CustomBranches_DataTree(locBranchRegister, locEvent, locReaction, locIsMCDataFlag);

	//Create branches
	locTreeInterface->Create_Branches(locBranchRegister);
	locTreeInterface->Set_TreeIndexBranchNames("RunNumber", "EventNumber");
}

TMap* DEventWriterROOT::Create_UserInfoMaps(DTreeBranchRegister& locBranchRegister, const std::shared_ptr<const JEvent>& locEvent, const DReaction* locReaction) const
{
	auto locReactionVertexInfo = dVertexInfoMap.find(locReaction)->second;

	//kinfit type
	DKinFitType locKinFitType = locReaction->Get_KinFitType();

	//create & add reaction identification maps
	TList* locUserInfo = locBranchRegister.Get_UserInfo();
	TMap* locNameToPIDMap = new TMap();
	locNameToPIDMap->SetName("NameToPIDMap");
	locUserInfo->Add(locNameToPIDMap);

	TMap* locNameToPositionMap = new TMap(); //not filled for target or initial particles
	locNameToPositionMap->SetName("NameToPositionMap");
	locUserInfo->Add(locNameToPositionMap);

	TMap* locPositionToNameMap = new TMap(); //not filled for target or initial particles
	locPositionToNameMap->SetName("PositionToNameMap");
	locUserInfo->Add(locPositionToNameMap);

	TMap* locPositionToPIDMap = new TMap();
	locPositionToPIDMap->SetName("PositionToPIDMap");
	locUserInfo->Add(locPositionToPIDMap);

	TMap* locDecayProductMap = new TMap(); //excludes resonances!!! //excludes intermediate decays (e.g. if xi- -> pi-, lambda -> pi-, pi-, p: will be xi- -> pi-, pi-, p and no lambda decay present)
	locDecayProductMap->SetName("DecayProductMap"); //parent name string -> tlist of decay product name strings
	locUserInfo->Add(locDecayProductMap);

	TMap* locMiscInfoMap = new TMap();
	locMiscInfoMap->SetName("MiscInfoMap");
	locUserInfo->Add(locMiscInfoMap);

	TList* locParticleNameList = new TList();
	locParticleNameList->SetName("ParticleNameList");
	locUserInfo->Add(locParticleNameList);

	//Set some misc info
	ostringstream locKinFitTypeStream;
	locKinFitTypeStream << locKinFitType;
	locMiscInfoMap->Add(new TObjString("KinFitType"), new TObjString(locKinFitTypeStream.str().c_str()));

	auto app = locEvent->GetJApplication();

	string ANALYSIS_VERSION_STRING = "";
	app->SetDefaultParameter("ANALYSIS:DATAVERSIONSTRING", ANALYSIS_VERSION_STRING);

	if(ANALYSIS_VERSION_STRING != "")
		locMiscInfoMap->Add(new TObjString("ANALYSIS:DATAVERSIONSTRING"), new TObjString(ANALYSIS_VERSION_STRING.c_str()));

	string HDDM_DATA_VERSION_STRING = "";
	app->SetDefaultParameter("REST:DATAVERSIONSTRING", HDDM_DATA_VERSION_STRING);

	if(HDDM_DATA_VERSION_STRING != "")
		locMiscInfoMap->Add(new TObjString("REST:DATAVERSIONSTRING"), new TObjString(HDDM_DATA_VERSION_STRING.c_str()));

	string REST_JANA_CALIB_CONTEXT = "";
	app->SetDefaultParameter("REST:JANACALIBCONTEXT", REST_JANA_CALIB_CONTEXT);

	if(REST_JANA_CALIB_CONTEXT == "")
		app->GetParameter("JANA_CALIB_CONTEXT", REST_JANA_CALIB_CONTEXT);

	if(REST_JANA_CALIB_CONTEXT != "")
		locMiscInfoMap->Add(new TObjString("REST:JANACALIBCONTEXT"), new TObjString(REST_JANA_CALIB_CONTEXT.c_str()));

	auto params = locEvent->GetJApplication()->GetJParameterManager();
	// Note: adding these parameters (e.g. in hd_root) will create warnings with "<-- NO DEFAULT! (TYPO?)". Safe to ignore these.
	if(params->Exists("ANALYSIS:FDC_VERBOSE_ROOT_OUTPUT"))
		{params->GetParameter("ANALYSIS:FDC_VERBOSE_ROOT_OUTPUT", FDC_VERBOSE_OUTPUT); cout << "ANALYSIS:FDC_VERBOSE_ROOT_OUTPUT set to " << FDC_VERBOSE_OUTPUT << ", IGNORE the \"<-- NO DEFAULT! (TYPO?)\" message " << endl;}
	if(params->Exists("ANALYSIS:BCAL_VERBOSE_ROOT_OUTPUT"))
		{params->GetParameter("ANALYSIS:BCAL_VERBOSE_ROOT_OUTPUT", BCAL_VERBOSE_OUTPUT); cout << "ANALYSIS:BCAL_VERBOSE_ROOT_OUTPUT set to " << BCAL_VERBOSE_OUTPUT << ", IGNORE the \"<-- NO DEFAULT! (TYPO?)\" message " << endl;}
	if(params->Exists("ANALYSIS:FCAL_VERBOSE_ROOT_OUTPUT"))
		{params->GetParameter("ANALYSIS:FCAL_VERBOSE_ROOT_OUTPUT", FCAL_VERBOSE_OUTPUT); cout << "ANALYSIS:FCAL_VERBOSE_ROOT_OUTPUT set to " << FCAL_VERBOSE_OUTPUT << ", IGNORE the \"<-- NO DEFAULT! (TYPO?)\" message " << endl;}
	if(params->Exists("ANALYSIS:CCAL_VERBOSE_ROOT_OUTPUT"))
		{params->GetParameter("ANALYSIS:CCAL_VERBOSE_ROOT_OUTPUT", CCAL_VERBOSE_OUTPUT); cout << "ANALYSIS:CCAL_VERBOSE_ROOT_OUTPUT set to " << CCAL_VERBOSE_OUTPUT << ", IGNORE the \"<-- NO DEFAULT! (TYPO?)\" message " << endl;}
	if(params->Exists("ANALYSIS:DIRC_ROOT_OUTPUT"))
		{params->GetParameter("ANALYSIS:DIRC_ROOT_OUTPUT", DIRC_OUTPUT); cout << "ANALYSIS:DIRC_ROOT_OUTPUT set to " << DIRC_OUTPUT << ", IGNORE the \"<-- NO DEFAULT! (TYPO?)\" message " << endl;}
	if(params->Exists("ANALYSIS:STORE_PULL_INFO"))
	    {params->GetParameter("ANALYSIS:STORE_PULL_INFO",STORE_PULL_INFO); cout << "ANALYSIS:STORE_PULL_INFO set to " << STORE_PULL_INFO << ", IGNORE the \"<-- NO DEFAULT! (TYPO?)\" message " << endl;}
	if(params->Exists("ANALYSIS:STORE_ERROR_MATRIX_INFO"))
    	{params->GetParameter("ANALYSIS:STORE_ERROR_MATRIX_INFO",STORE_ERROR_MATRIX_INFO); cout << "ANALYSIS:STORE_ERROR_MATRIX_INFO set to " << STORE_ERROR_MATRIX_INFO << ", IGNORE the \"<-- NO DEFAULT! (TYPO?)\" message " << endl;}
	if(params->Exists("ANALYSIS:STORE_MC_TRAJECTORIES"))
		{params->GetParameter("ANALYSIS:STORE_MC_TRAJECTORIES",STORE_MC_TRAJECTORIES); cout << "ANALYSIS:STORE_MC_TRAJECTORIES set to " << STORE_MC_TRAJECTORIES << ", IGNORE the \"<-- NO DEFAULT! (TYPO?)\" message " << endl;}

	if(params->Exists("ANALYSIS:STORE_SC_VETO_INFO"))
	    {params->GetParameter("ANALYSIS:STORE_SC_VETO_INFO",STORE_SC_VETO_INFO); cout << "ANALYSIS:STORE_SC_VETO_INFO set to " << STORE_SC_VETO_INFO << ", IGNORE the \"<-- NO DEFAULT! (TYPO?)\" message " << endl;}


	if(locKinFitType != d_NoFit)
	{
		DKinFitUtils_GlueX locKinFitUtils(locEvent);
		size_t locNumConstraints = 0, locNumUnknowns = 0;
		string locConstraintString = locKinFitUtils.Get_ConstraintInfo(locReactionVertexInfo, locReaction, locNumConstraints, locNumUnknowns);
		locMiscInfoMap->Add(new TObjString("KinFitConstraints"), new TObjString(locConstraintString.c_str()));

		ostringstream locKinFitInfoStream;
		locKinFitInfoStream << locNumConstraints;
		locMiscInfoMap->Add(new TObjString("NumKinFitConstraints"), new TObjString(locKinFitInfoStream.str().c_str()));

		locKinFitInfoStream.str("");
		locKinFitInfoStream << locNumUnknowns;
		locMiscInfoMap->Add(new TObjString("NumKinFitUnknowns"), new TObjString(locKinFitInfoStream.str().c_str()));
	}

	//find the # particles of each pid
	map<Particle_t, unsigned int> locParticleNumberMap;
	map<Particle_t, unsigned int> locDecayingParticleNumberMap;
	map<Particle_t, unsigned int> locTargetParticleNumberMap;
	for(size_t loc_i = 0; loc_i < locReaction->Get_NumReactionSteps(); ++loc_i)
	{
		const DReactionStep* locReactionStep = locReaction->Get_ReactionStep(loc_i);
		auto locFinalParticleIDs = locReactionStep->Get_FinalPIDs();

		auto locTargetPID = locReactionStep->Get_TargetPID();
		if(locTargetPID != Unknown)
		{
			if(locTargetParticleNumberMap.find(locTargetPID) == locTargetParticleNumberMap.end())
				locTargetParticleNumberMap[locTargetPID] = 1;
			else
				++locTargetParticleNumberMap[locTargetPID];
		}

		for(size_t loc_j = 0; loc_j < locFinalParticleIDs.size(); ++loc_j)
		{
			if(locReactionStep->Get_MissingParticleIndex() == int(loc_j)) //missing particle
				continue;
			Particle_t locPID = locFinalParticleIDs[loc_j];

			//see if decays in a future step
			int locDecayStepIndex = DAnalysis::Get_DecayStepIndex(locReaction, loc_i, loc_j);
			if(locDecayStepIndex >= 0) //decaying
			{
				if(locDecayingParticleNumberMap.find(locPID) == locDecayingParticleNumberMap.end())
					locDecayingParticleNumberMap[locPID] = 1;
				else
					++locDecayingParticleNumberMap[locPID];
			}
			else //detected, not decaying
			{
				if(locParticleNumberMap.find(locPID) == locParticleNumberMap.end())
					locParticleNumberMap[locPID] = 1;
				else
					++locParticleNumberMap[locPID];
			}
		}
	}

	//Create map objects
	map<Particle_t, unsigned int> locParticleNumberMap_Current, locDecayingParticleNumberMap_Current, locTargetParticleNumberMap_Current;
	Particle_t locTargetPID = Unknown;
	TObjString *locObjString_PID, *locObjString_Position, *locObjString_ParticleName;
	map<int, string> locDecayingParticleNames; //key is step index where they decay
	for(size_t loc_i = 0; loc_i < locReaction->Get_NumReactionSteps(); ++loc_i)
	{
		const DReactionStep* locReactionStep = locReaction->Get_ReactionStep(loc_i);

		//initial particle
		{
			ostringstream locPIDStream, locPositionStream, locParticleNameStream;
			Particle_t locPID = locReactionStep->Get_InitialPID();
			locPIDStream << PDGtype(locPID);
			locObjString_PID = new TObjString(locPIDStream.str().c_str());

			locPositionStream << loc_i << "_" << -1;
			locObjString_Position = new TObjString(locPositionStream.str().c_str());

			locPositionToPIDMap->Add(locObjString_Position, locObjString_PID);
			if((loc_i == 0) && ((locPID == Gamma) || (locPID == Electron) || (locPID == Positron)))
			{
				locParticleNameStream << "ComboBeam";
				locObjString_ParticleName = new TObjString(locParticleNameStream.str().c_str());
				locNameToPIDMap->Add(locObjString_ParticleName, locObjString_PID);
				locParticleNameList->AddLast(locObjString_ParticleName);
			}
			else //decaying particle
			{
				if(loc_i == 0)
					locParticleNameStream << "Decaying" << Convert_ToBranchName(ParticleType(locPID));
				else //name already created for final particle: use it
					locParticleNameStream << locDecayingParticleNames[loc_i];
				locObjString_ParticleName = new TObjString(locParticleNameStream.str().c_str());
				if(loc_i == 0) //in first step
				{
					locNameToPIDMap->Add(locObjString_ParticleName, locObjString_PID);
					locParticleNameList->AddLast(locObjString_ParticleName);
				}
			}
			locPositionToNameMap->Add(locObjString_Position, locObjString_ParticleName);
			locNameToPositionMap->Add(locObjString_ParticleName, locObjString_Position);
		}

		//target particle
		Particle_t locTempTargetPID = locReactionStep->Get_TargetPID();
		if(locTempTargetPID != Unknown)
		{
			locTargetPID = locTempTargetPID;

			if(locTargetParticleNumberMap_Current.find(locTargetPID) == locTargetParticleNumberMap_Current.end())
				locTargetParticleNumberMap_Current[locTargetPID] = 1;
			else
				++locTargetParticleNumberMap_Current[locTargetPID];

			ostringstream locPIDStream, locPositionStream, locParticleNameStream;
			locPIDStream << PDGtype(locTargetPID);
			locObjString_PID = new TObjString(locPIDStream.str().c_str());

			locPositionStream << loc_i << "_" << -2;
			locObjString_Position = new TObjString(locPositionStream.str().c_str());

			locPositionToPIDMap->Add(locObjString_Position, locObjString_PID);

			locParticleNameStream << "Target";
			if(locTargetParticleNumberMap[locTargetPID] > 1)
				locParticleNameStream << locTargetParticleNumberMap_Current[locTargetPID];
			locObjString_ParticleName = new TObjString(locParticleNameStream.str().c_str());

			locNameToPositionMap->Add(locObjString_ParticleName, locObjString_Position);
			locNameToPIDMap->Add(locObjString_ParticleName, locObjString_PID);
			locPositionToNameMap->Add(locObjString_Position, locObjString_ParticleName);

			locParticleNameList->AddLast(locObjString_ParticleName);
		}

		//final particles
		auto locFinalParticleIDs = locReactionStep->Get_FinalPIDs();
		for(size_t loc_j = 0; loc_j < locFinalParticleIDs.size(); ++loc_j)
		{
			ostringstream locPIDStream, locPositionStream;
			Particle_t locPID = locFinalParticleIDs[loc_j];
			locPIDStream << PDGtype(locPID);
			locObjString_PID = new TObjString(locPIDStream.str().c_str());

			locPositionStream << loc_i << "_" << loc_j;
			locObjString_Position = new TObjString(locPositionStream.str().c_str());

			if(locReactionStep->Get_MissingParticleIndex() == int(loc_j)) //missing particle
			{
				ostringstream locParticleNameStream;
				locParticleNameStream << "Missing" << Convert_ToBranchName(ParticleType(locPID));
				locObjString_ParticleName = new TObjString(locParticleNameStream.str().c_str());
				locNameToPositionMap->Add(locObjString_ParticleName, locObjString_Position);
				locPositionToNameMap->Add(locObjString_Position, locObjString_ParticleName);
				locNameToPIDMap->Add(locObjString_ParticleName, locObjString_PID);
				string locPIDName = locParticleNameStream.str() + string("__PID");
				locMiscInfoMap->Add(new TObjString(locPIDName.c_str()), locObjString_PID);
				ostringstream locMassStream;
				locMassStream << ParticleMass(locPID);
				string locMassName = locParticleNameStream.str() + string("__Mass");
				locMiscInfoMap->Add(new TObjString(locMassName.c_str()), new TObjString(locMassStream.str().c_str()));
				locParticleNameList->AddLast(locObjString_ParticleName);
				continue;
			}

			//build name
			ostringstream locParticleNameStream;
			//see if decays in a future step
			int locDecayStepIndex = DAnalysis::Get_DecayStepIndex(locReaction, loc_i, loc_j);
			if(locDecayStepIndex >= 0) //decays
			{
				if(locDecayingParticleNumberMap_Current.find(locPID) == locDecayingParticleNumberMap_Current.end())
					locDecayingParticleNumberMap_Current[locPID] = 1;
				else
					++locDecayingParticleNumberMap_Current[locPID];

				locParticleNameStream << "Decaying" << Convert_ToBranchName(ParticleType(locPID));
				if(locDecayingParticleNumberMap[locPID] > 1)
					locParticleNameStream << locDecayingParticleNumberMap_Current[locPID];
				locDecayingParticleNames[locDecayStepIndex] = locParticleNameStream.str();
			}
			else
			{
				if(locParticleNumberMap_Current.find(locPID) == locParticleNumberMap_Current.end())
					locParticleNumberMap_Current[locPID] = 1;
				else
					++locParticleNumberMap_Current[locPID];

				locParticleNameStream << Convert_ToBranchName(ParticleType(locPID));
				if(locParticleNumberMap[locPID] > 1)
					locParticleNameStream << locParticleNumberMap_Current[locPID];
			}

			locObjString_ParticleName = new TObjString(locParticleNameStream.str().c_str());
			locParticleNameList->AddLast(locObjString_ParticleName);

			locPositionToPIDMap->Add(locObjString_Position, locObjString_PID);
			locNameToPositionMap->Add(locObjString_ParticleName, locObjString_Position);
			locPositionToNameMap->Add(locObjString_Position, locObjString_ParticleName);
			locNameToPIDMap->Add(locObjString_ParticleName, locObjString_PID);
			if(locDecayStepIndex >= 0)
			{
				ostringstream locMassStream;
				locMassStream << ParticleMass(locPID);
				string locMassName = locParticleNameStream.str() + string("__Mass");
				locMiscInfoMap->Add(new TObjString(locMassName.c_str()), new TObjString(locMassStream.str().c_str()));
			}
		}
	}

	//setup target info
	Create_UserTargetInfo(locBranchRegister, locTargetPID);

	//fill decay product map
	deque<size_t> locSavedSteps;
	for(size_t loc_i = 0; loc_i < locReaction->Get_NumReactionSteps(); ++loc_i)
	{
		const DReactionStep* locReactionStep = locReaction->Get_ReactionStep(loc_i);

		//initial particle
		Particle_t locPID = locReactionStep->Get_InitialPID();
		if(loc_i == 0)
			continue;
		if(!IsFixedMass(locPID))
			continue; //don't save resonance decays to the map

		//check to see if this decay has already been saved
		bool locStepAlreadySavedFlag = false;
		for(size_t loc_j = 0; loc_j < locSavedSteps.size(); ++loc_j)
		{
			if(locSavedSteps[loc_j] != loc_i)
				continue;
			locStepAlreadySavedFlag = true;
			break;
		}
		if(locStepAlreadySavedFlag)
			continue;

		//construct the name 
		ostringstream locParticleNameStream;
		locParticleNameStream << locDecayingParticleNames[loc_i];
		locObjString_ParticleName = new TObjString(locParticleNameStream.str().c_str());

		TList* locDecayProductNames = NULL;
		Get_DecayProductNames(locReaction, loc_i, locPositionToNameMap, locDecayProductNames, locSavedSteps);
		locDecayProductMap->Add(locObjString_ParticleName, locDecayProductNames); //parent name string -> tobjarray of decay product name strings		
	}

	return locPositionToNameMap;
}

void DEventWriterROOT::Get_DecayProductNames(const DReaction* locReaction, size_t locReactionStepIndex, TMap* locPositionToNameMap, TList*& locDecayProductNames, deque<size_t>& locSavedSteps) const
{
	const DReactionStep* locReactionStep = locReaction->Get_ReactionStep(locReactionStepIndex);

	if(locDecayProductNames == NULL)
		locDecayProductNames = new TList();

	auto locFinalParticleIDs = locReactionStep->Get_FinalPIDs();
	for(size_t loc_j = 0; loc_j < locFinalParticleIDs.size(); ++loc_j)
	{
		//see if decays in a future step //save and continue if doesn't decay
		int locDecayStepIndex = DAnalysis::Get_DecayStepIndex(locReaction, locReactionStepIndex, loc_j);
		if(locDecayStepIndex < 0)
		{
			ostringstream locPositionStream;
			locPositionStream << locReactionStepIndex << "_" << loc_j;
			locDecayProductNames->AddLast(locPositionToNameMap->GetValue(locPositionStream.str().c_str()));
			continue;
		}

		//add decay products
		Get_DecayProductNames(locReaction, locDecayStepIndex, locPositionToNameMap, locDecayProductNames, locSavedSteps);
	}

	locSavedSteps.push_back(locReactionStepIndex);
}

void DEventWriterROOT::Create_UserTargetInfo(DTreeBranchRegister& locBranchRegister, Particle_t locTargetPID) const
{
	TList* locUserInfo = locBranchRegister.Get_UserInfo();
	TMap* locMiscInfoMap = (TMap*)locUserInfo->FindObject("MiscInfoMap");
	if(locMiscInfoMap == NULL)
	{
		locMiscInfoMap = new TMap();
		locMiscInfoMap->SetName("MiscInfoMap");
		locUserInfo->Add(locMiscInfoMap);
	}

	//PID
	ostringstream locPIDStream;
	locPIDStream << PDGtype(locTargetPID);
	locMiscInfoMap->Add(new TObjString("Target__PID"), new TObjString(locPIDStream.str().c_str()));

	//Mass
	ostringstream locMassStream;
	locMassStream << ParticleMass(locTargetPID);
	locMiscInfoMap->Add(new TObjString("Target__Mass"), new TObjString(locMassStream.str().c_str()));

	//X, Y
	ostringstream locZeroStream;
	locZeroStream << 0.0;
	TObjString* locObjString_Zero = new TObjString(locZeroStream.str().c_str());
	locMiscInfoMap->Add(new TObjString("Target__CenterX"), locObjString_Zero);
	locMiscInfoMap->Add(new TObjString("Target__CenterY"), locObjString_Zero);

	//Z
	ostringstream locPositionStream;
	locPositionStream << dTargetCenterZ;
	TObjString* locObjString_Position = new TObjString(locPositionStream.str().c_str());
	locMiscInfoMap->Add(new TObjString("Target__CenterZ"), locObjString_Position);
}

void DEventWriterROOT::Create_Branches_Thrown(DTreeBranchRegister& locBranchRegister, bool locIsOnlyThrownFlag) const
{
	//BEAM
	locBranchRegister.Register_Single<Int_t>(Build_BranchName("ThrownBeam", "PID"));
	locBranchRegister.Register_Single<TLorentzVector>(Build_BranchName("ThrownBeam", "X4")); //reported at target center
	locBranchRegister.Register_Single<TLorentzVector>(Build_BranchName("ThrownBeam", "P4"));
	locBranchRegister.Register_Single<Float_t>(Build_BranchName("ThrownBeam", "GeneratedEnergy"));

	//EVENT-WIDE INFO
	locBranchRegister.Register_Single<ULong64_t>("NumPIDThrown_FinalState"); //19 digits
	locBranchRegister.Register_Single<ULong64_t>("PIDThrown_Decaying"); //19 digits
	locBranchRegister.Register_Single<Float_t>("MCWeight");

	//PRODUCTS
	Create_Branches_ThrownParticles(locBranchRegister, locIsOnlyThrownFlag);
}

void DEventWriterROOT::Create_Branches_ThrownParticles(DTreeBranchRegister& locBranchRegister, bool locIsOnlyThrownFlag) const
{
	string locParticleBranchName = "Thrown";

	string locArraySizeString = "NumThrown";
	locBranchRegister.Register_Single<UInt_t>(locArraySizeString);

	//IDENTIFIERS
	locBranchRegister.Register_FundamentalArray<Int_t>(Build_BranchName(locParticleBranchName, "ParentIndex"), locArraySizeString, dInitNumThrownArraySize);
	locBranchRegister.Register_FundamentalArray<Int_t>(Build_BranchName(locParticleBranchName, "PID"), locArraySizeString, dInitNumThrownArraySize);
	if(!locIsOnlyThrownFlag)
	{
		locBranchRegister.Register_FundamentalArray<Int_t>(Build_BranchName(locParticleBranchName, "MatchID"), locArraySizeString, dInitNumThrownArraySize);
		locBranchRegister.Register_FundamentalArray<Float_t>(Build_BranchName(locParticleBranchName, "MatchFOM"), locArraySizeString, dInitNumThrownArraySize);
	}

	//KINEMATICS: THROWN //at the production vertex
	locBranchRegister.Register_ClonesArray<TLorentzVector>(Build_BranchName(locParticleBranchName, "X4"), dInitNumThrownArraySize);
	locBranchRegister.Register_ClonesArray<TLorentzVector>(Build_BranchName(locParticleBranchName, "P4"), dInitNumThrownArraySize);

	if(STORE_MC_TRAJECTORIES) {
		// Particles at point of creation (i.e. birth)
		locBranchRegister.Register_FundamentalArray<Int_t>(Build_BranchName(locParticleBranchName, "PID_trajBirth"), locArraySizeString, dInitNumThrownArraySize);
		locBranchRegister.Register_ClonesArray<TLorentzVector>(Build_BranchName(locParticleBranchName, "X4_trajBirth"), dInitNumThrownArraySize);
		locBranchRegister.Register_ClonesArray<TLorentzVector>(Build_BranchName(locParticleBranchName, "P4_trajBirth"), dInitNumThrownArraySize);
		// Particles at point of interaction (sometimes called "death", but this may be misleading for tracks: they can survive interactions)
		locBranchRegister.Register_FundamentalArray<Int_t>(Build_BranchName(locParticleBranchName, "PID_trajDeath"), locArraySizeString, dInitNumThrownArraySize);
		locBranchRegister.Register_ClonesArray<TLorentzVector>(Build_BranchName(locParticleBranchName, "X4_trajDeath"), dInitNumThrownArraySize);
		// we skip P4 of particle at death, as DMCTrajectoryPoints just saves (0,0,0,0) which isn't particularly enlightening
	}
}

void DEventWriterROOT::Create_Branches_Beam(DTreeBranchRegister& locBranchRegister, bool locIsMCDataFlag) const
{
	string locArraySizeString = "NumBeam";
	locBranchRegister.Register_Single<UInt_t>(locArraySizeString);

	string locParticleBranchName = "Beam";

	//IDENTIFIER
	locBranchRegister.Register_FundamentalArray<Int_t>(Build_BranchName(locParticleBranchName, "PID"), locArraySizeString, dInitNumBeamArraySize);
	if(locIsMCDataFlag)
		locBranchRegister.Register_FundamentalArray<Bool_t>(Build_BranchName(locParticleBranchName, "IsGenerator"), locArraySizeString, dInitNumBeamArraySize);

	//KINEMATICS: MEASURED //at the production vertex
	locBranchRegister.Register_ClonesArray<TLorentzVector>(Build_BranchName(locParticleBranchName, "X4_Measured"), dInitNumThrownArraySize);
	locBranchRegister.Register_ClonesArray<TLorentzVector>(Build_BranchName(locParticleBranchName, "P4_Measured"), dInitNumThrownArraySize);
}

void DEventWriterROOT::Create_Branches_ChargedHypotheses(DTreeBranchRegister& locBranchRegister, bool locIsMCDataFlag) const
{
	string locArraySizeString = "NumChargedHypos";
	locBranchRegister.Register_Single<UInt_t>(locArraySizeString);

	string locParticleBranchName = "ChargedHypo";

	//IDENTIFIERS / MATCHING
	locBranchRegister.Register_FundamentalArray<Int_t>(Build_BranchName(locParticleBranchName, "TrackID"), locArraySizeString, dInitNumTrackArraySize);
	locBranchRegister.Register_FundamentalArray<Int_t>(Build_BranchName(locParticleBranchName, "PID"), locArraySizeString, dInitNumTrackArraySize);
	if(locIsMCDataFlag)
		locBranchRegister.Register_FundamentalArray<Int_t>(Build_BranchName(locParticleBranchName, "ThrownIndex"), locArraySizeString, dInitNumTrackArraySize);

	//KINEMATICS //at the production vertex
	locBranchRegister.Register_ClonesArray<TLorentzVector>(Build_BranchName(locParticleBranchName, "X4_Measured"), dInitNumTrackArraySize);
	locBranchRegister.Register_ClonesArray<TLorentzVector>(Build_BranchName(locParticleBranchName, "P4_Measured"), dInitNumTrackArraySize);

	// Global PID
	locBranchRegister.Register_FundamentalArray<Float_t>(Build_BranchName(locParticleBranchName, "PIDFOM"), locArraySizeString, dInitNumTrackArraySize);

	//TRACKING INFO
	locBranchRegister.Register_FundamentalArray<UInt_t>(Build_BranchName(locParticleBranchName, "NDF_Tracking"), locArraySizeString, dInitNumTrackArraySize);
	locBranchRegister.Register_FundamentalArray<Float_t>(Build_BranchName(locParticleBranchName, "ChiSq_Tracking"), locArraySizeString, dInitNumTrackArraySize);
	locBranchRegister.Register_FundamentalArray<UInt_t>(Build_BranchName(locParticleBranchName, "NDF_DCdEdx"), locArraySizeString, dInitNumTrackArraySize);
	locBranchRegister.Register_FundamentalArray<Float_t>(Build_BranchName(locParticleBranchName, "ChiSq_DCdEdx"), locArraySizeString, dInitNumTrackArraySize);
	locBranchRegister.Register_FundamentalArray<Float_t>(Build_BranchName(locParticleBranchName, "dEdx_CDC"), locArraySizeString, dInitNumTrackArraySize);
	locBranchRegister.Register_FundamentalArray<Float_t>(Build_BranchName(locParticleBranchName, "dEdx_CDC_integral"), locArraySizeString, dInitNumTrackArraySize);
	locBranchRegister.Register_FundamentalArray<Float_t>(Build_BranchName(locParticleBranchName, "dEdx_FDC"), locArraySizeString, dInitNumTrackArraySize);
	if (FDC_VERBOSE_OUTPUT){
	  for (unsigned int j=0;j<4;j++){
	    locBranchRegister.Register_FundamentalArray<Float_t>(Build_BranchName(locParticleBranchName, dFDCxLeaves[j].c_str()), locArraySizeString, dInitNumTrackArraySize); 
	    locBranchRegister.Register_FundamentalArray<Float_t>(Build_BranchName(locParticleBranchName, dFDCyLeaves[j].c_str()), locArraySizeString, dInitNumTrackArraySize);  
	  }
	}

	//TIMING INFO
	locBranchRegister.Register_FundamentalArray<Float_t>(Build_BranchName(locParticleBranchName, "HitTime"), locArraySizeString, dInitNumTrackArraySize);
	locBranchRegister.Register_FundamentalArray<Float_t>(Build_BranchName(locParticleBranchName, "RFDeltaTVar"), locArraySizeString, dInitNumTrackArraySize);

	//PID QUALITY
	locBranchRegister.Register_FundamentalArray<Float_t>(Build_BranchName(locParticleBranchName, "Beta_Timing"), locArraySizeString, dInitNumTrackArraySize);
	locBranchRegister.Register_FundamentalArray<Float_t>(Build_BranchName(locParticleBranchName, "ChiSq_Timing"), locArraySizeString, dInitNumTrackArraySize);
	locBranchRegister.Register_FundamentalArray<UInt_t>(Build_BranchName(locParticleBranchName, "NDF_Timing"), locArraySizeString, dInitNumTrackArraySize);

	//HIT ENERGY
	locBranchRegister.Register_FundamentalArray<Float_t>(Build_BranchName(locParticleBranchName, "dEdx_TOF"), locArraySizeString, dInitNumTrackArraySize);
	locBranchRegister.Register_FundamentalArray<Float_t>(Build_BranchName(locParticleBranchName, "dEdx_ST"), locArraySizeString, dInitNumTrackArraySize);
	locBranchRegister.Register_FundamentalArray<Float_t>(Build_BranchName(locParticleBranchName, "Energy_BCAL"), locArraySizeString, dInitNumNeutralArraySize);
	locBranchRegister.Register_FundamentalArray<Float_t>(Build_BranchName(locParticleBranchName, "Energy_BCALPreshower"), locArraySizeString, dInitNumNeutralArraySize);
	if(BCAL_VERBOSE_OUTPUT) {
		locBranchRegister.Register_FundamentalArray<Float_t>(Build_BranchName(locParticleBranchName, "Energy_BCALLayer2"), locArraySizeString, dInitNumNeutralArraySize);
		locBranchRegister.Register_FundamentalArray<Float_t>(Build_BranchName(locParticleBranchName, "Energy_BCALLayer3"), locArraySizeString, dInitNumNeutralArraySize);
		locBranchRegister.Register_FundamentalArray<Float_t>(Build_BranchName(locParticleBranchName, "Energy_BCALLayer4"), locArraySizeString, dInitNumNeutralArraySize);
	}
	locBranchRegister.Register_FundamentalArray<Float_t>(Build_BranchName(locParticleBranchName, "SigLong_BCAL"), locArraySizeString, dInitNumNeutralArraySize);
	locBranchRegister.Register_FundamentalArray<Float_t>(Build_BranchName(locParticleBranchName, "SigTheta_BCAL"), locArraySizeString, dInitNumNeutralArraySize);
	locBranchRegister.Register_FundamentalArray<Float_t>(Build_BranchName(locParticleBranchName, "SigTrans_BCAL"), locArraySizeString, dInitNumNeutralArraySize);
	locBranchRegister.Register_FundamentalArray<Float_t>(Build_BranchName(locParticleBranchName, "RMSTime_BCAL"), locArraySizeString, dInitNumNeutralArraySize);
	locBranchRegister.Register_FundamentalArray<Float_t>(Build_BranchName(locParticleBranchName, "Energy_FCAL"), locArraySizeString, dInitNumNeutralArraySize);
	locBranchRegister.Register_FundamentalArray<Float_t>(Build_BranchName(locParticleBranchName, "E1E9_FCAL"), locArraySizeString, dInitNumNeutralArraySize);
	locBranchRegister.Register_FundamentalArray<Float_t>(Build_BranchName(locParticleBranchName, "E9E25_FCAL"), locArraySizeString, dInitNumNeutralArraySize);
	locBranchRegister.Register_FundamentalArray<Float_t>(Build_BranchName(locParticleBranchName, "SumU_FCAL"), locArraySizeString, dInitNumNeutralArraySize);
	locBranchRegister.Register_FundamentalArray<Float_t>(Build_BranchName(locParticleBranchName, "SumV_FCAL"), locArraySizeString, dInitNumNeutralArraySize);
	locBranchRegister.Register_FundamentalArray<Float_t>(Build_BranchName(locParticleBranchName, "NumBlocks_FCAL"), locArraySizeString, dInitNumNeutralArraySize);
	
	locBranchRegister.Register_FundamentalArray<Float_t>(Build_BranchName(locParticleBranchName, "Energy_CCAL"), locArraySizeString, dInitNumNeutralArraySize);


	//SHOWER MATCHING:
	locBranchRegister.Register_FundamentalArray<Float_t>(Build_BranchName(locParticleBranchName, "TrackBCAL_DeltaPhi"), locArraySizeString, dInitNumTrackArraySize);
	locBranchRegister.Register_FundamentalArray<Float_t>(Build_BranchName(locParticleBranchName, "TrackBCAL_DeltaZ"), locArraySizeString, dInitNumTrackArraySize);
	locBranchRegister.Register_FundamentalArray<Float_t>(Build_BranchName(locParticleBranchName, "TrackFCAL_DOCA"), locArraySizeString, dInitNumTrackArraySize);
	locBranchRegister.Register_FundamentalArray<Float_t>(Build_BranchName(locParticleBranchName, "TrackFCAL_DeltaT"), locArraySizeString, dInitNumTrackArraySize);

	//DIRC:
	if(DIRC_OUTPUT) {
		locBranchRegister.Register_FundamentalArray<Int_t>(Build_BranchName(locParticleBranchName, "NumPhotons_DIRC"), locArraySizeString, dInitNumTrackArraySize);
		locBranchRegister.Register_FundamentalArray<Float_t>(Build_BranchName(locParticleBranchName, "ExtrapolatedX_DIRC"), locArraySizeString, dInitNumTrackArraySize);
		locBranchRegister.Register_FundamentalArray<Float_t>(Build_BranchName(locParticleBranchName, "ExtrapolatedY_DIRC"), locArraySizeString, dInitNumTrackArraySize);
		locBranchRegister.Register_FundamentalArray<Float_t>(Build_BranchName(locParticleBranchName, "ThetaC_DIRC"), locArraySizeString, dInitNumTrackArraySize);
		locBranchRegister.Register_FundamentalArray<Float_t>(Build_BranchName(locParticleBranchName, "Lele_DIRC"), locArraySizeString, dInitNumTrackArraySize);
		locBranchRegister.Register_FundamentalArray<Float_t>(Build_BranchName(locParticleBranchName, "Lpi_DIRC"), locArraySizeString, dInitNumTrackArraySize);
		locBranchRegister.Register_FundamentalArray<Float_t>(Build_BranchName(locParticleBranchName, "Lk_DIRC"), locArraySizeString, dInitNumTrackArraySize);
		locBranchRegister.Register_FundamentalArray<Float_t>(Build_BranchName(locParticleBranchName, "Lp_DIRC"), locArraySizeString, dInitNumTrackArraySize);
	}
}

void DEventWriterROOT::Create_Branches_NeutralHypotheses(DTreeBranchRegister& locBranchRegister, bool locIsMCDataFlag) const
{
	string locArraySizeString = "NumNeutralHypos";
	string locParticleBranchName = "NeutralHypo";
	locBranchRegister.Register_Single<UInt_t>(locArraySizeString);

	//IDENTIFIERS / MATCHING
	locBranchRegister.Register_FundamentalArray<Int_t>(Build_BranchName(locParticleBranchName, "NeutralID"), locArraySizeString, dInitNumNeutralArraySize);
	locBranchRegister.Register_FundamentalArray<Int_t>(Build_BranchName(locParticleBranchName, "PID"), locArraySizeString, dInitNumNeutralArraySize);
	if(locIsMCDataFlag)
		locBranchRegister.Register_FundamentalArray<Int_t>(Build_BranchName(locParticleBranchName, "ThrownIndex"), locArraySizeString, dInitNumNeutralArraySize);

	//KINEMATICS //is combo-dependent: P4 defined by X4, X4 defined by other tracks
	locBranchRegister.Register_ClonesArray<TLorentzVector>(Build_BranchName(locParticleBranchName, "X4_Measured"), dInitNumNeutralArraySize);
	locBranchRegister.Register_ClonesArray<TLorentzVector>(Build_BranchName(locParticleBranchName, "P4_Measured"), dInitNumNeutralArraySize);
	
	//PID QUALITY
	locBranchRegister.Register_FundamentalArray<Float_t>(Build_BranchName(locParticleBranchName, "Beta_Timing"), locArraySizeString, dInitNumNeutralArraySize);
	locBranchRegister.Register_FundamentalArray<Float_t>(Build_BranchName(locParticleBranchName, "ChiSq_Timing"), locArraySizeString, dInitNumNeutralArraySize);
	locBranchRegister.Register_FundamentalArray<UInt_t>(Build_BranchName(locParticleBranchName, "NDF_Timing"), locArraySizeString, dInitNumNeutralArraySize);

	//SHOWER INFO
	locBranchRegister.Register_ClonesArray<TLorentzVector>(Build_BranchName(locParticleBranchName, "X4_Shower"), dInitNumNeutralArraySize);
	locBranchRegister.Register_FundamentalArray<Float_t>(Build_BranchName(locParticleBranchName, "ShowerQuality"), locArraySizeString, dInitNumNeutralArraySize);
	if (STORE_SC_VETO_INFO) {
	  locBranchRegister.Register_FundamentalArray<Int_t>(Build_BranchName(locParticleBranchName, "ShowerSC_BCAL_match"), locArraySizeString, dInitNumNeutralArraySize);
	  locBranchRegister.Register_FundamentalArray<Int_t>(Build_BranchName(locParticleBranchName, "ShowerSC_FCAL_match"), locArraySizeString, dInitNumNeutralArraySize);
	  locBranchRegister.Register_FundamentalArray<Float_t>(Build_BranchName(locParticleBranchName, "ShowerSC_BCAL_phi_min"), locArraySizeString, dInitNumNeutralArraySize);
	  locBranchRegister.Register_FundamentalArray<Float_t>(Build_BranchName(locParticleBranchName, "ShowerSC_FCAL_phi_min"), locArraySizeString, dInitNumNeutralArraySize);
	}
	locBranchRegister.Register_FundamentalArray<Int_t>(Build_BranchName(locParticleBranchName, "ShowerTOF_FCAL_match"), locArraySizeString, dInitNumNeutralArraySize);
	locBranchRegister.Register_FundamentalArray<Float_t>(Build_BranchName(locParticleBranchName, "ShowerTOF_FCAL_x_min"), locArraySizeString, dInitNumNeutralArraySize);
	locBranchRegister.Register_FundamentalArray<Float_t>(Build_BranchName(locParticleBranchName, "ShowerTOF_FCAL_y_min"), locArraySizeString, dInitNumNeutralArraySize);
		
	locBranchRegister.Register_FundamentalArray<Float_t>(Build_BranchName(locParticleBranchName, "Energy_BCAL"), locArraySizeString, dInitNumNeutralArraySize);
	locBranchRegister.Register_FundamentalArray<Float_t>(Build_BranchName(locParticleBranchName, "Energy_BCALPreshower"), locArraySizeString, dInitNumNeutralArraySize);
	if(BCAL_VERBOSE_OUTPUT) {
		locBranchRegister.Register_FundamentalArray<Float_t>(Build_BranchName(locParticleBranchName, "Energy_BCALLayer2"), locArraySizeString, dInitNumNeutralArraySize);
		locBranchRegister.Register_FundamentalArray<Float_t>(Build_BranchName(locParticleBranchName, "Energy_BCALLayer3"), locArraySizeString, dInitNumNeutralArraySize);
		locBranchRegister.Register_FundamentalArray<Float_t>(Build_BranchName(locParticleBranchName, "Energy_BCALLayer4"), locArraySizeString, dInitNumNeutralArraySize);
	}
	locBranchRegister.Register_FundamentalArray<Float_t>(Build_BranchName(locParticleBranchName, "SigLong_BCAL"), locArraySizeString, dInitNumNeutralArraySize);
	locBranchRegister.Register_FundamentalArray<Float_t>(Build_BranchName(locParticleBranchName, "SigTheta_BCAL"), locArraySizeString, dInitNumNeutralArraySize);
	locBranchRegister.Register_FundamentalArray<Float_t>(Build_BranchName(locParticleBranchName, "SigTrans_BCAL"), locArraySizeString, dInitNumNeutralArraySize);
	locBranchRegister.Register_FundamentalArray<Float_t>(Build_BranchName(locParticleBranchName, "RMSTime_BCAL"), locArraySizeString, dInitNumNeutralArraySize);
	locBranchRegister.Register_FundamentalArray<Float_t>(Build_BranchName(locParticleBranchName, "Energy_FCAL"), locArraySizeString, dInitNumNeutralArraySize);
	if(FCAL_VERBOSE_OUTPUT) {
		locBranchRegister.Register_FundamentalArray<Float_t>(Build_BranchName(locParticleBranchName, "E1E9_FCAL"), locArraySizeString, dInitNumNeutralArraySize);
		locBranchRegister.Register_FundamentalArray<Float_t>(Build_BranchName(locParticleBranchName, "E9E25_FCAL"), locArraySizeString, dInitNumNeutralArraySize);
		locBranchRegister.Register_FundamentalArray<Float_t>(Build_BranchName(locParticleBranchName, "SumU_FCAL"), locArraySizeString, dInitNumNeutralArraySize);
		locBranchRegister.Register_FundamentalArray<Float_t>(Build_BranchName(locParticleBranchName, "SumV_FCAL"), locArraySizeString, dInitNumNeutralArraySize);
		locBranchRegister.Register_FundamentalArray<Float_t>(Build_BranchName(locParticleBranchName, "NumBlocks_FCAL"), locArraySizeString, dInitNumNeutralArraySize);
	}
	locBranchRegister.Register_FundamentalArray<Float_t>(Build_BranchName(locParticleBranchName, "Energy_CCAL"), locArraySizeString, dInitNumNeutralArraySize);
	//NEARBY TRACKS
	locBranchRegister.Register_FundamentalArray<Float_t>(Build_BranchName(locParticleBranchName, "TrackBCAL_DeltaPhi"), locArraySizeString, dInitNumNeutralArraySize);
	locBranchRegister.Register_FundamentalArray<Float_t>(Build_BranchName(locParticleBranchName, "TrackBCAL_DeltaZ"), locArraySizeString, dInitNumNeutralArraySize);
	locBranchRegister.Register_FundamentalArray<Float_t>(Build_BranchName(locParticleBranchName, "TrackFCAL_DOCA"), locArraySizeString, dInitNumNeutralArraySize);
	locBranchRegister.Register_FundamentalArray<Float_t>(Build_BranchName(locParticleBranchName, "TrackFCAL_DeltaT"), locArraySizeString, dInitNumNeutralArraySize);

	//PHOTON PID INFO
		//Computed using DVertex (best estimate of reaction vertex using all "good" tracks)
		//Can be used to compute timing chisq //is invalid for non-photons, so computed assuming photon PID
		//Variance of X4_Measured.T() - RFTime, regardless of which RF bunch is chosen. //RF bunch is combo-depende
	locBranchRegister.Register_FundamentalArray<Float_t>(Build_BranchName(locParticleBranchName, "PhotonRFDeltaTVar"), locArraySizeString, dInitNumNeutralArraySize);
}

void DEventWriterROOT::Create_Branches_Combo(DTreeBranchRegister& locBranchRegister, const DReaction* locReaction, bool locIsMCDataFlag, TMap* locPositionToNameMap) const
{
	auto locReactionVertexInfo = dVertexInfoMap.find(locReaction)->second;
	string locNumComboString = "NumCombos";
	locBranchRegister.Register_Single<UInt_t>(locNumComboString);

	//kinfit type
	DKinFitType locKinFitType = locReaction->Get_KinFitType();
	bool locKinFitFlag = (locKinFitType != d_NoFit);
	bool locVertexKinFitFlag = locKinFitFlag && (locKinFitType != d_P4Fit);

	//Is-cut
	locBranchRegister.Register_FundamentalArray<Bool_t>("IsComboCut", locNumComboString, dInitNumComboArraySize);

	//create combo-dependent, particle-independent branches
	if(locIsMCDataFlag)
	{
		locBranchRegister.Register_FundamentalArray<Bool_t>("IsTrueCombo", locNumComboString, dInitNumComboArraySize);
		locBranchRegister.Register_FundamentalArray<Bool_t>("IsBDTSignalCombo", locNumComboString, dInitNumComboArraySize);
	}

	locBranchRegister.Register_FundamentalArray<Float_t>("RFTime_Measured", locNumComboString, dInitNumComboArraySize);
	if(locKinFitFlag)
	{
		locBranchRegister.Register_FundamentalArray<Float_t>("ChiSq_KinFit", locNumComboString, dInitNumComboArraySize);
		locBranchRegister.Register_FundamentalArray<UInt_t>("NDF_KinFit", locNumComboString, dInitNumComboArraySize);
		if((locKinFitType == d_SpacetimeFit) || (locKinFitType == d_P4AndSpacetimeFit))
			locBranchRegister.Register_FundamentalArray<Float_t>("RFTime_KinFit", locNumComboString, dInitNumComboArraySize);
	}
	locBranchRegister.Register_FundamentalArray<UChar_t>("NumUnusedShowers", locNumComboString, dInitNumComboArraySize);
	locBranchRegister.Register_FundamentalArray<Float_t>("Energy_UnusedShowers", locNumComboString, dInitNumComboArraySize);
	locBranchRegister.Register_FundamentalArray<UChar_t>("NumUnusedShowers_Quality", locNumComboString, dInitNumComboArraySize);
	locBranchRegister.Register_FundamentalArray<Float_t>("Energy_UnusedShowers_Quality", locNumComboString, dInitNumComboArraySize);
	locBranchRegister.Register_FundamentalArray<Float_t>("SumPMag_UnusedTracks", locNumComboString, dInitNumComboArraySize);
	locBranchRegister.Register_ClonesArray<TVector3>("SumP3_UnusedTracks", dInitNumComboArraySize);

	map<Particle_t, unsigned int> locParticleNumberMap_Current;
	for(size_t loc_i = 0; loc_i < locReaction->Get_NumReactionSteps(); ++loc_i)
	{
		const DReactionStep* locReactionStep = locReaction->Get_ReactionStep(loc_i);

		//initial particle
		Particle_t locInitialPID = locReactionStep->Get_InitialPID();
		//should check to make sure the beam particle isn't missing...
		if((loc_i == 0) && (locReactionStep->Get_InitialPID() != Unknown))
			Create_Branches_BeamComboParticle(locBranchRegister, locInitialPID, locKinFitType);
		else //decaying
		{
			//get the branch name
			ostringstream locPositionStream;
			locPositionStream << loc_i << "_-1";
			TObjString* locObjString = (TObjString*)locPositionToNameMap->GetValue(locPositionStream.str().c_str());
			string locParticleBranchName = (const char*)(locObjString->GetString());

			if(IsFixedMass(locInitialPID) && locReactionStep->Get_KinFitConstrainInitMassFlag() && ((locKinFitType == d_P4Fit) || (locKinFitType == d_P4AndVertexFit) || (locKinFitType == d_P4AndSpacetimeFit)))
				locBranchRegister.Register_ClonesArray<TLorentzVector>(Build_BranchName(locParticleBranchName, "P4_KinFit"), dInitNumComboArraySize);
			if((loc_i == 0) || IsDetachedVertex(locInitialPID))
				locBranchRegister.Register_ClonesArray<TLorentzVector>(Build_BranchName(locParticleBranchName, "X4"), dInitNumComboArraySize);

			auto locStepVertexInfo = locReactionVertexInfo->Get_StepVertexInfo(loc_i);
			auto locParentVertexInfo = locStepVertexInfo->Get_ParentVertexInfo();
			if(IsDetachedVertex(locInitialPID) && locVertexKinFitFlag && (locParentVertexInfo != nullptr) && locStepVertexInfo->Get_FittableVertexFlag() && locParentVertexInfo->Get_FittableVertexFlag())
				locBranchRegister.Register_FundamentalArray<Float_t>(Build_BranchName(locParticleBranchName, "PathLengthSigma"), locNumComboString, dInitNumComboArraySize);
		}

		//final particles
		auto locFinalParticleIDs = locReactionStep->Get_FinalPIDs();
		for(size_t loc_j = 0; loc_j < locFinalParticleIDs.size(); ++loc_j)
		{
			int locDecayStepIndex = DAnalysis::Get_DecayStepIndex(locReaction, loc_i, loc_j);
			if(locDecayStepIndex >= 0)
				continue; //decaying particle

			//get the branch name
			ostringstream locPositionStream;
			locPositionStream << loc_i << "_" << loc_j;
			TObjString* locObjString = (TObjString*)locPositionToNameMap->GetValue(locPositionStream.str().c_str());
			string locParticleBranchName = (const char*)(locObjString->GetString());

			//missing particle
			if(locReactionStep->Get_MissingParticleIndex() == int(loc_j))
			{
				// missing particle
				if((locKinFitType == d_P4Fit) || (locKinFitType == d_P4AndVertexFit) || (locKinFitType == d_P4AndSpacetimeFit))
					locBranchRegister.Register_ClonesArray<TLorentzVector>(Build_BranchName(locParticleBranchName, "P4_KinFit"), dInitNumComboArraySize);
				continue;
			}

			Particle_t locPID = locFinalParticleIDs[loc_j];
			if(ParticleCharge(locPID) == 0)
				Create_Branches_ComboNeutral(locBranchRegister, locParticleBranchName, locKinFitType);
			else
				Create_Branches_ComboTrack(locBranchRegister, locParticleBranchName, locKinFitType);
		}
	}
}

void DEventWriterROOT::Create_Branches_BeamComboParticle(DTreeBranchRegister& locBranchRegister, Particle_t locBeamPID, DKinFitType locKinFitType) const
{
	string locParticleBranchName = "ComboBeam";
	string locArraySizeString = "NumCombos";

	//IDENTIFIER
	locBranchRegister.Register_FundamentalArray<Int_t>(Build_BranchName(locParticleBranchName, "BeamIndex"), locArraySizeString, dInitNumComboArraySize);

	//KINEMATICS: KINFIT //at the interaction vertex
	if(locKinFitType != d_NoFit)
	{
		if(((locKinFitType != d_VertexFit) && (locKinFitType != d_SpacetimeFit)) || (ParticleCharge(locBeamPID) != 0))
			locBranchRegister.Register_ClonesArray<TLorentzVector>(Build_BranchName(locParticleBranchName, "P4_KinFit"), dInitNumComboArraySize);
		if(locKinFitType != d_P4Fit)
			locBranchRegister.Register_ClonesArray<TLorentzVector>(Build_BranchName(locParticleBranchName, "X4_KinFit"), dInitNumComboArraySize);
	}
}

void DEventWriterROOT::Create_Branches_ComboTrack(DTreeBranchRegister& locBranchRegister, string locParticleBranchName, DKinFitType locKinFitType) const
{
	string locArraySizeString = "NumCombos";

	//IDENTIFIER
	locBranchRegister.Register_FundamentalArray<Int_t>(Build_BranchName(locParticleBranchName, "ChargedIndex"), locArraySizeString, dInitNumComboArraySize);

	//MEASURED PID
	locBranchRegister.Register_FundamentalArray<Float_t>(Build_BranchName(locParticleBranchName, "Beta_Timing_Measured"), locArraySizeString, dInitNumComboArraySize);
	locBranchRegister.Register_FundamentalArray<Float_t>(Build_BranchName(locParticleBranchName, "ChiSq_Timing_Measured"), locArraySizeString, dInitNumComboArraySize);

	//KINFIT PID
	if((locKinFitType != d_NoFit) && (locKinFitType != d_SpacetimeFit) && (locKinFitType != d_P4AndSpacetimeFit))
	{
		locBranchRegister.Register_FundamentalArray<Float_t>(Build_BranchName(locParticleBranchName, "Beta_Timing_KinFit"), locArraySizeString, dInitNumComboArraySize);
		locBranchRegister.Register_FundamentalArray<Float_t>(Build_BranchName(locParticleBranchName, "ChiSq_Timing_KinFit"), locArraySizeString, dInitNumComboArraySize);
	}

	//KINFIT INFO //at the production vertex
	if(locKinFitType != d_NoFit)
	{
		//update p4 even if vertex-only fit, because charged momentum propagated through b-field
		locBranchRegister.Register_ClonesArray<TLorentzVector>(Build_BranchName(locParticleBranchName, "P4_KinFit"), dInitNumComboArraySize);
		if(locKinFitType != d_P4Fit)
			locBranchRegister.Register_ClonesArray<TLorentzVector>(Build_BranchName(locParticleBranchName, "X4_KinFit"), dInitNumComboArraySize);
	}
}

void DEventWriterROOT::Create_Branches_ComboNeutral(DTreeBranchRegister& locBranchRegister, string locParticleBranchName, DKinFitType locKinFitType) const
{
	string locArraySizeString = "NumCombos";

	//IDENTIFIER
	locBranchRegister.Register_FundamentalArray<Int_t>(Build_BranchName(locParticleBranchName, "NeutralIndex"), locArraySizeString, dInitNumComboArraySize);

	//KINEMATICS //is combo-dependent: P4 defined by X4, X4 defined by other tracks
	locBranchRegister.Register_ClonesArray<TLorentzVector>(Build_BranchName(locParticleBranchName, "X4_Measured"), dInitNumComboArraySize);
	locBranchRegister.Register_ClonesArray<TLorentzVector>(Build_BranchName(locParticleBranchName, "P4_Measured"), dInitNumComboArraySize);

	//MEASURED PID
	locBranchRegister.Register_FundamentalArray<Float_t>(Build_BranchName(locParticleBranchName, "Beta_Timing_Measured"), locArraySizeString, dInitNumComboArraySize);
	if(locParticleBranchName.substr(0, 6) == "Photon")
		locBranchRegister.Register_FundamentalArray<Float_t>(Build_BranchName(locParticleBranchName, "ChiSq_Timing_Measured"), locArraySizeString, dInitNumComboArraySize);

	//KINFIT PID
	if((locKinFitType != d_NoFit) && (locKinFitType != d_SpacetimeFit) && (locKinFitType != d_P4AndSpacetimeFit))
	{
		locBranchRegister.Register_FundamentalArray<Float_t>(Build_BranchName(locParticleBranchName, "Beta_Timing_KinFit"), locArraySizeString, dInitNumComboArraySize);
		if(locParticleBranchName.substr(0, 6) == "Photon")
			locBranchRegister.Register_FundamentalArray<Float_t>(Build_BranchName(locParticleBranchName, "ChiSq_Timing_KinFit"), locArraySizeString, dInitNumComboArraySize);
	}

	//KINFIT INFO //at the production vertex
	if(locKinFitType != d_NoFit)
	{
		locBranchRegister.Register_ClonesArray<TLorentzVector>(Build_BranchName(locParticleBranchName, "P4_KinFit"), dInitNumComboArraySize);
		if(locKinFitType != d_P4Fit)
			locBranchRegister.Register_ClonesArray<TLorentzVector>(Build_BranchName(locParticleBranchName, "X4_KinFit"), dInitNumComboArraySize);
	}
}

void DEventWriterROOT::Create_Branches_KinFitData(DTreeBranchRegister& locBranchRegister, const std::shared_ptr<const JEvent>& locEventLoop, const DReaction* locReaction, bool locIsMCDataFlag) const
{

	if(!STORE_PULL_INFO && !STORE_ERROR_MATRIX_INFO)
		return;
    
    //Get the fit-type (P4, P4 + Vertex, and Vertex-only)
    DKinFitType kfitType = locReaction->Get_KinFitType();
    
    vector<Particle_t> finalchargedPIDs,finalneutralPIDs;
    size_t nReactionSteps = locReaction->Get_NumReactionSteps();
    int nFinalChargedParticles,nFinalNeutralParticles;
    
    vector<string> pidVector = getPIDinReaction(locReaction);
    map<Particle_t,int> assignMap = getNameForParticle(locReaction,pidVector);
     
    //Check, if at least two charged particles are present in the final state: (important for vertex-fit)
    int sumAllChargedParticles = 0;
    int sumAllNeutralParticles = 0;
    //First, check how many neutral/charged particles are in the final stage
    //The pull-information is avaiablable for certain particle combinations (charged + neutral) only:
    //----------------------------------------------------------------------------------------------
    for(size_t loc_i=0;loc_i<nReactionSteps;loc_i++){
        
        //Charged Particles:
        finalchargedPIDs = locReaction->Get_ReactionStep(loc_i)->Get_FinalPIDs(true,d_Charged,true);
        nFinalChargedParticles = finalchargedPIDs.size();
        sumAllChargedParticles += nFinalChargedParticles;
        
         //Neutral Particles:
        finalneutralPIDs = locReaction->Get_ReactionStep(loc_i)->Get_FinalPIDs(true,d_Neutral,true);
        nFinalNeutralParticles = finalneutralPIDs.size();
        sumAllNeutralParticles += nFinalNeutralParticles;
    }
    //----------------------------------------------------------------------------------------------
    
    if(sumAllChargedParticles == 1 && (kfitType == d_VertexFit || kfitType == d_P4AndVertexFit)){
        STORE_PULL_INFO = false;
        jout <<"  "<< endl;
        jout <<">>> WARNING: Only one charged track in the final state! No vertex fit possible! No pulls will be stored... <<<"<< endl;
        jout <<"  "<< endl;
    }
    
    if(sumAllNeutralParticles > 0 && kfitType == d_VertexFit){
        STORE_PULL_INFO = false;
        jout <<"  "<< endl;
        jout <<">>> WARNING: Neutral particles involved in pure vertex fit! No pulls will be stored... <<<"<< endl;
        jout <<"  "<< endl;
    }
    
    if(kfitType == d_NoFit){
		STORE_PULL_INFO = false;
        jout <<"  "<< endl;
        jout <<">>> WARNING: No fit specified! No pulls will be stored... <<<"<< endl;
        jout <<"  "<< endl;
	}

    setPullFlag(locReaction,STORE_PULL_INFO);
        
    //Get Pulls for the beam:
    //Particle_t beamPID = locReaction->Get_ReactionStep(0)->Get_InitialPID();
    
    string particleCovM = "numEntries_ParticleErrM";
    string showerCovM = "numEntries_ShowerErrM";
    string decayCovM = "numEntries_DecayErrM";
    if(STORE_ERROR_MATRIX_INFO){
       locBranchRegister.Register_Single<Int_t>(particleCovM);
       locBranchRegister.Register_Single<Int_t>(showerCovM); 
    }
    
    //Set the branches for the beam photon:
    if(STORE_PULL_INFO) 
    	setTreePullBranches(locBranchRegister,"ComboBeam",kfitType,dInitNumComboArraySize,true);
    if(STORE_ERROR_MATRIX_INFO) 
    	locBranchRegister.Register_FundamentalArray< Float_t >(Build_BranchName("ComboBeam","ErrMatrix"),particleCovM,nEntriesParticleCov);
		
    //----------------------------------------------------------------------------------------------
    for(size_t loc_i=0;loc_i<nReactionSteps;loc_i++){
        
        //Charged Particles:
        finalchargedPIDs = locReaction->Get_ReactionStep(loc_i)->Get_FinalPIDs(true,d_Charged,true);
        nFinalChargedParticles = finalchargedPIDs.size();
        //----------------------------------------------------------------------------------------------
        for(int loc_j=0;loc_j<nFinalChargedParticles;loc_j++){
            //Define branches for final charged particles:
            string branchName = assignName(finalchargedPIDs.at(loc_j),assignMap);
            assignMap[finalchargedPIDs.at(loc_j)]--;
            if(branchName != "nada"){
                  if(STORE_PULL_INFO) setTreePullBranches(locBranchRegister,branchName,kfitType,dInitNumComboArraySize,false);
                  if(STORE_ERROR_MATRIX_INFO) locBranchRegister.Register_FundamentalArray< Float_t >(Build_BranchName(branchName,"ErrMatrix"),particleCovM,nEntriesParticleCov);
            }
        }
        //----------------------------------------------------------------------------------------------
        
        //Neutral Particles:
        finalneutralPIDs = locReaction->Get_ReactionStep(loc_i)->Get_FinalPIDs(true,d_Neutral,true);
        nFinalNeutralParticles = finalneutralPIDs.size();
        //----------------------------------------------------------------------------------------------
        for(int loc_j=0;loc_j<nFinalNeutralParticles;loc_j++){
            //Define branches for final neutral particles:
            string branchName = assignName(finalneutralPIDs.at(loc_j),assignMap);
            assignMap[finalneutralPIDs.at(loc_j)]--;
            if(branchName != "nada"){
                if(STORE_PULL_INFO) setTreePullBranches(locBranchRegister,branchName,kfitType,dInitNumComboArraySize,true);
                if(finalneutralPIDs.at(loc_j) == Gamma && STORE_ERROR_MATRIX_INFO) locBranchRegister.Register_FundamentalArray< Float_t >(Build_BranchName(branchName,"ErrMatrix"),showerCovM,nEntriesShowerCov);
            } 
         }
        //----------------------------------------------------------------------------------------------
     }
    //----------------------------------------------------------------------------------------------
    
    assignMap.clear();
    abundanceMap.clear();
}

void DEventWriterROOT::Fill_ThrownTree(const std::shared_ptr<const JEvent>& locEvent) const
{
	vector<const DMCThrown*> locMCThrowns_FinalState;
	locEvent->Get(locMCThrowns_FinalState, "FinalState");

	vector<const DMCThrown*> locMCThrowns_Decaying;
	if(STORE_THROWN_DECAYING_PARTICLES)
		locEvent->Get(locMCThrowns_Decaying, "Decaying");

	const DMCReaction* locMCReaction = NULL;
	locEvent->GetSingle(locMCReaction);

	ULong64_t locNumPIDThrown_FinalState = 0, locPIDThrown_Decaying = 0;
	Compute_ThrownPIDInfo(locMCThrowns_FinalState, locMCThrowns_Decaying, locNumPIDThrown_FinalState, locPIDThrown_Decaying);

	vector<const DMCThrown*> locMCThrownsToSave;
	map<const DMCThrown*, unsigned int> locThrownIndexMap;
	Group_ThrownParticles(locMCThrowns_FinalState, locMCThrowns_Decaying, locMCThrownsToSave, locThrownIndexMap);

	vector<const DBeamPhoton*> locTaggedMCGenBeams;
	locEvent->Get(locTaggedMCGenBeams, "TAGGEDMCGEN");

	vector<const DBeamPhoton*> locMCGenBeams;
	locEvent->Get(locMCGenBeams, "MCGEN");

	vector<const DMCTrajectoryPoint*> locDMCTrajectoryPoints;
	locEvent->Get(locDMCTrajectoryPoints);

	const DBeamPhoton* locTaggedMCGenBeam = locTaggedMCGenBeams.empty() ? locMCGenBeams[0] : locTaggedMCGenBeams[0]; //if empty: will have to do. 

	DEvent::GetLockService(locEvent)->RootWriteLock();

	//primary event info
	dThrownTreeFillData.Fill_Single<UInt_t>("RunNumber", locEvent->GetRunNumber());
	dThrownTreeFillData.Fill_Single<ULong64_t>("EventNumber", locEvent->GetEventNumber());

	//throwns
	Fill_ThrownInfo(&dThrownTreeFillData, locMCReaction, locTaggedMCGenBeam, locMCThrownsToSave, locThrownIndexMap, locNumPIDThrown_FinalState, locPIDThrown_Decaying, locDMCTrajectoryPoints);

	//Custom Branches
	Fill_CustomBranches_ThrownTree(&dThrownTreeFillData, locEvent, locMCReaction, locMCThrownsToSave);

	//FILL TTREE
	dThrownTreeInterface->Fill(dThrownTreeFillData);

	
	DEvent::GetLockService(locEvent)->RootUnLock();

}

void DEventWriterROOT::Fill_DataTrees(const std::shared_ptr<const JEvent>& locEvent, string locDReactionTag) const
{
	if(locDReactionTag == "Thrown")
	{
		cout << "WARNING: CANNOT FILL THROWN TREE WITH THIS FUNCTION." << endl;
		return;
	}

	vector<const DAnalysisResults*> locAnalysisResultsVector;
	locEvent->Get(locAnalysisResultsVector);

	vector<const DReaction*> locReactionsWithTag;
	locEvent->Get(locReactionsWithTag, locDReactionTag.c_str());

	for(size_t loc_i = 0; loc_i < locAnalysisResultsVector.size(); ++loc_i)
	{
		deque<const DParticleCombo*> locPassedParticleCombos;
		locAnalysisResultsVector[loc_i]->Get_PassedParticleCombos(locPassedParticleCombos);
		if(locPassedParticleCombos.empty())
			continue;

		const DReaction* locReaction = locAnalysisResultsVector[loc_i]->Get_Reaction();
		if(!locReaction->Get_EnableTTreeOutputFlag())
			continue;

		bool locReactionFoundFlag = false;
		for(size_t loc_j = 0; loc_j < locReactionsWithTag.size(); ++loc_j)
		{
			if(locReactionsWithTag[loc_j] != locReaction)
				continue;
			locReactionFoundFlag = true;
			break;
		}
		if(!locReactionFoundFlag)
			continue; //reaction not from this factory, continue

		Fill_DataTree(locEvent, locReaction, locPassedParticleCombos);
	}
}

void DEventWriterROOT::Fill_DataTree(const std::shared_ptr<const JEvent>& locEvent, const DReaction* locReaction, deque<const DParticleCombo*>& locParticleCombos) const
{
	if(locReaction->Get_ReactionName() == "Thrown")
	{
		cout << "WARNING: CANNOT FILL THROWN TREE WITH THIS FUNCTION." << endl;
		return;
	}
	
	if(!locReaction->Get_EnableTTreeOutputFlag())
	{
		cout << "WARNING: ROOT TTREE OUTPUT NOT ENABLED FOR THIS DREACTION (" << locReaction->Get_ReactionName() << ")" << endl;
		return;
	}

	/***************************************************** GET THROWN INFO *****************************************************/

	vector<const DMCThrown*> locMCThrowns_FinalState;
	locEvent->Get(locMCThrowns_FinalState, "FinalState");

	vector<const DMCThrown*> locMCThrowns_Decaying;
	locEvent->Get(locMCThrowns_Decaying, "Decaying");

	vector<const DMCThrownMatching*> locMCThrownMatchingVector;
	locEvent->Get(locMCThrownMatchingVector);
	const DMCThrownMatching* locMCThrownMatching = locMCThrownMatchingVector.empty() ? NULL : locMCThrownMatchingVector[0];

	vector<const DMCReaction*> locMCReactions;
	locEvent->Get(locMCReactions);
	const DMCReaction* locMCReaction = locMCReactions.empty() ? NULL : locMCReactions[0];

	vector<const DBeamPhoton*> locTaggedMCGenBeams;
	locEvent->Get(locTaggedMCGenBeams, "TAGGEDMCGEN");

	vector<const DBeamPhoton*> locMCGenBeams;
	locEvent->Get(locMCGenBeams, "MCGEN");

	vector<const DMCTrajectoryPoint*> locDMCTrajectoryPoints;
	locEvent->Get(locDMCTrajectoryPoints);

	const DBeamPhoton* locTaggedMCGenBeam = nullptr;

	if (locTaggedMCGenBeams.empty()){
      if ( !locMCGenBeams.empty() ) locTaggedMCGenBeam = locMCGenBeams[0];
	}
	else locTaggedMCGenBeam = locTaggedMCGenBeams[0];

	//Pre-compute thrown info
	ULong64_t locNumPIDThrown_FinalState = 0, locPIDThrown_Decaying = 0;
	Compute_ThrownPIDInfo(locMCThrowns_FinalState, locMCThrowns_Decaying, locNumPIDThrown_FinalState, locPIDThrown_Decaying);

	//Pre-compute thrown info
	vector<const DMCThrown*> locMCThrownsToSave;
	map<const DMCThrown*, unsigned int> locThrownIndexMap;
	Group_ThrownParticles(locMCThrowns_FinalState, locMCThrowns_Decaying, locMCThrownsToSave, locThrownIndexMap);

	/****************************************************** GET PARTICLES ******************************************************/

	bool locSaveUnusedFlag = locReaction->Get_SaveUnusedFlag();

	//Get PIDs need for reaction
	auto locDetectedPIDs = locReaction->Get_FinalPIDs(-1, false, false, d_AllCharges, false);
	set<Particle_t> locReactionPIDs;
	for(size_t loc_j = 0; loc_j < locDetectedPIDs.size(); ++loc_j)
		locReactionPIDs.insert(locDetectedPIDs[loc_j]);

	//GET HYPOTHESES
	vector<const DChargedTrackHypothesis*> locChargedTrackHypotheses;
	vector<const DNeutralParticleHypothesis*> locNeutralParticleHypotheses;
	if(locSaveUnusedFlag)
	{
		locChargedTrackHypotheses = Get_ChargedHypotheses(locEvent);
		locNeutralParticleHypotheses = Get_NeutralHypotheses(locEvent, locReactionPIDs);
	}
	else
	{
		locChargedTrackHypotheses = Get_ChargedHypotheses_Used(locEvent, locReaction, locParticleCombos);
		locNeutralParticleHypotheses = Get_NeutralHypotheses_Used(locEvent, locReaction, locReactionPIDs, locParticleCombos);
	}

	//GET BEAM PHOTONS
	bool locBeamUsedFlag = DAnalysis::Get_IsFirstStepBeam(locReaction);
	vector<const DBeamPhoton*> locBeamPhotons = Get_BeamPhotons(locParticleCombos);

	//create map of particles to array index:
		//used for pointing combo particles to the appropriate measured-particle array index
		//for hypos, they are the preselect versions if they exist, else the combo versions (e.g. PID not in REST)

	//indices: beam
	map<pair<oid_t, Particle_t>, size_t> locObjectToArrayIndexMap; //particle_t necessary for neutral showers!
	for(size_t loc_i = 0; loc_i < locBeamPhotons.size(); ++loc_i)
	{
		pair<oid_t, Particle_t> locBeamPair(locBeamPhotons[loc_i]->id, locBeamPhotons[loc_i]->PID());
		locObjectToArrayIndexMap[locBeamPair] = loc_i;
	}

	//indices: charged
	for(size_t loc_i = 0; loc_i < locChargedTrackHypotheses.size(); ++loc_i)
	{
		const DTrackTimeBased* locTrackTimeBased = locChargedTrackHypotheses[loc_i]->Get_TrackTimeBased();
		pair<oid_t, Particle_t> locTrackPair(locTrackTimeBased->id, locTrackTimeBased->PID());
		locObjectToArrayIndexMap[locTrackPair] = loc_i;
	}

	//indices: neutral
	for(size_t loc_i = 0; loc_i < locNeutralParticleHypotheses.size(); ++loc_i)
	{
		const DNeutralShower* locNeutralShower = locNeutralParticleHypotheses[loc_i]->Get_NeutralShower();
		pair<oid_t, Particle_t> locShowerPair(locNeutralShower->id, locNeutralParticleHypotheses[loc_i]->PID());
		locObjectToArrayIndexMap[locShowerPair] = loc_i;
	}

	/**************************************************** GET MISCELLANEOUS ****************************************************/

	//GET DETECTOR MATCHES
	const DDetectorMatches* locDetectorMatches = NULL;
	locEvent->GetSingle(locDetectorMatches);

	//GET DVERTEX
	const DVertex* locVertex = NULL;
	locEvent->GetSingle(locVertex);

	//GET TRIGGER
	const DTrigger* locTrigger = NULL;
	locEvent->GetSingle(locTrigger);

	/************************************************* EXECUTE ANALYSIS ACTIONS ************************************************/
	       
	DEvent::GetLockService(locEvent)->RootWriteLock();

	Bool_t locIsThrownTopologyFlag = kFALSE;
	vector<Bool_t> locIsTrueComboFlags;
	vector<Bool_t> locIsBDTSignalComboFlags;
	if(locMCReaction != NULL)
	{
		DCutAction_ThrownTopology* locThrownTopologyAction = dCutActionMap_ThrownTopology.find(locReaction)->second;
		locIsThrownTopologyFlag = (*locThrownTopologyAction)(locEvent, NULL); //combo not used/needed
		for(size_t loc_i = 0; loc_i < locParticleCombos.size(); ++loc_i)
		{
			DCutAction_TrueCombo* locTrueComboAction = dCutActionMap_TrueCombo.find(locReaction)->second;
			locIsTrueComboFlags.push_back((*locTrueComboAction)(locEvent, locParticleCombos[loc_i]));

			DCutAction_BDTSignalCombo* locBDTSignalComboAction = dCutActionMap_BDTSignalCombo.find(locReaction)->second;
			locIsBDTSignalComboFlags.push_back((*locBDTSignalComboAction)(locEvent, locParticleCombos[loc_i]));
		}
	}

	/***************************************************** FILL TTREE DATA *****************************************************/

	//Get tree fill data
	DTreeFillData* locTreeFillData = dTreeFillDataMap.find(locReaction)->second;

	//PRIMARY EVENT INFO
	locTreeFillData->Fill_Single<UInt_t>("RunNumber", locEvent->GetRunNumber());
	locTreeFillData->Fill_Single<ULong64_t>("EventNumber", locEvent->GetEventNumber());
	locTreeFillData->Fill_Single<UInt_t>("L1TriggerBits", locTrigger->Get_L1TriggerBits());
	locTreeFillData->Fill_Single<Double_t>("L1BCALEnergy", locTrigger->Get_GTP_BCALEnergy());
	locTreeFillData->Fill_Single<Double_t>("L1FCALEnergy", locTrigger->Get_GTP_FCALEnergy());


	//PRODUCTION X4
	DLorentzVector locProductionX4 = locVertex->dSpacetimeVertex;
	TLorentzVector locProductionTX4(locProductionX4.X(), locProductionX4.Y(), locProductionX4.Z(), locProductionX4.T());
	locTreeFillData->Fill_Single<TLorentzVector>("X4_Production", locProductionTX4);

	//THROWN INFORMATION
	if(locMCReaction != NULL)
	{
		Fill_ThrownInfo(locTreeFillData, locMCReaction, locTaggedMCGenBeam, locMCThrownsToSave, locThrownIndexMap, locNumPIDThrown_FinalState, locPIDThrown_Decaying, locDMCTrajectoryPoints, locMCThrownMatching);
		locTreeFillData->Fill_Single<Bool_t>("IsThrownTopology", locIsThrownTopologyFlag);
	}

	//INDEPENDENT BEAM PARTICLES
	if(locBeamUsedFlag)
	{
		//however, only fill with beam particles that are in the combos
		locTreeFillData->Fill_Single<UInt_t>("NumBeam", UInt_t(locBeamPhotons.size()));
		for(size_t loc_i = 0; loc_i < locBeamPhotons.size(); ++loc_i)
			Fill_BeamData(locTreeFillData, loc_i, locBeamPhotons[loc_i], locVertex, locMCThrownMatching);
	}

	//INDEPENDENT CHARGED TRACKS
	locTreeFillData->Fill_Single<UInt_t>("NumChargedHypos", UInt_t(locChargedTrackHypotheses.size()));
	for(size_t loc_i = 0; loc_i < locChargedTrackHypotheses.size(); ++loc_i)
		Fill_ChargedHypo(locTreeFillData, loc_i, locChargedTrackHypotheses[loc_i], locMCThrownMatching, locThrownIndexMap, locDetectorMatches);

	//INDEPENDENT NEUTRAL PARTICLES
	locTreeFillData->Fill_Single<UInt_t>("NumNeutralHypos", UInt_t(locNeutralParticleHypotheses.size()));
	for(size_t loc_i = 0; loc_i < locNeutralParticleHypotheses.size(); ++loc_i)
		Fill_NeutralHypo(locTreeFillData, loc_i, locNeutralParticleHypotheses[loc_i], locMCThrownMatching, locThrownIndexMap, locDetectorMatches);

	//UNUSED TRACKS
	double locSumPMag_UnusedTracks = 0.0;
	TVector3 locSumP3_UnusedTracks;
	int locNumUnusedTracks = dAnalysisUtilities->Calc_Momentum_UnusedTracks(locEvent, locParticleCombos[0], locSumPMag_UnusedTracks, locSumP3_UnusedTracks);
	locTreeFillData->Fill_Single<UChar_t>("NumUnusedTracks", locNumUnusedTracks);

	//COMBOS
	locTreeFillData->Fill_Single<UInt_t>("NumCombos", UInt_t(locParticleCombos.size()));
	for(size_t loc_i = 0; loc_i < locParticleCombos.size(); ++loc_i)
	{
		Fill_ComboData(locTreeFillData, locReaction, locParticleCombos[loc_i], loc_i, locObjectToArrayIndexMap);
		
		//ENERGY OF UNUSED SHOWERS (access to event loop required)
		double locEnergy_UnusedShowers = 0.;
		double locEnergy_UnusedShowers_Quality = 0.;
		int locNumber_UnusedShowers_Quality = 0;
		int locNumber_UnusedShowers = dAnalysisUtilities->Calc_Energy_UnusedShowers(locEvent, locParticleCombos[loc_i], locEnergy_UnusedShowers, locNumber_UnusedShowers_Quality, locEnergy_UnusedShowers_Quality);
		locTreeFillData->Fill_Array<UChar_t>("NumUnusedShowers", locNumber_UnusedShowers, loc_i);
		locTreeFillData->Fill_Array<Float_t>("Energy_UnusedShowers", locEnergy_UnusedShowers, loc_i);
		locTreeFillData->Fill_Array<UChar_t>("NumUnusedShowers_Quality", locNumber_UnusedShowers_Quality, loc_i);
		locTreeFillData->Fill_Array<Float_t>("Energy_UnusedShowers_Quality", locEnergy_UnusedShowers_Quality, loc_i);

		//MOMENTUM OF UNUSED TRACKS (access to event loop required)
		double locSumPMag_UnusedTracks = 0;
		TVector3 locSumP3_UnusedTracks;
		dAnalysisUtilities->Calc_Momentum_UnusedTracks(locEvent, locParticleCombos[loc_i], locSumPMag_UnusedTracks, locSumP3_UnusedTracks);
		locTreeFillData->Fill_Array<Float_t>("SumPMag_UnusedTracks", locSumPMag_UnusedTracks, loc_i);
		locTreeFillData->Fill_Array<TVector3>("SumP3_UnusedTracks", locSumP3_UnusedTracks, loc_i);

		if(locMCReaction != NULL)
		{
			locTreeFillData->Fill_Array<Bool_t>("IsTrueCombo", locIsTrueComboFlags[loc_i], loc_i);
			locTreeFillData->Fill_Array<Bool_t>("IsBDTSignalCombo", locIsTrueComboFlags[loc_i], loc_i);
		}
	}

	//Kinematic fit data (pulls and covariance matrices)
	Fill_KinFitData(locTreeFillData, locEvent, locReaction, locMCReaction, locMCThrownsToSave, locMCThrownMatching, locDetectorMatches, locBeamPhotons, locChargedTrackHypotheses, locNeutralParticleHypotheses, locParticleCombos);

	//CUSTOM
	Fill_CustomBranches_DataTree(locTreeFillData, locEvent, locReaction, locMCReaction, locMCThrownsToSave, locMCThrownMatching, locDetectorMatches, locBeamPhotons, locChargedTrackHypotheses, locNeutralParticleHypotheses, locParticleCombos);

	//FILL
	DTreeInterface* locTreeInterface = dTreeInterfaceMap.find(locReaction)->second;
	locTreeInterface->Fill(*locTreeFillData);	
	
	DEvent::GetLockService(locEvent)->RootUnLock();

}

vector<const DBeamPhoton*> DEventWriterROOT::Get_BeamPhotons(const deque<const DParticleCombo*>& locParticleCombos) const
{
	//however, only fill with beam particles that are in the combos
	set<const DBeamPhoton*> locBeamPhotonSet;
	vector<const DBeamPhoton*> locBeamPhotons;
	for(size_t loc_j = 0; loc_j < locParticleCombos.size(); ++loc_j)
	{
		const DParticleComboStep* locParticleComboStep = locParticleCombos[loc_j]->Get_ParticleComboStep(0);
		const DKinematicData* locKinematicData = locParticleComboStep->Get_InitialParticle_Measured();
		if(locKinematicData == NULL)
			continue;
		const DBeamPhoton* locBeamPhoton = dynamic_cast<const DBeamPhoton*>(locKinematicData);
		if(locBeamPhoton == NULL)
			continue;
		if(locBeamPhotonSet.find(locBeamPhoton) != locBeamPhotonSet.end())
			continue; //already included
		locBeamPhotonSet.insert(locBeamPhoton);
		locBeamPhotons.push_back(locBeamPhoton);
	}

	return locBeamPhotons;
}

vector<const DChargedTrackHypothesis*> DEventWriterROOT::Get_ChargedHypotheses(const std::shared_ptr<const JEvent>& locEvent) const
{
	//For default/preselect, save all
	//For combo, of new PIDs ONLY, save one of each for each track
		//save the one with the same RF bunch as the common bunch

	vector<const DChargedTrack*> locChargedTracks;
	locEvent->Get(locChargedTracks, "Combo");

	vector<const DChargedTrackHypothesis*> locChargedHyposToSave;
	for(auto& locChargedTrack : locChargedTracks)
		locChargedHyposToSave.insert(locChargedHyposToSave.end(), locChargedTrack->dChargedTrackHypotheses.begin(), locChargedTrack->dChargedTrackHypotheses.end());

	return locChargedHyposToSave;
}

vector<const DChargedTrackHypothesis*> DEventWriterROOT::Get_ChargedHypotheses_Used(const std::shared_ptr<const JEvent>& locEvent, const DReaction* locReaction, const deque<const DParticleCombo*>& locParticleCombos) const
{
	//get all hypos
	vector<const DChargedTrackHypothesis*> locAllHypos = Get_ChargedHypotheses(locEvent);

	//get used time-based tracks
	set<const DTrackTimeBased*> locUsedTimeBasedTracks;
	for(auto& locCombo : locParticleCombos)
	{
		auto locChargedParticles = locCombo->Get_FinalParticles_Measured(locReaction, d_Charged);
		for(auto& locParticle : locChargedParticles)
			locUsedTimeBasedTracks.insert(static_cast<const DChargedTrackHypothesis*>(locParticle)->Get_TrackTimeBased());
	}

	//loop through "all" hypos, removing those that weren't used
	for(auto locIterator = locAllHypos.begin(); locIterator != locAllHypos.end();)
	{
		const DTrackTimeBased* locTrackTimeBased = (*locIterator)->Get_TrackTimeBased();
		if(locUsedTimeBasedTracks.find(locTrackTimeBased) != locUsedTimeBasedTracks.end())
			++locIterator;
		else
		   locIterator = locAllHypos.erase(locIterator); 
	}

	return locAllHypos;
}

vector<const DNeutralParticleHypothesis*> DEventWriterROOT::Get_NeutralHypotheses(const std::shared_ptr<const JEvent>& locEvent, const set<Particle_t>& locReactionPIDs) const
{
	//For default/preselect, save all
	//For combo, of new PIDs ONLY, save one of each for each shower
		//save the one with the same RF bunch as the common bunch

	vector<const DNeutralParticle*> locNeutralParticles;
	locEvent->Get(locNeutralParticles, "Combo");

	vector<const DNeutralParticleHypothesis*> locNeutralHyposToSave;
	for(auto& locNeutralParticle : locNeutralParticles)
		locNeutralHyposToSave.insert(locNeutralHyposToSave.end(), locNeutralParticle->dNeutralParticleHypotheses.begin(), locNeutralParticle->dNeutralParticleHypotheses.end());

	return locNeutralHyposToSave;
}

vector<const DNeutralParticleHypothesis*> DEventWriterROOT::Get_NeutralHypotheses_Used(const std::shared_ptr<const JEvent>& locEvent, const DReaction* locReaction, const set<Particle_t>& locReactionPIDs, const deque<const DParticleCombo*>& locParticleCombos) const
{
	//get all hypos
	vector<const DNeutralParticleHypothesis*> locAllHypos = Get_NeutralHypotheses(locEvent, locReactionPIDs);

	//get used showers
	set<pair<const DNeutralShower*, Particle_t> > locUsedNeutralShowers;
	for(auto& locCombo : locParticleCombos)
	{
		auto locNeutralParticles = locCombo->Get_FinalParticles_Measured(locReaction, d_Neutral);
		for(auto& locParticle : locNeutralParticles)
		{
			const DNeutralShower* locNeutralShower = static_cast<const DNeutralParticleHypothesis*>(locParticle)->Get_NeutralShower();
			pair<const DNeutralShower*, Particle_t> locShowerPair(locNeutralShower, locParticle->PID());
			locUsedNeutralShowers.insert(locShowerPair);
		}
	}

	//loop through "all" hypos, removing those that weren't used
	for(auto locIterator = locAllHypos.begin(); locIterator != locAllHypos.end();)
	{
		const DNeutralShower* locNeutralShower = (*locIterator)->Get_NeutralShower();
		pair<const DNeutralShower*, Particle_t> locShowerPair(locNeutralShower, (*locIterator)->PID());
		if(locUsedNeutralShowers.find(locShowerPair) == locUsedNeutralShowers.end())
		   locIterator = locAllHypos.erase(locIterator);
		else
			++locIterator;
	}

	return locAllHypos;
}

ULong64_t DEventWriterROOT::Calc_ParticleMultiplexID(Particle_t locPID) const
{
	int locPower = ParticleMultiplexPower(locPID);
	if(locPower == -1)
		return 0;

	int locIsFinalStateInt = Is_FinalStateParticle(locPID);
	if(locPID == Pi0)
		locIsFinalStateInt = 1;

	if(locIsFinalStateInt == 1) //decimal
	{
		ULong64_t locParticleMultiplexID = 1;
		for(int loc_i = 0; loc_i < locPower; ++loc_i)
			locParticleMultiplexID *= ULong64_t(10);
		return locParticleMultiplexID;
	}
	//decaying: binary
	return (ULong64_t(1) << ULong64_t(locPower)); //bit-shift
}

void DEventWriterROOT::Compute_ThrownPIDInfo(const vector<const DMCThrown*>& locMCThrowns_FinalState, const vector<const DMCThrown*>& locMCThrowns_Decaying, ULong64_t& locNumPIDThrown_FinalState, ULong64_t& locPIDThrown_Decaying) const
{
	//THROWN PARTICLES BY PID
	locNumPIDThrown_FinalState = 0;
	for(size_t loc_i = 0; loc_i < locMCThrowns_FinalState.size(); ++loc_i) //final state
	{
		Particle_t locPID = locMCThrowns_FinalState[loc_i]->PID();
		ULong64_t locPIDMultiplexID = Calc_ParticleMultiplexID(locPID);
		if(locPIDMultiplexID == 0)
			continue; //unrecognized PID!!!
		unsigned int locCurrentNumParticles = (locNumPIDThrown_FinalState / locPIDMultiplexID) % ULong64_t(10);
		if(locCurrentNumParticles != 9)
			locNumPIDThrown_FinalState += locPIDMultiplexID;
	}

	locPIDThrown_Decaying = 0;
	for(size_t loc_i = 0; loc_i < locMCThrowns_Decaying.size(); ++loc_i) //decaying
	{
		Particle_t locPID = locMCThrowns_Decaying[loc_i]->PID();
		ULong64_t locPIDMultiplexID = Calc_ParticleMultiplexID(locPID);
		if(locPIDMultiplexID == 0)
			continue; //unrecognized PID!!!
		if(locPID != Pi0)
			locPIDThrown_Decaying |= locPIDMultiplexID; //bit-wise or
		else //save pi0's as final state instead of decaying
		{
			unsigned int locCurrentNumParticles = (locNumPIDThrown_FinalState / locPIDMultiplexID) % ULong64_t(10);
			if(locCurrentNumParticles != 9)
				locNumPIDThrown_FinalState += locPIDMultiplexID;
		}
	}
}

void DEventWriterROOT::Group_ThrownParticles(const vector<const DMCThrown*>& locMCThrowns_FinalState, const vector<const DMCThrown*>& locMCThrowns_Decaying, vector<const DMCThrown*>& locMCThrownsToSave, map<const DMCThrown*, unsigned int>& locThrownIndexMap) const
{
	locMCThrownsToSave.clear();
	locMCThrownsToSave.insert(locMCThrownsToSave.end(), locMCThrowns_FinalState.begin(), locMCThrowns_FinalState.end());
	locMCThrownsToSave.insert(locMCThrownsToSave.end(), locMCThrowns_Decaying.begin(), locMCThrowns_Decaying.end());

	//create map of mcthrown to array index
	locThrownIndexMap.clear();
	for(size_t loc_i = 0; loc_i < locMCThrownsToSave.size(); ++loc_i)
		locThrownIndexMap[locMCThrownsToSave[loc_i]] = loc_i;
}

void DEventWriterROOT::Fill_ThrownInfo(DTreeFillData* locTreeFillData, const DMCReaction* locMCReaction, const DBeamPhoton* locTaggedMCGenBeam, const vector<const DMCThrown*>& locMCThrowns, const map<const DMCThrown*, unsigned int>& locThrownIndexMap,  ULong64_t locNumPIDThrown_FinalState, ULong64_t locPIDThrown_Decaying,  const vector<const DMCTrajectoryPoint*> locDMCTrajectoryPoints, const DMCThrownMatching* locMCThrownMatching) const
{
	//THIS MUST BE CALLED FROM WITHIN A LOCK, SO DO NOT PASS IN JEVENTLOOP! //TOO TEMPTING TO DO SOMETHING BAD

	//WEIGHT
	locTreeFillData->Fill_Single<Float_t>("MCWeight", locMCReaction->weight);

	//THROWN BEAM
	locTreeFillData->Fill_Single<Int_t>(Build_BranchName("ThrownBeam", "PID"), PDGtype(locMCReaction->beam.PID()));
	locTreeFillData->Fill_Single<Float_t>(Build_BranchName("ThrownBeam", "GeneratedEnergy"), locMCReaction->beam.energy());

	DVector3 locThrownBeamX3 = locMCReaction->beam.position();
	TLorentzVector locThrownBeamTX4(locThrownBeamX3.X(), locThrownBeamX3.Y(), locThrownBeamX3.Z(), locMCReaction->beam.time());
	locTreeFillData->Fill_Single<TLorentzVector>(Build_BranchName("ThrownBeam", "X4"), locThrownBeamTX4);

	DLorentzVector locThrownBeamP4 = locTaggedMCGenBeam->lorentzMomentum();
	TLorentzVector locThrownBeamTP4(locThrownBeamP4.Px(), locThrownBeamP4.Py(), locThrownBeamP4.Pz(), locThrownBeamP4.E());
	locTreeFillData->Fill_Single<TLorentzVector>(Build_BranchName("ThrownBeam", "P4"), locThrownBeamTP4);

	//THROWN PRODUCTS
	locTreeFillData->Fill_Single<UInt_t>("NumThrown", locMCThrowns.size());
	for(size_t loc_i = 0; loc_i < locMCThrowns.size(); ++loc_i)
		Fill_ThrownParticleData(locTreeFillData, loc_i, locMCThrowns[loc_i], locThrownIndexMap, locMCThrownMatching);

	//PID INFO
	locTreeFillData->Fill_Single<ULong64_t>("NumPIDThrown_FinalState", locNumPIDThrown_FinalState);
	locTreeFillData->Fill_Single<ULong64_t>("PIDThrown_Decaying", locPIDThrown_Decaying);
    
	if(STORE_MC_TRAJECTORIES && locDMCTrajectoryPoints.size()>=1) Fill_ThrownParticleTrajectoryInfo(locTreeFillData, locDMCTrajectoryPoints);
}

void DEventWriterROOT::Fill_ThrownParticleData(DTreeFillData* locTreeFillData, unsigned int locArrayIndex, const DMCThrown* locMCThrown, const map<const DMCThrown*, unsigned int>& locThrownIndexMap, const DMCThrownMatching* locMCThrownMatching) const
{
	string locParticleBranchName = "Thrown";

	//IDENTIFIERS
	int locParentIndex = -1; //e.g. photoproduced
	map<const DMCThrown*, unsigned int>::const_iterator locIterator;
	for(locIterator = locThrownIndexMap.begin(); locIterator != locThrownIndexMap.end(); ++locIterator)
	{
		if(locIterator->first->myid != locMCThrown->parentid)
			continue;
		locParentIndex = locIterator->second;
		break;
	}
	locTreeFillData->Fill_Array<Int_t>(Build_BranchName(locParticleBranchName, "ParentIndex"), locParentIndex, locArrayIndex);
	locTreeFillData->Fill_Array<Int_t>(Build_BranchName(locParticleBranchName, "PID"), locMCThrown->pdgtype, locArrayIndex);

	//MATCHING
	if(locMCThrownMatching != NULL)
	{
		Int_t locMatchID = -1;
		double locMatchFOM = -1.0;
		if(ParticleCharge(locMCThrown->PID()) != 0)
		{
			const DChargedTrack* locChargedTrack = locMCThrownMatching->Get_MatchingChargedTrack(locMCThrown, locMatchFOM);
			if(locChargedTrack != NULL)
				locMatchID = locChargedTrack->candidateid;
		}
		else
		{
			//Can't use DNeutralShower JObject::id (must use dShowerID): 
				//Matching done with default-tag showers, but pre-select showers are saved to tree: JObject::id's don't match
			const DNeutralShower* locNeutralShower = locMCThrownMatching->Get_MatchingNeutralShower(locMCThrown, locMatchFOM);
			if(locNeutralShower != NULL)
				locMatchID = locNeutralShower->dShowerID;
		}
		locTreeFillData->Fill_Array<Int_t>(Build_BranchName(locParticleBranchName, "MatchID"), locMatchID, locArrayIndex);
		locTreeFillData->Fill_Array<Float_t>(Build_BranchName(locParticleBranchName, "MatchFOM"), locMatchFOM, locArrayIndex);
	}

	//KINEMATICS: THROWN //at the production vertex
	TLorentzVector locX4_Thrown(locMCThrown->position().X(), locMCThrown->position().Y(), locMCThrown->position().Z(), locMCThrown->time());
	locTreeFillData->Fill_Array<TLorentzVector>(Build_BranchName(locParticleBranchName, "X4"), locX4_Thrown, locArrayIndex);
	TLorentzVector locP4_Thrown(locMCThrown->momentum().X(), locMCThrown->momentum().Y(), locMCThrown->momentum().Z(), locMCThrown->energy());
	locTreeFillData->Fill_Array<TLorentzVector>(Build_BranchName(locParticleBranchName, "P4"), locP4_Thrown, locArrayIndex);
}

void DEventWriterROOT::Fill_ThrownParticleTrajectoryInfo(DTreeFillData* locTreeFillData, const vector<const DMCTrajectoryPoint*> locDMCTrajectoryPoints) const {

	string locParticleBranchName = "Thrown";
    Int_t all_index  =0;
	Int_t birth_index=0;
	Int_t death_index=0;

    // Array has to be reversed to match thrown tree ordering elsewhere
	for(int loc_i = locDMCTrajectoryPoints.size()-1; loc_i>=0; loc_i--) {
		Int_t PID = locDMCTrajectoryPoints[loc_i]->part;
		TLorentzVector traj_x4 = TLorentzVector(locDMCTrajectoryPoints[loc_i]->x,locDMCTrajectoryPoints[loc_i]->y,locDMCTrajectoryPoints[loc_i]->z,locDMCTrajectoryPoints[loc_i]->t);
		TLorentzVector traj_p4 = TLorentzVector(locDMCTrajectoryPoints[loc_i]->px,locDMCTrajectoryPoints[loc_i]->py,locDMCTrajectoryPoints[loc_i]->pz,locDMCTrajectoryPoints[loc_i]->E);
		if(all_index%2==1) { // Odd indices: birth
			if(locDMCTrajectoryPoints[loc_i]->step > 1e-10) {cout << "ERROR: DMCTrajectoryPoint FOUND IN UNEXPECTECTED ORDERING! Will not store event. Is TRAJECTORIES card >=2?" << endl; continue;}
			locTreeFillData->Fill_Array<Int_t>(Build_BranchName(locParticleBranchName, "PID_trajBirth"),PID,birth_index);
			locTreeFillData->Fill_Array<TLorentzVector>(Build_BranchName(locParticleBranchName, "X4_trajBirth"),traj_x4,birth_index);
			locTreeFillData->Fill_Array<TLorentzVector>(Build_BranchName(locParticleBranchName, "P4_trajBirth"),traj_p4,birth_index);
			birth_index++;
		}
		if(all_index%2==0) { // Even indices: death
			if(locDMCTrajectoryPoints[loc_i]->step < 1e-10) {cout << "ERROR: DMCTrajectoryPoint FOUND IN UNEXPECTECTED ORDERING! Will not store event. Is TRAJECTORIES card >=2?" << endl; continue;}
			locTreeFillData->Fill_Array<Int_t>(Build_BranchName(locParticleBranchName, "PID_trajDeath"),PID,death_index);
			locTreeFillData->Fill_Array<TLorentzVector>(Build_BranchName(locParticleBranchName, "X4_trajDeath"),traj_x4,death_index);
			// locTreeFillData->Fill_Array<TLorentzVector>(Build_BranchName(locParticleBranchName, "P4_trajDeath"),traj_x4,death_index); //not useful
			death_index++;
		}
        all_index++;
	}

}

void DEventWriterROOT::Fill_BeamData(DTreeFillData* locTreeFillData, unsigned int locArrayIndex, const DBeamPhoton* locBeamPhoton, const DVertex* locVertex, const DMCThrownMatching* locMCThrownMatching) const
{
	string locParticleBranchName = "Beam";

	//IDENTIFIER
	locTreeFillData->Fill_Array<Int_t>(Build_BranchName(locParticleBranchName, "PID"), PDGtype(locBeamPhoton->PID()), locArrayIndex);

	//MATCHING
	if(locMCThrownMatching != NULL)
	{
	        // Tag the thrown beam photon by comparing energy, time and counter
	        const DBeamPhoton* locBeamPhotonTaggedMCGEN = locMCThrownMatching->Get_TaggedMCGENBeamPhoton();
		Bool_t locIsGeneratorFlag = kFALSE;
		if (locBeamPhotonTaggedMCGEN != NULL)
		  locIsGeneratorFlag = (locBeamPhotonTaggedMCGEN->energy() == locBeamPhoton->energy() && locBeamPhotonTaggedMCGEN->dCounter == locBeamPhoton->dCounter && locBeamPhotonTaggedMCGEN->time() == locBeamPhoton->time()) ? kTRUE : kFALSE;
		locTreeFillData->Fill_Array<Bool_t>(Build_BranchName(locParticleBranchName, "IsGenerator"), locIsGeneratorFlag, locArrayIndex);
	}

	//KINEMATICS: MEASURED

	//use production vertex, propagate photon time
	DVector3 locTargetCenter = locBeamPhoton->position();
	DVector3 locProductionVertex = locVertex->dSpacetimeVertex.Vect();
	double locDeltaPath = (locProductionVertex - locTargetCenter).Mag();
	bool locDownstreamFlag = ((locProductionVertex.Z() - locTargetCenter.Z()) > 0.0);
	double locDeltaT = locDownstreamFlag ? locDeltaPath/29.9792458 : -1.0*locDeltaPath/29.9792458;
	double locTime = locBeamPhoton->time() + locDeltaT;

	TLorentzVector locX4_Measured(locProductionVertex.X(), locProductionVertex.Y(), locProductionVertex.Z(), locTime);
	locTreeFillData->Fill_Array<TLorentzVector>(Build_BranchName(locParticleBranchName, "X4_Measured"), locX4_Measured, locArrayIndex);

	DLorentzVector locDP4 = locBeamPhoton->lorentzMomentum();
	TLorentzVector locP4_Measured(locDP4.Px(), locDP4.Py(), locDP4.Pz(), locDP4.E());
	locTreeFillData->Fill_Array<TLorentzVector>(Build_BranchName(locParticleBranchName, "P4_Measured"), locP4_Measured, locArrayIndex);
}

void DEventWriterROOT::Fill_ChargedHypo(DTreeFillData* locTreeFillData, unsigned int locArrayIndex, const DChargedTrackHypothesis* locChargedTrackHypothesis, const DMCThrownMatching* locMCThrownMatching, const map<const DMCThrown*, unsigned int>& locThrownIndexMap, const DDetectorMatches* locDetectorMatches) const
{
	string locParticleBranchName = "ChargedHypo";

	//ASSOCIATED OBJECTS
	auto locTrackTimeBased = locChargedTrackHypothesis->Get_TrackTimeBased();

	const DBCALShower* locBCALShower = NULL;
	shared_ptr<const DBCALShowerMatchParams>locBCALShowerMatchParams
	  =locChargedTrackHypothesis->Get_BCALShowerMatchParams();
	if (locBCALShowerMatchParams!= nullptr){
	  locBCALShower = locBCALShowerMatchParams->dBCALShower;
	}

	const DFCALShower* locFCALShower = NULL;
	shared_ptr<const DFCALShowerMatchParams> locFCALShowerMatchParams
	  =locChargedTrackHypothesis->Get_FCALShowerMatchParams();
	if(locFCALShowerMatchParams != nullptr){
	  locFCALShower = locFCALShowerMatchParams->dFCALShower;
	}

	shared_ptr<const DFCALSingleHitMatchParams> locFCALSingleHitMatchParams
	  = locChargedTrackHypothesis->Get_FCALSingleHitMatchParams();

	//IDENTIFIERS
	locTreeFillData->Fill_Array<Int_t>(Build_BranchName(locParticleBranchName, "TrackID"), locTrackTimeBased->candidateid, locArrayIndex);
	locTreeFillData->Fill_Array<Int_t>(Build_BranchName(locParticleBranchName, "PID"), PDGtype(locChargedTrackHypothesis->PID()), locArrayIndex);

	//MATCHING
	if(locMCThrownMatching != NULL)
	{
		Int_t locThrownIndex = -1;
		double locMatchFOM = 0.0;
		const DMCThrown* locMCThrown = locMCThrownMatching->Get_MatchingMCThrown(locChargedTrackHypothesis, locMatchFOM);
		if(locMCThrown != NULL)
			locThrownIndex = locThrownIndexMap.find(locMCThrown)->second;
		locTreeFillData->Fill_Array<Int_t>(Build_BranchName(locParticleBranchName, "ThrownIndex"), locThrownIndex, locArrayIndex);
	}

	//KINEMATICS: MEASURED
	DVector3 locPosition = locChargedTrackHypothesis->position();
	TLorentzVector locTX4_Measured(locPosition.X(), locPosition.Y(), locPosition.Z(), locChargedTrackHypothesis->time());
	locTreeFillData->Fill_Array<TLorentzVector>(Build_BranchName(locParticleBranchName, "X4_Measured"), locTX4_Measured, locArrayIndex);

	DLorentzVector locDP4 = locChargedTrackHypothesis->lorentzMomentum();
	TLorentzVector locP4_Measured(locDP4.Px(), locDP4.Py(), locDP4.Pz(), locDP4.E());
	locTreeFillData->Fill_Array<TLorentzVector>(Build_BranchName(locParticleBranchName, "P4_Measured"), locP4_Measured, locArrayIndex);

	// Global PID
	locTreeFillData->Fill_Array<Float_t>(Build_BranchName(locParticleBranchName, "PIDFOM"), locChargedTrackHypothesis->Get_FOM(), locArrayIndex);

	//TRACKING INFO
	locTreeFillData->Fill_Array<UInt_t>(Build_BranchName(locParticleBranchName, "NDF_Tracking"), locTrackTimeBased->Ndof, locArrayIndex);
	locTreeFillData->Fill_Array<Float_t>(Build_BranchName(locParticleBranchName, "ChiSq_Tracking"), locTrackTimeBased->chisq, locArrayIndex);
	locTreeFillData->Fill_Array<UInt_t>(Build_BranchName(locParticleBranchName, "NDF_DCdEdx"), locChargedTrackHypothesis->Get_NDF_DCdEdx(), locArrayIndex);
	locTreeFillData->Fill_Array<Float_t>(Build_BranchName(locParticleBranchName, "ChiSq_DCdEdx"), locChargedTrackHypothesis->Get_ChiSq_DCdEdx(), locArrayIndex);
	locTreeFillData->Fill_Array<Float_t>(Build_BranchName(locParticleBranchName, "dEdx_CDC"), locChargedTrackHypothesis->Get_dEdx_CDC_amp(), locArrayIndex);
	locTreeFillData->Fill_Array<Float_t>(Build_BranchName(locParticleBranchName, "dEdx_CDC_integral"), locChargedTrackHypothesis->Get_dEdx_CDC_int(), locArrayIndex);
	locTreeFillData->Fill_Array<Float_t>(Build_BranchName(locParticleBranchName, "dEdx_FDC"), locTrackTimeBased->ddEdx_FDC, locArrayIndex);
	
	if (FDC_VERBOSE_OUTPUT){
	  if (locTrackTimeBased->extrapolations.find(SYS_FDC) != locTrackTimeBased->extrapolations.end()) {
	    vector<DTrackFitter::Extrapolation_t>locExtraps=locTrackTimeBased->extrapolations.at(SYS_FDC);
	    int locPackageIndex=-1,locOldIndex=-1;
	    for (unsigned int j=0;j<locExtraps.size();j++){
	      DVector3 locPos=locExtraps[j].position;
	      double locZ=locPos.z();
	   
	      if (locZ>dFdcPackages[3]) locPackageIndex=3;
	      else if (locZ>dFdcPackages[2]) locPackageIndex=2;
	      else if (locZ>dFdcPackages[1]) locPackageIndex=1;
	      else locPackageIndex=0;

	      if (locPackageIndex!=locOldIndex){
		locTreeFillData->Fill_Array<Float_t>(Build_BranchName(locParticleBranchName, dFDCxLeaves[locPackageIndex].c_str()), locPos.x(), locArrayIndex);
		locTreeFillData->Fill_Array<Float_t>(Build_BranchName(locParticleBranchName, dFDCyLeaves[locPackageIndex].c_str()), locPos.y(), locArrayIndex);
	      }
	      
	      if (locPackageIndex==3) break;
	      locOldIndex=locPackageIndex;
	    }
	  }
	}
	
	//HIT ENERGY
	double locTOFdEdx = (locChargedTrackHypothesis->Get_TOFHitMatchParams() != NULL) ? locChargedTrackHypothesis->Get_TOFHitMatchParams()->dEdx : 0.0;
	locTreeFillData->Fill_Array<Float_t>(Build_BranchName(locParticleBranchName, "dEdx_TOF"), locTOFdEdx, locArrayIndex);
	double locSCdEdx = (locChargedTrackHypothesis->Get_SCHitMatchParams() != NULL) ? locChargedTrackHypothesis->Get_SCHitMatchParams()->dEdx : 0.0;
	locTreeFillData->Fill_Array<Float_t>(Build_BranchName(locParticleBranchName, "dEdx_ST"), locSCdEdx, locArrayIndex);
	double locBCALEnergy = (locBCALShower != NULL) ? locBCALShower->E : 0.0;
	locTreeFillData->Fill_Array<Float_t>(Build_BranchName(locParticleBranchName, "Energy_BCAL"), locBCALEnergy, locArrayIndex);
	double locBCALPreshowerEnergy = (locBCALShower != NULL) ? locBCALShower->E_preshower : 0.0;
	locTreeFillData->Fill_Array<Float_t>(Build_BranchName(locParticleBranchName, "Energy_BCALPreshower"), locBCALPreshowerEnergy, locArrayIndex);
	if(BCAL_VERBOSE_OUTPUT) {
		double locBCALLayer2Energy = (locBCALShower != NULL) ? locBCALShower->E_L2 : 0.0;
		locTreeFillData->Fill_Array<Float_t>(Build_BranchName(locParticleBranchName, "Energy_BCALLayer2"), locBCALLayer2Energy, locArrayIndex);
		double locBCALLayer3Energy = (locBCALShower != NULL) ? locBCALShower->E_L3 : 0.0;
		locTreeFillData->Fill_Array<Float_t>(Build_BranchName(locParticleBranchName, "Energy_BCALLayer3"), locBCALLayer3Energy, locArrayIndex);
		double locBCALLayer4Energy = (locBCALShower != NULL) ? locBCALShower->E_L4 : 0.0;
		locTreeFillData->Fill_Array<Float_t>(Build_BranchName(locParticleBranchName, "Energy_BCALLayer4"), locBCALLayer4Energy, locArrayIndex);
	}
	
	double locFCALEnergy = (locFCALShower != NULL) ? locFCALShower->getEnergy() : 0.0;
	locTreeFillData->Fill_Array<Float_t>(Build_BranchName(locParticleBranchName, "Energy_FCAL"), locFCALEnergy, locArrayIndex);

	if(locFCALSingleHitMatchParams!=nullptr){
	  locFCALEnergy = locFCALSingleHitMatchParams->dEHit;
	  locTreeFillData->Fill_Array<Float_t>(Build_BranchName(locParticleBranchName, "Energy_FCAL"), locFCALEnergy, locArrayIndex);
	}
	
	//double locCCALEnergy = (locCCALShower != NULL) ? locCCALShower->getEnergy() : 0.0;
	//locTreeFillData->Fill_Array<Float_t>(Build_BranchName(locParticleBranchName, "Energy_CCAL"), locCCALEnergy, locArrayIndex);

	//SHOWER PROPERTIES
	double locSigLongBCAL = (locBCALShower != NULL) ? locBCALShower->sigLong : 0.0;
	double locSigThetaBCAL = (locBCALShower != NULL) ? locBCALShower->sigTheta : 0.0;
	double locSigTransBCAL = (locBCALShower != NULL) ? locBCALShower->sigTrans : 0.0;
	double locRMSTimeBCAL = (locBCALShower != NULL) ? locBCALShower->rmsTime : 0.0;
	locTreeFillData->Fill_Array<Float_t>(Build_BranchName(locParticleBranchName, "SigLong_BCAL"), locSigLongBCAL, locArrayIndex);
	locTreeFillData->Fill_Array<Float_t>(Build_BranchName(locParticleBranchName, "SigTheta_BCAL"), locSigThetaBCAL, locArrayIndex);
	locTreeFillData->Fill_Array<Float_t>(Build_BranchName(locParticleBranchName, "SigTrans_BCAL"), locSigTransBCAL, locArrayIndex);
	locTreeFillData->Fill_Array<Float_t>(Build_BranchName(locParticleBranchName, "RMSTime_BCAL"), locRMSTimeBCAL, locArrayIndex);
	
	double locE1E9FCAL = (locFCALShower != NULL) ? locFCALShower->getE1E9() : 0.0;
	double locE9E25FCAL = (locFCALShower != NULL) ? locFCALShower->getE9E25() : 0.0;
	double locSumUFCAL = (locFCALShower != NULL) ? locFCALShower->getSumU() : 0.0;
	double locSumVFCAL = (locFCALShower != NULL) ? locFCALShower->getSumV() : 0.0;
	double locNumBlocksFCAL = (locFCALShower != NULL) ? locFCALShower->getNumBlocks() : 0.0;
	if (locFCALSingleHitMatchParams!=nullptr) locNumBlocksFCAL=1.;
	locTreeFillData->Fill_Array<Float_t>(Build_BranchName(locParticleBranchName, "E1E9_FCAL"), locE1E9FCAL, locArrayIndex);
	locTreeFillData->Fill_Array<Float_t>(Build_BranchName(locParticleBranchName, "E9E25_FCAL"), locE9E25FCAL, locArrayIndex);
	locTreeFillData->Fill_Array<Float_t>(Build_BranchName(locParticleBranchName, "SumU_FCAL"), locSumUFCAL, locArrayIndex);
	locTreeFillData->Fill_Array<Float_t>(Build_BranchName(locParticleBranchName, "SumV_FCAL"), locSumVFCAL, locArrayIndex);
	locTreeFillData->Fill_Array<Float_t>(Build_BranchName(locParticleBranchName, "NumBlocks_FCAL"), locNumBlocksFCAL, locArrayIndex);

	//TIMING INFO
	locTreeFillData->Fill_Array<Float_t>(Build_BranchName(locParticleBranchName, "HitTime"), locChargedTrackHypothesis->t1(), locArrayIndex);
	double locStartTimeError = locChargedTrackHypothesis->t0_err();
	double locRFDeltaTVariance = (*locChargedTrackHypothesis->errorMatrix())(6, 6) + locStartTimeError*locStartTimeError;
	locTreeFillData->Fill_Array<Float_t>(Build_BranchName(locParticleBranchName, "RFDeltaTVar"), locRFDeltaTVariance, locArrayIndex);

	//MEASURED PID INFO
	locTreeFillData->Fill_Array<Float_t>(Build_BranchName(locParticleBranchName, "Beta_Timing"), locChargedTrackHypothesis->measuredBeta(), locArrayIndex);
	locTreeFillData->Fill_Array<Float_t>(Build_BranchName(locParticleBranchName, "ChiSq_Timing"), locChargedTrackHypothesis->Get_ChiSq_Timing(), locArrayIndex);
	locTreeFillData->Fill_Array<UInt_t>(Build_BranchName(locParticleBranchName, "NDF_Timing"), locChargedTrackHypothesis->Get_NDF_Timing(), locArrayIndex);

	//SHOWER MATCHING: BCAL
	double locTrackBCAL_DeltaPhi = 999.0, locTrackBCAL_DeltaZ = 999.0;
	if(locBCALShowerMatchParams != nullptr)
	{
	  locTrackBCAL_DeltaPhi = locBCALShowerMatchParams->dDeltaPhiToShower;
	  locTrackBCAL_DeltaZ = locBCALShowerMatchParams->dDeltaZToShower;
	}
	locTreeFillData->Fill_Array<Float_t>(Build_BranchName(locParticleBranchName, "TrackBCAL_DeltaPhi"), locTrackBCAL_DeltaPhi, locArrayIndex);
	locTreeFillData->Fill_Array<Float_t>(Build_BranchName(locParticleBranchName, "TrackBCAL_DeltaZ"), locTrackBCAL_DeltaZ, locArrayIndex);

	//SHOWER MATCHING: FCAL
	double locDOCAToShower_FCAL = 999.0;
	double locDeltaT_TrackToShower_FCAL = 999.0;
	if(locFCALShowerMatchParams!=nullptr) {
	  locDOCAToShower_FCAL = locFCALShowerMatchParams->dDOCAToShower;
	  locDeltaT_TrackToShower_FCAL = locFCALShower->getTime() 
	    -(locChargedTrackHypothesis->t0()
	      + locFCALShowerMatchParams->dFlightTime);
	}
	locTreeFillData->Fill_Array<Float_t>(Build_BranchName(locParticleBranchName, "TrackFCAL_DOCA"), locDOCAToShower_FCAL, locArrayIndex);
	locTreeFillData->Fill_Array<Float_t>(Build_BranchName(locParticleBranchName, "TrackFCAL_DeltaT"), locDeltaT_TrackToShower_FCAL, locArrayIndex);

	// Single hit matching:  FCAL
	if(locFCALSingleHitMatchParams!=nullptr){
	  double locDOCAToFCALHit = locFCALSingleHitMatchParams->dDOCAToHit;
	  locTreeFillData->Fill_Array<Float_t>(Build_BranchName(locParticleBranchName, "TrackFCAL_DOCA"), locDOCAToFCALHit, locArrayIndex);

	  locDeltaT_TrackToShower_FCAL = locFCALSingleHitMatchParams->dTHit
	    - ( locChargedTrackHypothesis->t0() 
		+ locFCALSingleHitMatchParams->dFlightTime);
	  locTreeFillData->Fill_Array<Float_t>(Build_BranchName(locParticleBranchName, "TrackFCAL_DeltaT"), locDeltaT_TrackToShower_FCAL, locArrayIndex);

	}

	// DIRC
	if(DIRC_OUTPUT) {
		int locDIRCNumPhotons = 0;
		double locDIRCExtrapolatedX = 999;
		double locDIRCExtrapolatedY = 999;
		double locDIRCThetaC = 999.;
		double locDIRCLele = 999.;
		double locDIRCLpi = 999.;
		double locDIRCLk = 999.;
		double locDIRCLp = 999.;
		auto locDIRCMatchParams = locChargedTrackHypothesis->Get_DIRCMatchParams();
		if(locDIRCMatchParams != NULL) {
		        locDIRCExtrapolatedX = locDIRCMatchParams->dExtrapolatedPos.X();
			locDIRCExtrapolatedY = locDIRCMatchParams->dExtrapolatedPos.Y();
			locDIRCNumPhotons = locDIRCMatchParams->dNPhotons;
			locDIRCThetaC = locDIRCMatchParams->dThetaC;
			locDIRCLele =  locDIRCMatchParams->dLikelihoodElectron;
			locDIRCLpi =  locDIRCMatchParams->dLikelihoodPion;	
			locDIRCLk =  locDIRCMatchParams->dLikelihoodKaon;
			locDIRCLp =  locDIRCMatchParams->dLikelihoodProton;
		}
		locTreeFillData->Fill_Array<Int_t>(Build_BranchName(locParticleBranchName, "NumPhotons_DIRC"), locDIRCNumPhotons, locArrayIndex);
		locTreeFillData->Fill_Array<Float_t>(Build_BranchName(locParticleBranchName, "ExtrapolatedX_DIRC"), locDIRCExtrapolatedX, locArrayIndex);
		locTreeFillData->Fill_Array<Float_t>(Build_BranchName(locParticleBranchName, "ExtrapolatedY_DIRC"), locDIRCExtrapolatedY, locArrayIndex);
		locTreeFillData->Fill_Array<Float_t>(Build_BranchName(locParticleBranchName, "ThetaC_DIRC"), locDIRCThetaC, locArrayIndex);
		locTreeFillData->Fill_Array<Float_t>(Build_BranchName(locParticleBranchName, "Lele_DIRC"), locDIRCLele, locArrayIndex);
		locTreeFillData->Fill_Array<Float_t>(Build_BranchName(locParticleBranchName, "Lpi_DIRC"), locDIRCLpi, locArrayIndex);
		locTreeFillData->Fill_Array<Float_t>(Build_BranchName(locParticleBranchName, "Lk_DIRC"), locDIRCLk, locArrayIndex);
		locTreeFillData->Fill_Array<Float_t>(Build_BranchName(locParticleBranchName, "Lp_DIRC"), locDIRCLp, locArrayIndex);
	}
}

void DEventWriterROOT::Fill_NeutralHypo(DTreeFillData* locTreeFillData, unsigned int locArrayIndex, const DNeutralParticleHypothesis* locNeutralParticleHypothesis, const DMCThrownMatching* locMCThrownMatching, const map<const DMCThrown*, unsigned int>& locThrownIndexMap, const DDetectorMatches* locDetectorMatches) const
{
	string locParticleBranchName = "NeutralHypo";
	const DNeutralShower* locNeutralShower = locNeutralParticleHypothesis->Get_NeutralShower();

	//ASSOCIATED OBJECTS
	const DBCALShower* locBCALShower = NULL;
	locNeutralShower->GetSingle(locBCALShower);
	const DFCALShower* locFCALShower = NULL;
	locNeutralShower->GetSingle(locFCALShower);
	const DCCALShower* locCCALShower = NULL;
	locNeutralShower->GetSingle(locCCALShower);

	//IDENTIFIERS
	Particle_t locPID = locNeutralParticleHypothesis->PID();
	locTreeFillData->Fill_Array<Int_t>(Build_BranchName(locParticleBranchName, "NeutralID"), locNeutralShower->dShowerID, locArrayIndex);
	locTreeFillData->Fill_Array<Int_t>(Build_BranchName(locParticleBranchName, "PID"), PDGtype(locPID), locArrayIndex);

	//MATCHING
	if(locMCThrownMatching != NULL)
	{
		Int_t locThrownIndex = -1;
		double locMatchFOM = 0.0;
		const DMCThrown* locMCThrown = locMCThrownMatching->Get_MatchingMCThrown(locNeutralParticleHypothesis, locMatchFOM);
		if(locMCThrown != NULL)
			locThrownIndex = locThrownIndexMap.find(locMCThrown)->second;
		locTreeFillData->Fill_Array<Int_t>(Build_BranchName(locParticleBranchName, "ThrownIndex"), locThrownIndex, locArrayIndex);
	}

	//KINEMATICS: MEASURED
	DVector3 locPosition = locNeutralParticleHypothesis->position();
	TLorentzVector locX4_Measured(locPosition.X(), locPosition.Y(), locPosition.Z(), locNeutralParticleHypothesis->time());
	locTreeFillData->Fill_Array<TLorentzVector>(Build_BranchName(locParticleBranchName, "X4_Measured"), locX4_Measured, locArrayIndex);

	DLorentzVector locDP4 = locNeutralParticleHypothesis->lorentzMomentum();
	TLorentzVector locP4_Measured(locDP4.Px(), locDP4.Py(), locDP4.Pz(), locDP4.E());
	locTreeFillData->Fill_Array<TLorentzVector>(Build_BranchName(locParticleBranchName, "P4_Measured"), locP4_Measured, locArrayIndex);

	//MEASURED PID INFO
	locTreeFillData->Fill_Array<Float_t>(Build_BranchName(locParticleBranchName, "Beta_Timing"), locNeutralParticleHypothesis->measuredBeta(), locArrayIndex);
	locTreeFillData->Fill_Array<Float_t>(Build_BranchName(locParticleBranchName, "ChiSq_Timing"), locNeutralParticleHypothesis->Get_ChiSq(), locArrayIndex);
	locTreeFillData->Fill_Array<UInt_t>(Build_BranchName(locParticleBranchName, "NDF_Timing"), locNeutralParticleHypothesis->Get_NDF(), locArrayIndex);
	locTreeFillData->Fill_Array<Float_t>(Build_BranchName(locParticleBranchName, "ShowerQuality"), locNeutralShower->dQuality, locArrayIndex);
	if (STORE_SC_VETO_INFO) {
	  locTreeFillData->Fill_Array<Int_t>(Build_BranchName(locParticleBranchName, "ShowerSC_BCAL_match"), locNeutralShower->dSC_BCAL_match, locArrayIndex);
	  locTreeFillData->Fill_Array<Int_t>(Build_BranchName(locParticleBranchName, "ShowerSC_FCAL_match"), locNeutralShower->dSC_FCAL_match, locArrayIndex);
	  locTreeFillData->Fill_Array<Float_t>(Build_BranchName(locParticleBranchName, "ShowerSC_BCAL_phi_min"), locNeutralShower->dSC_BCAL_phi_min, locArrayIndex);
	  locTreeFillData->Fill_Array<Float_t>(Build_BranchName(locParticleBranchName, "ShowerSC_FCAL_phi_min"), locNeutralShower->dSC_FCAL_phi_min, locArrayIndex);
	}
	locTreeFillData->Fill_Array<Int_t>(Build_BranchName(locParticleBranchName, "ShowerTOF_FCAL_match"), locNeutralShower->dTOF_FCAL_match, locArrayIndex);
	locTreeFillData->Fill_Array<Float_t>(Build_BranchName(locParticleBranchName, "ShowerTOF_FCAL_x_min"), locNeutralShower->dTOF_FCAL_x_min, locArrayIndex);
	locTreeFillData->Fill_Array<Float_t>(Build_BranchName(locParticleBranchName, "ShowerTOF_FCAL_y_min"), locNeutralShower->dTOF_FCAL_y_min, locArrayIndex);
	
	//SHOWER ENERGY
	DetectorSystem_t locDetector = locNeutralShower->dDetectorSystem;
	double locBCALEnergy = (locDetector == SYS_BCAL) ? locNeutralShower->dEnergy : 0.0;
	locTreeFillData->Fill_Array<Float_t>(Build_BranchName(locParticleBranchName, "Energy_BCAL"), locBCALEnergy, locArrayIndex);
	double locBCALPreshowerEnergy = (locDetector == SYS_BCAL) ? static_cast<const DBCALShower*>(locNeutralShower->dBCALFCALShower)->E_preshower : 0.0;
	locTreeFillData->Fill_Array<Float_t>(Build_BranchName(locParticleBranchName, "Energy_BCALPreshower"), locBCALPreshowerEnergy, locArrayIndex);
	if(BCAL_VERBOSE_OUTPUT) {
		double locBCALLayer2Energy = (locBCALShower != NULL) ? locBCALShower->E_L2 : 0.0;
		locTreeFillData->Fill_Array<Float_t>(Build_BranchName(locParticleBranchName, "Energy_BCALLayer2"), locBCALLayer2Energy, locArrayIndex);
		double locBCALLayer3Energy = (locBCALShower != NULL) ? locBCALShower->E_L3 : 0.0;
		locTreeFillData->Fill_Array<Float_t>(Build_BranchName(locParticleBranchName, "Energy_BCALLayer3"), locBCALLayer3Energy, locArrayIndex);
		double locBCALLayer4Energy = (locBCALShower != NULL) ? locBCALShower->E_L4 : 0.0;
		locTreeFillData->Fill_Array<Float_t>(Build_BranchName(locParticleBranchName, "Energy_BCALLayer4"), locBCALLayer4Energy, locArrayIndex);
	}
	
	double locFCALEnergy = (locDetector == SYS_FCAL) ? locNeutralShower->dEnergy : 0.0;
	locTreeFillData->Fill_Array<Float_t>(Build_BranchName(locParticleBranchName, "Energy_FCAL"), locFCALEnergy, locArrayIndex);

	double locCCALEnergy = (locDetector == SYS_CCAL) ? locNeutralShower->dEnergy : 0.0;
	locTreeFillData->Fill_Array<Float_t>(Build_BranchName(locParticleBranchName, "Energy_CCAL"), locCCALEnergy, locArrayIndex);

	//SHOWER POSITION
	DLorentzVector locHitDX4 = locNeutralShower->dSpacetimeVertex;
	TLorentzVector locTX4_Shower(locHitDX4.X(), locHitDX4.Y(), locHitDX4.Z(), locHitDX4.T());
	locTreeFillData->Fill_Array<TLorentzVector>(Build_BranchName(locParticleBranchName, "X4_Shower"), locTX4_Shower, locArrayIndex);

	//SHOWER PROPERTIES
	double locSigLongBCAL = (locBCALShower != NULL) ? locBCALShower->sigLong : 0.0;
	double locSigThetaBCAL = (locBCALShower != NULL) ? locBCALShower->sigTheta : 0.0;
	double locSigTransBCAL = (locBCALShower != NULL) ? locBCALShower->sigTrans : 0.0;
	double locRMSTimeBCAL = (locBCALShower != NULL) ? locBCALShower->rmsTime : 0.0;
	locTreeFillData->Fill_Array<Float_t>(Build_BranchName(locParticleBranchName, "SigLong_BCAL"), locSigLongBCAL, locArrayIndex);
	locTreeFillData->Fill_Array<Float_t>(Build_BranchName(locParticleBranchName, "SigTheta_BCAL"), locSigThetaBCAL, locArrayIndex);
	locTreeFillData->Fill_Array<Float_t>(Build_BranchName(locParticleBranchName, "SigTrans_BCAL"), locSigTransBCAL, locArrayIndex);
	locTreeFillData->Fill_Array<Float_t>(Build_BranchName(locParticleBranchName, "RMSTime_BCAL"), locRMSTimeBCAL, locArrayIndex);
	
	if(FCAL_VERBOSE_OUTPUT) {
		double locE1E9FCAL = (locFCALShower != NULL) ? locFCALShower->getE1E9() : 0.0;
		double locE9E25FCAL = (locFCALShower != NULL) ? locFCALShower->getE9E25() : 0.0;
		double locSumUFCAL = (locFCALShower != NULL) ? locFCALShower->getSumU() : 0.0;
		double locSumVFCAL = (locFCALShower != NULL) ? locFCALShower->getSumV() : 0.0;
        double locNumBlocksFCAL = (locFCALShower != NULL) ? locFCALShower->getNumBlocks() : 0.0;
		locTreeFillData->Fill_Array<Float_t>(Build_BranchName(locParticleBranchName, "E1E9_FCAL"), locE1E9FCAL, locArrayIndex);
		locTreeFillData->Fill_Array<Float_t>(Build_BranchName(locParticleBranchName, "E9E25_FCAL"), locE9E25FCAL, locArrayIndex);
		locTreeFillData->Fill_Array<Float_t>(Build_BranchName(locParticleBranchName, "SumU_FCAL"), locSumUFCAL, locArrayIndex);
		locTreeFillData->Fill_Array<Float_t>(Build_BranchName(locParticleBranchName, "SumV_FCAL"), locSumVFCAL, locArrayIndex);
        locTreeFillData->Fill_Array<Float_t>(Build_BranchName(locParticleBranchName, "NumBlocks_FCAL"), locNumBlocksFCAL, locArrayIndex);
	}
	
    
	//Track DOCA to Shower - BCAL
	double locNearestTrackBCALDeltaPhi = 999.0, locNearestTrackBCALDeltaZ = 999.0;
	if(locBCALShower != NULL)
	{
		if(!locDetectorMatches->Get_DistanceToNearestTrack(locBCALShower, locNearestTrackBCALDeltaPhi, locNearestTrackBCALDeltaZ))
		{
			locNearestTrackBCALDeltaPhi = 999.0;
			locNearestTrackBCALDeltaZ = 999.0;
		}
		else if((locNearestTrackBCALDeltaPhi > 999.0) || (locNearestTrackBCALDeltaZ > 999.0))
		{
			locNearestTrackBCALDeltaPhi = 999.0;
			locNearestTrackBCALDeltaZ = 999.0;
		}
	}
	locTreeFillData->Fill_Array<Float_t>(Build_BranchName(locParticleBranchName, "TrackBCAL_DeltaPhi"), locNearestTrackBCALDeltaPhi, locArrayIndex);
	locTreeFillData->Fill_Array<Float_t>(Build_BranchName(locParticleBranchName, "TrackBCAL_DeltaZ"), locNearestTrackBCALDeltaZ, locArrayIndex);

	//Track DOCA to Shower - FCAL
	double locDistanceToNearestTrack_FCAL = 999.0;
	double locDeltaT_TrackToShower_FCAL = 999.0;
	if(locFCALShower != NULL)
	{
		if(!locDetectorMatches->Get_DistanceToNearestTrack(locFCALShower, locDistanceToNearestTrack_FCAL))
			locDistanceToNearestTrack_FCAL = 999.0;
		if(locDistanceToNearestTrack_FCAL > 999.0)
			locDistanceToNearestTrack_FCAL = 999.0;
		locDeltaT_TrackToShower_FCAL = locFCALShower->getTime() - ( locNeutralParticleHypothesis->t0() + locFCALShower->getTimeTrack() );
	}
	locTreeFillData->Fill_Array<Float_t>(Build_BranchName(locParticleBranchName, "TrackFCAL_DOCA"),   locDistanceToNearestTrack_FCAL, locArrayIndex);
	locTreeFillData->Fill_Array<Float_t>(Build_BranchName(locParticleBranchName, "TrackFCAL_DeltaT"), locDeltaT_TrackToShower_FCAL, locArrayIndex);

	//PHOTON PID INFO
	double locStartTimeError = locNeutralParticleHypothesis->t0_err();
	double locPhotonRFDeltaTVar = (*locNeutralParticleHypothesis->errorMatrix())(6, 6) + locStartTimeError*locStartTimeError;
	if(locPID != Gamma)
		locPhotonRFDeltaTVar = 0.0;
	locTreeFillData->Fill_Array<Float_t>(Build_BranchName(locParticleBranchName, "PhotonRFDeltaTVar"), locPhotonRFDeltaTVar, locArrayIndex);
}

void DEventWriterROOT::Fill_ComboData(DTreeFillData* locTreeFillData, const DReaction* locReaction, const DParticleCombo* locParticleCombo, unsigned int locComboIndex, const map<pair<oid_t, Particle_t>, size_t>& locObjectToArrayIndexMap) const
{
	//MAIN CLASSES
	const DKinFitResults* locKinFitResults = locParticleCombo->Get_KinFitResults();
	const DEventRFBunch* locEventRFBunch = locParticleCombo->Get_EventRFBunch();

	//IS COMBO CUT
	locTreeFillData->Fill_Array<Bool_t>("IsComboCut", kFALSE, locComboIndex);

	//RF INFO
	double locRFTime = (locEventRFBunch != NULL) ? locEventRFBunch->dTime : numeric_limits<double>::quiet_NaN();
	locTreeFillData->Fill_Array<Float_t>("RFTime_Measured", locRFTime, locComboIndex);

	//KINFIT INFO
	DKinFitType locKinFitType = locReaction->Get_KinFitType();
	bool locKinFitFlag = (locKinFitType != d_NoFit);
	if(locKinFitFlag)
	{
		if(locKinFitResults != NULL)
		{
			locTreeFillData->Fill_Array<Float_t>("ChiSq_KinFit", locKinFitResults->Get_ChiSq(), locComboIndex);
			locTreeFillData->Fill_Array<UInt_t>("NDF_KinFit", locKinFitResults->Get_NDF(), locComboIndex);
			if((locKinFitType == d_SpacetimeFit) || (locKinFitType == d_P4AndSpacetimeFit))
			{
				double locRFTime_KinFit = -9.9E9; //NOT IMPLEMENTED YET
				locTreeFillData->Fill_Array<Float_t>("RFTime_KinFit", locRFTime_KinFit, locComboIndex);
			}
		}
		else
		{
			locTreeFillData->Fill_Array<Float_t>("ChiSq_KinFit", 0.0, locComboIndex);
			locTreeFillData->Fill_Array<UInt_t>("NDF_KinFit", 0, locComboIndex);
			if((locKinFitType == d_SpacetimeFit) || (locKinFitType == d_P4AndSpacetimeFit))
				locTreeFillData->Fill_Array<Float_t>("RFTime_KinFit", -9.9E9, locComboIndex);
		}
	}

	//STEP DATA
	for(size_t loc_i = 0; loc_i < locParticleCombo->Get_NumParticleComboSteps(); ++loc_i)
		Fill_ComboStepData(locTreeFillData, locReaction, locParticleCombo, loc_i, locComboIndex, locKinFitType, locObjectToArrayIndexMap);
}

void DEventWriterROOT::Fill_ComboStepData(DTreeFillData* locTreeFillData, const DReaction* locReaction, const DParticleCombo* locParticleCombo, unsigned int locStepIndex, unsigned int locComboIndex, DKinFitType locKinFitType, const map<pair<oid_t, Particle_t>, size_t>& locObjectToArrayIndexMap) const
{
	auto locReactionVertexInfo = dVertexInfoMap.find(locReaction)->second;
	auto locReactionStep = locReaction->Get_ReactionStep(locStepIndex);
	const TList* locUserInfo = dTreeInterfaceMap.find(locReaction)->second->Get_UserInfo(); //No Lock!  But this should be unchanging at this point anyway
	const TMap* locPositionToNameMap = (TMap*)locUserInfo->FindObject("PositionToNameMap");

	auto locParticleComboStep = locParticleCombo->Get_ParticleComboStep(locStepIndex);
	DLorentzVector locStepX4 = locParticleComboStep->Get_SpacetimeVertex();
	TLorentzVector locStepTX4(locStepX4.X(), locStepX4.Y(), locStepX4.Z(), locStepX4.T());

	//beam & production vertex
	Particle_t locInitialPID = locReactionStep->Get_InitialPID();
	const DKinematicData* locInitialParticle = locParticleComboStep->Get_InitialParticle();
	const DBeamPhoton* locBeamPhoton = dynamic_cast<const DBeamPhoton*>(locInitialParticle);
	if(locBeamPhoton != NULL)
	{
		const DKinematicData* locInitParticleMeasured = locParticleComboStep->Get_InitialParticle_Measured();
		const DBeamPhoton* locMeasuredBeamPhoton = dynamic_cast<const DBeamPhoton*>(locInitParticleMeasured);

		//get array index
		pair<oid_t, Particle_t> locBeamPair(locMeasuredBeamPhoton->id, locMeasuredBeamPhoton->PID());
		size_t locBeamIndex = locObjectToArrayIndexMap.find(locBeamPair)->second;

		Fill_ComboBeamData(locTreeFillData, locComboIndex, locBeamPhoton, locBeamIndex, locKinFitType);
	}
	else //decaying
	{
		//get the branch name
		ostringstream locPositionStream;
		locPositionStream << locStepIndex << "_-1";
		TObjString* locObjString = (TObjString*)locPositionToNameMap->GetValue(locPositionStream.str().c_str());
		string locParticleBranchName = (const char*)(locObjString->GetString());

		auto locP4FitFlag = ((locKinFitType == d_P4Fit) || (locKinFitType == d_P4AndVertexFit) || (locKinFitType == d_P4AndSpacetimeFit));
		if(IsFixedMass(locInitialPID) && locReactionStep->Get_KinFitConstrainInitMassFlag() && locP4FitFlag)
		{
			TLorentzVector locDecayP4;
			if(locInitialParticle == NULL)
			{
				//fit failed to converge, calc from other particles
				DLorentzVector locDecayDP4 = dAnalysisUtilities->Calc_FinalStateP4(locReaction, locParticleCombo, locStepIndex, false);
				locDecayDP4.SetE(sqrt(locDecayDP4.Vect().Mag2() + ParticleMass(locInitialPID)*ParticleMass(locInitialPID)));
				locDecayP4.SetPxPyPzE(locDecayDP4.Px(), locDecayDP4.Py(), locDecayDP4.Pz(), locDecayDP4.E());
			}
			else
				locDecayP4.SetPxPyPzE(locInitialParticle->momentum().X(), locInitialParticle->momentum().Y(), locInitialParticle->momentum().Z(), locInitialParticle->energy());
			locTreeFillData->Fill_Array<TLorentzVector>(Build_BranchName(locParticleBranchName, "P4_KinFit"), locDecayP4, locComboIndex);
		}

		if((locStepIndex == 0) || IsDetachedVertex(locInitialPID))
			locTreeFillData->Fill_Array<TLorentzVector>(Build_BranchName(locParticleBranchName, "X4"), locStepTX4, locComboIndex);

		auto locStepVertexInfo = locReactionVertexInfo->Get_StepVertexInfo(locStepIndex);
		auto locParentVertexInfo = locStepVertexInfo->Get_ParentVertexInfo();
		auto locVertexKinFitFlag = ((locKinFitType != d_P4Fit) && (locKinFitType != d_NoFit));
		if(IsDetachedVertex(locInitialPID) && locVertexKinFitFlag && (locParentVertexInfo != nullptr) && locStepVertexInfo->Get_FittableVertexFlag() && locParentVertexInfo->Get_FittableVertexFlag())
		{
			auto locKinFitParticle = locParticleComboStep->Get_InitialKinFitParticle();
			auto locPathLengthSigma = locKinFitParticle->Get_PathLengthUncertainty();
			locTreeFillData->Fill_Array<Float_t>(Build_BranchName(locParticleBranchName, "PathLengthSigma"), locPathLengthSigma, locComboIndex);
		}
	}

	//final state particles
	for(size_t loc_i = 0; loc_i < locParticleComboStep->Get_NumFinalParticles(); ++loc_i)
	{
		Particle_t locPID = locReactionStep->Get_FinalPID(loc_i);
		const DKinematicData* locKinematicData = locParticleComboStep->Get_FinalParticle(loc_i);
		const DKinematicData* locKinematicData_Measured = locParticleComboStep->Get_FinalParticle_Measured(loc_i);

		//decaying particle
		if(DAnalysis::Get_DecayStepIndex(locReaction, locStepIndex, loc_i) >= 0)
			continue;

		//get the branch name
		ostringstream locPositionStream;
		locPositionStream << locStepIndex << "_" << loc_i;
		TObjString* locObjString = (TObjString*)locPositionToNameMap->GetValue(locPositionStream.str().c_str());
		string locParticleBranchName = (const char*)(locObjString->GetString());

		//missing particle
		if(locReactionStep->Get_MissingParticleIndex() == int(loc_i))
		{
			if((locKinFitType == d_P4Fit) || (locKinFitType == d_P4AndVertexFit) || (locKinFitType == d_P4AndSpacetimeFit))
			{
				TLorentzVector locMissingP4;
				if(locKinematicData == NULL)
				{
					//fit failed to converge, calc from other particles
					DLorentzVector locMissingDP4 = dAnalysisUtilities->Calc_MissingP4(locReaction, locParticleCombo, false);
					locMissingDP4.SetE(sqrt(locMissingDP4.Vect().Mag2() + ParticleMass(locPID)*ParticleMass(locPID)));
					locMissingP4.SetPxPyPzE(locMissingDP4.Px(), locMissingDP4.Py(), locMissingDP4.Pz(), locMissingDP4.E());
				}
				else
					locMissingP4.SetPxPyPzE(locKinematicData->momentum().X(), locKinematicData->momentum().Y(), locKinematicData->momentum().Z(), locKinematicData->energy());

				locTreeFillData->Fill_Array<TLorentzVector>(Build_BranchName(locParticleBranchName, "P4_KinFit"), locMissingP4, locComboIndex);
			}
			continue;
		}

		//fill the data
		if(ParticleCharge(locPID) == 0)
		{
			const DNeutralParticleHypothesis* locNeutralHypo = dynamic_cast<const DNeutralParticleHypothesis*>(locKinematicData);
			const DNeutralParticleHypothesis* locMeasuredNeutralHypo = dynamic_cast<const DNeutralParticleHypothesis*>(locKinematicData_Measured);

			//get array index
			const DNeutralShower* locNeutralShower = locNeutralHypo->Get_NeutralShower();
			pair<oid_t, Particle_t> locNeutralPair(locNeutralShower->id, locNeutralHypo->PID());
			size_t locNeutralIndex = locObjectToArrayIndexMap.find(locNeutralPair)->second;

			Fill_ComboNeutralData(locTreeFillData, locComboIndex, locParticleBranchName, locMeasuredNeutralHypo, locNeutralHypo, locNeutralIndex, locKinFitType);
		}
		else
		{
			const DChargedTrackHypothesis* locChargedHypo = dynamic_cast<const DChargedTrackHypothesis*>(locKinematicData);
			const DChargedTrackHypothesis* locMeasuredChargedHypo = dynamic_cast<const DChargedTrackHypothesis*>(locKinematicData_Measured);

			//get array index
			const DTrackTimeBased* locTrackTimeBased = locChargedHypo->Get_TrackTimeBased();
			pair<oid_t, Particle_t> locTrackPair(locTrackTimeBased->id, locChargedHypo->PID());
			size_t locChargedIndex = locObjectToArrayIndexMap.find(locTrackPair)->second;

			Fill_ComboChargedData(locTreeFillData, locComboIndex, locParticleBranchName, locMeasuredChargedHypo, locChargedHypo, locChargedIndex, locKinFitType);
		}
	}
}

void DEventWriterROOT::Fill_ComboBeamData(DTreeFillData* locTreeFillData, unsigned int locComboIndex, const DBeamPhoton* locBeamPhoton, size_t locBeamIndex, DKinFitType locKinFitType) const
{
	string locParticleBranchName = "ComboBeam";

	//IDENTIFIER
	locTreeFillData->Fill_Array<Int_t>(Build_BranchName(locParticleBranchName, "BeamIndex"), locBeamIndex, locComboIndex);

	//KINEMATICS: KINFIT
	if(locKinFitType != d_NoFit)
	{
		if(locKinFitType != d_P4Fit)
		{
			DVector3 locPosition = locBeamPhoton->position();
			TLorentzVector locX4_KinFit(locPosition.X(), locPosition.Y(), locPosition.Z(), locBeamPhoton->time());
			locTreeFillData->Fill_Array<TLorentzVector>(Build_BranchName(locParticleBranchName, "X4_KinFit"), locX4_KinFit, locComboIndex);
		}

		//if charged, bends in b-field, update p4 when vertex changes
		if(((locKinFitType != d_VertexFit) && (locKinFitType != d_SpacetimeFit)) || (ParticleCharge(locBeamPhoton->PID()) != 0))
		{
			DLorentzVector locDP4 = locBeamPhoton->lorentzMomentum();
			TLorentzVector locP4_KinFit(locDP4.Px(), locDP4.Py(), locDP4.Pz(), locDP4.E());
			locTreeFillData->Fill_Array<TLorentzVector>(Build_BranchName(locParticleBranchName, "P4_KinFit"), locP4_KinFit, locComboIndex);
		}
	}
}

void DEventWriterROOT::Fill_ComboChargedData(DTreeFillData* locTreeFillData, unsigned int locComboIndex, string locParticleBranchName, const DChargedTrackHypothesis* locMeasuredChargedHypo, const DChargedTrackHypothesis* locChargedHypo, size_t locChargedIndex, DKinFitType locKinFitType) const
{
	//IDENTIFIER
	locTreeFillData->Fill_Array<Int_t>(Build_BranchName(locParticleBranchName, "ChargedIndex"), locChargedIndex, locComboIndex);

	//MEASURED PID
	locTreeFillData->Fill_Array<Float_t>(Build_BranchName(locParticleBranchName, "Beta_Timing_Measured"), locMeasuredChargedHypo->measuredBeta(), locComboIndex);
	locTreeFillData->Fill_Array<Float_t>(Build_BranchName(locParticleBranchName, "ChiSq_Timing_Measured"), locMeasuredChargedHypo->Get_ChiSq_Timing(), locComboIndex);

	//KINFIT PID
	if((locKinFitType != d_NoFit) && (locKinFitType != d_SpacetimeFit) && (locKinFitType != d_P4AndSpacetimeFit))
	{
		locTreeFillData->Fill_Array<Float_t>(Build_BranchName(locParticleBranchName, "Beta_Timing_KinFit"), locChargedHypo->measuredBeta(), locComboIndex);
		locTreeFillData->Fill_Array<Float_t>(Build_BranchName(locParticleBranchName, "ChiSq_Timing_KinFit"), locChargedHypo->Get_ChiSq_Timing(), locComboIndex);
	}

	//KINFIT
	if(locKinFitType != d_NoFit)
	{
		//KINEMATICS
		if(locKinFitType != d_P4Fit)
		{
			DVector3 locPosition = locChargedHypo->position();
			TLorentzVector locX4_KinFit(locPosition.X(), locPosition.Y(), locPosition.Z(), locChargedHypo->time());
			locTreeFillData->Fill_Array<TLorentzVector>(Build_BranchName(locParticleBranchName, "X4_KinFit"), locX4_KinFit, locComboIndex);
		}

		//update even if vertex-only fit, because charged momentum propagated through b-field
		DLorentzVector locDP4 = locChargedHypo->lorentzMomentum();
		TLorentzVector locP4_KinFit(locDP4.Px(), locDP4.Py(), locDP4.Pz(), locDP4.E());
		locTreeFillData->Fill_Array<TLorentzVector>(Build_BranchName(locParticleBranchName, "P4_KinFit"), locP4_KinFit, locComboIndex);
	}
}

void DEventWriterROOT::Fill_ComboNeutralData(DTreeFillData* locTreeFillData, unsigned int locComboIndex, string locParticleBranchName, const DNeutralParticleHypothesis* locMeasuredNeutralHypo, const DNeutralParticleHypothesis* locNeutralHypo, size_t locNeutralIndex, DKinFitType locKinFitType) const
{
	//IDENTIFIER
	locTreeFillData->Fill_Array<Int_t>(Build_BranchName(locParticleBranchName, "NeutralIndex"), locNeutralIndex, locComboIndex);

	//KINEMATICS: MEASURED
	DVector3 locPosition = locMeasuredNeutralHypo->position();
	TLorentzVector locX4_Measured(locPosition.X(), locPosition.Y(), locPosition.Z(), locMeasuredNeutralHypo->time());
	locTreeFillData->Fill_Array<TLorentzVector>(Build_BranchName(locParticleBranchName, "X4_Measured"), locX4_Measured, locComboIndex);

	DLorentzVector locDP4 = locMeasuredNeutralHypo->lorentzMomentum();
	TLorentzVector locP4_Measured(locDP4.Px(), locDP4.Py(), locDP4.Pz(), locDP4.E());
	locTreeFillData->Fill_Array<TLorentzVector>(Build_BranchName(locParticleBranchName, "P4_Measured"), locP4_Measured, locComboIndex);

	//MEASURED PID INFO
	locTreeFillData->Fill_Array<Float_t>(Build_BranchName(locParticleBranchName, "Beta_Timing_Measured"), locMeasuredNeutralHypo->measuredBeta(), locComboIndex);
	if(locParticleBranchName.substr(0, 6) == "Photon")
		locTreeFillData->Fill_Array<Float_t>(Build_BranchName(locParticleBranchName, "ChiSq_Timing_Measured"), locMeasuredNeutralHypo->Get_ChiSq(), locComboIndex);

	//KINFIT PID INFO
	if((locKinFitType != d_NoFit) && (locKinFitType != d_SpacetimeFit) && (locKinFitType != d_P4AndSpacetimeFit))
	{
		locTreeFillData->Fill_Array<Float_t>(Build_BranchName(locParticleBranchName, "Beta_Timing_KinFit"), locNeutralHypo->measuredBeta(), locComboIndex);
		if(locParticleBranchName.substr(0, 6) == "Photon")
			locTreeFillData->Fill_Array<Float_t>(Build_BranchName(locParticleBranchName, "ChiSq_Timing_KinFit"), locNeutralHypo->Get_ChiSq(), locComboIndex);
	}

	//KINFIT
	if(locKinFitType != d_NoFit)
	{
		//KINEMATICS
		if(locKinFitType != d_P4Fit)
		{
			DVector3 locPosition = locNeutralHypo->position();
			TLorentzVector locX4_KinFit(locPosition.X(), locPosition.Y(), locPosition.Z(), locNeutralHypo->time());
			locTreeFillData->Fill_Array<TLorentzVector>(Build_BranchName(locParticleBranchName, "X4_KinFit"), locX4_KinFit, locComboIndex);
		}

		//update even if vertex-only fit, because neutral momentum defined by vertex
		DLorentzVector locDP4 = locNeutralHypo->lorentzMomentum();
		TLorentzVector locP4_KinFit(locDP4.Px(), locDP4.Py(), locDP4.Pz(), locDP4.E());
		locTreeFillData->Fill_Array<TLorentzVector>(Build_BranchName(locParticleBranchName, "P4_KinFit"), locP4_KinFit, locComboIndex);
	}
}

void DEventWriterROOT::Fill_KinFitData(DTreeFillData* locTreeFillData, const std::shared_ptr<const JEvent>& locEventLoop, const DReaction* locReaction, const DMCReaction* locMCReaction, const vector<const DMCThrown*>& locMCThrowns,
		const DMCThrownMatching* locMCThrownMatching, const DDetectorMatches* locDetectorMatches,
		const vector<const DBeamPhoton*>& locBeamPhotons, const vector<const DChargedTrackHypothesis*>& locChargedHypos,
		const vector<const DNeutralParticleHypothesis*>& locNeutralHypos, const deque<const DParticleCombo*>& locParticleCombos) const
{
	if(!STORE_PULL_INFO && !STORE_ERROR_MATRIX_INFO)
		return;

	if(STORE_ERROR_MATRIX_INFO){
	  locTreeFillData->Fill_Single<Int_t>("numEntries_ParticleErrM", nEntriesParticleCov);
	  locTreeFillData->Fill_Single<Int_t>("numEntries_ShowerErrM", nEntriesShowerCov);
	}
    bool writeOutPulls = getPullFlag( locReaction );
        
    //Retreive and fill pull-information:
    //######################################################################################################################
    DKinFitType kfitType = locReaction->Get_KinFitType();
    
    //Now fill the pulls for all particles:
    int nParticleCombos = locParticleCombos.size();
    int nMeasuredFinalParticles;
    double currentCharge;
    Particle_t currentPID;
        
    //----------------------------------------------------------------------------
    for(int iPC=0;iPC<nParticleCombos;iPC++){
        Get_PullsFromFit(locParticleCombos.at(iPC));
        
        vector<string> pidVector = getPIDinReaction(locReaction);
        map<Particle_t,int> assignMap = getNameForParticle(locReaction,pidVector);
        
        
        //Look at beam photons:
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        const DParticleComboStep* locParticleComboStep = locParticleCombos[iPC]->Get_ParticleComboStep(0);
        const DKinematicData* beamPhoton = locParticleComboStep->Get_InitialParticle_Measured();
        
        if(kfitType == d_P4Fit || kfitType == d_P4AndVertexFit){
		  if(writeOutPulls){	
             map<DKinFitPullType, double> someBeamMap = getPulls(beamPhoton,NULL,kfitType);
             fillTreePullBranches(locTreeFillData,"ComboBeam",kfitType,someBeamMap,iPC,true);
             someBeamMap.clear();
	      }
	    }
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        
        //Look at the decay products:
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        vector<const DKinematicData*> finalParticlesMeasured = locParticleCombos.at(iPC)->Get_FinalParticles_Measured(locReaction, d_AllCharges);
        vector<const DKinematicData*> finalParticles = locParticleCombos.at(iPC)->Get_FinalParticles(locReaction); //, d_AllCharges);
        
        vector<const JObject*> finalParticleObjects = locParticleCombos.at(iPC)->Get_FinalParticle_SourceObjects(d_AllCharges);
        nMeasuredFinalParticles = finalParticlesMeasured.size();
        
        //----------------------------------------------------------------------------
        for(int iFP=0;iFP<nMeasuredFinalParticles;iFP++){
            const DKinematicData* part = finalParticlesMeasured.at(iFP);
            const DKinematicData* partFit = finalParticles.at(iFP);
            const  DNeutralShower* sh = dynamic_cast<const DNeutralShower*>(finalParticleObjects.at(iFP));
            bool isNeutral = false;
            
            map<DKinFitPullType, double>  someMap;
            if(writeOutPulls) someMap = getPulls(part,sh,kfitType);
            //-----------------------------------------------------
            if(sh == NULL){
				currentCharge = part->charge();
                currentPID = part->PID();
			}else{
				currentCharge = 0.0;
                currentPID = Gamma;
		    }
            
            if(currentCharge == 0) isNeutral = true;
            //-----------------------------------------------------
            
            string branchName = assignName(currentPID,assignMap);
            assignMap[currentPID]--;
            //-----------------------------------------------------
            if(branchName != "nada"){
                if(STORE_PULL_INFO){
                    fillTreePullBranches(locTreeFillData,branchName,kfitType,someMap,iPC,isNeutral);
                    fillTreeTrackPullBranches(locTreeFillData,branchName,kfitType,someMap,iPC,isNeutral,part,partFit);
                }
                if(STORE_ERROR_MATRIX_INFO)
                	fillTreeErrMBranches(locTreeFillData,branchName,kfitType,part,sh,isNeutral);
            }
            //-----------------------------------------------------
            if(STORE_PULL_INFO) someMap.clear();
		  }
        //----------------------------------------------------------------------------
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        
        myPullsMap.clear();
        assignMap.clear();
        abundanceMap.clear();
        
    }

}


//I guess this is the most clumsiest way of assigning a few names to a tree-branch, but it is doing what it is supposed to do...
//*************************************************************************************************************************************
vector<string> DEventWriterROOT::getPIDinReaction(const DReaction* locReaction)const
{
    vector <string> myVector;
    vector<Particle_t> particle = locReaction->Get_FinalPIDs(-1,false,false,d_AllCharges,true);
    int nParticleIDs = particle.size();
    //----------------------------------------------------
    for(int k=0;k<nParticleIDs;k++){
        myVector.push_back(EnumString(particle.at(k)));
    }
    //----------------------------------------------------
    return myVector;
}

map<Particle_t, int> DEventWriterROOT::getNameForParticle(const DReaction* someReaction, vector<string> someVector)const
{
    map <Particle_t,int> myMap;
    vector<Particle_t> particle = someReaction->Get_FinalPIDs(-1,false,false,d_AllCharges,true);
    int nParticleIDs = particle.size();
    int currentCounter = 0;
    int nEl = someVector.size();
    //----------------------------------------------------
    for(int k=0;k<nParticleIDs;k++){
        currentCounter = 0;
        
        //----------------------------------------------------
        for(int j=0;j<nEl;j++){
            if(EnumString(particle.at(k)) == someVector.at(j)){
                currentCounter++;
                someVector.at(j) = "";
            }
        }
        //----------------------------------------------------
        if(currentCounter > 0){
            abundanceMap[particle.at(k)] = currentCounter;
            myMap[particle.at(k)] = currentCounter;
        }
    }
    //----------------------------------------------------
    return myMap;
}

string DEventWriterROOT::assignName(Particle_t someParticle, map<Particle_t,int> someMap)const
{
    string myName = "nada";
    int abundance = abundanceMap.find(someParticle)->second;
    if(abundance == 1) {
        myName = EnumString(someParticle);
    } else if(abundance > 1) {
        int index = abundance - someMap.find(someParticle)->second +1;
        string addName;
        ostringstream convert;
        convert << index;
        addName = convert.str();
        if(strcmp(EnumString(someParticle),"Gamma")==0) {
            myName = "Photon" + addName;
        } else {
        	myName = EnumString(someParticle) + addName;
		}
    }
    
    return myName;
}

void DEventWriterROOT::Get_PullsFromFit(const DParticleCombo* particleCombos) const
{
    const DKinFitResults* fitResults = particleCombos->Get_KinFitResults();
    if(fitResults != NULL){
        fitResults->Get_Pulls(myPullsMap);
    }
}

map<DKinFitPullType, double> DEventWriterROOT::getPulls(const JObject* particle, const JObject* shower, DKinFitType yourFitType) const{
	map<DKinFitPullType, double> myMap;

	if(particle != NULL || shower != NULL) {
	
		   if(yourFitType == d_P4Fit || yourFitType == d_VertexFit) {
		   myMap = myPullsMap.find(particle)->second;
	   }else if(yourFitType == d_P4AndVertexFit ){ //Option noFit is not considered
	       if(shower == NULL) {
			   myMap = myPullsMap.find(particle)->second;
		   } else myMap = myPullsMap.find(shower)->second; 	
	   }
    }
	
	return myMap;
}


//Get the pull-features:
void DEventWriterROOT::setTreePullBranches(DTreeBranchRegister& locBranchRegister,string yourBranchName,DKinFitType yourFitType, int yourNCombos, bool isNeutral) const
{
    string locArraySizeString = "NumCombos";
    
    if(yourFitType == d_P4Fit){
        //Regular Pulls:
        locBranchRegister.Register_FundamentalArray<Double_t>(Build_BranchName(yourBranchName,"Px_Pull"),locArraySizeString, yourNCombos);
        locBranchRegister.Register_FundamentalArray<Double_t>(Build_BranchName(yourBranchName,"Py_Pull"),locArraySizeString, yourNCombos);
        locBranchRegister.Register_FundamentalArray<Double_t>(Build_BranchName(yourBranchName,"Pz_Pull"),locArraySizeString, yourNCombos);
        
        //Pulls used for tracking:
        locBranchRegister.Register_FundamentalArray<Double_t>(Build_BranchName(yourBranchName,"QPt_Pull"),locArraySizeString, yourNCombos);
        locBranchRegister.Register_FundamentalArray<Double_t>(Build_BranchName(yourBranchName,"Phi_Pull"),locArraySizeString, yourNCombos);
        locBranchRegister.Register_FundamentalArray<Double_t>(Build_BranchName(yourBranchName,"tanLambda_Pull"),locArraySizeString, yourNCombos);
	locBranchRegister.Register_FundamentalArray<Double_t>(Build_BranchName(yourBranchName,"D_Pull"),locArraySizeString, yourNCombos);
      
    }else if(yourFitType == d_P4AndVertexFit){
        //------------------------------------------------------------
        if(isNeutral){
            locBranchRegister.Register_FundamentalArray<Double_t>(Build_BranchName(yourBranchName,"E_Pull"),locArraySizeString, yourNCombos);
            locBranchRegister.Register_FundamentalArray<Double_t>(Build_BranchName(yourBranchName,"Xx_Pull"),locArraySizeString, yourNCombos);
            locBranchRegister.Register_FundamentalArray<Double_t>(Build_BranchName(yourBranchName,"Xy_Pull"),locArraySizeString, yourNCombos);
            locBranchRegister.Register_FundamentalArray<Double_t>(Build_BranchName(yourBranchName,"Xz_Pull"),locArraySizeString, yourNCombos);
        }else{
            //Regular Pulls:
            locBranchRegister.Register_FundamentalArray<Double_t>(Build_BranchName(yourBranchName,"Px_Pull"),locArraySizeString, yourNCombos);
            locBranchRegister.Register_FundamentalArray<Double_t>(Build_BranchName(yourBranchName,"Py_Pull"),locArraySizeString, yourNCombos);
            locBranchRegister.Register_FundamentalArray<Double_t>(Build_BranchName(yourBranchName,"Pz_Pull"),locArraySizeString, yourNCombos);
            locBranchRegister.Register_FundamentalArray<Double_t>(Build_BranchName(yourBranchName,"Xx_Pull"),locArraySizeString, yourNCombos);
            locBranchRegister.Register_FundamentalArray<Double_t>(Build_BranchName(yourBranchName,"Xy_Pull"),locArraySizeString, yourNCombos);
            locBranchRegister.Register_FundamentalArray<Double_t>(Build_BranchName(yourBranchName,"Xz_Pull"),locArraySizeString, yourNCombos);
            
            //Pulls used for tracking:
            locBranchRegister.Register_FundamentalArray<Double_t>(Build_BranchName(yourBranchName,"QPt_Pull"),locArraySizeString, yourNCombos);
            locBranchRegister.Register_FundamentalArray<Double_t>(Build_BranchName(yourBranchName,"Phi_Pull"),locArraySizeString, yourNCombos);
            locBranchRegister.Register_FundamentalArray<Double_t>(Build_BranchName(yourBranchName,"tanLambda_Pull"),locArraySizeString, yourNCombos);
	    locBranchRegister.Register_FundamentalArray<Double_t>(Build_BranchName(yourBranchName,"D_Pull"),locArraySizeString, yourNCombos);
        }
        //------------------------------------------------------------
    }else if(yourFitType == d_VertexFit && !isNeutral){
            locBranchRegister.Register_FundamentalArray<Double_t>(Build_BranchName(yourBranchName,"Xx_Pull"),locArraySizeString, yourNCombos);
            locBranchRegister.Register_FundamentalArray<Double_t>(Build_BranchName(yourBranchName,"Xy_Pull"),locArraySizeString, yourNCombos);
            locBranchRegister.Register_FundamentalArray<Double_t>(Build_BranchName(yourBranchName,"Xz_Pull"),locArraySizeString, yourNCombos);
    }
}

//Fill the tree branches:
void DEventWriterROOT::fillTreePullBranches(DTreeFillData* locTreeFillData,string yourBranchName,DKinFitType yourFitType,map<DKinFitPullType, double> yourPullsMap, int yourIndex, bool isNeutral) const
{
    
        if(yourFitType == d_P4Fit){
            locTreeFillData->Fill_Array<Double_t>(Build_BranchName(yourBranchName, "Px_Pull"), yourPullsMap.find(d_PxPull)->second,yourIndex);
            locTreeFillData->Fill_Array<Double_t>(Build_BranchName(yourBranchName, "Py_Pull"), yourPullsMap.find(d_PyPull)->second,yourIndex);
            locTreeFillData->Fill_Array<Double_t>(Build_BranchName(yourBranchName, "Pz_Pull"), yourPullsMap.find(d_PzPull)->second,yourIndex);
    
        }else if(yourFitType == d_P4AndVertexFit){
           //------------------------------------------------------------
           if(isNeutral){
                locTreeFillData->Fill_Array<Double_t>(Build_BranchName(yourBranchName, "E_Pull"), yourPullsMap.find(d_EPull)->second,yourIndex);
                locTreeFillData->Fill_Array<Double_t>(Build_BranchName(yourBranchName, "Xx_Pull"), yourPullsMap.find(d_XxPull)->second,yourIndex);
                locTreeFillData->Fill_Array<Double_t>(Build_BranchName(yourBranchName, "Xy_Pull"), yourPullsMap.find(d_XyPull)->second,yourIndex);
                locTreeFillData->Fill_Array<Double_t>(Build_BranchName(yourBranchName, "Xz_Pull"), yourPullsMap.find(d_XzPull)->second,yourIndex);
           }else{
                locTreeFillData->Fill_Array<Double_t>(Build_BranchName(yourBranchName, "Px_Pull"), yourPullsMap.find(d_PxPull)->second,yourIndex);
                locTreeFillData->Fill_Array<Double_t>(Build_BranchName(yourBranchName, "Py_Pull"), yourPullsMap.find(d_PyPull)->second,yourIndex);
                locTreeFillData->Fill_Array<Double_t>(Build_BranchName(yourBranchName, "Pz_Pull"), yourPullsMap.find(d_PzPull)->second,yourIndex);
                locTreeFillData->Fill_Array<Double_t>(Build_BranchName(yourBranchName, "Xx_Pull"), yourPullsMap.find(d_XxPull)->second,yourIndex);
                locTreeFillData->Fill_Array<Double_t>(Build_BranchName(yourBranchName, "Xy_Pull"), yourPullsMap.find(d_XyPull)->second,yourIndex);
                locTreeFillData->Fill_Array<Double_t>(Build_BranchName(yourBranchName, "Xz_Pull"), yourPullsMap.find(d_XzPull)->second,yourIndex);
           }
           //------------------------------------------------------------
        }else if(yourFitType == d_VertexFit && !isNeutral){
               locTreeFillData->Fill_Array<Double_t>(Build_BranchName(yourBranchName, "Xx_Pull"), yourPullsMap.find(d_XxPull)->second,yourIndex);
               locTreeFillData->Fill_Array<Double_t>(Build_BranchName(yourBranchName, "Xy_Pull"), yourPullsMap.find(d_XyPull)->second,yourIndex);
               locTreeFillData->Fill_Array<Double_t>(Build_BranchName(yourBranchName, "Xz_Pull"), yourPullsMap.find(d_XzPull)->second,yourIndex);
        }
}

//Make sure, the pull information is written out, when available
void DEventWriterROOT::setPullFlag(const DReaction* currentReaction, bool myFlag) const
{
    writePulls[currentReaction] = myFlag;
}

bool DEventWriterROOT::getPullFlag(const DReaction* currentReaction) const
{
    bool outFlag = writePulls.find(currentReaction)->second;
    return outFlag;
}

//Now fill the individual elements of the error matrix to the tree:
void DEventWriterROOT::fillTreeErrMBranches(DTreeFillData* locTreeFillData,string yourBranchName,DKinFitType yourFitType, const DKinematicData* particle, const DNeutralShower* shower, bool isNeutral) const
{
    if(yourFitType == d_P4Fit) {
		fillTreeParticleErrMBranches(locTreeFillData,yourBranchName,particle);
	} else if(yourFitType == d_P4AndVertexFit) {
		if(isNeutral) {
			fillTreeShowerErrMBranches(locTreeFillData,yourBranchName,shower);
		} else {
			fillTreeParticleErrMBranches(locTreeFillData,yourBranchName,particle);
		}
	} else if(yourFitType == d_VertexFit && !isNeutral) {
		fillTreeParticleErrMBranches(locTreeFillData,yourBranchName,particle);
	}     	
}

//Fill the 7x7 error matrix elements:
void DEventWriterROOT::fillTreeParticleErrMBranches(DTreeFillData* locTreeFillData,string yourBranchName, const DKinematicData* particle) const
{
	if(particle != NULL){	
	   const TMatrixTSym<float> errMatrix = *( particle->errorMatrix() );
	
	   int mIndex = 0;
	   float matrixEl = 0.0;
	   for(int row=0;row<7;row++){
		   for(int col=0;col<7;col++){
			    if(col >= row){
					matrixEl = errMatrix[row][col];
					locTreeFillData->Fill_Array<Float_t>(Build_BranchName(yourBranchName, "ErrMatrix"),matrixEl,mIndex);
					mIndex++;
				}
		   }
	   }

    }
}

//Fill the 5x5 error matrix elements:
void DEventWriterROOT::fillTreeShowerErrMBranches(DTreeFillData* locTreeFillData,string yourBranchName, const DNeutralShower* shower) const
{
	if(shower != NULL){
	   const TMatrixTSym <float> errMatrix = *( shower->dCovarianceMatrix );
	   float matrixEl = 0.0;
	   
	   int mIndex = 0;
	   for(int row=0;row<5;row++){
		   for(int col=0;col<5;col++){
			    if(col >= row){
				   matrixEl = errMatrix[row][col];
			       locTreeFillData->Fill_Array<Float_t>(Build_BranchName(yourBranchName, "ErrMatrix"),matrixEl,mIndex);
				   mIndex++;
				}
		   }
	   }

    }
}

//GET THE TRACKING PULLS:

//Get the difference in pull errors for one momentum component:
double DEventWriterROOT::getDiffSquaredErrP_Component(double yourRecComponent,double yourFitComponent,double yourPull) const
{
    double diff = yourRecComponent - yourFitComponent;
    double error_component = 0.0;
    
    if(diff != 0.0 && yourPull != 0.0){
        error_component = diff*diff / (yourPull*yourPull);
    }
    
    return error_component;
}

// Get the difference in pull errors for the position
vector< vector<double> > DEventWriterROOT::getSquaredErrX(const DKinematicData* particle, const DKinematicData* particleFit, map<DKinFitPullType, double> yourPullsMap) const{
    
    double x_pull = yourPullsMap.find(d_XxPull)->second;
    double y_pull = yourPullsMap.find(d_XyPull)->second;
    
    double x_rec = particle->x();
    double x_fit = particleFit->x();
    
    double y_rec = particle->y();
    double y_fit = particleFit->y();
    
    double error_x = getDiffSquaredErrP_Component(x_rec,x_fit,x_pull);
    double error_y = getDiffSquaredErrP_Component(y_rec,y_fit,y_pull);
    
    const TMatrixTSym<float> errMatrix = *( particle->errorMatrix() ); //--> Reconstructed errors!
    double error_x_rec = errMatrix[3][3];
    double error_y_rec = errMatrix[4][4];
    
    double error_x_fit = error_x_rec - error_x;
    double error_y_fit = error_y_rec - error_y;
    
    vector <double> xComponent;
    vector <double> yComponent;
    
    xComponent.push_back(error_x_rec);
    xComponent.push_back(error_x_fit);
    
    yComponent.push_back(error_y_rec);
    yComponent.push_back(error_y_fit);
    
    vector < vector<double> > myErrors;
    myErrors.push_back(xComponent);
    myErrors.push_back(yComponent);
   
    return myErrors;

}

//Get the difference in pull errors for the momentum
vector< vector<double> > DEventWriterROOT::getSquaredErrP(const DKinematicData* particle, const DKinematicData* particleFit, map<DKinFitPullType, double> yourPullsMap) const
{
    
    double px_pull = yourPullsMap.find(d_PxPull)->second;
    double py_pull = yourPullsMap.find(d_PyPull)->second;
    double pz_pull = yourPullsMap.find(d_PzPull)->second;
    
    double px_rec = particle->px();
    double px_fit = particleFit->px();
    
    double py_rec = particle->py();
    double py_fit = particleFit->py();
    
    double pz_rec = particle->pz();
    double pz_fit = particleFit->pz();
    
    double error_x = getDiffSquaredErrP_Component(px_rec,px_fit,px_pull);
    double error_y = getDiffSquaredErrP_Component(py_rec,py_fit,py_pull);
    double error_z = getDiffSquaredErrP_Component(pz_rec,pz_fit,pz_pull);
    
    const TMatrixTSym<float> errMatrix = *( particle->errorMatrix() ); //--> Reconstructed errors!
    double error_x_rec = errMatrix[0][0];
    double error_y_rec = errMatrix[1][1];
    double error_z_rec = errMatrix[2][2];
    
    double error_x_fit = error_x_rec - error_x;
    double error_y_fit = error_y_rec - error_y;
    double error_z_fit = error_z_rec - error_z;
    
    vector <double> xComponent;
    vector <double> yComponent;
    vector <double> zComponent;
    
    xComponent.push_back(error_x_rec);
    xComponent.push_back(error_x_fit);
    
    yComponent.push_back(error_y_rec);
    yComponent.push_back(error_y_fit);
    
    zComponent.push_back(error_z_rec);
    zComponent.push_back(error_z_fit);
    
    vector < vector<double> > myErrors;
    myErrors.push_back(xComponent);
    myErrors.push_back(yComponent);
    myErrors.push_back(zComponent);
   
    return myErrors;
}

//1.) Start with phi:
//Calculate the error for one state (fitted,reconstructed):
double DEventWriterROOT::getPhiError(const DKinematicData* particle, vector< vector<double> > yourErrorMatrix, int isFitted) const
{
    double myPhiError = 0.0;
    double myPx = particle->px();
    double myPy = particle->py();
    
    //Phi = artan(Px/Py) --> caclulate derivatives from this...
    if(myPy != 0.0){
        
        double myDPx = yourErrorMatrix[0][isFitted];
        double myDPy = yourErrorMatrix[1][isFitted];
        
        double arg = myPx / myPy;
        double factor1 = 1.0 / pow( myPy*(1.0 + arg*arg), 2);
        double factor2 = (myDPx + arg*arg*myDPy);
        
        myPhiError = factor1*factor2;
    }
    
    return myPhiError;
}


//Now calculate the pull:
double DEventWriterROOT::getPhiPull(const DKinematicData* particle, const DKinematicData* particleFit, vector< vector<double> > yourErrorMatrix) const
{
    double myPhiPull = -100.0;
    
    double phi_rec = ( particle->momentum() ).Phi();
    double phi_fit = ( particleFit->momentum() ).Phi();
    
    double dPhi_rec = getPhiError(particle,yourErrorMatrix,0);
    double dPhi_fit = getPhiError(particleFit,yourErrorMatrix,1);
    
    double pull_norm = dPhi_rec - dPhi_fit;
    
    if(pull_norm > 0.0){
        myPhiPull = (phi_rec - phi_fit) / sqrt(pull_norm);
    }
    
    return myPhiPull;
}

//2.) Tan(lambda):
//Get the error of lambda:
double DEventWriterROOT::getLambdaError(const DKinematicData* particle, vector< vector<double> > yourErrorMatrix, int isFitted) const
{
    double myTanLambdaError = 0.0;
    //lambda = pi/2 - theta --> dlambda = dtheta...
    
    double myPx = particle->px();
    double myPy = particle->py();
    double myPz = particle->pz();
    
    double myDPx = yourErrorMatrix[0][isFitted];
    double myDPy = yourErrorMatrix[1][isFitted];
    double myDPz = yourErrorMatrix[2][isFitted];
    
    double r2 = myPx*myPx + myPy*myPy + myPz*myPz;
    double lambda = 0.5*M_PI - ( particle->momentum() ).Theta();
    //---------------------------
    if(r2 > 0.0 && cos(lambda) != 0.0){
        double r = sqrt(r2);
        double factor1 = 1.0 / ( pow(r,6) * pow(cos(lambda),4) );
        double sum1 = myPx*myPz;
        double sum2 = myPy*myPz;
        double sum3 = (r - myPz*myPz);
        double factor2 = sum1*sum1*myDPx + sum2*sum2*myDPy + sum3*sum3*myDPz;
        
        myTanLambdaError = factor1*factor2;
    }
    //---------------------------
    
    
    return myTanLambdaError;
}


//Go for the pull itself:
double DEventWriterROOT::getTanLambdaPull(const DKinematicData* particle, const DKinematicData* particleFit, vector< vector<double> > yourErrorMatrix) const
{
    double myTanLambdaPull = -100.0;
    
    //Reconstructed values:
    double lambda_rec = 0.5*M_PI - ( particle->momentum() ).Theta();
    double tanLambda_rec = tan(lambda_rec);
    double dtanLambda_rec = getLambdaError(particle,yourErrorMatrix,0);
    
    //Fitted values:
    double lambda_fit = 0.5*M_PI - ( particleFit->momentum() ).Theta();
    double tanLambda_fit = tan(lambda_fit);
    double dtanLambda_fit = getLambdaError(particleFit,yourErrorMatrix,1);
    
    double pull_norm = dtanLambda_rec-dtanLambda_fit;
    //---------------------------
    if(pull_norm > 0.0){
        myTanLambdaPull = (tanLambda_rec-tanLambda_fit) / sqrt(pull_norm);
    }
    //---------------------------
    
    return myTanLambdaPull;
}

//3.) q/pt:
//Calculate error for 1/pt:
double DEventWriterROOT::getQPTError(const DKinematicData* particle, vector< vector<double> > yourErrorMatrix, int isFitted) const
{
    double myQPTError = 0.0;
    double myPx = particle->px();
    double myPy = particle->py();
    double r2 = myPx*myPx + myPy*myPy;
    
    double myDPx = yourErrorMatrix[0][isFitted];
    double myDPy = yourErrorMatrix[1][isFitted];
    //---------------------------
    if(r2 > 0.0){
        double q = particle->charge();
        double factor1 = q*q / pow(r2,3);
        double factor2 = (myPx*myPx*myDPx + myPy*myPy*myDPy);
        
        myQPTError = factor1*factor2;
    }
    //---------------------------
    
    return myQPTError;
}


//Get the pull:
double DEventWriterROOT::getQPTPull(const DKinematicData* particle, const DKinematicData* particleFit, vector< vector<double> > yourErrorMatrix) const
{
    double myPTPull = -100.0;
    
    double particle_charge = particle->charge();
    
    double myPx_rec = particle->px();
    double myPy_rec = particle->py();
    double r_rec = myPx_rec*myPx_rec + myPy_rec*myPy_rec;
    
    double myPx_fit = particleFit->px();
    double myPy_fit = particleFit->py();
    double r_fit = myPx_fit*myPx_fit + myPy_fit*myPy_fit;
    
    //---------------------------
    if(r_rec > 0.0 && r_fit > 0.0){
        double qpt_rec = particle_charge/sqrt(r_rec);
        double dqpt_rec = getQPTError(particle,yourErrorMatrix,0);
        
        double qpt_fit = particle_charge/sqrt(r_fit);
        double dqpt_fit = getQPTError(particleFit,yourErrorMatrix,1);
        
        double pull_norm = dqpt_rec-dqpt_fit;
        //---------------------------
        if(pull_norm > 0.0){
	  myPTPull = (qpt_rec-qpt_fit)/sqrt(pull_norm);
        }
        //---------------------------
    }
    //---------------------------
    
    
    return myPTPull;
}

//4.) D:
//Get the error of D:
double DEventWriterROOT::getDError(const DKinematicData* particle, vector< vector<double> > yourErrorMatrix, int isFitted) const
{

  double myDsq=particle->position().Perp2()+1e-8;
  double myx=particle->x();
  double myy=particle->y();

  double myVarX=yourErrorMatrix[0][isFitted];
  double myVarY=yourErrorMatrix[1][isFitted];
  double myDError=(myx*myx*myVarX+myy*myy*myVarY)/myDsq;

  return myDError;
}

//Go for the pull itself:
double DEventWriterROOT::getDPull(const DKinematicData* particle, const DKinematicData* particleFit, vector< vector<double> > yourErrorMatrix) const
{
  double myDPull=-100.0;
  
  //Reconstructed values:
  double D_rec=particle->position().Perp();
  double dx_rec=particle->x();
  double dy_rec=particle->y();
  double phi_rec=particle->momentum().Phi();
  double cosphi_rec=cos(phi_rec);
  double sinphi_rec=sin(phi_rec);
  if ((dx_rec>0.0 && sinphi_rec>0.0) || (dy_rec<0.0 && cosphi_rec>0.0)
      || (dy_rec>0.0 && cosphi_rec<0.0) || (dx_rec<0.0 && sinphi_rec<0.0))
    D_rec*=-1.;
  double dD_rec=getDError(particle,yourErrorMatrix,0);
  
  //Fitted values:
  double D_fit=particleFit->position().Perp();
  double dx_fit=particleFit->x();
  double dy_fit=particleFit->y();
  double phi_fit=particleFit->momentum().Phi();
  double cosphi_fit=cos(phi_fit);
  double sinphi_fit=sin(phi_fit);
  if ((dx_fit>0.0 && sinphi_fit>0.0) || (dy_fit<0.0 && cosphi_fit>0.0)
      || (dy_fit>0.0 && cosphi_fit<0.0) || (dx_fit<0.0 && sinphi_fit<0.0))
    D_fit*=-1.;
  double dD_fit=getDError(particle,yourErrorMatrix,1);

  double pull_norm = dD_rec-dD_fit;
  //---------------------------
  if(pull_norm > 0.0){
    myDPull = (D_rec-D_fit) / sqrt(pull_norm);
  }
  //---------------------------
    
  return myDPull;
}

//Create a vector containing all pulls:
vector<double> DEventWriterROOT::collectTrackingPulls(const DKinematicData* particle, const DKinematicData* particleFit, map<DKinFitPullType, double> yourPullsMap) const
{
    //1.) Get the pre-and post-fit errors:
    vector< vector<double> > myErrors = getSquaredErrP(particle,particleFit,yourPullsMap);
    vector< vector<double> > myXErrors = getSquaredErrX(particle,particleFit,yourPullsMap);
    
    //2.) Calculate the pulls:
    //2.1.) q/pt:
    double qpt_pull = getQPTPull(particle,particleFit,myErrors);
    //2.2.) phi:
    double phi_pull = getPhiPull(particle,particleFit,myErrors);
    //2.3.) tan(lambda):
    double tanLambda_pull = getTanLambdaPull(particle,particleFit,myErrors);
    //2.4.) D
    double D_pull = getDPull(particle,particleFit,myXErrors);
    
    //3.) Store everything into a vector:
    vector<double> myTrackingPulls;
    myTrackingPulls.push_back(qpt_pull);
    myTrackingPulls.push_back(phi_pull);
    myTrackingPulls.push_back(tanLambda_pull);
    myTrackingPulls.push_back(D_pull);
    
    return myTrackingPulls;
}

//Fill the tracking pulls:
void DEventWriterROOT::fillTreeTrackPullBranches(DTreeFillData* locTreeFillData,string yourBranchName,DKinFitType yourFitType,map<DKinFitPullType, double> yourPullsMap, int yourIndex, bool isNeutral, const DKinematicData* particle, const DKinematicData* particleFit) const
{
    if( !isNeutral && (yourFitType == d_P4Fit || yourFitType == d_P4AndVertexFit) ){//Get the tracking pulls for charged tracks only...
        //1.) Get the vector containing the tracking pulls:
        vector<double> myTrackingPulls = collectTrackingPulls(particle,particleFit,yourPullsMap);
        
        //2.) Fill the pulls to the tree:
        locTreeFillData->Fill_Array<Double_t>(Build_BranchName(yourBranchName, "QPt_Pull"),myTrackingPulls[0],yourIndex);
        locTreeFillData->Fill_Array<Double_t>(Build_BranchName(yourBranchName, "Phi_Pull"),myTrackingPulls[1],yourIndex);
        locTreeFillData->Fill_Array<Double_t>(Build_BranchName(yourBranchName, "tanLambda_Pull"),myTrackingPulls[2],yourIndex);
	locTreeFillData->Fill_Array<Double_t>(Build_BranchName(yourBranchName, "D_Pull"),myTrackingPulls[3],yourIndex);
    }
}
