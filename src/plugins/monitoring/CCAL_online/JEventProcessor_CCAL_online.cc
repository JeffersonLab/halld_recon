// $Id$
//
//    File: JEventProcessor_CCAL_online.cc
// Created: Fri Nov  9 11:58:09 EST 2012
// Creator: wolin (on Linux stan.jlab.org 2.6.32-279.11.1.el6.x86_64 x86_64)
//

#include <stdint.h>
#include <vector>
#include <iostream>
#include "JEventProcessor_CCAL_online.h"
#include <JANA/JApplication.h>

#include <CCAL/DCCALDigiHit.h>
#include <CCAL/DCCALHit.h>
#include <CCAL/DCCALGeometry.h>
#include <CCAL/DCCALShower.h>

#include <FCAL/DFCALShower.h>

#include <PID/DBeamPhoton.h>

#include <DAQ/Df250PulseIntegral.h>
#include <DAQ/Df250PulsePedestal.h>
#include <DAQ/Df250PulseData.h>

#include "units.h"
#include "DLorentzVector.h"
#include "DVector3.h"
#include "HDGEOMETRY/DGeometry.h"
#include "DANA/DApplication.h"
#include "DANA/DStatusBits.h"
#include "TRIGGER/DL1Trigger.h"
#include "TRIGGER/DTrigger.h"

#include <TDirectory.h>
#include <TH2F.h>
#include <TH1I.h>
#include <TH2I.h>
#include <TProfile.h>
#include <TProfile2D.h>

using namespace std;
using namespace jana;


//----------------------------------------------------------------------------------


// Routine used to create our JEventProcessor
extern "C"{
  void InitPlugin(JApplication *app){
    InitJANAPlugin(app);
    app->AddProcessor(new JEventProcessor_CCAL_online());
  }
}


//----------------------------------------------------------------------------------


JEventProcessor_CCAL_online::JEventProcessor_CCAL_online() {
}


//----------------------------------------------------------------------------------


JEventProcessor_CCAL_online::~JEventProcessor_CCAL_online() {
}


//----------------------------------------------------------------------------------

