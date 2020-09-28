/**************************************************************************                                                                                                                           
* HallD software                                                          * 
* Copyright(C) 2019  GlueX and PrimEX-D Collaborations                    * 
*                                                                         *                                                                                                                               
* Author: The GlueX and PrimEX-D Collaborations                           *                                                                                                                                
* Contributors: Andrew Smith                                              *                                                                                                                               
*                                                                         *                                                                                                                               
* This software is provided "as is" without any warranty.                 *
**************************************************************************/

#include "JEventProcessor_compton.h"

extern "C"{
void InitPlugin(JApplication *app){
	InitJANAPlugin(app);
    	app->AddProcessor(new JEventProcessor_compton());
}
} // "C"




//------------------
// init
//------------------
jerror_t JEventProcessor_compton::init(void)
{
  	
	
	
	hTrig     =  new TH1F( "hTrig",   "GTP Trigger Bits", 33, -0.5, 32.5 );
	hfpTrig   =  new TH1F( "hfpTrig",  "FP Trigger Bits", 33, -0.5, 32.5 );
	
	
	
	//----------   Timing Plots   ----------//
	
	
	h_fcal_rf_dt    =  new TH1F( "fcal_rf_dt",   "FCAL - RF Time; [ns]",              1000, -20., 20. );
	h_ccal_rf_dt    =  new TH1F( "ccal_rf_dt",   "CCAL - RF Time; [ns]",              1000, -20., 20. );
	h_beam_rf_dt    =  new TH1F( "beam_rf_dt",   "Beam Photon Time - RF Time; [ns]",  1000, -20., 20. );
	
	h_beam_ccal_dt  =  new TH1F( "beam_ccal_dt", "Beam Photon Time - CCAL Shower Time; [ns]", 1000, -20., 20. );
	h_beam_fcal_dt  =  new TH1F( "beam_fcal_dt", "Beam Photon Time - FCAL Shower Time; [ns]", 1000, -20., 20. );
	
	
	//----------   DeltaPhi Plots   ----------//
	
	
	TDirectory *dir_deltaPhi = new TDirectoryFile( "DeltaPhi", "DeltaPhi" );
  	dir_deltaPhi->cd();
	
	h1_deltaPhi         =  new TH1F( "deltaPhi",     "#Delta#phi; | #phi_{1} - #phi_{2} | [deg.]",                                   1000, 100.0, 260.0 );
	h1_deltaPhi_e       =  new TH1F( "deltaPhi_e",   "#Delta#phi (5#sigma #DeltaE cut); | #phi_{1} - #phi_{2} | [deg.]",             1000, 100.0, 260.0 );
	h1_deltaPhi_ek      =  new TH1F( "deltaPhi_ek",  "#Delta#phi (5#sigma #DeltaE + #DeltaK cuts ); | #phi_{1} - #phi_{2} | [deg.]", 1000, 100.0, 260.0 );
	
	h2_deltaPhi         =  new TH2F( "deltaPhi_v_egam",     "#Delta#phi; E_{#gamma} [GeV]; | #phi_{1} - #phi_{2} | [deg.]",                                   120, 0., 12.,  1000, 100.0, 260.0 );
	h2_deltaPhi_e       =  new TH2F( "deltaPhi_v_egam_e",   "#Delta#phi (5#sigma #DeltaE cut); E_{#gamma} [GeV]; | #phi_{1} - #phi_{2} | [deg.]",             120, 0., 12.,  1000, 100.0, 260.0 );
	h2_deltaPhi_ek      =  new TH2F( "deltaPhi_v_egam_ek",  "#Delta#phi (5#sigma #DeltaE + #DeltaK cuts ); E_{#gamma} [GeV]; | #phi_{1} - #phi_{2} | [deg.]", 120, 0., 12.,  1000, 100.0, 260.0 );
	
	dir_deltaPhi->cd( "../" );
	
	
	//----------   dTheta Plots   ----------//
	
	
	TDirectory *dir_dTheta = new TDirectoryFile( "dTheta", "dTheta" );
  	dir_dTheta->cd();
	
	h1_dTheta         =  new TH1F( "dTheta",     "Opening Angle; #theta_{op} [deg.]",                                               2000, 0.0, 10.0 );
	h1_dTheta_p       =  new TH1F( "dTheta_p",   "Opening Angle (5#sigma #Delta#phi cut); #theta_{op} [deg.]",                      2000, 0.0, 10.0 );
	h1_dTheta_pe      =  new TH1F( "dTheta_pe",  "Opening Angle (5#sigma #Delta#phi + #DeltaE cuts ); #theta_{op} [deg.]", 	        2000, 0.0, 10.0 );
	h1_dTheta_pek     =  new TH1F( "dTheta_pek", "Opening Angle (5#sigma #Delta#phi + #DeltaE + #DeltaK cuts); #theta_{op} [deg.]", 2000, 0.0, 10.0 );
	
	h2_dTheta         =  new TH2F( "dTheta_v_egam",     "Opening Angle; E_{#gamma} [GeV]; #theta_{op} [deg.]",                                               120, 0., 12., 2000, 0.0, 10.0 );
	h2_dTheta_p       =  new TH2F( "dTheta_v_egam_p",   "Opening Angle (5#sigma #Delta#phi cut); E_{#gamma} [GeV]; #theta_{op} [deg.]",                      120, 0., 12., 2000, 0.0, 10.0 );
	h2_dTheta_pe      =  new TH2F( "dTheta_v_egam_pe",  "Opening Angle (5#sigma #Delta#phi + #DeltaE cuts ); E_{#gamma} [GeV]; #theta_{op} [deg.]",          120, 0., 12., 2000, 0.0, 10.0 );
	h2_dTheta_pek     =  new TH2F( "dTheta_v_egam_pek", "Opening Angle (5#sigma #Delta#phi + #DeltaE + #DeltaK cuts); E_{#gamma} [GeV]; #theta_{op} [deg.]", 120, 0., 12., 2000, 0.0, 10.0 );
	
	dir_dTheta->cd( "../" );
	
	
	//----------   DeltaR Plots   ----------//
	
	
	TDirectory *dir_deltaR = new TDirectoryFile( "DeltaR", "DeltaR" );
  	dir_deltaR->cd();
	
	h1_deltaR         =  new TH1F( "deltaR",     "#DeltaR; #DeltaR [cm]",                                               1000, 0.0, 100.0 );
	h1_deltaR_p       =  new TH1F( "deltaR_p",   "#DeltaR (5#sigma #Delta#phi cut); #DeltaR [cm]",                      1000, 0.0, 100.0 );
	h1_deltaR_pe      =  new TH1F( "deltaR_pe",  "#DeltaR (5#sigma #Delta#phi + #DeltaE cuts ); #DeltaR [cm]",          1000, 0.0, 100.0 );
	h1_deltaR_pek     =  new TH1F( "deltaR_pek", "#DeltaR (5#sigma #Delta#phi + #DeltaE + #DeltaK cuts); #DeltaR [cm]", 1000, 0.0, 100.0 );
	
	h2_deltaR         =  new TH2F( "deltaR_v_egam",     "#DeltaR; E_{#gamma} [GeV]; #DeltaR [cm]",                                               120, 0., 12., 1000, 0.0, 100.0 );
	h2_deltaR_p       =  new TH2F( "deltaR_v_egam_p",   "#DeltaR (5#sigma #Delta#phi cut); E_{#gamma} [GeV]; #DeltaR [cm]",                      120, 0., 12., 1000, 0.0, 100.0 );
	h2_deltaR_pe      =  new TH2F( "deltaR_v_egam_pe",  "#DeltaR (5#sigma #Delta#phi + #DeltaE cuts ); E_{#gamma} [GeV]; #DeltaR [cm]",          120, 0., 12., 1000, 0.0, 100.0 );
	h2_deltaR_pek     =  new TH2F( "deltaR_v_egam_pek", "#DeltaR (5#sigma #Delta#phi + #DeltaE + #DeltaK cuts); E_{#gamma} [GeV]; #DeltaR [cm]", 120, 0., 12., 1000, 0.0, 100.0 );
	
	dir_deltaPhi->cd( "../" );
	
	
	//----------   DeltaT Plots   ----------//
	
	
	TDirectory *dir_deltaT = new TDirectoryFile( "DeltaT", "DeltaT" );
  	dir_deltaT->cd();
	
	h1_deltaT      =  new TH1F( "deltaT",     "#DeltaT; t_{CCAL} - t_{FCAL} [ns]",                                               1000, -10.0, 10.0 );
	h1_deltaT_p    =  new TH1F( "deltaT_p",   "#DeltaT (5#sigma #Delta#phi cut); t_{CCAL} - t_{FCAL} [ns]",                      1000, -10.0, 10.0 );
	h1_deltaT_pe   =  new TH1F( "deltaT_pe",  "#DeltaT (5#sigma #Delta#phi + #DeltaE cuts ); t_{CCAL} - t_{FCAL} [ns]",          1000, -10.0, 10.0 );
	h1_deltaT_pek  =  new TH1F( "deltaT_pek", "#DeltaT (5#sigma #Delta#phi + #DeltaE + #DeltaK cuts); t_{CCAL} - t_{FCAL} [ns]", 1000, -10.0, 10.0 );
	
	h2_deltaT      =  new TH2F( "deltaT_v_egam",     "#DeltaT; E_{#gamma} [GeV]; t_{CCAL} - t_{FCAL} [ns]",                                               120, 0., 12., 1000, -10.0, 10.0 );
	h2_deltaT_p    =  new TH2F( "deltaT_v_egam_p",   "#DeltaT (5#sigma #Delta#phi cut); E_{#gamma} [GeV]; t_{CCAL} - t_{FCAL} [ns]",                      120, 0., 12., 1000, -10.0, 10.0 );
	h2_deltaT_pe   =  new TH2F( "deltaT_v_egam_pe",  "#DeltaT (5#sigma #Delta#phi + #DeltaE cuts ); E_{#gamma} [GeV]; t_{CCAL} - t_{FCAL} [ns]",          120, 0., 12., 1000, -10.0, 10.0 );
	h2_deltaT_pek  =  new TH2F( "deltaT_v_egam_pek", "#DeltaT (5#sigma #Delta#phi + #DeltaE + #DeltaK cuts); E_{#gamma} [GeV]; t_{CCAL} - t_{FCAL} [ns]", 120, 0., 12., 1000, -10.0, 10.0 );
	
	dir_deltaT->cd( "../" );
	
	
	//--------------------------------------//
	
	
	
	
	TDirectory *dir_deltaE = new TDirectoryFile( "DeltaE", "DeltaE" );
	dir_deltaE->cd();
	
	h1_deltaE     = new TH1F( "deltaE",    Form("#DeltaE (%.1f GeV < E_{#gamma} < 12.0 GeV); E_{1} + E_{2} - E_{#gamma} [GeV]", MIN_BEAM_ENERGY_CUT), 
		2000, -4.0, 4.0 );
	h1_deltaE_p   = new TH1F( "deltaE_p",  Form("#DeltaE (%.1f GeV < E_{#gamma} < 12.0 GeV) (5#sigma #Delta#phi Cut); E_{1} + E_{2} - E_{#gamma} [GeV]", MIN_BEAM_ENERGY_CUT), 
		2000, -4.0, 4.0 );
	h1_deltaE_pk  = new TH1F( "deltaE_pe", Form("#DeltaE (%.1f GeV < E_{#gamma} < 12.0 GeV) (5#sigma #Delta#phi + #DeltaK Cuts); E_{1} + E_{2} - E_{#gamma} [GeV]", MIN_BEAM_ENERGY_CUT), 
		2000, -4.0, 4.0 );
	
	h2_deltaE     = new TH2F( "deltaE_v_egam",    "#DeltaE vs. E_{#gamma}; E_{#gamma} [GeV]; E_{1} + E_{2} - E_{#gamma} [GeV]",
		120, 0., 12., 2000, -4.0, 4.0 );
	h2_deltaE_p   = new TH2F( "deltaE_v_egam_p",  "#DeltaE vs. E_{#gamma} (5#sigma #Delta#phi Cut); E_{#gamma} [GeV]; E_{1} + E_{2} - E_{#gamma} [GeV]",
		120, 0., 12., 2000, -4.0, 4.0 );
	h2_deltaE_pk  = new TH2F( "deltaE_v_egam_pk", "#DeltaE vs. E_{#gamma} (5#sigma #Delta#phi + #DeltaK Cuts); E_{#gamma} [GeV]; E_{1} + E_{2} - E_{#gamma} [GeV]",
		120, 0., 12., 2000, -4.0, 4.0 );
	
	
	h2_deltaE_v_fcalE = new TH2F( "deltaE_v_fcalE", "#DeltaE vs. E_{#FCAL}; E_{FCAL} [GeV]; E_{CCAL} + E_{FCAL} - E_{#gamma} [GeV]",  600, 0.,  6., 2000, -4., 4. );
	h2_deltaE_v_ccalE = new TH2F( "deltaE_v_ccalE", "#DeltaE vs. E_{#CCAL}; E_{CCAL} [GeV]; E_{CCAL} + E_{FCAL} - E_{#gamma} [GeV]", 1200, 0., 12., 2000, -4., 4. );
	
	
	for( int ir = 0; ir < 10; ir++ ) 
	{
		
		h1_deltaE_ring[ir]    = new TH1F( Form("deltaE_ring%d",    ir+1), Form("#DeltaE (ring %d); E_{#gamma} [GeV]; E_{1} + E_{2} - E_{#gamma} [GeV]",ir+1), 
			2000, -4.0, 4.0 );
		h1_deltaE_p_ring[ir]  = new TH1F( Form("deltaE_p_ring%d",  ir+1), Form("#DeltaE (ring %d) (5#sigma #Delta#phi Cut); E_{#gamma} [GeV]; E_{1} + E_{2} - E_{#gamma} [GeV]",ir+1), 
			2000, -4.0, 4.0 );
		h1_deltaE_pk_ring[ir] = new TH1F( Form("deltaE_pk_ring%d", ir+1), Form("#DeltaE (ring %d) (5#sigma #Delta#phi + #DeltaK Cuts); E_{#gamma} [GeV]; E_{1} + E_{2} - E_{#gamma} [GeV]",ir+1), 
			2000, -4.0, 4.0 );
		
		h2_deltaE_ring[ir]    = new TH2F( Form("deltaE_v_egam_ring%d",    ir+1), Form("#DeltaE (ring %d); E_{#gamma} [GeV]; E_{1} + E_{2} - E_{#gamma} [GeV]",ir+1), 
			120, 0., 12., 2000, -4.0, 4.0 );
		h2_deltaE_p_ring[ir]  = new TH2F( Form("deltaE_v_egam_p_ring%d",  ir+1), Form("#DeltaE (ring %d) (5#sigma #Delta#phi Cut); E_{#gamma} [GeV]; E_{1} + E_{2} - E_{#gamma} [GeV]",ir+1), 
			120, 0., 12., 2000, -4.0, 4.0 );
		h2_deltaE_pk_ring[ir] = new TH2F( Form("deltaE_v_egam_pk_ring%d", ir+1), Form("#DeltaE (ring %d) (5#sigma #Delta#phi + #DeltaK Cuts); E_{#gamma} E_{1} + E_{2} - E_{#gamma} [GeV]; [GeV]",ir+1), 
			120, 0., 12., 2000, -4.0, 4.0 );
		
		h2_deltaE_v_fcalE_ring[ir] = new TH2F( Form("deltaE_v_fcalE_ring%d",ir+1), Form("#DeltaE vs. E_{#FCAL} (ring %d); E_{FCAL} [GeV]; E_{CCAL} + E_{FCAL} - E_{#gamma} [GeV]",ir+1), 
			 600, 0.,  6., 2000, -4., 4. );
		h2_deltaE_v_ccalE_ring[ir] = new TH2F( Form("deltaE_v_ccalE_ring%d",ir+1), Form("#DeltaE vs. E_{#CCAL} (ring %d); E_{FCAL} [GeV]; E_{CCAL} + E_{FCAL} - E_{#gamma} [GeV]",ir+1), 
			1200, 0., 12., 2000, -4., 4. );
		
	}
	
	dir_deltaE->cd( "../" );
	
	
	
	
	TDirectory *dir_deltaK = new TDirectoryFile( "DeltaK", "DeltaK" );
	dir_deltaK->cd();
	
	h1_deltaK     = new TH1F( "deltaK",    Form("#DeltaK (%.1f GeV < E_{#gamma} < 12.0 GeV); E_{Comp} - E_{#gamma} [GeV]", MIN_BEAM_ENERGY_CUT), 
		2000, -4.0, 4.0 );
	h1_deltaK_p   = new TH1F( "deltaK_p",  Form("#DeltaK (%.1f GeV < E_{#gamma} < 12.0 GeV) (5#sigma #Delta#phi Cut); E_{Comp} - E_{#gamma} [GeV]", MIN_BEAM_ENERGY_CUT), 
		2000, -4.0, 4.0 );
	h1_deltaK_pe  = new TH1F( "deltaK_pe", Form("#DeltaK (%.1f GeV < E_{#gamma} < 12.0 GeV) (5#sigma #Delta#phi + #DeltaE Cuts); E_{Comp} - E_{#gamma} [GeV]", MIN_BEAM_ENERGY_CUT), 
		2000, -4.0, 4.0 );
	
	h2_deltaK     = new TH2F( "deltaK_v_egam",    "#DeltaK vs. E_{#gamma}; E_{#gamma} [GeV]; E_{Comp} - E_{#gamma} [GeV]", 
		120, 0., 12., 2000, -4.0, 4.0 );
	h2_deltaK_p   = new TH2F( "deltaK_v_egam_p",  "#DeltaK vs. E_{#gamma} (5#sigma #Delta#phi Cut); E_{#gamma} [GeV]; E_{Comp} - E_{#gamma} [GeV]", 
		120, 0., 12., 2000, -4.0, 4.0 );
	h2_deltaK_pe  = new TH2F( "deltaK_v_egam_pe", "#DeltaK vs. E_{#gamma} (5#sigma #Delta#phi + #DeltaE Cuts); E_{#gamma} [GeV]; E_{Comp} - E_{#gamma} [GeV]", 
		120, 0., 12., 2000, -4.0, 4.0 );
	
	for( int ir = 0; ir < 10; ir++ ) 
	{
		
		h1_deltaK_ring[ir]    = new TH1F( Form("deltaK_ring%d",    ir+1), Form("#DeltaK (ring %d); E_{#gamma} [GeV]; [GeV]",ir+1), 
			2000, -4.0, 4.0 );
		h1_deltaK_p_ring[ir]  = new TH1F( Form("deltaK_p_ring%d",  ir+1), Form("#DeltaK (ring %d) (5#sigma #Delta#phi Cut); E_{#gamma} [GeV]; [GeV]",ir+1), 
			2000, -4.0, 4.0 );
		h1_deltaK_pe_ring[ir] = new TH1F( Form("deltaK_pe_ring%d", ir+1), Form("#DeltaK (ring %d) (5#sigma #Delta#phi + #DeltaE Cuts); E_{#gamma} [GeV]; [GeV]",ir+1), 
			2000, -4.0, 4.0 );
		
		h2_deltaK_ring[ir]    = new TH2F( Form("deltaK_v_egam_ring%d",    ir+1), Form("#DeltaK (ring %d); E_{#gamma} [GeV]; [GeV]",ir+1), 
			120, 0., 12., 2000, -4.0, 4.0 );
		h2_deltaK_p_ring[ir]  = new TH2F( Form("deltaK_v_egam_p_ring%d",  ir+1), Form("#DeltaK (ring %d) (5#sigma #Delta#phi Cut); E_{#gamma} [GeV]; [GeV]",ir+1), 
			120, 0., 12., 2000, -4.0, 4.0 );
		h2_deltaK_pe_ring[ir] = new TH2F( Form("deltaK_v_egam_pe_ring%d", ir+1), Form("#DeltaK (ring %d) (5#sigma #Delta#phi + #DeltaE Cuts); E_{#gamma} [GeV]; [GeV]",ir+1), 
			120, 0., 12., 2000, -4.0, 4.0 );
		
	}
	
	dir_deltaK->cd( "../" );
	
	
	
	
	
	TDirectory *dir_xy = new TDirectoryFile( "XY", "XY" );
	dir_xy->cd();
	
	h_fcal_xy  =  new TH2F( "fcal_xy", "FCAL Shower Position; x_{FCAL} [cm]; y_{FCAL} [cm]", 1000, -60., 60., 1000, -60., 60. );
	h_ccal_xy  =  new TH2F( "ccal_xy", "CCAL Shower Position; x_{CCAL} [cm]; y_{CCAL} [cm]", 1000, -13., 13., 1000, -13., 13. );
	
	for( int ir = 0; ir < 10; ir++ ) 
	{
		
		h_fcal_xy_ring[ir]  =  new TH2F( Form("fcal_xy_ring%d",ir+1), Form("FCAL Shower Position (ring %d)",ir+1), 1000, -60., 60., 1000, -60., 60. );
		h_ccal_xy_ring[ir]  =  new TH2F( Form("ccal_xy_ring%d",ir+1), Form("CCAL Shower Position (ring %d)",ir+1), 1000, -13., 13., 1000, -13., 13. );
	
	}
	
	dir_xy->cd( "../" );
	
	
	
	
	
	
  	return NOERROR;
}




