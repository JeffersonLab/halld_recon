// Author: David Lawrence  June 25, 2004
//
//
// hd_ana.cc
//

#include <iostream>
#include <iomanip>
#include <string>
using namespace std;

#include <stdlib.h>

#include <DANA/DApplication.h>
#include <HDGEOMETRY/DMagneticFieldMap.h>

#include <TFile.h>
#include <TTree.h>
#include <TVector3.h>
#include <TH2.h>

int32_t RUN_NUMBER = 30000;

int Nr = 81;
int Nphi = 1;
int Nz = 401;

double Rmin = 1.0*2.54;
double Rmax = 81.0*2.54;

double Phimin = 0.0;
double Phimax = 0.0;

double Zmin = -126.0*2.54;
double Zmax = 274.0*2.54;
double Z0 = 0.0;

bool INCLUDE_GRADIENTS = false;

void Usage(void);
void ParseCommandLineArgs(int narg, char* argv[], vector<char*> &unused_args);

//-----------
// main
//-----------
int main(int narg, char *argv[])
{

	// Parse command line arguments and then create a DApplication
	vector<char*> unused_args;
	ParseCommandLineArgs(narg, argv, unused_args);

	if( !unused_args.empty() ){
		cout << "The following arguments will be passed to DApplication:" << endl;
		for(auto a : unused_args) cout << "   " << a << endl;
		cout << endl;
	}

	DApplication *dapp = new DApplication(unused_args.size(), unused_args.empty() ? NULL:&unused_args[0]);
	dapp->GetJApp()->Initialize();

	// open ROOT file
	float val[100];
	TFile* ROOTfile = new TFile("bfield.root","RECREATE","Produced by bfield2root");
	ROOTfile->SetCompressionLevel(6);
	cout<<"Opened ROOT file \"bfield.root\""<<endl;

	// Have Dapplication create Magnetic Field Map
	DMagneticFieldMap *bfield = dapp->GetBfield(RUN_NUMBER);

	// Create Tree
	TTree *tree = new TTree("Bfield","Magnetic Field");
	tree->SetMarkerStyle(8);
	tree->SetMarkerColor(kBlue);
	string leaves = "x/F:y:z:r:phi:Bx:By:Bz:Br:Bphi";
	if(INCLUDE_GRADIENTS){
		leaves += ":dBxdx:dBxdy:dBxdz";
		leaves += ":dBydx:dBydy:dBydz";
		leaves += ":dBzdx:dBzdy:dBzdz";
	}
	tree->Branch("B",val,leaves.c_str());
	
	// Loop over cylindrical grid and fill tree
	double r = Rmin;
	for(int ir=0; ir<Nr; ir++){
		if(Nr>1) r = Rmin + (double)ir*(Rmax-Rmin)/(double)(Nr-1);
		double phi = Phimin;
		for(int iphi=0; iphi<Nphi; iphi++){
			if(Nphi>1) phi = Phimin + (double)iphi*(Phimax-Phimin)/(double)(Nphi-1);
			double z = Zmin;
			for(int iz=0; iz<Nz; iz++){
				if(Nz>1) z = Zmin + (double)iz*(Zmax-Zmin)/(double)(Nz-1);
				
				double x = r*cos(phi);
				double y = r*sin(phi);
				
				double Bx, By, Bz;
				double dBxdx, dBxdy, dBxdz;
				double dBydx, dBydy, dBydz;
				double dBzdx, dBzdy, dBzdz;

				bfield->GetFieldAndGradient(x, y, z-Z0, Bx, By, Bz, 
					dBxdx, dBxdy, dBxdz,
					dBydx, dBydy, dBydz,
					dBzdx, dBzdy, dBzdz);
				
				TVector3 B(Bx, By, Bz);
				double Br = B.Dot(TVector3(x, y, 0.0))/sqrt(x*x + y*y);
				double Bphi = atan2(By, Bx);
				
				val[0] = x;
				val[1] = y;
				val[2] = z;
				val[3] = r;
				val[4] = phi;
				val[5] = Bx;
				val[6] = By;
				val[7] = Bz;
				val[8] = Br;
				val[9] = Bphi;
				
				val[10] = dBxdx;
				val[11] = dBxdy;
				val[12] = dBxdz;

				val[13] = dBydx;
				val[14] = dBydy;
				val[15] = dBydz;

				val[16] = dBzdx;
				val[17] = dBzdy;
				val[18] = dBzdz;

				tree->Fill();
			}
		}	
	}
	
	// Create 2D histos in R and Z
	TH2D *Bz_vs_r_vs_z = new TH2D("Bz_vs_r_vs_z", "", Nz, Zmin, Zmax, Nr, Rmin, Rmax);
	Bz_vs_r_vs_z->SetXTitle("z (cm)");
	Bz_vs_r_vs_z->SetYTitle("r (cm)");
	Bz_vs_r_vs_z->SetStats(0);
	TH2D *Btot_vs_r_vs_z = (TH2D*)Bz_vs_r_vs_z->Clone("Btot_vs_r_vs_z");
	TH2D *dBtot_vs_r_vs_z = (TH2D*)Bz_vs_r_vs_z->Clone("dBtot_vs_r_vs_z");
	for(int ibin=1; ibin<=Bz_vs_r_vs_z->GetNbinsX(); ibin++){
		double z = Bz_vs_r_vs_z->GetXaxis()->GetBinCenter(ibin);
		for(int jbin=1; jbin<=Bz_vs_r_vs_z->GetNbinsY(); jbin++){
			double r = Bz_vs_r_vs_z->GetYaxis()->GetBinCenter(jbin);
			
			double Bx, By, Bz;
			double dBxdx, dBxdy, dBxdz;
			double dBydx, dBydy, dBydz;
			double dBzdx, dBzdy, dBzdz;

			bfield->GetFieldAndGradient(r, 0.0, z-Z0, Bx, By, Bz, 
				dBxdx, dBxdy, dBxdz,
				dBydx, dBydy, dBydz,
				dBzdx, dBzdy, dBzdz);
			double Btot = sqrt(Bx*Bx + By*By + Bz*Bz);
			
			// Gradient of magnitude
			TVector3 gradient(dBxdx*Bx/Btot + dBydx*By/Btot + dBzdx*Bz/Btot,
			                  dBxdy*Bx/Btot + dBydy*By/Btot + dBzdy*Bz/Btot,
					          dBxdz*Bx/Btot + dBydz*By/Btot + dBzdz*Bz/Btot);
			double dBtot = gradient.Mag();
			
			Bz_vs_r_vs_z->SetBinContent(ibin, jbin, Bz);
			Btot_vs_r_vs_z->SetBinContent(ibin, jbin, Btot);
			dBtot_vs_r_vs_z->SetBinContent(ibin, jbin, dBtot);
		}
	}

	// Create 2D histos in X and Z that cover a larger range
	TH2D *Btot_vs_x_vs_z = new TH2D("Btot_vs_x_vs_z", "", 800, -100.0, 700.0, 400, -200.0, 200.0);
	Btot_vs_x_vs_z->SetXTitle("x (cm)");
	Btot_vs_x_vs_z->SetYTitle("r (cm)");
	Btot_vs_x_vs_z->SetStats(0);
	for(int ibin=1; ibin<=Btot_vs_x_vs_z->GetNbinsX(); ibin++){
		double z = Btot_vs_x_vs_z->GetXaxis()->GetBinCenter(ibin);
		for(int jbin=1; jbin<=Btot_vs_x_vs_z->GetNbinsY(); jbin++){
			double x = Btot_vs_x_vs_z->GetYaxis()->GetBinCenter(jbin);
			
			double Bx, By, Bz;
			double dBxdx, dBxdy, dBxdz;
			double dBydx, dBydy, dBydz;
			double dBzdx, dBzdy, dBzdz;

			bfield->GetFieldAndGradient(x, 0.0, z-Z0, Bx, By, Bz, 
				dBxdx, dBxdy, dBxdz,
				dBydx, dBydy, dBydz,
				dBzdx, dBzdy, dBzdz);
			double Btot = sqrt(Bx*Bx + By*By + Bz*Bz);
			
			Btot_vs_x_vs_z->SetBinContent(ibin, jbin, Btot);
		}
	}

	// Angle 
	TH2D *cos_theta_vs_r_vs_z = new TH2D("cos_theta_vs_r_vs_z", "", 651, -25, 625.0, 400, 0.0, 200.0);
	cos_theta_vs_r_vs_z->SetXTitle("z (cm)");
	cos_theta_vs_r_vs_z->SetYTitle("r (cm)");
	cos_theta_vs_r_vs_z->SetStats(0);
	for(int ibin=1; ibin<=cos_theta_vs_r_vs_z->GetNbinsX(); ibin++){
		double z = cos_theta_vs_r_vs_z->GetXaxis()->GetBinCenter(ibin);
		for(int jbin=1; jbin<=cos_theta_vs_r_vs_z->GetNbinsY(); jbin++){
			double r = cos_theta_vs_r_vs_z->GetYaxis()->GetBinCenter(jbin);
			
			double Bx, By, Bz;
			bfield->GetField(r, 0.0, z-Z0, Bx, By, Bz);
			double Btot = sqrt(Bx*Bx + By*By + Bz*Bz);
			
			cos_theta_vs_r_vs_z->SetBinContent(ibin, jbin, Bz/Btot);
		}
	}

	// Zoom in to TOF
	TH2D *Btot_vs_r_vs_z_tof = new TH2D("Btot_vs_r_vs_z_tof", "", 61, 590.0, 650.0, 200, 76.0, 180.0);
	Btot_vs_r_vs_z_tof->SetXTitle("z (cm)");
	Btot_vs_r_vs_z_tof->SetYTitle("r (cm)");
	Btot_vs_r_vs_z_tof->SetStats(0);
	TH2D *Bz_vs_r_vs_z_tof = (TH2D*)Btot_vs_r_vs_z_tof->Clone("Bz_vs_r_vs_z_tof");
	TH2D *Br_vs_r_vs_z_tof = (TH2D*)Btot_vs_r_vs_z_tof->Clone("Br_vs_r_vs_z_tof");
	for(int ibin=1; ibin<=Btot_vs_r_vs_z_tof->GetNbinsX(); ibin++){
		double z = Btot_vs_r_vs_z_tof->GetXaxis()->GetBinCenter(ibin);
		for(int jbin=1; jbin<=Btot_vs_r_vs_z_tof->GetNbinsY(); jbin++){
			double r = Btot_vs_r_vs_z_tof->GetYaxis()->GetBinCenter(jbin);
			
			double Bx, By, Bz;
			double dBxdx, dBxdy, dBxdz;
			double dBydx, dBydy, dBydz;
			double dBzdx, dBzdy, dBzdz;

			bfield->GetFieldAndGradient(r, 0.0, z-Z0, Bx, By, Bz, 
				dBxdx, dBxdy, dBxdz,
				dBydx, dBydy, dBydz,
				dBzdx, dBzdy, dBzdz);
			double Btot = sqrt(Bx*Bx + By*By + Bz*Bz);
			double Br = sqrt(Bx*Bx + By*By);
			
			Btot_vs_r_vs_z_tof->SetBinContent(ibin, jbin, Btot);
			Bz_vs_r_vs_z_tof->SetBinContent(ibin, jbin, Bz);
			Br_vs_r_vs_z_tof->SetBinContent(ibin, jbin, Br);
		}
	}

	ROOTfile->Write();
	delete ROOTfile;
	cout<<endl<<"Closed ROOT file"<<endl;
		
	return 0;
}

