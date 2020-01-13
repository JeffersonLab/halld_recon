
#include "JEventProcessor_compton_simple.h"

extern "C"{
void InitPlugin(JApplication *app){
	InitJANAPlugin(app);
    	app->AddProcessor(new JEventProcessor_compton_simple());
}
} // "C"




//------------------
// init
//------------------
jerror_t JEventProcessor_compton_simple::init(void)
{
  	
	
	h_deltaPhi  = new TH1I( "deltaPhi", "#Delta#phi; #phi_{CCAL} - #phi_{FCAL} [deg.]", 5000, -250., 250. );
	h_deltaT    = new TH1I( "deltaT",   "#DeltaT; t_{FCAL} - t_{CCAL} [ns]", 1000, -20., 20. );
	
	h_beam_fcal_dt = new TH1I( "beam_fcal_dt", "t_{#gamma} - t_{FCAL}; [ns]", 1000, -20., 20. );
	h_beam_ccal_dt = new TH1I( "beam_ccal_dt", "t_{#gamma} - t_{CCAL}; [ns]", 1000, -20., 20. );
	
	
	
	
	
	
	
	h_deltaE    = new TH1I( "deltaE",   "#DeltaE; (E_{FCAL} + E_{CCAL} - E_{#gamma} - m_{e}) / (E_{#gamma} - m_{e})", 1000, -1.0, 1.0 );
	h_deltaE_e  = new TH1I( "deltaE_e", "#DeltaE (TOF/FCAL Match); (E_{FCAL} + E_{CCAL} - E_{#gamma} - m_{e}) / (E_{#gamma} - m_{e})", 1000, -1.0, 1.0 );
	h_deltaE_g  = new TH1I( "deltaE_g", "#DeltaE (No TOF/FCAL Match); (E_{FCAL} + E_{CCAL} - E_{#gamma} - m_{e}) / (E_{#gamma} - m_{e})", 1000, -1.0, 1.0 );
	
	h_deltaE_acc    = new TH1I( "deltaE_acc",   "#DeltaE (accidentals); (E_{FCAL} + E_{CCAL} - E_{#gamma} - m_{e}) / (E_{#gamma} - m_{e})", 1000, -1.0, 1.0 );
	h_deltaE_e_acc  = new TH1I( "deltaE_e_acc", "#DeltaE (accidentals) (TOF/FCAL Match); (E_{FCAL} + E_{CCAL} - E_{#gamma} - m_{e}) / (E_{#gamma} - m_{e})", 1000, -1.0, 1.0 );
	h_deltaE_g_acc  = new TH1I( "deltaE_g_acc", "#DeltaE (accidentals) (No TOF/FCAL Match); (E_{FCAL} + E_{CCAL} - E_{#gamma} - m_{e}) / (E_{#gamma} - m_{e})", 1000, -1.0, 1.0 );
	
	
	
	h_deltaE_vs_beamE    = new TH2I( "deltaE_vs_beamE", "#DeltaE vs. E_{#gamma}", 120, 0., 12., 1000, -1.0, 1.0 );
	h_deltaE_vs_beamE->GetXaxis()->SetTitle( "E_{#gamma} [GeV]" );
	h_deltaE_vs_beamE->GetYaxis()->SetTitle( "(E_{FCAL} + E_{CCAL} - E_{#gamma} - m_{e}) / (E_{#gamma} - m_{e})" );
	
	h_deltaE_vs_beamE_e  = new TH2I( "deltaE_vs_beamE_e", "#DeltaE vs. E_{#gamma} (TOF/FCAL Match)", 120, 0., 12., 1000, -1.0, 1.0 );
	h_deltaE_vs_beamE_e->GetXaxis()->SetTitle( "E_{#gamma} [GeV]" );
	h_deltaE_vs_beamE_e->GetYaxis()->SetTitle( "(E_{FCAL} + E_{CCAL} - E_{#gamma} - m_{e}) / (E_{#gamma} - m_{e})" );
	
	h_deltaE_vs_beamE_g  = new TH2I( "deltaE_vs_beamE_g", "#DeltaE vs. E_{#gamma} (No TOF/FCAL Match)", 120, 0., 12., 1000, -1.0, 1.0 );
	h_deltaE_vs_beamE_g->GetXaxis()->SetTitle( "E_{#gamma} [GeV]" );
	h_deltaE_vs_beamE_g->GetYaxis()->SetTitle( "(E_{FCAL} + E_{CCAL} - E_{#gamma} - m_{e}) / (E_{#gamma} - m_{e})" );
	
	
	
	h_deltaE_vs_beamE_acc    = new TH2I( "deltaE_vs_beamE_acc", "#DeltaE vs. E_{#gamma} (accidentals)", 120, 0., 12., 1000, -1.0, 1.0 );
	h_deltaE_vs_beamE_acc->GetXaxis()->SetTitle( "E_{#gamma} [GeV]" );
	h_deltaE_vs_beamE_acc->GetYaxis()->SetTitle( "(E_{FCAL} + E_{CCAL} - E_{#gamma} - m_{e}) / (E_{#gamma} - m_{e})" );
	
	h_deltaE_vs_beamE_e_acc  = new TH2I( "deltaE_vs_beamE_e_acc", "#DeltaE vs. E_{#gamma} (accidentals) (TOF/FCAL Match)", 120, 0., 12., 1000, -1.0, 1.0 );
	h_deltaE_vs_beamE_e_acc->GetXaxis()->SetTitle( "E_{#gamma} [GeV]" );
	h_deltaE_vs_beamE_e_acc->GetYaxis()->SetTitle( "(E_{FCAL} + E_{CCAL} - E_{#gamma} - m_{e}) / (E_{#gamma} - m_{e})" );
	
	h_deltaE_vs_beamE_g_acc  = new TH2I( "deltaE_vs_beamE_g_acc", "#DeltaE vs. E_{#gamma} (accidentals) (No TOF/FCAL Match)", 120, 0., 12., 1000, -1.0, 1.0 );
	h_deltaE_vs_beamE_g_acc->GetXaxis()->SetTitle( "E_{#gamma} [GeV]" );
	h_deltaE_vs_beamE_g_acc->GetYaxis()->SetTitle( "(E_{FCAL} + E_{CCAL} - E_{#gamma} - m_{e}) / (E_{#gamma} - m_{e})" );
	
	
	
	
	
	
	
	h_deltaK_cut   = new TH1I( "deltaK_cut",   "#DeltaK (|#DeltaE| < 0.12)", 1000, -1.0, 1.0 );
	h_deltaK_e_cut = new TH1I( "deltaK_e_cut", "#DeltaK (|#DeltaE| < 0.12)", 1000, -1.0, 1.0 );
	h_deltaK_g_cut = new TH1I( "deltaK_g_cut", "#DeltaK (|#DeltaE| < 0.12)", 1000, -1.0, 1.0 );
	
	h_deltaK_cut_acc   = new TH1I( "deltaK_cut_acc",   "#DeltaK (|#DeltaE| < 0.12) (accidentals)", 1000, -1.0, 1.0 );
	h_deltaK_e_cut_acc = new TH1I( "deltaK_e_cut_acc", "#DeltaK (|#DeltaE| < 0.12) (accidentals) (TOF/FCAL match)", 1000, -1.0, 1.0 );
	h_deltaK_g_cut_acc = new TH1I( "deltaK_g_cut_acc", "#DeltaK (|#DeltaE| < 0.12) (accidentals) (No TOF/FCAL match)", 1000, -1.0, 1.0 );
	
	
	
	
	
	
	
	h_deltaK    = new TH1I( "deltaK",   "#DeltaK; (E_{Compton} - E_{#gamma} - m_{e}) / (E_{#gamma} - m_{e})", 1000, -1.0, 1.0 );
	h_deltaK_e  = new TH1I( "deltaK_e", "#DeltaK (TOF/FCAL Match); (E_{Compton} - E_{#gamma} - m_{e}) / (E_{#gamma} - m_{e})", 1000, -1.0, 1.0 );
	h_deltaK_g  = new TH1I( "deltaK_g", "#DeltaK (No TOF/FCAL Match); (E_{Compton} - E_{#gamma} - m_{e}) / (E_{#gamma} - m_{e})", 1000, -1.0, 1.0 );
	
	h_deltaK_acc    = new TH1I( "deltaK_acc",   "#DeltaK (accidentals); (E_{Compton} - E_{#gamma} - m_{e}) / (E_{#gamma} - m_{e})", 1000, -1.0, 1.0 );
	h_deltaK_e_acc  = new TH1I( "deltaK_e_acc", "#DeltaK (accidentals) (TOF/FCAL Match); (E_{Compton} - E_{#gamma} - m_{e}) / (E_{#gamma} - m_{e})", 1000, -1.0, 1.0 );
	h_deltaK_g_acc  = new TH1I( "deltaK_g_acc", "#DeltaK (accidentals) (No TOF/FCAL Match); (E_{Compton} - E_{#gamma} - m_{e}) / (E_{#gamma} - m_{e})", 1000, -1.0, 1.0 );
	
	
	
	h_deltaK_vs_beamE   = new TH2I( "deltaK_vs_beamE", "#DeltaK vs. E_{#gamma}", 120, 0., 12., 1000, -1.0, 1.0 );
	h_deltaK_vs_beamE->GetXaxis()->SetTitle( "E_{#gamma} [GeV]" );
	h_deltaK_vs_beamE->GetYaxis()->SetTitle( "(E_{Compton} - E_{#gamma} - m_{e}) / (E_{#gamma} - m_{e})" );
	
	h_deltaK_vs_beamE_e   = new TH2I( "deltaK_vs_beamE_e", "#DeltaK vs. E_{#gamma} (TOF/FCAL Match)", 120, 0., 12., 1000, -1.0, 1.0 );
	h_deltaK_vs_beamE_e->GetXaxis()->SetTitle( "E_{#gamma} [GeV]" );
	h_deltaK_vs_beamE_e->GetYaxis()->SetTitle( "(E_{Compton} - E_{#gamma} - m_{e}) / (E_{#gamma} - m_{e})" );
	
	h_deltaK_vs_beamE_g   = new TH2I( "deltaK_vs_beamE_g", "#DeltaK vs. E_{#gamma} (No TOF/FCAL Match)", 120, 0., 12., 1000, -1.0, 1.0 );
	h_deltaK_vs_beamE_g->GetXaxis()->SetTitle( "E_{#gamma} [GeV]" );
	h_deltaK_vs_beamE_g->GetYaxis()->SetTitle( "(E_{Compton} - E_{#gamma} - m_{e}) / (E_{#gamma} - m_{e})" );
	
	
	
	h_deltaK_vs_beamE_acc   = new TH2I( "deltaK_vs_beamE_acc", "#DeltaK vs. E_{#gamma} (accidentals)", 120, 0., 12., 1000, -1.0, 1.0 );
	h_deltaK_vs_beamE_acc->GetXaxis()->SetTitle( "E_{#gamma} [GeV]" );
	h_deltaK_vs_beamE_acc->GetYaxis()->SetTitle( "(E_{Compton} - E_{#gamma} - m_{e}) / (E_{#gamma} - m_{e})" );
	
	h_deltaK_vs_beamE_e_acc   = new TH2I( "deltaK_vs_beamE_e_acc", "#DeltaK vs. E_{#gamma} (accidentals) (TOF/FCAL Match)", 120, 0., 12., 1000, -1.0, 1.0 );
	h_deltaK_vs_beamE_e_acc->GetXaxis()->SetTitle( "E_{#gamma} [GeV]" );
	h_deltaK_vs_beamE_e_acc->GetYaxis()->SetTitle( "(E_{Compton} - E_{#gamma} - m_{e}) / (E_{#gamma} - m_{e})" );
	
	h_deltaK_vs_beamE_g_acc   = new TH2I( "deltaK_vs_beamE_g_acc", "#DeltaK vs. E_{#gamma} (accidentals) (No TOF/FCAL Match)", 120, 0., 12., 1000, -1.0, 1.0 );
	h_deltaK_vs_beamE_g_acc->GetXaxis()->SetTitle( "E_{#gamma} [GeV]" );
	h_deltaK_vs_beamE_g_acc->GetYaxis()->SetTitle( "(E_{Compton} - E_{#gamma} - m_{e}) / (E_{#gamma} - m_{e})" );
	
	
	
	
	
	
	h_xy_fcal     = new TH2I( "xy_fcal",     "Position of FCAL Shower", 1000, -60., 60., 1000, -60., 60. );
	h_xy_fcal_acc = new TH2I( "xy_fcal_acc", "Position of FCAL Shower (tagger accidentals)", 1000, -60., 60., 1000, -60., 60. );
	
	h_xy_ccal     = new TH2I( "xy_ccal",     "Position of CCAL Shower", 1000, -13., 13., 1000, -13., 13. );
	h_xy_ccal_acc = new TH2I( "xy_ccal_acc", "Position of CCAL Shower (tagger accidentals)", 1000, -13., 13., 1000, -13., 13. );
	
	
	
		
	
	
	
  	return NOERROR;
}




