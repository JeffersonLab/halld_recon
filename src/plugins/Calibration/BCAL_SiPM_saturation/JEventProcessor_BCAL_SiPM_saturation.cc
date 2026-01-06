// $Id$
//
//    File: JEventProcessor_BCAL_SiPM_saturation.cc
//          Modified file from BCAL_neutron_discriminator.cc   ES  5/10/2018
// Created: Thu Apr  5 16:36:00 EDT 2018
// Creator: dalton (on Linux gluon119.jlab.org 2.6.32-642.3.1.el6.x86_64 x86_64)
//

#include "JEventProcessor_BCAL_SiPM_saturation.h"
#include "TRACKING/DMCThrown.h"
// #include "BCAL/DBCALHit.h"
// #include "BCAL/DBCALTDCHit.h"
// #include "BCAL/DBCALCluster.h"
// #include "BCAL/DBCALDigiHit.h"
#include "BCAL/DBCALPoint.h"
// #include "BCAL/DBCALUnifiedHit.h"
// #include "BCAL/DBCALGeometry.h"
#include "BCAL/DBCALShower.h"
#include "DANA/DStatusBits.h"
#include "DANA/DEvent.h"
// #include "PID/DChargedTrack.h"
// #include "PID/DEventRFBunch.h"
// #include "PID/DDetectorMatches.h"
#include "PID/DNeutralShower.h"
// #include "PID/DVertex.h"
// #include "TRACKING/DTrackTimeBased.h"
// #include "TRIGGER/DL1Trigger.h"


// Routine used to create our JEventProcessor
#include <JANA/JApplication.h>
#include <JANA/JFactoryT.h>
extern "C"{
void InitPlugin(JApplication *app){
	InitJANAPlugin(app);
	app->Add(new JEventProcessor_BCAL_SiPM_saturation());
}
} // "C"


//------------------
// JEventProcessor_BCAL_SiPM_saturation (Constructor)
//------------------
JEventProcessor_BCAL_SiPM_saturation::JEventProcessor_BCAL_SiPM_saturation()
{
	SetTypeName("JEventProcessor_BCAL_SiPM_saturation");
}

//------------------
// ~JEventProcessor_BCAL_SiPM_saturation (Destructor)
//------------------
JEventProcessor_BCAL_SiPM_saturation::~JEventProcessor_BCAL_SiPM_saturation()
{

}

