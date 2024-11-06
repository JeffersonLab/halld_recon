// $Id$
//
//    File: JEventProcessor_FCAL_online.cc
// Created: Fri Nov  9 11:58:09 EST 2012
// Creator: wolin (on Linux stan.jlab.org 2.6.32-279.11.1.el6.x86_64 x86_64)


#include <stdint.h>
#include <vector>
#include <limits>

#include "JEventProcessor_TOF_online.h"

using namespace std;

#include <DANA/DEvent.h>

#include <DAQ/DCODAEventInfo.h>
#include <DAQ/DCODAROCInfo.h>
#include <DAQ/DCAEN1290TDCHit.h>

#include "TOF/DTOFHit.h"
#include "TOF/DTOFDigiHit.h"
#include "TOF/DTOFTDCDigiHit.h"
#include "TOF/DTOFGeometry.h"

#include <TDirectory.h>
#include <TH2.h>
#include <TH1.h>
#include <TCanvas.h>
#include <TPolyLine.h>


// root hist pointers
static TH1I *tofe;
static TH1I *toft;
static TH2I *tofo1;
static TH2I *tofo2;

static TH1I *tdcOccS;
static TH1I *tdcOccN;
static TH1I *tdcOccU;
static TH1I *tdcOccD;

static TH1I *adcOccS;
static TH1I *adcOccN;
static TH1I *adcOccU;
static TH1I *adcOccD;

static TH2I *planeHor;
static TH2I *planeVer;

static TH1I *tof_num_events;

static TH1I *histPed;
static TH1I *hTimeAdc;
static TH1I *hTimeTdc;

static TH2F *TOFPedestalsPlane0;
static TH2F *TOFPedestalsPlane1;
static TH2F *TOFSignalsRawPlane0;
static TH2F *TOFSignalsRawPlane1;
static TH2F *TOFTimesPlane0;
static TH2F *TOFTimesPlane1;

static TH2F *TOFWalkExample;


//----------------------------------------------------------------------------------


// Routine used to create our JEventProcessor
extern "C"{
  void InitPlugin(JApplication *app){
    InitJANAPlugin(app);
    app->Add(new JEventProcessor_TOF_online());
  }
}


//----------------------------------------------------------------------------------


JEventProcessor_TOF_online::JEventProcessor_TOF_online() {
	SetTypeName("JEventProcessor_TOF_online");
}


//----------------------------------------------------------------------------------


JEventProcessor_TOF_online::~JEventProcessor_TOF_online() {
}


//----------------------------------------------------------------------------------

void JEventProcessor_TOF_online::Init() {

  // create root folder for tof and cd to it, store main dir
  TDirectory *main = gDirectory;
  gDirectory->mkdir("tof")->cd();


  // book hist
  tof_num_events = new TH1I("tof_num_events","TOF Number of events",1, 0.5, 1.5);

  tofe    = new TH1I("tofe","TOF energy in keV",100,0,15000);
  toft    = new TH1I("toft","TOF time in ns",200,0,200);
  tofo1   = new TH2I("tofo1","TOF occupancy plane 1 by bar,top/bottom",50,0,50,2,0,2);
  tofo2   = new TH2I("tofo2","TOF occupancy plane 2 by left/right,bar",2,0,2,50,0,50);

  tdcOccS = new TH1I("tdcOccS","TOF, TDC Occupancy",90,1,46);
  tdcOccN = new TH1I("tdcOccN","TOF, TDC Occupancy",90,1,46);
  tdcOccU = new TH1I("tdcOccU","TOF, TDC Occupancy",90,1,46);
  tdcOccD = new TH1I("tdcOccD","TOF, TDC Occupancy",90,1,46);

  adcOccS = new TH1I("adcOccS","TOF, fADC Occupancy",90,1,46);
  adcOccN = new TH1I("adcOccN","TOF, fADC Occupancy",90,1,46);
  adcOccU = new TH1I("adcOccU","TOF, fADC Occupancy",90,1,46);
  adcOccD = new TH1I("adcOccD","TOF, fADC Occupancy",90,1,46);

  histPed = new TH1I("histPed","TOF, Pedestals",100, 0, 500);

  hTimeAdc = new TH1I("hTimeAdc","TOF, fADC time",100,0,400);
  hTimeTdc = new TH1I("hTimeTdc","TOF, TDC time",100,0,750);

  planeHor = new TH2I("planeHor","TOF Upstream, Hit position, Horizontal Plane",84,-126,126,84,-126,126);
  planeVer = new TH2I("planeVer","TOF Upstream, Hit position, Vertical Plane",84,-126,126,84,-126,126);


  TOFPedestalsPlane0 = new TH2F("TOFPedestalsPlane0","TOF Pedestals Plane 0 all PMTs",
				100,0.,500., 92, 0., 92);
  TOFPedestalsPlane1 = new TH2F("TOFPedestalsPlane1","TOF Pedestals Plane 1 all PMTs",
				100,0.,500., 92, 0., 92);
  TOFSignalsRawPlane0 = new TH2F("TOFSignalsRawPlane0","TOF ADC Integral Plane 0 all PMTs",
				 300,0.,10000., 92, 0., 92);
  TOFSignalsRawPlane1 = new TH2F("TOFSignalsRawPlane1","TOF ADC Integral Plane 1 all PMTs",
				 300,0.,10000., 92, 0., 92);

  TOFTimesPlane0 = new TH2F("TOFTimesPlane0","TOF TDC times Plane 0 all PMTs",
			    800,0.,1000., 92, 0., 92);
  TOFTimesPlane1 = new TH2F("TOFTimesPlane1","TOF TDC times Plane 1 all PMTs",
			    800,0.,1000., 92, 0., 92);

  TOFWalkExample = new TH2F("TOFWalkEXample", "TOF T-vs-E walk correction example", 200, 10., 24000., 500, 200.,350.);

  // back to main dir
  main->cd();
}