//------------------
// brun
//------------------
jerror_t JEventProcessor_compton::brun(JEventLoop *eventLoop, int32_t runnumber)
{
	
	DGeometry*   dgeom = NULL;
  	DApplication* dapp = dynamic_cast< DApplication* >( eventLoop->GetJApplication() );
  	if( dapp )   dgeom = dapp->GetDGeometry( runnumber );
   	
	if( dgeom ){
    	  	dgeom->GetTargetZ( m_beamZ );
		dgeom->GetFCALPosition( m_fcalX, m_fcalY, m_fcalZ );
		dgeom->GetCCALPosition( m_ccalX, m_ccalY, m_ccalZ );
  	} else{
    	  	cerr << "No geometry accessbile to compton_analysis plugin." << endl;
    	  	return RESOURCE_UNAVAILABLE;
  	}
	
	
	jana::JCalibration *jcalib = japp->GetJCalibration(runnumber);
  	std::map<string, float> beam_spot;
  	jcalib->Get("PHOTON_BEAM/beam_spot", beam_spot);
  	m_beamX  =  beam_spot.at("x");
  	m_beamY  =  beam_spot.at("y");
	
	
	
  	return NOERROR;
}




//------------------
// evnt
//------------------
jerror_t JEventProcessor_compton::evnt(JEventLoop *eventLoop, uint64_t eventnumber)
{
	
	
	//----------   Reject front panel triggers   ----------//
	
	const DL1Trigger *trig = NULL;
  	try {
      	  	eventLoop->GetSingle(trig);
  	} catch (...) {}
	if (trig == NULL) { return NOERROR; }
	
	
	uint32_t trigmask = trig->trig_mask;	
	uint32_t fp_trigmask = trig->fp_trig_mask;
	
	for( int ibit = 0; ibit < 33; ibit++ ) {
	  	if(trigmask & (1 << ibit)) hTrig->Fill(ibit);
	  	if(fp_trigmask & (1 << ibit)) hfpTrig->Fill(ibit);
	}
	
	if( fp_trigmask ) return NOERROR;
	
	
	
	
	
	const DEventRFBunch *locRFBunch = NULL;
	try { 
	  	eventLoop->GetSingle( locRFBunch, "CalorimeterOnly" );
	} catch (...) { return NOERROR; }
	double rfTime = locRFBunch->dTime;
	if( locRFBunch->dNumParticleVotes < 2 ) return NOERROR;
	
	
	
	
	
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
	
	
	
	
	
	DVector3 vertex;
	vertex.SetXYZ( m_beamX, m_beamY, m_beamZ );
	
	
	
	
	
	vector< const DBeamPhoton* > beam_photons;
	vector< const DCCALShower* > ccal_showers;
	vector< const DFCALShower* > fcal_showers;
	
	eventLoop->Get( beam_photons );
	eventLoop->Get( ccal_showers );
	eventLoop->Get( fcal_showers );
	
	vector< const DFCALShower* > CompShowers_fcal;
	vector< const DCCALShower* > CompShowers_ccal;
	
	
	
	
	japp->RootFillLock(this);  // Acquire root lock
	
	
	
	int n_fcal_showers = 0;	
	for( vector< const DFCALShower* >::const_iterator show = fcal_showers.begin(); 
		show != fcal_showers.end(); show++ ) {
		
		double loc_E      =  (*show)->getEnergy();
		
		DVector3 loc_pos  =  (*show)->getPosition_log()  -  vertex;
		double loc_theta  =  loc_pos.Theta() * (180. / TMath::Pi());
		double loc_r      =  loc_pos.Mag();
		
		double loc_t      =  (*show)->getTime() - (loc_r/c) - rfTime;
		
		DVector3 face_pos = (*show)->getPosition_log();
		
		int row   = fcalGeom->row(    static_cast<float>(face_pos.Y()) );
		int col   = fcalGeom->column( static_cast<float>(face_pos.X()) );
		int layer = fcalLayer( row, col );
		
		
		if( (loc_E > MIN_FCAL_ENERGY_CUT) && (fabs(loc_t) < FCAL_RF_CUT) && (layer > 1) && (loc_theta < 4.0) ) {
		  	n_fcal_showers++;
		  	CompShowers_fcal.push_back( (*show) );
		}
		
		h_fcal_rf_dt->Fill( loc_t );
		
	}
	
	
	int n_ccal_showers = 0;	
	for( vector< const DCCALShower* >::const_iterator show = ccal_showers.begin();
		show != ccal_showers.end(); show++ ) {
		
		double loc_E = (*show)->E;
		
		double loc_x = (*show)->x1  -  vertex.X();
		double loc_y = (*show)->y1  -  vertex.Y();
		double loc_z = (*show)->z   -  vertex.Z();
		
		double loc_r = sqrt( loc_x*loc_x  +  loc_y*loc_y  +  loc_z*loc_z );
		double loc_t = (*show)->time - (loc_r/c) - rfTime;
		
		
		double xface = (*show)->x1  -  m_ccalX;
		double yface = (*show)->y1  -  m_ccalY;
		
		int row   = ccalGeom->row(    static_cast<float>(yface) );
		int col   = ccalGeom->column( static_cast<float>(xface) );
		int layer = ccalLayer( row, col );
		
		
		if( (loc_E > MIN_CCAL_ENERGY_CUT) && (fabs(loc_t) < FCAL_RF_CUT) && (layer > 1) && (layer < 5) && (col != 1) ) {
			n_ccal_showers++;
			CompShowers_ccal.push_back( (*show) );
		}
		
		h_ccal_rf_dt->Fill( loc_t );
		
	}
	
	
	
	
	if( !CompShowers_fcal.size() || !CompShowers_ccal.size() ) {
		japp->RootFillUnLock(this);
		return NOERROR;
	}
	
	
	
	
	
	
	
	
	//----------     Check FCAL-CCAL Pairs     ----------//
	
	
	vector< ComptonCandidate_t > candidates;
	
	
	for( vector< const DFCALShower* >::const_iterator show1 = CompShowers_fcal.begin(); 
		show1 != CompShowers_fcal.end(); show1++ ) {
		
		
		double e1         =  (*show1)->getEnergy();
		DVector3 pos1     =  (*show1)->getPosition_log()  -  vertex;
		
		double r1         =  pos1.Mag();
		double t1         =  (*show1)->getTime()  -  (r1/c);
		double phi1       =  pos1.Phi() * (180. / TMath::Pi());
		double theta1     =  pos1.Theta();
		
		
		const DFCALCluster *locCluster;
		(*show1)->GetSingle(locCluster);
		int chan1      = locCluster->getChannelEmax();
		double radius1 = fcalGeom->positionOnFace(chan1).Mod();
		int ring1      = (int)(radius1 / 5.);
		
		
		
		for( vector< const DCCALShower* >::const_iterator show2 = CompShowers_ccal.begin(); 
			show2 != CompShowers_ccal.end(); show2++ ) {
			
			
			double e2     =  (*show2)->E;
			DVector3 pos2( (*show2)->x1, (*show2)->y1, (*show2)->z );
			pos2          =  pos2  -  vertex;
			
			double r2     =  pos2.Mag();
			double t2     =  (*show2)->time  -  (r2 / c);
			double phi2   =  pos2.Phi() * (180. / TMath::Pi());
			double theta2 =  pos2.Theta();
			
			
			// calculate cos(theta) (opening angle):
			
			double costheta  = ( pos1.X()*pos2.X()  +  pos1.Y()*pos2.Y()  +  pos1.Z()*pos2.Z() ) / ( r1*r2 );
			double dTheta    = acos( costheta ) * (180. / TMath::Pi());
			
			
			// calculate deltaPhi and deltaT:
			
			double deltaPhi  =  fabs(phi2 - phi1);
			double deltaT    =  t2 - t1;
			
			
			// calculate separation distance of particles:
			
			double x1_face   =  ((m_fcalZ - vertex.Z()) / pos1.Z()) * pos1.X();
			double y1_face   =  ((m_fcalZ - vertex.Z()) / pos1.Z()) * pos1.Y();
			double x2_face   =  ((m_fcalZ - vertex.Z()) / pos2.Z()) * pos2.X();
			double y2_face   =  ((m_fcalZ - vertex.Z()) / pos2.Z()) * pos2.Y();
			double deltaR    =  sqrt( pow(x1_face-x2_face,2.0)  +  pow(y1_face-y2_face,2.0) );
			
			
			for( vector< const DBeamPhoton* >::const_iterator gam = beam_photons.begin();
				gam != beam_photons.end(); gam++ ) {
				
				
				double eb    = (*gam)->lorentzMomentum().E();
				double tb    = (*gam)->time();
				
				double brfdt = tb - rfTime;
				double  bfdt = tb - t1;
				double  bcdt = tb - t2;
				
				h_beam_rf_dt->Fill( brfdt );
				h_beam_fcal_dt->Fill( bfdt );
				h_beam_ccal_dt->Fill( bcdt );
				
				int bunch_val;
				
				if( fabs(brfdt) < 2.004 )
					bunch_val = 1;
				else if( (  -(2.004 + 3.*4.008) <= brfdt && brfdt <= -(2.004 + 2.*4.008) )
					|| ( (2.004 + 2.*4.008) <= brfdt && brfdt <=  (2.004 + 3.*4.008) ) )
					bunch_val = 0;
				else 
					continue;
				
				
				if( eb < MIN_BEAM_ENERGY_CUT ) continue;
				
				
				double ecomp1   =   1. / ( (1./eb)  +  (1./m_e)*(1. - cos(theta1)) );
				double ecomp2   =   1. / ( (1./eb)  +  (1./m_e)*(1. - cos(theta2)) );
				double deltaE   =  (e1     + e2    ) - (eb + m_e);
				double deltaK   =  (ecomp1 + ecomp2) - (eb + m_e);
				
				
				ComptonCandidate_t loc_Cand;
				
				loc_Cand.bunch_val = bunch_val;
				
				loc_Cand.e1        = e1;
				loc_Cand.x1        = pos1.X();
				loc_Cand.y1        = pos1.Y();
				loc_Cand.z1        = pos1.Z();
				loc_Cand.e2        = e2;
				loc_Cand.x2        = pos2.X();
				loc_Cand.y2        = pos2.Y();
				loc_Cand.z2        = pos2.Z();
				
				loc_Cand.ring      = ring1;
				
				loc_Cand.deltaT    = deltaT;
				loc_Cand.deltaPhi  = deltaPhi;
				loc_Cand.dTheta    = dTheta;
				loc_Cand.deltaR    = deltaR;
				loc_Cand.deltaE    = deltaE;
				loc_Cand.deltaK    = deltaK;
				
				loc_Cand.eb          = eb;
				/*
				loc_Cand.tag_counter = (*gam)->dCounter;
				DetectorSystem_t sys = (*gam)->dSystem;
				if( sys==SYS_TAGH )      loc_Cand.tag_sys = 0;
				else if( sys==SYS_TAGM ) loc_Cand.tag_sys = 1;
				*/
				candidates.push_back( loc_Cand );
				
				
			} // end DBeamPhoton loop
			
		} // end DCCALShower loop
		
	} // end DFCALShower loop
	
	
	
	
	fill_histograms( candidates );
	
	
	
	
	
	japp->RootFillUnLock(this);  // Release root lock
	
	
	
	
	
	
  	return NOERROR;
}