//------------------
// Init
//------------------
void JEventProcessor_BCAL_SiPM_saturation::Init()
{
	// This is called once at program startup. 
	VERBOSE = 0;
	auto app = GetApplication();
	app->SetDefaultParameter("BCAL_SiPM_saturation:VERBOSE", VERBOSE, "Verbosity level");
	Int_t nbins=100;

	// This is called once at program startup. 
	gDirectory->mkdir("BCAL_SiPM_saturation");
	gDirectory->cd("BCAL_SiPM_saturation");

	gDirectory->mkdir("Hists1D");
	gDirectory->cd("Hists1D");

    dHistEthrown = new TH1F("Ethrown", "BCAL SiPM Saturation; Thrown Energy (GeV)",4*nbins,0,10);
    dHistEshower = new TH1F("Eshower", "BCAL SiPM Saturation; Shower Energy (GeV)",10*nbins,0,10);
    dHistThrownTheta = new TH1F("Thrown Theta", "BCAL SiPM Saturation; Thrown Theta (degrees)",4*90,0,90);
    dHistNCell = new TH1F("NCell", "BCAL SiPM Saturation; Number of cells",nbins,0,100 );
    dHistLayer = new TH1F("layer", "BCAL SiPM Saturation; Layer Number",5,0,5);
    dHistEpoint = new TH1F("Ept", "BCAL SiPM Saturation; Point Energy (GeV)",10*nbins,0,10);


   	nbins=5100;

    dHistPeakLayer1 = new TH1F("Hit pulse_peak layer=1", "BCAL SiPM Saturation; Hit Pulse_peak (counts)",nbins,-100,5000);
    dHistIntegralLayer1 = new TH1F("Hit integral layer=1", "BCAL SiPM Saturation; Hit integral (GeV)",nbins,0, 10);
    dHistPeakLayer2 = new TH1F("Hit pulse_peak layer=2", "BCAL SiPM Saturation; Hit Pulse_peak (counts)",nbins,-100,5000);
    dHistIntegralLayer2 = new TH1F("Hit integral layer=2", "BCAL SiPM Saturation; Hit integral (GeV)",nbins,0, 10);
    dHistPeakLayer3 = new TH1F("Hit pulse_peak layer=3", "BCAL SiPM Saturation; Hit Pulse_peak (counts)",nbins,-100,5000);
    dHistIntegralLayer3 = new TH1F("Hit integral layer=3", "BCAL SiPM Saturation; Hit integral (GeV)",nbins,0, 10);
    dHistPeakLayer4 = new TH1F("Hit pulse_peak layer=4", "BCAL SiPM Saturation; Hit Pulse_peak (counts)",nbins,-100,5000);
    dHistIntegralLayer4 = new TH1F("Hit integral layer=4", "BCAL SiPM Saturation; Hit integral (GeV)",nbins,0, 10);
    dHistEcalc = new TH1F("Ecalc", "BCAL SiPM Saturation; Calc Energy (GeV)",4*nbins,0,10);
    dHistEcalcEpt = new TH1F("Ecalc-Ept", "BCAL SiPM Saturation; Calc-Pt Energy (GeV)",nbins,-0.05,0.05);

	gDirectory->cd("..");

	gDirectory->mkdir("Hists2D");
	gDirectory->cd("Hists2D");

   	nbins=100;

    dHistEshowerVsEthrown = new TH2F("Eshower_vs_Ethrown",  "BCAL SiPM Saturation; Thrown Energy (GeV); Shower Energy (GeV)",
                         4*nbins,0,10,4*nbins,0,10);
    dHistEdiffVsEthrown = new TH2F("EDiff_vs_Ethrown",  "BCAL SiPM Saturation; Thrown Energy (GeV); (Shower - Thrown) Energy (GeV)",
                         4*nbins,0,10,nbins,-0.5,0.5);
    dHistEdiffFracVsEthrown = new TH2F("EDiff/Ethrown_vs_Ethrown", "BCAL SiPM Saturation; Thrown Energy (GeV); (EShower - EThrown)/Ethrown",
                         4*nbins,0,10,nbins,-0.2,0.2);

	gDirectory->cd("..");

	gDirectory->cd("..");

	return; //NOERROR;
}

//------------------
// BeginRun
//------------------
void JEventProcessor_BCAL_SiPM_saturation::BeginRun(const std::shared_ptr<const JEvent>& event)
{
	// This is called whenever the run number changes

    attenuation_parameters.clear();
    DEvent::GetCalib(event, "/BCAL/attenuation_parameters", attenuation_parameters);
    /*int channel = 0;
       for (int module=1; module<=48; module++) {
           for (int layer=1; layer<=4; layer++) {
               for (int sector=1; sector<=4; sector++) {
                   printf("%2i  %2i  %2i %12.4f %12.4f %12.4f\n",
                          module,layer,sector,
                          attenuation_parameters[channel][0],
                          attenuation_parameters[channel][1],
                          attenuation_parameters[channel][2]);
               }
               channel++;
           }
	   }*/
}

