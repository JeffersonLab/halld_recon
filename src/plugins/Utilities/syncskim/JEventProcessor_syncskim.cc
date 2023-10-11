// $Id$
//
//    File: JEventProcessor_syncskim.cc
// Created: Wed Feb 22 20:04:25 EST 2017
// Creator: davidl (on Linux gluon48.jlab.org 2.6.32-431.20.3.el6.x86_64 x86_64)
//

#include <limits>
using namespace std;

#include <TH1.h>

#include "JEventProcessor_syncskim.h"

#include <DAQ/DL1Info.h>
#include <DAQ/DCODAEventInfo.h>
#include <DAQ/DCODAROCInfo.h>
#include <DAQ/DCODAControlEvent.h>


// Routine used to create our JEventProcessor
#include <JANA/JApplication.h>
#include <JANA/JFactoryT.h>
extern "C"{
void InitPlugin(JApplication *app){
	InitJANAPlugin(app);
	app->Add(new JEventProcessor_syncskim());
}
} // "C"


//------------------
// JEventProcessor_syncskim (Constructor)
//------------------
JEventProcessor_syncskim::JEventProcessor_syncskim()
{
	SetTypeName("JEventProcessor_syncskim");

	sum_n = 0.0;
	sum_x = 0.0;
	sum_y = 0.0;
	sum_xy = 0.0;
	sum_x2 = 0.0;
	
	last_control_event_t = 0.0;
	last_physics_event_t = 0.0;
}

//------------------
// ~JEventProcessor_syncskim (Destructor)
//------------------
JEventProcessor_syncskim::~JEventProcessor_syncskim()
{

}

//------------------
// Init
//------------------
void JEventProcessor_syncskim::Init()
{
	auto app = GetApplication();
	auto params = app->GetJParameterManager();
	lockService = app->GetService<JLockService>();

	bool SYNCSKIM_FAST = false;
	app->SetDefaultParameter("SYNCSKIM:FAST", SYNCSKIM_FAST, "Set to non-zero to automatically turn off all parsing and emulation not needed by this plugin so it runs blindingly fast");

	SYNCSKIM_ROCID = 34;
	app->SetDefaultParameter("SYNCSKIM:ROCID", SYNCSKIM_ROCID, "ROC id from which to use timestamp. Set to 0 to use average timestamp from CODA EB. Default is 34 (rocBCAL4)");

	// The default behavior here has changed.
	// It used to be that these settings were always made and there
	// was no option to turn them off. In order to make this 
	// friendlier with the monitoring runs which this will soon
	// be included with, the option was added and the adjustment
	// of these setting disabled by default.
	// 4/17/2020 D.L.
	if(SYNCSKIM_FAST){
		params->SetParameter("EVIO:LINK",              false); 
		params->SetParameter("EVIO:LINK_BORCONFIG",    false); 
		params->SetParameter("EVIO:PARSE_F250",        false); 
		params->SetParameter("EVIO:PARSE_F125",        false); 
		params->SetParameter("EVIO:PARSE_F1TDC",       false); 
		params->SetParameter("EVIO:PARSE_CAEN1290TDC", false); 
		params->SetParameter("EVIO:PARSE_CONFIG",      false); 
		params->SetParameter("EVIO:PARSE_BOR",         false); 
		params->SetParameter("EVIO:PARSE_EPICS",       false); 
		params->SetParameter("EVIO:PARSE_EVENTTAG",    false); 
		//params->SetParameter("EVIO:PARSE_TRIGGER",     false);
		params->SetParameter("EVIO:APPLY_TRANSLATION_TABLE", false);
		params->SetParameter("EVIO:F250_EMULATION_MODE", 0);
		params->SetParameter("EVIO:F125_EMULATION_MODE", 0);
	}
	
	lockService->RootWriteLock();

    TDirectory *main = gDirectory;

	file = new TFile("syncskim.root", "RECREATE");
	
	tree = new TTree("synctree", "Sync Events Tree");
	tree->Branch("run_number",    &synevt.run_number,    "run_number/i"    );
	tree->Branch("run_type",      &synevt.run_type,      "run_type/i"      );
	tree->Branch("event_number",  &synevt.event_number,  "event_number/l"  );
	tree->Branch("event_type",    &synevt.event_type,    "event_type/s"    );
	tree->Branch("avg_timestamp", &synevt.avg_timestamp, "avg_timestamp/l" );

	tree->Branch("nsync",         &synevt.nsync,         "nsync/i"         );
	tree->Branch("trig_number",   &synevt.trig_number,   "trig_number/i"   );
	tree->Branch("live_time",     &synevt.live_time,     "live_time/i"     );
	tree->Branch("busy_time",     &synevt.nsync,         "busy_time/i"     );
	tree->Branch("live_inst",     &synevt.live_inst,     "live_inst/i"     );
	tree->Branch("unix_time",     &synevt.unix_time,     "unix_time/i"     );
 
	tree->Branch("gtp_sc",        &synevt.gtp_sc,        "gtp_sc[32]/i"    );
	tree->Branch("fp_sc",         &synevt.fp_sc,         "fp_sc[16]/i"     );
	tree->Branch("gtp_rate",      &synevt.gtp_rate,      "gtp_rate[32]/i"  );
	tree->Branch("fp_rate",       &synevt.fp_rate,       "fp_rate[16]/i"   );
	tree->Print();

	conversion_tree = new TTree("conversion_tree", "Parameters for CCDB to convert timestamps to unix time");
	conversion_tree->Branch("run_number",         &convparms.run_number,         "run_number/i"         );
	conversion_tree->Branch("first_event_number", &convparms.first_event_number, "first_event_number/l" );
	conversion_tree->Branch("last_event_number",  &convparms.last_event_number,  "last_event_number/l"  );
	conversion_tree->Branch("tics_per_sec",       &convparms.tics_per_sec,       "tics_per_sec/l"       );
	conversion_tree->Branch("unix_start_time",    &convparms.unix_start_time,    "unix_start_time/l"    );
	
	convparms.run_number         = 0;
	convparms.first_event_number = 0;
	convparms.last_event_number  = 0;
	convparms.tics_per_sec       = 0;
	convparms.unix_start_time    = 0;
	
	main->cd();
	
	lockService->RootUnLock();
}

