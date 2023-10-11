// $Id$
//
//    File: DDIRCLut.cc
//

#include <cassert>
#include <math.h>
using namespace std;

#include "DDIRCLut.h"

#include <JANA/JEvent.h>
#include <JANA/Calibrations/JCalibrationManager.h>

#include "HDGEOMETRY/DGeometry.h"


//---------------------------------
// Init
//---------------------------------
bool DDIRCLut::Init(const std::shared_ptr<const JEvent>& event) {

	auto event_number = event->GetEventNumber();
	auto runnumber = event->GetRunNumber();
	auto app = event->GetJApplication();
	auto calibration = app->GetService<JCalibrationManager>()->GetJCalibration(runnumber);
	dGeometryManager = app->GetService<DGeometryManager>();
	lockService = app->GetService<JLockService>();

        DIRC_DEBUG_HISTS = false;
        app->SetDefaultParameter("DIRC:DEBUG_HISTS",DIRC_DEBUG_HISTS);

	DIRC_TRUTH_BARHIT = false;
	app->SetDefaultParameter("DIRC:TRUTH_BARHIT",DIRC_TRUTH_BARHIT);

	DIRC_TRUTH_PIXELTIME = false;
	app->SetDefaultParameter("DIRC:TRUTH_PIXELTIME",DIRC_TRUTH_PIXELTIME);

	// timing cuts for photons
	DIRC_CUT_TDIFFD = 2; // direct cut in ns
	app->SetDefaultParameter("DIRC:CUT_TDIFFD",DIRC_CUT_TDIFFD);
	DIRC_CUT_TDIFFR = 3; // reflected cut in ns
	app->SetDefaultParameter("DIRC:CUT_TDIFFR",DIRC_CUT_TDIFFR);

	// Gives DeltaT = 0, but shouldn't it be v=20.3767 [cm/ns] for 1.47125
	DIRC_LIGHT_V = 19.65; // v=19.8 [cm/ns] for 1.5141
	app->SetDefaultParameter("DIRC:LIGHT_V",DIRC_LIGHT_V);

	// sigma (thetaC for single photon) in radians
	DIRC_SIGMA_THETAC = 0.008;
	app->SetDefaultParameter("DIRC:SIGMA_THETAC",DIRC_SIGMA_THETAC);

	// Rotate tracks angle based on bar box survey data
	DIRC_ROTATE_TRACK = true;
	app->SetDefaultParameter("DIRC:ROTATE_TRACK",DIRC_ROTATE_TRACK);

	// PMT angle offsets for each bar from CCDB table (deprecated)
	DIRC_THETAC_OFFSET = false;
	app->SetDefaultParameter("DIRC:THETAC_OFFSET",DIRC_THETAC_OFFSET);

	// PMT angle and time offsets to correct LUT from CCDB resource
	DIRC_LUT_CORR = true;
	app->SetDefaultParameter("DIRC:LUT_CORR",DIRC_LUT_CORR);

	// CHROMATIC CORRECTION
	DIRC_CHROMATIC_CORR = true;
	app->SetDefaultParameter("DIRC:CHROMATIC_CORR",DIRC_CHROMATIC_CORR);
	DIRC_CHROMATIC_CONST = 0.0025;
	app->SetDefaultParameter("DIRC:CHROMATIC_CONST",DIRC_CHROMATIC_CONST);

	// CHERENKOV ANGLE CUT
	DIRC_CUT_ANGLE = 0.03;
	app->SetDefaultParameter("DIRC:CUT_ANGLE",DIRC_CUT_ANGLE);

	// set PID for different passes in debuging histograms
	dFinalStatePIDs.push_back(Positron);
	dFinalStatePIDs.push_back(PiPlus);
	dFinalStatePIDs.push_back(KPlus);
	dFinalStatePIDs.push_back(Proton);

	dMaxChannels = DDIRCGeometry::kPMTs*DDIRCGeometry::kPixels;

	dCriticalAngle = asin(1.00028/1.47125); // n_quarzt = 1.47125; //(1.47125 <==> 390nm)
	dIndex = 1.473;

	if(DIRC_DEBUG_HISTS)
		CreateDebugHistograms();

	dDIRCLutReader = dGeometryManager->GetDIRCLut(runnumber);
	
	// get DIRC geometry
	vector<const DDIRCGeometry*> locDIRCGeometry;
        event->Get(locDIRCGeometry);
        dDIRCGeometry = locDIRCGeometry[0];

	// get cherenkov angle corrections from CCDB
	vector <double> thetac_offsets_bar(5184);
	if(calibration->Get("/DIRC/thetac_offsets_bar", thetac_offsets_bar))
	  jout << "Can't find requested /DIRC/thetac_offsets_bar in CCDB for this run!" << endl;
	
	for(int ibar=0; ibar<DDIRCGeometry::kBars; ibar++) {
	  for(int ipmt=0; ipmt<DDIRCGeometry::kPMTs; ipmt++) {
	    int index = ipmt + ibar*DDIRCGeometry::kPMTs;
	    dThetaCOffset[ibar][ipmt] = thetac_offsets_bar.at(index);
	  }
	}

	// get track rotation corrections from CCDB
	vector< map<string, double> > bar_rotation(DDIRCGeometry::kBars);
	if(calibration->Get("/DIRC/bar_rotation", bar_rotation))
	  jout << "Can't find requested /DIRC/bar_rotation in CCDB for this run!" << endl;
	
	for(int ibar=0; ibar<DDIRCGeometry::kBars; ibar++) {
	    dRotationX[ibar] = bar_rotation[ibar].at("rotationX")*TMath::DegToRad();
	    dRotationY[ibar] = bar_rotation[ibar].at("rotationY")*TMath::DegToRad();
	    dRotationZ[ibar] = bar_rotation[ibar].at("rotationZ")*TMath::DegToRad();
	}

	return true;
}