//------------------
// erun
//------------------
jerror_t JEventProcessor_compton::erun(void)
{
  	
  	return NOERROR;
}




//------------------
// fini
//------------------
jerror_t JEventProcessor_compton::fini(void)
{
	
  	return NOERROR;
}


//--------------------------------------------
// Get CCAL layer from row and column
//--------------------------------------------
int JEventProcessor_compton::ccalLayer(int row, int col) {
	
	
	int layer;
	
	if( (row > 3 && row < 8) && (col > 3 && col < 8) ) layer = 1;
	else if( (row > 2 && row <  9) && (col > 2 && col <  9) ) layer = 2;
	else if( (row > 1 && row < 10) && (col > 1 && col < 10) ) layer = 3;
	else if( (row > 0 && row < 11) && (col > 0 && col < 11) ) layer = 4;
	else layer = 5;	
	
	return layer;
}


//--------------------------------------------
// Get FCAL layer from row and column
//--------------------------------------------
int JEventProcessor_compton::fcalLayer(int row, int col) {
	
	
	int layer;
	
	if( (row > 26 && row < 32) && (col > 26 && col < 32) ) layer = 1;
	else if( (row > 25 && row < 33) && (col > 25 && col < 33) ) layer =  2;
	else if( (row > 24 && row < 34) && (col > 24 && col < 34) ) layer =  3;
	else if( (row > 23 && row < 35) && (col > 23 && col < 35) ) layer =  4;
	else if( (row > 22 && row < 36) && (col > 22 && col < 36) ) layer =  5;
	else if( (row > 21 && row < 37) && (col > 21 && col < 37) ) layer =  6;
	else if( (row > 20 && row < 38) && (col > 20 && col < 38) ) layer =  7;
	else if( (row > 19 && row < 39) && (col > 19 && col < 39) ) layer =  8;
	else if( (row > 18 && row < 40) && (col > 18 && col < 40) ) layer =  9;
	else if( (row > 17 && row < 41) && (col > 17 && col < 41) ) layer = 10;
	else layer = 11;
	
	
	return layer;
}



