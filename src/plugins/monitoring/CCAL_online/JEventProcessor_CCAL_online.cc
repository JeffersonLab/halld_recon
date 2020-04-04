// $Id$
//
//    File: JEventProcessor_CCAL_online.cc
// Created: Fri Nov  9 11:58:09 EST 2012
// Creator: wolin (on Linux stan.jlab.org 2.6.32-279.11.1.el6.x86_64 x86_64)
//


#include "JEventProcessor_CCAL_online.h"



extern "C"{
	void InitPlugin( JApplication *app ) {
		InitJANAPlugin(app);
		app->AddProcessor( new JEventProcessor_CCAL_online() );
	}
}




//----------------------------------------------------------------------------------

jerror_t JEventProcessor_CCAL_online::init( void ) {

	
	TDirectory *main = gDirectory;
	gDirectory->mkdir("ccal")->cd();
	
	
	
	ccal_num_events = new TH1D("ccal_num_events","CCAL Number of Events",1,0.5,1.5);
	
	
	// CCALDigiHit Plots
	
	hdigN           = new TH1I("digN", "CCAL Number of DigiHits; Number of DigiHits; Events", 144, 0, 144 );
	hdigOcc2D       = new TH2F("digOcc2D", "CCAL DigiHit Occupancy; column; row", 12, -0.5, 11.5, 12, -0.5, 11.5 );
	hdigInt         = new TH1I("digInt", "CCAL Pulse Integral", 300, 0, 30000);
	hdigPeak        = new TH1I("digPeak", "CCAL Pulse Peak", 300, 0, 5000);
	hdigT           = new TH1I("digT", "CCAL Pulse Time; Time [4 ns]; Pulses / ns", 4096, 0, 4096);
	hdigPed         = new TH1I("digPed", "CCAL Pedestal (All Channels); ADC Counts; Pulses / 10 Counts", 100, 0, 1000);
	hdigPedChan     = new TProfile("digPedChan", "CCAL Pedestal vs. Channel; ccal id; Average Pedestal [ADC Counts]", 144, -0.5, 143.5);
	hdigPed2D       = new TH2F("digPed2D", "CCAL Pedestals [ADC Counts]; column; row",12, -0.5, 11.5, 12, -0.5, 11.5);
	hdigPedSq2D     = new TH2F("digPedSq2D", "CCAL Pedestals Squared [ADC Counts]; column; row", 12, -0.5, 11.5, 12, -0.5, 11.5);
	hdigIntVsPeak   = new TH2I("digIntVsPeak", "CCAL Pulse Integral vs. Peak Sample; Peak Sample [ADC Units]; Integral [ADC Units]", 200, 0, 4000, 200, 0, 40000 );
	hdigQF          = new TH1I("digQual", "CCAL Hit Quality; Quality Factor Index; Number of Pulses", 16, -0.5, 15.5 );
	
	
	
	// CCALHit Plots
	
	hhitN       = new TH1I("hitN", "CCAL Number of Hits; Number of Hits; Events", 144, 0, 144 );
	hhitE       = new TH1I("hitE", "CCAL Hit Energy; Energy [MeV]; Hits / 100 MeV", 100, 0, 10000 );
	hhitETot    = new TH1I("hitETot", "CCAL Hit Total Energy; Energy [MeV]; Events / 100 MeV", 120, 0, 12000 );
	hhitiop     = new TH1I("hitiop", "CCAL Pulse Integral over Peak", 300, 0, 300);
	hhitT       = new TH1I("hitT", "CCAL Hit Time; t [ns]; Hits / 4 ns", 100, -100, 300 );
	hhitE2D     = new TH2F("hitE2D", "CCAL Hit Average Energy [MeV]; column; row", 12, -0.5, 11.5, 12, -0.5, 11.5 );
	hhitOcc2D   = new TH2F("hitOcc2D", "CCAL Hit Occupancy; column; row", 12, -0.5, 11.5, 12, -0.5, 11.5 );
	
	
	
	// CCALShower Plots
	
	hshowN      = new TH1I("showN", "CCAL Number of showers; Number of Showers; Events", 10, -0.5, 9.5 );
	hshowE      = new TH1I("showE", "CCAL Shower Energy; Energy [MeV]; Showers / 50 MeV", 100, 0, 15000 );
	hshowETot   = new TH1I("showETot", "CCAL Shower Total Energy; Energy [MeV]; Events / 100 MeV", 100, 0, 15000 );
	hshowT      = new TH1I("showT", "CCAL Shower Time; t [ns]; Showers / 4 ns", 100, -100, 300 );
	hshowDime   = new TH1I("showDime", "CCAL Shower Dimension; Modules in Shower", 50, -0.5, 49.5);
	hshowXYHigh = new TH2I("showXYHigh", "CCAL Shower Positions (E > 1 GeV); x [cm]; y [cm]", 100, -12.5, 12.5, 100, -12.5, 12.5 );
	hshowXYLow  = new TH2I("showXYLow", "CCAL Shower Positions (E < 1 GeV); x [cm]; y [cm]", 100, -12.5, 12.5, 100, -12.5, 12.5 );
	hshowPhi    = new TH1I("showPhi", "CCAL Shower #phi; #phi [rad]; Showers / 62.8 mrad", 100, -3.14, 3.14 );
	hshowPhi->SetMinimum( 0 );
	hshow2GMass = new TH1I( "show2GMass", "CCAL 2 Shower Invariant Mass (E > 1 GeV); Invariant Mass [GeV]", 500, 0.0, 0.6 );
	hshow2GMass_fcal = new TH1I("show2GMass_fcal", "Invariant Mass, 1 CCAL & 1 FCAL Shower (E > 1 GeV); Invariant Mass [GeV]", 500, 0.0, 0.6 );
	hshowOccEmax = new TH2I("showOccEmax", "Occupancy when E_{max} > 3 GeV and E_{cl}-E_{max} < 1.0 GeV; column; row", 12, -0.5, 11.5, 12, -0.5, 11.5);
	
	
	
	// Compton Plots
	
	hcomp_bfdt         = new TH1F("comp_bfdt","FCAL Shower Time - Beam Time; t_{fcal}-t_{beam} [ns]; Counts / 0.5 ns", 1000, -50., 50.);
	hcomp_fcdt         = new TH1F("comp_fcdt","CCAL Time - FCAL Time; t_{fcal}-t_{ccal} [ns]; Counts / 0.5 ns", 1000, -50., 50.);
	hcomp_bcdt     = new TH1F("comp_bcdt","CCAL Shower Time - Beam Time; t_{ccal}-t_{beam} [ns]; Counts / 0.5 ns", 1000, -50., 50.);
	
	hcomp_cratio   = new TH1F("comp_cratio", "CCAL Energy over Calculated Compton Energy; #frac{E_{ccal}}{E_{comp}}", 1000, 0., 2.);
	hcomp_cfbratio = new TH1F("comp_cfbratio", "Energy Conservation; #frac{E_{ccal}+E_{fcal}-E_{beam}}{E_{beam}}", 1000, -1., 1.);
	hcomp_cfb2d    = new TH2F("comp_cfb2d", "Reconstructed Energy vs. Calculated Compton Energy; #frac{E_{ccal,comp}+E_{fcal,comp}}{E_{beam}}; #frac{E_{ccal}+E_{fcal}}{E_{beam}}", 
		200, 0., 2., 200, 0., 2.);
	hcomp_pfpc     = new TH1F("comp_pfpc", "FCAL - CCAL Azimuthal Angle; #phi_{fcal}-#phi_{ccal} [deg]; Counts / 0.5 degrees", 1440, -360., 360.);
	hcomp_cxy      = new TH2F("comp_cxy","Reconstructed Compton Positions in CCAL; x_{ccal} [cm]; y_{ccal} [cm]", 200, -12.5, 12.5, 200, -12.5, 12.5);
	hcomp_fxy      = new TH2F("comp_fxy","Reconstructed Compton Positions in FCAL; x_{fcal} [cm]; y_{fcal} [cm]", 200, -50., 50., 200, -50., 50.);
	
	hcomp_cratio_bkgd   = new TH1F("comp_cratio_bkgd", "CCAL Energy over Calculated Compton Energy (Accidentals); #frac{E_{ccal}}{E_{comp}}", 1000, 0., 2.);
	hcomp_cfbratio_bkgd = new TH1F("comp_cfbratio_bkgd", "Energy Conservation (Accidentals); #frac{E_{ccal}+E_{fcal}-E_{beam}}{E_{beam}}", 1000, -1., 1.);
	hcomp_cfb2d_bkgd    = new TH2F("comp_cfb2d_bkgd", "Reconstructed Energy vs. Calculated Compton Energy (Accidentals); #frac{E_{ccal,comp}+E_{fcal,comp}}{E_{beam}}; #frac{E_{ccal}+E_{fcal}}{E_{beam}}", 
		200, 0., 2., 200, 0., 2.);
	hcomp_pfpc_bkgd     = new TH1F("comp_pfpc_bkgd", "FCAL - CCAL Azimuthal Angle (Accidentals); #phi_{fcal}-#phi_{ccal} [deg]; Counts / 0.5 degrees", 1440, -360., 360.);
	hcomp_cxy_bkgd      = new TH2F("comp_cxy_bkgd","Reconstructed Compton Positions in CCAL (Accidentals); x_{ccal} [cm]; y_{ccal} [cm]", 200, -12.5, 12.5, 200, -12.5, 12.5);
	hcomp_fxy_bkgd      = new TH2F("comp_fxy_bkgd","Reconstructed Compton Positions in FCAL (Accidentals); x_{fcal} [cm]; y_{fcal} [cm]", 200, -50., 50., 200, -50., 50.);

	hNPhotons = new TH1I("NPhotons","Number of Beam Photons per Event",80,-0.5,79.5);
	
	
	
	 main->cd();
	
	return NOERROR;
}