bool DDIRCLut::CreateDebugHistograms() {

	////////////////////////////////////
	// LUT histograms and diagnostics //
	////////////////////////////////////

	//dapp->RootWriteLock(); //ACQUIRE ROOT LOCK!!
	{
		//TDirectory *mainDir = gDirectory;
		//TDirectory *dircDir = gDirectory->mkdir("DIRC_debug");
		//dircDir->cd();

		hDiff = (TH1I*)gROOT->FindObject("hDiff");
		hDiff_Pixel[0] = (TH2I*)gROOT->FindObject("hDiff_Pixel_N");
		hDiff_Pixel[1] = (TH2I*)gROOT->FindObject("hDiff_Pixel_S");
		hDiffT = (TH1I*)gROOT->FindObject("hDiffT");
		hDiffD = (TH1I*)gROOT->FindObject("hDiffD");
		hDiffR = (TH1I*)gROOT->FindObject("hDiffR");
		hTime = (TH1I*)gROOT->FindObject("hTime");
		hCalc = (TH1I*)gROOT->FindObject("hCalc");
		hNph = (TH1I*)gROOT->FindObject("hNph");
		if(!hDiff) hDiff = new TH1I("hDiff",";t_{calc}-t_{measured} [ns];entries [#]", 400,-20,20);
		if(!hDiff_Pixel[0]) hDiff_Pixel[0] = new TH2I("hDiff_Pixel_N","; Channel ID; t_{calc}-t_{measured} [ns];entries [#]", dMaxChannels, 0, dMaxChannels, 400,-20,20);
		if(!hDiff_Pixel[1]) hDiff_Pixel[1] = new TH2I("hDiff_Pixel_S","; Channel ID; t_{calc}-t_{measured} [ns];entries [#]", dMaxChannels, 0, dMaxChannels, 400,-20,20);
		if(!hDiffT) hDiffT = new TH1I("hDiffT",";t_{calc}-t_{measured} [ns];entries [#]", 400,-20,20);
		if(!hDiffD) hDiffD = new TH1I("hDiffD",";t_{calc}-t_{measured} [ns];entries [#]", 400,-20,20);
		if(!hDiffR) hDiffR = new TH1I("hDiffR",";t_{calc}-t_{measured} [ns];entries [#]", 400,-20,20);
		if(!hTime) hTime = new TH1I("hTime",";propagation time [ns];entries [#]",   1000,0,200);
		if(!hCalc) hCalc = new TH1I("hCalc",";calculated time [ns];entries [#]",   1000,0,200);
		if(!hNph) hNph = new TH1I("hNph",";detected photons [#];entries [#]", 150,0,150);	
		
		// DeltaThetaC for each particle type (e, pi, K, p) and per pixel
		for(uint loc_i=0; loc_i<dFinalStatePIDs.size(); loc_i++) {
			Particle_t locPID = dFinalStatePIDs[loc_i];
			string locParticleName = ParticleType(locPID);
			string locParticleROOTName = ParticleName_ROOT(locPID);
			
			hDeltaThetaC[locPID] = (TH1I*)gROOT->FindObject(Form("hDeltaThetaC_%s",locParticleName.data()));
			if(!hDeltaThetaC[locPID]) 
				hDeltaThetaC[locPID] = new TH1I(Form("hDeltaThetaC_%s",locParticleName.data()),  "cherenkov angle; #Delta#theta_{C} [rad]", 200,-0.5,0.5);
			hDeltaThetaC_Pixel[locPID] = (TH2I*)gROOT->FindObject(Form("hDeltaThetaC_Pixel_%s",locParticleName.data()));
			if(!hDeltaThetaC_Pixel[locPID]) 
				hDeltaThetaC_Pixel[locPID] = new TH2I(Form("hDeltaThetaC_Pixel_%s",locParticleName.data()),  "cherenkov angle; Pixel ID; #Delta#theta_{C} [rad]", 10000, 0, 10000, 200,-0.5,0.5);
		}

		//mainDir->cd();
	}
	//dapp->RootUnLock(); //REMOVE ROOT LOCK!!
	
	return true;
}

