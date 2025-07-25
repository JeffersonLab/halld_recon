#include "ANALYSIS/DHistogramActions.h"

void DHistogramAction_PID::Initialize(const std::shared_ptr<const JEvent>& locEvent)
{
	string locHistName, locHistTitle;
	string locParticleName, locParticleName2, locParticleROOTName, locParticleROOTName2;
	Particle_t locPID, locPID2;

	Run_Update(locEvent);

	//auto locDesiredPIDs = Get_Reaction()->Get_FinalPIDs(-1, false, false, d_AllCharges, false);
	auto locDesiredPIDs = Get_Reaction()->Get_FinalPIDs(-1, false, true, d_AllCharges, false);

	vector<const DMCThrown*> locMCThrowns;
	locEvent->Get(locMCThrowns);

	//CREATE THE HISTOGRAMS
	//Since we are creating histograms, the contents of gDirectory will be modified: must use JANA-wide ROOT lock
	DEvent::GetLockService(locEvent)->RootWriteLock(); //ACQUIRE ROOT LOCK!!
	{
		CreateAndChangeTo_ActionDirectory();
		for(size_t loc_i = 0; loc_i < locDesiredPIDs.size(); ++loc_i)
		{
			locPID = locDesiredPIDs[loc_i];
			locParticleName = ParticleType(locPID);
			locParticleROOTName = ParticleName_ROOT(locPID);
			CreateAndChangeTo_Directory(locParticleName, locParticleName);

			if(ParticleCharge(locPID) == 0 && Is_FinalStateParticle(locPID))
			{
				//BCAL
				CreateAndChangeTo_Directory("BCAL", "BCAL");
				locHistName = "Beta";
				locHistTitle =  string("BCAL ") + locParticleROOTName + string(" Candidates;#beta");
				dHistMap_Beta[locPID][SYS_BCAL] = GetOrCreate_Histogram<TH1I>(locHistName, locHistTitle, dNumBetaBins, dMinBeta, dMaxBeta);
				locHistName = "Shower_Energy";
				locHistTitle =  string("BCAL ") + locParticleROOTName + string(" Candidates;Shower Energy (GeV)");
				dHistMap_CalE[locPID][SYS_BCAL] = GetOrCreate_Histogram<TH1I>(locHistName, locHistTitle, 200, 0, 1.);
				gDirectory->cd("..");

				//FCAL
				CreateAndChangeTo_Directory("FCAL", "FCAL");
				locHistName = "Beta";
				locHistTitle =  string("FCAL ") + locParticleROOTName + string(" Candidates;#beta");
				dHistMap_Beta[locPID][SYS_FCAL] = GetOrCreate_Histogram<TH1I>(locHistName, locHistTitle, dNumBetaBins, dMinBeta, dMaxBeta);
				locHistName = "Shower_Energy";
				locHistTitle =  string("FCAL ") + locParticleROOTName + string(" Candidates;Shower Energy (GeV)");
				dHistMap_CalE[locPID][SYS_FCAL] = GetOrCreate_Histogram<TH1I>(locHistName, locHistTitle, 200, 0, 1.);
				gDirectory->cd("..");

				//ECAL
				CreateAndChangeTo_Directory("ECAL", "ECAL");
				locHistName = "Beta";
				locHistTitle =  string("ECAL ") + locParticleROOTName + string(" Candidates;#beta");
				dHistMap_Beta[locPID][SYS_ECAL] = GetOrCreate_Histogram<TH1I>(locHistName, locHistTitle, dNumBetaBins, dMinBeta, dMaxBeta);
				locHistName = "Shower_Energy";
				locHistTitle =  string("ECAL ") + locParticleROOTName + string(" Candidates;Shower Energy (GeV)");
				dHistMap_CalE[locPID][SYS_ECAL] = GetOrCreate_Histogram<TH1I>(locHistName, locHistTitle, 200, 0, 1.);
				gDirectory->cd("..");

				//CCAL
				CreateAndChangeTo_Directory("CCAL", "CCAL");
				locHistName = "Beta";
				locHistTitle =  string("CCAL ") + locParticleROOTName + string(" Candidates;#beta");
				dHistMap_Beta[locPID][SYS_CCAL] = GetOrCreate_Histogram<TH1I>(locHistName, locHistTitle, dNumBetaBins, dMinBeta, dMaxBeta);
				locHistName = "Shower_Energy";
				locHistTitle =  string("CCAL ") + locParticleROOTName + string(" Candidates;Shower Energy (GeV)");
				dHistMap_CalE[locPID][SYS_CCAL] = GetOrCreate_Histogram<TH1I>(locHistName, locHistTitle, 200, 0, 1.);
				gDirectory->cd("..");
			}

			//q = 0
			if(locPID == Gamma)
			{
				//BCAL
				CreateAndChangeTo_Directory("BCAL", "BCAL");

				locHistName = "BetaVsP";
				locHistTitle =  string("BCAL ") + locParticleROOTName + string(" Candidates;Shower Energy (GeV);#beta");
				dHistMap_BetaVsP[locPID][SYS_BCAL] = GetOrCreate_Histogram<TH2I>(locHistName, locHistTitle, dNum2DPBins, dMinP, dMaxBCALP, dNum2DBetaBins, dMinBeta, dMaxBeta);

				locHistName = "DeltaTVsShowerE_Photon";
				locHistTitle = string("BCAL ") + locParticleROOTName + string(" Candidates;Shower Energy (GeV);#Deltat_{BCAL - RF}");
				dHistMap_DeltaTVsP[locPID][SYS_BCAL] = GetOrCreate_Histogram<TH2I>(locHistName, locHistTitle, dNum2DPBins, dMinP, dMaxP, dNum2DDeltaTBins, dMinDeltaT, dMaxDeltaT);

				locHistName = "TimePullVsShowerE_Photon";
				locHistTitle = string("BCAL ") + locParticleROOTName + string(";Shower Energy (GeV);#Deltat/#sigma_{#Deltat}");
				dHistMap_TimePullVsP[Gamma][SYS_BCAL] = GetOrCreate_Histogram<TH2I>(locHistName, locHistTitle, dNum2DPBins, dMinP, dMaxP, dNum2DPullBins, dMinPull, dMaxPull);

				locHistName = "TimeFOMVsShowerE_Photon";
				locHistTitle = string("BCAL ") + locParticleROOTName + string(";Shower Energy (GeV);Timing PID Confidence Level");
				dHistMap_TimeFOMVsP[Gamma][SYS_BCAL] = GetOrCreate_Histogram<TH2I>(locHistName, locHistTitle, dNum2DPBins, dMinP, dMaxP, dNum2DFOMBins, 0.0, 1.0);


				gDirectory->cd("..");

				//FCAL
				CreateAndChangeTo_Directory("FCAL", "FCAL");

				locHistName = "BetaVsP";
				locHistTitle =  string("FCAL ") + locParticleROOTName + string(" Candidates;Shower Energy (GeV);#beta");
				dHistMap_BetaVsP[locPID][SYS_FCAL] = GetOrCreate_Histogram<TH2I>(locHistName, locHistTitle, dNum2DPBins, dMinP, dMaxP, dNum2DBetaBins, dMinBeta, dMaxBeta);

				locHistName = "DeltaTVsShowerE_Photon";
				locHistTitle = string("FCAL ") + locParticleROOTName + string(";p (GeV/c);#Deltat_{FCAL - RF}");
				dHistMap_DeltaTVsP[locPID][SYS_FCAL] = GetOrCreate_Histogram<TH2I>(locHistName, locHistTitle, dNum2DPBins, dMinP, dMaxP, dNum2DDeltaTBins, dMinDeltaT, dMaxDeltaT);

				locHistName = "TimePullVsShowerE_Photon";
				locHistTitle = string("FCAL ") + locParticleROOTName + string(";Shower Energy (GeV);#Deltat/#sigma_{#Deltat}");
				dHistMap_TimePullVsP[Gamma][SYS_FCAL] = GetOrCreate_Histogram<TH2I>(locHistName, locHistTitle, dNum2DPBins, dMinP, dMaxP, dNum2DPullBins, dMinPull, dMaxPull);

				locHistName = "TimeFOMVsShowerE_Photon";
				locHistTitle = string("FCAL ") + locParticleROOTName + string(";Shower Energy (GeV);Timing PID Confidence Level");
				dHistMap_TimeFOMVsP[Gamma][SYS_FCAL] = GetOrCreate_Histogram<TH2I>(locHistName, locHistTitle, dNum2DPBins, dMinP, dMaxP, dNum2DFOMBins, 0.0, 1.0);

				gDirectory->cd("..");

				//ECAL
				CreateAndChangeTo_Directory("ECAL", "ECAL");

				locHistName = "BetaVsP";
				locHistTitle =  string("ECAL ") + locParticleROOTName + string(" Candidates;Shower Energy (GeV);#beta");
				dHistMap_BetaVsP[locPID][SYS_ECAL] = GetOrCreate_Histogram<TH2I>(locHistName, locHistTitle, dNum2DPBins, dMinP, dMaxP, dNum2DBetaBins, dMinBeta, dMaxBeta);

				locHistName = "DeltaTVsShowerE_Photon";
				locHistTitle = string("ECAL ") + locParticleROOTName + string(";p (GeV/c);#Deltat_{ECAL - RF}");
				dHistMap_DeltaTVsP[locPID][SYS_ECAL] = GetOrCreate_Histogram<TH2I>(locHistName, locHistTitle, dNum2DPBins, dMinP, dMaxP, dNum2DDeltaTBins, dMinDeltaT, dMaxDeltaT);

				locHistName = "TimePullVsShowerE_Photon";
				locHistTitle = string("ECAL ") + locParticleROOTName + string(";Shower Energy (GeV);#Deltat/#sigma_{#Deltat}");
				dHistMap_TimePullVsP[Gamma][SYS_ECAL] = GetOrCreate_Histogram<TH2I>(locHistName, locHistTitle, dNum2DPBins, dMinP, dMaxP, dNum2DPullBins, dMinPull, dMaxPull);

				locHistName = "TimeFOMVsShowerE_Photon";
				locHistTitle = string("ECAL ") + locParticleROOTName + string(";Shower Energy (GeV);Timing PID Confidence Level");
				dHistMap_TimeFOMVsP[Gamma][SYS_ECAL] = GetOrCreate_Histogram<TH2I>(locHistName, locHistTitle, dNum2DPBins, dMinP, dMaxP, dNum2DFOMBins, 0.0, 1.0);

				gDirectory->cd("..");

				//CCAL
				CreateAndChangeTo_Directory("CCAL", "CCAL");

				locHistName = "BetaVsP";
				locHistTitle =  string("CCAL ") + locParticleROOTName + string(" Candidates;Shower Energy (GeV);#beta");
				dHistMap_BetaVsP[locPID][SYS_CCAL] = GetOrCreate_Histogram<TH2I>(locHistName, locHistTitle, dNum2DPBins, dMinP, dMaxP, dNum2DBetaBins, dMinBeta, dMaxBeta);

				locHistName = "DeltaTVsShowerE_Photon";
				locHistTitle = string("CCAL ") + locParticleROOTName + string(";p (GeV/c);#Deltat_{CCAL - RF}");
				dHistMap_DeltaTVsP[locPID][SYS_CCAL] = GetOrCreate_Histogram<TH2I>(locHistName, locHistTitle, dNum2DPBins, dMinP, dMaxP, dNum2DDeltaTBins, dMinDeltaT, dMaxDeltaT);

				locHistName = "TimePullVsShowerE_Photon";
				locHistTitle = string("CCAL ") + locParticleROOTName + string(";Shower Energy (GeV);#Deltat/#sigma_{#Deltat}");
				dHistMap_TimePullVsP[Gamma][SYS_CCAL] = GetOrCreate_Histogram<TH2I>(locHistName, locHistTitle, dNum2DPBins, dMinP, dMaxP, dNum2DPullBins, dMinPull, dMaxPull);

				locHistName = "TimeFOMVsShowerE_Photon";
				locHistTitle = string("CCAL ") + locParticleROOTName + string(";Shower Energy (GeV);Timing PID Confidence Level");
				dHistMap_TimeFOMVsP[Gamma][SYS_CCAL] = GetOrCreate_Histogram<TH2I>(locHistName, locHistTitle, dNum2DPBins, dMinP, dMaxP, dNum2DFOMBins, 0.0, 1.0);

				gDirectory->cd("..");
			}
			else if(ParticleCharge(locPID) != 0)
			{
				//SC
				CreateAndChangeTo_Directory("SC", "SC");

				locHistName = "dEdXVsP";
				locHistTitle = locParticleROOTName + string(" Candidates;p (GeV/c);SC dE/dX (MeV/cm)");
				dHistMap_dEdXVsP[locPID][SYS_START] = GetOrCreate_Histogram<TH2I>(locHistName, locHistTitle, dNum2DPBins, dMinP, dMaxP, dNum2DdEdxBins, dMindEdX, dMaxdEdX);

				locHistName = "DeltadEdXVsP";
				locHistTitle = locParticleROOTName + string(" Candidates;p (GeV/c);SC #Delta(dE/dX) (MeV/cm)");
				dHistMap_DeltadEdXVsP[locPID][SYS_START] = GetOrCreate_Histogram<TH2I>(locHistName, locHistTitle, dNum2DPBins, dMinP, dMaxP, dNum2DDeltadEdxBins, dMinDeltadEdx, dMaxDeltadEdx);

				locHistName = "DeltaBetaVsP";
				locHistTitle = locParticleROOTName + string(" Candidates;p (GeV/c);SC #Delta#beta");
				dHistMap_DeltaBetaVsP[locPID][SYS_START] = GetOrCreate_Histogram<TH2I>(locHistName, locHistTitle, dNum2DPBins, dMinP, dMaxP, dNum2DDeltaBetaBins, dMinDeltaBeta, dMaxDeltaBeta);

				locHistName = "BetaVsP";
				locHistTitle = locParticleROOTName + string(" Candidates;p (GeV/c);SC #beta");
				dHistMap_BetaVsP[locPID][SYS_START] = GetOrCreate_Histogram<TH2I>(locHistName, locHistTitle, dNum2DPBins, dMinP, dMaxP, dNum2DBetaBins, dMinBeta, dMaxBeta);

				locHistName = "DeltaTVsP";
				locHistTitle = locParticleROOTName + string(" Candidates;p (GeV/c);#Deltat_{SC - RF}");
				dHistMap_DeltaTVsP[locPID][SYS_START] = GetOrCreate_Histogram<TH2I>(locHistName, locHistTitle, dNum2DPBins, dMinP, dMaxP, dNum2DDeltaTBins, dMinDeltaT, dMaxDeltaT);

				locHistName = string("TimePullVsP_") + locParticleName;
				locHistTitle = string("SC ") + locParticleROOTName + string(";p (GeV/c);#Deltat/#sigma_{#Deltat}");
				dHistMap_TimePullVsP[locPID][SYS_START] = GetOrCreate_Histogram<TH2I>(locHistName, locHistTitle, dNum2DPBins, dMinP, dMaxP, dNum2DPullBins, dMinPull, dMaxPull);

				locHistName = string("TimeFOMVsP_") + locParticleName;
				locHistTitle = string("SC") + locParticleROOTName + string(";p (GeV/c);Timing PID Confidence Level");
				dHistMap_TimeFOMVsP[locPID][SYS_START] = GetOrCreate_Histogram<TH2I>(locHistName, locHistTitle, dNum2DPBins, dMinP, dMaxP, dNum2DFOMBins, 0.0, 1.0);

				gDirectory->cd("..");

				//TOF
				CreateAndChangeTo_Directory("TOF", "TOF");

				locHistName = "dEdXVsP";
				locHistTitle = locParticleROOTName + string(" Candidates;p (GeV/c);TOF dE/dX (MeV/cm)");
				dHistMap_dEdXVsP[locPID][SYS_TOF] = GetOrCreate_Histogram<TH2I>(locHistName, locHistTitle, dNum2DPBins, dMinP, dMaxP, dNum2DdEdxBins, dMindEdX, dMaxdEdX);

				locHistName = "DeltadEdXVsP";
				locHistTitle = locParticleROOTName + string(" Candidates;p (GeV/c);TOF #Delta(dE/dX) (MeV/cm)");
				dHistMap_DeltadEdXVsP[locPID][SYS_TOF] = GetOrCreate_Histogram<TH2I>(locHistName, locHistTitle, dNum2DPBins, dMinP, dMaxP, dNum2DDeltadEdxBins, dMinDeltadEdx, dMaxDeltadEdx);

				locHistName = "DeltaBetaVsP";
				locHistTitle = locParticleROOTName + string(" Candidates;p (GeV/c);TOF #Delta#beta");
				dHistMap_DeltaBetaVsP[locPID][SYS_TOF] = GetOrCreate_Histogram<TH2I>(locHistName, locHistTitle, dNum2DPBins, dMinP, dMaxP, dNum2DDeltaBetaBins, dMinDeltaBeta, dMaxDeltaBeta);

				locHistName = "BetaVsP";
				locHistTitle = locParticleROOTName + string(" Candidates;p (GeV/c);TOF #beta");
				dHistMap_BetaVsP[locPID][SYS_TOF] = GetOrCreate_Histogram<TH2I>(locHistName, locHistTitle, dNum2DPBins, dMinP, dMaxP, dNum2DBetaBins, dMinBeta, dMaxBeta);

				locHistName = "DeltaTVsP";
				locHistTitle = locParticleROOTName + string(" Candidates;p (GeV/c);#Deltat_{TOF - RF}");
				dHistMap_DeltaTVsP[locPID][SYS_TOF] = GetOrCreate_Histogram<TH2I>(locHistName, locHistTitle, dNum2DPBins, dMinP, dMaxP, dNum2DDeltaTBins, dMinDeltaT, dMaxDeltaT);

				locHistName = string("TimePullVsP_") + locParticleName;
				locHistTitle = string("TOF ") + locParticleROOTName + string(";p (GeV/c);#Deltat/#sigma_{#Deltat}");
				dHistMap_TimePullVsP[locPID][SYS_TOF] = GetOrCreate_Histogram<TH2I>(locHistName, locHistTitle, dNum2DPBins, dMinP, dMaxP, dNum2DPullBins, dMinPull, dMaxPull);

				locHistName = string("TimeFOMVsP_") + locParticleName;
				locHistTitle = string("TOF ") + locParticleROOTName + string(";p (GeV/c);Timing PID Confidence Level");
				dHistMap_TimeFOMVsP[locPID][SYS_TOF] = GetOrCreate_Histogram<TH2I>(locHistName, locHistTitle, dNum2DPBins, dMinP, dMaxP, dNum2DFOMBins, 0.0, 1.0);

				gDirectory->cd("..");

				//BCAL
				CreateAndChangeTo_Directory("BCAL", "BCAL");

				locHistName = "BetaVsP";
				locHistTitle = locParticleROOTName + string(" Candidates;p (GeV/c);BCAL #beta");
				dHistMap_BetaVsP[locPID][SYS_BCAL] = GetOrCreate_Histogram<TH2I>(locHistName, locHistTitle, dNum2DPBins, dMinP, dMaxBCALP, dNum2DBetaBins, dMinBeta, dMaxBeta);

				locHistName = "DeltaBetaVsP";
				locHistTitle = locParticleROOTName + string(" Candidates;p (GeV/c);BCAL #Delta#beta");
				dHistMap_DeltaBetaVsP[locPID][SYS_BCAL] = GetOrCreate_Histogram<TH2I>(locHistName, locHistTitle, dNum2DPBins, dMinP, dMaxBCALP, dNum2DDeltaBetaBins, dMinDeltaBeta, dMaxDeltaBeta);

				locHistName = "DeltaTVsP";
				locHistTitle = locParticleROOTName + string(" Candidates;p (GeV/c);#Deltat_{BCAL - RF}");
				dHistMap_DeltaTVsP[locPID][SYS_BCAL] = GetOrCreate_Histogram<TH2I>(locHistName, locHistTitle, dNum2DPBins, dMinP, dMaxP, dNum2DDeltaTBins, dMinDeltaT, dMaxDeltaT);

				locHistName = string("TimePullVsP_") + locParticleName;
				locHistTitle = string("BCAL ") + locParticleROOTName + string(";p (GeV/c);#Deltat/#sigma_{#Deltat}");
				dHistMap_TimePullVsP[locPID][SYS_BCAL] = GetOrCreate_Histogram<TH2I>(locHistName, locHistTitle, dNum2DPBins, dMinP, dMaxP, dNum2DPullBins, dMinPull, dMaxPull);

				locHistName = string("TimeFOMVsP_") + locParticleName;
				locHistTitle = string("BCAL ") + locParticleROOTName + string(";p (GeV/c);Timing PID Confidence Level");
				dHistMap_TimeFOMVsP[locPID][SYS_BCAL] = GetOrCreate_Histogram<TH2I>(locHistName, locHistTitle, dNum2DPBins, dMinP, dMaxP, dNum2DFOMBins, 0.0, 1.0);

				locHistName = "EOverPVsP";
				locHistTitle = locParticleROOTName + string(" Candidates;p (GeV/c);BCAL E_{Shower}/p_{Track} (c);");
				dHistMap_EOverPVsP[locPID][SYS_BCAL] = GetOrCreate_Histogram<TH2I>(locHistName, locHistTitle, dNum2DPBins, dMinP, dMaxBCALP, dNum2DEOverPBins, dMinEOverP, dMaxEOverP);

				locHistName = "EOverPVsTheta";
				locHistTitle = locParticleROOTName + string(" Candidates;#theta#circ;BCAL E_{Shower}/p_{Track} (c);");
				dHistMap_EOverPVsTheta[locPID][SYS_BCAL] = GetOrCreate_Histogram<TH2I>(locHistName, locHistTitle, dNum2DBCALThetaBins, dMinBCALTheta, dMaxBCALTheta, dNum2DEOverPBins, dMinEOverP, dMaxEOverP);

				gDirectory->cd("..");

				//FCAL
				CreateAndChangeTo_Directory("FCAL", "FCAL");

				locHistName = "BetaVsP";
				locHistTitle = locParticleROOTName + string(" Candidates;p (GeV/c);FCAL #beta");
				dHistMap_BetaVsP[locPID][SYS_FCAL] = GetOrCreate_Histogram<TH2I>(locHistName, locHistTitle, dNum2DPBins, dMinP, dMaxP, dNum2DBetaBins, dMinBeta, dMaxBeta);

				locHistName = "DeltaBetaVsP";
				locHistTitle = locParticleROOTName + string(" Candidates;p (GeV/c);FCAL #Delta#beta");
				dHistMap_DeltaBetaVsP[locPID][SYS_FCAL] = GetOrCreate_Histogram<TH2I>(locHistName, locHistTitle, dNum2DPBins, dMinP, dMaxP, dNum2DDeltaBetaBins, dMinDeltaBeta, dMaxDeltaBeta);

				locHistName = "DeltaTVsP";
				locHistTitle = locParticleROOTName + string(" Candidates;p (GeV/c);#Deltat_{FCAL - RF}");
				dHistMap_DeltaTVsP[locPID][SYS_FCAL] = GetOrCreate_Histogram<TH2I>(locHistName, locHistTitle, dNum2DPBins, dMinP, dMaxP, dNum2DDeltaTBins, dMinDeltaT, dMaxDeltaT);

				locHistName = string("TimePullVsP_") + locParticleName;
				locHistTitle = string("FCAL ") + locParticleROOTName + string(";p (GeV/c);#Deltat/#sigma_{#Deltat}");
				dHistMap_TimePullVsP[locPID][SYS_FCAL] = GetOrCreate_Histogram<TH2I>(locHistName, locHistTitle, dNum2DPBins, dMinP, dMaxP, dNum2DPullBins, dMinPull, dMaxPull);

				locHistName = string("TimeFOMVsP_") + locParticleName;
				locHistTitle = string("FCAL ") + locParticleROOTName + string(";p (GeV/c);Timing PID Confidence Level");
				dHistMap_TimeFOMVsP[locPID][SYS_FCAL] = GetOrCreate_Histogram<TH2I>(locHistName, locHistTitle, dNum2DPBins, dMinP, dMaxP, dNum2DFOMBins, 0.0, 1.0);

				locHistName = "EOverPVsP";
				locHistTitle = locParticleROOTName + string(" Candidates;p (GeV/c);FCAL E_{Shower}/p_{Track} (c);");
				dHistMap_EOverPVsP[locPID][SYS_FCAL] = GetOrCreate_Histogram<TH2I>(locHistName, locHistTitle, dNum2DPBins, dMinP, dMaxP, dNum2DEOverPBins, dMinEOverP, dMaxEOverP);

				locHistName = "EOverPVsTheta";
				locHistTitle = locParticleROOTName + string(" Candidates;#theta#circ;FCAL E_{Shower}/p_{Track} (c);");
				dHistMap_EOverPVsTheta[locPID][SYS_FCAL] = GetOrCreate_Histogram<TH2I>(locHistName, locHistTitle, dNum2DFCALThetaBins, dMinFCALTheta, dMaxFCALTheta, dNum2DEOverPBins, dMinEOverP, dMaxEOverP);

				gDirectory->cd("..");

				//ECAL
				CreateAndChangeTo_Directory("ECAL", "ECAL");

				locHistName = "BetaVsP";
				locHistTitle = locParticleROOTName + string(" Candidates;p (GeV/c);ECAL #beta");
				dHistMap_BetaVsP[locPID][SYS_ECAL] = GetOrCreate_Histogram<TH2I>(locHistName, locHistTitle, dNum2DPBins, dMinP, dMaxP, dNum2DBetaBins, dMinBeta, dMaxBeta);

				locHistName = "DeltaBetaVsP";
				locHistTitle = locParticleROOTName + string(" Candidates;p (GeV/c);ECAL #Delta#beta");
				dHistMap_DeltaBetaVsP[locPID][SYS_ECAL] = GetOrCreate_Histogram<TH2I>(locHistName, locHistTitle, dNum2DPBins, dMinP, dMaxP, dNum2DDeltaBetaBins, dMinDeltaBeta, dMaxDeltaBeta);

				locHistName = "DeltaTVsP";
				locHistTitle = locParticleROOTName + string(" Candidates;p (GeV/c);#Deltat_{ECAL - RF}");
				dHistMap_DeltaTVsP[locPID][SYS_ECAL] = GetOrCreate_Histogram<TH2I>(locHistName, locHistTitle, dNum2DPBins, dMinP, dMaxP, dNum2DDeltaTBins, dMinDeltaT, dMaxDeltaT);

				locHistName = string("TimePullVsP_") + locParticleName;
				locHistTitle = string("ECAL ") + locParticleROOTName + string(";p (GeV/c);#Deltat/#sigma_{#Deltat}");
				dHistMap_TimePullVsP[locPID][SYS_ECAL] = GetOrCreate_Histogram<TH2I>(locHistName, locHistTitle, dNum2DPBins, dMinP, dMaxP, dNum2DPullBins, dMinPull, dMaxPull);

				locHistName = string("TimeFOMVsP_") + locParticleName;
				locHistTitle = string("ECAL ") + locParticleROOTName + string(";p (GeV/c);Timing PID Confidence Level");
				dHistMap_TimeFOMVsP[locPID][SYS_ECAL] = GetOrCreate_Histogram<TH2I>(locHistName, locHistTitle, dNum2DPBins, dMinP, dMaxP, dNum2DFOMBins, 0.0, 1.0);

				locHistName = "EOverPVsP";
				locHistTitle = locParticleROOTName + string(" Candidates;p (GeV/c);ECAL E_{Shower}/p_{Track} (c);");
				dHistMap_EOverPVsP[locPID][SYS_ECAL] = GetOrCreate_Histogram<TH2I>(locHistName, locHistTitle, dNum2DPBins, dMinP, dMaxP, dNum2DEOverPBins, dMinEOverP, dMaxEOverP);

				locHistName = "EOverPVsTheta";
				locHistTitle = locParticleROOTName + string(" Candidates;#theta#circ;ECAL E_{Shower}/p_{Track} (c);");
				dHistMap_EOverPVsTheta[locPID][SYS_ECAL] = GetOrCreate_Histogram<TH2I>(locHistName, locHistTitle, dNum2DFCALThetaBins, dMinFCALTheta, dMaxFCALTheta, dNum2DEOverPBins, dMinEOverP, dMaxEOverP);

				gDirectory->cd("..");

				//CCAL
				CreateAndChangeTo_Directory("CCAL", "CCAL");

				locHistName = "BetaVsP";
				locHistTitle = locParticleROOTName + string(" Candidates;p (GeV/c);CCAL #beta");
				dHistMap_BetaVsP[locPID][SYS_CCAL] = GetOrCreate_Histogram<TH2I>(locHistName, locHistTitle, dNum2DPBins, dMinP, dMaxP, dNum2DBetaBins, dMinBeta, dMaxBeta);

				locHistName = "DeltaBetaVsP";
				locHistTitle = locParticleROOTName + string(" Candidates;p (GeV/c);CCAL #Delta#beta");
				dHistMap_DeltaBetaVsP[locPID][SYS_CCAL] = GetOrCreate_Histogram<TH2I>(locHistName, locHistTitle, dNum2DPBins, dMinP, dMaxP, dNum2DDeltaBetaBins, dMinDeltaBeta, dMaxDeltaBeta);

				locHistName = "DeltaTVsP";
				locHistTitle = locParticleROOTName + string(" Candidates;p (GeV/c);#Deltat_{CCAL - RF}");
				dHistMap_DeltaTVsP[locPID][SYS_CCAL] = GetOrCreate_Histogram<TH2I>(locHistName, locHistTitle, dNum2DPBins, dMinP, dMaxP, dNum2DDeltaTBins, dMinDeltaT, dMaxDeltaT);

				locHistName = string("TimePullVsP_") + locParticleName;
				locHistTitle = string("CCAL ") + locParticleROOTName + string(";p (GeV/c);#Deltat/#sigma_{#Deltat}");
				dHistMap_TimePullVsP[locPID][SYS_CCAL] = GetOrCreate_Histogram<TH2I>(locHistName, locHistTitle, dNum2DPBins, dMinP, dMaxP, dNum2DPullBins, dMinPull, dMaxPull);

				locHistName = string("TimeFOMVsP_") + locParticleName;
				locHistTitle = string("CCAL ") + locParticleROOTName + string(";p (GeV/c);Timing PID Confidence Level");
				dHistMap_TimeFOMVsP[locPID][SYS_CCAL] = GetOrCreate_Histogram<TH2I>(locHistName, locHistTitle, dNum2DPBins, dMinP, dMaxP, dNum2DFOMBins, 0.0, 1.0);

				locHistName = "EOverPVsP";
				locHistTitle = locParticleROOTName + string(" Candidates;p (GeV/c);CCAL E_{Shower}/p_{Track} (c);");
				dHistMap_EOverPVsP[locPID][SYS_CCAL] = GetOrCreate_Histogram<TH2I>(locHistName, locHistTitle, dNum2DPBins, dMinP, dMaxP, dNum2DEOverPBins, dMinEOverP, dMaxEOverP);
				
				locHistName = "EOverPVsTheta";
				locHistTitle = locParticleROOTName + string(" Candidates;#theta#circ;CCAL E_{Shower}/p_{Track} (c);");
				dHistMap_EOverPVsTheta[locPID][SYS_CCAL] = GetOrCreate_Histogram<TH2I>(locHistName, locHistTitle, dNum2DCCALThetaBins, dMinCCALTheta, dMaxCCALTheta, dNum2DEOverPBins, dMinEOverP, dMaxEOverP);

				gDirectory->cd("..");

				//CDC
				CreateAndChangeTo_Directory("CDC", "CDC");

				locHistName = "dEdXVsP_Int";
				locHistTitle = locParticleROOTName + string(" Candidates;p (GeV/c);CDC dE/dx [Integral-based] (keV/cm)");
				dHistMap_dEdXVsP[locPID][SYS_CDC] = GetOrCreate_Histogram<TH2I>(locHistName, locHistTitle, dNum2DPBins, dMinP, dMaxP, dNum2DdEdxBins, dMindEdX, dMaxdEdX);

				locHistName = "dEdXVsP_Amp";
				locHistTitle = locParticleROOTName + string(" Candidates;p (GeV/c);CDC dE/dx [Amplitude-based] (keV/cm)");
				dHistMap_dEdXVsP[locPID][SYS_CDC_AMP] = GetOrCreate_Histogram<TH2I>(locHistName, locHistTitle, dNum2DPBins, dMinP, dMaxP, dNum2DdEdxBins, dMindEdX, dMaxdEdX);

				locHistName = "DeltadEdXVsP_Int";
				locHistTitle = locParticleROOTName + string(" Candidates;p (GeV/c);CDC #Delta(dE/dX) [Integral-based] (keV/cm)");
				dHistMap_DeltadEdXVsP[locPID][SYS_CDC] = GetOrCreate_Histogram<TH2I>(locHistName, locHistTitle, dNum2DPBins, dMinP, dMaxP, dNum2DDeltadEdxBins, dMinDeltadEdx, dMaxDeltadEdx);

				locHistName = "DeltadEdXVsP_Amp";
				locHistTitle = locParticleROOTName + string(" Candidates;p (GeV/c);CDC #Delta(dE/dX) [Amplitude-based] (keV/cm)");
				dHistMap_DeltadEdXVsP[locPID][SYS_CDC_AMP] = GetOrCreate_Histogram<TH2I>(locHistName, locHistTitle, dNum2DPBins, dMinP, dMaxP, dNum2DDeltadEdxBins, dMinDeltadEdx, dMaxDeltadEdx);

				locHistName = string("dEdXPullVsP_Int_") + locParticleName;
				locHistTitle = locParticleROOTName + string(";p (GeV/c);CDC #Delta(dE/dX)/#sigma_{#Delta(dE/dX)} [Integral-based]");
				dHistMap_dEdXPullVsP[locPID][SYS_CDC] = GetOrCreate_Histogram<TH2I>(locHistName, locHistTitle, dNum2DPBins, dMinP, dMaxP, dNum2DPullBins, dMinPull, dMaxPull);

				locHistName = string("dEdXPullVsP_Amp_") + locParticleName;
				locHistTitle = locParticleROOTName + string(";p (GeV/c);CDC #Delta(dE/dX)/#sigma_{#Delta(dE/dX)} [Amplitude-based]");
				dHistMap_dEdXPullVsP[locPID][SYS_CDC_AMP] = GetOrCreate_Histogram<TH2I>(locHistName, locHistTitle, dNum2DPBins, dMinP, dMaxP, dNum2DPullBins, dMinPull, dMaxPull);

				locHistName = string("dEdXFOMVsP_Int_") + locParticleName;
				locHistTitle = locParticleROOTName + string(";p (GeV/c);CDC dE/dx [Integral-based] PID Confidence Level");
				dHistMap_dEdXFOMVsP[locPID][SYS_CDC] = GetOrCreate_Histogram<TH2I>(locHistName, locHistTitle, dNum2DPBins, dMinP, dMaxP, dNum2DFOMBins, 0.0, 1.0);

				locHistName = string("dEdXFOMVsP_Amp_") + locParticleName;
				locHistTitle = locParticleROOTName + string(";p (GeV/c);CDC dE/dx [Amplitude-based] PID Confidence Level");
				dHistMap_dEdXFOMVsP[locPID][SYS_CDC_AMP] = GetOrCreate_Histogram<TH2I>(locHistName, locHistTitle, dNum2DPBins, dMinP, dMaxP, dNum2DFOMBins, 0.0, 1.0);

				gDirectory->cd("..");

				//FDC
				CreateAndChangeTo_Directory("FDC", "FDC");

				locHistName = "dEdXVsP";
				locHistTitle = locParticleROOTName + string(" Candidates;p (GeV/c);FDC dE/dx (keV/cm)");
				dHistMap_dEdXVsP[locPID][SYS_FDC] = GetOrCreate_Histogram<TH2I>(locHistName, locHistTitle, dNum2DPBins, dMinP, dMaxP, dNum2DdEdxBins, dMindEdX, dMaxdEdX);

				locHistName = "DeltadEdXVsP";
				locHistTitle = locParticleROOTName + string(" Candidates;p (GeV/c);FDC #Delta(dE/dX) (keV/cm)");
				dHistMap_DeltadEdXVsP[locPID][SYS_FDC] = GetOrCreate_Histogram<TH2I>(locHistName, locHistTitle, dNum2DPBins, dMinP, dMaxP, dNum2DDeltadEdxBins, dMinDeltadEdx, dMaxDeltadEdx);

				locHistName = string("dEdXPullVsP_") + locParticleName;
				locHistTitle = locParticleROOTName + string(";p (GeV/c);FDC #Delta(dE/dX)/#sigma_{#Delta(dE/dX)}");
				dHistMap_dEdXPullVsP[locPID][SYS_FDC] = GetOrCreate_Histogram<TH2I>(locHistName, locHistTitle, dNum2DPBins, dMinP, dMaxP, dNum2DPullBins, dMinPull, dMaxPull);

				locHistName = string("dEdXFOMVsP_") + locParticleName;
				locHistTitle = locParticleROOTName + string(";p (GeV/c);FDC dE/dx PID Confidence Level");
				dHistMap_dEdXFOMVsP[locPID][SYS_FDC] = GetOrCreate_Histogram<TH2I>(locHistName, locHistTitle, dNum2DPBins, dMinP, dMaxP, dNum2DFOMBins, 0.0, 1.0);

				gDirectory->cd("..");

				// DIRC
				CreateAndChangeTo_Directory("DIRC", "DIRC");
				
				locHistName = string("NumPhotons_") + locParticleName;
				locHistTitle = locParticleROOTName + string("; DIRC NumPhotons");
				dHistMap_NumPhotons_DIRC[locPID] = new TH1I(locHistName.c_str(), locHistTitle.c_str(), dDIRCNumPhotonsBins, dDIRCMinNumPhotons, dDIRCMaxNumPhotons);
				
				locHistName = string("ThetaCVsP_") + locParticleName;
				locHistTitle = locParticleROOTName + string("; Momentum (GeV); DIRC #theta_{C}");
				dHistMap_ThetaCVsP_DIRC[locPID] = new TH2I(locHistName.c_str(), locHistTitle.c_str(), dNum2DPBins, dMinP, dMaxP, dDIRCThetaCBins, dDIRCMinThetaC, dDIRCMaxThetaC);
				
				locHistName = string("Ldiff_kpiVsP_") + locParticleName;
				locHistTitle = locParticleROOTName + string("; Momentum (GeV); DIRC L_{K}-L_{#pi}");
				dHistMap_Ldiff_kpiVsP_DIRC[locPID] = new TH2I(locHistName.c_str(), locHistTitle.c_str(), dNum2DPBins, dMinP, dMaxP, dDIRCLikelihoodBins, -1*dDIRCMaxLikelihood, dDIRCMaxLikelihood);
				
				locHistName = string("Ldiff_pkVsP_") + locParticleName;
				locHistTitle = locParticleROOTName + string("; Momentum (GeV); DIRC L_{p}-L_{K}");
				dHistMap_Ldiff_pkVsP_DIRC[locPID] = new TH2I(locHistName.c_str(), locHistTitle.c_str(), dNum2DPBins, dMinP, dMaxP, dDIRCLikelihoodBins, -1*dDIRCMaxLikelihood, dDIRCMaxLikelihood);

				gDirectory->cd("..");
			} //end of charged

			//one per thrown pid:
			if(!locMCThrowns.empty())
			{
				CreateAndChangeTo_Directory("Throwns", "Throwns");
				for(size_t loc_j = 0; loc_j < dThrownPIDs.size(); ++loc_j)
				{
					locPID2 = dThrownPIDs[loc_j];
					if((ParticleCharge(locPID2) != ParticleCharge(locPID)) && (locPID2 != UnknownParticle))
						continue;
					locParticleName2 = ParticleType(locPID2);
					locParticleROOTName2 = ParticleName_ROOT(locPID2);

					//Confidence Level for Thrown PID
					pair<Particle_t, Particle_t> locPIDPair(locPID, locPID2);
					locHistName = string("PIDConfidenceLevel_ForThrownPID_") + locParticleName2;
					locHistTitle = locParticleROOTName + string(" PID, Thrown PID = ") + locParticleROOTName2 + string(";PID Confidence Level");
					dHistMap_PIDFOMForTruePID[locPIDPair] = GetOrCreate_Histogram<TH1I>(locHistName, locHistTitle, dNumFOMBins, 0.0, 1.0);
				}
				gDirectory->cd("..");
			}

			//no FOM for massive neutrals
			if((ParticleCharge(locPID) != 0 && Is_FinalStateParticle(locPID)) || (locPID == Gamma))
			{
				// Overall Confidence Level
				locHistName = "PIDConfidenceLevel";
				locHistTitle = locParticleROOTName + string(" PID;PID Confidence Level");
				dHistMap_PIDFOM[locPID] = GetOrCreate_Histogram<TH1I>(locHistName, locHistTitle, dNumFOMBins, 0.0, 1.0);

				// P Vs Theta, PID FOM = NaN
				locHistName = "PVsTheta_NaNPIDFOM";
				locHistTitle = locParticleROOTName + string(", PID FOM = NaN;#theta#circ;p (GeV/c)");
				dHistMap_PVsTheta_NaNPIDFOM[locPID] = GetOrCreate_Histogram<TH2I>(locHistName, locHistTitle, dNum2DThetaBins, dMinTheta, dMaxTheta, dNum2DPBins, dMinP, dMaxP);
			}

			// look at decaying particles
			if(!Is_FinalStateParticle(locPID))
			{			
				// Flight Distance
				locHistName = "FlightDistance";
				locHistTitle = locParticleROOTName + string(" Flight Distance;Flight Distance (cm)");
				dHistMap_FlightDistance[locPID] = GetOrCreate_Histogram<TH1I>(locHistName, locHistTitle, dNumFlightDistanceBins, 0.0, dMaxFlightDistance);

				// Flight Distance Vs P
				locHistName = "FlightDistanceVsP";
				locHistTitle = locParticleROOTName + string(" Flight Distance;Flight Distance (cm);p (GeV/c)");
				dHistMap_FlightDistanceVsP[locPID] = GetOrCreate_Histogram<TH2I>(locHistName, locHistTitle, dNumFlightDistanceBins, 0.0, dMaxFlightDistance, dNum2DPBins, dMinP, dMaxP);

				// Flight Distance Vs Theta
				locHistName = "FlightDistanceVsTheta";
				locHistTitle = locParticleROOTName + string(" Flight Distance;Flight Distance (cm);#theta#circ");
				dHistMap_FlightDistanceVsTheta[locPID] = GetOrCreate_Histogram<TH2I>(locHistName, locHistTitle, dNumFlightDistanceBins, 0.0, dMaxFlightDistance, dNum2DThetaBins, dMinTheta, dMaxTheta);


				// Flight Distance
				locHistName = "FlightSignificance";
				locHistTitle = locParticleROOTName + string(" Flight Significance;Flight Significance (#sigma)");
				dHistMap_FlightSignificance[locPID] = GetOrCreate_Histogram<TH1I>(locHistName, locHistTitle, dNumFlightSignificanceBins, 0.0, dMaxFlightSignificance);

				// Flight Distance Vs P
				locHistName = "FlightSignificanceVsP";
				locHistTitle = locParticleROOTName + string(" Flight Significance;Flight Significance (#sigma);p (GeV/c)");
				dHistMap_FlightSignificanceVsP[locPID] = GetOrCreate_Histogram<TH2I>(locHistName, locHistTitle, dNumFlightSignificanceBins, 0.0, dMaxFlightSignificance, dNum2DPBins, dMinP, dMaxP);

				// Flight Distance Vs Theta
				locHistName = "FlightSignificanceVsTheta";
				locHistTitle = locParticleROOTName + string(" Flight Significance;Flight Significance (#sigma);#theta#circ");
				dHistMap_FlightSignificanceVsTheta[locPID] = GetOrCreate_Histogram<TH2I>(locHistName, locHistTitle, dNumFlightSignificanceBins, 0.0, dMaxFlightSignificance, dNum2DThetaBins, dMinTheta, dMaxTheta);

			}


			gDirectory->cd("..");
		} //end of PID loop

		//Return to the base directory
		ChangeTo_BaseDirectory();
	}
	DEvent::GetLockService(locEvent)->RootUnLock(); //RELEASE ROOT LOCK!!
}

