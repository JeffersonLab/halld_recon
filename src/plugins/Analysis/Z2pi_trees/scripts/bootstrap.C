void bootstrap (TString filename0="twopi_primakoff_DSelect", Int_t nfiles=100)
{
  /* File: bootstrap.C
    Read *.fit2 files with output of bootstrap sampling to determine uncertainties

Here are the notes in the bootstrap.py script from the parent directory. This file is in the "subdirectory Bootstrap_fit2"

The following structure is assumed (although easily modified)

parent directory: Z2pi_trees  
- contains all the root trees and analysis of a single fit

current directory: Bootstrap 
- contains this file which performs multiple fits with different bootstrap samples
- fit_2pi_primakoff_W_100000_TEMPLATE.cfg has the template for the configuration file. It is a copy of the configuration in the parent directory, except a) uses the ROOTDataReaderBootstrap reader for the data, b) The template has the RANDOM string, which will be replaced with a sequential number for the TRandom2 seed used by the reader.

subdirectory Bootstrap_fit2
- the results of the fits are written to files in the subdirectory Bootstrap_fit2 into files with extensions fit2. 
- The root script bootstrap.C will read the files with the bootstrap fits to obtain bootstrap errors and produced comparison plots between the bootstrap method and fit outputs.
. the output file twopi_primakoff_DSelect_out.fit2 has the same format as the input files, but contains the fitted values from the 00000 file and errors from the bootstrap method. This file can be used with the root script twopi_primakoff.C in the Z2pi_trees parent directory to generate output similar to that of a single fit but with the bootstrap error in the tables.


    */

  gStyle->SetPalette(1,0);
  gStyle->SetOptStat(111111);
  // gStyle->SetOptStat(0);
  gStyle->SetOptFit(111111);
  gStyle->SetPadRightMargin(0.15);
  gStyle->SetPadLeftMargin(0.15);
  gStyle->SetPadBottomMargin(0.15);


    TString string;
    vector <TString> sdme;
    const Double_t PI=3.14159;


    // define histograms

    /*const Int_t nhist=5*5;
    TH1F *eps[nhist];

    for (Int_t jj=0; jj<nhist; jj++) {
      string.Form("eps_%05d",jj);
      cout << "string=" << string << endl;
      eps[jj] = new TH1F(string,string,100,-10,10);
      }*/

    
    vector <double> parms;
    vector <double> parms_err;
    vector <double> parms_ij;     // off diagonal element

    vector <double> g1v00_re;
    vector <double> g1v00_im;
    vector <double> g1v00s_re;
    vector <double> g1v00s_im;
    vector <double> g1v11_re;
    vector <double> g1v11_im;
    vector <double> g1v10_re;
    vector <double> g1v10_im;
    vector <double> g1v1m1_re;
    vector <double> g1v1m1_im;

    vector <double> g1v00_re_err;
    vector <double> g1v00_im_err;
    vector <double> g1v00s_re_err;
    vector <double> g1v00s_im_err;
    vector <double> g1v11_re_err;
    vector <double> g1v11_im_err;
    vector <double> g1v10_re_err;
    vector <double> g1v10_im_err;
    vector <double> g1v1m1_re_err;
    vector <double> g1v1m1_im_err;

    vector <double> g1v00_reim_fit;
    vector <double> g1v00s_reim_fit;
    vector <double> g1v11_reim_fit;
    vector <double> g1v10_reim_fit;
    vector <double> g1v1m1_reim_fit;


    // initialize moments

    Double_t g1v00_re1=0;
    Double_t g1v00_im1=0;
    Double_t g1v00s_re1=0;
    Double_t g1v00s_im1=0;
    Double_t g1v11_re1=0;
    Double_t g1v11_im1=0;
    Double_t g1v10_re1=0;
    Double_t g1v10_im1=0;
    Double_t g1v1m1_re1=0;
    Double_t g1v1m1_im1=0;

    Double_t g1v00_re2=0;
    Double_t g1v00_im2=0;
    Double_t g1v00s_re2=0;
    Double_t g1v00s_im2=0;
    Double_t g1v11_re2=0;
    Double_t g1v11_im2=0;
    Double_t g1v10_re2=0;
    Double_t g1v10_im2=0;
    Double_t g1v1m1_re2=0;
    Double_t g1v1m1_im2=0;

    Double_t g1v00_reim=0;     // correllations
    Double_t g1v00s_reim=0;
    Double_t g1v11_reim=0;
    Double_t g1v10_reim=0;
    Double_t g1v1m1_reim=0;

      
    // now read and print fitted values
    
    ifstream parameters;
    TString infile;
    Int_t ntokens;
    Int_t ntokens_save;;

    for (Int_t kk=0; kk<nfiles; kk++) {
      infile.Form("%s_%05d%s",filename0.Data(),kk,".fit2");   // file with parameters
    cout << "*** Starting to read parameters from " << infile.Data() << endl << endl;
    parameters.open (infile.Data());
    if (!parameters) {
        cout << "ERROR: Failed to open data file= " << infile.Data() << endl;
        return;
    }

    // Read first line with fitted parameters
    Int_t nlines=0;
    TString line;
    line.ReadLine(parameters);
    TObjArray *tokens = line.Tokenize("\t");
    ntokens = tokens->GetEntries();

    // save values from first file (00)

    if (kk == 0) {
      ntokens_save = ntokens;
      for (Int_t k=0; k<ntokens/3; k++){
	sdme.push_back( (((TObjString*)tokens->At(3*k))->GetString()) );
	parms.push_back( (((TObjString*)tokens->At(3*k+1))->GetString()).Atof() );
	parms_err.push_back( (((TObjString*)tokens->At(3*k+2))->GetString()).Atof());
      }
    }

    // parameters and their errors

    g1v00_re.push_back( (((TObjString*)tokens->At(1))->GetString()).Atof() );
    g1v00_im.push_back( (((TObjString*)tokens->At(4))->GetString()).Atof() );
    g1v00s_re.push_back( (((TObjString*)tokens->At(7))->GetString()).Atof() );
    g1v00s_im.push_back( (((TObjString*)tokens->At(10))->GetString()).Atof() );
    g1v11_re.push_back( (((TObjString*)tokens->At(13))->GetString()).Atof() );
    g1v11_im.push_back( (((TObjString*)tokens->At(16))->GetString()).Atof() );
    g1v10_re.push_back( (((TObjString*)tokens->At(19))->GetString()).Atof() );
    g1v10_im.push_back( (((TObjString*)tokens->At(22))->GetString()).Atof() );
    g1v1m1_re.push_back( (((TObjString*)tokens->At(25))->GetString()).Atof() );
    g1v1m1_im.push_back( (((TObjString*)tokens->At(28))->GetString()).Atof() );

    g1v00_re_err.push_back( (((TObjString*)tokens->At(2))->GetString()).Atof() );
    g1v00_im_err.push_back( (((TObjString*)tokens->At(5))->GetString()).Atof() );
    g1v00s_re_err.push_back( (((TObjString*)tokens->At(8))->GetString()).Atof() );
    g1v00s_im_err.push_back( (((TObjString*)tokens->At(11))->GetString()).Atof() );
    g1v11_re_err.push_back( (((TObjString*)tokens->At(14))->GetString()).Atof() );
    g1v11_im_err.push_back( (((TObjString*)tokens->At(17))->GetString()).Atof() );
    g1v10_re_err.push_back( (((TObjString*)tokens->At(20))->GetString()).Atof() );
    g1v10_im_err.push_back( (((TObjString*)tokens->At(23))->GetString()).Atof() );
    g1v1m1_re_err.push_back( (((TObjString*)tokens->At(26))->GetString()).Atof() );
    g1v1m1_im_err.push_back( (((TObjString*)tokens->At(29))->GetString()).Atof() );

    // read lines with covariance matrix

    
    while (line.ReadLine(parameters)){
      tokens = line.Tokenize(" ");
      ntokens = tokens->GetEntries();
      cout << "kk=" << kk << " line=" << line << endl;
      cout << "kk=" << kk << " nlines=" << nlines << " ntokens=" << ntokens << endl;
      if (nlines == 0) g1v00_reim_fit.push_back( (((TObjString*)tokens->At(nlines+1))->GetString()).Atof() );
      if (nlines == 2) g1v00s_reim_fit.push_back( (((TObjString*)tokens->At(nlines+1))->GetString()).Atof() );
      if (nlines == 4) g1v11_reim_fit.push_back( (((TObjString*)tokens->At(nlines+1))->GetString()).Atof() );
      if (nlines == 6) g1v10_reim_fit.push_back( (((TObjString*)tokens->At(nlines+1))->GetString()).Atof() );
      if (nlines == 8) g1v1m1_reim_fit.push_back( (((TObjString*)tokens->At(nlines+1))->GetString()).Atof() );
      nlines++;
    }

    parameters.close();

    cout << "g1v00_re=" << g1v00_re[kk] << endl;
    cout << "g1v00_im=" << g1v00_im[kk] << endl;
    cout << "g1v00s_re=" << g1v00s_re[kk] << endl;
    cout << "g1v00s_im=" << g1v00s_im[kk] << endl;
    cout << "g1v11_re=" << g1v11_re[kk] << endl;
    cout << "g1v11_im=" << g1v11_im[kk] << endl;
    cout << "g1v10_re=" << g1v10_re[kk] << endl;
    cout << "g1v10_im=" << g1v10_im[kk] << endl;
    cout << "g1v1m1_re=" << g1v1m1_re[kk] << endl;
    cout << "g1v1m1_im=" << g1v1m1_im[kk] << endl << endl;

    cout << "kk=" << kk << " g1v00_reim_fit=" << g1v00_reim_fit[kk] << endl;
    cout << "kk=" << kk << " g1v00s_reim_fit=" << g1v00s_reim_fit[kk] << endl;
    cout << "kk=" << kk << " g1v11_reim_fit=" << g1v11_reim_fit[kk] << endl;
    cout << "kk=" << kk << " g1v10_reim_fit=" << g1v10_reim_fit[kk] << endl;
    cout << "kk=" << kk << " g1v1m1_reim_fit=" << g1v1m1_reim_fit[kk] << endl;

    g1v00_re1 += g1v00_re[kk];
    g1v00_im1 += g1v00_im[kk];
    g1v00s_re1 += g1v00s_re[kk];
    g1v00s_im1 += g1v00s_im[kk];
    g1v11_re1 += g1v11_re[kk];
    g1v11_im1 += g1v11_im[kk];
    g1v10_re1 += g1v10_re[kk];
    g1v10_im1 += g1v10_im[kk];
    g1v1m1_re1 += g1v1m1_re[kk];
    g1v1m1_im1 += g1v1m1_im[kk];

    g1v00_re2 += g1v00_re[kk]*g1v00_re[kk];
    g1v00_im2 += g1v00_im[kk]*g1v00_im[kk];
    g1v00s_re2 += g1v00s_re[kk]*g1v00s_re[kk];
    g1v00s_im2 += g1v00s_im[kk]*g1v00s_im[kk];
    g1v11_re2 += g1v11_re[kk]*g1v11_re[kk];
    g1v11_im2 += g1v11_im[kk]*g1v11_im[kk];
    g1v10_re2 += g1v10_re[kk]*g1v10_re[kk];
    g1v10_im2 += g1v10_im[kk]*g1v10_im[kk];
    g1v1m1_re2 += g1v1m1_re[kk]*g1v1m1_re[kk];
    g1v1m1_im2 += g1v1m1_im[kk]*g1v1m1_im[kk];

    g1v00_reim += g1v00_re[kk]*g1v00_im[kk];
    g1v00s_reim += g1v00s_re[kk]*g1v00s_im[kk];
    g1v11_reim += g1v11_re[kk]*g1v11_im[kk];
    g1v10_reim += g1v10_re[kk]*g1v10_im[kk];
    g1v1m1_reim += g1v1m1_re[kk]*g1v1m1_im[kk];

    // end of loop over files
    }

    g1v00_re1 /= nfiles;
    g1v00_im1 /= nfiles;
    g1v00s_re1 /= nfiles;
    g1v00s_im1 /= nfiles;
    g1v11_re1 /= nfiles;
    g1v11_im1 /= nfiles;
    g1v10_re1 /= nfiles;
    g1v10_im1 /= nfiles;
    g1v1m1_re1 /= nfiles;
    g1v1m1_im1 /= nfiles;

    g1v00_re2 /= nfiles;
    g1v00_im2 /= nfiles;
    g1v00s_re2 /= nfiles;
    g1v00s_im2 /= nfiles;
    g1v11_re2 /= nfiles;
    g1v11_im2 /= nfiles;
    g1v10_re2 /= nfiles;
    g1v10_im2 /= nfiles;
    g1v1m1_re2 /= nfiles;
    g1v1m1_im2 /= nfiles;

    g1v00_reim /= nfiles;
    g1v00s_reim /= nfiles;
    g1v11_reim /= nfiles;
    g1v10_reim /= nfiles;
    g1v1m1_reim /= nfiles;

    g1v00_reim -= g1v00_re1*g1v00_im1;
    g1v00s_reim -= g1v00s_re1*g1v00s_im1;
    g1v11_reim -= g1v11_re1*g1v11_im1;
    g1v10_reim -= g1v10_re1*g1v10_im1;
    g1v1m1_reim -= g1v1m1_re1*g1v1m1_im1;

    
    Double_t g1v00_re_sigma2 = g1v00_re2-g1v00_re1*g1v00_re1;
    Double_t g1v00_im_sigma2 = g1v00_im2-g1v00_im1*g1v00_im1 ;
    Double_t g1v00s_re_sigma2 = g1v00s_re2-g1v00s_re1*g1v00s_re1;
    Double_t g1v00s_im_sigma2 = g1v00s_im2-g1v00s_im1*g1v00s_im1;
    Double_t g1v11_re_sigma2 = g1v11_re2-g1v11_re1*g1v11_re1;
    Double_t g1v11_im_sigma2 = g1v11_im2-g1v11_im1*g1v11_im1;
    Double_t g1v10_re_sigma2 = g1v10_re2-g1v10_re1*g1v10_re1;
    Double_t g1v10_im_sigma2 = g1v10_im2-g1v10_im1*g1v10_im1;
    Double_t g1v1m1_re_sigma2 = g1v1m1_re2-g1v1m1_re1*g1v1m1_re1;
    Double_t g1v1m1_im_sigma2 = g1v1m1_im2-g1v1m1_im1*g1v1m1_im1;

    g1v00_re_sigma = g1v00_re_sigma2 > 0? sqrt(g1v00_re_sigma2) : 0;
    g1v00_im_sigma = g1v00_im_sigma2 > 0? sqrt(g1v00_im_sigma2) : 0;
    g1v00s_re_sigma = g1v00s_re_sigma2 > 0? sqrt(g1v00s_re_sigma2) : 0;
    g1v00s_im_sigma = g1v00s_im_sigma2 > 0? sqrt(g1v00s_im_sigma2) : 0;
    g1v11_re_sigma = g1v11_re_sigma2 > 0? sqrt(g1v11_re_sigma2) : 0;
    g1v11_im_sigma = g1v11_im_sigma2 > 0? sqrt(g1v11_im_sigma2) : 0;
    g1v10_re_sigma = g1v10_re_sigma2 > 0? sqrt(g1v10_re_sigma2) : 0;
    g1v10_im_sigma = g1v10_im_sigma2 > 0? sqrt(g1v10_im_sigma2) : 0;
    g1v1m1_re_sigma = g1v1m1_re_sigma2 > 0? sqrt(g1v1m1_re_sigma2) : 0;
    g1v1m1_im_sigma = g1v1m1_im_sigma2 > 0? sqrt(g1v1m1_im_sigma2) : 0;


    cout << endl;

    cout << " g1v00_re1=" << g1v00_re1 << endl;
    cout << " g1v00_im1=" << g1v00_im1 << endl;
    cout << " g1v00s_re1=" << g1v00s_re1 << endl;
    cout << " g1v00s_im1=" << g1v00s_im1 << endl;
    cout << " g1v11_re1=" << g1v11_re1 << endl;
    cout << " g1v11_im1=" << g1v11_im1 << endl;
    cout << " g1v10_re1=" << g1v10_re1 << endl;
    cout << " g1v10_im1=" << g1v10_im1 << endl;
    cout << " g1v1m1_re1=" << g1v1m1_re1 << endl;
    cout << " g1v1m1_im1=" << g1v1m1_im1 << endl;
    cout << endl;

    cout << " g1v00_re_sigma=" << g1v00_re_sigma << endl;
    cout << " g1v00_im_sigma=" << g1v00_im_sigma << endl;
    cout << " g1v00s_re_sigma=" << g1v00s_re_sigma << endl;
    cout << " g1v00s_im_sigma=" << g1v00s_im_sigma << endl;
    cout << " g1v11_re_sigma=" << g1v11_re_sigma << endl;
    cout << " g1v11_im_sigma=" << g1v11_im_sigma << endl;
    cout << " g1v10_re_sigma=" << g1v10_re_sigma << endl;
    cout << " g1v10_im_sigma=" << g1v10_im_sigma << endl;
    cout << " g1v1m1_re_sigma=" << g1v1m1_re_sigma << endl;
    cout << " g1v1m1_im_sigma=" << g1v1m1_im_sigma << endl;
    cout << endl;

    cout << " g1v00_reim=" << g1v00_reim << endl;
    cout << " g1v00s_reim=" << g1v00s_reim << endl;
    cout << " g1v11_reim=" << g1v11_reim << endl;
    cout << " g1v10_reim=" << g1v10_reim << endl;
    cout << " g1v1m1_reim=" << g1v1m1_reim << endl;


    // define historgrams
    const Int_t nbins=100;
    Double_t xscale=10;
    Double_t covmax=10;

    TH1F *h1_g1v00_re = new  TH1F("h1_g1v00_re","g1v00 RE",nbins,-xscale*g1v00_re_sigma,xscale*g1v00_re_sigma);
    TH1F *h1_g1v00_im = new  TH1F("h1_g1v00_im","g1v00 IM",nbins,-xscale*g1v00_im_sigma,xscale*g1v00_im_sigma);
    TH1F *h1_g1v00s_re = new  TH1F("h1_g1v00s_re","g1v00s RE",nbins,-xscale*g1v00s_re_sigma,xscale*g1v00s_re_sigma);
    TH1F *h1_g1v00s_im = new  TH1F("h1_g1v00s_im","g1v00s IM",nbins,-xscale*g1v00s_im_sigma,xscale*g1v00s_im_sigma);
    TH1F *h1_g1v11_re = new  TH1F("h1_g1v11_re","g1v11 RE",nbins,-xscale*g1v11_re_sigma,xscale*g1v11_re_sigma);
    TH1F *h1_g1v11_im = new  TH1F("h1_g1v11_im","g1v11 IM",nbins,-xscale*g1v11_im_sigma,xscale*g1v11_im_sigma);
    TH1F *h1_g1v10_re = new  TH1F("h1_g1v10_re","g1v10 RE",nbins,-xscale*g1v10_re_sigma,xscale*g1v10_re_sigma);
    TH1F *h1_g1v10_im = new  TH1F("h1_g1v10_im","g1v10 IM",nbins,-xscale*g1v10_im_sigma,xscale*g1v10_im_sigma);
    TH1F *h1_g1v1m1_re = new  TH1F("h1_g1v1m1_re","g1v1m1 RE",nbins,-xscale*g1v1m1_re_sigma,xscale*g1v1m1_re_sigma);
    TH1F *h1_g1v1m1_im = new  TH1F("h1_g1v1m1_im","g1v1m1 IM",nbins,-xscale*g1v1m1_im_sigma,xscale*g1v1m1_im_sigma);

    xscale = 2;
    TH1F *h1_g1v00_re_err = new  TH1F("h1_g1v00_re_err","g1v00 RE fit error",nbins,0,xscale*g1v00_re_sigma);
    TH1F *h1_g1v00_im_err = new  TH1F("h1_g1v00_im_err","g1v00 IM fit error",nbins,0,xscale*g1v00_im_sigma);
    TH1F *h1_g1v00s_re_err = new  TH1F("h1_g1v00s_re_err","g1v00s RE fit error",nbins,0,xscale*g1v00s_re_sigma);
    TH1F *h1_g1v00s_im_err = new  TH1F("h1_g1v00s_im_err","g1v00s IM fit error",nbins,0,xscale*g1v00s_im_sigma);
    TH1F *h1_g1v11_re_err = new  TH1F("h1_g1v11_re_err","g1v11 RE fit error",nbins,0,xscale*g1v11_re_sigma);
    TH1F *h1_g1v11_im_err = new  TH1F("h1_g1v11_im_err","g1v11 IM fit error",nbins,0,xscale*g1v11_im_sigma);
    TH1F *h1_g1v10_re_err = new  TH1F("h1_g1v10_re_err","g1v10 RE fit error",nbins,0,xscale*g1v10_re_sigma);
    TH1F *h1_g1v10_im_err = new  TH1F("h1_g1v10_im_err","g1v10 IM fit error",nbins,0,xscale*g1v10_im_sigma);
    TH1F *h1_g1v1m1_re_err = new  TH1F("h1_g1v1m1_re_err","g1v1m1 RE fit error",nbins,0,xscale*g1v1m1_re_sigma);
    TH1F *h1_g1v1m1_im_err = new  TH1F("h1_g1v1m1_im_err","g1v1m1 IM fit error",nbins,0,xscale*g1v1m1_im_sigma);

    TH1F *h1_g1v00_reim = new  TH1F("h1_g1v00_reim","g1v00 #rho_{xy} Corr Re/Im",nbins,-covmax,covmax);
    TH1F *h1_g1v00s_reim = new  TH1F("h1_g1v00s_reim","g1v00s #rho_{xy} Corr Re/Im",nbins,-covmax,covmax);
    TH1F *h1_g1v11_reim = new  TH1F("h1_g1v11_reim","g1v11 #rho_{xy} Corr Re/Im",nbins,-covmax,covmax);
    TH1F *h1_g1v10_reim = new  TH1F("h1_g1v10_reim","g1v10 #rho_{xy} Corr Re/Im",nbins,-covmax,covmax);
    TH1F *h1_g1v1m1_reim = new  TH1F("h1_g1v1m1_reim","g1v1m1 #rho_{xy} Corr Re/Im",nbins,-covmax,covmax);

    TH1F *h1_g1v00_reim_fit = new  TH1F("h1_g1v00_reim_fit","g1v00 Fit V_{xy} Corr Re/Im",nbins,-covmax,covmax);
    TH1F *h1_g1v00s_reim_fit = new  TH1F("h1_g1v00s_reim_fit","g1v00s Fit V_{xy} Corr Re/Im",nbins,0,1000);
    TH1F *h1_g1v11_reim_fit = new  TH1F("h1_g1v11_reim_fit","g1v11 Fit V_{xy} Corr Re/Im",nbins,0,200);
    TH1F *h1_g1v10_reim_fit = new  TH1F("h1_g1v10_reim_fit","g1v10 Fit V_{xy} Corr Re/Im",nbins,-50,50);
    TH1F *h1_g1v1m1_reim_fit = new  TH1F("h1_g1v1m1_reim_fit","g1v1m1 Fit V_{xy} Corr Re/Im",nbins,-20,20);

    // fill histograms

    for (Int_t kk=0; kk<nfiles; kk++) {
      h1_g1v00_re->Fill(g1v00_re[kk]-g1v00_re1);
      h1_g1v00_im->Fill(g1v00_im[kk]-g1v00_im1);
      h1_g1v00s_re->Fill(g1v00s_re[kk]-g1v00s_re1);
      h1_g1v00s_im->Fill(g1v00s_im[kk]-g1v00s_im1);
      h1_g1v11_re->Fill(g1v11_re[kk]-g1v11_re1);
      h1_g1v11_im->Fill(g1v11_im[kk]-g1v11_im1);
      h1_g1v10_re->Fill(g1v10_re[kk]-g1v10_re1);
      h1_g1v10_im->Fill(g1v10_im[kk]-g1v10_im1);
      h1_g1v1m1_re->Fill(g1v1m1_re[kk]-g1v1m1_re1);
      h1_g1v1m1_im->Fill(g1v1m1_im[kk]-g1v1m1_im1);

      h1_g1v00_re_err->Fill(g1v00_re_err[kk]);
      h1_g1v00s_re_err->Fill(g1v00s_re_err[kk]);
      h1_g1v11_re_err->Fill(g1v11_re_err[kk]);
      h1_g1v10_re_err->Fill(g1v10_re_err[kk]);
      h1_g1v1m1_re_err->Fill(g1v1m1_re_err[kk]);

      h1_g1v00_im_err->Fill(g1v00_im_err[kk]);
      h1_g1v00s_im_err->Fill(g1v00s_im_err[kk]);
      h1_g1v11_im_err->Fill(g1v11_im_err[kk]);
      h1_g1v10_im_err->Fill(g1v10_im_err[kk]);
      h1_g1v1m1_im_err->Fill(g1v1m1_im_err[kk]);

      Double_t rhoxy = (g1v00_re[kk]-g1v00_re1)*(g1v00_im[kk]-g1v00_im1)/(g1v00_re_sigma*g1v00_im_sigma);
      h1_g1v00_reim->Fill(rhoxy);
      rhoxy = (g1v00s_re[kk]-g1v00s_re1)*(g1v00s_im[kk]-g1v00s_im1)/(g1v00s_re_sigma*g1v00s_im_sigma);
      h1_g1v00s_reim->Fill(rhoxy);
      rhoxy = (g1v11_re[kk]-g1v11_re1)*(g1v11_im[kk]-g1v11_im1)/(g1v11_re_sigma*g1v11_im_sigma);
      h1_g1v11_reim->Fill(rhoxy);
      rhoxy = (g1v10_re[kk]-g1v10_re1)*(g1v10_im[kk]-g1v10_im1)/(g1v10_re_sigma*g1v10_im_sigma);
      h1_g1v10_reim->Fill(rhoxy);
      rhoxy = (g1v1m1_re[kk]-g1v1m1_re1)*(g1v1m1_im[kk]-g1v1m1_im1)/(g1v1m1_re_sigma*g1v1m1_im_sigma);
      h1_g1v1m1_reim->Fill(rhoxy);

      h1_g1v00_reim_fit->Fill(g1v00_reim_fit[kk]);
      h1_g1v00s_reim_fit->Fill(g1v00s_reim_fit[kk]);
      h1_g1v11_reim_fit->Fill(g1v11_reim_fit[kk]);
      h1_g1v10_reim_fit->Fill(g1v10_reim_fit[kk]);
      h1_g1v1m1_reim_fit->Fill(g1v1m1_reim_fit[kk]);

    }

    TString text="";
    
    // plot histograms
    TCanvas *c1 = new TCanvas("c1", "c1",200,10,1500,1000);
    c1->Divide(5,4);

    c1->cd(1);
    h1_g1v00_re->Draw();
    h1_g1v00_re->Fit("gaus");
    TF1 *gaus = h1_g1v00_re->GetFunction("gaus");
    Double_t mean = gaus->GetParameter(1);
    Double_t sigma = gaus->GetParameter(2);
    cout << "mean=" << mean << " sigma=" << sigma << endl;
    text.Form("#sigma=%.2f",sigma);
    TLatex *t1 = new TLatex(0.2,0.8,text);
    t1->SetNDC();
    t1->Draw();
    Double_t rms = h1_g1v00_re->GetRMS();
    text.Form("rms=%.2f",rms);
    t1->DrawLatex(0.2,0.7,text);
    // overwrite the uncertainties with Bootstrap estimates
    parms_err[0] = rms;
    
    c1->cd(2);
    h1_g1v00s_re->Draw();
    h1_g1v00s_re->Fit("gaus");
    gaus = h1_g1v00s_re->GetFunction("gaus");
    mean = gaus->GetParameter(1);
    sigma = gaus->GetParameter(2);
    cout << "mean=" << mean << " sigma=" << sigma << endl;
    text.Form("#sigma=%.2f",sigma);
    t1->DrawLatex(0.2,0.8,text);
    rms = h1_g1v00s_re->GetRMS();
    text.Form("rms=%.2f",rms);
    t1->DrawLatex(0.2,0.7,text);
    // overwrite the uncertainties with Bootstrap estimates
    parms_err[2] = rms;
    
    c1->cd(3);
    h1_g1v11_re->Draw();
    h1_g1v11_re->Fit("gaus");
    gaus = h1_g1v11_re->GetFunction("gaus");
    mean = gaus->GetParameter(1);
    sigma = gaus->GetParameter(2);
    cout << "mean=" << mean << " sigma=" << sigma << endl;
    text.Form("#sigma=%.2f",sigma);
    t1->DrawLatex(0.2,0.8,text);
    rms = h1_g1v11_re->GetRMS();
    text.Form("rms=%.2f",rms);
    t1->DrawLatex(0.2,0.7,text);
    // overwrite the uncertainties with Bootstrap estimates
    parms_err[4] = rms;
    
    c1->cd(4);
    h1_g1v10_re->Draw();
    h1_g1v10_re->Fit("gaus");
    gaus = h1_g1v10_re->GetFunction("gaus");
    mean = gaus->GetParameter(1);
    sigma = gaus->GetParameter(2);
    cout << "mean=" << mean << " sigma=" << sigma << endl;
    text.Form("#sigma=%.2f",sigma);
    t1->DrawLatex(0.2,0.8,text);
    rms = h1_g1v10_re->GetRMS();
    text.Form("rms=%.2f",rms);
    t1->DrawLatex(0.2,0.7,text);
    // overwrite the uncertainties with Bootstrap estimates
    parms_err[6] = rms;
    
    c1->cd(5);
    h1_g1v1m1_re->Draw();
    h1_g1v1m1_re->Fit("gaus");
    gaus = h1_g1v1m1_re->GetFunction("gaus");
    mean = gaus->GetParameter(1);
    sigma = gaus->GetParameter(2);
    cout << "mean=" << mean << " sigma=" << sigma << endl;
    text.Form("#sigma=%.2f",sigma);
    t1->DrawLatex(0.2,0.8,text);
    rms = h1_g1v1m1_re->GetRMS();
    text.Form("rms=%.2f",rms);
    t1->DrawLatex(0.2,0.7,text);
    // overwrite the uncertainties with Bootstrap estimates
    parms_err[8] = rms;
    
    c1->cd(6);
    h1_g1v00_im->Draw();
    h1_g1v00_im->Fit("gaus");
    gaus = h1_g1v00_im->GetFunction("gaus");
    mean = gaus->GetParameter(1);
    sigma = gaus->GetParameter(2);
    cout << "mean=" << mean << " sigma=" << sigma << endl;
    text.Form("#sigma=%.2f",sigma);
    t1->DrawLatex(0.2,0.8,text);
    rms = h1_g1v00_im->GetRMS();
    text.Form("rms=%.2f",rms);
    t1->DrawLatex(0.2,0.7,text);
    // overwrite the uncertainties with Bootstrap estimates
    parms_err[1] = rms;
    
    c1->cd(7);
    h1_g1v00s_im->Draw();
    h1_g1v00s_im->Fit("gaus");
    gaus = h1_g1v00s_im->GetFunction("gaus");
    mean = gaus->GetParameter(1);
    sigma = gaus->GetParameter(2);
    cout << "mean=" << mean << " sigma=" << sigma << endl;
    text.Form("#sigma=%.2f",sigma);
    t1->DrawLatex(0.2,0.8,text);
    rms = h1_g1v00s_im->GetRMS();
    text.Form("rms=%.2f",rms);
    t1->DrawLatex(0.2,0.7,text);
    // overwrite the uncertainties with Bootstrap estimates
    parms_err[3] = rms;
    
    c1->cd(8);
    h1_g1v11_im->Draw();
    h1_g1v11_im->Fit("gaus");
    gaus = h1_g1v11_im->GetFunction("gaus");
    mean = gaus->GetParameter(1);
    sigma = gaus->GetParameter(2);
    cout << "mean=" << mean << " sigma=" << sigma << endl;
    text.Form("#sigma=%.2f",sigma);
    t1->DrawLatex(0.2,0.8,text);
    rms = h1_g1v11_im->GetRMS();
    text.Form("rms=%.2f",rms);
    t1->DrawLatex(0.2,0.7,text);
    // overwrite the uncertainties with Bootstrap estimates
    parms_err[5] = rms;
    
    c1->cd(9);
    h1_g1v10_im->Draw();
    h1_g1v10_im->Fit("gaus");
    gaus = h1_g1v10_im->GetFunction("gaus");
    mean = gaus->GetParameter(1);
    sigma = gaus->GetParameter(2);
    cout << "mean=" << mean << " sigma=" << sigma << endl;
    text.Form("#sigma=%.2f",sigma);
    t1->DrawLatex(0.2,0.8,text);
    rms = h1_g1v10_im->GetRMS();
    text.Form("rms=%.2f",rms);
    t1->DrawLatex(0.2,0.7,text);
    // overwrite the uncertainties with Bootstrap estimates
    parms_err[7] = rms;
    
    c1->cd(10);
    h1_g1v1m1_im->Draw();
    h1_g1v1m1_im->Fit("gaus");
    gaus = h1_g1v1m1_im->GetFunction("gaus");
    mean = gaus->GetParameter(1);
    sigma = gaus->GetParameter(2);
    cout << "mean=" << mean << " sigma=" << sigma << endl;
    text.Form("#sigma=%.2f",sigma);
    t1->DrawLatex(0.2,0.8,text);
    rms = h1_g1v1m1_im->GetRMS();
    text.Form("rms=%.2f",rms);
    t1->DrawLatex(0.2,0.7,text);
    // overwrite the uncertainties with Bootstrap estimates
    parms_err[9] = rms;
    
    c1->cd(11);
    h1_g1v00_reim->Draw();
    /*h1_g1v00_reim->Fit("gaus");
    gaus = h1_g1v00_reim->GetFunction("gaus");
    mean = gaus->GetParameter(1);
    sigma = gaus->GetParameter(2);
    cout << "mean=" << mean << " sigma=" << sigma << endl;
    text.Form("#mu=%.2f",mean);
    text.Form("#sigma=%.2f",sigma);
    t1->DrawLatex(0.2,0.8,text);*/
    mean = 0;
    parms_ij.push_back(mean*g1v00_re_sigma*g1v00_im_sigma);
    
    c1->cd(12);
    h1_g1v00s_reim->Draw();
    mean = h1_g1v00s_reim->GetMean();
    rms = h1_g1v00s_reim->GetRMS();
    // h1_g1v00s_reim->Fit("gaus");
    // gaus = h1_g1v00s_reim->GetFunction("gaus");
    // mean = gaus->GetParameter(1);
    // sigma = gaus->GetParameter(2);
    cout << "mean=" << mean << " rms=" << rms << endl;
    text.Form("#rho_{xy}=%.2f",mean);
    // text.Form("#sigma=%.2f",sigma);
    parms_ij.push_back(mean*g1v00s_re_sigma*g1v00s_im_sigma);
    t1->DrawLatex(0.2,0.8,text);
    text.Form("V_{xy}=%.2f",mean*g1v00s_re_sigma*g1v00s_im_sigma);
    t1->DrawLatex(0.2,0.70,text);
    
    c1->cd(13);
    h1_g1v11_reim->Draw();
    mean = h1_g1v11_reim->GetMean();
    rms = h1_g1v11_reim->GetRMS();
    text.Form("#rho_{xy}=%.2f",mean);
    // h1_g1v11_reim->Fit("gaus");
    // gaus = h1_g1v11_reim->GetFunction("gaus");
    // mean = gaus->GetParameter(1);
    sigma = gaus->GetParameter(2);
    cout << "mean=" << mean << " rms=" << rms << endl;
    text.Form("#rho_{xy}=%.2f",mean);
    // text.Form("#sigma=%.2f",sigma);
    parms_ij.push_back(mean*g1v11_re_sigma*g1v11_im_sigma);
    t1->DrawLatex(0.2,0.8,text);
    text.Form("V_{xy}=%.2f",mean*g1v11_re_sigma*g1v11_im_sigma);
    t1->DrawLatex(0.2,0.70,text);
    
    c1->cd(14);
    h1_g1v10_reim->Draw();
    mean = h1_g1v10_reim->GetMean();
    rms = h1_g1v10_reim->GetRMS();
    text.Form("#rho_{xy}=%.2f",mean);
    // h1_g1v10_reim->Fit("gaus");
    // gaus = h1_g1v10_reim->GetFunction("gaus");
    // mean = gaus->GetParameter(1);
    sigma = gaus->GetParameter(2);
    cout << "mean=" << mean << " rms=" << rms << endl;
    text.Form("#rho_{xy}=%.2f",mean);
    // text.Form("#sigma=%.2f",sigma);
    parms_ij.push_back(mean*g1v10_re_sigma*g1v10_im_sigma);
    t1->DrawLatex(0.2,0.8,text);
    text.Form("V_{xy}=%.2f",mean*g1v10_re_sigma*g1v10_im_sigma);
    t1->DrawLatex(0.2,0.70,text);
    
    c1->cd(15);
    h1_g1v1m1_reim->Draw();
    mean = h1_g1v1m1_reim->GetMean();
    rms = h1_g1v1m1_reim->GetRMS();
    text.Form("#rho_{xy}=%.2f",mean);
    // h1_g1v1m1_reim->Fit("gaus");
    // gaus = h1_g1v1m1_reim->GetFunction("gaus");
    // mean = gaus->GetParameter(1);
    sigma = gaus->GetParameter(2);
    cout << "mean=" << mean << " rms=" << rms << endl;
    text.Form("#rho_{xy}=%.2f",mean);
    // text.Form("#sigma=%.2f",sigma);
    parms_ij.push_back(mean*g1v1m1_re_sigma*g1v1m1_im_sigma);
    t1->DrawLatex(0.2,0.8,text);
    text.Form("V_{xy}=%.2f",mean*g1v1m1_re_sigma*g1v1m1_im_sigma);
    t1->DrawLatex(0.2,0.70,text);


    c1->cd(16);
    h1_g1v00_reim_fit->Draw();

    c1->cd(17);
    h1_g1v00s_reim_fit->Draw();
    mean = h1_g1v00s_reim_fit->GetMean();
    text.Form("V_{xy}=%.2f",mean);
    t1->DrawLatex(0.2,0.70,text);

    c1->cd(18);
    h1_g1v11_reim_fit->Draw();
    mean = h1_g1v11_reim_fit->GetMean();
    text.Form("V_{xy}=%.2f",mean);
    t1->DrawLatex(0.2,0.70,text);

    c1->cd(19);
    h1_g1v10_reim_fit->Draw();
    mean = h1_g1v10_reim_fit->GetMean();
    text.Form("V_{xy}=%.2f",mean);
    t1->DrawLatex(0.2,0.70,text);

    c1->cd(20);
    h1_g1v1m1_reim_fit->Draw();
    mean = h1_g1v1m1_reim_fit->GetMean();
    text.Form("V_{xy}=%.2f",mean);
    t1->DrawLatex(0.2,0.70,text);



    // plot histograms
    TCanvas *c2 = new TCanvas("c2", "c2",200,10,1500,1000);
    c2->Divide(5,4);

    c2->cd(1);
    h1_g1v00_re->Draw();
    h1_g1v00_re->Fit("gaus");
    gaus = h1_g1v00_re->GetFunction("gaus");
    mean = gaus->GetParameter(1);
    sigma = gaus->GetParameter(2);
    cout << "mean=" << mean << " sigma=" << sigma << endl;
    text.Form("#sigma=%.2f",sigma);
    t1->DrawLatex(0.2,0.8,text);
    rms = h1_g1v00_re->GetRMS();
    text.Form("rms=%.2f",rms);
    t1->DrawLatex(0.2,0.7,text);
    
    c2->cd(2);
    h1_g1v00s_re->Draw();
    h1_g1v00s_re->Fit("gaus");
    gaus = h1_g1v00s_re->GetFunction("gaus");
    mean = gaus->GetParameter(1);
    sigma = gaus->GetParameter(2);
    cout << "mean=" << mean << " sigma=" << sigma << endl;
    text.Form("#sigma=%.2f",sigma);
    t1->DrawLatex(0.2,0.8,text);
    rms = h1_g1v00s_re->GetRMS();
    text.Form("rms=%.2f",rms);
    t1->DrawLatex(0.2,0.7,text);
    
    c2->cd(3);
    h1_g1v11_re->Draw();
    h1_g1v11_re->Fit("gaus");
    gaus = h1_g1v11_re->GetFunction("gaus");
    mean = gaus->GetParameter(1);
    sigma = gaus->GetParameter(2);
    cout << "mean=" << mean << " sigma=" << sigma << endl;
    text.Form("#sigma=%.2f",sigma);
    t1->DrawLatex(0.2,0.8,text);
    rms = h1_g1v11_re->GetRMS();
    text.Form("rms=%.2f",rms);
    t1->DrawLatex(0.2,0.7,text);
    
    c2->cd(4);
    h1_g1v10_re->Draw();
    h1_g1v10_re->Fit("gaus");
    gaus = h1_g1v10_re->GetFunction("gaus");
    mean = gaus->GetParameter(1);
    sigma = gaus->GetParameter(2);
    cout << "mean=" << mean << " sigma=" << sigma << endl;
    text.Form("#sigma=%.2f",sigma);
    t1->DrawLatex(0.2,0.8,text);
    rms = h1_g1v10_re->GetRMS();
    text.Form("rms=%.2f",rms);
    t1->DrawLatex(0.2,0.7,text);
    
    c2->cd(5);
    h1_g1v1m1_re->Draw();
    h1_g1v1m1_re->Fit("gaus");
    gaus = h1_g1v1m1_re->GetFunction("gaus");
    mean = gaus->GetParameter(1);
    sigma = gaus->GetParameter(2);
    cout << "mean=" << mean << " sigma=" << sigma << endl;
    text.Form("#sigma=%.2f",sigma);
    t1->DrawLatex(0.2,0.8,text);
    rms = h1_g1v1m1_re->GetRMS();
    text.Form("rms=%.2f",rms);
    t1->DrawLatex(0.2,0.7,text);

    c2->cd(6);
    h1_g1v00_re_err->Draw();
    mean = h1_g1v00_re_err->GetMean();
    text.Form("V_{xy}=%.2f",mean);
    t1->DrawLatex(0.2,0.70,text);

    c2->cd(7);
    h1_g1v00s_re_err->Draw();
    mean = h1_g1v00s_re_err->GetMean();
    text.Form("V_{xy}=%.2f",mean);
    t1->DrawLatex(0.2,0.70,text);

    c2->cd(8);
    h1_g1v11_re_err->Draw();
    mean = h1_g1v11_re_err->GetMean();
    text.Form("V_{xy}=%.2f",mean);
    t1->DrawLatex(0.2,0.70,text);

    c2->cd(9);
    h1_g1v10_re_err->Draw();
    mean = h1_g1v10_re_err->GetMean();
    text.Form("V_{xy}=%.2f",mean);
    t1->DrawLatex(0.2,0.70,text);

    c2->cd(10);
    h1_g1v1m1_re_err->Draw();
    mean = h1_g1v1m1_re_err->GetMean();
    text.Form("V_{xy}=%.2f",mean);
    t1->DrawLatex(0.2,0.70,text);
    
    c2->cd(11);
    h1_g1v00_im->Draw();
    h1_g1v00_im->Fit("gaus");
    gaus = h1_g1v00_im->GetFunction("gaus");
    mean = gaus->GetParameter(1);
    sigma = gaus->GetParameter(2);
    cout << "mean=" << mean << " sigma=" << sigma << endl;
    text.Form("#sigma=%.2f",sigma);
    t1->DrawLatex(0.2,0.8,text);
    rms = h1_g1v00_im->GetRMS();
    text.Form("rms=%.2f",rms);
    t1->DrawLatex(0.2,0.7,text);
    
    c2->cd(12);
    h1_g1v00s_im->Draw();
    h1_g1v00s_im->Fit("gaus");
    gaus = h1_g1v00s_im->GetFunction("gaus");
    mean = gaus->GetParameter(1);
    sigma = gaus->GetParameter(2);
    cout << "mean=" << mean << " sigma=" << sigma << endl;
    text.Form("#sigma=%.2f",sigma);
    t1->DrawLatex(0.2,0.8,text);
    rms = h1_g1v00s_im->GetRMS();
    text.Form("rms=%.2f",rms);
    t1->DrawLatex(0.2,0.7,text);
    
    c2->cd(13);
    h1_g1v11_im->Draw();
    h1_g1v11_im->Fit("gaus");
    gaus = h1_g1v11_im->GetFunction("gaus");
    mean = gaus->GetParameter(1);
    sigma = gaus->GetParameter(2);
    cout << "mean=" << mean << " sigma=" << sigma << endl;
    text.Form("#sigma=%.2f",sigma);
    t1->DrawLatex(0.2,0.8,text);
    rms = h1_g1v11_im->GetRMS();
    text.Form("rms=%.2f",rms);
    t1->DrawLatex(0.2,0.7,text);
    
    c2->cd(14);
    h1_g1v10_im->Draw();
    h1_g1v10_im->Fit("gaus");
    gaus = h1_g1v10_im->GetFunction("gaus");
    mean = gaus->GetParameter(1);
    sigma = gaus->GetParameter(2);
    cout << "mean=" << mean << " sigma=" << sigma << endl;
    text.Form("#sigma=%.2f",sigma);
    t1->DrawLatex(0.2,0.8,text);
    rms = h1_g1v10_im->GetRMS();
    text.Form("rms=%.2f",rms);
    t1->DrawLatex(0.2,0.7,text);
    
    c2->cd(15);
    h1_g1v1m1_im->Draw();
    h1_g1v1m1_im->Fit("gaus");
    gaus = h1_g1v1m1_im->GetFunction("gaus");
    mean = gaus->GetParameter(1);
    sigma = gaus->GetParameter(2);
    cout << "mean=" << mean << " sigma=" << sigma << endl;
    text.Form("#sigma=%.2f",sigma);
    t1->DrawLatex(0.2,0.8,text);
    rms = h1_g1v1m1_im->GetRMS();
    text.Form("rms=%.2f",rms);
    t1->DrawLatex(0.2,0.7,text);

    c2->cd(16);
    h1_g1v00_im_err->Draw();
    /*mean = h1_g1v00_im_err->GetMean();
    text.Form("V_{xy}=%.2f",mean);
    t1->DrawLatex(0.2,0.70,text);*/

    c2->cd(17);
    h1_g1v00s_im_err->Draw();
    mean = h1_g1v00s_im_err->GetMean();
    text.Form("V_{xy}=%.2f",mean);
    t1->DrawLatex(0.2,0.70,text);

    c2->cd(18);
    h1_g1v11_im_err->Draw();
    mean = h1_g1v11_im_err->GetMean();
    text.Form("V_{xy}=%.2f",mean);
    t1->DrawLatex(0.2,0.70,text);

    c2->cd(19);
    h1_g1v10_im_err->Draw();
    mean = h1_g1v10_im_err->GetMean();
    text.Form("V_{xy}=%.2f",mean);
    t1->DrawLatex(0.2,0.70,text);

    c2->cd(20);
    h1_g1v1m1_im_err->Draw();
    mean = h1_g1v1m1_im_err->GetMean();
    text.Form("V_{xy}=%.2f",mean);
    t1->DrawLatex(0.2,0.70,text);
    




    // output file with boostrap uncertainties

    TString outfile;
    ofstream outparms;

    outfile.Form("%s_out.fit2",filename0.Data());   // file with bootstrap uncertainties
    cout << "*** Starting to write parameters to " << outfile.Data() << endl << endl;
    outparms.open (outfile.Data());
      
    outparms << endl;

    for (Int_t k=0; k<ntokens_save/3; k++){
	outparms << sdme[k] << "\t" << parms[k] << "\t" << parms_err[k] << "\t";
    }
    outparms << endl;

    Int_t nparms=10;
    vector <Double_t> covrow;


    for (Int_t k=0; k<nparms; k++){
      covrow.erase(covrow.begin(),covrow.end());
      for (Int_t j=0; j<nparms; j++){
	if (k == j) {
	  covrow.push_back(parms_err[j]*parms_err[j]);
	}
	else if (k%2==0 && k+1 == j)
	  covrow.push_back(parms_ij[k/2]);
	else {
	  covrow.push_back(0.);
	}
      }
      for (Int_t j=0; j<nparms; j++){
	outparms.width(20);
	outparms << covrow[j];
	// outparms.width(20);
	// outparms << parms_err[j];
      }
      outparms << endl;
    }

    outparms << endl;

    outparms.close();
    

    c1->SaveAs("bootstrap.pdf(");
    c2->SaveAs("bootstrap.pdf)");
}