bool DDIRCLut::CalcLUT(TVector3 locProjPos, TVector3 locProjMom, const vector<const DDIRCPmtHit*> locDIRCHits, double locFlightTime, double locMass, shared_ptr<DDIRCMatchParams>& locDIRCMatchParams, const vector<const DDIRCTruthBarHit*> locDIRCBarHits, map<shared_ptr<const DDIRCMatchParams>, vector<const DDIRCPmtHit*> >& locDIRCTrackMatchParams) const
{
	// get bar and track position/momentum from extrapolation
	TVector3 momInBar = locProjMom;
	TVector3 posInBar = locProjPos;

	////////////////////////////////////////////
	// option to cheat and use truth position //
	////////////////////////////////////////////
	if(DIRC_TRUTH_BARHIT && locDIRCBarHits.size() > 0) {

		TVector3 bestMatchPos, bestMatchMom;
		double bestFlightTime = 999.;
		double bestMatchDist = 999.;
		for(int i=0; i<(int)locDIRCBarHits.size(); i++) {
			TVector3 locDIRCBarHitPos(locDIRCBarHits[i]->x, locDIRCBarHits[i]->y, locDIRCBarHits[i]->z);
			TVector3 locDIRCBarHitMom(locDIRCBarHits[i]->px, locDIRCBarHits[i]->py, locDIRCBarHits[i]->pz);
			if((posInBar - locDIRCBarHitPos).Mag() < bestMatchDist) {
				bestMatchDist = (posInBar - locDIRCBarHitPos).Mag();
				bestMatchPos = locDIRCBarHitPos;
				bestMatchMom = locDIRCBarHitMom;
				bestFlightTime = locDIRCBarHits[i]->t;
			}
		}
		
		momInBar = bestMatchMom;
		posInBar = bestMatchPos;
		locFlightTime = bestFlightTime;
	}

	// clear object to store good photons
	if(locDIRCMatchParams == nullptr)
		locDIRCMatchParams = std::make_shared<DDIRCMatchParams>();

	// initialize variables for LUT summary
	double locAngle = CalcAngle(momInBar.Mag(), locMass);
	map<Particle_t, double> locExpectedAngle = CalcExpectedAngles(momInBar.Mag());
	map<Particle_t, double> logLikelihoodSum;
	Particle_t locHypothesisPID = PiPlus;
	for(uint loc_i = 0; loc_i<dFinalStatePIDs.size(); loc_i++) {
		logLikelihoodSum[dFinalStatePIDs[loc_i]]=0;
		if(fabs(ParticleMass(dFinalStatePIDs[loc_i])-locMass) < 0.01) locHypothesisPID = dFinalStatePIDs[loc_i];
	}

	// loop over DIRC hits
	int nPhotons = 0;
	int nPhotonsThetaC = 0;
	double meanThetaC = 0.;
	double meanDeltaT = 0.;
	for (unsigned int loc_i = 0; loc_i < locDIRCHits.size(); loc_i++){

		const DDIRCPmtHit* locDIRCHit = locDIRCHits[loc_i];
		bool locIsGood = false;
		bool locIsReflected = false;
		CalcPhoton(locDIRCHit, locFlightTime, posInBar, momInBar, locExpectedAngle, locAngle, locHypothesisPID, locIsReflected, logLikelihoodSum, nPhotonsThetaC, meanThetaC, meanDeltaT, locIsGood);
		if(locIsGood) {
			// count good photons and add hits to associated objects
			nPhotons++;
			locDIRCTrackMatchParams[locDIRCMatchParams].push_back(locDIRCHit);
		} 
	}// end loop over hits
		
	if(DIRC_DEBUG_HISTS) {
		lockService->RootWriteLock();
		hNph->Fill(nPhotons);
		lockService->RootUnLock();
	}
	
	// skip tracks without enough photons
	if(nPhotons<5) 
		return false;

	// set DIRCMatchParameters contents
	locDIRCMatchParams->dThetaC = meanThetaC/(double)nPhotonsThetaC;
	locDIRCMatchParams->dDeltaT = meanDeltaT/(double)nPhotonsThetaC;
	locDIRCMatchParams->dExpectedThetaC = locAngle;
	locDIRCMatchParams->dLikelihoodElectron = logLikelihoodSum[Positron];
	locDIRCMatchParams->dLikelihoodPion = logLikelihoodSum[PiPlus];
	locDIRCMatchParams->dLikelihoodKaon = logLikelihoodSum[KPlus];
	locDIRCMatchParams->dLikelihoodProton = logLikelihoodSum[Proton];
	locDIRCMatchParams->dNPhotons = nPhotons;
	locDIRCMatchParams->dExtrapolatedPos = posInBar;
	locDIRCMatchParams->dExtrapolatedMom = momInBar;
	locDIRCMatchParams->dExtrapolatedTime = locFlightTime;

	return true;
}

