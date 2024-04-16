// $Id$
//
//    File: JEventProcessor_FCALLEDTree.cc
// Created: Fri May 19 12:39:24 EDT 2017
// Creator: mashephe (on Linux stanley.physics.indiana.edu 2.6.32-642.6.2.el6.x86_64 unknown)
//

#include "JEventProcessor_FCALLEDTree.h"
using namespace jana;

#include "FCAL/DFCALGeometry.h"
#include "FCAL/DFCALHit.h"
#include "FCAL/DFCALDigiHit.h"

#include "TTree.h"

// Routine used to create our JEventProcessor
#include <JANA/JApplication.h>
#include <JANA/JFactory.h>
extern "C"{
  void InitPlugin(JApplication *app){
    InitJANAPlugin(app);
    app->AddProcessor(new JEventProcessor_FCALLEDTree());
  }
} // "C"

static TH2F *hXYGeo;
static TH2F *hXYGeo_w;
static TH2F *hE;
static TH2F *hped;
static TH2F *hpeak;
static TH2F *hinteg;


//------------------
// JEventProcessor_FCALLEDTree (Constructor)
//------------------
JEventProcessor_FCALLEDTree::JEventProcessor_FCALLEDTree()
{
  btree = 0;
  gPARMS->SetDefaultParameter( "FCALLED:Tree", btree );
}

//------------------
// ~JEventProcessor_FCALLEDTree (Destructor)
//------------------
JEventProcessor_FCALLEDTree::~JEventProcessor_FCALLEDTree()
{

}

//------------------
// init
//------------------
jerror_t JEventProcessor_FCALLEDTree::init(void)
{
  // This is called once at program startup. 
  japp->RootWriteLock();
  
  if (btree == 1) {
  
    m_tree = new TTree( "fcalBlockHits", "FCAL Block Hits" );

    m_tree->Branch( "nHits", &m_nHits, "nHits/I" );
    m_tree->Branch( "chan", m_chan, "chan[nHits]/I" );
    m_tree->Branch( "x", m_x, "x[nHits]/F" );
    m_tree->Branch( "y", m_y, "y[nHits]/F" );
    m_tree->Branch( "E", m_E, "E[nHits]/F" );
    m_tree->Branch( "t", m_t, "t[nHits]/F" );
    m_tree->Branch( "integ", m_integ, "integ[nHits]/F" );
    m_tree->Branch( "ped", m_ped, "ped[nHits]/F" );
    m_tree->Branch( "peak", m_peak, "peak[nHits]/F" );
    
    m_tree->Branch( "run", &m_run, "run/I" );
    m_tree->Branch( "event", &m_event, "event/L" );
    m_tree->Branch( "eTot", &m_eTot, "eTot/F" );
  }

  // create root folder and cd to it, store main dir
  TDirectory *main = gDirectory;  // save current directory
  TDirectory *hvscandir = main->mkdir("hv_scan");
  hvscandir->cd();

  hXYGeo = new TH2F("XYGeo", ";row;column #;Counts", 59, 0, 59, 59, 0, 59);
  hXYGeo_w = new TH2F("XYGeo_w", ";row;column #;E_{max}^{sum} [GeV]", 59, 0, 59, 59, 0, 59);
  hE = new TH2F("m_E", ";channel;energy;Counts", 2800, 0, 2800, 1200, 0, 12.);
  hped = new TH2F("m_ped", ";channel;pedestal;Counts", 2800, 0, 2800, 4096, 0., 4096.);
  hpeak = new TH2F("m_peak", ";channel;peak;Counts", 2800, 0, 2800, 4096, 0., 4096.);
  hinteg = new TH2F("m_integ", ";channel;integ;Counts", 2800, 0, 2800, 4096, 0., 4096.);

  main->cd();
  
 japp->RootUnLock();
  
  return NOERROR;
}

//------------------
// brun
//------------------
jerror_t JEventProcessor_FCALLEDTree::brun(JEventLoop *eventLoop, int32_t runnumber)
{

  // this is not thread safe and may lead to an incorrect run number for
  // a few events in the worst case scenario -- I don't think it is a major problem
  m_run = runnumber;
  
  // This is called whenever the run number changes
  return NOERROR;
}

//------------------
// evnt
//------------------
jerror_t JEventProcessor_FCALLEDTree::evnt(JEventLoop *loop, uint64_t eventnumber)
{
 
  vector< const DFCALHit* > hits;
  loop->Get( hits );

  if( hits.size() > kMaxHits ) return NOERROR;
  
  vector<const DFCALGeometry*> fcalGeomVect;
  loop->Get( fcalGeomVect );
  if (fcalGeomVect.size() < 1)
    return OBJECT_NOT_AVAILABLE;
  const DFCALGeometry& fcalGeom = *(fcalGeomVect[0]);
  

  m_event = eventnumber;
  
  m_nHits = 0;
  m_eTot = 0;
  
    
  japp->RootFillLock(this);

  for( vector< const DFCALHit* >::const_iterator hit = hits.begin();
       hit != hits.end();
       ++hit ){
    
    vector< const DFCALDigiHit* > digiHits;
    (**hit).Get( digiHits );

    if( digiHits.size() != 1 ) std::cout << "ERROR:  wrong size!! " << std::endl;

    const DFCALDigiHit& dHit = *(digiHits[0]);

    m_chan[m_nHits] = fcalGeom.channel( (**hit).row, (**hit).column );
    m_x[m_nHits] = (**hit).x;
    m_y[m_nHits] = (**hit).y;
    m_E[m_nHits] = (**hit).E;
    m_t[m_nHits] = (**hit).t;
    
    m_eTot += (**hit).E;

    m_ped[m_nHits] = (float)dHit.pedestal/dHit.nsamples_pedestal;
    m_peak[m_nHits] = dHit.pulse_peak - m_ped[m_nHits];
    m_integ[m_nHits] = dHit.pulse_integral -
      (m_ped[m_nHits]*dHit.nsamples_integral);
  
    int row = fcalGeom.row((**hit).x);
    int col = fcalGeom.column((**hit).y);

    hXYGeo->Fill( row, col );
    hXYGeo_w->Fill(row, col, (**hit).E );
    hE->Fill(fcalGeom.channel((**hit).row, (**hit).column ), (**hit).E);
    hped->Fill(fcalGeom.channel((**hit).row, (**hit).column ), (float)dHit.pedestal/dHit.nsamples_pedestal);
    hpeak->Fill(fcalGeom.channel((**hit).row, (**hit).column ), dHit.pulse_peak - m_ped[m_nHits]);
    hinteg->Fill(fcalGeom.channel((**hit).row, (**hit).column ), dHit.pulse_integral - (m_ped[m_nHits]*dHit.nsamples_integral));

    ++m_nHits;
  }

  if (btree == 1) {  
    m_tree->Fill();
  }
  
  japp->RootFillUnLock(this);


  return NOERROR;
}

//------------------
// erun
//------------------
jerror_t JEventProcessor_FCALLEDTree::erun(void)
{
  // This is called whenever the run number changes, before it is
  // changed to give you a chance to clean up before processing
  // events from the next run number.
  return NOERROR;
}

//------------------
// fini
//------------------
jerror_t JEventProcessor_FCALLEDTree::fini(void)
{
  // Called before program exit after event processing is finished.
  if (btree == 1) {  
	  japp->RootWriteLock();
	  m_tree->Write();
	  japp->RootUnLock();
  }
  return NOERROR;
}