//------------------
// brun
//------------------
jerror_t JEventProcessor_compton_simple::brun(JEventLoop *eventLoop, int32_t runnumber)
{
	
	DGeometry*   dgeom = NULL;
  	DApplication* dapp = dynamic_cast< DApplication* >( eventLoop->GetJApplication() );
  	if( dapp )   dgeom = dapp->GetDGeometry( runnumber );
   
  	if( dgeom ){
    	  	dgeom->GetTargetZ( targetZ );
		dgeom->GetCCALZ( ccalZ );
		dgeom->GetFCALZ( fcalZ );
  	} else{
    	  	cerr << "No geometry accessbile to ccal_timing monitoring plugin." << endl;
    	  	return RESOURCE_UNAVAILABLE;
  	}	
	
	
	
  	return NOERROR;
}




//------------------
// evnt
//------------------
jerror_t JEventProcessor_compton_simple::evnt(JEventLoop *eventLoop, uint64_t eventnumber)
{
	
	
	//----------   Reject front panel triggers   ----------//
	
	const DL1Trigger *trig = NULL;
  	try {
      	  	eventLoop->GetSingle(trig);
  	} catch (...) {}
	if (trig == NULL) { return NOERROR; }
	
	uint32_t fp_trigmask = trig->fp_trig_mask; if( fp_trigmask ) return NOERROR;
	
	
	
	
	//----------   Get Event RF Bunch   ----------//
	/*
	const DEventRFBunch *locRFBunch = NULL;
	try { 
	  	eventLoop->GetSingle( locRFBunch, "CalorimeterOnly" );
	} catch (...) { return NOERROR; }
	double rfTime = locRFBunch->dTime;
	if( locRFBunch->dNumParticleVotes < 2 ) return NOERROR;
	*/
	
	
	
	//----------   Use center of target as vertex for now   ----------//
	
	DVector3 vertex;
	vertex.SetXYZ( 0.0, 0.0, targetZ );
	
	
	
	
	
	//-----   Get FCAL and CCAL Geometry objects   -----//
	
	vector< const DFCALGeometry* > fcalGeomVec;
  	eventLoop->Get( fcalGeomVec );
	
  	if( fcalGeomVec.size() != 1 ) {
    		cerr << "No FCAL geometry accessbile." << endl;
    		return RESOURCE_UNAVAILABLE;
  	}
	
	const DFCALGeometry *fcalGeom = fcalGeomVec[0];
	
	
	vector< const DCCALGeometry* > ccalGeomVec;
  	eventLoop->Get( ccalGeomVec );
	
  	if( ccalGeomVec.size() != 1 ) {
    		cerr << "No CCAL geometry accessbile." << endl;
    		return RESOURCE_UNAVAILABLE;
  	}
	
	const DCCALGeometry *ccalGeom = ccalGeomVec[0];
	
	
	
	
	
	//----------   Get data objects   ----------//
	
	vector< const DBeamPhoton* > beam_photons;
	vector< const DCCALShower* > ccal_showers;
	vector< const DFCALShower* > fcal_showers;
	vector< const DTOFPoint*   >  tof_points;
	
	eventLoop->Get( beam_photons );
	eventLoop->Get( ccal_showers );
	eventLoop->Get( fcal_showers );
	eventLoop->Get(  tof_points  );
	
	
	
	
	
	japp->RootFillLock(this);  // Acquire root lock
	
	
	for( vector< const DFCALShower* >::const_iterator show1 = fcal_showers.begin(); 
		show1 != fcal_showers.end(); show1++ ) {
		
		
		double e1    =  (*show1)->getEnergy();
		double x1    =  (*show1)->getPosition().X() + fcalX - vertex.X();
		double y1    =  (*show1)->getPosition().Y() + fcalY - vertex.Y();
		double z1    =  (*show1)->getPosition().Z() - vertex.Z();
		double r1    =  sqrt( x1*x1 + y1*y1 + z1*z1 );
		double t1    =  (*show1)->getTime() - (r1/c);
		double phi1  =  (atan2(y1,x1))*180./pi;
		
		
		
		//-----   Only select fcal showers in main RF bunch   -----//
		
		//if( fabs(t1-rfTime) > 2.5 ) continue;
		
		
		
		//-----   Reject events in the first inner ring of FCAL   -----//
		
		double x1face = (*show1)->getPosition().X();
		double y1face = (*show1)->getPosition().Y();		
		
		int row1 = fcalGeom->row( static_cast<float>(y1face) );
		int col1 = fcalGeom->column( static_cast<float>(x1face) );
		
		if( (row1 > 25 && row1 < 31) && (col1 > 25 && col1 < 31) ) continue;
		
		
		
		
		//-----   Check for match between TOF and FCAL   -----//
		
		int tof_match = 0;
		for( vector< const DTOFPoint* >::const_iterator tof_p = tof_points.begin(); 
			tof_p != tof_points.end(); tof_p++ ) {
			
			double xtof = (*tof_p)->pos.X() - vertex.X();
			double ytof = (*tof_p)->pos.Y() - vertex.Y();
			double ztof = (*tof_p)->pos.Z() - vertex.Z();
			
			double rtof = sqrt( xtof*xtof + ytof*ytof + ztof*ztof );
			double ttof = (*tof_p)->t - (rtof/c);
			
			xtof = xtof * (z1 / ztof);
			ytof = ytof * (z1 / ztof);
			
			int hbar  = (*tof_p)->dHorizontalBar;			
			int hstat = (*tof_p)->dHorizontalBarStatus;
			int vbar  = (*tof_p)->dVerticalBar;
			int vstat = (*tof_p)->dVerticalBarStatus;
			
			double dx, dy;
			//double dt = ttof - rfTime;
			
			if( hstat==3 && vstat==3 ) {
				dx = x1 - xtof;
				dy = y1 - ytof;
			} else if( vstat==3 ) {
				dx = x1 - bar2x(vbar)*(z1 / ztof);
				dy = y1 - ytof;
			} else if( hstat==3 ) {
				dx = x1 - xtof;
				dy = y1 - bar2x(hbar)*(z1 / ztof);
			} else {
				dx = x1 - bar2x(vbar)*(z1 / ztof);
				dy = y1 - bar2x(hbar)*(z1 / ztof);
			}
			
			if( fabs(dx) < 6. && fabs(dy) < 6. ) { 
				//if( dt > -1. && dt < 3. ) tof_match = 1;
				tof_match = 1;
			}
			
		} // end DTOFPoint loop
		
		
		
		
		
		
		//----------   Loop over CCAL Showers   ----------//
		
		for( vector< const DCCALShower* >::const_iterator show2 = ccal_showers.begin(); 
			show2 != ccal_showers.end(); show2++ ) {
			
			
			double e2    =  (*show2)->E;
			double x2    =  (*show2)->x1 + ccalX - vertex.X();
			double y2    =  (*show2)->y1 + ccalY - vertex.Y();
			double z2    =  (*show2)->z  - vertex.Z();
			double r2    =  sqrt( x2*x2 + y2*y2 + z2*z2 );
			double t2    =  (*show2)->time - (r2/c);
			double phi2  =  (atan2(y2,x2))*180./pi;
			
			
			
			//-----   Only select ccal showers in main RF bunch   -----//
			
			//if( fabs(t2-rfTime) > 2.5 ) continue;
			
			
			
			//-----   Reject events in the most outer ring of CCAL   -----//
			
			double x2face = (*show2)->x1;
			double y2face = (*show2)->y1;		
			
			int row2 = ccalGeom->row( static_cast<float>(y2face) );
			int col2 = ccalGeom->column( static_cast<float>(x2face) );
			
			if( row2 == 0 || row2 == 11 || col2 == 0 || col2 == 11 ) continue;
			
			
			
			
			double deltaPhi  =  phi2 - phi1;
			double deltaT    =  t2 - t1;
			
			h_deltaPhi->Fill( deltaPhi );
			h_deltaT->Fill( deltaT );
			
			
			if( fabs( fabs(deltaPhi) - 180. ) > 30. ) continue;
			
			
			
			
			//----------   Loop over Beam Photons   ----------//
			
			for( vector< const DBeamPhoton* >::const_iterator gam = beam_photons.begin();
				gam != beam_photons.end(); gam++ ) {
				
				double eb = (*gam)->lorentzMomentum().E();
				double tb = (*gam)->time();
				
				
					
				//double brfdt = tb - rfTime;
				double  bfdt = tb - t1;
				double  bcdt = tb - t2;
				
				h_beam_fcal_dt->Fill( bfdt );
				h_beam_ccal_dt->Fill( bcdt );
					
				
				
				
				
				double ecomp1   =   1. / ( 1./eb + (1.-(z1/r1))/me );
				double ecomp2   =   1. / ( 1./eb + (1.-(z2/r2))/me );
				
				double deltaE   =  (e1 + e2)/(eb + me) - 1.0;
				double deltaK   =  (ecomp1 + ecomp2)/(eb + me) - 1.0;
					
				
				
				
				
				if( fabs(bcdt) < 2. ) {
					
					h_deltaE->Fill( deltaE );
					h_deltaE_vs_beamE->Fill( eb, deltaE );
					
					h_deltaK->Fill( deltaK );
					h_deltaK_vs_beamE->Fill( eb, deltaK );
					
					if( fabs(deltaE) < 1.2 ) {
					  	h_deltaK_cut->Fill( deltaK );
						if( tof_match ) h_deltaK_e_cut->Fill( deltaK );
						else h_deltaK_g_cut->Fill( deltaK );
					}
					
					
					// TOF Match (electron in FCAL, photon in CCAL):
					
					if( tof_match ) {
						
						h_deltaE_e->Fill( deltaE );
						h_deltaE_vs_beamE_e->Fill( eb, deltaE );
						
						h_deltaK_e->Fill( deltaK );
						h_deltaK_vs_beamE_e->Fill( eb, deltaK );
						
					} else {
						
						h_deltaE_g->Fill( deltaE );
						h_deltaE_vs_beamE_g->Fill( eb, deltaE );
						
						h_deltaK_g->Fill( deltaK );
						h_deltaK_vs_beamE_g->Fill( eb, deltaK );
						
					}
					
					
					if( fabs(deltaE) < 1.2 && fabs(deltaK) < 1.2 ) {
						
						h_xy_fcal->Fill( x1, y1 );
						h_xy_ccal->Fill( x2, y2 );
						
					}
					
				} else if( (bcdt > -22. && bcdt < -18.) || (bcdt > 18. && bcdt < 22.) ) {
					
					h_deltaE_acc->Fill( deltaE );
					h_deltaE_vs_beamE_acc->Fill( eb, deltaE );
					
					h_deltaK_acc->Fill( deltaK );
					h_deltaK_vs_beamE_acc->Fill( eb, deltaK );
					
					if( fabs(deltaE) < 1.2 ) {
					  	h_deltaK_cut_acc->Fill( deltaK );
						if( tof_match ) h_deltaK_e_cut_acc->Fill( deltaK );
						else h_deltaK_g_cut_acc->Fill( deltaK );
					}
					
					// TOF Match (electron in FCAL, photon in CCAL):
					
					if( tof_match ) {
						
						h_deltaE_e_acc->Fill( deltaE );
						h_deltaE_vs_beamE_e_acc->Fill( eb, deltaE );
						
						h_deltaK_e_acc->Fill( deltaK );
						h_deltaK_vs_beamE_e_acc->Fill( eb, deltaK );
						
					} else {
						
						h_deltaE_g_acc->Fill( deltaE );
						h_deltaE_vs_beamE_g_acc->Fill( eb, deltaE );
						
						h_deltaK_g_acc->Fill( deltaK );
						h_deltaK_vs_beamE_g_acc->Fill( eb, deltaK );
						
					}
					
					
					if( fabs(deltaE) < 1.2 && fabs(deltaK) < 1.2 && fabs(fabs(deltaPhi)-180.) < 30. ) {
						
						h_xy_fcal_acc->Fill( x1, y1 );
						h_xy_ccal_acc->Fill( x2, y2 );
						
					}					
					
				}
				
			} // end DBeamPhoton loop			
			
		} // end DCCALShower loop
		
	} // end DFCALShower loop
	
	
	
	japp->RootFillUnLock(this);  // Release root lock
	
	
	
	
	
	
  	return NOERROR;
}




//------------------
// erun
//------------------
jerror_t JEventProcessor_compton_simple::erun(void)
{
  	
  	return NOERROR;
}




//------------------
// fini
//------------------
jerror_t JEventProcessor_compton_simple::fini(void)
{
	
  	return NOERROR;
}




//--------------------------------------------
// Get TOF position from bar
//--------------------------------------------
double JEventProcessor_compton_simple::bar2x(int bar) {

	int ic = 2*bar - 45; 
	
	double pos;
	if( ic ==  1  || ic == -1 ) pos = 3.0*(double)ic;
        else if( ic ==  3  || ic ==  5 ) pos = 1.5*(double)(ic+2);
        else if( ic == -3  || ic == -5 ) pos = 1.5*(double)(ic-2);
	else if( ic >  5 ) pos = 3.*(ic-2);
        else pos = 3.*(ic+2);
	
        double x = 1.1*pos;
	
	return x;
}