void DHistogramAction_PID::Run_Update(const std::shared_ptr<const JEvent>& locEvent)
{
	vector<const DParticleID*> locParticleIDs;
	locEvent->Get(locParticleIDs);

	vector<const DAnalysisUtilities*> locAnalysisUtilitiesVector;
	locEvent->Get(locAnalysisUtilitiesVector);

	dParticleID = locParticleIDs[0];
	dAnalysisUtilities = locAnalysisUtilitiesVector[0];
}

bool DHistogramAction_PID::Perform_Action(const std::shared_ptr<const JEvent>& locEvent, const DParticleCombo* locParticleCombo)
{
	vector<const DMCThrownMatching*> locMCThrownMatchingVector;
	locEvent->Get(locMCThrownMatchingVector);
	const DMCThrownMatching* locMCThrownMatching = locMCThrownMatchingVector.empty() ? NULL : locMCThrownMatchingVector[0];

	const DEventRFBunch* locEventRFBunch = locParticleCombo->Get_EventRFBunch();

	vector<const DReactionVertexInfo*> locVertexInfos;
	locEvent->Get(locVertexInfos);
	
	// figure out what the right DReactionVertexInfo is
	const DReactionVertexInfo *locReactionVertexInfo = nullptr;
	for(auto& locVertexInfo : locVertexInfos) {
		auto locVertexReactions = locVertexInfo->Get_Reactions();
		if(find(locVertexReactions.begin(), locVertexReactions.end(), Get_Reaction()) != locVertexReactions.end()) {
			locReactionVertexInfo = locVertexInfo;
			break;
		}
	}

	for(size_t loc_i = 0; loc_i < locParticleCombo->Get_NumParticleComboSteps(); ++loc_i)
	{	
		const DParticleComboStep* locParticleComboStep = locParticleCombo->Get_ParticleComboStep(loc_i);
		auto locParticles = Get_UseKinFitResultsFlag() ? locParticleComboStep->Get_FinalParticles(Get_Reaction()->Get_ReactionStep(loc_i), false, false, d_AllCharges) : locParticleComboStep->Get_FinalParticles_Measured(Get_Reaction()->Get_ReactionStep(loc_i), d_AllCharges);
		for(size_t loc_j = 0; loc_j < locParticles.size(); ++loc_j)
		{
			//check if will be duplicate
			pair<Particle_t, const JObject*> locParticleInfo(locParticles[loc_j]->PID(), Get_FinalParticle_SourceObject(locParticles[loc_j]));
			pair<const DEventRFBunch*, pair<Particle_t, const JObject*> > locHistInfo(locEventRFBunch, locParticleInfo);
			if(dPreviouslyHistogrammedParticles.find(locHistInfo) != dPreviouslyHistogrammedParticles.end())
				continue; //previously histogrammed

			dPreviouslyHistogrammedParticles.insert(locHistInfo);
			if(Is_FinalStateParticle(locParticles[loc_j]->PID())) { 
				if(ParticleCharge(locParticles[loc_j]->PID()) != 0) //charged
					Fill_ChargedHists(static_cast<const DChargedTrackHypothesis*>(locParticles[loc_j]), locMCThrownMatching, locEventRFBunch);
				else //neutral
					Fill_NeutralHists(static_cast<const DNeutralParticleHypothesis*>(locParticles[loc_j]), locMCThrownMatching, locEventRFBunch);
			}
		}

		// fill info on decaying particles, which is on the particle combo step level
		if(((locParticleComboStep->Get_InitialParticle()!=nullptr) && !Is_FinalStateParticle(locParticleComboStep->Get_InitialParticle()->PID()))) {  // decaying particles
			// FOR NOW: we only calculate these displaced vertex quantities if a kinematic fit 
			// was performed where the vertex is constrained
			// TODO: Cleverly handle the other cases

			// pull out information related to the kinematic fit
			if( !Get_UseKinFitResultsFlag() ) continue; 
			if( locReactionVertexInfo == nullptr ) continue; 

			// extract the info about the vertex, to make sure that the information that we 
			// need to calculate displaced vertices is there
			auto locStepVertexInfo = locReactionVertexInfo->Get_StepVertexInfo(loc_i);

			Fill_DecayingHists(locParticleComboStep->Get_InitialParticle(), locParticleComboStep->Get_InitialKinFitParticle(), locStepVertexInfo);
		}
		
	}

	return true;
}

