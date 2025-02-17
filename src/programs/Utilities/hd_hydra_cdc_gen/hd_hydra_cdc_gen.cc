//
// This program will read in CDC occupancy histograms from 
// a list of ROOT files (Produced by RootSpy) and generate
// PNG files using the standard CDC occupancy macro. The
// histograms will be modified to mimic one or more HV boards
// having failed though so as to produce "bad" images that
// can be used in Hydra model training.
//
// Important info:
//
//  1. By default it looks for a file called "root_files.txt"
//     in the current directory for the list of ROOT files
//     to use. One file per line. Empty lines and comment lines
//     starting with "#" are allowed. In-line comments are not.
//
//  2. The default output directory will be called "images"
//     and will be created by running the "mkdir -p" shell
//     command. Thus, it won't error if it already exists.
//     One can also specify this with the "RSAI:OUTPUT_DIR"
//     config. parameter.
//
//  3. This is hard-coded to produce an image for each HV
//     board so that a complete set of single board failure
//     images is produced. To generate 2 board failures this
//     file will need to be edited and recompiled.
//
//  4. A file "origin_log.txt" will be created in the output
//     images directory in order to makea record of where
//     each image file came from since they are generically named.
//
//
// This links in the standard Hall-D DANA libraries so that
// the occupancy_online plugin can be attached and the 
// CDC_occupancy macro accessed that way. Unlike most JANA
// programs though, it does not process events. (Unfortunately
// this adds some ugliness in that we create a dummy event
// source just to avoid JANA errors/warnings).
//
// Being a JANA program, this also has access to config.
// parameters which can be set using the standard command
// line options.
//
//
//


#include <stdio.h>
#include <ctype.h>
#include <algorithm>
#include <vector>
#include <set>

#include <JANA/JEventSourceGenerator.h>
#include <DANA/DApplication.h>
#include <RSAI_KO/CDC_pins.h>
using namespace std;

#include <TH1.h>
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
#include <TMemFile.h>
#include "rs_macroutils.h"


//......................................................................
// These two classes are just to provide a dummy event source so JANA
// does not complain about not having one. (Who designed this thing?!)
class JEventSourceDummy:public JEventSource{
	public:
		JEventSourceDummy(const char* source_name):JEventSource(source_name){}
		virtual const char* className(void){return static_className();}
		static const char* static_className(void){return "JEventSourceDummy";}

		void GetEvent(std::shared_ptr<JEvent> event){ return; }
		bool GetObjects(std::shared_ptr<JEvent>&, JFactory *factory){ return false; }
};
class JEventSourceDummyGenerator:public JEventSourceGenerator{
	public:
		std::string GetType() const override {
                    return "JEventSourceDummyGenerator";
                }
		std::string GetDescription() const override { return "dummy"; }
		double CheckOpenable(string source){ return 1.0; }
		JEventSource* MakeJEventSource(string source){return new JEventSourceDummy(source.c_str());}
		
};
//......................................................................

// Type to hold a set of board efficiencies. This allows us to 
// specify multiple boards with different efficiencies so
// we can easily train with multiple boards out.
typedef std::map<std::string, double> board_eff_t;

// Globals
TCanvas *c1 = nullptr;
string OUTPUT_DIR = "images";
static int MIN_EVENTS_RSAI = 1.0E0; // normally 1.0E5
double MIN_CDC_BIN_CONTENT = 1.0;
map<string, map<int, int> > CHUNK_COUNTER; // key=base_name(i.e. of macro/file) val=map of pads with key=pad number and val=counter
TDirectory *dir_top = nullptr;
TDirectory *dir_occupancy = nullptr;
TDirectory *dir_original = nullptr;
string CURRENT_ROOT_FILE;
set<string> CURRENT_KO_BOARDS;
ofstream *ofs_log = nullptr; // for writing log of where each image file came from


                     int main(int narg, char *argv[]);
                    void Usage(JApplication &app);
std::vector<std::string> GetRootFilenames(JApplication* app);
      std::vector<TH1*>  GetCDCHists(std::string &fname);
      std::vector<TH1*>  MakeCDC_KO_hists(std::vector<TH1*> &cdc_hists, board_eff_t &board_efficiency);
						  void FreeHists( std::vector<TH1*> &hists);
						  void InitilizeRootInterpreter(JApplication* app);
						  void RunCDC_occupancy_macro(JApplication* app);
						  void ExecuteMacro(JApplication* app, TDirectory *f, string macro);

