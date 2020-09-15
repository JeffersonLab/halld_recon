// Author: David Lawrence  Aug. 30, 2020
//
//
// hd_rsai_gen.cc
//

#include <dlfcn.h>
#include <thread>

#include <TROOT.h>
#include <TCanvas.h>
#include <TDirectory.h>
#include <TObjArray.h>
#include <TH1F.h>
#include <TList.h>
#include <TStyle.h>
#include <TInterpreter.h>
#include <TLatex.h>
#include <TPad.h>
#include <TASImage.h>

#include <DANA/DApplication.h>

#include "rs_macroutils.h"

string OUTPUT_DIR = "images";
static bool DONE = false;
map<string, map<int, int> > CHUNK_COUNTER; // key=base_name(i.e. of macro/file) val=map of pads with key=pad number and val=counter
static double POLL_DELAY = 10;   // time between polling runs
bool MACRO_THREAD_RUNNING = false;

// The following is passed to the macros as a means to set the scale for how often
// images should be produced. Most occupancy plots will just use this directly as
// the number of events processed while making the plot.
static int MIN_EVENTS_RSAI = 1.0E5;


void ParseCommandLineArguments(int &narg, char *argv[]);
void Usage(void);


void MacroThread(JApplication *app);
void ExecuteMacro(TDirectory *f, string macro);


//-----------
// main
//-----------
int main(int narg, char *argv[])
{
	// Parse the command line
	ParseCommandLineArguments(narg, argv);

	// Instantiate an event loop object
	DApplication app(narg, argv);

	gPARMS->SetDefaultParameter( "RSAI:OUTPUT_DIR", OUTPUT_DIR, "Directory to write image files to.");
	gPARMS->SetDefaultParameter( "RSAI:POLL_DELAY", POLL_DELAY, "Minimum time to wait between executing macros in seconds.");
	gPARMS->SetDefaultParameter( "RSAI:MIN_EVENTS_RSAI", MIN_EVENTS_RSAI, "Baseline min. number of events before macro emits image. (Individual macros may scale or ignore this).");

	// Launch a separate thread for periodically running the macros.
	std::thread macrothr(MacroThread, &app);

	while(! MACRO_THREAD_RUNNING ) std::this_thread::sleep_for(std::chrono::milliseconds(100));
	
	// Run though all events, calling our event processor's methods
	app.monitor_heartbeat = 0;
	app.Run();

	DONE = true;
	macrothr.join();

	if( app.GetExitCode() ) cerr << "Exit code: " << app.GetExitCode() << endl;
	return app.GetExitCode();
}


//-----------
// ParseCommandLineArguments
//-----------
void ParseCommandLineArguments(int &narg, char *argv[])
{
	if(narg==1)Usage();
	
	for(int i=1;i<narg;i++){
		if(argv[i][0] != '-')continue;
		switch(argv[i][1]){
			case 'h':
				Usage();
				break;
		}
	}
}

//-----------
// Usage
//-----------
void Usage(void)
{
	// Make sure a JApplication object exists so we can call Usage()
	JApplication *app = japp;
	if(app == NULL) app = new DApplication(0, NULL);

	cout<<"Usage:"<<endl;
	cout<<"       hd_rsai_gen [options] source1 source2 ..."<<endl;
	cout<<endl;
	cout<<"Generate image files using the online monitoring macros which are"<<endl;
	cout<<"suitable for testing/training Hydra AI models"<<endl;
	cout<<endl;
	cout<<"Options:"<<endl;
	cout<<endl;
	app->Usage();
	cout<<endl;
	cout<<"   -h        Print this message"<<endl;
	cout<<endl;

	exit(0);
}