void DHistogramAction_PID::Fill_DecayingHists(const DKinematicData *locInitialParticle, std::shared_ptr<const DKinFitParticle> locKinFitParticle, const DReactionStepVertexInfo *locStepVertexInfo)
{
	DKinFitType locKinFitType = Get_Reaction()->Get_KinFitType();

	//Particle_t locPID = static_cast<Particle_t>(locKinFitParticle->Get_PID());
	//double locP = locKinFitParticle->Get_Momentum().Mag();
	//double locTheta = locKinFitParticle->Get_Momentum().Theta()*180.0/TMath::Pi();
	Particle_t locPID = locInitialParticle->PID();
	double locP = locInitialParticle->momentum().Mag();
	double locTheta = locInitialParticle->momentum().Theta()*180.0/TMath::Pi();

	// comment
	auto locParentVertexInfo = locStepVertexInfo->Get_ParentVertexInfo();
	auto locVertexKinFitFlag = ((locKinFitType != d_P4Fit) && (locKinFitType != d_NoFit));

	if(IsDetachedVertex(locPID) && locVertexKinFitFlag && (locParentVertexInfo != nullptr) && locStepVertexInfo->Get_FittableVertexFlag() && locParentVertexInfo->Get_FittableVertexFlag())
	{
		auto locPathLength = locKinFitParticle->Get_PathLength();
		auto locPathLengthSigma = locKinFitParticle->Get_PathLengthUncertainty();
		double locPathLengthSignificance = locPathLength / locPathLengthSigma;

		dHistMap_FlightDistance[locPID]->Fill(locPathLength);
		dHistMap_FlightDistanceVsP[locPID]->Fill(locPathLength, locP);
		dHistMap_FlightDistanceVsTheta[locPID]->Fill(locPathLength, locTheta);

		dHistMap_FlightSignificance[locPID]->Fill(locPathLengthSignificance);
		dHistMap_FlightSignificanceVsP[locPID]->Fill(locPathLengthSignificance, locP);
		dHistMap_FlightSignificanceVsTheta[locPID]->Fill(locPathLengthSignificance, locTheta);

	}

}


void DHistogramAction_PID::Fill_ChargedHists(const DChargedTrackHypothesis* locChargedTrackHypothesis, const DMCThrownMatching* locMCThrownMatching, const DEventRFBunch* locEventRFBunch)
{
	Particle_t locPID = locChargedTrackHypothesis->PID();
	double locBeta_Timing = locChargedTrackHypothesis->measuredBeta();
	double locDeltaBeta = locBeta_Timing - locChargedTrackHypothesis->lorentzMomentum().Beta();
	double locDeltaT = (locChargedTrackHypothesis->Get_TimeAtPOCAToVertex() - locChargedTrackHypothesis->t0());
	double locFOM_Timing = (locChargedTrackHypothesis->Get_NDF_Timing() > 0) ? TMath::Prob(locChargedTrackHypothesis->Get_ChiSq_Timing(), locChargedTrackHypothesis->Get_NDF_Timing()) : numeric_limits<double>::quiet_NaN();

	double locP = locChargedTrackHypothesis->momentum().Mag();
	double locTheta = locChargedTrackHypothesis->momentum().Theta()*180.0/TMath::Pi();
	double locMatchFOM = 0.0;
	const DMCThrown* locMCThrown = (locMCThrownMatching != NULL) ? locMCThrownMatching->Get_MatchingMCThrown(locChargedTrackHypothesis, locMatchFOM) : NULL;

	auto locBCALShowerMatchParams = locChargedTrackHypothesis->Get_BCALShowerMatchParams();
	auto locFCALShowerMatchParams = locChargedTrackHypothesis->Get_FCALShowerMatchParams();
	auto locECALShowerMatchParams = locChargedTrackHypothesis->Get_ECALShowerMatchParams();
	auto locTOFHitMatchParams = locChargedTrackHypothesis->Get_TOFHitMatchParams();
	auto locSCHitMatchParams = locChargedTrackHypothesis->Get_SCHitMatchParams();
	auto locDIRCMatchParams = locChargedTrackHypothesis->Get_DIRCMatchParams();

	auto locTrackTimeBased = locChargedTrackHypothesis->Get_TrackTimeBased();

	double locTimePull = 0.0;
	unsigned int locTimeNDF = 0;
	dParticleID->Calc_TimingChiSq(locChargedTrackHypothesis, locTimeNDF, locTimePull);
	DetectorSystem_t locTimeDetector = locChargedTrackHypothesis->t1_detector();

	//FILL HISTOGRAMS
	//Since we are filling histograms local to this action, it will not interfere with other ROOT operations: can use action-wide ROOT lock
	//Note, the mutex is unique to this DReaction + action_string combo: actions of same class with different hists will have a different mutex
	Lock_Action();
	{
		dHistMap_PIDFOM[locPID]->Fill(locChargedTrackHypothesis->Get_FOM());

		//SC dE/dx
		if(locSCHitMatchParams != NULL)
		{
			dHistMap_dEdXVsP[locPID][SYS_START]->Fill(locP, locSCHitMatchParams->dEdx*1.0E3);
			double locdx = locSCHitMatchParams->dHitEnergy/locSCHitMatchParams->dEdx;
			double locProbabledEdx = 0.0, locSigmadEdx = 0.0;
			dParticleID->GetScintMPdEandSigma(locP, locChargedTrackHypothesis->mass(), locdx, locProbabledEdx, locSigmadEdx);
			dHistMap_DeltadEdXVsP[locPID][SYS_START]->Fill(locP, (locSCHitMatchParams->dEdx - locProbabledEdx)*1.0E3);
		}

		//TOF dE/dx
		if(locTOFHitMatchParams != NULL)
		{
			dHistMap_dEdXVsP[locPID][SYS_TOF]->Fill(locP, locTOFHitMatchParams->dEdx*1.0E3);
			double locdx = locTOFHitMatchParams->dHitEnergy/locTOFHitMatchParams->dEdx;
			double locProbabledEdx = 0.0, locSigmadEdx = 0.0;
			dParticleID->GetScintMPdEandSigma(locP, locChargedTrackHypothesis->mass(), locdx, locProbabledEdx, locSigmadEdx);
			dHistMap_DeltadEdXVsP[locPID][SYS_TOF]->Fill(locP, (locTOFHitMatchParams->dEdx - locProbabledEdx)*1.0E3);
		}

		//BCAL E/p
		if(locBCALShowerMatchParams != NULL)
		{
			const DBCALShower* locBCALShower = locBCALShowerMatchParams->dBCALShower;
			double locEOverP = locBCALShower->E/locP;
			dHistMap_EOverPVsP[locPID][SYS_BCAL]->Fill(locP, locEOverP);
			dHistMap_EOverPVsTheta[locPID][SYS_BCAL]->Fill(locTheta, locEOverP);
		}

		//FCAL E/p
		if(locFCALShowerMatchParams != NULL)
		{
			const DFCALShower* locFCALShower = locFCALShowerMatchParams->dFCALShower;
			double locEOverP = locFCALShower->getEnergy()/locP;
			dHistMap_EOverPVsP[locPID][SYS_FCAL]->Fill(locP, locEOverP);
			dHistMap_EOverPVsTheta[locPID][SYS_FCAL]->Fill(locTheta, locEOverP);
		}
		
		//ECAL E/p
		if(locECALShowerMatchParams != NULL)
		{
			const DECALShower* locECALShower = locECALShowerMatchParams->dECALShower;
			double locEOverP = locECALShower->E/locP;
			dHistMap_EOverPVsP[locPID][SYS_ECAL]->Fill(locP, locEOverP);
			dHistMap_EOverPVsTheta[locPID][SYS_ECAL]->Fill(locTheta, locEOverP);
		}
		
		/*
		//CCAL E/p
		if(locCCALShowerMatchParams != NULL)
		{
			const DFCALShower* locCCALShower = locCCALShowerMatchParams->dFCALShower;
			double locEOverP = locCCALShower->getEnergy()/locP;
			dHistMap_EOverPVsP[locPID][SYS_CCAL]->Fill(locP, locEOverP);
			dHistMap_EOverPVsTheta[locPID][SYS_CCAL]->Fill(locTheta, locEOverP);
		}
		*/
		//DIRC
		if(locDIRCMatchParams != NULL) {
			
			int locNumPhotons_DIRC = locDIRCMatchParams->dNPhotons;
			double locThetaC_DIRC = locDIRCMatchParams->dThetaC;
			dHistMap_NumPhotons_DIRC[locPID]->Fill(locNumPhotons_DIRC);
			dHistMap_ThetaCVsP_DIRC[locPID]->Fill(locP, locThetaC_DIRC);
			double locLpi_DIRC = locDIRCMatchParams->dLikelihoodPion;
			double locLk_DIRC = locDIRCMatchParams->dLikelihoodKaon;
			double locLp_DIRC = locDIRCMatchParams->dLikelihoodProton;
			dHistMap_Ldiff_kpiVsP_DIRC[locPID]->Fill(locP, locLk_DIRC-locLpi_DIRC);
			dHistMap_Ldiff_pkVsP_DIRC[locPID]->Fill(locP, locLp_DIRC-locLk_DIRC);
		}

		//Timing
		if(dHistMap_BetaVsP[locPID].find(locTimeDetector) != dHistMap_BetaVsP[locPID].end())
		{
			dHistMap_BetaVsP[locPID][locTimeDetector]->Fill(locP, locBeta_Timing);
			dHistMap_DeltaBetaVsP[locPID][locTimeDetector]->Fill(locP, locDeltaBeta);
			dHistMap_DeltaTVsP[locPID][locTimeDetector]->Fill(locP, locDeltaT);
			dHistMap_TimePullVsP[locPID][locTimeDetector]->Fill(locP, locTimePull);
			dHistMap_TimeFOMVsP[locPID][locTimeDetector]->Fill(locP, locFOM_Timing);
		}

		//CDC dE/dx
		if(locTrackTimeBased->dNumHitsUsedFordEdx_CDC > 0)
		{
		        dHistMap_dEdXVsP[locPID][SYS_CDC]->Fill(locP, locChargedTrackHypothesis->Get_dEdx_CDC_int()*1.0E6);
			dHistMap_dEdXVsP[locPID][SYS_CDC_AMP]->Fill(locP, locChargedTrackHypothesis->Get_dEdx_CDC_amp()*1.0E6);

			double locProbabledEdx = dParticleID->GetMostProbabledEdx_DC(locP, locChargedTrackHypothesis->mass(), locTrackTimeBased->ddx_CDC, true);
			double locDeltadEdx = locChargedTrackHypothesis->Get_dEdx_CDC_int() - locProbabledEdx;
			dHistMap_DeltadEdXVsP[locPID][SYS_CDC]->Fill(locP, 1.0E6*locDeltadEdx);
			double locDeltadEdx_amp = locChargedTrackHypothesis->Get_dEdx_CDC_amp() - locProbabledEdx;
			dHistMap_DeltadEdXVsP[locPID][SYS_CDC_AMP]->Fill(locP, 1.0E6*locDeltadEdx_amp);

			double locMeandx = locTrackTimeBased->ddx_CDC/locTrackTimeBased->dNumHitsUsedFordEdx_CDC;
			double locSigmadEdx = dParticleID->GetdEdxSigma_DC(locTrackTimeBased->dNumHitsUsedFordEdx_CDC, locP, locChargedTrackHypothesis->mass(), locMeandx, true);
			double locdEdXPull = locDeltadEdx/locSigmadEdx;
			dHistMap_dEdXPullVsP[locPID][SYS_CDC]->Fill(locP, locDeltadEdx/locSigmadEdx);
			double locdEdXPull_amp = locDeltadEdx_amp/locSigmadEdx;
			dHistMap_dEdXPullVsP[locPID][SYS_CDC_AMP]->Fill(locP, locDeltadEdx_amp/locSigmadEdx);

			double locdEdXChiSq = locdEdXPull*locdEdXPull;
			double locdEdXFOM = TMath::Prob(locdEdXChiSq, locTrackTimeBased->dNumHitsUsedFordEdx_CDC);
			dHistMap_dEdXFOMVsP[locPID][SYS_CDC]->Fill(locP, locdEdXFOM);
			double locdEdXChiSq_amp = locdEdXPull_amp*locdEdXPull_amp;
			double locdEdXFOM_amp = TMath::Prob(locdEdXChiSq_amp, locTrackTimeBased->dNumHitsUsedFordEdx_CDC);
			dHistMap_dEdXFOMVsP[locPID][SYS_CDC_AMP]->Fill(locP, locdEdXFOM_amp);
		}

		//FDC dE/dx
		if(locTrackTimeBased->dNumHitsUsedFordEdx_FDC > 0)
		{
			dHistMap_dEdXVsP[locPID][SYS_FDC]->Fill(locP, locTrackTimeBased->ddEdx_FDC*1.0E6);
			double locProbabledEdx = dParticleID->GetMostProbabledEdx_DC(locP, locChargedTrackHypothesis->mass(), locTrackTimeBased->ddx_FDC, false);
			double locDeltadEdx = locTrackTimeBased->ddEdx_FDC - locProbabledEdx;
			dHistMap_DeltadEdXVsP[locPID][SYS_FDC]->Fill(locP, 1.0E6*locDeltadEdx);
			double locMeandx = locTrackTimeBased->ddx_FDC/locTrackTimeBased->dNumHitsUsedFordEdx_FDC;
			double locSigmadEdx = dParticleID->GetdEdxSigma_DC(locTrackTimeBased->dNumHitsUsedFordEdx_FDC, locP, locChargedTrackHypothesis->mass(), locMeandx, false);
			double locdEdXPull = locDeltadEdx/locSigmadEdx;
			dHistMap_dEdXPullVsP[locPID][SYS_FDC]->Fill(locP, locDeltadEdx/locSigmadEdx);
			double locdEdXChiSq = locdEdXPull*locdEdXPull;
			double locdEdXFOM = TMath::Prob(locdEdXChiSq, locTrackTimeBased->dNumHitsUsedFordEdx_FDC);
			dHistMap_dEdXFOMVsP[locPID][SYS_FDC]->Fill(locP, locdEdXFOM);
		}

		pair<Particle_t, Particle_t> locPIDPair(locPID, UnknownParticle); //default unless matched
		if(locMCThrown != NULL) //else bogus track (not matched to any thrown tracks)
			locPIDPair.second = (Particle_t)(locMCThrown->type); //matched
		if(dHistMap_PIDFOMForTruePID.find(locPIDPair) != dHistMap_PIDFOMForTruePID.end()) //else hist not created or PID is weird
			dHistMap_PIDFOMForTruePID[locPIDPair]->Fill(locChargedTrackHypothesis->Get_FOM());

		if(locChargedTrackHypothesis->Get_NDF() == 0) //NaN
			dHistMap_PVsTheta_NaNPIDFOM[locPID]->Fill(locTheta, locP);
	}
	Unlock_Action();
}

void DHistogramAction_PID::Fill_NeutralHists(const DNeutralParticleHypothesis* locNeutralParticleHypothesis, const DMCThrownMatching* locMCThrownMatching, const DEventRFBunch* locEventRFBunch)
{
	Particle_t locPID = locNeutralParticleHypothesis->PID();

	double locBeta_Timing = locNeutralParticleHypothesis->measuredBeta();
	double locDeltaT = locNeutralParticleHypothesis->time() - locNeutralParticleHypothesis->t0();

	double locP = locNeutralParticleHypothesis->momentum().Mag();
	double locShowerE = locNeutralParticleHypothesis->Get_NeutralShower()->dEnergy;
	double locMatchFOM = 0.0;
	const DMCThrown* locMCThrown = (locMCThrownMatching != NULL) ? locMCThrownMatching->Get_MatchingMCThrown(locNeutralParticleHypothesis, locMatchFOM) : NULL;

	double locTimePull = 0.0;
	unsigned int locTimeNDF = 0;
	dParticleID->Calc_TimingChiSq(locNeutralParticleHypothesis, locTimeNDF, locTimePull);

	DetectorSystem_t locSystem = locNeutralParticleHypothesis->t1_detector();

	//FILL HISTOGRAMS
	//Since we are filling histograms local to this action, it will not interfere with other ROOT operations: can use action-wide ROOT lock
	//Note, the mutex is unique to this DReaction + action_string combo: actions of same class with different hists will have a different mutex
	Lock_Action();
	{
		//Beta (good for all PIDs)
		dHistMap_Beta[locPID][locSystem]->Fill(locBeta_Timing);
		dHistMap_CalE[locPID][locSystem]->Fill(locShowerE);
		if(locPID != Gamma)
		{
			Unlock_Action();
			return;
		}

		dHistMap_PIDFOM[locPID]->Fill(locNeutralParticleHypothesis->Get_FOM());
		dHistMap_BetaVsP[locPID][locSystem]->Fill(locP, locBeta_Timing);
		dHistMap_DeltaTVsP[locPID][locSystem]->Fill(locP, locDeltaT);
		dHistMap_TimePullVsP[locPID][locSystem]->Fill(locP, locTimePull);
		dHistMap_TimeFOMVsP[locPID][locSystem]->Fill(locP, locNeutralParticleHypothesis->Get_FOM());

		pair<Particle_t, Particle_t> locPIDPair(locPID, UnknownParticle); //default unless matched
		if(locMCThrown != NULL) //else bogus track (not matched to any thrown tracks)
			locPIDPair.second = (Particle_t)(locMCThrown->type); //matched
		if(dHistMap_PIDFOMForTruePID.find(locPIDPair) != dHistMap_PIDFOMForTruePID.end()) //else hist not created or PID is weird
			dHistMap_PIDFOMForTruePID[locPIDPair]->Fill(locNeutralParticleHypothesis->Get_FOM());
	}
	Unlock_Action();
}