vector<pair<double,double>> DDIRCLut::CalcPhoton(const DDIRCPmtHit *locDIRCHit, double locFlightTime, TVector3 posInBar, TVector3 momInBar, map<Particle_t, double> locExpectedAngle, double locAngle, Particle_t locPID, bool &isReflected, map<Particle_t, double> &logLikelihoodSum, int &nPhotonsThetaC, double &meanThetaC, double &meanDeltaT, bool &isGood) const
{	
	// initialize photon pairs for time and thetaC
	pair<double, double> locDIRCPhoton(-999., -999.);
	vector<pair<double, double>> locDIRCPhotons;

	TVector3 fnX1 = TVector3 (1,0,0);
	TVector3 fnY1 = TVector3 (0,1,0);
	TVector3 fnZ1 = TVector3 (0,0,1);

	double tangle,luttheta,evtime;
	int64_t pathid; 
	TVector3 dir,dird;
	
	// get bar number from geometry
	int bar = dDIRCGeometry->GetBar(posInBar.Y()); 
	if(bar < 0 || bar > 47) return locDIRCPhotons;
	
	// get channel information for LUT
	int channel = locDIRCHit->ch;
	
	// get box from bar number (North/Upper = 0 and South/Lower = 1)
	int box = (bar < 24) ? 1 : 0;
	if((box == 0 && channel < dMaxChannels) || (box == 1 && channel >= dMaxChannels)) 
		return locDIRCPhotons;

	int box_channel = channel%dMaxChannels;
	int box_pmt = box_channel/DDIRCGeometry::kPixels;

	double rotX = 0, rotY = 0, rotZ=0;
	uint xbinSize = 200.0 / dDIRCLutReader->GetLutCorrNbins();
	int bin = (int)( (posInBar.X()+100.) / xbinSize );
	if(bar>=0 && bar<=47 && bin>=0 && bin<=39) {
	  rotX = dRotationX[bar];
	  rotY = dRotationY[bar];
	  rotZ = dRotationZ[bar];
	}

	// use hit time to determine if reflected or not
	double hitTime = locDIRCHit->t - locFlightTime;
	
	// option to use truth hit time rather than smeared hit time
	vector<const DDIRCTruthPmtHit*> locTruthDIRCHits = locDIRCHit->Get<DDIRCTruthPmtHit>();
	if(DIRC_TRUTH_PIXELTIME && locTruthDIRCHits.size() > 0) {
		double locTruthTime = locTruthDIRCHits[0]->t - locFlightTime;
		hitTime = locTruthTime;          
	}
	
	// needs to be X dependent choice for reflection cut (from CCDB?)
	bool reflected = hitTime>35; // try only some photons as reflected for now

	// get position along bar for calculated time 
	double radiatorL = dDIRCGeometry->GetBarLength(bar);
	double barend = dDIRCGeometry->GetBarEnd(bar);
	double lenz = fabs(barend - posInBar.X());  
	
	// get length for reflected and direct photons
	double rlenz = 2*radiatorL - lenz; // reflected
	double dlenz = lenz; // direct
	if(reflected) lenz = 2*radiatorL - lenz;
		
	// check for pixel before going through loop
	if(dDIRCLutReader->GetLutPixelAngleSize(bar, box_channel) == 0) 
		return locDIRCPhotons;
	
	// loop over LUT table for this bar/pixel to calculate thetaC
	for(uint i = 0; i < dDIRCLutReader->GetLutPixelAngleSize(bar, box_channel); i++){
		
		dird   = dDIRCLutReader->GetLutPixelAngle(bar, box_channel, i); 
		evtime = dDIRCLutReader->GetLutPixelTime(bar, box_channel, i); 
		pathid = dDIRCLutReader->GetLutPixelPath(bar, box_channel, i); 
		
		// in MC we can check if the path of the LUT and measured photon are the same
		bool samepath(false);
		if(!locTruthDIRCHits.empty() && fabs(pathid - locTruthDIRCHits[0]->path)<0.0001) 
			samepath=true;
		
		for(int r=0; r<2; r++){
			if(!reflected && r==1) continue;
			
			if(r) lenz = rlenz;
			else lenz = dlenz;
			
			for(int u = 0; u < 4; u++){
				if(u == 0) dir = dird;
				if(u == 1) dir.SetXYZ( dird.X(),-dird.Y(),  dird.Z());
				if(u == 2) dir.SetXYZ( dird.X(), dird.Y(), -dird.Z());
				if(u == 3) dir.SetXYZ( dird.X(),-dird.Y(), -dird.Z());
				if(r) dir.SetXYZ( -dir.X(), dir.Y(), dir.Z());
				if(dir.Angle(fnY1) < dCriticalAngle || dir.Angle(fnZ1) < dCriticalAngle) continue;

				dir = dir.Unit();

				luttheta = dir.Angle(TVector3(-1,0,0));
				if(luttheta > TMath::PiOver2()) luttheta = TMath::Pi()-luttheta;

				double bartime = lenz/cos(luttheta)/DIRC_LIGHT_V;
				double totalTime = bartime+evtime;

				// LUT time corrections
				if(DIRC_LUT_CORR){
				  if (reflected) totalTime -= dDIRCLutReader->GetLutCorrTimeReflected(bar,box_pmt,bin);
				  else totalTime -= dDIRCLutReader->GetLutCorrTimeDirect(bar,box_pmt,bin);
				}

				// calculate time difference
				double locDeltaT = totalTime-hitTime;

				TVector3 trackMom = momInBar;
				if(DIRC_ROTATE_TRACK) { // rotate tracks to bar plane from survey data
				  trackMom.RotateX(rotX);
				  trackMom.RotateY(rotY);
				  trackMom.RotateZ(rotZ);
				}
				tangle = trackMom.Angle(dir);
 
				if(DIRC_THETAC_OFFSET) { // ad-hoc correction per-PMT
				  tangle -= dThetaCOffset[bar][box_pmt];
				}

				// LUT angle corrections
				if(DIRC_LUT_CORR){
				  if (r) tangle += dDIRCLutReader->GetLutCorrAngleReflected(bar,box_pmt,bin);
				  else tangle += dDIRCLutReader->GetLutCorrAngleDirect(bar,box_pmt,bin);
				}
				
				// chromatic correction
				if (fabs(locDeltaT) < 2.0 && DIRC_CHROMATIC_CORR) 
				  tangle += DIRC_CHROMATIC_CONST * locDeltaT; 

				if(DIRC_DEBUG_HISTS) {	
					lockService->RootWriteLock();
					hTime->Fill(hitTime);
					hCalc->Fill(totalTime);
					
					if(fabs(tangle-0.5*(locExpectedAngle[PiPlus]+locExpectedAngle[KPlus]))<0.2){
						hDiff->Fill(locDeltaT);
						hDiff_Pixel[box]->Fill(channel%dMaxChannels, locDeltaT);
						if(samepath){
							hDiffT->Fill(locDeltaT);
							if(r) hDiffR->Fill(locDeltaT);
							else hDiffD->Fill(locDeltaT);
						}
					}
					lockService->RootUnLock();
				}
				
				// save hits array which pass some lose time and angle criteria
				if(fabs(locDeltaT) < 100.0 && fabs(tangle-0.5*(locExpectedAngle[PiPlus]+locExpectedAngle[KPlus]))<0.2) {
					locDIRCPhoton.first = totalTime;
					locDIRCPhoton.second = tangle;
					locDIRCPhotons.push_back(locDIRCPhoton);
				}

				// reject photons that are too far out of time
				if(!r && fabs(locDeltaT)>DIRC_CUT_TDIFFD) continue;
				if( r && fabs(locDeltaT)>DIRC_CUT_TDIFFR) continue;
				
				if(DIRC_DEBUG_HISTS) {
					lockService->RootWriteLock();
					//hDeltaThetaC[locPID]->Fill(tangle-locAngle);
					//hDeltaThetaC_Pixel[locPID]->Fill(channel, tangle-locAngle);
					lockService->RootUnLock();
				}
				
				// remove photon candidates not used in likelihood
				if(locPID==Proton || locPID==AntiProton) {
				  if(fabs(tangle-locExpectedAngle[PiPlus])>DIRC_CUT_ANGLE && fabs(tangle-locExpectedAngle[KPlus])>DIRC_CUT_ANGLE && fabs(tangle-locExpectedAngle[Proton])>DIRC_CUT_ANGLE) continue;
				}
				else if(fabs(tangle-locExpectedAngle[PiPlus])>DIRC_CUT_ANGLE && fabs(tangle-locExpectedAngle[KPlus])>DIRC_CUT_ANGLE) continue;

				isReflected = r;

				// save good photons to matched list
				isGood = true;
				
				// count good photons
				nPhotonsThetaC++;
				
				// calculate average ThetaC and DeltaT
				meanThetaC += tangle;
				meanDeltaT += locDeltaT;
				
				
				// calculate likelihood for each mass hypothesis
				for(uint loc_j = 0; loc_j<dFinalStatePIDs.size(); loc_j++) {
					logLikelihoodSum[dFinalStatePIDs[loc_j]] += TMath::Log( CalcLikelihood(locExpectedAngle[dFinalStatePIDs[loc_j]], tangle));
				}
				
			}
		} // end loop over reflections
	} // end loop over nodes
	
	return locDIRCPhotons;
}

