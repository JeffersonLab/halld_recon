
//
// This ROOT macro orginally created as a skelton using the
// "mkrootmacro" script. (davidl@jlab.org)
//
// Run this for an amorphous radiator run using the following.
// Ideally, one would use an entire EVIO file's worth of data.
//
//   ssh hdops@gluonraid6
//   mkdir ~/work/2023.01.15.amorphous_normalization
//   cd ~/work/2023.01.15.amorphous_normalization
//   ln -s $HALLD_RECON_HOME/src/plugins/monitoring/highlevel_online/AmorphousNormalization/ .
//   setenv run 120219
//   hd_ana --loadconfigs AmorphousNormalization/jana.conf /gluonraid6/rawdata/volatile/RunPeriod*/rawdata/Run${run}/hd_rawdata_${run}_000.evio
//   mv janaroot.root janaroot_${run}.root
//   root -l -q "AmorphousNormalization/extract_photon_energy_spectrum.C(${run})"
//
//   
//

#include "StandardLabels.C"

#include <TColor.h>
#include <TGraph.h>
#include <TFile.h>
#include <TTree.h>
#include <TF1.h>

void extract_photon_energy_spectrum(int run=40859)
{
	TColor::CreateColorWheel();
	
	char tmp[256];
	sprintf(tmp, "janaroot_%06d.root", run);
	string fname = tmp;
	string source = "<unknown>";
	
	// Option to extract spectrum from online monitoring
	// ROOT file produced from highlevel_online plugin or
	// one produced by janaroot plugin
	TH1I  *h = NULL;
	TFile *f = new TFile(fname.c_str());
	if(fname.find("hdmon_online") == 0){
		gDirectory->cd("rootspy");
		gDirectory->cd("highlevel");
		h = (TH1I*)gDirectory->Get("BeamEnergy");
		source = "BeamEnergy histogram from the online monitoring root file";
	}else{
		TTree *t = (TTree*)gDirectory->Get("DBeamPhoton");
		if(t){
			h = new TH1I("EbeamPhoton", "" , 240, 0.0, 12.0);
			t->Project("EbeamPhoton", "E");
		}
		source = "DBeamPhoton::E of a janaroot file";
	}
	
	if(!h){
		cout << "Problem getting histogram!" << endl;
		return;
	}
	
	cout << endl;
	cout << "Please cut and paste the following into:" << endl;
	cout << endl;
	cout << "$HALLD_RECON_HOME/src/plugins/monitoring/highlevel_online/HistMacro_Beam.C" << endl;
	cout << endl;
	cout << "//--------------------------------------------------------------------" << endl;
	cout << "// This taken from the bin contents of: " << source << endl;
	cout << "// for run " << run << ", an amorphous target run." << endl;
	cout << "// The data in the table below was created with a macro in:" << endl;
	cout << "//   $HALLD_RECON_HOME/src/plugins/monitoring/highlevel_online/AmorphousNormalization" << endl;
	cout << "//" << endl;
	cout << "// The working directory used was:" << endl;
	cout << "//     ~hdops/2018.10.05.amorphous_normalization" << endl;
	cout << "//" << endl;
	cout << "string amorphous_label = \"Normalized to Amorphous run " << run << "\";" << endl;
	cout << endl;
	cout << "	Double_t amorphous_data[] = {" << endl;
	for(int ibin=1; ibin<=h->GetNbinsX(); ibin++){
		
		Int_t v = h->GetBinContent(ibin);
		
		char str[256];
		sprintf(str, "%9d.0", v);
		
		if(ibin%10 == 1) cout << "\t\t" ;

		cout << str << ", ";
		if(ibin%10 == 0) cout << endl;
	}
	cout << "	0.0};" << endl;
	cout << "//--------------------------------------------------------------------" << endl;

}