//-----------
// MacroThread
//-----------
void MacroThread(JApplication *app)
{
	std::cout << "Macro thread started!" << std::endl;

	japp->RootWriteLock();
	// Create TCanvas, but set ROOT to batch mode so that it doesn't actually open a window.
	gROOT->SetBatch(kTRUE);
	auto c1 = new TCanvas("c1", "A Canvas", 1600, 1200);

	// Setup interpretor so macros don't have to include these things.
	gROOT->ProcessLine("#include <iostream>");
	gROOT->ProcessLine("using namespace std;");
	gROOT->ProcessLine("#define ROOTSPY_MACROS");
	gROOT->ProcessLine("extern void rs_SetFlag(const string flag, int val);");
	gROOT->ProcessLine("extern int  rs_GetFlag(const string flag);");
	gROOT->ProcessLine("extern void rs_ResetHisto(const string hnamepath);");
	gROOT->ProcessLine("extern void rs_RestoreHisto(const string hnamepath);");
	gROOT->ProcessLine("extern void rs_ResetAllMacroHistos(const string hnamepath);");
	gROOT->ProcessLine("extern void rs_RestoreAllMacroHistos(const string hnamepath);");
	gROOT->ProcessLine("extern void rs_SavePad(const string fname, int ipad);");
	gROOT->ProcessLine("void InsertSeriesData(string sdata){}"); // disable insertion of time series data for RSAI
	gROOT->ProcessLine("void InsertSeriesMassFit(string ptype, double mass, double width, double mass_err, double width_err, double unix_time=0.0){}"); // (same as previous)
	japp->RootUnLock();

	// Set flag so macros will NOT automatically reset histograms after
	// a successful fit. This flag is used by RSTimeSeries
	rs_SetFlag("RESET_AFTER_FIT", 0);

	// Set flag so macros will automatically reset histograms after
	// a successful fit. This flag is used by RSAI
	rs_SetFlag("Is_RSAI", 1);
	rs_SetFlag("RESET_AFTER_SAVEPAD", 1);
	rs_SetFlag("MIN_EVENTS_RSAI", MIN_EVENTS_RSAI);

	// The following ensures that the routines in rs_macroutils are
	// linked in to the final executable so they are available to
	// the macros.
	if(rs_GetFlag("RESET_AFTER_FIT") != 0){
		// (should never actually get here)
		rs_ResetHisto("N/A");
		rs_RestoreHisto("N/A");
		rs_SavePad("N/A", 0);
	}

	// Make output directory
	string cmd("mkdir -p " + OUTPUT_DIR);
	std::cout << "Running external shell command: " << cmd << std::endl;
	system(cmd.c_str());

	auto base_root_dir = gDirectory;

	// Loop until we are told to stop for some reason
	while(!DONE) {

		MACRO_THREAD_RUNNING = true;

		// Run all macros
		std::set<string> fnames_written;
		for(auto p : MACROS){

			auto &macroFilename = p.first;
			auto &macroString = p.second;

			// Only execute macros that contain the Is_RSAI string since only those
			// should have code supporting RSAI image creation.
			if( macroString.find("Is_RSAI") == string::npos ){
				std::cout << "Skipping " << macroFilename << std::endl;
				continue;
			}

			std::cout << "Executing: " << macroFilename << std::endl;

			// We don't keep a list of macros in the same way as RootSpy
			// here so instead of relying on the

			c1->cd();
			c1->Clear();
			ExecuteMacro(base_root_dir, macroString);

			// Due to some long standing bug in TPad, the SaveAs method does not work
			// properly and we are advised to use only TCanvas::SaveAs. See:
			// https://sft.its.cern.ch/jira/browse/ROOT-7633
			//
			// Thus, if we are to save any pads, then we must save the entire canvas
			// first and then crop the pads from that. Technically, there should be a way
			// to do this in memory by getting the TASImage of the canvas directly, but
			// that looks to be rather complicated.
			if( ! rs_PadsToSave.empty() ){

				// Save whole canvas to temporary file.
				string tmp_img_fname = OUTPUT_DIR + "/tmp_canvas.png";
				c1->Update();
				c1->SaveAs(tmp_img_fname.c_str());

				// Loop over base filenames. Generally, there will only be one of these.
				for( auto p : rs_PadsToSave ) {
					auto &basename = p.first;

					// Loops over pads to save using this base filename
					for (int ipad : p.second) {

						// If the pad has a non-default name, use that. Otherwise, use a zero-padded pad number
						auto *pad = c1->GetPad(ipad);
						stringstream default_pad_name;
						default_pad_name << c1->GetName();
						if( ipad != 0 ) default_pad_name << "_" << ipad;
						string pad_name = pad->GetName();

						// Standard filename format includes pad name or number and time "chunk"
						char fname_base[256];
						if( ipad == 0 ){
							// Whole canvas excludes pad from fname
							sprintf(fname_base, "%s_%04d.png", basename.c_str(), ++CHUNK_COUNTER[basename][ipad]);
						}else if( pad_name == default_pad_name.str() ){
							// Use pad number in fname
							sprintf(fname_base, "%s-%02d_%04d.png", basename.c_str(), ipad, ++CHUNK_COUNTER[basename][ipad]);
						}else{
							// Use pad name in fname
							sprintf(fname_base, "%s-%s_%04d.png", basename.c_str(), pad_name.c_str(), ++CHUNK_COUNTER[basename][ipad]);
						}
						char fname[512];
						sprintf(fname, "%s/%s",  OUTPUT_DIR.c_str(), fname_base);

						// Get pad of interest
						if (pad) {

							// Determine crop parameters for this pad
							int x1 = pad->XtoAbsPixel(pad->GetX1());
							int x2 = pad->XtoAbsPixel(pad->GetX2());
							int y1 = pad->YtoAbsPixel(pad->GetY1());
							int y2 = pad->YtoAbsPixel(pad->GetY2());

							int w = x2 - x1;
							int h = y1 - y2; // yep, seems backwards doesn't it?

							// Read in image as TASImage object and crop it
							auto img = TASImage::Open( tmp_img_fname.c_str() );
							img->Crop(x1, y2, w, h);

							// Write cropped image to file
							cout << "    - Writing file " << fname << endl;
							img->WriteImage(fname, TImage::kPng);
							delete img;
							fnames_written.insert(fname);

						}else{
							_DBG_ << "Unable to get pad " << ipad << " for " << macroFilename << " (for writing to " << fname << ")" << endl;
						}
					}
				}

				// Delete temporary image file
				unlink( tmp_img_fname.c_str() );
			}

			// Clear global
			rs_PadsToSave.clear();

		}

		// Lock ROOT
		japp->RootWriteLock();

		//std::cout << "--- MACROS list ----------------------------" << std::endl;
		//for(auto p : MACROS) std::cout << " -- " << p.first << std::endl;
		//std::cout << "--------------------------------------------" << std::endl;

		// Reset any macro histograms specified by macros
		for( auto m : macros_to_reset ){
			auto key = m.substr(2); // chop off leading "//"
			if( MACROS.count(key)){
				// Found macro. Get list of histograms from macro contents
				std::stringstream ss(MACROS[key]);
				std::string to;
				std::set<std::string> hnames;
				while(std::getline(ss,to,'\n')){
					if(to.find("// hnamepath: ") == 0){
						hnames.insert(to.substr(strlen("// hnamepath: ")));
					}
				}
				std::cout << "Will reset " << hnames.size() << " histograms for macro: " << m << std::endl;

				// Add these to full list of histograms to reset below
				hnamepaths_to_reset.insert( hnames.begin(), hnames.end() );

			}else{
				std::cout << "Unable to find macro for \"" << key << "\"" << std::endl;
			}
		}

		// Reset any histograms specified by macros
		for( auto h : hnamepaths_to_reset ){
			std::cout << "Resetting histogram: " << h << std::endl;
			TH1 *hist = nullptr;
			gROOT->GetObject(h.c_str(), hist);
			if(hist){
				hist->Reset();
			}else{
				std::cout << "--- Unable to find histogram: " << h << std::endl;
			}
		}

		macros_to_reset.clear();
		hnamepaths_to_reset.clear();

		// Unlock ROOT
		japp->RootUnLock();

		// sleep for awhile
		sleep(POLL_DELAY);
	}


	std::cout << "Macro thread ended!" << std::endl;

}

//-------------------
// ExecuteMacro
//-------------------
void ExecuteMacro(TDirectory *f, string macro)
{
	// Lock ROOT
	japp->RootWriteLock();

	TDirectory *savedir = gDirectory;
	f->cd();

	// Keep a separate TSyle for each macro we draw. This used to
	// allow macros to change the style and have it stay changed
	// until the next macro is drawn.
	static std::map<string, TStyle*> styles;
	if( styles.count( macro ) == 0 ){
		styles[macro] = new TStyle();
	}
	*gStyle = *styles[macro];

	// execute script line-by-line
	// maybe we should do some sort of sanity check first?
	istringstream macro_stream(macro);
	int iline = 0;
	while(!macro_stream.eof()) {
		string s;
		getline(macro_stream, s);
		iline++;

		Long_t err = gROOT->ProcessLine(s.c_str());
		if(err == TInterpreter::kProcessing){
			cout << "Processing macro ..." << endl;
		}else  if(err != TInterpreter::kNoError){
			cout << "Error processing the macro line " << iline << ": " << err << endl;
			cout << "\"" << s << "\"" << endl;
			break;
		}
	}

	// restore
	savedir->cd();

	// Unlock ROOT
	japp->RootUnLock();

}