void DHistogramAction_TrackVertexComparison::Initialize(const std::shared_ptr<const JEvent>& locEvent)
{
	string locHistName, locHistTitle, locStepROOTName, locParticleName, locParticleROOTName;
	Particle_t locPID, locHigherMassPID, locLowerMassPID;
	string locHigherMassParticleName, locLowerMassParticleName, locHigherMassParticleROOTName, locLowerMassParticleROOTName;

	Run_Update(locEvent);

	size_t locNumSteps = Get_Reaction()->Get_NumReactionSteps();
	dHistDeque_TrackZToCommon.resize(locNumSteps);
	dHistDeque_TrackTToCommon.resize(locNumSteps);
	dHistDeque_TrackDOCAToCommon.resize(locNumSteps);
	dHistDeque_MaxTrackDeltaZ.resize(locNumSteps);
	dHistDeque_MaxTrackDeltaT.resize(locNumSteps);
	dHistDeque_MaxTrackDOCA.resize(locNumSteps);
	dHistDeque_TrackDeltaTVsP.resize(locNumSteps);

	//CREATE THE HISTOGRAMS
	//Since we are creating histograms, the contents of gDirectory will be modified: must use JANA-wide ROOT lock
	DEvent::GetLockService(locEvent)->RootWriteLock(); //ACQUIRE ROOT LOCK!!
	{
		CreateAndChangeTo_ActionDirectory();
		for(size_t loc_i = 0; loc_i < locNumSteps; ++loc_i)
		{
			auto locDetectedChargedPIDs = Get_Reaction()->Get_FinalPIDs(loc_i, false, false, d_Charged, false);
			if(locDetectedChargedPIDs.empty())
				continue;

			const DReactionStep* locReactionStep = Get_Reaction()->Get_ReactionStep(loc_i);
			ostringstream locStepName;
			locStepName << "Step" << loc_i << "__" << DAnalysis::Get_StepName(locReactionStep, true, false);
			locStepROOTName = DAnalysis::Get_StepName(locReactionStep, true, true);
			CreateAndChangeTo_Directory(locStepName.str(), locStepName.str());

			// Max Track DeltaZ
			locHistName = "MaxTrackDeltaZ";
			locHistTitle = locStepROOTName + string(";Largest Track #DeltaVertex-Z (cm)");
			dHistDeque_MaxTrackDeltaZ[loc_i] = GetOrCreate_Histogram<TH1I>(locHistName, locHistTitle, dNumDeltaVertexZBins, 0.0, dMaxDeltaVertexZ);

			// Max Track DeltaT
			locHistName = "MaxTrackDeltaT";
			locHistTitle = locStepROOTName + string(";Largest Track #DeltaVertex-T (ns)");
			dHistDeque_MaxTrackDeltaT[loc_i] = GetOrCreate_Histogram<TH1I>(locHistName, locHistTitle, dNumDeltaVertexTBins, 0.0, dMaxDeltaVertexT);

			// Max Track DOCA
			locHistName = "MaxTrackDOCA";
			locHistTitle = locStepROOTName + string(";Largest Track DOCA (cm)");
			dHistDeque_MaxTrackDOCA[loc_i] = GetOrCreate_Histogram<TH1I>(locHistName, locHistTitle, dNumDOCABins, 0.0, dMaxDOCA);

			for(size_t loc_j = 0; loc_j < locDetectedChargedPIDs.size(); ++loc_j)
			{
				locPID = locDetectedChargedPIDs[loc_j];
				if(dHistDeque_TrackZToCommon[loc_i].find(locPID) != dHistDeque_TrackZToCommon[loc_i].end())
					continue; //already created for this pid
				locParticleName = ParticleType(locPID);
				locParticleROOTName = ParticleName_ROOT(locPID);

				// TrackZ To Common
				locHistName = string("TrackZToCommon_") + locParticleName;
				locHistTitle = locParticleROOTName + string(", ") + locStepROOTName + string(";#DeltaVertex-Z (Track, Common) (cm)");
				dHistDeque_TrackZToCommon[loc_i][locPID] = GetOrCreate_Histogram<TH1I>(locHistName, locHistTitle, dNumDeltaVertexZBins, dMinDeltaVertexZ, dMaxDeltaVertexZ);

				// TrackT To Common
				locHistName = string("TrackTToCommon_") + locParticleName;
				locHistTitle = locParticleROOTName + string(", ") + locStepROOTName + string(";#DeltaVertex-T (Track, Common) (ns)");
				dHistDeque_TrackTToCommon[loc_i][locPID] = GetOrCreate_Histogram<TH1I>(locHistName, locHistTitle, dNumDeltaVertexTBins, dMinDeltaVertexT, dMaxDeltaVertexT);

				// TrackDOCA To Common
				locHistName = string("TrackDOCAToCommon_") + locParticleName;
				locHistTitle = locParticleROOTName + string(", ") + locStepROOTName + string(";DOCA (Track, Common) (cm)");
				dHistDeque_TrackDOCAToCommon[loc_i][locPID] = GetOrCreate_Histogram<TH1I>(locHistName, locHistTitle, dNumDOCABins, dMinDOCA, dMaxDOCA);

				// DeltaT Vs P against beam photon
				if((loc_i == 0) && DAnalysis::Get_IsFirstStepBeam(Get_Reaction()) && (dHistMap_BeamTrackDeltaTVsP.find(locPID) == dHistMap_BeamTrackDeltaTVsP.end()))
				{
					locHistName = string("TrackDeltaTVsP_") + ParticleType(locPID) + string("_Beam") + ParticleType(locPID);
					locHistTitle = locStepROOTName + string(";") + ParticleName_ROOT(locPID) + string(" Momentum (GeV/c);t_{") + ParticleName_ROOT(locPID) + string("} - t_{Beam ") + ParticleName_ROOT(locPID) + string("} (ns)");
					dHistMap_BeamTrackDeltaTVsP[locPID] = GetOrCreate_Histogram<TH2I>(locHistName, locHistTitle, dNum2DPBins, dMinP, dMaxP, dNumDeltaVertexTBins, dMinDeltaVertexT, dMaxDeltaVertexT);
				}
			}

			//delta-t vs p
			auto locDetectedChargedPIDs_HasDupes = Get_Reaction()->Get_FinalPIDs(loc_i, false, false, d_Charged, true);
			for(int loc_j = 0; loc_j < int(locDetectedChargedPIDs_HasDupes.size()) - 1; ++loc_j)
			{
				locPID = locDetectedChargedPIDs_HasDupes[loc_j];
				for(size_t loc_k = loc_j + 1; loc_k < locDetectedChargedPIDs_HasDupes.size(); ++loc_k)
				{
					if(ParticleMass(locDetectedChargedPIDs_HasDupes[loc_k]) > ParticleMass(locPID))
					{
						locHigherMassPID = locDetectedChargedPIDs_HasDupes[loc_k];
						locLowerMassPID = locPID;
					}
					else
					{
						locHigherMassPID = locPID;
						locLowerMassPID = locDetectedChargedPIDs_HasDupes[loc_k];
					}
					pair<Particle_t, Particle_t> locParticlePair(locHigherMassPID, locLowerMassPID);
					if(dHistDeque_TrackDeltaTVsP[loc_i].find(locParticlePair) != dHistDeque_TrackDeltaTVsP[loc_i].end())
						continue; //already created for this pair

					locHigherMassParticleName = ParticleType(locHigherMassPID);
					locHigherMassParticleROOTName = ParticleName_ROOT(locHigherMassPID);
					locLowerMassParticleName = ParticleType(locLowerMassPID);
					locLowerMassParticleROOTName = ParticleName_ROOT(locLowerMassPID);

					// DeltaT Vs P
					locHistName = string("TrackDeltaTVsP_") + locHigherMassParticleName + string("_") + locLowerMassParticleName;
					locHistTitle = locStepROOTName + string(";") + locHigherMassParticleROOTName + string(" Momentum (GeV/c);t_{") + locHigherMassParticleROOTName + string("} - t_{") + locLowerMassParticleROOTName + string("} (ns)");
					dHistDeque_TrackDeltaTVsP[loc_i][locParticlePair] = GetOrCreate_Histogram<TH2I>(locHistName, locHistTitle, dNum2DPBins, dMinP, dMaxP, dNumDeltaVertexTBins, dMinDeltaVertexT, dMaxDeltaVertexT);
				}
			}
			gDirectory->cd("..");
		}
		//Return to the base directory
		ChangeTo_BaseDirectory();
	}
	DEvent::GetLockService(locEvent)->RootUnLock(); //RELEASE ROOT LOCK!!
}

void DHistogramAction_TrackVertexComparison::Run_Update(const std::shared_ptr<const JEvent>& locEvent)
{
	vector<const DAnalysisUtilities*> locAnalysisUtilitiesVector;
	locEvent->Get(locAnalysisUtilitiesVector);
	dAnalysisUtilities = locAnalysisUtilitiesVector[0];
}

bool DHistogramAction_TrackVertexComparison::Perform_Action(const std::shared_ptr<const JEvent>& locEvent, const DParticleCombo* locParticleCombo)
{
	Particle_t locPID;
	double locDOCA, locDeltaZ, locDeltaT, locMaxDOCA, locMaxDeltaZ, locMaxDeltaT;

	//note: very difficult to tell when results will be duplicate: just histogram all combos
	for(size_t loc_i = 0; loc_i < locParticleCombo->Get_NumParticleComboSteps(); ++loc_i)
	{
		const DParticleComboStep* locParticleComboStep = locParticleCombo->Get_ParticleComboStep(loc_i);
		auto locParticles = locParticleComboStep->Get_FinalParticles_Measured(Get_Reaction()->Get_ReactionStep(loc_i), d_Charged);
		if(locParticles.empty())
			continue;

		//Grab/Find common vertex & time
		DVector3 locVertex = locParticleComboStep->Get_Position();
		double locVertexTime = locParticleComboStep->Get_Time();

		for(size_t loc_j = 0; loc_j < locParticles.size(); ++loc_j)
		{
			locPID = locParticles[loc_j]->PID();

			//find max's
			locMaxDOCA = -1.0;
			locMaxDeltaZ = -1.0;
			locMaxDeltaT = -1.0;
			for(size_t loc_k = loc_j + 1; loc_k < locParticles.size(); ++loc_k)
			{
				locDeltaZ = fabs(locParticles[loc_j]->position().Z() - locParticles[loc_k]->position().Z());
				if(locDeltaZ > locMaxDeltaZ)
					locMaxDeltaZ = locDeltaZ;

				locDeltaT = fabs(locParticles[loc_j]->time() - locParticles[loc_k]->time());
				if(locDeltaT > locMaxDeltaT)
					locMaxDeltaT = locDeltaT;

				locDOCA = dAnalysisUtilities->Calc_DOCA(locParticles[loc_j], locParticles[loc_k]);
				if(locDOCA > locMaxDOCA)
					locMaxDOCA = locDOCA;
			}

			//delta-t vs p
			deque<pair<const DKinematicData*, size_t> > locParticlePairs;
			size_t locHigherMassParticleIndex, locLowerMassParticleIndex;
			//minimize number of locks by keeping track of the results and saving them at the end
			deque<pair<Particle_t, Particle_t> > locPIDPairs;
			deque<double> locPs;
			deque<double> locDeltaTs;
			for(size_t loc_k = loc_j + 1; loc_k < locParticles.size(); ++loc_k)
			{
				locParticlePairs.clear();
				locParticlePairs.push_back(pair<const DKinematicData*, size_t>(locParticles[loc_j], loc_i));
				locParticlePairs.push_back(pair<const DKinematicData*, size_t>(locParticles[loc_k], loc_i));

				if(locParticles[loc_k]->mass() > locParticles[loc_j]->mass())
				{
					locHigherMassParticleIndex = loc_k;
					locLowerMassParticleIndex = loc_j;
				}
				else
				{
					locHigherMassParticleIndex = loc_j;
					locLowerMassParticleIndex = loc_k;
				}

				locDeltaTs.push_back(locParticles[locHigherMassParticleIndex]->time() - locParticles[locLowerMassParticleIndex]->time());
				locPs.push_back(locParticles[locHigherMassParticleIndex]->momentum().Mag());
				locPIDPairs.push_back(pair<Particle_t, Particle_t>(locParticles[locHigherMassParticleIndex]->PID(), locParticles[locLowerMassParticleIndex]->PID()));
			}

			const DKinematicData* locBeamParticle = ((loc_i == 0) && DAnalysis::Get_IsFirstStepBeam(Get_Reaction())) ? locParticleComboStep->Get_InitialParticle() : NULL;
			double locBeamDeltaT = (locBeamParticle != NULL) ? locParticles[loc_j]->time() - locBeamParticle->time() : numeric_limits<double>::quiet_NaN();
			locDOCA = dAnalysisUtilities->Calc_DOCAToVertex(locParticles[loc_j], locVertex);

			//do all at once to reduce #locks & amount of time within the lock
			//FILL HISTOGRAMS
			//Since we are filling histograms local to this action, it will not interfere with other ROOT operations: can use action-wide ROOT lock
			//Note, the mutex is unique to this DReaction + action_string combo: actions of same class with different hists will have a different mutex
			Lock_Action();
			{
				//comparison to common vertex/time
				dHistDeque_TrackZToCommon[loc_i][locPID]->Fill(locParticles[loc_j]->position().Z() - locVertex.Z());
				dHistDeque_TrackTToCommon[loc_i][locPID]->Fill(locParticles[loc_j]->time() - locVertexTime);
				dHistDeque_TrackDOCAToCommon[loc_i][locPID]->Fill(locDOCA);
				//hist max's
				if(locMaxDeltaZ > 0.0) //else none found (e.g. only 1 detected charged track)
				{
					dHistDeque_MaxTrackDeltaZ[loc_i]->Fill(locMaxDeltaZ);
					dHistDeque_MaxTrackDeltaT[loc_i]->Fill(locMaxDeltaT);
					dHistDeque_MaxTrackDOCA[loc_i]->Fill(locMaxDOCA);
				}
				//delta-t's
				if(locBeamParticle != NULL)
					dHistMap_BeamTrackDeltaTVsP[locPID]->Fill(locParticles[loc_j]->momentum().Mag(), locBeamDeltaT);
				for(size_t loc_k = 0; loc_k < locPIDPairs.size(); ++loc_k)
				{
					if(dHistDeque_TrackDeltaTVsP[loc_i].find(locPIDPairs[loc_k]) == dHistDeque_TrackDeltaTVsP[loc_i].end())
					{
						//pair not found: equal masses and order switched somehow //e.g. mass set differently between REST and reconstruction
						pair<Particle_t, Particle_t> locTempPIDPair(locPIDPairs[loc_k]);
						locPIDPairs[loc_k].first = locTempPIDPair.second;
						locPIDPairs[loc_k].second = locTempPIDPair.first;
					}
					dHistDeque_TrackDeltaTVsP[loc_i][locPIDPairs[loc_k]]->Fill(locPs[loc_k], locDeltaTs[loc_k]);
				}
			}
			Unlock_Action();
		} //end of particle loop
	} //end of step loop
	return true;
}