//------------------
// Process
//------------------
void JEventProcessor_BCAL_SiPM_saturation::Process(const std::shared_ptr<const JEvent>& event)
{

auto lockService = DEvent::GetLockService(event);
  // If not Physics event call BCAL showers to initialize parameters
  if (!DEvent::GetStatusBit(event, kSTATUS_PHYSICS_EVENT)) {
  	vector<const DBCALShower*> BCALShowers;
  	event->Get(BCALShowers);
  }

  // Check to see if this is a physics event
  if (DEvent::GetStatusBit(event, kSTATUS_PHYSICS_EVENT)) {

  // Check if thrown information is available
  vector<const DMCThrown*> MCThrowns;
  event->Get(MCThrowns);
  unsigned int NumThrown=MCThrowns.size();
  double Ethrown=0;
  double thetathrown=0;
  if (NumThrown == 1) {
    const DMCThrown* locMCThrown = MCThrowns[0];
    Ethrown = locMCThrown->energy();
    DVector3 pgen = locMCThrown->momentum();
    //pgen.Print();
    thetathrown = 180./3.14159*pgen.Theta();
  }

  if (NumThrown==0 || thetathrown > 12) {   // Either data or ignore MC photons hitting end of bcal

    // Get vector of neutral showers in event
  	vector<const DNeutralShower*> NeutralShowers;
  	event->Get(NeutralShowers);

    unsigned int NumShowers=NeutralShowers.size();
    float Eshower = 0;

    // loop over neutral showers
    for (unsigned int i = 0; i < NumShowers; i++){
        const DNeutralShower* locNeutralShower = NeutralShowers[i];
        // Must be BCAL shower
        DetectorSystem_t locDetector = locNeutralShower->dDetectorSystem;
        if (locDetector != SYS_BCAL) continue;
        // Get shower properties
        vector<const DBCALShower*> BCALShowers;
		locNeutralShower->Get(BCALShowers);

		// Should be only one BCAL shower for each neutral shower
		const DBCALShower* locBCALShower= BCALShowers[0];
			Eshower = locBCALShower->E;

		// for MC select showers that are greater than 50% of thrown energy
		if (NumThrown == 1 && Eshower < Ethrown/2.) 
		  continue;
   
        /*float E_preshower = locBCALShower->E_preshower;
        float z = locBCALShower->z;
        float x = locBCALShower->x;
        float y = locBCALShower->y;
        float R = sqrt(x*x+y*y);
        float sigLong = locBCALShower->sigLong;    // rho
        float sigTrans = locBCALShower->sigTrans;  // phi
        float sigTheta = locBCALShower->sigTheta;*/

        // cout << " Shower i=" << i << " E_shower=" << Eshower << " E_preshower=" << E_preshower << " x=" << x << " y=" << y << " z=" << z << " R=" << R << " sigLong=" << sigLong << " sigTrans=" << sigTrans << " sigTheta=" << sigTheta << endl;

        // Get vector of points in this shower 
        // load these earlier to collect the histogram fill calls below
        vector<const DBCALPoint*> Points;
		locNeutralShower->Get(Points);
        uint Ncell = Points.size();
        
    	lockService->RootFillLock(this); //ACQUIRE ROOT LOCK!!

		// Fill histogram for showers
		dHistEthrown->Fill(Ethrown);
		dHistEshower->Fill(Eshower);
		dHistThrownTheta->Fill(thetathrown);

        // Fill 2D histograms
		dHistEshowerVsEthrown->Fill(Ethrown, Eshower);
		dHistEdiffVsEthrown->Fill(Ethrown, Eshower - Ethrown);
		dHistEdiffFracVsEthrown->Fill(Ethrown, Ethrown>0? (Eshower - Ethrown)/Ethrown : 0);
	
        dHistNCell->Fill(Ncell);

    	lockService->RootFillUnLock(this); //RELEASE ROOT LOCK


        for (unsigned int j = 0; j < Ncell; j++){
            const DBCALPoint* locPoint = Points[j];
            float t = locPoint->t();
            float z = locPoint->z();
			float Ept = locPoint->E();
			int module= locPoint->module();
			int layer = locPoint->layer();
			int sector = locPoint->sector();
			// cout << " j=" << j << " t=" << t << endl;

			// calculate point energy (code taken from DBCALPoint.cc)
			float z_bcal_center = 212;
			float fibLen = 390;
			float zLocal = z - z_bcal_center; 


			float dUp = 0.5 * fibLen + zLocal;
			float dDown = 0.5 * fibLen - zLocal;
			if (dUp>fibLen)   dUp=fibLen;
			if (dUp<0)        dUp=0;
			if (dDown>fibLen) dDown=fibLen;
			if (dDown<0)      dDown=0;
			int channel = (module-1)*16 + (layer-1)*4 + (sector-1);
			// cout << " module=" << module << " layer=" << layer << " sector=" << sector << " channel=" << channel << endl;
			// cout << " attenuation length 00=" << attenuation_parameters[channel][0] << endl;
			float attenuation_length = attenuation_parameters[channel][0];
			float attUp = exp( -dUp / attenuation_length );
			float attDown = exp( -dDown / attenuation_length );
 

			if (VERBOSE>=3) cout << " VERBOSE >=3" << " t=" << t << " z=" << z << endl;


    		lockService->RootFillLock(this); //ACQUIRE ROOT LOCK!!
	
			// Fill 1D histograms
			dHistLayer->Fill(layer);
			dHistEpoint->Fill(Ept);

	    	lockService->RootFillUnLock(this); //RELEASE ROOT LOCK

			// cout << " Point: Ept=" << Ept << endl;
			float upHit=0;
			float downHit=0;
			// following two lines commented out to supress warning
			//	float uppeak=0;
			//	float downpeak=0;

            vector<const DBCALHit*> Hits;
			locPoint->Get(Hits);
            uint Nhits = Hits.size();

            for (unsigned int j = 0; j < Nhits; j++){
                const DBCALHit* locHit = Hits[j];
	        float Ehit = locHit->E;
	        /*int module = locHit->module;
	        int layer = locHit->layer;
	        int sector = locHit->sector;*/
	        int end = locHit->end;
	        int pulse_peak = locHit->pulse_peak;

			// cout << " module=" << module << " layer=" << layer << " sector=" << sector << " pulse_peak/Ehit=" << pulse_peak/Ehit << endl;

			if (end == 0) {
			  upHit = Ehit;
			  // following line commented out to supress warning
			  //		  uppeak = pulse_peak;
			}
			if (end == 1) {
			  downHit = Ehit;
			  // following line commented out to supress warning
			  //		  downpeak = pulse_peak;
			}


    		lockService->RootFillLock(this); //ACQUIRE ROOT LOCK!!

            // Fill 1D histograms
			if (layer == 1) {
				dHistPeakLayer1->Fill(pulse_peak);
				dHistIntegralLayer1->Fill(Ehit);
			}
			else if (layer == 2) {
				dHistPeakLayer2->Fill(pulse_peak);
				dHistIntegralLayer2->Fill(Ehit);
			}
			else if (layer == 3) {
				dHistPeakLayer3->Fill(pulse_peak);
				dHistIntegralLayer3->Fill(Ehit);
			}
			else if (layer == 4) {
				dHistPeakLayer4->Fill(pulse_peak);
				dHistIntegralLayer4->Fill(Ehit);
			}
			else {
			  cout << " ***Illegal layer=" << layer << endl;
			}

	    	lockService->RootFillUnLock(this); //RELEASE ROOT LOCK

	     }



		// use these to correct the energy
		float E_US =  ( upHit / attUp );
		float E_DS =  ( downHit / attDown );
		float Ecalc =  ( E_US + E_DS ) / 2;

		/*cout << " i=" << i << " module=" << module <<  " layer=" <<  layer << " sector=" << sector 
                     << " lambda=" << attenuation_length << " Eup=" << upHit << " Edown=" << downHit << " Point: Ept=" 
                     << Ept << " Ecalc=" << Ecalc << " Diff=" << Ept-Ecalc <<  endl;*/

    	lockService->RootFillLock(this); //ACQUIRE ROOT LOCK!!
		dHistEcalc->Fill(Ecalc);
		dHistEcalcEpt->Fill(Ecalc-Ept);
		lockService->RootFillUnLock(this); //RELEASE ROOT LOCK

	}



    }
  }
  }
}

//------------------
// EndRun
//------------------
void JEventProcessor_BCAL_SiPM_saturation::EndRun()
{
	// This is called whenever the run number changes, before it is
	// changed to give you a chance to clean up before processing
	// events from the next run number.
}

//------------------
// Finish
//------------------
void JEventProcessor_BCAL_SiPM_saturation::Finish()
{
	// Called before program exit after event processing is finished.
}

