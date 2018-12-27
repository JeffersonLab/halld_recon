// $Id$
//
//    File: DDIRCLut.cc
//

#include <cassert>
#include <math.h>
using namespace std;

#include "DDIRCLut.h"
#include "DANA/DApplication.h"
#include <JANA/JCalibration.h>

//---------------------------------
// DDIRCLut    (Constructor)
//---------------------------------
DDIRCLut::DDIRCLut() 
{
	DIRC_DEBUG_HISTS = false;
	gPARMS->SetDefaultParameter("DIRC:DEBUG_HISTS",DIRC_DEBUG_HISTS);

	DIRC_TRUTH_BARHIT = false;
	gPARMS->SetDefaultParameter("DIRC:TRUTH_BARHIT",DIRC_TRUTH_BARHIT);

	DIRC_TRUTH_PIXELTIME = false;
	gPARMS->SetDefaultParameter("DIRC:TRUTH_PIXELTIME",DIRC_TRUTH_PIXELTIME);

	// set PID for different passes in debuging histograms
	dFinalStatePIDs.push_back(Positron);
	dFinalStatePIDs.push_back(PiPlus);
	dFinalStatePIDs.push_back(KPlus);
	dFinalStatePIDs.push_back(Proton);

	if(DIRC_DEBUG_HISTS) 	
		CreateDebugHistograms();	
}

bool DDIRCLut::brun(JEventLoop *loop) {

	dapp = dynamic_cast<DApplication*>(loop->GetJApplication());
	dDIRCLutReader = dapp->GetDIRCLut(loop->GetJEvent().GetRunNumber());
	
	// get DIRC geometry
	vector<const DDIRCGeometry*> locDIRCGeometry;
        loop->Get(locDIRCGeometry);
        dDIRCGeometry = locDIRCGeometry[0];
	
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
		hDiff_Pixel = (TH2I*)gROOT->FindObject("hDiff_Pixel");
		hDiffT = (TH1I*)gROOT->FindObject("hDiffT");
		hDiffD = (TH1I*)gROOT->FindObject("hDiffD");
		hDiffR = (TH1I*)gROOT->FindObject("hDiffR");
		hTime = (TH1I*)gROOT->FindObject("hTime");
		hCalc = (TH1I*)gROOT->FindObject("hCalc");
		hNph = (TH1I*)gROOT->FindObject("hNph");
		if(!hDiff) hDiff = new TH1I("hDiff",";t_{calc}-t_{measured} [ns];entries [#]", 400,-20,20);
		if(!hDiff_Pixel) hDiff_Pixel = new TH2I("hDiff_Pixel","; Pixel ID; t_{calc}-t_{measured} [ns];entries [#]", 10000, 0, 10000, 400,-20,20);
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
			
			hDeltaThetaC[loc_i] = (TH1I*)gROOT->FindObject(Form("hDeltaThetaC_%s",locParticleName.data()));
			if(!hDeltaThetaC[loc_i]) 
				hDeltaThetaC[loc_i] = new TH1I(Form("hDeltaThetaC_%s",locParticleName.data()),  "cherenkov angle; #Delta#theta_{C} [rad]", 200,-0.5,0.5);
			hDeltaThetaC_Pixel[loc_i] = (TH2I*)gROOT->FindObject(Form("hDeltaThetaC_Pixel_%s",locParticleName.data()));
			if(!hDeltaThetaC_Pixel[loc_i]) 
				hDeltaThetaC_Pixel[loc_i] = new TH2I(Form("hDeltaThetaC_Pixel_%s",locParticleName.data()),  "cherenkov angle; Pixel ID; #Delta#theta_{C} [rad]", 10000, 0, 10000, 200,-0.5,0.5);
		}

		//mainDir->cd();
	}
	//dapp->RootUnLock(); //REMOVE ROOT LOCK!!
	
	return true;
}