// overloaded function when calculating outside LUT factory
vector<pair<double,double>> DDIRCLut::CalcPhoton(const DDIRCPmtHit *locDIRCHit, double locFlightTime, TVector3 posInBar, TVector3 momInBar, map<Particle_t, double> locExpectedAngle, double locAngle, Particle_t locPID, bool &isReflected, map<Particle_t, double> &logLikelihoodSum) const
{
	int nPhotonsThetaC=0;
	double meanThetaC=0.0, meanDeltaT=0.0;
	bool isGood=false;
	return CalcPhoton(locDIRCHit, locFlightTime, posInBar, momInBar, locExpectedAngle, locAngle, locPID, isReflected, logLikelihoodSum, nPhotonsThetaC, meanThetaC, meanDeltaT, isGood);
}

double DDIRCLut::CalcLikelihood(double locExpectedThetaC, double locThetaC) const {
	
	double locLikelihood = TMath::Exp(-0.5*( (locExpectedThetaC-locThetaC)/DIRC_SIGMA_THETAC * (locExpectedThetaC-locThetaC)/DIRC_SIGMA_THETAC ) ) + 0.00001;

	return locLikelihood;
}

double DDIRCLut::CalcAngle(double locP, double locMass) const {
	return acos(sqrt(locP*locP + locMass*locMass)/locP/dIndex);
}

map<Particle_t, double> DDIRCLut::CalcExpectedAngles(double locP) const {
	
	map<Particle_t, double> locExpectedAngles;
	for(uint loc_i = 0; loc_i<dFinalStatePIDs.size(); loc_i++) {
		locExpectedAngles[dFinalStatePIDs[loc_i]] = acos(sqrt(locP*locP + ParticleMass(dFinalStatePIDs[loc_i])*ParticleMass(dFinalStatePIDs[loc_i]))/locP/dIndex);
	}
	return locExpectedAngles;
}