//-----------------------
// Usage
//-----------------------
void Usage(void)
{
	cout<<endl;
	cout<<"Usage:"<<endl;
	cout<<"   bfield2root [options]"<<endl;
	cout<<endl;
	cout<<" options:"<<endl;
	cout<<"    -h, --help   Show this Usage statement"<<endl;
	cout<<"    -R      #    Set the run number to use when accessing field map"<<endl;
	cout<<"    -Nr     #    Set the number of grid points in R"<<endl;
	cout<<"    -Nphi   #    Set the number of grid points in Phi"<<endl;
	cout<<"    -Nz     #    Set the number of grid points in Z"<<endl;
	cout<<"    -Rmin   #    Set the minimum value for R (cm)"<<endl;
	cout<<"    -Rmax   #    Set the maximum value for R (cm)"<<endl;
	cout<<"    -Phimin #    Set the minimum value for Phi (radians)"<<endl;
	cout<<"    -Phimax #    Set the maximum value for Phi (radians)"<<endl;
	cout<<"    -Zmin   #    Set the minimum value for Z (cm)"<<endl;
	cout<<"    -Zmax   #    Set the maximum value for Z (cm)"<<endl;
	cout<<"    -Z0     #    Shift the map's z-coordinate"<<endl;
	cout<<"    -G           Include gradient values in tree"<<endl;
	cout<<endl;
	cout<<" The bfield to root program can be used to generate a ROOT TTree of the"<<endl;
	cout<<"magnetic field of the Hall-D solenoid. The field is read from the same source"<<endl;
	cout<<"as is used by the reconstruction and simulation programs. Since it is a DANA"<<endl;
	cout<<"based program, it accepts the same same arguments to manipulate the field as"<<endl;
	cout<<"other DANA programs. Namely, -PBFIELD_MAP=XXX and -PBFIELD_TYPE=YYY ."<<endl;
	cout<<endl;
	cout<<"For example:  -PBFIELD_MAP=Magnets/Solenoid/solenoid_1500"<<endl;
	cout<<endl;
	cout<<"The entries in the TTree are evaluated on a grid in cylindrical coordinates"<<endl;
	cout<<"that likely does not reflect the points in the underlying map. As such, the"<<endl;
	cout<<"values in the tree are a result of the interpolation of the underlying map's"<<endl;
	cout<<"points."<<endl;
	cout<<endl;

}