void DHistogramAction_ParticleComboKinematics::Initialize(const std::shared_ptr<const JEvent>& locEvent)
{
	if(Get_UseKinFitResultsFlag() && (Get_Reaction()->Get_KinFitType() == d_NoFit))
	{
		cout << "WARNING: REQUESTED HISTOGRAM OF KINEMATIC FIT RESULTS WHEN NO KINEMATIC FIT!!!" << endl;
		return; //no fit performed, but kinfit data requested!!
	}

	string locHistName, locHistTitle, locStepROOTName, locParticleName, locParticleROOTName;
	Particle_t locPID;
	
	Run_Update(locEvent);

	size_t locNumSteps = Get_Reaction()->Get_NumReactionSteps();
	dHistDeque_PVsTheta.resize(locNumSteps);
	dHistDeque_PhiVsTheta.resize(locNumSteps);
	dHistDeque_BetaVsP.resize(locNumSteps);
	dHistDeque_DeltaBetaVsP.resize(locNumSteps);
	dHistDeque_P.resize(locNumSteps);
	dHistDeque_Theta.resize(locNumSteps);
	dHistDeque_Phi.resize(locNumSteps);
	dHistDeque_VertexZ.resize(locNumSteps);
	dHistDeque_VertexYVsX.resize(locNumSteps);

	auto locIsFirstStepBeam = DAnalysis::Get_IsFirstStepBeam(Get_Reaction());

	//CREATE THE HISTOGRAMS
	//Since we are creating histograms, the contents of gDirectory will be modified: must use JANA-wide ROOT lock
	DEvent::GetLockService(locEvent)->RootWriteLock(); //ACQUIRE ROOT LOCK!!
	{
		CreateAndChangeTo_ActionDirectory();
		
		//beam particle
		locPID = Get_Reaction()->Get_ReactionStep(0)->Get_InitialPID();
		if(locIsFirstStepBeam)
		{
			locParticleName = string("Beam_") + ParticleType(locPID);
			CreateAndChangeTo_Directory(locParticleName, locParticleName);
			locParticleROOTName = ParticleName_ROOT(locPID);

			// Momentum
			locHistName = "Momentum";
			locHistTitle = string("Beam ") + locParticleROOTName + string(";p (GeV/c)");
			dBeamParticleHist_P = GetOrCreate_Histogram<TH1I>(locHistName, locHistTitle, dNumPBins, dMinP, dMaxP);

			// Theta
			locHistName = "Theta";
			locHistTitle = string("Beam ") + locParticleROOTName + string(";#theta#circ");
			dBeamParticleHist_Theta = GetOrCreate_Histogram<TH1I>(locHistName, locHistTitle, dNumThetaBins, dMinTheta, dMaxTheta);

			// Phi
			locHistName = "Phi";
			locHistTitle = string("Beam ") + locParticleROOTName + string(";#phi#circ");
			dBeamParticleHist_Phi = GetOrCreate_Histogram<TH1I>(locHistName, locHistTitle, dNumPhiBins, dMinPhi, dMaxPhi);

			// P Vs Theta
			locHistName = "PVsTheta";
			locHistTitle = string("Beam ") + locParticleROOTName + string(";#theta#circ;p (GeV/c)");
			dBeamParticleHist_PVsTheta = GetOrCreate_Histogram<TH2I>(locHistName, locHistTitle, dNum2DThetaBins, dMinTheta, dMaxTheta, dNum2DPBins, dMinP, dMaxP);

			// Phi Vs Theta
			locHistName = "PhiVsTheta";
			locHistTitle = string("Beam ") + locParticleROOTName + string(";#theta#circ;#phi#circ");
			dBeamParticleHist_PhiVsTheta = GetOrCreate_Histogram<TH2I>(locHistName, locHistTitle, dNum2DThetaBins, dMinTheta, dMaxTheta, dNum2DPhiBins, dMinPhi, dMaxPhi);

			// Vertex-Z
			locHistName = "VertexZ";
			locHistTitle = string("Beam ") + locParticleROOTName + string(";Vertex-Z (cm)");
			dBeamParticleHist_VertexZ = GetOrCreate_Histogram<TH1I>(locHistName, locHistTitle, dNumVertexZBins, dMinVertexZ, dMaxVertexZ);

			// Vertex-Y Vs Vertex-X
			locHistName = "VertexYVsX";
			locHistTitle = string("Beam ") + locParticleROOTName + string(";Vertex-X (cm);Vertex-Y (cm)");
			dBeamParticleHist_VertexYVsX = GetOrCreate_Histogram<TH2I>(locHistName, locHistTitle, dNumVertexXYBins, dMinVertexXY, dMaxVertexXY, dNumVertexXYBins, dMinVertexXY, dMaxVertexXY);

			// Delta-T (Beam, RF)
			locHistName = "DeltaTRF";
			locHistTitle = string("Beam ") + locParticleROOTName + string(";#Deltat_{Beam - RF} (ns)");
			dBeamParticleHist_DeltaTRF = GetOrCreate_Histogram<TH1I>(locHistName, locHistTitle, dNumDeltaTRFBins, dMinDeltaTRF, dMaxDeltaTRF);

			// Delta-T (Beam, RF) Vs Beam E
			locHistName = "DeltaTRFVsBeamE";
			locHistTitle = string("Beam ") + locParticleROOTName + string(";E (GeV);#Deltat_{Beam - RF} (ns)");
			dBeamParticleHist_DeltaTRFVsBeamE = GetOrCreate_Histogram<TH2I>(locHistName, locHistTitle, dNum2DPBins, dMinP, dMaxP, dNumDeltaTRFBins, dMinDeltaTRF, dMaxDeltaTRF);

			gDirectory->cd("..");
		}

		//Steps
		for(size_t loc_i = 0; loc_i < locNumSteps; ++loc_i)
		{
			const DReactionStep* locReactionStep = Get_Reaction()->Get_ReactionStep(loc_i);
			ostringstream locStepName;
			locStepName << "Step" << loc_i << "__" << DAnalysis::Get_StepName(locReactionStep, true, false);
			locStepROOTName = DAnalysis::Get_StepName(locReactionStep, true, true);

			Particle_t locInitialPID = locReactionStep->Get_InitialPID();
			

			//get PIDs
			vector<Particle_t> locPIDs;
			bool locLastPIDMissingFlag = false;
			if(!Get_UseKinFitResultsFlag()) //measured, ignore missing & decaying particles (ignore target anyway)
				locPIDs = Get_Reaction()->Get_FinalPIDs(loc_i, false, false, d_AllCharges, false);
			else //kinematic fit: decaying & missing particles are reconstructed
			{
				locPIDs = Get_Reaction()->Get_FinalPIDs(loc_i, false, true, d_AllCharges, false);
				if((!locIsFirstStepBeam) || (loc_i != 0)) //add decaying parent particle //skip if on beam particle!
					locPIDs.insert(locPIDs.begin(), locInitialPID);

				Particle_t locMissingPID = locReactionStep->Get_MissingPID();
				if(locMissingPID != UnknownParticle)
				{
					locPIDs.push_back(locMissingPID);
					locLastPIDMissingFlag = true;
				}
			}

			bool locDirectoryCreatedFlag = false;
			for(size_t loc_j = 0; loc_j < locPIDs.size(); ++loc_j)
			{
				locPID = locPIDs[loc_j];
				bool locIsMissingFlag = false;
				if((loc_j == locPIDs.size() - 1) && locLastPIDMissingFlag)
				{
					locParticleName = string("(") + ParticleType(locPID) + string(")");
					locParticleROOTName = string("(") + ParticleName_ROOT(locPID) + string(")");
					locIsMissingFlag = true;
				}
				else
				{
					locParticleName = ParticleType(locPID);
					locParticleROOTName = ParticleName_ROOT(locPID);
					if(dHistDeque_P[loc_i].find(locPID) != dHistDeque_P[loc_i].end())
						continue; //pid already done
				}

				if(!locDirectoryCreatedFlag)
				{
					CreateAndChangeTo_Directory(locStepName.str(), locStepName.str());
					locDirectoryCreatedFlag = true;
				}

				CreateAndChangeTo_Directory(locParticleName, locParticleName);

				// Momentum
				locHistName = "Momentum";
				locHistTitle = locParticleROOTName + string(", ") + locStepROOTName + string(";p (GeV/c)");
				dHistDeque_P[loc_i][locPID][locIsMissingFlag] = GetOrCreate_Histogram<TH1I>(locHistName, locHistTitle, dNumPBins, dMinP, dMaxP);

				// Theta
				locHistName = "Theta";
				locHistTitle = locParticleROOTName + string(", ") + locStepROOTName + string(";#theta#circ");
				dHistDeque_Theta[loc_i][locPID][locIsMissingFlag] = GetOrCreate_Histogram<TH1I>(locHistName, locHistTitle, dNumThetaBins, dMinTheta, dMaxTheta);

				// Phi
				locHistName = "Phi";
				locHistTitle = locParticleROOTName + string(", ") + locStepROOTName + string(";#phi#circ");
				dHistDeque_Phi[loc_i][locPID][locIsMissingFlag] = GetOrCreate_Histogram<TH1I>(locHistName, locHistTitle, dNumPhiBins, dMinPhi, dMaxPhi);

				// P Vs Theta
				locHistName = "PVsTheta";
				locHistTitle = locParticleROOTName + string(", ") + locStepROOTName + string(";#theta#circ;p (GeV/c)");
				dHistDeque_PVsTheta[loc_i][locPID][locIsMissingFlag] = GetOrCreate_Histogram<TH2I>(locHistName, locHistTitle, dNum2DThetaBins, dMinTheta, dMaxTheta, dNum2DPBins, dMinP, dMaxP);

				// Phi Vs Theta
				locHistName = "PhiVsTheta";
				locHistTitle = locParticleROOTName + string(", ") + locStepROOTName + string(";#theta#circ;#phi#circ");
				dHistDeque_PhiVsTheta[loc_i][locPID][locIsMissingFlag] = GetOrCreate_Histogram<TH2I>(locHistName, locHistTitle, dNum2DThetaBins, dMinTheta, dMaxTheta, dNum2DPhiBins, dMinPhi, dMaxPhi);

				//beta vs p
				locHistName = "BetaVsP";
				locHistTitle = locParticleROOTName + string(";p (GeV/c);#beta");
				dHistDeque_BetaVsP[loc_i][locPID][locIsMissingFlag] = GetOrCreate_Histogram<TH2I>(locHistName, locHistTitle, dNum2DPBins, dMinP, dMaxP, dNumBetaBins, dMinBeta, dMaxBeta);

				//delta-beta vs p
				locHistName = "DeltaBetaVsP";
				locHistTitle = locParticleROOTName + string(";p (GeV/c);#Delta#beta");
				dHistDeque_DeltaBetaVsP[loc_i][locPID][locIsMissingFlag] = GetOrCreate_Histogram<TH2I>(locHistName, locHistTitle, dNum2DPBins, dMinP, dMaxP, dNumDeltaBetaBins, dMinDeltaBeta, dMaxDeltaBeta);

				// Vertex-Z
				locHistName = "VertexZ";
				locHistTitle = locParticleROOTName + string(", ") + locStepROOTName + string(";Vertex-Z (cm)");
				dHistDeque_VertexZ[loc_i][locPID][locIsMissingFlag] = GetOrCreate_Histogram<TH1I>(locHistName, locHistTitle, dNumVertexZBins, dMinVertexZ, dMaxVertexZ);

				// Vertex-Y Vs Vertex-X
				locHistName = "VertexYVsX";
				locHistTitle = locParticleROOTName + string(", ") + locStepROOTName + string(";Vertex-X (cm);Vertex-Y (cm)");
				dHistDeque_VertexYVsX[loc_i][locPID][locIsMissingFlag] = GetOrCreate_Histogram<TH2I>(locHistName, locHistTitle, dNumVertexXYBins, dMinVertexXY, dMaxVertexXY, dNumVertexXYBins, dMinVertexXY, dMaxVertexXY);

				gDirectory->cd("..");
			} //end of particle loop

			// Vertex
			string locInitParticleROOTName = ParticleName_ROOT(locInitialPID);
			string locInitParticleName = ParticleType(locInitialPID);
			if((loc_i == 0) || IsDetachedVertex(locInitialPID))
			{
				if(!locDirectoryCreatedFlag)
				{
					CreateAndChangeTo_Directory(locStepName.str(), locStepName.str());
					locDirectoryCreatedFlag = true;
				}

				locHistName = "StepVertexZ";
				locHistTitle = (loc_i == 0) ? ";Production Vertex-Z (cm)" : string(";") + locInitParticleROOTName + string(" Decay Vertex-Z (cm)");
				dHistMap_StepVertexZ[loc_i] = GetOrCreate_Histogram<TH1I>(locHistName, locHistTitle, dNumVertexZBins, dMinVertexZ, dMaxVertexZ);

				locHistName = "StepVertexYVsX";
				locHistTitle = (loc_i == 0) ? "Production Vertex" : locInitParticleROOTName + string(" Decay Vertex");
				locHistTitle += string(";Vertex-X (cm);Vertex-Y (cm)");
				dHistMap_StepVertexYVsX[loc_i] = GetOrCreate_Histogram<TH2I>(locHistName, locHistTitle, dNumVertexXYBins, dMinVertexXY, dMaxVertexXY, dNumVertexXYBins, dMinVertexXY, dMaxVertexXY);
			}

			if((loc_i != 0) && IsDetachedVertex(locInitialPID))
			{
				if(!locDirectoryCreatedFlag)
				{
					CreateAndChangeTo_Directory(locStepName.str(), locStepName.str());
					locDirectoryCreatedFlag = true;
				}

				locHistName = locInitParticleName + string("PathLength");
				locHistTitle = string(";") + locInitParticleROOTName + string(" Path Length (cm)");
				dHistMap_DetachedPathLength[loc_i] = GetOrCreate_Histogram<TH1I>(locHistName, locHistTitle, dNumPathLengthBins, 0.0, dMaxPathLength);

				locHistName = locInitParticleName + string("Lifetime");
				locHistTitle = string(";") + locInitParticleROOTName + string(" Lifetime (ns)");
				dHistMap_DetachedLifetime[loc_i] = GetOrCreate_Histogram<TH1I>(locHistName, locHistTitle, dNumLifetimeBins, 0.0, dMaxLifetime);

				if(Get_UseKinFitResultsFlag())
				{
					locHistName = locInitParticleName + string("Lifetime_RestFrame");
					locHistTitle = string(";") + locInitParticleROOTName + string(" Rest Frame Lifetime (ns)");
					dHistMap_DetachedLifetime_RestFrame[loc_i] = GetOrCreate_Histogram<TH1I>(locHistName, locHistTitle, dNumLifetimeBins, 0.0, dMaxLifetime);
				}
			}

			if(locDirectoryCreatedFlag)
				gDirectory->cd("..");
		} //end of step loop

		//Return to the base directory
		ChangeTo_BaseDirectory();
	}
	DEvent::GetLockService(locEvent)->RootUnLock(); //RELEASE ROOT LOCK!!
}

void DHistogramAction_ParticleComboKinematics::Run_Update(const std::shared_ptr<const JEvent>& locEvent)
{
	vector<const DParticleID*> locParticleIDs;
	locEvent->Get(locParticleIDs);

	vector<const DAnalysisUtilities*> locAnalysisUtilitiesVector;
	locEvent->Get(locAnalysisUtilitiesVector);
	
	DGeometry *locGeometry = DEvent::GetDGeometry(locEvent);
	locGeometry->GetTargetZ(dTargetZCenter);

	dParticleID = locParticleIDs[0];
	dAnalysisUtilities = locAnalysisUtilitiesVector[0];
}

bool DHistogramAction_ParticleComboKinematics::Perform_Action(const std::shared_ptr<const JEvent>& locEvent, const DParticleCombo* locParticleCombo)
{
	if(Get_UseKinFitResultsFlag() && (Get_Reaction()->Get_KinFitType() == d_NoFit))
	{
		cout << "WARNING: REQUESTED HISTOGRAM OF KINEMATIC FIT RESULTS WHEN NO KINEMATIC FIT!!! Skipping histogram." << endl;
		return true; //no fit performed, but kinfit data requested!!
	}

	const DEventRFBunch* locEventRFBunch = locParticleCombo->Get_EventRFBunch();
	auto locIsFirstStepBeam = DAnalysis::Get_IsFirstStepBeam(Get_Reaction());

	const DKinematicData* locKinematicData;
	for(size_t loc_i = 0; loc_i < locParticleCombo->Get_NumParticleComboSteps(); ++loc_i)
	{
		const DParticleComboStep* locParticleComboStep = locParticleCombo->Get_ParticleComboStep(loc_i);
		auto locReactionStep = Get_Reaction()->Get_ReactionStep(loc_i);

		//initial particle
		Particle_t locInitialPID = locReactionStep->Get_InitialPID();
		if(Get_UseKinFitResultsFlag())
			locKinematicData = locParticleComboStep->Get_InitialParticle();
		else
			locKinematicData = locParticleComboStep->Get_InitialParticle_Measured();
		if(locKinematicData != NULL)
		{
			if(locIsFirstStepBeam && (loc_i == 0))
			{
				//check if will be duplicate
				const JObject* locSourceObject = locParticleComboStep->Get_InitialParticle_Measured();
				if(dPreviouslyHistogrammedBeamParticles.find(locSourceObject) == dPreviouslyHistogrammedBeamParticles.end())
				{
					dPreviouslyHistogrammedBeamParticles.insert(locSourceObject);
					Fill_BeamHists(locKinematicData, locEventRFBunch);
				}
			}
			else if(Get_UseKinFitResultsFlag()) //decaying particle, but kinfit so can hist
				Fill_Hists(locEvent, locKinematicData, false, loc_i); //has many source object, and is unique to this combo: no dupes to check against: let it ride
		}

		//VERTEX INFORMATION
		//other than first, skipped if not detached vertex
		DLorentzVector locStepSpacetimeVertex = locParticleComboStep->Get_SpacetimeVertex();
		if((loc_i == 0) || IsDetachedVertex(locInitialPID))
		{
			dHistMap_StepVertexZ[loc_i]->Fill(locStepSpacetimeVertex.Z());
			dHistMap_StepVertexYVsX[loc_i]->Fill(locStepSpacetimeVertex.X(), locStepSpacetimeVertex.Y());
		}

		//DETACHED VERTEX INFORMATION
		if((loc_i != 0) && IsDetachedVertex(locInitialPID))
		{
			int locFromStepIndex = DAnalysis::Get_InitialParticleDecayFromIndices(Get_Reaction(), loc_i).first;
			DLorentzVector locFromSpacetimeVertex = locParticleCombo->Get_ParticleComboStep(locFromStepIndex)->Get_SpacetimeVertex();
			DLorentzVector locDeltaSpacetime = locStepSpacetimeVertex - locFromSpacetimeVertex;

			double locPathLength = locDeltaSpacetime.Vect().Mag();
			dHistMap_DetachedPathLength[loc_i]->Fill(locPathLength);
			dHistMap_DetachedLifetime[loc_i]->Fill(locDeltaSpacetime.T());

			if(locKinematicData != NULL)
			{
				DLorentzVector locInitialP4 = locKinematicData->lorentzMomentum();
				//below, t, x, and tau are really delta-t, delta-x, and delta-tau
				//tau = gamma*(t - beta*x/c)  //plug in: t = x/(beta*c), gamma = 1/sqrt(1 - beta^2)
				//tau = (x/(beta*c) - beta*x/c)/sqrt(1 - beta^2)  //plug in beta = p*c/E
				//tau = (x*E/(p*c^2) - p*x/E)/sqrt(1 - p^2*c^2/E^2)  //multiply num & denom by E*P, factor x/c^2 out of num
				//tau = (x/c^2)*(E^2 - p^2*c^2)/(p*sqrt(E^2 - p^2*c^2))  //plug in m^2*c^4 = E^2 - p^2*c^2
				//tau = (x/c^2)*m^2*c^4/(p*m*c^2)  //cancel c's & m's
				//tau = x*m/p
				//however, in data, p & m are in units with c = 1, so need an extra 1/c: tau = x*m/(c*p)
				double locRestFrameLifetime = locPathLength*ParticleMass(locInitialPID)/(29.9792458*locInitialP4.P()); //tau
				dHistMap_DetachedLifetime_RestFrame[loc_i]->Fill(locRestFrameLifetime);
				//note that tau = hbar / Gamma, hbar = 6.582119E-22 MeV*s, Gamma = Resonance FWHM
			}
		}

		//final particles
		for(size_t loc_j = 0; loc_j < locParticleComboStep->Get_NumFinalParticles(); ++loc_j)
		{
			if(Get_UseKinFitResultsFlag())
				locKinematicData = locParticleComboStep->Get_FinalParticle(loc_j);
			else
				locKinematicData = locParticleComboStep->Get_FinalParticle_Measured(loc_j);
			if(locKinematicData == NULL)
				continue; //e.g. a decaying or missing particle whose params aren't set yet

			//check if duplicate
			const JObject* locSourceObject = Get_FinalParticle_SourceObject(locKinematicData);
			if(locSourceObject != NULL) //else is reconstructed missing/decaying particle: has many source object, and is unique to this combo: no dupes to check against: let it ride
			{
				pair<Particle_t, const JObject*> locParticleInfo(locKinematicData->PID(), locSourceObject);
				pair<size_t, pair<Particle_t, const JObject*> > locHistInfo(loc_i, locParticleInfo);
				if(dPreviouslyHistogrammedParticles.find(locHistInfo) != dPreviouslyHistogrammedParticles.end())
					continue; //previously histogrammed
				dPreviouslyHistogrammedParticles.insert(locHistInfo);
			}

			bool locIsMissingFlag = (Get_Reaction()->Get_ReactionStep(loc_i)->Get_MissingParticleIndex() == int(loc_j));
			Fill_Hists(locEvent, locKinematicData, locIsMissingFlag, loc_i);
		} //end of particle loop
	} //end of step loop
	return true;
}

void DHistogramAction_ParticleComboKinematics::Fill_Hists(const std::shared_ptr<const JEvent>& locEvent, const DKinematicData* locKinematicData, bool locIsMissingFlag, size_t locStepIndex)
{
	Particle_t locPID = locKinematicData->PID();
	DVector3 locMomentum = locKinematicData->momentum();
	DVector3 locPosition = locKinematicData->position();

	double locPhi = locMomentum.Phi()*180.0/TMath::Pi();
	double locTheta = locMomentum.Theta()*180.0/TMath::Pi();
	double locP = locMomentum.Mag();

	double locBeta_Timing = 0.0;
	auto locNeutralHypo = dynamic_cast<const DNeutralParticleHypothesis*>(locKinematicData);
	if(locNeutralHypo != nullptr)
		locBeta_Timing = locNeutralHypo->measuredBeta();
	else
	{
		auto locChargedHypo = dynamic_cast<const DChargedTrackHypothesis*>(locKinematicData);
		if(locChargedHypo != nullptr)
			locBeta_Timing = locChargedHypo->measuredBeta();
	}
	double locDeltaBeta = locBeta_Timing - locKinematicData->lorentzMomentum().Beta();

	//FILL HISTOGRAMS
	//Since we are filling histograms local to this action, it will not interfere with other ROOT operations: can use action-wide ROOT lock
	//Note, the mutex is unique to this DReaction + action_string combo: actions of same class with different hists will have a different mutex
	Lock_Action();
	{
		dHistDeque_P[locStepIndex][locPID][locIsMissingFlag]->Fill(locP);
		dHistDeque_Phi[locStepIndex][locPID][locIsMissingFlag]->Fill(locPhi);
		dHistDeque_Theta[locStepIndex][locPID][locIsMissingFlag]->Fill(locTheta);
		dHistDeque_PVsTheta[locStepIndex][locPID][locIsMissingFlag]->Fill(locTheta, locP);
		dHistDeque_PhiVsTheta[locStepIndex][locPID][locIsMissingFlag]->Fill(locTheta, locPhi);
		dHistDeque_BetaVsP[locStepIndex][locPID][locIsMissingFlag]->Fill(locP, locBeta_Timing);
		dHistDeque_DeltaBetaVsP[locStepIndex][locPID][locIsMissingFlag]->Fill(locP, locDeltaBeta);
		dHistDeque_VertexZ[locStepIndex][locPID][locIsMissingFlag]->Fill(locKinematicData->position().Z());
		dHistDeque_VertexYVsX[locStepIndex][locPID][locIsMissingFlag]->Fill(locKinematicData->position().X(), locKinematicData->position().Y());
	}
	Unlock_Action();
}

void DHistogramAction_ParticleComboKinematics::Fill_BeamHists(const DKinematicData* locKinematicData, const DEventRFBunch* locEventRFBunch)
{
	DVector3 locMomentum = locKinematicData->momentum();
	double locPhi = locMomentum.Phi()*180.0/TMath::Pi();
	double locTheta = locMomentum.Theta()*180.0/TMath::Pi();
	double locP = locMomentum.Mag();
	double locDeltaTRF = locKinematicData->time() - (locEventRFBunch->dTime + (locKinematicData->z() - dTargetZCenter)/29.9792458);

	//FILL HISTOGRAMS
	//Since we are filling histograms local to this action, it will not interfere with other ROOT operations: can use action-wide ROOT lock
	//Note, the mutex is unique to this DReaction + action_string combo: actions of same class with different hists will have a different mutex
	Lock_Action();
	{
		dBeamParticleHist_P->Fill(locP);
		dBeamParticleHist_Phi->Fill(locPhi);
		dBeamParticleHist_Theta->Fill(locTheta);
		dBeamParticleHist_PVsTheta->Fill(locTheta, locP);
		dBeamParticleHist_PhiVsTheta->Fill(locTheta, locPhi);
		dBeamParticleHist_VertexZ->Fill(locKinematicData->position().Z());
		dBeamParticleHist_VertexYVsX->Fill(locKinematicData->position().X(), locKinematicData->position().Y());
		dBeamParticleHist_DeltaTRF->Fill(locDeltaTRF);
		dBeamParticleHist_DeltaTRFVsBeamE->Fill(locKinematicData->energy(), locDeltaTRF);
	}
	Unlock_Action();
}

void DHistogramAction_InvariantMass::Initialize(const std::shared_ptr<const JEvent>& locEvent)
{
	string locHistName, locHistTitle;
	double locMassPerBin = 1000.0*(dMaxMass - dMinMass)/((double)dNumMassBins);

	Run_Update(locEvent);

	string locParticleNamesForHist = "";
	if(dInitialPID != UnknownParticle)
	{
		auto locChainPIDs = DAnalysis::Get_ChainPIDs(Get_Reaction(), dInitialPID, !Get_UseKinFitResultsFlag(), true);
		locParticleNamesForHist = DAnalysis::Convert_PIDsToROOTName(locChainPIDs);
	}
	else
	{
		for(size_t loc_i = 0; loc_i < dToIncludePIDs.size(); ++loc_i)
			locParticleNamesForHist += ParticleName_ROOT(dToIncludePIDs[loc_i]);
	}

	bool locBeamPresent = DAnalysis::Get_IsFirstStepBeam(Get_Reaction());

	//CREATE THE HISTOGRAMS
	//Since we are creating histograms, the contents of gDirectory will be modified: must use JANA-wide ROOT lock
	DEvent::GetLockService(locEvent)->RootWriteLock(); //ACQUIRE ROOT LOCK!!
	{
		CreateAndChangeTo_ActionDirectory();

		locHistName = "InvariantMass";
		ostringstream locStream;
		locStream << locMassPerBin;
		locHistTitle = string(";") + locParticleNamesForHist + string(" Invariant Mass (GeV/c^{2});# Combos / ") + locStream.str() + string(" MeV/c^{2}");
		dHist_InvariantMass = GetOrCreate_Histogram<TH1I>(locHistName, locHistTitle, dNumMassBins, dMinMass, dMaxMass);

		if(locBeamPresent)
		{
			locHistName = "InvariantMassVsBeamE";
			locHistTitle = string(";Beam Energy (GeV);") + locParticleNamesForHist + string(" Invariant Mass (GeV/c^{2})");
			dHist_InvariantMassVsBeamE = GetOrCreate_Histogram<TH2D>(locHistName, locHistTitle, dNum2DBeamEBins, dMinBeamE, dMaxBeamE, dNum2DMassBins, dMinMass, dMaxMass);
		}

		//Return to the base directory
		ChangeTo_BaseDirectory();
	}
	DEvent::GetLockService(locEvent)->RootUnLock(); //RELEASE ROOT LOCK!!
}