//-----------
// main
//-----------
int main(int narg, char *argv[])
{
	// Instantiate an event loop object
	JApplication app;

	//if(narg<=1)Usage(app);

	// Add a dummy event source to avoid annoying complaints from JANA
	app.Add(new JEventSourceDummy("dummy"));

	// Add occupancy_online plugin so we have access to RootSPy macros
	app.AddPlugin("occupancy_online");

	// Initialize JANA
	app.SetDefaultParameter( "RSAI:OUTPUT_DIR", OUTPUT_DIR, "Directory to write image files to.");
	app.SetDefaultParameter( "RSAI:MIN_EVENTS_RSAI", MIN_EVENTS_RSAI, "Baseline min. number of events before macro emits image. (Individual macros may scale or ignore this).");
	app.SetDefaultParameter( "RSAI:MIN_CDC_BIN_CONTENT", MIN_CDC_BIN_CONTENT, "Minimum value a CDC occupancy histogram bin should have after efficiency is applied. This allows an efficiency of zero without the bin being empty and therefore white.");
        app.Initialize();

	// Read in list of root files to use as histogram source
	auto fnames = GetRootFilenames(&app);

	// Determine the list of board inefficiencies
	vector<board_eff_t> board_efficiencies;
	for( auto m : CDC_pins ){
		board_eff_t eff;
		eff[m.first] = 0.0;
		board_efficiencies.push_back(eff);
	}
	cout << "Defined " << board_efficiencies.size() << " CDC HV board failure modes" << endl;

	// Make ROOT directories based in memory
	dir_top = new TMemFile("junk.root", "RECREATE");
	dir_occupancy = dir_top->mkdir("occupancy");
	dir_original  = dir_top->mkdir("original");
	
	// Initialize the ROOT interpreter so it is ready to run RootSpy macros
	InitilizeRootInterpreter(&app);
	
	// Open log file so we can record some details about where each image file came from
	string fname_log = OUTPUT_DIR + "/origin_log.txt";
	ofs_log = new ofstream(fname_log);
	(*ofs_log) << "# filename  chunk  macro  ipad  HVboard_list  ROOT_file" << endl;

	// Loop over input root files
	for( auto root_fname : fnames ){
		auto hists_orig = GetCDCHists(root_fname);
		
		// Loop over all board inefficiency configurations
		for( auto board_eff_set : board_efficiencies ){
			auto hists_KO = MakeCDC_KO_hists(hists_orig, board_eff_set);
			
			RunCDC_occupancy_macro(&app);

			FreeHists( hists_KO );
		}
		
		// Free histograms
		FreeHists( hists_orig );
	}
	
	// Close log file
	delete ofs_log;
	
	// Tell JANA to quit cleanly
	app.Quit();
	sleep(1); // try and allow threads to quit cleanly
	if( app.GetExitCode() ) cerr << "Exit code: " << app.GetExitCode() << endl;
	return app.GetExitCode();
}

//-----------
// Usage
//-----------
void Usage(JApplication &app)
{
	cout<<endl;
	cout<<"Usage:"<<endl;
	cout<<"    hydra_cdc [options] source1 source2 source3 ..."<<endl;
	cout<<endl;
        jana::PrintUsage();
	cout<<endl;
	
	exit(0);
}

//-----------
// GetRootFilenames
//-----------
std::vector<std::string> GetRootFilenames(JApplication* app)
{
	std::vector<std::string> fnames;

	string ROOT_FILELIST = "root_files.txt";
	app->SetDefaultParameter("ROOT_FILELIST", ROOT_FILELIST, "Name of text file containing the list of ROOT files to be used to read histograms from.");
	ifstream ifs(ROOT_FILELIST);
	if( !ifs.is_open() ){
		jerr << "Unable to open \"" << ROOT_FILELIST << "\"!!" << endl;
		exit(-1);
	}
	while(! ifs.eof()){
	
		char str[1024];
		ifs.getline(str, 1024);
		string fname(str);
		
		// Strip off white space
		auto f = [](unsigned char c){return std::isspace(c);};
		fname.erase(remove_if(fname.begin(), fname.end(), f), fname.end());
		
		// Ignore commented out lines
		if(fname.empty()) continue;
		if(fname[0] == '#') continue;
		
		fnames.push_back( fname );
		jout << fname << endl;
	}
	
	jout << "Found " << fnames.size() << " filenames in " << ROOT_FILELIST << endl;
	return fnames;
}