//-----------------------
// ParseCommandLineArgs
//-----------------------
void ParseCommandLineArgs(int narg, char* argv[], vector<char*> &unused_args)
{
	unused_args.push_back(argv[0]);

	for(int i=1; i<narg; i++){
		string arg(argv[i]);
		string next(i<narg-1 ? argv[i+1]:"");
		float argf = atof(next.c_str());
		int   argi = atoi(next.c_str());
		bool used_next = false; // keep track if "next" is used so we can have a single error check below
		
		if(arg=="-h" || arg=="--help"){Usage(); exit(0);}
		else if(arg=="-R"     ){used_next=true; RUN_NUMBER = argi;}
		else if(arg=="-Nr"    ){used_next=true; Nr = argi;}
		else if(arg=="-Nphi"  ){used_next=true; Nphi = argi;}
		else if(arg=="-Nz"    ){used_next=true; Nz = argi;}
		else if(arg=="-Rmin"  ){used_next=true; Rmin = argf;}
		else if(arg=="-Rmax"  ){used_next=true; Rmax = argf;}
		else if(arg=="-Phimin"){used_next=true; Phimin = argf;}
		else if(arg=="-Phimax"){used_next=true; Phimax = argf;}
		else if(arg=="-Zmin"  ){used_next=true; Zmin = argf;}
		else if(arg=="-Zmax"  ){used_next=true; Zmax = argf;}
		else if(arg=="-Z0"    ){used_next=true; Z0 = argf;}
		else if(arg=="-G"     ){INCLUDE_GRADIENTS=true;}
		else{ unused_args.push_back(argv[i]); }

		if(used_next){
			// skip to next argument
			i++;
			
			// If i is now > than narg, then no value was passed to an "-XXX" argument
			if(i>narg){
				_DBG_<<"No argument given for \""<<arg<<"\" argument!"<<endl;
				Usage();
				exit(-1);
			}
		}
	}
}