void DHistogramAction_InvariantMass::Run_Update(const std::shared_ptr<const JEvent>& locEvent)
{
	DGeometry *locGeometry = DEvent::GetDGeometry(locEvent);
	locGeometry->GetTargetZ(dTargetZCenter);

	locEvent->GetSingle(dAnalysisUtilities);

	//BEAM BUNCH PERIOD
	vector<double> locBeamPeriodVector;
	DEvent::GetCalib(locEvent, "PHOTON_BEAM/RF/beam_period", locBeamPeriodVector);
	dBeamBunchPeriod = locBeamPeriodVector[0];
}

bool DHistogramAction_InvariantMass::Perform_Action(const std::shared_ptr<const JEvent>& locEvent, const DParticleCombo* locParticleCombo)
{
	auto locFirstStep = locParticleCombo->Get_ParticleComboStep(0);
	bool locBeamPresent = DAnalysis::Get_IsFirstStepBeam(Get_Reaction());
	auto locBeam = Get_UseKinFitResultsFlag() ? locFirstStep->Get_InitialParticle() : locFirstStep->Get_InitialParticle_Measured();

	vector<double> locMassesToFill;
	vector<double> loc2DMassesToFill;	
	for(size_t loc_i = 0; loc_i < locParticleCombo->Get_NumParticleComboSteps(); ++loc_i)
	{
		const DReactionStep* locReactionStep = Get_Reaction()->Get_ReactionStep(loc_i);
		if((dInitialPID != UnknownParticle) && (locReactionStep->Get_InitialPID() != dInitialPID))
			continue;
		if((dStepIndex != -1) && (int(loc_i) != dStepIndex))
			continue;

		//build all possible combinations of the included pids
		set<set<size_t> > locIndexCombos = dAnalysisUtilities->Build_IndexCombos(locReactionStep, dToIncludePIDs);

		//loop over them
		set<set<size_t> >::iterator locComboIterator = locIndexCombos.begin();
		for(; locComboIterator != locIndexCombos.end(); ++locComboIterator)
		{
			set<pair<const JObject*, unsigned int> > locSourceObjects;
			DLorentzVector locFinalStateP4 = dAnalysisUtilities->Calc_FinalStateP4(Get_Reaction(), locParticleCombo, loc_i, *locComboIterator, locSourceObjects, Get_UseKinFitResultsFlag());
			if(dPreviousSourceObjects.find(locSourceObjects) == dPreviousSourceObjects.end())
			{
				dPreviousSourceObjects.insert(locSourceObjects);
				locMassesToFill.push_back(locFinalStateP4.M());
			}

			auto locBeamSourceObjects = std::make_pair(locSourceObjects, locBeam);
			if(locBeamPresent && (dPreviousSourceObjects_Beam.find(locBeamSourceObjects) == dPreviousSourceObjects_Beam.end()))
			{
				dPreviousSourceObjects_Beam.insert(locBeamSourceObjects);
				loc2DMassesToFill.push_back(locFinalStateP4.M());
			}
		}
		//don't break: e.g. if multiple pi0's, histogram invariant mass of each one
	}

	// calculate accidental scaling factor (if needed)
	double locWeight = 1.;
	if(dSubtractAccidentals) {
		auto locIsFirstStepBeam = DAnalysis::Get_IsFirstStepBeam(Get_Reaction());
		if(locIsFirstStepBeam) {
			const DEventRFBunch* locEventRFBunch = locParticleCombo->Get_EventRFBunch();
			const DKinematicData* locBeamParticle = locParticleCombo->Get_ParticleComboStep(0)->Get_InitialParticle();
			
			double locDeltaTRF = locBeamParticle->time() - (locEventRFBunch->dTime + (locBeamParticle->z() - dTargetZCenter)/29.9792458);

			if(fabs(locDeltaTRF) > dBeamBunchPeriod/2.)
				locWeight = -1./(2.*Get_Reaction()->Get_NumPlusMinusRFBunches());
		}
	}

	//FILL HISTOGRAMS
	//Since we are filling histograms local to this action, it will not interfere with other ROOT operations: can use action-wide ROOT lock
	//Note, the mutex is unique to this DReaction + action_string combo: actions of same class with different hists will have a different mutex
	Lock_Action();
	{
		if(dSubtractAccidentals) {
			for(size_t loc_i = 0; loc_i < locMassesToFill.size(); ++loc_i)
				dHist_InvariantMass->Fill(locMassesToFill[loc_i], locWeight);
			for(size_t loc_i = 0; loc_i < loc2DMassesToFill.size(); ++loc_i)
				dHist_InvariantMassVsBeamE->Fill(locBeam->energy(), loc2DMassesToFill[loc_i], locWeight);
		} else {
			for(size_t loc_i = 0; loc_i < locMassesToFill.size(); ++loc_i)
				dHist_InvariantMass->Fill(locMassesToFill[loc_i]);
			for(size_t loc_i = 0; loc_i < loc2DMassesToFill.size(); ++loc_i)
				dHist_InvariantMassVsBeamE->Fill(locBeam->energy(), loc2DMassesToFill[loc_i]);
		}
	}
	Unlock_Action();

	return true;
}

void DHistogramAction_MissingMass::Initialize(const std::shared_ptr<const JEvent>& locEvent)
{
	double locMassPerBin = 1000.0*(dMaxMass - dMinMass)/((double)dNumMassBins);
	string locInitialParticlesROOTName = DAnalysis::Get_InitialParticlesName(Get_Reaction()->Get_ReactionStep(0), true);
	vector<Particle_t> locMissingMassOffOfPIDs(dMissingMassOffOfPIDs.begin(), dMissingMassOffOfPIDs.end());
	auto locChainPIDs = DAnalysis::Get_ChainPIDs(Get_Reaction(), Get_Reaction()->Get_ReactionStep(0)->Get_InitialPID(), dMissingMassOffOfStepIndex, locMissingMassOffOfPIDs, !Get_UseKinFitResultsFlag(), true);
	string locFinalParticlesROOTName = DAnalysis::Convert_PIDsToROOTName(locChainPIDs);

	Run_Update(locEvent);

	//CREATE THE HISTOGRAMS
	//Since we are creating histograms, the contents of gDirectory will be modified: must use JANA-wide ROOT lock
	DEvent::GetLockService(locEvent)->RootWriteLock(); //ACQUIRE ROOT LOCK!!
	{
		CreateAndChangeTo_ActionDirectory();

		string locHistName = "MissingMass";
		ostringstream locStream;
		locStream << locMassPerBin;
		string locHistTitle = string(";") + locInitialParticlesROOTName + string("#rightarrow") + locFinalParticlesROOTName + string(" Missing Mass (GeV/c^{2});# Combos / ") + locStream.str() + string(" MeV/c^{2}");
		dHist_MissingMass = GetOrCreate_Histogram<TH1I>(locHistName, locHistTitle, dNumMassBins, dMinMass, dMaxMass);

		locHistName = "MissingMassVsBeamE";
		locMassPerBin *= ((double)dNumMassBins)/((double)dNum2DMassBins);
		locStream.str("");
		locStream << locMassPerBin;
		locHistTitle = string(";Beam Energy (GeV);") + locInitialParticlesROOTName + string("#rightarrow") + locFinalParticlesROOTName + string(" Missing Mass (GeV/c^{2})");
		dHist_MissingMassVsBeamE = GetOrCreate_Histogram<TH2I>(locHistName, locHistTitle, dNum2DBeamEBins, dMinBeamE, dMaxBeamE, dNum2DMassBins, dMinMass, dMaxMass);

		locHistName = "MissingMassVsMissingP";
		locStream.str("");
		locStream << locMassPerBin;
		locHistTitle = string(";Missing P (GeV/c);") + locInitialParticlesROOTName + string("#rightarrow") + locFinalParticlesROOTName + string(" Missing Mass (GeV/c^{2})");
		dHist_MissingMassVsMissingP = GetOrCreate_Histogram<TH2I>(locHistName, locHistTitle, dNum2DMissPBins, dMinMissP, dMaxMissP, dNum2DMassBins, dMinMass, dMaxMass);

		//Return to the base directory
		ChangeTo_BaseDirectory();
	}
	DEvent::GetLockService(locEvent)->RootUnLock(); //RELEASE ROOT LOCK!!
}

void DHistogramAction_MissingMass::Run_Update(const std::shared_ptr<const JEvent>& locEvent)
{
	DGeometry *locGeometry = DEvent::GetDGeometry(locEvent);
	locGeometry->GetTargetZ(dTargetZCenter);

	locEvent->GetSingle(dAnalysisUtilities);

	//BEAM BUNCH PERIOD
	vector<double> locBeamPeriodVector;
	DEvent::GetCalib(locEvent, "PHOTON_BEAM/RF/beam_period", locBeamPeriodVector);
	dBeamBunchPeriod = locBeamPeriodVector[0];
}

bool DHistogramAction_MissingMass::Perform_Action(const std::shared_ptr<const JEvent>& locEvent, const DParticleCombo* locParticleCombo)
{
	double locBeamEnergy = locParticleCombo->Get_ParticleComboStep(0)->Get_InitialParticle()->energy();

	//build all possible combinations of the included pids
	set<set<size_t> > locIndexCombos = dAnalysisUtilities->Build_IndexCombos(Get_Reaction()->Get_ReactionStep(dMissingMassOffOfStepIndex), dMissingMassOffOfPIDs);

	//loop over them
	vector<pair<double, double> > locMassesToFill; //first is missing mass, second is missing p
	set<set<size_t> >::iterator locComboIterator = locIndexCombos.begin();
	for(; locComboIterator != locIndexCombos.end(); ++locComboIterator)
	{
		set<pair<const JObject*, unsigned int> > locSourceObjects;
		DLorentzVector locMissingP4 = dAnalysisUtilities->Calc_MissingP4(Get_Reaction(), locParticleCombo, 0, dMissingMassOffOfStepIndex, *locComboIterator, locSourceObjects, Get_UseKinFitResultsFlag());

		if(dPreviousSourceObjects.find(locSourceObjects) != dPreviousSourceObjects.end())
			continue; //dupe: already histed!
		dPreviousSourceObjects.insert(locSourceObjects);

		locMassesToFill.push_back(pair<double, double>(locMissingP4.M(), locMissingP4.P()));
	}
	
	// calculate accidental scaling factor (if needed)
	double locWeight = 1.;
	if(dSubtractAccidentals) {
		auto locIsFirstStepBeam = DAnalysis::Get_IsFirstStepBeam(Get_Reaction());
		if(locIsFirstStepBeam) {
			const DEventRFBunch* locEventRFBunch = locParticleCombo->Get_EventRFBunch();
			const DKinematicData* locBeamParticle = locParticleCombo->Get_ParticleComboStep(0)->Get_InitialParticle();
			
			double locDeltaTRF = locBeamParticle->time() - (locEventRFBunch->dTime + (locBeamParticle->z() - dTargetZCenter)/29.9792458);

			if(fabs(locDeltaTRF) > dBeamBunchPeriod/2.)
				locWeight = -1./(2.*Get_Reaction()->Get_NumPlusMinusRFBunches());
		}
	}

	//FILL HISTOGRAMS
	//Since we are filling histograms local to this action, it will not interfere with other ROOT operations: can use action-wide ROOT lock
	//Note, the mutex is unique to this DReaction + action_string combo: actions of same class with different hists will have a different mutex
	Lock_Action();
	{
		for(size_t loc_i = 0; loc_i < locMassesToFill.size(); ++loc_i)
		{
			// only fill histograms with weights if needed, to avoid complications
			if(dSubtractAccidentals) {
				dHist_MissingMass->Fill(locMassesToFill[loc_i].first, locWeight);
				dHist_MissingMassVsBeamE->Fill(locBeamEnergy, locMassesToFill[loc_i].first, locWeight);
				dHist_MissingMassVsMissingP->Fill(locMassesToFill[loc_i].second, locMassesToFill[loc_i].first, locWeight);
			} else {
				dHist_MissingMass->Fill(locMassesToFill[loc_i].first);
				dHist_MissingMassVsBeamE->Fill(locBeamEnergy, locMassesToFill[loc_i].first);
				dHist_MissingMassVsMissingP->Fill(locMassesToFill[loc_i].second, locMassesToFill[loc_i].first);
			}
		}
	}
	Unlock_Action();

	return true;
}

void DHistogramAction_MissingMassSquared::Initialize(const std::shared_ptr<const JEvent>& locEvent)
{
	double locMassSqPerBin = 1000.0*1000.0*(dMaxMassSq - dMinMassSq)/((double)dNumMassBins);
	string locInitialParticlesROOTName = DAnalysis::Get_InitialParticlesName(Get_Reaction()->Get_ReactionStep(0), true);
	vector<Particle_t> locMissingMassOffOfPIDs(dMissingMassOffOfPIDs.begin(), dMissingMassOffOfPIDs.end());
	auto locChainPIDs = DAnalysis::Get_ChainPIDs(Get_Reaction(), Get_Reaction()->Get_ReactionStep(0)->Get_InitialPID(), dMissingMassOffOfStepIndex, locMissingMassOffOfPIDs, !Get_UseKinFitResultsFlag(), true);
	string locFinalParticlesROOTName = DAnalysis::Convert_PIDsToROOTName(locChainPIDs);

	Run_Update(locEvent);

	//CREATE THE HISTOGRAMS
	//Since we are creating histograms, the contents of gDirectory will be modified: must use JANA-wide ROOT lock
	DEvent::GetLockService(locEvent)->RootWriteLock(); //ACQUIRE ROOT LOCK!!
	{
		CreateAndChangeTo_ActionDirectory();

		string locHistName = "MissingMassSquared";
		ostringstream locStream;
		locStream << locMassSqPerBin;
		string locHistTitle = string(";") + locInitialParticlesROOTName + string("#rightarrow") + locFinalParticlesROOTName + string(" Missing Mass Squared (GeV/c^{2})^{2};# Combos / ") + locStream.str() + string(" (MeV/c^{2})^{2}");
		dHist_MissingMassSquared = GetOrCreate_Histogram<TH1I>(locHistName, locHistTitle, dNumMassBins, dMinMassSq, dMaxMassSq);

		locHistName = "MissingMassSquaredVsBeamE";
		locMassSqPerBin *= ((double)dNumMassBins)/((double)dNum2DMassBins);
		locStream.str();
		locStream << locMassSqPerBin;
		locHistTitle = string(";Beam Energy (GeV);") + locInitialParticlesROOTName + string("#rightarrow") + locFinalParticlesROOTName + string(" Missing Mass Squared (GeV/c^{2})^{2};");
		dHist_MissingMassSquaredVsBeamE = GetOrCreate_Histogram<TH2I>(locHistName, locHistTitle, dNum2DBeamEBins, dMinBeamE, dMaxBeamE, dNum2DMassBins, dMinMassSq, dMaxMassSq);

		locHistName = "MissingMassSquaredVsMissingP";
		locStream.str("");
		locStream << locMassSqPerBin;
		locHistTitle = string(";Missing P (GeV/c);") + locInitialParticlesROOTName + string("#rightarrow") + locFinalParticlesROOTName + string(" Missing Mass Squared (GeV/c^{2})^{2}");
		dHist_MissingMassSquaredVsMissingP = GetOrCreate_Histogram<TH2I>(locHistName, locHistTitle, dNum2DMissPBins, dMinMissP, dMaxMissP, dNum2DMassBins, dMinMassSq, dMaxMassSq);

		//Return to the base directory
		ChangeTo_BaseDirectory();
	}
	DEvent::GetLockService(locEvent)->RootUnLock(); //RELEASE ROOT LOCK!!
}

void DHistogramAction_MissingMassSquared::Run_Update(const std::shared_ptr<const JEvent>& locEvent)
{
	DGeometry *locGeometry = DEvent::GetDGeometry(locEvent);
	locGeometry->GetTargetZ(dTargetZCenter);

	locEvent->GetSingle(dAnalysisUtilities);

	//BEAM BUNCH PERIOD
	vector<double> locBeamPeriodVector;
	DEvent::GetCalib(locEvent, "PHOTON_BEAM/RF/beam_period", locBeamPeriodVector);
	dBeamBunchPeriod = locBeamPeriodVector[0];
}

bool DHistogramAction_MissingMassSquared::Perform_Action(const std::shared_ptr<const JEvent>& locEvent, const DParticleCombo* locParticleCombo)
{
	double locBeamEnergy = locParticleCombo->Get_ParticleComboStep(0)->Get_InitialParticle()->energy();

	//build all possible combinations of the included pids
	set<set<size_t> > locIndexCombos = dAnalysisUtilities->Build_IndexCombos(Get_Reaction()->Get_ReactionStep(dMissingMassOffOfStepIndex), dMissingMassOffOfPIDs);

	//loop over them
	vector<pair<double, double> > locMassesToFill; //first is missing mass, second is missing p
	set<set<size_t> >::iterator locComboIterator = locIndexCombos.begin();
	for(; locComboIterator != locIndexCombos.end(); ++locComboIterator)
	{
		set<pair<const JObject*, unsigned int> > locSourceObjects;
		DLorentzVector locMissingP4 = dAnalysisUtilities->Calc_MissingP4(Get_Reaction(), locParticleCombo, 0, dMissingMassOffOfStepIndex, *locComboIterator, locSourceObjects, Get_UseKinFitResultsFlag());

		if(dPreviousSourceObjects.find(locSourceObjects) != dPreviousSourceObjects.end())
			continue; //dupe: already histed!
		dPreviousSourceObjects.insert(locSourceObjects);

		locMassesToFill.push_back(pair<double, double>(locMissingP4.M2(), locMissingP4.P()));
	}

	// calculate accidental scaling factor (if needed)
	double locWeight = 1.;
	if(dSubtractAccidentals) {
		auto locIsFirstStepBeam = DAnalysis::Get_IsFirstStepBeam(Get_Reaction());
		if(locIsFirstStepBeam) {
			const DEventRFBunch* locEventRFBunch = locParticleCombo->Get_EventRFBunch();
			const DKinematicData* locBeamParticle = locParticleCombo->Get_ParticleComboStep(0)->Get_InitialParticle();
			
			double locDeltaTRF = locBeamParticle->time() - (locEventRFBunch->dTime + (locBeamParticle->z() - dTargetZCenter)/29.9792458);

			if(fabs(locDeltaTRF) > dBeamBunchPeriod/2.)
				locWeight = -1./(2.*Get_Reaction()->Get_NumPlusMinusRFBunches());
		}
	}

	//FILL HISTOGRAMS
	//Since we are filling histograms local to this action, it will not interfere with other ROOT operations: can use action-wide ROOT lock
	//Note, the mutex is unique to this DReaction + action_string combo: actions of same class with different hists will have a different mutex
	Lock_Action();
	{
		for(size_t loc_i = 0; loc_i < locMassesToFill.size(); ++loc_i)
		{
			if(dSubtractAccidentals) {
				dHist_MissingMassSquared->Fill(locMassesToFill[loc_i].first, locWeight);
				dHist_MissingMassSquaredVsBeamE->Fill(locBeamEnergy, locMassesToFill[loc_i].first, locWeight);
				dHist_MissingMassSquaredVsMissingP->Fill(locMassesToFill[loc_i].second, locMassesToFill[loc_i].first, locWeight);
			} else {
				dHist_MissingMassSquared->Fill(locMassesToFill[loc_i].first);
				dHist_MissingMassSquaredVsBeamE->Fill(locBeamEnergy, locMassesToFill[loc_i].first);
				dHist_MissingMassSquaredVsMissingP->Fill(locMassesToFill[loc_i].second, locMassesToFill[loc_i].first);
			}
		}
	}
	Unlock_Action();

	return true;
}

void DHistogramAction_2DInvariantMass::Initialize(const std::shared_ptr<const JEvent>& locEvent)
{
	string locHistName, locHistTitle;
	
	Run_Update(locEvent);

	string locParticleNamesForHistX = "";
	for(size_t loc_i = 0; loc_i < dXPIDs.size(); ++loc_i)
		locParticleNamesForHistX += ParticleName_ROOT(dXPIDs[loc_i]);

	string locParticleNamesForHistY = "";
	for(size_t loc_i = 0; loc_i < dYPIDs.size(); ++loc_i)
		locParticleNamesForHistY += ParticleName_ROOT(dYPIDs[loc_i]);

	string locMassString = " Invariant Mass (GeV/c^{2});";

	//CREATE THE HISTOGRAMS
	//Since we are creating histograms, the contents of gDirectory will be modified: must use JANA-wide ROOT lock
	DEvent::GetLockService(locEvent)->RootWriteLock(); //ACQUIRE ROOT LOCK!!
	{
		CreateAndChangeTo_ActionDirectory();

		locHistName = "2DInvariantMass";
		locHistTitle = string(";") + locParticleNamesForHistX + locMassString + locParticleNamesForHistY + locMassString;
		dHist_2DInvaraintMass = GetOrCreate_Histogram<TH2I>(locHistName, locHistTitle, dNumXBins, dMinX, dMaxX, dNumYBins, dMinY, dMaxY);

		//Return to the base directory
		ChangeTo_BaseDirectory();
	}
	DEvent::GetLockService(locEvent)->RootUnLock(); //RELEASE ROOT LOCK!!
}