//------------------
// BeginRun
//------------------
void JEventProcessor_syncskim::BeginRun(const std::shared_ptr<const JEvent>& event)
{
}

//------------------
// Process
//------------------
void JEventProcessor_syncskim::Process(const std::shared_ptr<const JEvent>& event)
{
	vector<const DCODAControlEvent*> controlevents;
	event->Get(controlevents);
	if(!controlevents.empty()){
		last_control_event_t = (double) controlevents[0]->unix_time;
		last_physics_event_t = 0.0;
	}

	vector<const DCODAEventInfo*> codainfos;
	event->Get(codainfos);
	if(codainfos.empty()) return;
	const DCODAEventInfo *codainfo = codainfos[0];

	uint64_t mytimestamp = 0.0;
	if( SYNCSKIM_ROCID == 0 ){
		mytimestamp = codainfo->avg_timestamp;
	}else{
		vector<const DCODAROCInfo*> codarocinfos;
		event->Get(codarocinfos);
		if(codarocinfos.empty()) return;
		for( auto codarocinfo : codarocinfos ){
			if( codarocinfo->rocid == SYNCSKIM_ROCID ){
				mytimestamp = codarocinfo->timestamp;
				break;
			}
		}
	}
	if( mytimestamp == 0.0 ) return;
	
	if( (last_control_event_t!=0.0) && (last_physics_event_t==0.0) ){
		last_physics_event_t = (double)mytimestamp / 250.0E6;
	}

	vector<const DL1Info*> l1infos;
	event->Get(l1infos);
	if(l1infos.empty()) return;

	lockService->RootWriteLock();
	
	for(auto l1info : l1infos){
		
		time_t t = l1info->unix_time;
		cout << "sync event at " << ctime(&t);
		
		synevt.run_number    = codainfo->run_number;
		synevt.run_type      = codainfo->run_type;
		synevt.event_number  = codainfo->event_number;
		synevt.event_type    = codainfo->event_type;
		synevt.avg_timestamp = mytimestamp;
	
		synevt.nsync       = l1info->nsync;
		synevt.trig_number = l1info->trig_number;
		synevt.live_time   = l1info->live_time;
		synevt.busy_time   = l1info->busy_time;
		synevt.live_inst   = l1info->live_inst;
		synevt.unix_time   = l1info->unix_time;
		for(int i=0; i<32; i++){
			synevt.gtp_sc[i]   = l1info->gtp_sc[i];
			synevt.gtp_rate[i] = l1info->gtp_rate[i];
			if(i<16){
				synevt.fp_sc[i]   = l1info->fp_sc[i];
				synevt.fp_rate[i] = l1info->fp_rate[i];
			}
		}		
		
		tree->Fill();

		convparms.run_number = synevt.run_number;
		if( convparms.first_event_number == 0 ) convparms.first_event_number = codainfo->event_number;
		convparms.last_event_number = codainfo->event_number;
		
		// scale and shift x/y values to make sure range of sum doesn't cut them off
		double x = (double)synevt.avg_timestamp / 250.0E6;
		double y = (double)synevt.unix_time - 13.0E8;
		sum_n  += 1.0;
		sum_x  += x;
		sum_y  += y;
		sum_xy += x * y;
		sum_x2 += x * x;
	}
	
	lockService->RootUnLock();
}

//------------------
// EndRun
//------------------
void JEventProcessor_syncskim::EndRun()
{
}

//------------------
// Finish
//------------------
void JEventProcessor_syncskim::Finish()
{

	double m = (sum_n*sum_xy - sum_x*sum_y)/(sum_n*sum_x2 - sum_x*sum_x);
	double b = (sum_y*sum_x2 - sum_x*sum_xy)/(sum_n*sum_x2 - sum_x*sum_x);
	
	// scale shift back
	b += 13.0E8;
	double one_over_m = 250.0E6/m;
	
	// Fallback if no sync events
	if(sum_n==0.0){
		cout << endl << "No sync events found! Conversion values will be taken from control+physics events." << endl;
		
		one_over_m = 250.0E6;
		b = last_control_event_t - last_physics_event_t;
	}
	
	typedef std::numeric_limits< double > dbl;
	cout.precision(dbl::max_digits10);
	
	cout << endl << "timestamp to unix time conversion: tics_per_sec=" << one_over_m << " unix_start_time=" << b << endl << endl;

	// Write results to ROOT file, flush it and close it
	if( file ){
		lockService->RootWriteLock();
		
		convparms.tics_per_sec    = one_over_m;
		convparms.unix_start_time = b;
		conversion_tree->Fill();

		file->Write();
		file->Close();
		delete file;
		
		lockService->RootUnLock();
	}
}