jerror_t JEventProcessor_CCAL_online::init(void) {

  // create root folder for fcal and cd to it, store main dir
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

  hclusN      = new TH1I("clusN", "CCAL Number of Clusters; Number of Clusters; Events", 10, -0.5, 9.5 );
  hclusE      = new TH1I("clusE", "CCAL Cluster Energy; Energy [MeV]; Clusters / 50 MeV", 100, 0, 15000 );
  hclusETot   = new TH1I("clusETot", "CCAL Cluster Total Energy; Energy [MeV]; Events / 100 MeV", 100, 0, 15000 );
  hclusT      = new TH1I("clusT", "CCAL Cluster Time; t [ns]; Clusters / 4 ns", 100, -100, 300 );
  hclusDime   = new TH1I("clusDime", "CCAL Cluster Dimension; Modules in Cluster", 50, -0.5, 49.5);
  hclusXYHigh = new TH2I("clusXYHigh", "CCAL Cluster Positions (E > 200 MeV); x [cm]; y [cm]", 100, -12.5, 12.5, 100, -12.5, 12.5 );
  hclusXYLow  = new TH2I("clusXYLow", "CCAL Cluster Positions (E < 200 MeV); x [cm]; y [cm]", 100, -12.5, 12.5, 100, -12.5, 12.5 );
  hclusPhi    = new TH1I("clusPhi", "CCAL Cluster #phi; #phi [rad]; Clusters / 62.8 mrad", 100, -3.14, 3.14 );
  hclusPhi->SetMinimum( 0 );
  hclus2GMass = new TH1I( "clus2GMass", "CCAL 2 Cluster Invariant Mass E > 1 GeV; Invariant Mass [GeV]", 500, 0.0, 0.6 );
  hclus2GMass_fcal = new TH1I("clus2GMass_fcal", "Invariant Mass, 1 CCAL & 1 FCAL Cluster E > 1 GeV; Invariant Mass [GeV]", 500, 0.0, 0.6 );
  hclusOccEmax = new TH2I("clusOccEmax","Occupancy when E_{max} > 3 GeV and E_{cl}-E_{max} < 1.0 GeV; column; row", 12, -0.5, 11.5, 12, -0.5, 11.5); 

  // Compton Plots
  
  hcomp_bfdt         = new TH1F("comp_bfdt","Beam Time - FCAL Shower Time; t_{beam}-t_{fcal} [ns]; Counts / 0.5 ns", 400, -100., 100.);
  hcomp_fcdt         = new TH1F("comp_fcdt","CCAL Time - FCAL Time; t_{fcal}-t_{ccal} [ns]; Counts / 0.5 ns", 400, -100., 100.);
  hcomp_bcdt_full    = new TH1F("comp_bcdt_full","Beam Time - CCAL Shower Time (No Cuts); t_{beam}-t_{ccal} [ns]; Counts / 0.5 ns", 400, -100., 100.);
  
  hcomp_cratio   = new TH1F("comp_cratio", "CCAL Energy over Calculated Compton Energy; #frac{E_{ccal}}{E_{comp}}", 200, 0., 2.);
  hcomp_cfbratio = new TH1F("comp_cfbratio", "Energy Conservation; #frac{E_{ccal}+E_{fcal}-E_{beam}}{E_{beam}}", 200, -1., 1.);
  hcomp_cfb2d    = new TH2F("comp_cfb2d", "Reconstructed Energy vs. Calculated Compton Energy; #frac{E_{ccal,comp}+E_{fcal,comp}}{E_{beam}}; #frac{E_{ccal}+E_{fcal}}{E_{beam}}", 200, 0., 1.5, 200, 0., 1.5);
  hcomp_pfpc     = new TH1F("comp_pfpc", "FCAL - CCAL Azimuthal Angle; #phi_{fcal}-#phi_{ccal} [deg]; Counts / 0.5 degrees", 1440, -360., 360.);
  hcomp_cxy      = new TH2F("comp_cxy","Reconstructed Compton Positions in CCAL; x_{ccal} [cm]; y_{ccal} [cm]", 200, -12.5, 12.5, 200, -12.5, 12.5);
  hcomp_fxy      = new TH2F("comp_fxy","Reconstructed Compton Positions in FCAL; x_{fcal} [cm]; y_{fcal} [cm]", 200, -30., 30., 200, -30., 30.);
  hcomp_bcdt     = new TH1F("comp_bcdt","Beam Time - CCAL Shower Time; t_{beam}-t_{ccal} [ns]; Counts / 0.5 ns", 400, -100., 100.);
  
  hcomp_cratio_bkgd   = new TH1F("comp_cratio_bkgd", "CCAL Energy over Calculated Compton Energy (Accidentals); #frac{E_{ccal}}{E_{comp}}", 200, 0., 2.);
  hcomp_cfbratio_bkgd = new TH1F("comp_cfbratio_bkgd", "Energy Conservation (Accidentals); #frac{E_{ccal}+E_{fcal}-E_{beam}}{E_{beam}}", 200, -1., 1.);
  hcomp_cfb2d_bkgd    = new TH2F("comp_cfb2d_bkgd", "Reconstructed Energy vs. Calculated Compton Energy (Accidentals); #frac{E_{ccal,comp}+E_{fcal,comp}}{E_{beam}}; #frac{E_{ccal}+E_{fcal}}{E_{beam}}", 200, 0., 1.5, 200, 0., 1.5);
  hcomp_pfpc_bkgd     = new TH1F("comp_pfpc_bkgd", "FCAL - CCAL Azimuthal Angle (Accidentals); #phi_{fcal}-#phi_{ccal} [deg]; Counts / 0.5 degrees", 1440, -360., 360.);
  hcomp_cxy_bkgd      = new TH2F("comp_cxy_bkgd","Reconstructed Compton Positions in CCAL (Accidentals); x_{ccal} [cm]; y_{ccal} [cm]", 200, -12.5, 12.5, 200, -12.5, 12.5);
  hcomp_fxy_bkgd      = new TH2F("comp_fxy_bkgd","Reconstructed Compton Positions in FCAL (Accidentals); x_{fcal} [cm]; y_{fcal} [cm]", 200, -30., 30., 200, -30., 30.);
  hcomp_bcdt_bkgd     = new TH1F("comp_bcdt_bkgd","Beam Time - CCAL Shower Time (Accidentals); t_{beam}-t_{ccal} [ns]; Counts / 0.5 ns", 400, -100., 100.);
  
  hfcalOcc = new TH2F("fcalOcc","FCAL Occupancy for events > 0.5 GeV; x_{fcal} [cm]; y_{fcal} [cm]", 200, -30., 30., 200, -30., 30.);

  hNPhotons = new TH1I("NPhotons","Number of Beam Photons per Event",80,-0.5,79.5);

  // back to main dir
  main->cd();

  return NOERROR;
}