void DHistogramAction_2DInvariantMass::Run_Update(const std::shared_ptr<const JEvent>& locEvent)
{
	DGeometry *locGeometry = DEvent::GetDGeometry(locEvent);
	locGeometry->GetTargetZ(dTargetZCenter);

	locEvent->GetSingle(dAnalysisUtilities);

	//BEAM BUNCH PERIOD
	vector<double> locBeamPeriodVector;
	DEvent::GetCalib(locEvent, "PHOTON_BEAM/RF/beam_period", locBeamPeriodVector);
	dBeamBunchPeriod = locBeamPeriodVector[0];
}

bool DHistogramAction_2DInvariantMass::Perform_Action(const std::shared_ptr<const JEvent>& locEvent, const DParticleCombo* locParticleCombo)
{
	vector<pair<double, double> > locMassesToFill;
	for(size_t loc_i = 0; loc_i < locParticleCombo->Get_NumParticleComboSteps(); ++loc_i)
	  {
		const DReactionStep* locReactionStep = Get_Reaction()->Get_ReactionStep(loc_i);
		if((dStepIndex != -1) && (int(loc_i) != dStepIndex))
			continue;

		//build all possible combinations of the included pids
		set<set<size_t> > locXIndexCombos = dAnalysisUtilities->Build_IndexCombos(locReactionStep, dXPIDs);
		set<set<size_t> > locYIndexCombos = dAnalysisUtilities->Build_IndexCombos(locReactionStep, dYPIDs);

		//(double) loop over them
		set<set<size_t> >::iterator locXComboIterator = locXIndexCombos.begin();
		for(; locXComboIterator != locXIndexCombos.end(); ++locXComboIterator)
		{
		        set<pair<const JObject*, unsigned int> > locXSourceObjects;
			DLorentzVector locXP4 = dAnalysisUtilities->Calc_FinalStateP4(Get_Reaction(), locParticleCombo, loc_i, *locXComboIterator, locXSourceObjects, Get_UseKinFitResultsFlag());

			set<set<size_t> >::iterator locYComboIterator = locYIndexCombos.begin();
			for(; locYComboIterator != locYIndexCombos.end(); ++locYComboIterator)
			{
				set<pair<const JObject*, unsigned int> > locYSourceObjects;
				DLorentzVector locYP4 = dAnalysisUtilities->Calc_FinalStateP4(Get_Reaction(), locParticleCombo, loc_i, *locYComboIterator, locYSourceObjects, Get_UseKinFitResultsFlag());

				if(locXSourceObjects == locYSourceObjects)
					continue; //the same!

				set<set<pair<const JObject*, unsigned int> > > locAllSourceObjects;
				locAllSourceObjects.insert(locXSourceObjects);
				locAllSourceObjects.insert(locYSourceObjects);
				if(dPreviousSourceObjects.find(locAllSourceObjects) != dPreviousSourceObjects.end())
					continue; //dupe: already histed! (also, could be that the X/Y swapped combo has already been histed: don't double-count!
				dPreviousSourceObjects.insert(locAllSourceObjects);

				locMassesToFill.push_back(pair<double, double>(locXP4.M(), locYP4.M()));
			}
		}
	}

	// calculate accidental scaling factor (if needed)
	double locWeight = 1.;
	if(dSubtractAccidentals) {
		auto locIsFirstStepBeam = DAnalysis::Get_IsFirstStepBeam(Get_Reaction());
		if(locIsFirstStepBeam) {
			const DEventRFBunch* locEventRFBunch = locParticleCombo->Get_EventRFBunch();
			const DKinematicData* locBeamParticle = locParticleCombo->Get_ParticleComboStep(0)->Get_InitialParticle();
			
			double locDeltaTRF = locBeamParticle->time() - (locEventRFBunch->dTime + (locBeamParticle->z() - dTargetZCenter)/29.9792458);

			if(fabs(locDeltaTRF) > dBeamBunchPeriod/2.)
				locWeight = -1./(2.*Get_Reaction()->Get_NumPlusMinusRFBunches());
		}
	}

	//FILL HISTOGRAMS
	//Since we are filling histograms local to this action, it will not interfere with other ROOT operations: can use action-wide ROOT lock
	//Note, the mutex is unique to this DReaction + action_string combo: actions of same class with different hists will have a different mutex
	Lock_Action();
	{
		if(dSubtractAccidentals) {
			for(size_t loc_i = 0; loc_i < locMassesToFill.size(); ++loc_i)
				dHist_2DInvaraintMass->Fill(locMassesToFill[loc_i].first, locMassesToFill[loc_i].second, locWeight);
		} else {
			for(size_t loc_i = 0; loc_i < locMassesToFill.size(); ++loc_i)
				dHist_2DInvaraintMass->Fill(locMassesToFill[loc_i].first, locMassesToFill[loc_i].second);
		}
	}
	Unlock_Action();

	return true;
}

void DHistogramAction_Dalitz::Initialize(const std::shared_ptr<const JEvent>& locEvent)
{
	string locHistName, locHistTitle;
	
	Run_Update(locEvent);

	string locParticleNamesForHistX = "";
	for(size_t loc_i = 0; loc_i < dXPIDs.size(); ++loc_i)
		locParticleNamesForHistX += ParticleName_ROOT(dXPIDs[loc_i]);

	string locParticleNamesForHistY = "";
	for(size_t loc_i = 0; loc_i < dYPIDs.size(); ++loc_i)
		locParticleNamesForHistY += ParticleName_ROOT(dYPIDs[loc_i]);

	string locMassString = " Invariant Mass Squared (GeV/c^{2})^{2};";

	//CREATE THE HISTOGRAMS
	//Since we are creating histograms, the contents of gDirectory will be modified: must use JANA-wide ROOT lock
	DEvent::GetLockService(locEvent)->RootWriteLock(); //ACQUIRE ROOT LOCK!!
	{
		CreateAndChangeTo_ActionDirectory();

		locHistName = "DalitzPlot";
		locHistTitle = string(";") + locParticleNamesForHistX + locMassString + locParticleNamesForHistY + locMassString;
		dHist_DalitzPlot = GetOrCreate_Histogram<TH2I>(locHistName, locHistTitle, dNumXBins, dMinX, dMaxX, dNumYBins, dMinY, dMaxY);

		//Return to the base directory
		ChangeTo_BaseDirectory();
	}
	DEvent::GetLockService(locEvent)->RootUnLock(); //RELEASE ROOT LOCK!!
}

void DHistogramAction_Dalitz::Run_Update(const std::shared_ptr<const JEvent>& locEvent)
{
	locEvent->GetSingle(dAnalysisUtilities);
}

bool DHistogramAction_Dalitz::Perform_Action(const std::shared_ptr<const JEvent>& locEvent, const DParticleCombo* locParticleCombo)
{
	vector<pair<double, double> > locMassesToFill;
	for(size_t loc_i = 0; loc_i < locParticleCombo->Get_NumParticleComboSteps(); ++loc_i)
	{
		const DReactionStep* locReactionStep = Get_Reaction()->Get_ReactionStep(loc_i);
		if((dStepIndex != -1) && (int(loc_i) != dStepIndex))
			continue;

		//build all possible combinations of the included pids
		set<set<size_t> > locXIndexCombos = dAnalysisUtilities->Build_IndexCombos(locReactionStep, dXPIDs);
		set<set<size_t> > locYIndexCombos = dAnalysisUtilities->Build_IndexCombos(locReactionStep, dYPIDs);

		//(double) loop over them
		set<set<size_t> >::iterator locXComboIterator = locXIndexCombos.begin();
		for(; locXComboIterator != locXIndexCombos.end(); ++locXComboIterator)
		{
			set<pair<const JObject*, unsigned int> > locXSourceObjects;
			DLorentzVector locXP4 = dAnalysisUtilities->Calc_FinalStateP4(Get_Reaction(), locParticleCombo, loc_i, *locXComboIterator, locXSourceObjects, Get_UseKinFitResultsFlag());

			set<set<size_t> >::iterator locYComboIterator = locYIndexCombos.begin();
			for(; locYComboIterator != locYIndexCombos.end(); ++locYComboIterator)
			{
				set<pair<const JObject*, unsigned int> > locYSourceObjects;
				DLorentzVector locYP4 = dAnalysisUtilities->Calc_FinalStateP4(Get_Reaction(), locParticleCombo, loc_i, *locYComboIterator, locYSourceObjects, Get_UseKinFitResultsFlag());

				if(locXSourceObjects == locYSourceObjects)
					continue; //the same!

				set<set<pair<const JObject*, unsigned int> > > locAllSourceObjects;
				locAllSourceObjects.insert(locXSourceObjects);
				locAllSourceObjects.insert(locYSourceObjects);
				if(dPreviousSourceObjects.find(locAllSourceObjects) != dPreviousSourceObjects.end())
					continue; //dupe: already histed! (also, could be that the X/Y swapped combo has already been histed: don't double-count!
				dPreviousSourceObjects.insert(locAllSourceObjects);

				locMassesToFill.push_back(pair<double, double>(locXP4.M2(), locYP4.M2()));
			}
		}
	}

	//FILL HISTOGRAMS
	//Since we are filling histograms local to this action, it will not interfere with other ROOT operations: can use action-wide ROOT lock
	//Note, the mutex is unique to this DReaction + action_string combo: actions of same class with different hists will have a different mutex
	Lock_Action();
	{
		for(size_t loc_i = 0; loc_i < locMassesToFill.size(); ++loc_i)
			dHist_DalitzPlot->Fill(locMassesToFill[loc_i].first, locMassesToFill[loc_i].second);
	}
	Unlock_Action();

	return true;
}

void DHistogramAction_KinFitResults::Initialize(const std::shared_ptr<const JEvent>& locEvent)
{
        auto params = japp->GetJParameterManager();
        if (params->Exists("KINFIT:DEPENDENCE_HISTS")){
                bool locHistDependenceFlag = false;
                params->SetDefaultParameter("KINFIT:DEPENDENCE_HISTS", locHistDependenceFlag);
                params->GetParameter("KINFIT:DEPENDENCE_HISTS", locHistDependenceFlag);
                dHistDependenceFlag = locHistDependenceFlag;
	}

	auto locReaction = Get_Reaction();
	DKinFitType locKinFitType = locReaction->Get_KinFitType();
	if(locKinFitType == d_NoFit)
		return;

	//Get the DReactionVertexInfo for this reaction
	vector<const DReactionVertexInfo*> locReactionVertexInfos;
	locEvent->Get(locReactionVertexInfos);
	const DReactionVertexInfo* locReactionVertexInfo = nullptr;
	for(auto& locVertexInfo : locReactionVertexInfos)
	{
		auto locReactions = locVertexInfo->Get_Reactions();
		std::sort(locReactions.begin(), locReactions.end());
		if(!std::binary_search(locReactions.begin(), locReactions.end(), Get_Reaction()))
			continue;
		locReactionVertexInfo = locVertexInfo;
		break;
	}

	dKinFitUtils = new DKinFitUtils_GlueX(locEvent);
	Run_Update(locEvent);

	size_t locNumConstraints = 0, locNumUnknowns = 0;
	string locConstraintString = dKinFitUtils->Get_ConstraintInfo(locReactionVertexInfo, Get_Reaction(), locNumConstraints, locNumUnknowns);

	size_t locNDF = locNumConstraints - locNumUnknowns;
	bool locIncludeBeamlineInVertexFitFlag = dKinFitUtils->Get_IncludeBeamlineInVertexFitFlag();

	bool locP4IsFit = ((locKinFitType != d_VertexFit) && (locKinFitType != d_SpacetimeFit));
	bool locSpactimeIsFitFlag = (locKinFitType == d_SpacetimeFit) || (locKinFitType == d_P4AndSpacetimeFit);
	bool locVertexIsFitFlag = (locSpactimeIsFitFlag || (locKinFitType == d_VertexFit) || (locKinFitType == d_P4AndVertexFit));

	//bool locIsInclusiveChannelFlag = Get_Reaction()->Get_IsInclusiveChannelFlag();
	//Below, should in theory check on whether to create pxyz pull histograms in the inclusive channel case
	//But, this is tricky: can have inclusive (no p4) but still have mass constraints (p4)

	//CREATE THE HISTOGRAMS
	//Since we are creating histograms, the contents of gDirectory will be modified: must use JANA-wide ROOT lock
	DEvent::GetLockService(locEvent)->RootWriteLock(); //ACQUIRE ROOT LOCK!!
	{
		CreateAndChangeTo_ActionDirectory();

		// Confidence Level
		string locHistName = "ConfidenceLevel";
		ostringstream locHistTitle;
		locHistTitle << "Kinematic Fit Constraints: " << locConstraintString << ";Confidence Level (" << locNumConstraints;
		locHistTitle << " Constraints, " << locNumUnknowns << " Unknowns: " << locNDF << "-C Fit);# Combos";
		dHist_ConfidenceLevel = GetOrCreate_Histogram<TH1I>(locHistName, locHistTitle.str(), dNumConfidenceLevelBins, 0.0, 1.0);

		// Reduced chi^2 (chi^2/# d.o.f.)
		locHistName = "ChiSq";
		locHistTitle.str("");   // clear the ostringstream
		locHistTitle << "Kinematic Fit Constraints: " << locConstraintString << "; #chi^{2}/# d.o.f. (" << locNumConstraints;
		locHistTitle << " Constraints, " << locNumUnknowns << " Unknowns: " << locNDF << "-C Fit);# Combos";
		dHist_ChiSq = GetOrCreate_Histogram<TH1I>(locHistName, locHistTitle.str(), dNumChiSqBins, 0.0, dMaxChiSq);

		//beam (pulls & conlev)
		Particle_t locInitialPID = Get_Reaction()->Get_ReactionStep(0)->Get_InitialPID();
		bool locBeamFlag = Get_IsFirstStepBeam(Get_Reaction());
		if(locBeamFlag)
		{
			auto locStepVertexInfo = locReactionVertexInfo->Get_StepVertexInfo(0);
			auto locFullConstrainParticles = locStepVertexInfo->Get_FullConstrainParticles(true, d_InitialState, d_AllCharges, false);
			bool locIsInVertexFitFlag = locVertexIsFitFlag && locIncludeBeamlineInVertexFitFlag && !locFullConstrainParticles.empty();
			bool locIsChargedFlag = (ParticleCharge(locInitialPID) != 0);

			if(locP4IsFit || locIsInVertexFitFlag)
			{
				string locFullROOTName = string("Beam ") + ParticleName_ROOT(locInitialPID);

				if(dHistDependenceFlag)
				{
					//Conlev
					locHistName = "ConfidenceLevel_VsBeamEnergy";
					locHistTitle.str("");
					locHistTitle << "Kinematic Fit Constraints: " << locConstraintString << ";Beam Energy (GeV);Confidence Level (" << locNumConstraints;
					locHistTitle << " Constraints, " << locNumUnknowns << " Unknowns: " << locNDF << "-C Fit)";
					dHistMap_ConfidenceLevel_VsP[pair<int, Particle_t>(-1, locInitialPID)] = GetOrCreate_Histogram<TH2I>(locHistName, locHistTitle.str(), dNum2DBeamEBins, dMinBeamE, dMaxBeamE, dNum2DConfidenceLevelBins, 0.0, 1.0);
				}

				//Pulls
				CreateAndChangeTo_Directory("Beam", "Beam");
				map<DKinFitPullType, TH1I*> locParticlePulls;
				map<DKinFitPullType, TH2I*> locParticlePullsVsP, locParticlePullsVsTheta, locParticlePullsVsPhi;
				Create_ParticlePulls(locFullROOTName, locIsChargedFlag, locIsInVertexFitFlag, false, -1, locInitialPID);

				gDirectory->cd("..");
			}
		}

		//final particles
		for(size_t loc_i = 0; loc_i < Get_Reaction()->Get_NumReactionSteps(); ++loc_i)
		{
			const DReactionStep* locReactionStep = Get_Reaction()->Get_ReactionStep(loc_i);
			auto locStepVertexInfo = locReactionVertexInfo->Get_StepVertexInfo(loc_i);
			auto locFullConstrainParticles = locStepVertexInfo->Get_FullConstrainParticles(true, d_FinalState, d_AllCharges, false);
			ostringstream locStepName;
			locStepName << "Step" << loc_i << "__" << DAnalysis::Get_StepName(locReactionStep, true, false);
			string locStepROOTName = DAnalysis::Get_StepName(locReactionStep, true, true);

			//Conlev
			auto locPIDs = Get_Reaction()->Get_FinalPIDs(loc_i, false, false, d_AllCharges, false);
			for(auto locPID : locPIDs)
			{
				bool locIsInVertexFitFlag = locVertexIsFitFlag && locStepVertexInfo->Get_FittableVertexFlag();

				bool locIsNeutralShowerFlag = (locIsInVertexFitFlag && (ParticleCharge(locPID) == 0));
				if((ParticleMass(locPID) > 0.0) && !locSpactimeIsFitFlag)
					locIsNeutralShowerFlag = false; //massive shower momentum is defined by t, which isn't fit: use particle
				if(!locP4IsFit && !locIsInVertexFitFlag)
					continue; //p4 is not fit, and this is not in a vertex fit: no pulls
				if(locIsNeutralShowerFlag && !locP4IsFit)
					continue; //vertex-only fit: neutral shower does not constrain: no pulls

				string locParticleName = ParticleType(locPID);
				string locParticleROOTName = ParticleName_ROOT(locPID);

				if(dHistDependenceFlag)
				{
					//vs p
					locHistName = string("ConfidenceLevel_Vs") + locParticleName + string("P");
					locHistTitle.str("");
					locHistTitle << "Kinematic Fit Constraints: " << locConstraintString << ";" << locParticleROOTName << " p (GeV/c);Confidence Level (" << locNumConstraints;
					locHistTitle << " Constraints, " << locNumUnknowns << " Unknowns: " << locNDF << "-C Fit)";
					dHistMap_ConfidenceLevel_VsP[pair<int, Particle_t>(loc_i, locPID)] = GetOrCreate_Histogram<TH2I>(locHistName, locHistTitle.str(), dNum2DPBins, dMinP, dMaxP, dNum2DConfidenceLevelBins, 0.0, 1.0);

					//vs theta
					locHistName = string("ConfidenceLevel_Vs") + locParticleName + string("Theta");
					locHistTitle.str("");
					locHistTitle << "Kinematic Fit Constraints: " << locConstraintString << ";" << locParticleROOTName << " #theta#circ;Confidence Level (" << locNumConstraints;
					locHistTitle << " Constraints, " << locNumUnknowns << " Unknowns: " << locNDF << "-C Fit)";
					dHistMap_ConfidenceLevel_VsTheta[pair<int, Particle_t>(loc_i, locPID)] = GetOrCreate_Histogram<TH2I>(locHistName, locHistTitle.str(), dNum2DThetaBins, dMinTheta, dMaxTheta, dNum2DConfidenceLevelBins, 0.0, 1.0);

					//vs phi
					locHistName = string("ConfidenceLevel_Vs") + locParticleName + string("Phi");
					locHistTitle.str("");
					locHistTitle << "Kinematic Fit Constraints: " << locConstraintString << ";" << locParticleROOTName << " #phi#circ;Confidence Level (" << locNumConstraints;
					locHistTitle << " Constraints, " << locNumUnknowns << " Unknowns: " << locNDF << "-C Fit)";
					dHistMap_ConfidenceLevel_VsPhi[pair<int, Particle_t>(loc_i, locPID)] = GetOrCreate_Histogram<TH2I>(locHistName, locHistTitle.str(), dNum2DPhiBins, dMinPhi, dMaxPhi, dNum2DConfidenceLevelBins, 0.0, 1.0);
				}
			}

			//pulls
			bool locFilledFlag = false;
			for(auto locPID : locPIDs)
			{
				bool locIsInVertexFitFlag = locVertexIsFitFlag && locStepVertexInfo->Get_FittableVertexFlag();
				bool locIsChargedFlag = (ParticleCharge(locPID) != 0);

				bool locIsNeutralShowerFlag = (locIsInVertexFitFlag && (ParticleCharge(locPID) == 0));
				if((ParticleMass(locPID) > 0.0) && !locSpactimeIsFitFlag)
					locIsNeutralShowerFlag = false; //massive shower momentum is defined by t, which isn't fit: use particle
				if(!locP4IsFit && !locIsInVertexFitFlag)
					continue; //p4 is not fit, and this is not in a vertex fit: no pulls
				if(locIsNeutralShowerFlag && !locP4IsFit)
					continue; //vertex-only fit: neutral shower does not constrain: no pulls

				if(!locFilledFlag) //first call
				{
					locFilledFlag = true;
					CreateAndChangeTo_Directory(locStepName.str(), locStepName.str());
				}

				string locParticleName = ParticleType(locPID);
				CreateAndChangeTo_Directory(locParticleName, locParticleName);

				string locParticleROOTName = ParticleName_ROOT(locPID);
				string locFullROOTName = locParticleROOTName + string(", ") + locStepROOTName;
				Create_ParticlePulls(locFullROOTName, locIsChargedFlag, locIsInVertexFitFlag, locIsNeutralShowerFlag, loc_i, locPID);

				gDirectory->cd("..");
			} //end of particle loop

			if(locFilledFlag)
				gDirectory->cd("..");
		} //end of step loop

		//Return to the base directory
		ChangeTo_BaseDirectory();
	}
	DEvent::GetLockService(locEvent)->RootUnLock(); //RELEASE ROOT LOCK!!
}