//--------------------------------------------
// Fill Histograms
//--------------------------------------------
void JEventProcessor_compton::fill_histograms( vector< ComptonCandidate_t > Comp_Candidates ) {
	
	
	
	int n_candidates  =  static_cast<int>( Comp_Candidates.size() );
	
	for( int ic = 0; ic < n_candidates; ic++ ) {
				
		ComptonCandidate_t loc_Cand = Comp_Candidates[ic];
				
		//-------------------------------------------//
		
		int bunch_val        =  loc_Cand.bunch_val;
		
		double eb            =  loc_Cand.eb;
		/*
		int tag_sys          =  loc_Cand.tag_sys;
		int tag_counter      =  loc_Cand.tag_counter;
		*/
		
		double deltaPhi      =  loc_Cand.deltaPhi;
		double dTheta        =  loc_Cand.dTheta;
		double deltaR        =  loc_Cand.deltaR;
		double deltaT        =  loc_Cand.deltaT;
		double deltaE        =  loc_Cand.deltaE;
		double deltaK        =  loc_Cand.deltaK;
		
		int ring1            =  loc_Cand.ring;
		
		double e1            =  loc_Cand.e1;
		double e2            =  loc_Cand.e2;
		
		double x1            =  loc_Cand.x1;
		double y1            =  loc_Cand.y1;
		double x2            =  loc_Cand.x2;
		double y2            =  loc_Cand.y2;
		
		
		//--------------     Cuts      --------------//
		
		
		/*
		double   deltaE_mu,   deltaE_sig;
		double   deltaK_mu,   deltaK_sig;
		double deltaPhi_mu, deltaPhi_sig;
		
		if( tag_sys==0 ) {
			
			deltaE_mu    = deltaE_mu_tagh[tag_counter];
			deltaE_sig   = deltaE_sig_tagh[tag_counter];
			
			deltaK_mu    = deltaK_mu_tagh[tag_counter];
			deltaK_sig   = deltaK_sig_tagh[tag_counter];
			
			deltaPhi_mu  = deltaPhi_mu_tagh[tag_counter];
			deltaPhi_sig = deltaPhi_sig_tagh[tag_counter];
			
		} else {
			
			deltaE_mu    = deltaE_mu_tagm[tag_counter];
			deltaE_sig   = deltaE_sig_tagm[tag_counter];
			
			deltaK_mu    = deltaK_mu_tagm[tag_counter];
			deltaK_sig   = deltaK_sig_tagm[tag_counter];
			
			deltaPhi_mu  = deltaPhi_mu_tagm[tag_counter];
			deltaPhi_sig = deltaPhi_sig_tagm[tag_counter];
			
		}
		*/
		
		double deltaPhi_mu   =   deltaPhi_mu_p0  +  (  deltaPhi_mu_p1/sqrt(eb) )  +  (  deltaPhi_mu_p2/eb );
		double deltaPhi_sig  =  deltaPhi_sig_p0  +  ( deltaPhi_sig_p1/sqrt(eb) )  +  ( deltaPhi_sig_p2/eb );
		
		double deltaE_mu     =   deltaE_mu_p0  +   deltaE_mu_p1*pow(eb,1.0)   +  deltaE_mu_p2*pow(eb,2.0)  +  deltaE_mu_p3*pow(eb,3.0);
		double deltaE_sig    =  deltaE_sig_p0  +  ( deltaE_sig_p1/sqrt(eb) )  +  ( deltaE_sig_p2/eb );
		deltaE_sig          *=  eb;
		
		double deltaK_mu     =   deltaK_mu_p0  +   deltaK_mu_p1*pow(eb,1.0)   +   deltaK_mu_p2*pow(eb,2.0)  +   deltaK_mu_p3*pow(eb,3.0);
		double deltaK_sig    =  deltaK_sig_p0  +  deltaK_sig_p1*pow(eb,1.0)   +  deltaK_sig_p2*pow(eb,2.0)  +  deltaK_sig_p3*pow(eb,3.0);
		deltaK_sig          *=  eb;
		
		
		int p5_cut = 0;
		if( fabs(deltaPhi - deltaPhi_mu) < 5.0*deltaPhi_sig ) { p5_cut = 1; }
		
		int e5_cut = 0;
		if( fabs(deltaE   - deltaE_mu  ) < 5.0*deltaE_sig   ) { e5_cut = 1; }
		
		int k5_cut = 0;
		if( fabs(deltaK   - deltaK_mu  ) < 5.0*deltaK_sig   ) { k5_cut = 1; }
		
		
		//-------------------------------------------//
		
		
		double fill_weight;
		if( bunch_val ) fill_weight =  1.0;
		else            fill_weight = -0.5;
		
		
		
		
		h1_deltaPhi->Fill( deltaPhi, fill_weight );
		h2_deltaPhi->Fill( eb, deltaPhi, fill_weight );
		
		if( e5_cut ) {
			
			h1_deltaPhi_e->Fill( deltaPhi, fill_weight );
			h2_deltaPhi_e->Fill( eb, deltaPhi, fill_weight );
			
			if( k5_cut ) {
				
				h1_deltaPhi_ek->Fill( deltaPhi, fill_weight );
				h2_deltaPhi_ek->Fill( eb, deltaPhi, fill_weight );
				
			}
		}
		
		
		
		
		h1_dTheta->Fill( dTheta, fill_weight );
		h1_deltaR->Fill( deltaR, fill_weight );
		h1_deltaT->Fill( deltaT, fill_weight );
		
		h2_dTheta->Fill( eb, dTheta, fill_weight );
		h2_deltaR->Fill( eb, deltaR, fill_weight );
		h2_deltaT->Fill( eb, deltaT, fill_weight );
		
		if( p5_cut ) {
			
			h1_dTheta_p->Fill( dTheta, fill_weight );
			h1_deltaR_p->Fill( deltaR, fill_weight );
			h1_deltaT_p->Fill( deltaT, fill_weight );
			
			h2_dTheta_p->Fill( eb, dTheta, fill_weight );
			h2_deltaR_p->Fill( eb, deltaR, fill_weight );
			h2_deltaT_p->Fill( eb, deltaT, fill_weight );
			
			if( e5_cut ) {
				
				h1_dTheta_pe->Fill( dTheta, fill_weight );
				h1_deltaR_pe->Fill( deltaR, fill_weight );
				h1_deltaT_pe->Fill( deltaT, fill_weight );
				
				h2_dTheta_pe->Fill( eb, dTheta, fill_weight );
				h2_deltaR_pe->Fill( eb, deltaR, fill_weight );
				h2_deltaT_pe->Fill( eb, deltaT, fill_weight );
				
				if( k5_cut ) {
					
					h1_dTheta_pek->Fill( dTheta, fill_weight );
					h1_deltaR_pek->Fill( deltaR, fill_weight );
					h1_deltaT_pek->Fill( deltaT, fill_weight );
					
					h2_dTheta_pek->Fill( eb, dTheta, fill_weight );
					h2_deltaR_pek->Fill( eb, deltaR, fill_weight );
					h2_deltaT_pek->Fill( eb, deltaT, fill_weight );
					
				}
			}
		}
		
		
		
		h1_deltaE->Fill( deltaE, fill_weight );
		h1_deltaE_ring[ring1]->Fill( deltaE, fill_weight );
		
		h2_deltaE->Fill( eb, deltaE, fill_weight );
		h2_deltaE_ring[ring1]->Fill( eb, deltaE, fill_weight );
		
		h2_deltaE_v_fcalE->Fill( e1, deltaE, fill_weight );
		h2_deltaE_v_ccalE->Fill( e2, deltaE, fill_weight );
		h2_deltaE_v_fcalE_ring[ring1]->Fill( e1, deltaE, fill_weight );
		h2_deltaE_v_ccalE_ring[ring1]->Fill( e2, deltaE, fill_weight );
		
		
		if( p5_cut ) {
			
			h1_deltaE_p->Fill( deltaE, fill_weight );
			h1_deltaE_p_ring[ring1]->Fill( deltaE, fill_weight );
			
			h2_deltaE_p->Fill( eb, deltaE, fill_weight );
			h2_deltaE_p_ring[ring1]->Fill( eb, deltaE, fill_weight );
			
			if( k5_cut ) {
				
				h1_deltaE_pk->Fill( deltaE, fill_weight );
				h1_deltaE_pk_ring[ring1]->Fill( deltaE, fill_weight );
				
				h2_deltaE_pk->Fill( eb, deltaE, fill_weight );
				h2_deltaE_pk_ring[ring1]->Fill( eb, deltaE, fill_weight );
				
			}
		}
		
		
		
		h1_deltaK->Fill( deltaK, fill_weight );
		h1_deltaK_ring[ring1]->Fill( deltaK, fill_weight );
		
		h2_deltaK->Fill( eb, deltaK, fill_weight );
		h2_deltaK_ring[ring1]->Fill( eb, deltaK, fill_weight );
		
		if( p5_cut ) {
			
			h1_deltaK_p->Fill( deltaK, fill_weight );
			h1_deltaK_p_ring[ring1]->Fill( deltaK, fill_weight );
			
			h2_deltaK_p->Fill( eb, deltaK, fill_weight );
			h2_deltaK_p_ring[ring1]->Fill( eb, deltaK, fill_weight );
			
			if( e5_cut ) {
				
				h1_deltaK_pe->Fill( deltaK, fill_weight );
				h1_deltaK_pe_ring[ring1]->Fill( deltaK, fill_weight );
				
				h2_deltaK_pe->Fill( eb, deltaK, fill_weight );
				h2_deltaK_pe_ring[ring1]->Fill( eb, deltaK, fill_weight );
				
			}
		}
		
		
		if( p5_cut && k5_cut && e5_cut ) {
			
			h_fcal_xy->Fill( x1, y1 );
			h_ccal_xy->Fill( x2, y2 );
			
			h_fcal_xy_ring[ring1]->Fill( x1, y1 );
			h_ccal_xy_ring[ring1]->Fill( x2, y2 );
			
		}
		
		
		
	}
	
	
	
	
	
	
	return;
}




















