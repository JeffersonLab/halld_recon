
#include "JEventProcessor_FCAL_TimingOffsets_Primex.h"
#include "DANA/DEvent.h"
#include <TDirectoryFile.h>
#include <map>

extern "C"{
void InitPlugin(JApplication *app){
	InitJANAPlugin(app);
    	app->Add(new JEventProcessor_FCAL_TimingOffsets_Primex());
}
} // "C"




//------------------
// Init
//------------------
void JEventProcessor_FCAL_TimingOffsets_Primex::Init()
{
	auto app = GetApplication();
	lockService = app->GetService<JLockService>();
  	
	TDirectory *dir_FCAL = new TDirectoryFile( "FCAL_TimingOffsets", "FCAL_TimingOffsets" );
  	dir_FCAL->cd();
	
	
	h_nfcal                = new TH1I( "nfcal", "Number of FCAL Hits", 300, -0.5, 299.5 );
	h_nfcal_cluster        = new TH1I( "nfcal_cluster", "Number of FCAL Hits in Cluster", 
		300, -0.5, 299.5 );
	
	h_fcal_rf_dt           = new TH1I( "fcal_rf_dt", 
		"FCAL Hit Time - Event RF Time; t_{FCAL} - t_{RF} [ns]", 4000, -200., 200. );
	
	h_fcal_rf_dt_VS_chan   = new TH2I( "fcal_rf_dt_VS_chan", 
		"FCAL Hit Time - Event RF Time; FCAL Channel Number; t_{FCAL} - t_{RF} [ns]", 
		2800, -0.5, 2799.5, 1000, -50., 50. );
	
	h_fcal_rf_dt_show      = new TH1I( "fcal_rf_dt_show", 
		"FCAL Shower Time - Event RF Time; t_{FCAL} - t_{RF} [ns]", 4000, -200., 200. );
	
	
	
	h_fcal_beam_dt 	       = new TH1I( "fcal_beam_dt", 
		"FCAL Hit Time - Beam Time; t_{FCAL} - t_{#gamma} [ns]", 4000, -200., 200. );
	
	h_fcal_beam_dt_VS_chan = new TH2I( "fcal_beam_dt_VS_chan", 
		"FCAL Hit Time - Beam Photon Time; FCAL Channel Number; t_{FCAL} - t_{#gamma} [ns]", 
		2800, -0.5, 2799.5, 1000, -50., 50. );
	
	
	
	h_tof_dx0  = new TH1I( "tof_dx0", "FCAL - TOF #Deltax; [cm]", 10000, -50., 50. );
	h_tof_dy0  = new TH1I( "tof_dy0", "FCAL - TOF #Deltay; [cm]", 10000, -50., 50. );
	h_tof_dr0  = new TH1I( "tof_dr0", "FCAL - TOF #Deltar; [cm]",  5000,   0., 50. );
	
	h_tof_dx1  = new TH1I( "tof_dx1", "FCAL - TOF #Deltax; [cm]", 10000, -50., 50. );
	h_tof_dy1  = new TH1I( "tof_dy1", "FCAL - TOF #Deltay; [cm]", 10000, -50., 50. );
	h_tof_dr1  = new TH1I( "tof_dr1", "FCAL - TOF #Deltar; [cm]",  5000,   0., 50. );
	
	h_tof_dx2  = new TH1I( "tof_dx2", "FCAL - TOF #Deltax; [cm]", 10000, -50., 50. );
	h_tof_dy2  = new TH1I( "tof_dy2", "FCAL - TOF #Deltay; [cm]", 10000, -50., 50. );
	h_tof_dr2  = new TH1I( "tof_dr2", "FCAL - TOF #Deltar; [cm]",  5000,   0., 50. );
	
	h_tof_dx3  = new TH1I( "tof_dx3", "FCAL - TOF #Deltax; [cm]", 10000,   0., 50. );
	h_tof_dy3  = new TH1I( "tof_dy3", "FCAL - TOF #Deltay; [cm]", 10000,   0., 50. );
	h_tof_dr3  = new TH1I( "tof_dr3", "FCAL - TOF #Deltar; [cm]",  5000,   0., 50. );
	
	
	dir_FCAL->cd( "../" );
	
}




//------------------
// BeginRun
//------------------
void JEventProcessor_FCAL_TimingOffsets_Primex::BeginRun(const std::shared_ptr<const JEvent>& event)
{
	
	DGeometry*   dgeom = GetDGeometry(event);

  	if( dgeom ){
    	  	dgeom->GetTargetZ( targetZ );
		dgeom->GetFCALPosition( fcalX, fcalY, fcalZ );
  	} else{
    	  	cerr << "No geometry accessible to FCAL_TimingOffsets_Primex plugin." << endl;
    	  	throw JException("No geometry accessible to FCAL_TimingOffsets_Primex plugin.");
  	}
	
	
	JCalibration *jcalib = GetJCalibration(event);
  	std::map<string, float> beam_spot;
	jcalib->Get("PHOTON_BEAM/beam_spot", beam_spot);
  	beamX = beam_spot.at("x");
  	beamY = beam_spot.at("y");
}