//----------------------------------------------------------------------------------


jerror_t JEventProcessor_CCAL_online::brun(JEventLoop *eventLoop, int32_t runnumber) 
{
	
	DGeometry *dgeom = NULL;
	DApplication *dapp = dynamic_cast< DApplication* >( eventLoop->GetJApplication() );
	if( dapp ) dgeom = dapp->GetDGeometry( runnumber );
	
	if( dgeom ){
		
		dgeom->GetTargetZ( m_beamZ );
		
	}
	else{
		
		cerr << "No geometry accessbile to CCAL_online monitoring plugin." << endl;
		return RESOURCE_UNAVAILABLE;
	}
	
	
	jana::JCalibration *jcalib = japp->GetJCalibration(runnumber);
  	std::map<string, float> beam_spot;
  	jcalib->Get("PHOTON_BEAM/beam_spot", beam_spot);
  	m_beamX  =  beam_spot.at("x");
  	m_beamY  =  beam_spot.at("y");
	
	
	
	return NOERROR;
}




//----------------------------------------------------------------------------------


jerror_t JEventProcessor_CCAL_online::evnt(JEventLoop *eventLoop, uint64_t eventnumber) {
	
	
	
	//----------   Check Trigger   ----------//
	
	bool goodtrigger = true;
	
	const DL1Trigger *trig = NULL;
	const DTrigger  *dtrig = NULL;
	
	try {
		eventLoop->GetSingle(trig);
		eventLoop->GetSingle(dtrig);
	} catch (...) {}
	
	if (trig) {
		if( trig->fp_trig_mask ) 
			goodtrigger = false;
		else if( !(dtrig->Get_IsPhysicsEvent()) ) 
			goodtrigger = false;
	} else {
		
		// HDDM files are from simulation, so keep them even though they have no trigger
		bool locIsHDDMEvent = eventLoop->GetJEvent().GetStatusBit(kSTATUS_HDDM);
		if( !locIsHDDMEvent ) goodtrigger = false;	
	}
	
	if( !goodtrigger ) {
		return NOERROR;
	}
	
	
	
	//----------   Get Data Objects   ----------//
	
	vector< const DCCALGeometry* > ccalGeomVec;
  	eventLoop->Get( ccalGeomVec );
	
  	if( ccalGeomVec.size() != 1 ) {
    		cerr << "No CCAL geometry accessbile." << endl;
    		return RESOURCE_UNAVAILABLE;
  	}
	
	const DCCALGeometry *ccalGeom = ccalGeomVec[0];
	
	
	vector< const DCCALDigiHit*  > locDigiHits;
	vector< const DCCALHit*      > locHits;
	vector< const DCCALShower*   > locCCALShowers;
	vector< const DFCALShower*   > locFCALShowers;
	vector< const DBeamPhoton*   > locBeamPhotons;
	
	eventLoop->Get( locDigiHits );
	eventLoop->Get( locHits );
	
	if( locHits.size() <= 50 ) {
		eventLoop->Get( locCCALShowers ); 
		eventLoop->Get( locFCALShowers );
		eventLoop->Get( locBeamPhotons );
	}
	
	
	const DEventRFBunch *locRFBunch = NULL;
	try { 
	  	eventLoop->GetSingle( locRFBunch, "CalorimeterOnly" );
	} catch (...) { return NOERROR; }
	double rfTime = locRFBunch->dTime;
	if( locRFBunch->dNumParticleVotes < 2 ) return NOERROR;
	
	
	
	DVector3 locVertex;
	locVertex.SetXYZ( m_beamX, m_beamY, m_beamZ );
	
	
	
	double hitETot = 0.;
	for( vector< const DCCALHit*>::const_iterator hitItr = locHits.begin();
		hitItr != locHits.end(); ++hitItr ) { hitETot += (**hitItr).E; }
	
	double showETot = 0.;
	for( vector< const DCCALShower*>::const_iterator showItr = locCCALShowers.begin();
		showItr != locCCALShowers.end(); ++showItr ) { showETot += (**showItr).E; }
	
	
	
	
	//----------   Fill Histograms   ----------//
	
	
	japp->RootFillLock(this); //ACQUIRE ROOT FILL LOCK
	
	
	//-----   DigiHit Plots   -----//
	
	
	if( locDigiHits.size() > 0 )
		ccal_num_events->Fill(1);
	
	hdigN->Fill( (int)locDigiHits.size() );  
	
	for( vector< const DCCALDigiHit* >::const_iterator dHitItr = locDigiHits.begin();
		dHitItr != locDigiHits.end(); ++dHitItr ) {
		
		const DCCALDigiHit& dHit = (**dHitItr);
		int chan = ccalGeom->channel( dHit.row, dHit.column );
		
		hdigOcc2D->Fill( dHit.column, dHit.row );
		hdigInt->Fill( dHit.pulse_integral );
		hdigPeak->Fill( dHit.pulse_peak );
		hdigT->Fill( dHit.pulse_time );
		hdigPed->Fill( dHit.pedestal );
		hdigPedChan->Fill( chan, dHit.pedestal );
		hdigPed2D->Fill( dHit.column, dHit.row, dHit.pedestal );
		hdigPedSq2D->Fill( dHit.column, dHit.row, dHit.pedestal*dHit.pedestal );
		hdigQF->Fill( dHit.QF );
		hdigIntVsPeak->Fill( dHit.pulse_peak, dHit.pulse_integral );
		
	}
	
	
	
	//-----   Hit Plots   -----//
	
	
	hhitETot->Fill( hitETot );
	hhitN->Fill( (int)locHits.size() );
	
	for( vector< const DCCALHit* >::const_iterator hit_itr = locHits.begin();
		hit_itr != locHits.end(); ++hit_itr ){
		
		const DCCALHit& hit = (**hit_itr);
		
		hhitE->Fill( hit.E );
		hhitT->Fill( hit.t*k_to_nsec );
		hhitOcc2D->Fill( hit.column, hit.row );
		hhitiop->Fill( hit.intOverPeak );
		hhitE2D->Fill( hit.column, hit.row, hit.E );
		
	}
	
	
	
	// for events with a lot of hits -- stop now
	
	if( locHits.size() > 50 ){
		japp->RootFillUnLock(this); //RELEASE ROOT FILL LOCK
		return NOERROR;
	}
	
	
	
	
	
	//-----   Shower Plots   -----//
	
	
	hshowN->Fill( locCCALShowers.size() );
	
	if( locCCALShowers.size() > 0 )
		hshowETot->Fill( showETot * k_to_MeV );
	
	for( vector< const DCCALShower*>::const_iterator showItr = locCCALShowers.begin();
		showItr != locCCALShowers.end(); ++showItr ){
		
		const DCCALShower& show1 = (**showItr);
		
		double show1X    = show1.x1  -  locVertex.X();
		double show1Y    = show1.y1  -  locVertex.Y();
		double show1Z    = show1.z   -  locVertex.Z();
		
		double show1E    = show1.E;
		double show1Emax = show1.Emax;
		int chan1        = show1.idmax;
		int col1         = chan1%12;
		int row1         = chan1/12;
		int layer1       = ccalLayer( row1, col1 );
		
		DVector3 show1Mom;
		show1Mom.SetX( show1X );
		show1Mom.SetY( show1Y );
		show1Mom.SetZ( show1Z );
		show1Mom.SetMag( show1E );
		
		hshowPhi->Fill( show1Mom.Phi() );
		hshowE->Fill( show1E * k_to_MeV );
		hshowT->Fill( show1.time );
		hshowDime->Fill( show1.dime );
		
		if( show1E > 1. )
			hshowXYHigh->Fill( show1X, show1Y );
		else
			hshowXYLow->Fill( show1X, show1Y );
		
		if( (show1Emax > 3.0) && (show1E - show1Emax < 1.0) )
			hshowOccEmax->Fill( col1, row1 );
		
		if( ( show1E < 1*k_GeV ) ||
			( layer1 == 1  ) ||
			( show1.dime < 2 )
		) continue;
		
		
		for( vector< const DCCALShower*>::const_iterator show2Itr = showItr + 1;
			show2Itr != locCCALShowers.end(); ++show2Itr ){
			
			const DCCALShower& show2 = (**show2Itr);
			
			double show2X  =  show2.x1  -  locVertex.X();
			double show2Y  =  show2.y1  -  locVertex.Y();
			double show2Z  =  show2.z   -  locVertex.Z();
			
			double show2E  =  show2.E;
			int chan2      =  show2.idmax;
			int col2       =  chan2%12;
			int row2       =  chan2/12;
			int layer2     =  ccalLayer( row2, col2 );
			
			if( ( show2E < 1*k_GeV ) ||
				( layer2 == 1  ) ||
				( show2.dime < 2 )
			) continue;
			
			DVector3 show2Mom;
			show2Mom.SetX( show2X );
			show2Mom.SetY( show2Y );
			show2Mom.SetZ( show2Z );
			show2Mom.SetMag( show2.E );
			
			DLorentzVector gam1( show1Mom, show1Mom.Mag() );
			DLorentzVector gam2( show2Mom, show2Mom.Mag() );
			
			hshow2GMass->Fill( ( gam1 + gam2 ).M() );
		}
	}
	
	
	
	//-----   FCAL+CCAL Mgg   -----//
	
	for( vector< const DFCALShower*>::const_iterator showFItr = locFCALShowers.begin();
		showFItr != locFCALShowers.end(); ++showFItr ){
		
		const DFCALShower& showF = (**showFItr);
		
		DVector3 showFMom = showF.getPosition();
		showFMom = showFMom  -  locVertex;
		showFMom.SetMag( showF.getEnergy() );
		
		if( ( showF.getEnergy() < 1*k_GeV ) ||
			( showF.getPosition().Pt() < 10*k_cm ) ) continue;
		
		for( vector< const DCCALShower*>::const_iterator showCItr = locCCALShowers.begin();
			showCItr != locCCALShowers.end(); ++showCItr ){
			
			const DCCALShower& showC = (**showCItr);
			
			DVector3 showCMom;
			showCMom.SetXYZ( showC.x1, showC.y1, showC.z );
			showCMom = showCMom - locVertex;
			showCMom.SetMag( showC.E );
			
			// reject inner layer:
			
			int chan        = showC.idmax;
			int col         = chan%12;
			int row         = chan/12;
			int layer       = ccalLayer( row, col );
			
			if( layer==1 ) continue;
			
			DLorentzVector gam1( showFMom, showFMom.Mag() );
			DLorentzVector gam2( showCMom, showCMom.Mag() );
			
			hshow2GMass_fcal->Fill( ( gam1 + gam2 ).M() );
		}
	}
	
	
	
	//-----   Compton Analysis   -----//
	
	hNPhotons->Fill( locBeamPhotons.size() );
	
	for(unsigned int ih = 0; ih < locFCALShowers.size(); ++ih) {
		
		const DFCALShower *fcal_shower = locFCALShowers[ih];
		
		DVector3 fcal_pos = fcal_shower->getPosition()  -  locVertex;
		double fcal_t     = fcal_shower->getTime()  -  (fcal_pos.Mag() / c);
		double fcal_e     = fcal_shower->getEnergy();
		
		if( fcal_e < 0.4 ) continue;
		
		double phif       = fcal_pos.Phi();
		
		
		for(unsigned int ic = 0; ic < locCCALShowers.size(); ++ic) {
			
			const DCCALShower *ccal_shower = locCCALShowers[ic];
			
			DVector3 ccal_pos;
			ccal_pos.SetXYZ( ccal_shower->x1, ccal_shower->y1, ccal_shower->z );
			ccal_pos = ccal_pos - locVertex;
			
			// reject inner layer:
			
			int chan        = ccal_shower->idmax;
			int col         = chan%12;
			int row         = chan/12;
			int layer       = ccalLayer( row, col );
			
			if( layer==1 ) continue;
			
			
			double ccal_t = ccal_shower->time  -  (ccal_pos.Mag() / c);
			double ccal_e = ccal_shower->E;
			
			if( ccal_e < 3.0 ) continue;
			
			double phic   = ccal_pos.Phi();
			
			double dt_fc_cc = ccal_t - fcal_t;
			hcomp_fcdt->Fill( dt_fc_cc );
			
			if( fabs(fcal_t - rfTime) > 2.004 ) continue;
			if( fabs(ccal_t - rfTime) > 2.004 ) continue;
			
			
			
			for(unsigned int ib = 0; ib < locBeamPhotons.size(); ++ib) {
				
				const DBeamPhoton *beam_photon = locBeamPhotons[ib];
				double beam_e = beam_photon->lorentzMomentum().E();
				double beam_t = beam_photon->time();
				
				double brfdt  = beam_t - rfTime;
				
				double dt_fcal_bm = fcal_t - beam_t;
				double dt_ccal_bm   = ccal_t - beam_t;
				
				hcomp_bfdt->Fill( dt_fcal_bm );
				hcomp_bcdt->Fill( dt_ccal_bm );
				
				if( beam_e < 6. ) continue;
				
				double ecompf  =  1. / ( (1./beam_e)  +  (1./m_e)*(1. - cos(fcal_pos.Theta())) );
				double ecompc  =  1. / ( (1./beam_e)  +  (1./m_e)*(1. - cos(ccal_pos.Theta())) );
				
				
				if( fabs(brfdt) <  2.004 ) {
					
					hcomp_cratio->Fill( ccal_e/ecompc );
					hcomp_cfbratio->Fill( (ccal_e+fcal_e-beam_e)/beam_e );
					hcomp_cxy->Fill( ccal_pos.X(), ccal_pos.Y() );
					hcomp_fxy->Fill( fcal_pos.X(), fcal_pos.Y() );
					hcomp_cfb2d->Fill( (ecompc+ecompf)/beam_e, (ccal_e+fcal_e)/beam_e );
					hcomp_pfpc->Fill( (phif-phic)*180./TMath::Pi() );
					
				} else if( (  -(6.012 + 2.*4.008) <= brfdt && brfdt <= -(6.012 + 1.*4.008) )
					|| (   (6.012 + 1.*4.008) <= brfdt && brfdt <=  (6.012 + 2.*4.008) ) ) {
					
					hcomp_cratio_bkgd->Fill( ccal_e/ecompc );
					hcomp_cfbratio_bkgd->Fill( (ccal_e+fcal_e-beam_e)/beam_e );
					hcomp_cxy_bkgd->Fill( ccal_pos.X(), ccal_pos.Y() );
					hcomp_fxy_bkgd->Fill( fcal_pos.X(), fcal_pos.Y() );
					hcomp_cfb2d_bkgd->Fill( (ecompc+ecompf)/beam_e, (ccal_e+fcal_e)/beam_e );
					hcomp_pfpc_bkgd->Fill( (phif-phic)*180./TMath::Pi() );
					
				}
			}
		}
	}
	
	
	
	japp->RootFillUnLock(this); //RELEASE ROOT FILL LOCK
	return NOERROR;
}




//----------------------------------------------------------------------------------


jerror_t JEventProcessor_CCAL_online::erun( void ) 
{
	
	return NOERROR;
}



jerror_t JEventProcessor_CCAL_online::fini( void ) 
{
	
	return NOERROR;
}



int JEventProcessor_CCAL_online::ccalLayer( int row, int col ) 
{
	
	int layer;
	
	if( (row > 3 && row < 8) && (col > 3 && col < 8) ) layer = 1;
	else if( (row > 2 && row <  9) && (col > 2 && col <  9) ) layer = 2;
	else if( (row > 1 && row < 10) && (col > 1 && col < 10) ) layer = 3;
	else if( (row > 0 && row < 11) && (col > 0 && col < 11) ) layer = 4;
	else layer = 5;	
	
	return layer;
}