//----------------------------------------------------------------------------------


void JEventProcessor_TOF_online::BeginRun(const std::shared_ptr<const JEvent>& event) {
  // This is called whenever the run number changes

  map<string,double> tdcshift;
  const DTOFGeometry *locTOFGeometry = nullptr;
  event->GetSingle(locTOFGeometry);
  string locTOFTDCShiftTable = locTOFGeometry->Get_CCDB_DirectoryName() + "/tdc_shift";
  if(!GetCalib(event, locTOFTDCShiftTable.c_str(), tdcshift)) {
    TOF_TDC_SHIFT = tdcshift["TOF_TDC_SHIFT"];
  }
}


//----------------------------------------------------------------------------------


void JEventProcessor_TOF_online::Process(const std::shared_ptr<const JEvent>& event) {
  // This is called for every event. Use of common resources like writing
  // to a file or filling a histogram should be mutex protected. Using
  // loop-Get(...) to get reconstructed objects (and thereby activating the
  // reconstruction algorithm) should be done outside of any mutex lock
  // since multiple threads may call this method at the same time.

  const DTOFGeometry* locTOFGeometry;
  event->GetSingle(locTOFGeometry);

  uint32_t E,t,pedestal;
  int plane,bar,end;
  int count_tdc = 0;
  int count_adc = 0;
  double tdc_time,pulse_time;

  int ExamplePlane = 0;
  int ExampleBar = 10;
  int ExampleEnd = 0;
  int RefFound = 0;
  float tADC = 0.;
  float tTDC = 0.;
  float EADC = 0.;

  double hit_north[locTOFGeometry->Get_NBars()+1];
  double hit_south[locTOFGeometry->Get_NBars()+1];
  double hit_up[locTOFGeometry->Get_NBars()+1];
  double hit_down[locTOFGeometry->Get_NBars()+1];
  // comment out "width" to suppress warning
  //  double position, time, width;
  double position, time;
  float integral;

  Float_t distY_Horz = -126; // Horizontal plane start counting from the Bottom to Top
  Float_t distX_Vert = -126; // Vertical plane start counting from the North to South

  memset(hit_north,0,sizeof(hit_north));
  memset(hit_south,0,sizeof(hit_south));
  memset(hit_up,0,sizeof(hit_up));
  memset(hit_down,0,sizeof(hit_down));

  // First determine timing shift to resolve 6-fold ambiguity
  /*
  vector<const DCODAEventInfo*> locCODAEventInfo;
  event->Get(locCODAEventInfo);
  
  if (locCODAEventInfo.size() == 0){
    return;
  }
  */
  

  vector< const DCAEN1290TDCHit*> CAENHits;
  event->Get(CAENHits);
  if (CAENHits.size()<=0){
    return;
  }
  uint32_t locROCID = CAENHits[0]->rocid; // this is the crate we want the trigger time from
  int indx = -1;
  vector <const DCODAROCInfo*> ROCS;
  event->Get(ROCS);
  for ( unsigned int n=0; n<ROCS.size(); n++) {
    if (locROCID == ROCS[n]->rocid){
      indx = n;
      break;
    }
  }
  if (indx<0){
    return;
  }

  uint64_t TriggerTime = ROCS[indx]->timestamp;
  int TriggerBIT = TriggerTime%6;
  float TimingShift = TOF_TDC_SHIFT - (float)TriggerBIT;
  //cout<<TOF_TDC_SHIFT<<endl;
  //float TimingShift = -(float)TriggerBIT;
  if(TimingShift <= 0) { 
    TimingShift += 6.;
  } 
  TimingShift *= 4. ;

  // get data for tof
  vector<const DTOFHit*> dtofhits;
  event->Get(dtofhits);

	// FILL HISTOGRAMS
	// Since we are filling histograms local to this plugin, it will not interfere with other ROOT operations: can use plugin-wide ROOT fill lock
	lockService->RootWriteLock(); //ACQUIRE ROOT FILL LOCK

  if(dtofhits.size() > 0)
	  tof_num_events->Fill(1);

  for(unsigned int i=0; i<dtofhits.size(); i++) {
    const DTOFHit *dtofhit = dtofhits[i];

    plane = dtofhit->plane;
    bar   = dtofhit->bar;
    end   = dtofhit->end;
    E     = dtofhit->dE*1000000.;  // in MeV
    t     = dtofhit->t;         // in nanoseconds
    time     = dtofhit->t;         // in nanoseconds

    const DTOFDigiHit *digi = NULL;
    const DTOFTDCDigiHit *tdig = NULL;

    dtofhit->GetSingle(tdig);
    tdc_time = std::numeric_limits<double>::quiet_NaN(); //updated below if good
    pulse_time = std::numeric_limits<double>::quiet_NaN(); //updated below if good
    if(tdig){
      const uint32_t &Rtdc_time = tdig->time;
      if(Rtdc_time!=0xFFFF){ 
	tdc_time = 0.025*(double)Rtdc_time;
	if (plane){
	  TOFTimesPlane1->Fill((float)tdc_time, (float)bar-1+end*44);
	} else {
	  TOFTimesPlane0->Fill((float)tdc_time, (float)bar-1+end*44);
	}
      }

      if ( (plane == ExamplePlane) &&
	   (bar == ExampleBar) &&
	   (end == ExampleEnd)){
	tTDC = (float)tdig->time * 0.0234375;
	RefFound++;
      }

    }

    dtofhit->GetSingle(digi);
    pedestal = 0xFFFF;
    if (digi){
      const uint32_t &Rpedestal = digi->pedestal;
      const uint32_t &Rpulse_time = digi->pulse_time;
      if(Rpedestal!=0xFFFF && Rpulse_time!=0xFFFF){
        pedestal = Rpedestal;
	pulse_time = 0.0625*(double)Rpulse_time;
	integral = (float)digi->pulse_integral - (float)digi->pedestal*
	  (float)digi->nsamples_integral/(float)digi->nsamples_pedestal;
	integral /= (float)digi->nsamples_integral;
	integral *=10.;
	if (plane){
	  TOFPedestalsPlane1->Fill((float)pedestal,(float)bar-1+end*44);
	  TOFSignalsRawPlane1->Fill(integral, (float)bar-1+end*44);
	} else {
	  TOFPedestalsPlane0->Fill((float)pedestal,(float)bar-1+end*44);
	  TOFSignalsRawPlane0->Fill(integral, (float)bar-1+end*44);
	}
      }

      if ( (plane == ExamplePlane) &&
	   (bar == ExampleBar) &&
	   (end == ExampleEnd)){
	tADC = (float)digi->pulse_time * 0.0625;
	EADC = (float)digi->pulse_integral - (float)digi->pedestal*
	  (float)digi->nsamples_integral/(float)digi->nsamples_pedestal;
	RefFound++;
      }
    }
    

    if(dtofhits[i]->dE>0.0) {
      
      
      //      fill hist
      
      tofe->Fill(E);
      toft->Fill(t);
      if(plane==0) {  // vertical, North is 0
        tofo1->Fill(bar,end);
      } else {
        tofo2->Fill(end,bar);
      }
      
    } // close if E>0

    if (dtofhit->has_fADC) hTimeAdc->Fill(pulse_time);
    if (dtofhit->has_TDC) hTimeTdc->Fill(tdc_time);

    switch(plane)
      {
      case 0:
	if(end == 1){
	  //cout << "Down : " << bar << endl;
	  if (dtofhit->has_fADC){
	    if (pedestal!=0xFFFF) histPed->Fill(pedestal);
	    adcOccD->Fill(bar);
	    count_adc++;
	  }
	  if (dtofhit->has_TDC){
	    tdcOccD->Fill(bar);
	    count_tdc++;
	    if ( ( (hit_down[bar]<=0) || (t < hit_down[bar]) ) && (!locTOFGeometry->Is_ShortBar(bar)) ){
	      hit_down[bar] = time;
	    }
	  }
	}
	else if(end == 0){
	  //cout << "Up : " << bar << endl;
	  if (dtofhit->has_fADC){
	    if (pedestal!=0xFFFF) histPed->Fill(pedestal);
	    adcOccU->Fill(bar);
	    count_adc++;
	  }
	  if (dtofhit->has_TDC){
	    tdcOccU->Fill(bar);
	    count_tdc++;
	    if ( ( (hit_up[bar]<=0) || (t < hit_up[bar]) ) && (!locTOFGeometry->Is_ShortBar(bar)) ){
	      hit_up[bar] = time;
	    }
	  }
	}
	break;
      case 1:
	if(end == 0){
	  //cout << "North : " << bar << endl;
	  if (dtofhit->has_fADC){
	    if (pedestal!=0xFFFF) histPed->Fill(pedestal);
	    adcOccN->Fill(bar);
	    count_adc++;
	  }
	  if (dtofhit->has_TDC){
	    tdcOccN->Fill(bar);
	    count_tdc++;
	    if ( ( (hit_north[bar]<=0) || (t < hit_north[bar]) ) && (!locTOFGeometry->Is_ShortBar(bar)) ){
	      hit_north[bar] = time;
	    }
	  }
	}
	else if(end == 1){
	  //cout << "South : " << bar << endl;
	  if (dtofhit->has_fADC){
	    if (pedestal!=0xFFFF) histPed->Fill(pedestal);
	    adcOccS->Fill(bar);
	    count_adc++;
	  }
	  if (dtofhit->has_TDC){
	    tdcOccS->Fill(bar);
	    count_tdc++;
	    if ( ( (hit_south[bar]<=0) || (t < hit_south[bar]) ) && (!locTOFGeometry->Is_ShortBar(bar)) ){
	      hit_south[bar] = time;
	    }
	  }
	}
	break;
      }
    
  } // close DTOFHit size

  if (RefFound>1){
    TOFWalkExample->Fill(EADC, (tTDC-tADC+TimingShift));
  }


  for (int i=1; i<locTOFGeometry->Get_NBars()+1; i++)
    {
	
	double width = locTOFGeometry->Get_BarWidth(i);
	double bar_half_width = width / 2.;
	distY_Horz += bar_half_width;
	distX_Vert += bar_half_width;

      if( hit_south[i]>0 && hit_north[i]>0 )
	{
	  position = (15.2*(Float_t(hit_south[i] - hit_north[i])/2) );
	  distY_Horz = distY_Horz + (drand48()-0.5)*width;
	  if (position )planeHor->Fill(position,distY_Horz);
	}

      if( hit_up[i]>0 && hit_down[i]>0 )
	{
	  position = (15.2*(Float_t(hit_up[i] - hit_down[i])/2) );
	  distX_Vert = distX_Vert + (drand48()-0.5)*width;
	  planeVer->Fill(distX_Vert,position);
	}

	distY_Horz += bar_half_width;
	distX_Vert += bar_half_width;

    }

	lockService->RootUnLock(); //RELEASE ROOT FILL LOCK

}


//----------------------------------------------------------------------------------


void JEventProcessor_TOF_online::EndRun() {
  // This is called whenever the run number changes, before it is
  // changed to give you a chance to clean up before processing
  // events from the next run number.
}


//----------------------------------------------------------------------------------


void JEventProcessor_TOF_online::Finish() {
  // Called before program exit after event processing is finished.
}


//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