//------------------
// Process
//------------------
void JEventProcessor_FCAL_TimingOffsets_Primex::Process(const std::shared_ptr<const JEvent>& event)
{
	
	
	const DL1Trigger *trig = NULL;
  	try {
      	  	event->GetSingle(trig);
  	} catch (...) {}
	if (trig == NULL) { return; }
	
	uint32_t fp_trigmask = trig->fp_trig_mask;
	if( fp_trigmask ) return;
	
	
	
	
	
	
	//---------------   FCAL-RF Timing Shifts   ---------------//	
	
	
	const DEventRFBunch *locRFBunch = NULL;
	try { 
	  	event->GetSingle( locRFBunch, "CalorimeterOnly" );
	} catch (...) { return; }
	double rfTime = locRFBunch->dTime;
	if( locRFBunch->dNumParticleVotes < 2 ) return;
	
	
	
	vector< const DFCALHit*    > fcal_hits;
	event->Get( fcal_hits    );
	
	vector< const DFCALShower* > fcal_showers;
	event->Get( fcal_showers );
	
	vector< const DTOFPoint*   > tof_points;
	event->Get( tof_points );
	
	vector< const DBeamPhoton* > beam_photons;
	event->Get( beam_photons );
	
	int n_fcal_hits = (int)fcal_hits.size();
	

	DVector3 vertex;
	vertex.SetXYZ( beamX, beamY, targetZ );

    // fill some caches of the objects retrived using the Get() function,
    // which is notoriously slow (see FCAL_online for more details...)
    map< const DFCALShower*, const DFCALCluster* > cluster_map;
    for( auto &shower : fcal_showers ) {
    	const DFCALCluster* cluster = nullptr;
    	shower->GetSingle(cluster);
    	cluster_map[shower] = cluster;
    }
    
	
	lockService->RootFillLock(this);  // Acquire root lock

	int isGoodEvent = 0;
	
	for( vector< const DFCALShower* >::const_iterator show = fcal_showers.begin(); 
		show != fcal_showers.end(); show++ ) {
		
		
		double energy = (*show)->getEnergy();
		if( energy < MIN_FCAL_ENERGY_TimingCalib ) continue;
		
		double pos_corrected_z = (*show)->getPosition().Z();
		
		
		
		int tof_match = check_TOF_match( (*show), tof_points, vertex );
		if( tof_match ) continue;
		
		
		
		double show_x = (*show)->getPosition().X() - vertex.X();
		double show_y = (*show)->getPosition().Y() - vertex.Y();
		double show_z = (*show)->getPosition().Z() - vertex.Z();
		double show_r = sqrt( show_x*show_x  +  show_y*show_y  +  show_z*show_z );
		double show_t = (*show)->getTime()  -  (show_r/c);
		
		h_fcal_rf_dt_show->Fill( show_t - rfTime );
		
		
		
		
		
		//----------   Loop over hits belonging to this shower   ----------//
		
		
		
		const DFCALCluster *clus = cluster_map[*show];
		if(clus == nullptr)  continue;
		const vector< DFCALCluster::DFCALClusterHit_t > hitVector = clus->GetHits();
		
		int loc_n_hits = (int)hitVector.size();
		if( loc_n_hits < 1 ) continue;
		
		
		
		// only use the hit with maximum energy deposition:
		
		DFCALCluster::DFCALClusterHit_t fcalHit = hitVector[0];
		
		
		
		double  hitE      =  fcalHit.E;
		if( hitE < MIN_FCAL_ENERGY_TimingCalib ) continue;
		
		
		int ChannelNumber =  fcalHit.ch;
		double chanx      =  fcalHit.x       - vertex.X()  +  fcalX;
		double chany      =  fcalHit.y       - vertex.Y()  +  fcalY;
		double chanz      =  pos_corrected_z - vertex.Z();
		double hitTime    =  fcalHit.t;
		
		double dR = sqrt( chanx*chanx  +  chany*chany  +  chanz*chanz );
		
		
		
		// propagate hit time to the interaction vertex:
		
		double tCorr   = ( fcalZ + DFCALGeometry::blockLength() - pos_corrected_z ) 
			/ FCAL_C_EFFECTIVE;
		
		hitTime = hitTime  -  tCorr  -  (dR/c);
		
		
		
		double fcal_rf_dt = hitTime - rfTime;
		
		h_fcal_rf_dt->Fill( fcal_rf_dt );
		h_fcal_rf_dt_VS_chan->Fill( ChannelNumber, fcal_rf_dt );
		
		h_nfcal_cluster->Fill( loc_n_hits );
		
		
		
		// Also look at time difference between beam photon:
		
		for( unsigned int ib = 0; ib < beam_photons.size(); ib++ ) {
			
			double beam_time    = beam_photons[ib]->time();
			double fcal_beam_dt = hitTime  -  beam_time;
			
			h_fcal_beam_dt->Fill( fcal_beam_dt );
			h_fcal_beam_dt_VS_chan->Fill( ChannelNumber, fcal_beam_dt );
			
		}
		
		
		isGoodEvent++;
		
		
		
		/*
		for( unsigned int iHit = 0; iHit < hitVector.size(); iHit++ ) {
			
			double hitE       =  hitVector[iHit].E;
			if( hitE <= 0.4 ) continue;
			
			double hitT       =  hitVector[iHit].t;
			double tCorr      =  ( fcalZ + DFCALGeometry::blockLength() - pos_corrected_z )
				/ FCAL_C_EFFECTIVE;
			hitT             -=  tCorr;
			
			int ChannelNumber =  hitVector[iHit].ch;
			double chanx      =  hitVector[iHit].x  -  vertex.X()  +  fcalX;
			double chany      =  hitVector[iHit].y  -  vertex.Y()  +  fcalY;
			double chanz      =  pos_corrected_z    -  vertex.Z();
			double flightT    =  sqrt( chanx*chanx  +  chany*chany  +  chanz*chanz ) / c;
			
			hitT             -=  flightT;
			
			
			double fcal_rf_dt = hitT  -  rfTime;
			
			h_fcal_rf_dt->Fill( fcal_rf_dt );
			h_fcal_rf_dt_VS_chan->Fill( ChannelNumber, fcal_rf_dt );
			
			isGoodEvent++;
		}
		*/
		
		
		
		
		
		
	}
	
	
	if( isGoodEvent ) h_nfcal->Fill( n_fcal_hits );
	
	
	lockService->RootFillUnLock(this);  // Release root lock
	
	
	
	
}