//----------------------------------------------------------------
// GetCDCHists
//
// Open the given ROOT file and read in the CDC occupancy hists.
// Return them in order (by ring number).
//----------------------------------------------------------------
std::vector<TH1*> GetCDCHists(std::string &fname)
{
	CURRENT_ROOT_FILE = fname;
	cout << "--------------------------------------------------" << endl;
	cout << "Reading histograms from " << CURRENT_ROOT_FILE << endl;
	auto savedir = gDirectory;
	TFile rootfile(CURRENT_ROOT_FILE.c_str());
	savedir->cd();
	
	// Make list of histograms to get
	std::vector<std::string> hnames;
	for( int iring=1; iring<=28; iring++){
		char hname[256];
		sprintf(hname, "rootspy/occupancy/cdc_occ_ring_%02d", iring);
		hnames.push_back( hname );
	}
	hnames.push_back( "rootspy/occupancy/cdc_num_events" );
	hnames.push_back( "rootspy/occupancy/cdc_axes" );
	
	// Get pointers to all histograms and re-parent them to the save directory
	std::vector<TH1*> hists;
	for( auto hname : hnames ){
		auto h = (TH1*)rootfile.Get(hname.c_str());
		if( h ) {
			cout << "Loaded " << h->GetName() << " " << h << endl;
			h->SetDirectory( dir_original );
			hists.push_back(h);
		}
	}
	
	return hists;
}

//----------------------------------------------------------------
// MakeCDC_KO_hists
//
// Using the given set of cdc_hists and the board efficiency,
// create a new  set of CDC occupancy histograms with the
// ineffciencies applied.
//----------------------------------------------------------------
std::vector<TH1*> MakeCDC_KO_hists(std::vector<TH1*> &cdc_hists, board_eff_t &board_efficiency)
{

	// Make ROOT directory structure to clone histograms into
	auto savedir = gDirectory;
	dir_occupancy->cd();

	// Make copies of the original histograms
	std::vector<TH1*> KO_hists;
	for( auto h : cdc_hists ){
		auto h_copy = (TH1*)h->Clone( h->GetName() );
		h_copy->SetDirectory( dir_occupancy );
		KO_hists.push_back( h_copy );
	}
	savedir->cd();
	
	// Record which boards are knocked out so they can be written to log file
	CURRENT_KO_BOARDS.clear();

	// Apply inefficiencies to the copies
	for( auto p : board_efficiency ){
		auto hv_board = p.first;
		auto eff = p.second;
		
		CURRENT_KO_BOARDS.insert( hv_board );
		
		// Loop over straws connected to this HV board
		for( auto w : CDC_pins[hv_board] ){
			auto iring = w.first;
			auto istraw = w.second;
			auto h = KO_hists[ iring-1 ];
			auto v = h->GetBinContent(istraw, 1); // n.b. CDC occupancy hists are actually TH2F !
			auto v_new = (int)floor(v*eff);
			if( v_new < MIN_CDC_BIN_CONTENT ) v_new = MIN_CDC_BIN_CONTENT;
			h->SetBinContent(istraw, 1, v_new);
		}
	}

	return KO_hists;
}

//----------------------------------------------------------------
// FreeHists
//
// Delete all of the given histogram objects 
//----------------------------------------------------------------
void FreeHists( std::vector<TH1*> &hists)
{
	for( auto h : hists ) delete h;
}

//----------------------------------------------------------------
// InitilizeRootInterpreter
//----------------------------------------------------------------
void InitilizeRootInterpreter(JApplication *app)
{
	app->GetService<JLockService>()->RootWriteLock();
	// Create TCanvas, but set ROOT to batch mode so that it doesn't actually open a window.
	gROOT->SetBatch(kTRUE);
	c1 = new TCanvas("c1", "A Canvas", 1600, 1200);

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
	app->GetService<JLockService>()->RootUnLock();

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
}

//----------------------------------------------------------------
// RunCDC_occupancy_macro
//----------------------------------------------------------------
void RunCDC_occupancy_macro(JApplication* app)
{

	auto base_root_dir = gDirectory;

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

		if( macroFilename != "CDC_occupancy" ) continue;

		std::cout << "Executing: " << macroFilename << std::endl;

		c1->cd();
		c1->Clear();
		ExecuteMacro(app, base_root_dir, macroString);

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
						
						// Write log of file
						if(ofs_log){
							(*ofs_log) << fname_base
							           << " " << CHUNK_COUNTER[basename][ipad]
							           << " " << basename
									     << " " << ipad
									     << "  ";
							for( auto hv_board :	CURRENT_KO_BOARDS ){
								(*ofs_log) << "HV:" << hv_board << ",";
							}								  
							(*ofs_log) << " " << CURRENT_ROOT_FILE << endl;
						}

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
}

//-------------------
// ExecuteMacro
//-------------------
void ExecuteMacro(JApplication* app, TDirectory *f, string macro)
{
	// Lock ROOT
        app->GetService<JLockService>()->RootWriteLock();

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
	app->GetService<JLockService>()->RootUnLock();

}