//----------------------------------------------------------------------------------


jerror_t JEventProcessor_CCAL_online::brun(JEventLoop *eventLoop, int32_t runnumber) {
  // This is called whenever the run number changes

  DGeometry *dgeom = NULL;
  DApplication *dapp = dynamic_cast< DApplication* >( eventLoop->GetJApplication() );
  if( dapp ) dgeom = dapp->GetDGeometry( runnumber );
   
  if( dgeom ){

    dgeom->GetTargetZ( m_targetZ );

  }
  else{

    cerr << "No geometry accessbile to CCAL_online monitoring plugin." << endl;
    return RESOURCE_UNAVAILABLE;
  }
  
  vector< double > ccal_gains_ch;
  eventLoop->GetCalib("/CCAL/gains",ccal_gains_ch);
  for(unsigned int ii=0; ii<ccal_gains_ch.size(); ii++) {
    cout << ii << " " << ccal_gains_ch[ii] << endl;
  }

  return NOERROR;
}


//----------------------------------------------------------------------------------


jerror_t JEventProcessor_CCAL_online::evnt(JEventLoop *eventLoop, uint64_t eventnumber) {


  vector< const DCCALGeometry* > geomVec;
  vector< const DCCALDigiHit*  > digiHits;
  vector< const DCCALHit*      > hits;
  vector< const DCCALShower*   > CCALshowerVec;
  vector< const DFCALShower*   > FCALshowerVec;
  vector< const DBeamPhoton*   > beam_photons;
  
  
  //------------------------------------------------------------------
  // First check that this is not a front panel trigger or no trigger
  
  bool goodtrigger=1;
  
  const DL1Trigger *trig = NULL;
  const DTrigger *dtrig = NULL;
  try {
      eventLoop->GetSingle(trig);
      eventLoop->GetSingle(dtrig);
  } catch (...) {}
  if (trig) {
      if (trig->fp_trig_mask){
          goodtrigger=0;
      }
      else if(!(dtrig->Get_IsPhysicsEvent())){
          goodtrigger=0;
      }
  } else {
      // HDDM files are from simulation, so keep them even though they have no trigger
      bool locIsHDDMEvent = eventLoop->GetJEvent().GetStatusBit(kSTATUS_HDDM);
      if (!locIsHDDMEvent) goodtrigger=0;	
  }
	
  if (!goodtrigger) {
      return NOERROR;
  }
  
  //------------------------------------------------------------------
  // Get data objects
  
  eventLoop->Get( geomVec );
  eventLoop->Get( digiHits );
  eventLoop->Get( hits );
  if( hits.size() <= 50 ) {
    eventLoop->Get( CCALshowerVec ); 
    eventLoop->Get( FCALshowerVec );
    eventLoop->Get( beam_photons );
  }
  const DCCALGeometry& ccalGeom = *(geomVec[0]);
  
  
  
  float hitETot = 0., showETot = 0.;
  for( vector< const DCCALHit*>::const_iterator hitItr = hits.begin();
       hitItr != hits.end(); ++hitItr ) { hitETot += (**hitItr).E; }
  for( vector< const DCCALShower*>::const_iterator showItr = CCALshowerVec.begin();
       showItr != CCALshowerVec.end(); ++showItr ) { showETot += (**showItr).E; }
  


  // FILL HISTOGRAMS
  // Since we are filling histograms local to this plugin, it will not interfere with other ROOT operations: can use plugin-wide ROOT fill lock
  japp->RootFillLock(this); //ACQUIRE ROOT FILL LOCK

  //----------------------------------------------------------------------------------
  // Fill DigiHit-Level plots


  if( digiHits.size() > 0 )
	  ccal_num_events->Fill(1);

  hdigN->Fill( digiHits.size() );  
  
  for( vector< const DCCALDigiHit* >::const_iterator dHitItr = digiHits.begin();
       dHitItr != digiHits.end(); ++dHitItr ) {

    const DCCALDigiHit& dHit = (**dHitItr);
    int chan = ccalGeom.channel( dHit.row, dHit.column );
    
    hdigOcc2D->Fill( dHit.column, dHit.row );
    hdigInt->Fill( dHit.pulse_integral );
    hdigPeak->Fill( dHit.pulse_peak );
    hdigT->Fill( dHit.pulse_time );
    hdigPed->Fill( dHit.pedestal );
    hdigPedChan->Fill( chan, dHit.pedestal );
    hdigPed2D->Fill( dHit.column, dHit.row, dHit.pedestal );
    hdigQF->Fill( dHit.QF );
    hdigIntVsPeak->Fill( dHit.pulse_peak, dHit.pulse_integral );

  }


  //----------------------------------------------------------------------------------
  // Fill Hit-Level plots


  hhitETot->Fill( hitETot );
  hhitN->Fill( hits.size() );
  
  for( vector< const DCCALHit* >::const_iterator hit_itr = hits.begin();
       hit_itr != hits.end(); ++hit_itr ){

    const DCCALHit& hit = (**hit_itr);

    hhitE->Fill( hit.E );
    hhitT->Fill( hit.t*k_to_nsec );
    hhitOcc2D->Fill( hit.column, hit.row );
    hhitiop->Fill( hit.intOverPeak );
    hhitE2D->Fill( hit.column, hit.row, hit.E );
    
  }



  // for events with a lot of hits -- stop now
  
  if( hits.size() > 50 ){
    japp->RootFillUnLock(this); //RELEASE ROOT FILL LOCK
    return NOERROR;
  }

  //
  // if there are a small number of hits go ahead
  // and run the clusterizer and make a few plots
  // utilizing the list of clusters and showers
  //



  //----------------------------------------------------------------------------------
  // Fill Shower-Level plots
  
  hclusN->Fill( CCALshowerVec.size() );

  if( CCALshowerVec.size() > 0 )
    hclusETot->Fill( showETot * k_to_MeV );

  for( vector< const DCCALShower*>::const_iterator showItr = CCALshowerVec.begin();
       showItr != CCALshowerVec.end(); ++showItr ){

    const DCCALShower& shower = (**showItr);
    
    double showX    = shower.x1;
    double showY    = shower.y1;
    double showR    = sqrt( showX*showX + showY*showY );
    double showE    = shower.E;
    double showEmax = shower.Emax;
    int idmax       = shower.idmax;
    int ccal_col    = idmax%12;
    int ccal_row    = idmax/12;

    DVector3 show1Mom;
    show1Mom.SetX( showX );
    show1Mom.SetY( showY );
    show1Mom.SetZ( ZCCAL );
    show1Mom.SetMag( showE );
    
    hclusPhi->Fill( show1Mom.Phi() );
    hclusE->Fill( showE * k_to_MeV );
    hclusT->Fill( shower.time );
    hclusDime->Fill( shower.dime );

    if( showE > 200*k_MeV )
      hclusXYHigh->Fill( showX, showY );
    else
      hclusXYLow->Fill( showX, showY );

    if( (showEmax > 3.0) && (showE - showEmax < 1.0) )
      hclusOccEmax->Fill( ccal_col, ccal_row );

    if( ( showE < 1*k_GeV ) ||
	( showR < 8*k_cm ) ||
	( shower.dime < 2 )
	) continue;

    for( vector< const DCCALShower*>::const_iterator show2Itr = showItr + 1;
	 show2Itr != CCALshowerVec.end(); ++show2Itr ){

      const DCCALShower& shower2 = (**show2Itr);

      double show2X = shower2.x;
      double show2Y = shower2.y;

      double show2R = sqrt( show2X * show2X + show2Y * show2Y );

      if( ( shower2.E < 1*k_GeV ) ||
	  ( show2R < 8*k_cm ) ||
	  ( shower2.dime < 2 )
	  ) continue;
 
      DVector3 show2Mom;
      show2Mom.SetX( show2X );
      show2Mom.SetY( show2Y );
      show2Mom.SetZ( ZCCAL );
      show2Mom.SetMag( shower2.E );

      DLorentzVector gam1( show1Mom, show1Mom.Mag() );
      DLorentzVector gam2( show2Mom, show2Mom.Mag() );

      hclus2GMass->Fill( ( gam1 + gam2 ).M() );
    }
  }


  //----------------------------------------------------------------------------------
  // FCAL + CCAL Invariant mass:

  for( vector< const DFCALShower*>::const_iterator showFItr = FCALshowerVec.begin();
       showFItr != FCALshowerVec.end(); ++showFItr ){

    const DFCALShower& showF = (**showFItr);
 
    DVector3 showFMom = showF.getPosition();
    showFMom.SetZ( showFMom.Z() - m_targetZ );
    showFMom.SetMag( showF.getEnergy() );
    
    if( ( showF.getEnergy() < 1*k_GeV ) ||
	( showF.getPosition().Pt() < 10*k_cm ) ) continue;
    
    for( vector< const DCCALShower*>::const_iterator showCItr = CCALshowerVec.begin();
       showCItr != CCALshowerVec.end(); ++showCItr ){
       
      const DCCALShower& showC = (**showCItr);
      
      double showCX   = showC.x1;
      double showCY   = showC.y1;
      double showCR   = sqrt( showCX*showCX + showCY*showCY );

      DVector3 showCMom;
      showCMom.SetX( showCX );
      showCMom.SetY( showCY );
      showCMom.SetZ( ZCCAL );
      showCMom.SetMag( showC.E );
      
      if( ( showC.E < 1*k_GeV ) ||
	  ( showCR < 5*k_cm ) ||
	  ( showC.dime < 2 )
	  ) continue;
	  
      DLorentzVector gam1( showFMom, showFMom.Mag() );
      DLorentzVector gam2( showCMom, showCMom.Mag() );
      
      hclus2GMass->Fill( ( gam1 + gam2 ).M() );
    }
  }


  //----------------------------------------------------------------------------------
  // compton analysis:
  
  hNPhotons->Fill( beam_photons.size() );
  for(unsigned int ib = 0; ib < beam_photons.size(); ++ib) {
    const DBeamPhoton *beam_photon = beam_photons[ib];
    double beam_e = beam_photon->lorentzMomentum().E();
    double beam_t = beam_photon->time();
    
    for(unsigned int ih = 0; ih < FCALshowerVec.size(); ++ih) {
      const DFCALShower *fcal_shower = FCALshowerVec[ih];
      double fcal_e = fcal_shower->getEnergy();
      double fcal_x = fcal_shower->getPosition().X();
      double fcal_y = fcal_shower->getPosition().Y();
      double fcal_z = fcal_shower->getPosition().Z();
      double fcal_t = fcal_shower->getTime();
      
      if(fcal_e<0.5) continue;
      
      hfcalOcc->Fill( fcal_x, fcal_y );
      
      double dt_fcal_bm = fcal_t - beam_t;
      hcomp_bfdt->Fill(dt_fcal_bm);
      
      double phif   = atan2(fcal_y,fcal_x);
      double rf     = hypot(fcal_x,fcal_y);
      double zf     = fcal_z - m_targetZ;
      double ecompf = 1. / ( 1./beam_e + (1.-cos(rf/zf)) / me);
      
      for(unsigned int ic = 0; ic < CCALshowerVec.size(); ++ic) {
        const DCCALShower *ccal_shower = CCALshowerVec[ic];
	double ccal_e = ccal_shower->E;
	double ccal_x = ccal_shower->x1;
	double ccal_y = ccal_shower->y1;
	double ccal_t = ccal_shower->time;
	
	if(ccal_e<1.5) continue;
	
	double dt_cc_bm = ccal_t - beam_t;	
	double dt_fc_cc = ccal_t - fcal_t;
	hcomp_fcdt->Fill( dt_fc_cc );
	hcomp_bcdt_full->Fill( dt_cc_bm );
	
	double phic   = atan2(ccal_y,ccal_x);
	double rc     = hypot(ccal_x,ccal_y);
	double zc     = ZCCAL - m_targetZ;
	double ecompc = 1. / (1./beam_e + (1.-cos(rc/zc)) / me);
	
	if(dt_fc_cc < 15. || dt_fc_cc > 28.) continue;
	
	if(dt_fcal_bm > 15. && dt_fcal_bm < 23.) {
	
	  hcomp_cratio->Fill( ccal_e/ecompc );
	  hcomp_cfbratio->Fill( (ccal_e+fcal_e-beam_e)/beam_e );
	  hcomp_cxy->Fill( ccal_x, ccal_y );
	  hcomp_fxy->Fill( fcal_x, fcal_y );
	  hcomp_cfb2d->Fill( (ecompc+ecompf)/beam_e, (ccal_e+fcal_e)/beam_e );
	  hcomp_pfpc->Fill( (phif-phic)*180./pi );
	  
	  hcomp_bcdt->Fill(dt_cc_bm);
	
	} else if( (dt_fcal_bm > -15. && dt_fcal_bm < 1.) || (dt_fcal_bm > 37. && dt_fcal_bm < 53.) ) {
	
	  hcomp_cratio_bkgd->Fill( ccal_e/ecompc );
	  hcomp_cfbratio_bkgd->Fill( (ccal_e+fcal_e-beam_e)/beam_e );
	  hcomp_cxy_bkgd->Fill( ccal_x, ccal_y );
	  hcomp_fxy_bkgd->Fill( fcal_x, fcal_y );
	  hcomp_cfb2d_bkgd->Fill( (ecompc+ecompf)/beam_e, (ccal_e+fcal_e)/beam_e );
	  hcomp_pfpc_bkgd->Fill( (phif-phic)*180./pi );
	  
	  hcomp_bcdt_bkgd->Fill(dt_cc_bm);
	
	}
	
      }
    }
  }  
    
  japp->RootFillUnLock(this); //RELEASE ROOT FILL LOCK
  return NOERROR;
}


//----------------------------------------------------------------------------------


jerror_t JEventProcessor_CCAL_online::erun(void) {
  // This is called whenever the run number changes, before it is
  // changed to give you a chance to clean up before processing
  // events from the next run number.
  return NOERROR;
}


//----------------------------------------------------------------------------------


jerror_t JEventProcessor_CCAL_online::fini(void) {
  // Called before program exit after event processing is finished.
  return NOERROR;
}


//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