//------------------
// EndRun
//------------------
void JEventProcessor_FCAL_TimingOffsets_Primex::EndRun()
{
}




//------------------
// Finish
//------------------
void JEventProcessor_FCAL_TimingOffsets_Primex::Finish()
{
}







//--------------------------------------------
// Get TOF position from bar
//--------------------------------------------
double JEventProcessor_FCAL_TimingOffsets_Primex::bar2x(int bar) {

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







//--------------------------------------------
// Check for TOF-FCAL Matches
//--------------------------------------------
int JEventProcessor_FCAL_TimingOffsets_Primex::check_TOF_match( const DFCALShower* show, 
	vector< const DTOFPoint* > tof_points, DVector3 vertex ) {
	
	
	int tof_match = 0;
	
	double x1 =  show->getPosition().X()  -  vertex.X();
	double y1 =  show->getPosition().Y()  -  vertex.Y();
	double z1 =  show->getPosition().Z()  -  vertex.Z();
	
	for( vector< const DTOFPoint* >::const_iterator tof = tof_points.begin(); 
		tof != tof_points.end(); tof++ ) {
		
		
		double xt = (*tof)->pos.X() - vertex.X();
		double yt = (*tof)->pos.Y() - vertex.Y();
		double zt = (*tof)->pos.Z() - vertex.Z();
		
		
		xt *= z1 / zt;
		yt *= z1 / zt;
		
		
		int hbar  = (*tof)->dHorizontalBar;
		int hstat = (*tof)->dHorizontalBarStatus;
		int vbar  = (*tof)->dVerticalBar;
		int vstat = (*tof)->dVerticalBarStatus;
		
		double dx, dy, dr;
		
		if( hstat==3 && vstat==3 ) // both planes have hits in both ends
		{
			dx  =  x1 - xt;
			dy  =  y1 - yt;
			dr  =  sqrt( dx*dx + dy*dy );
			
			h_tof_dx0->Fill( dx );
			h_tof_dy0->Fill( dy );
			h_tof_dr0->Fill( dr );
			
		} else if( hstat==3 ) // only good position information in horizontal plane
		{
			dx = x1 - xt;
			dy = y1 - (bar2x(hbar) - vertex.Y()) * (z1/zt);
			dr  =  sqrt( dx*dx + dy*dy );
			
			h_tof_dx1->Fill( dx );
			h_tof_dy1->Fill( dy );
			h_tof_dr1->Fill( dr );
			
		} else if( vstat==3 ) 
		{
			dx = x1 - (bar2x(vbar) - vertex.X()) * (z1/zt);
			dy = y1 - yt;
			dr  =  sqrt( dx*dx + dy*dy );
			
			h_tof_dx2->Fill( dx );
			h_tof_dy2->Fill( dy );
			h_tof_dr2->Fill( dr );
			
		} else
		{
			dx = x1 - (bar2x(vbar) +  - vertex.X()) * (z1/zt);
			dy = y1 - (bar2x(hbar) - vertex.Y()) * (z1/zt);
			dr  =  sqrt( dx*dx + dy*dy );
			
			h_tof_dx3->Fill( dx );
			h_tof_dy3->Fill( dy );
			h_tof_dr3->Fill( dr );
			
		}
		
		if( ( fabs(dx - TOF_X_MEAN) < 3.*TOF_X_SIG ) && 
		    ( fabs(dy - TOF_Y_MEAN) < 3.*TOF_Y_SIG ) ) { tof_match = 1; }
		
		
	}
	
	
	return tof_match;
}