bool DDIRCLut::CalcLUT(TVector3 locProjPos, TVector3 locProjMom, const vector<const DDIRCPmtHit*> locDIRCHits, double locFlightTime, Particle_t locPID, shared_ptr<DDIRCMatchParams>& locDIRCMatchParams, const vector<const DDIRCTruthBarHit*> locDIRCBarHits) const
{
	double locMass = ParticleMass(locPID);

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
	
	double tangle,luttheta,evtime;
	int64_t pathid; 
	TVector3 dir,dird;
	double criticalAngle = asin(1.00028/1.47125); // n_quarzt = 1.47125; //(1.47125 <==> 390nm)
	
	double mAngle = acos(sqrt(momInBar.Mag()*momInBar.Mag() + locMass*locMass)/momInBar.Mag()/1.473);

	// expected angles for electron, pion, kaon and proton
	double locExpectedAngle[4];
	int locHypothesisIndex = -1;
	for(int loc_i = 0; loc_i<4; loc_i++) {
		locExpectedAngle[loc_i] = acos(sqrt(momInBar.Mag()*momInBar.Mag() + ParticleMass(dFinalStatePIDs[loc_i])*ParticleMass(dFinalStatePIDs[loc_i]))/momInBar.Mag()/1.473);
		if(fabs(ParticleMass(dFinalStatePIDs[loc_i])-ParticleMass(locPID)) < 0.01) locHypothesisIndex = loc_i;
	}

	TVector3 fnX1 = TVector3 (1,0,0);
	TVector3 fnY1 = TVector3 (0,1,0);
	TVector3 fnZ1 = TVector3 (0,0,1);

	// timing cuts for photons
	double cut_tdiffd=2; // direct cut in ns
	double cut_tdiffr=3; // reflected cut in ns

	// clear object to store good photons
	if(locDIRCMatchParams == nullptr)
		locDIRCMatchParams = std::make_shared<DDIRCMatchParams>();
	locDIRCMatchParams->dPhotons.clear();

	// loop over DIRC hits
	double logLikelihoodSum[4] = {0, 0, 0, 0};
	int nPhotons = 0;
	int nPhotonsThetaC = 0;
	int nPhotonsThetaCLoose = 0;
	double meanThetaC = 0.;
	for (unsigned int loc_i = 0; loc_i < locDIRCHits.size(); loc_i++){

		const DDIRCPmtHit* locDIRCHit = locDIRCHits[loc_i];
		vector<const DDIRCTruthPmtHit*> locTruthDIRCHits;
		locDIRCHit->Get(locTruthDIRCHits);

		// cheat and determine bar from truth info (replace with Geometry->GetBar(X,Y) function)
		int bar = dDIRCGeometry->GetBar(posInBar.Y()); //locTruthDIRCHits[0]->key_bar;
		if(bar < 0 || bar > 47) continue;

                // get channel information for LUT
		int sensorId = locDIRCHit->ch;

		// use hit time to determine if reflected or not
		double hitTime = locDIRCHit->t - locFlightTime;

		// option to use truth hit time rather than smeared hit time
		if(DIRC_TRUTH_PIXELTIME && locTruthDIRCHits.size() > 0) {
			double locTruthTime = locTruthDIRCHits[0]->t - locFlightTime;
			hitTime = locTruthTime;          
		}

		// needs to be X dependent choice for reflection cut (from CCDB?)
		bool reflected = hitTime>38;
		
		/*
		  Need to double check all position units!
		  GEANT4 is in mm, but GlueX default (ie. tracks, etc.) is in cm...
		*/

		// get position along bar for calculated time 
		double radiatorL = 4*1225; // get from CCDB
		double barend = 2940; // get from CCDB
		double lenz = 0;
		if(posInBar.Y() < 0) 
			lenz = fabs(barend + posInBar.X()*10);  
		else 
			lenz = fabs(posInBar.X()*10 - barend);

		// get length for reflected and direct photons
		double rlenz = 2*radiatorL - lenz; // reflected
		double dlenz = lenz; // direct
		
		if(reflected) lenz = 2*radiatorL - lenz;
		
		bool isGood(false);

		// check for pixel before going through loop
		if(dDIRCLutReader->GetLutPixelAngleSize(bar, sensorId) == 0) continue;
		
		// loop over LUT table for this bar/pixel to calculate thetaC	     
		for(uint i = 0; i < dDIRCLutReader->GetLutPixelAngleSize(bar, sensorId); i++){

			dird   = dDIRCLutReader->GetLutPixelAngle(bar, sensorId, i); 
			evtime = dDIRCLutReader->GetLutPixelTime(bar, sensorId, i); 
			pathid = dDIRCLutReader->GetLutPixelPath(bar, sensorId, i); 
			
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
					if(dir.Angle(fnY1) < criticalAngle || dir.Angle(fnZ1) < criticalAngle) continue;
					
					luttheta = dir.Angle(TVector3(-1,0,0));
					if(luttheta > TMath::PiOver2()) luttheta = TMath::Pi()-luttheta;
					tangle = momInBar.Angle(dir);//-0.002; //correction
					
					double bartime = lenz/cos(luttheta)/198.0; //203.767; 
					double totalTime = bartime+evtime;

					// calculate time difference
					double locDeltaT = totalTime-hitTime;

					if(DIRC_DEBUG_HISTS) {	
						dapp->RootWriteLock(); 
						hTime->Fill(hitTime);
						hCalc->Fill(totalTime);
						
						if(fabs(tangle-0.5*(locExpectedAngle[1]+locExpectedAngle[2]))<0.2){
							hDiff->Fill(locDeltaT);
							hDiff_Pixel->Fill(sensorId, locDeltaT);
							if(samepath){
								hDiffT->Fill(locDeltaT);
								if(r) hDiffR->Fill(locDeltaT);
								else hDiffD->Fill(locDeltaT);
							}
						}
						dapp->RootUnLock();
					}
					
					// save hits array which pass some lose time and angle criteria
					vector<double> photonInfo;
					photonInfo.push_back(tangle);  
					photonInfo.push_back(locDeltaT); 
					photonInfo.push_back(sensorId); 
					photonInfo.push_back(hitTime);
					if(fabs(locDeltaT) < 20.0 && fabs(tangle-0.5*(locExpectedAngle[1]+locExpectedAngle[2]))<0.2) 
						locDIRCMatchParams->dPhotons.push_back(photonInfo);

					// reject photons that are too far out of time
					if(!r && fabs(locDeltaT)>cut_tdiffd) continue;
					if(r && fabs(locDeltaT) >cut_tdiffr) continue;

					if(DIRC_DEBUG_HISTS) {
						dapp->RootWriteLock();
					        hDeltaThetaC[locHypothesisIndex]->Fill(tangle-mAngle);
						hDeltaThetaC_Pixel[locHypothesisIndex]->Fill(sensorId, tangle-mAngle);
						dapp->RootUnLock();
					}
					
					// calculate average thetaC
					nPhotonsThetaCLoose++;
					meanThetaC += tangle;

					// remove photon candidates not used in likelihood
					if(fabs(tangle-0.5*(locExpectedAngle[1]+locExpectedAngle[2]))>0.02) continue;
					
					// save good photons to DIRCLutPhotons object
					isGood = true;
					
					// count good photons
					nPhotonsThetaC++;
					
					// calculate likelihood for each mass hypothesis
					for(int loc_j = 0; loc_j<4; loc_j++) {
						logLikelihoodSum[loc_j] += TMath::Log( CalcLikelihood(locExpectedAngle[loc_j], tangle));
					}

				}
			} // end loop over reflections
		} // end loop over nodes

		// count good photons
		if(isGood) nPhotons++;

	} // end loop over hits

	if(DIRC_DEBUG_HISTS) {
		dapp->RootWriteLock();
		hNph->Fill(nPhotons);
		dapp->RootUnLock();
	}

	// skip tracks without enough photons
	if(nPhotons<5) 
		return false;

	// set DIRCMatchParameters contents
	locDIRCMatchParams->dThetaC = meanThetaC/(double)nPhotonsThetaCLoose/2.; // why factor 2?
	locDIRCMatchParams->dExpectedThetaC = mAngle;
	locDIRCMatchParams->dLikelihoodElectron = logLikelihoodSum[0];
	locDIRCMatchParams->dLikelihoodPion = logLikelihoodSum[1];
	locDIRCMatchParams->dLikelihoodKaon = logLikelihoodSum[2];
	locDIRCMatchParams->dLikelihoodProton = logLikelihoodSum[3];
	locDIRCMatchParams->dNPhotons = nPhotons;
	locDIRCMatchParams->dExtrapolatedPos = posInBar;
	locDIRCMatchParams->dExtrapolatedMom = momInBar;
	locDIRCMatchParams->dExtrapolatedTime = locFlightTime;

	return true;
}

double DDIRCLut::CalcLikelihood(double locExpectedThetaC, double locThetaC) const {
	
	double locSigmaThetaC = 0.0085; // sigma (thetaC for single photon)
	double locLikelihood = TMath::Exp(-0.5*( (locExpectedThetaC-locThetaC)/locSigmaThetaC * (locExpectedThetaC-locThetaC)/locSigmaThetaC ) ) + 0.00001;

	return locLikelihood;
}