void DHistogramAction_KinFitResults::Run_Update(const std::shared_ptr<const JEvent>& locEvent)
{
	locEvent->GetSingle(dAnalysisUtilities);
	dKinFitUtils->Set_RunDependent_Data(locEvent);
}

void DHistogramAction_KinFitResults::Create_ParticlePulls(string locFullROOTName, bool locIsChargedFlag, bool locIsInVertexFitFlag, bool locIsNeutralShowerFlag, int locStepIndex, Particle_t locPID)
{
	auto locParticlePair = std::make_pair(locStepIndex, locPID);

	DKinFitType locKinFitType = Get_Reaction()->Get_KinFitType();
	bool locP4IsFit = ((locKinFitType != d_VertexFit) && (locKinFitType != d_SpacetimeFit));

	//Determine pull types
	set<pair<DKinFitPullType, pair<string, string> > > locPullTypes;

	//p4 pulls:
	string locHistName, locHistTitle;
	if(locIsNeutralShowerFlag && locP4IsFit) //neutral shower not in a p4-only fit
		locPullTypes.insert(pair<DKinFitPullType, pair<string, string> >(d_EPull, pair<string, string>("Pull_E", "E Pull")));
	else if(!locIsNeutralShowerFlag && (locP4IsFit || locIsInVertexFitFlag))
	{
		//all detected particles (except neutral showers) have p3 used in p4 fits and vertex fits
		//however, don't include if the particles aren't actually in the fit (vertex-only, and too few particles at that vertex to constrain)
		locPullTypes.insert(pair<DKinFitPullType, pair<string, string> >(d_PxPull, pair<string, string>("Pull_Px", "p_{x} Pull")));
		locPullTypes.insert(pair<DKinFitPullType, pair<string, string> >(d_PyPull, pair<string, string>("Pull_Py", "p_{y} Pull")));
		locPullTypes.insert(pair<DKinFitPullType, pair<string, string> >(d_PzPull, pair<string, string>("Pull_Pz", "p_{z} Pull")));
	}

	//vertex pulls:
	if((locIsNeutralShowerFlag && locP4IsFit) || (locIsChargedFlag && locIsInVertexFitFlag))
	{
		locPullTypes.insert(pair<DKinFitPullType, pair<string, string> >(d_XxPull, pair<string, string>("Pull_Xx", "x_{x} Pull")));
		locPullTypes.insert(pair<DKinFitPullType, pair<string, string> >(d_XyPull, pair<string, string>("Pull_Xy", "x_{y} Pull")));
		locPullTypes.insert(pair<DKinFitPullType, pair<string, string> >(d_XzPull, pair<string, string>("Pull_Xz", "x_{z} Pull")));
	}

	//time pulls:
	if(locIsInVertexFitFlag && ((locKinFitType == d_SpacetimeFit) || (locKinFitType == d_P4AndSpacetimeFit)))
		locPullTypes.insert(pair<DKinFitPullType, pair<string, string> >(d_TPull, pair<string, string>("Pull_T", "t Pull")));

	bool locIsBeamFlag = (locFullROOTName.substr(0, 4) == "Beam");
	int locNum2DPBins = locIsBeamFlag ? dNum2DBeamEBins : dNum2DPBins;
	double locMinP = locIsBeamFlag ? dMinBeamE : dMinP;
	double locMaxP = locIsBeamFlag ? dMaxBeamE : dMaxP;

	//Create histograms
	set<pair<DKinFitPullType, pair<string, string> > >::iterator locIterator = locPullTypes.begin();
	for(; locIterator != locPullTypes.end(); ++locIterator)
	{
		pair<string, string> locStrings = (*locIterator).second;
		auto locPullType = (*locIterator).first;

		//1D
		locHistName = locStrings.first;
		locHistTitle = locFullROOTName + string(";") + locStrings.second + string(";# Combos");
		dHistMap_Pulls[locParticlePair][locPullType] = GetOrCreate_Histogram<TH1I>(locHistName, locHistTitle, dNumPullBins, dMinPull, dMaxPull);

		int locNumDeltaBins;
		double locMaxDeltaRange;
		Get_DeltaBinningParams(locPullType, false, locNumDeltaBins, locMaxDeltaRange);
		Get_HistNameTitle(locPullType, locFullROOTName, 0, locHistName, locHistTitle);
		dHistMap_Deltas[locParticlePair][locPullType] = GetOrCreate_Histogram<TH1I>(locHistName, locHistTitle, locNumDeltaBins, -1.0*locMaxDeltaRange, locMaxDeltaRange);

		if(!dHistDependenceFlag)
			continue;
		Get_DeltaBinningParams(locPullType, true, locNumDeltaBins, locMaxDeltaRange);

		//vs p
		locHistName = locStrings.first + string("_VsP");
		locHistTitle = locFullROOTName + string(";p (GeV/c);") + locStrings.second;
		dHistMap_PullsVsP[locParticlePair][locPullType] = GetOrCreate_Histogram<TH2I>(locHistName, locHistTitle, locNum2DPBins, locMinP, locMaxP, dNumPullBins, dMinPull, dMaxPull);
		Get_HistNameTitle(locPullType, locFullROOTName, 1, locHistName, locHistTitle);
		dHistMap_DeltasVsP[locParticlePair][locPullType] = GetOrCreate_Histogram<TH2I>(locHistName, locHistTitle, locNum2DPBins, locMinP, locMaxP, locNumDeltaBins, -1.0*locMaxDeltaRange, locMaxDeltaRange);
		if(locIsBeamFlag)
			continue;

		//vs theta
		locHistName = locStrings.first + string("_VsTheta");
		locHistTitle = locFullROOTName + string(";#theta#circ;") + locStrings.second;
		dHistMap_PullsVsTheta[locParticlePair][locPullType] = GetOrCreate_Histogram<TH2I>(locHistName, locHistTitle, dNum2DThetaBins, dMinTheta, dMaxTheta, dNumPullBins, dMinPull, dMaxPull);
		Get_HistNameTitle(locPullType, locFullROOTName, 2, locHistName, locHistTitle);
		dHistMap_DeltasVsTheta[locParticlePair][locPullType] = GetOrCreate_Histogram<TH2I>(locHistName, locHistTitle, dNum2DThetaBins, dMinTheta, dMaxTheta, locNumDeltaBins, -1.0*locMaxDeltaRange, locMaxDeltaRange);

		//vs phi
		locHistName = locStrings.first + string("_VsPhi");
		locHistTitle = locFullROOTName + string(";#phi#circ;") + locStrings.second;
		dHistMap_PullsVsPhi[locParticlePair][locPullType] = GetOrCreate_Histogram<TH2I>(locHistName, locHistTitle, dNum2DPhiBins, dMinPhi, dMaxPhi, dNumPullBins, dMinPull, dMaxPull);
		Get_HistNameTitle(locPullType, locFullROOTName, 3, locHistName, locHistTitle);
		dHistMap_DeltasVsPhi[locParticlePair][locPullType] = GetOrCreate_Histogram<TH2I>(locHistName, locHistTitle, dNum2DPhiBins, dMinPhi, dMaxPhi, locNumDeltaBins, -1.0*locMaxDeltaRange, locMaxDeltaRange);
	}
}

void DHistogramAction_KinFitResults::Get_HistNameTitle(DKinFitPullType locPullType, string locFullROOTName, int locVsKey, string& locHistName, string& locHistTitle)
{
	string locNameParam, locTitleParam;
	if(locPullType == d_EPull)
	{
		locNameParam = "ShowerE";
		locTitleParam = "E";
	}
	else if(locPullType == d_PxPull) //delta-theta!!
	{
		locNameParam = "Theta";
		locTitleParam = "#theta#circ";
	}
	else if(locPullType == d_PyPull) //delta-phi!!
	{
		locNameParam = "Phi";
		locTitleParam = "#phi#circ";
	}
	else if(locPullType == d_PzPull) //delta-p/p!!
	{
		locNameParam = "POverP";
		locTitleParam = "p/p";
	}
	else if(locPullType == d_XxPull)
	{
		locNameParam = "X";
		locTitleParam = "Vertex-X (cm)";
	}
	else if(locPullType == d_XyPull)
	{
		locNameParam = "Y";
		locTitleParam = "Vertex-Y (cm)";
	}
	else if(locPullType == d_XzPull)
	{
		locNameParam = "Z";
		locTitleParam = "Vertex-Z (cm)";
	}
	else if(locPullType == d_TPull)
	{
		locNameParam = "T";
		locTitleParam = "T (ns)";
	}

	if(locVsKey == 0) //1D
	{
		locHistName = string("Delta") + locNameParam;
		locHistTitle = locFullROOTName + string(";#Delta") + locTitleParam;
	}
	else if(locVsKey == 1) //p
	{
		locHistName = string("Delta") + locNameParam + string("_VsP");
		locHistTitle = locFullROOTName + string(";p (GeV/c);#Delta") + locTitleParam;
	}
	else if(locVsKey == 2) //theta
	{
		locHistName = string("Delta") + locNameParam + string("_VsTheta");
		locHistTitle = locFullROOTName + string(";#theta#circ;#Delta") + locTitleParam;
	}
	else if(locVsKey == 3) //phi
	{
		locHistName = string("Delta") + locNameParam + string("_VsPhi");
		locHistTitle = locFullROOTName + string(";#phi#circ;#Delta") + locTitleParam;
	}
}

void DHistogramAction_KinFitResults::Get_DeltaBinningParams(DKinFitPullType locPullType, bool loc2DFlag, int& locNumBins, double& locMax)
{
	if(locPullType == d_EPull)
	{
		locNumBins = loc2DFlag ? dNum2DDeltaShowerEBins : dNumDeltaShowerEBins;
		locMax = dMaxDeltaShowerE;
	}
	else if(locPullType == d_PxPull) //delta-theta!!
	{
		locNumBins = loc2DFlag ? dNum2DDeltaThetaBins : dNumDeltaThetaBins;
		locMax = dMaxDeltaTheta;
	}
	else if(locPullType == d_PyPull) //delta-phi!!
	{
		locNumBins = loc2DFlag ? dNum2DDeltaPhiBins : dNumDeltaPhiBins;
		locMax = dMaxDeltaPhi;
	}
	else if(locPullType == d_PzPull) //delta-p/p!!
	{
		locNumBins = loc2DFlag ? dNum2DDeltaPOverPBins : dNumDeltaPOverPBins;
		locMax = dMaxDeltaPOverP;
	}
	else if((locPullType == d_XxPull) || (locPullType == d_XyPull))
	{
		locNumBins = loc2DFlag ? dNum2DDeltaVertexXYBins : dNumDeltaVertexXYBins;
		locMax = dMaxDeltaVertexXY;
	}
	else if(locPullType == d_XzPull)
	{
		locNumBins = loc2DFlag ? dNum2DDeltaVertexZBins : dNumDeltaVertexZBins;
		locMax = dMaxDeltaVertexZ;
	}
	else if(locPullType == d_TPull)
	{
		locNumBins = loc2DFlag ? dNum2DDeltaTBins : dNumDeltaTBins;
		locMax = dMaxDeltaT;
	}
}

bool DHistogramAction_KinFitResults::Perform_Action(const std::shared_ptr<const JEvent>& locEvent, const DParticleCombo* locParticleCombo)
{
	//kinfit results are unique for each DParticleCombo: no need to check for duplicates
	const DKinFitResults* locKinFitResults = locParticleCombo->Get_KinFitResults();
	if(locKinFitResults == NULL)
		return true;

	// Get Confidence Level
	double locConfidenceLevel = locKinFitResults->Get_ConfidenceLevel();
	
	// Get reduced chi^2
	double locChiSq = locKinFitResults->Get_ChiSq()/locKinFitResults->Get_NDF() ;

	// Get Pulls
	map<const JObject*, map<DKinFitPullType, double> > locPulls; //DKinematicData is the MEASURED particle
	locKinFitResults->Get_Pulls(locPulls);

	Particle_t locInitPID = Get_Reaction()->Get_ReactionStep(0)->Get_InitialPID();
	bool locBeamFlag = DAnalysis::Get_IsFirstStepBeam(Get_Reaction());

	//FILL HISTOGRAMS
	//Since we are filling histograms local to this action, it will not interfere with other ROOT operations: can use action-wide ROOT lock
	//Note, the mutex is unique to this DReaction + action_string combo: actions of same class with different hists will have a different mutex
	Lock_Action();
	{
		dHist_ConfidenceLevel->Fill(locConfidenceLevel);
		dHist_ChiSq->Fill(locChiSq);
		
		// beam
		if(locBeamFlag)
		{
			const DKinematicData* locKinematicData = locParticleCombo->Get_ParticleComboStep(0)->Get_InitialParticle_Measured();
			map<DKinFitPullType, double> locParticlePulls = locPulls[locKinematicData];
			if(!locParticlePulls.empty())
			{
				pair<int, Particle_t> locParticlePair(-1, locInitPID);
				DVector3 locMomentum = locKinematicData->momentum();
				double locP = locMomentum.Mag();

				//con lev
				if(dHistDependenceFlag)
					dHistMap_ConfidenceLevel_VsP[locParticlePair]->Fill(locP, locConfidenceLevel);

				//pulls
				if(locConfidenceLevel >= dPullHistConfidenceLevelCut)
				{
					map<DKinFitPullType, double>::iterator locIterator = locParticlePulls.begin();
					for(; locIterator != locParticlePulls.end(); ++locIterator)
					{
						dHistMap_Pulls[locParticlePair][locIterator->first]->Fill(locIterator->second);
						if(dHistDependenceFlag)
							dHistMap_PullsVsP[locParticlePair][locIterator->first]->Fill(locP, locIterator->second);
					}
				}

				//deltas
				double locDeltaPOverP = (locParticleCombo->Get_ParticleComboStep(0)->Get_InitialParticle()->pmag() - locP)/locP;
				dHistMap_Deltas[locParticlePair][d_PzPull]->Fill(locDeltaPOverP);
				if(dHistDependenceFlag)
					dHistMap_DeltasVsP[locParticlePair][d_PzPull]->Fill(locP, locDeltaPOverP);
			}
		}

		// final particle pulls
		for(size_t loc_i = 0; loc_i < locParticleCombo->Get_NumParticleComboSteps(); ++loc_i)
		{
			const DParticleComboStep* locParticleComboStep = locParticleCombo->Get_ParticleComboStep(loc_i);
			auto locParticles = locParticleComboStep->Get_FinalParticles_Measured(Get_Reaction()->Get_ReactionStep(loc_i));
			auto locKinFitParticles = locParticleComboStep->Get_FinalParticles(Get_Reaction()->Get_ReactionStep(loc_i), false, false);
			for(size_t loc_j = 0; loc_j < locParticles.size(); ++loc_j)
			{
				auto& locParticle = locParticles[loc_j];
				auto& locKinFitParticle = locKinFitParticles[loc_j];

				//get pulls for this particle
				map<DKinFitPullType, double> locParticlePulls;
				map<const JObject*, map<DKinFitPullType, double> >::iterator locParticleIterator = locPulls.find(locParticle);
				if(locParticleIterator != locPulls.end())
					locParticlePulls = locParticleIterator->second;
				else //is neutral shower
				{
					auto locNeutralHypo = dynamic_cast<const DNeutralParticleHypothesis*>(locParticle);
					locParticlePulls = locPulls[locNeutralHypo->Get_NeutralShower()];
				}
				if(locParticlePulls.empty())
					continue;

				pair<int, Particle_t> locParticlePair(loc_i, locParticle->PID());
				DVector3 locMomentum = locParticle->momentum();
				double locP = locMomentum.Mag();
				double locTheta = locMomentum.Theta()*180.0/TMath::Pi();
				double locPhi = locMomentum.Phi()*180.0/TMath::Pi();

				//deltas //loop used to easily figure out what params changed
				for(auto locIterator = locParticlePulls.begin(); locIterator != locParticlePulls.end(); ++locIterator)
				{
					auto locPullType = locIterator->first;
					auto locDelta = Get_Delta(locPullType, locParticle, locKinFitParticle);
					dHistMap_Deltas[locParticlePair][locPullType]->Fill(locDelta);
					if(!dHistDependenceFlag)
						continue;

					dHistMap_DeltasVsP[locParticlePair][locPullType]->Fill(locP, locDelta);
					dHistMap_DeltasVsTheta[locParticlePair][locPullType]->Fill(locTheta, locDelta);
					dHistMap_DeltasVsPhi[locParticlePair][locPullType]->Fill(locPhi, locDelta);
				}

				//con lev
				if(dHistDependenceFlag)
				{
					dHistMap_ConfidenceLevel_VsP[locParticlePair]->Fill(locP, locConfidenceLevel);
					dHistMap_ConfidenceLevel_VsTheta[locParticlePair]->Fill(locTheta, locConfidenceLevel);
					dHistMap_ConfidenceLevel_VsPhi[locParticlePair]->Fill(locPhi, locConfidenceLevel);
				}
				if(locConfidenceLevel < dPullHistConfidenceLevelCut)
					continue;

				//fill histograms
				map<DKinFitPullType, double>::iterator locIterator = locParticlePulls.begin();
				for(; locIterator != locParticlePulls.end(); ++locIterator)
				{
					dHistMap_Pulls[locParticlePair][locIterator->first]->Fill(locIterator->second);
					if(!dHistDependenceFlag)
						continue;
					dHistMap_PullsVsP[locParticlePair][locIterator->first]->Fill(locP, locIterator->second);
					dHistMap_PullsVsTheta[locParticlePair][locIterator->first]->Fill(locTheta, locIterator->second);
					dHistMap_PullsVsPhi[locParticlePair][locIterator->first]->Fill(locPhi, locIterator->second);
				}
			}
		}
	}
	Unlock_Action();

	return true;
}

double DHistogramAction_KinFitResults::Get_Delta(DKinFitPullType locPullType, const DKinematicData* locMeasured, const DKinematicData* locKinFit)
{
	//kinfit - measured
	if(locPullType == d_EPull)
		return locKinFit->energy() - locMeasured->energy();
	else if(locPullType == d_PxPull) //delta-theta!!
		return (locKinFit->momentum().Theta() - locMeasured->momentum().Theta())*180.0/TMath::Pi();
	else if(locPullType == d_PyPull) //delta-phi!!
	{
		auto locDeltaPhi = (locKinFit->momentum().Phi() - locMeasured->momentum().Phi())*180.0/TMath::Pi();
		while(locDeltaPhi > 180.0)
			locDeltaPhi -= 360.0;
		while(locDeltaPhi < -180.0)
			locDeltaPhi += 360.0;
		return locDeltaPhi;
	}
	else if(locPullType == d_PzPull) //delta-p-over-p!!
	{
		auto locP = locMeasured->momentum().Mag();
		return (locKinFit->momentum().Mag() - locP)/locP;
	}
	else if(locPullType == d_XxPull)
		return locKinFit->position().X() - locMeasured->position().X();
	else if(locPullType == d_XyPull)
		return locKinFit->position().Y() - locMeasured->position().Y();
	else if(locPullType == d_XzPull)
		return locKinFit->position().Z() - locMeasured->position().Z();
	else if(locPullType == d_TPull)
		return locKinFit->time() - locMeasured->time();
	else
		return 0.0;
}

void DHistogramAction_MissingTransverseMomentum::Initialize(const std::shared_ptr<const JEvent>& locEvent)
{
	string locHistName, locHistTitle;
	double locPtPerBin = 1000.0*(dMaxPt - dMinPt)/((double)dNumPtBins);

	Run_Update(locEvent);

	//CREATE THE HISTOGRAMS
	//Since we are creating histograms, the contents of gDirectory will be modified: must use JANA-wide ROOT lock
	DEvent::GetLockService(locEvent)->RootWriteLock(); //ACQUIRE ROOT LOCK!!
	{
		CreateAndChangeTo_ActionDirectory();

		locHistName = "MissingTransverseMomentum";
		ostringstream locStream;
		locStream << locPtPerBin;
		locHistTitle = string(";") + string(" Missing Transverse Momentum (GeV/c);# Combos / ") + locStream.str() + string(" MeV/c");
		dHist_MissingTransverseMomentum = GetOrCreate_Histogram<TH1I>(locHistName, locHistTitle, dNumPtBins, dMinPt, dMaxPt);

		//Return to the base directory
		ChangeTo_BaseDirectory();
	}
	DEvent::GetLockService(locEvent)->RootUnLock(); //RELEASE ROOT LOCK!!
}

void DHistogramAction_MissingTransverseMomentum::Run_Update(const std::shared_ptr<const JEvent>& locEvent)
{
	vector<const DAnalysisUtilities*> locAnalysisUtilitiesVector;
	locEvent->Get(locAnalysisUtilitiesVector);
	dAnalysisUtilities = locAnalysisUtilitiesVector[0];
}

bool DHistogramAction_MissingTransverseMomentum::Perform_Action(const std::shared_ptr<const JEvent>& locEvent, const DParticleCombo* locParticleCombo)
{
	set<pair<const JObject*, unsigned int> > locSourceObjects;
	DLorentzVector locFinalStateP4 = dAnalysisUtilities->Calc_FinalStateP4(Get_Reaction(), locParticleCombo, 0, locSourceObjects, Get_UseKinFitResultsFlag()); // Use step '0'

	if(dPreviousSourceObjects.find(locSourceObjects) != dPreviousSourceObjects.end())
		return true; //dupe: already histed!
	dPreviousSourceObjects.insert(locSourceObjects);

	double locMissingTransverseMomentum = locFinalStateP4.Pt();

	//FILL HISTOGRAMS
	//Since we are filling histograms local to this action, it will not interfere with other ROOT operations: can use action-wide ROOT lock
	//Note, the mutex is unique to this DReaction + action_string combo: actions of same class with different hists will have a different mutex
	Lock_Action();
	{
		dHist_MissingTransverseMomentum->Fill(locMissingTransverseMomentum);
	}
	Unlock_Action();

	return true;
}

