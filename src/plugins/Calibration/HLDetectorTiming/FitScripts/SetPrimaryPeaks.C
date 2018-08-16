namespace ExtractTDCADCTimingNS {
TFile * thisFile;

TH1I * Get1DHistogram(const char * plugin, const char * directoryName, const char * name, bool print = true){
    TH1I * histogram;
    TString fullName = TString(plugin) + "/" + TString(directoryName) + "/" + TString(name);
    thisFile->GetObject(fullName, histogram);
    if (histogram == 0){
        if (print) cout << "Unable to find histogram " << fullName.Data() << endl;
        return NULL;
    }
    return histogram;
}

TH2I * Get2DHistogram(const char * plugin, const char * directoryName, const char * name){
    TH2I * histogram;
    TString fullName = TString(plugin) + "/" + TString(directoryName) + "/" + TString(name);
    thisFile->GetObject(fullName, histogram);
    if (histogram == 0){
        cout << "Unable to find histogram " << fullName.Data() << endl;
        return NULL;
    }
    return histogram;
}
};

void GetCCDBConstants(TString path, Int_t run, TString variation, vector<double>& container, Int_t column = 1){
    char command[256];
    sprintf(command, "ccdb dump %s:%i:%s", path.Data(), run, variation.Data());
    FILE* inputPipe = gSystem->OpenPipe(command, "r");
    if(inputPipe == NULL)
        return;
    //get the first (comment) line
    char buff[1024];
    if(fgets(buff, sizeof(buff), inputPipe) == NULL)
        return;
    //get the remaining lines
    double entry;
    int counter = 0;
    while(fgets(buff, sizeof(buff), inputPipe) != NULL){
        istringstream locConstantsStream(buff);
        while (locConstantsStream >> entry){
            counter++;
            if (counter % column == 0) container.push_back(entry);
        }
    }
    //Close the pipe
    gSystem->ClosePipe(inputPipe);
}
//Overload this function to handle the base time offsets
void GetCCDBConstants1(TString path, Int_t run, TString variation, double& constant1){
    char command[256];
    sprintf(command, "ccdb dump %s:%i:%s", path.Data(), run, variation.Data());
    FILE* inputPipe = gSystem->OpenPipe(command, "r");
    if(inputPipe == NULL)
        return;
    //get the first (comment) line
    char buff[1024];
    if(fgets(buff, sizeof(buff), inputPipe) == NULL)
        return;
    //get the line containing the values
    while(fgets(buff, sizeof(buff), inputPipe) != NULL){
        istringstream locConstantsStream(buff);
	locConstantsStream >> constant1;
    }
    //Close the pipe
    gSystem->ClosePipe(inputPipe);
}

void GetCCDBConstants2(TString path, Int_t run, TString variation, double& constant1, double& constant2){
    char command[256];
    sprintf(command, "ccdb dump %s:%i:%s", path.Data(), run, variation.Data());
    FILE* inputPipe = gSystem->OpenPipe(command, "r");
    if(inputPipe == NULL)
        return;
    //get the first (comment) line
    char buff[1024];
    if(fgets(buff, sizeof(buff), inputPipe) == NULL)
        return;
    //get the line containing the values
    while(fgets(buff, sizeof(buff), inputPipe) != NULL){
        istringstream locConstantsStream(buff);
	locConstantsStream >> constant1 >> constant2;
    }
    //Close the pipe
    gSystem->ClosePipe(inputPipe);
}

int GetCCDBIndexTAGM(int column, int row){
    int CCDBIndex = column + row;
    if (column > 9) CCDBIndex += 5;
    if (column > 27) CCDBIndex += 5;
    if (column > 81) CCDBIndex += 5;
    if (column > 99) CCDBIndex += 5;

    return CCDBIndex;
}

int GetF1TDCslotTAGH(int id) {
    double N = 32.0; // channels per slot
    if (id >= 132 && id <= 172) throw("TAGH: unknown id in [132,172]");
    int HVid = (id <= 131) ? id : (id - 274 + 233);
    return int((HVid-1)/N) + 1;
}

Double_t FitFunctionLeft(Double_t *x, Double_t *par)
{
    Float_t xx =x[0];
    Double_t f = par[0]*TMath::Exp(-0.5 * (TMath::Power((xx - par[1]) / par[2] , 2) ) )+ par[0]*TMath::Exp(-0.5 * (TMath::Power((xx - par[1] - par[3]) / par[2] , 2) ) );
    return f;
}

Double_t FitFunctionRight(Double_t *x, Double_t *par)
{
    Float_t xx =x[0];
    Double_t f = par[0]*TMath::Exp(-0.5 * (TMath::Power((xx - par[1]) / par[2] , 2) ) )+ par[0]*TMath::Exp(-0.5 * (TMath::Power((xx - par[1] + par[3]) / par[2] , 2) ) );
    return f;
}

void SetPrimaryPeaks(TString fileName = "hd_root.root", int runNumber = 10390, TString variation = "default", TString prefix = ""){

    // set "prefix" in case you want to think about thowing the text files into another directory
    cout << "Performing TDC/ADC timing fits for File: " << fileName.Data() << " Run: " << runNumber << " Variation: " << variation.Data() << endl;

    ExtractTDCADCTimingNS::thisFile = TFile::Open( fileName , "UPDATE");
    if (ExtractTDCADCTimingNS::thisFile == 0) {
        cout << "Unable to open file " << fileName.Data() << "...Exiting" << endl;
        return;
    }
    ofstream outFile;

	// If we've already aligned ADC's and TDC's, say from an earlier run,
	// realign detectors using ADC/TDC matched hits to preserve the alignment
	// otherwise, align them separately
	bool AssumeADCTDCAlignment = true;  

	//Setpoints - assume SC peak set to 0, may need to adjust in the future
	double nominalFCALTime = 17.;
	double nominalBCALADCTime = 15.;
	double nominalBCALTDCTime = 15.;
    double nominalSCTime = 0.;
    double nominalTOFTime = 21.;
	double nominalCDCTime = 6.;
	double nominalFDCCathodeTime = 14.;
	double nominalFDCWireTime = 5.5;
    double nominalTAGHTime = -5.;
    double nominalTAGMTime = -4.;	


    // We need to grab the existing values from the CCDB so that we know what was used in the creation of the ROOT file
    cout << "Grabbing CCDB constants..." << endl;

    // First are the base times for all of the detectors
    double cdc_base_time,      fcal_base_time;
    double sc_base_time_tdc,   sc_base_time_adc;
    double fdc_base_time_tdc,  fdc_base_time_adc;
    double bcal_base_time_tdc, bcal_base_time_adc;
    double tagm_base_time_tdc, tagm_base_time_adc;
    double tagh_base_time_tdc, tagh_base_time_adc;
    double tof_base_time_tdc,  tof_base_time_adc;

    double beam_period;

    GetCCDBConstants1("/CDC/base_time_offset" ,runNumber, variation, cdc_base_time);
    GetCCDBConstants1("/FCAL/base_time_offset",runNumber, variation, fcal_base_time);
    GetCCDBConstants1("/PHOTON_BEAM/RF/beam_period",runNumber, variation, beam_period);
    GetCCDBConstants2("/FDC/base_time_offset" ,runNumber, variation, fdc_base_time_adc, fdc_base_time_tdc);
    GetCCDBConstants2("/BCAL/base_time_offset" ,runNumber, variation, bcal_base_time_adc, bcal_base_time_tdc);
    GetCCDBConstants2("/PHOTON_BEAM/microscope/base_time_offset" ,runNumber, variation, tagm_base_time_adc, tagm_base_time_tdc);
    GetCCDBConstants2("/PHOTON_BEAM/hodoscope/base_time_offset" ,runNumber, variation, tagh_base_time_adc, tagh_base_time_tdc);
    GetCCDBConstants2("/START_COUNTER/base_time_offset" ,runNumber, variation, sc_base_time_adc, sc_base_time_tdc);
    GetCCDBConstants2("/TOF/base_time_offset" ,runNumber, variation, tof_base_time_adc, tof_base_time_tdc);

    cout << "CDC base times = " << cdc_base_time << endl;
    cout << "FCAL base times = " << fcal_base_time << endl;
    cout << "FDC base times = " << fdc_base_time_adc << ", " << fdc_base_time_tdc << endl;
    cout << "BCAL base times = " << bcal_base_time_adc << ", " << bcal_base_time_tdc << endl;
    cout << "SC base times = " << sc_base_time_adc << ", " << sc_base_time_tdc << endl;
    cout << "TOF base times = " << tof_base_time_adc << ", " << tof_base_time_tdc << endl;
    cout << "TAGH base times = " << tagh_base_time_adc << ", " << tagh_base_time_tdc << endl;
    cout << "TAGM base times = " << tagm_base_time_adc << ", " << tagm_base_time_tdc << endl;

    cout << "beam_period = " << beam_period << endl;

    cout << "Done grabbing CCDB constants...Entering fits..." << endl;

    //Move the base times

	// CDC
    float CDC_ADC_Offset = 0.0;
    TH1I * this1DHist = ExtractTDCADCTimingNS::Get1DHistogram("HLDetectorTiming", "CDC", "CDCHit time");
    if(this1DHist != NULL){
        Int_t firstBin = this1DHist->FindLastBinAbove( 1 , 1); // Find first bin with content above 1 in the histogram
        for (int i = 0; i <= 25; i++){
            if ((firstBin + i) > 0) this1DHist->SetBinContent((firstBin + i), 0);
        }
        //Fit a gaussian to the left of the main peak
        Double_t maximum = this1DHist->GetBinCenter(this1DHist->GetMaximumBin());
        TF1 *f = new TF1("f", "gaus");
        f->SetParameters(100, maximum, 20);
        f->FixParameter(1 , maximum);
        TFitResultPtr fr = this1DHist->Fit(f, "S", "", maximum - 20, maximum + 20); // Cant fix value at end of range
        double mean = fr->Parameter(1);
        float sigma = fr->Parameter(2);
        CDC_ADC_Offset = mean - sigma;
        delete f;
    }

    outFile.open(prefix + "cdc_base_time.txt");
    outFile << cdc_base_time - (CDC_ADC_Offset-nominalCDCTime) << endl;
    outFile.close();

	// FDC
    float FDC_ADC_Offset = 0.0, FDC_TDC_Offset = 0.0;
    this1DHist = ExtractTDCADCTimingNS::Get1DHistogram("HLDetectorTiming", "FDC", "FDCHit Cathode time");
    if(this1DHist != NULL){
        Int_t firstBin = this1DHist->FindFirstBinAbove( 1 , 1); // Find first bin with content above 1 in the histogram
        for (int i = 0; i <= 30; i++){
            if ((firstBin + i) > 0) this1DHist->SetBinContent((firstBin + i), 0);
        }
        //Fit a gaussian to the left of the main peak
        Double_t maximum = this1DHist->GetBinCenter(this1DHist->GetMaximumBin());
        TF1 *f = new TF1("f", "gaus");
        f->SetParameters(100, maximum, 20);
        f->FixParameter(1 , maximum);
        TFitResultPtr fr = this1DHist->Fit(f, "S", "", maximum - 25, maximum + 20); // Cant fix value at end of range
        double mean = fr->Parameter(1);
        float sigma = fr->Parameter(2);
        FDC_ADC_Offset = mean;
        delete f;
    }
    this1DHist = ExtractTDCADCTimingNS::Get1DHistogram("HLDetectorTiming", "FDC", "FDCHit Wire time");
    if(this1DHist != NULL){
        Int_t firstBin = this1DHist->FindLastBinAbove( 1 , 1); // Find first bin with content above 1 in the histogram
        for (int i = 0; i <= 25; i++){
            if ((firstBin + i) > 0) this1DHist->SetBinContent((firstBin + i), 0);
        }
        //Fit a gaussian to the left of the main peak
        Double_t maximum = this1DHist->GetBinCenter(this1DHist->GetMaximumBin());
        TF1 *f = new TF1("f", "gaus");
        f->SetParameters(10000, maximum, 30);
        //f->FixParameter(1 , maximum);
        TFitResultPtr fr = this1DHist->Fit(f, "S", "", maximum - 18, maximum + 6); // Cant fix value at end of range
        double mean = fr->Parameter(1);
        float sigma = fr->Parameter(2);
        FDC_TDC_Offset = mean;
        delete f;
    }

    outFile.open(prefix + "fdc_base_time.txt");
    outFile << fdc_base_time_adc - (FDC_ADC_Offset-nominalFDCCathodeTime) 
            << " " << fdc_base_time_tdc - (FDC_TDC_Offset-nominalFDCWireTime) << endl;
    outFile.close();

    // Now that we have the file open, do all of the fits and write the output
    // Fit all plots with expected funtional form, output files for CCDB input
    int minHits = 7;

    // In order to calibrate the SC in one step, we need to work with the base times and with the TDC/ADC offsets at the same time
    // Sort of complicates things but saves considerable time.

    float SC_ADC_Offset = 0.0;
    float SC_TDC_Offset = 0.0;
    float TOF_ADC_Offset = 0.0;
    float TOF_TDC_Offset = 0.0;
    float FCAL_Offset = 0.0;
    float BCAL_ADC_Offset = 0.0;   
    float BCAL_TDC_Offset = 0.0;   // handle TDC's differently some day
    float TAGH_ADC_Offset = 0.0;
    float TAGH_TDC_Offset = 0.0;
    float TAGM_ADC_Offset = 0.0;
    float TAGM_TDC_Offset = 0.0;

	// FCAL
    this1DHist = ExtractTDCADCTimingNS::Get1DHistogram("HLDetectorTiming", "FCAL", "FCALHit time");
    if(this1DHist != NULL){
        //Gaussian
        Double_t maximum = this1DHist->GetBinCenter(this1DHist->GetMaximumBin());
        TFitResultPtr fr = this1DHist->Fit("gaus", "S", "", maximum - 5, maximum + 5);
        double mean = fr->Parameter(1);
        FCAL_Offset = mean;
    }
    outFile.open(prefix + "fcal_base_time.txt");
    outFile << fcal_base_time - (FCAL_Offset-nominalFCALTime) << endl;
    outFile.close();
    
	// BCAL
	this1DHist = ExtractTDCADCTimingNS::Get1DHistogram("HLDetectorTiming", "BCAL", "BCALHit ADC time");
	if(this1DHist != NULL){
		//Gaussian
		Double_t maximum = this1DHist->GetBinCenter(this1DHist->GetMaximumBin());
		TFitResultPtr fr = this1DHist->Fit("gaus", "S", "", maximum - 7, maximum + 7);
		double mean = fr->Parameter(1);
		BCAL_ADC_Offset = mean;
	}
	this1DHist = ExtractTDCADCTimingNS::Get1DHistogram("HLDetectorTiming", "BCAL", "BCALHit TDC time");
	if(this1DHist != NULL){
		//Gaussian
		Double_t maximum = this1DHist->GetBinCenter(this1DHist->GetMaximumBin());
		TFitResultPtr fr = this1DHist->Fit("gaus", "S", "", maximum - 7, maximum + 7);
		double mean = fr->Parameter(1);
		BCAL_TDC_Offset = mean;
	}
	outFile.open(prefix + "bcal_base_time.txt");
	outFile << bcal_base_time_adc - (BCAL_ADC_Offset-nominalBCALADCTime) 
			<< " " << bcal_base_time_tdc - (BCAL_TDC_Offset-nominalBCALTDCTime) << endl;
	outFile.close();

	// SC, TOF, TAGH, TAGM
	if(AssumeADCTDCAlignment) {
		double SC_Offset = 0.;
		double TOF_Offset = 0.;
		double TAGM_Offset = 0.;
		double TAGH_Offset = 0.;

		// START COUNTER
		this1DHist = ExtractTDCADCTimingNS::Get1DHistogram("HLDetectorTiming", "SC", "SCHit Matched time");
		if(this1DHist != NULL){
			//Gaussian
			Double_t maximum = this1DHist->GetBinCenter(this1DHist->GetMaximumBin());
			TFitResultPtr fr = this1DHist->Fit("gaus", "S", "", maximum - 5, maximum + 5);
			double mean = fr->Parameter(1);
			SC_Offset = mean;
		}
		outFile.open(prefix + "sc_base_time.txt");
		outFile << sc_base_time_adc - (SC_Offset-nominalSCTime) 
				<< " " << sc_base_time_tdc - (SC_Offset-nominalSCTime) << endl;
		outFile.close();

		// TOF
		this1DHist = ExtractTDCADCTimingNS::Get1DHistogram("HLDetectorTiming", "TOF", "TOFHit Matched time");
		if(this1DHist != NULL){
			Int_t firstBin = this1DHist->FindFirstBinAbove(1 , 1); // Find first bin with content above 1 in the histogram
			for (int i = 0; i <= 20; i++){
				if ((firstBin + i) > 0) this1DHist->SetBinContent((firstBin + i), 0);
			}
			//Gaussian
			Double_t maximum = this1DHist->GetBinCenter(this1DHist->GetMaximumBin());
			TFitResultPtr fr = this1DHist->Fit("gaus", "S", "", maximum - 4, maximum + 4);
			double mean = fr->Parameter(1);
			TOF_Offset = mean;
		}
		outFile.open(prefix + "tof_base_time.txt");
		outFile << tof_base_time_adc - (TOF_Offset-nominalTOFTime) 
				<< " " << tof_base_time_tdc - (TOF_Offset-nominalTOFTime) << endl;
		outFile.close();

		// TAGH
		this1DHist = ExtractTDCADCTimingNS::Get1DHistogram("HLDetectorTiming", "TAGH", "TAGHHit Matched time");
		if(this1DHist != NULL){
			//Gaussian
			Double_t maximum = this1DHist->GetBinCenter(this1DHist->GetMaximumBin());
			TFitResultPtr fr = this1DHist->Fit("gaus", "S", "", maximum - 10, maximum + 10);
			double mean = fr->Parameter(1);
			TAGH_Offset = mean;
		}
		outFile.open(prefix + "tagh_base_time.txt");
		outFile << tagh_base_time_adc - (TAGH_Offset-nominalTAGHTime) 
				<< " " << tagh_base_time_tdc - (TAGH_Offset-nominalTAGHTime) << endl;
		outFile.close();

		// TAGM
		this1DHist = ExtractTDCADCTimingNS::Get1DHistogram("HLDetectorTiming", "TAGM", "TAGMHit Matched time");
		if(this1DHist != NULL){
			Int_t firstBin = this1DHist->FindFirstBinAbove( 1 , 1); // Find first bin with content above 1 in the histogram
			for (int i = 0; i <= 100; i++){
				if ((firstBin + i) > 0) this1DHist->SetBinContent((firstBin + i), 0);
			}
		   //Gaussian
			Double_t maximum = this1DHist->GetBinCenter(this1DHist->GetMaximumBin());
			TFitResultPtr fr = this1DHist->Fit("gaus", "S", "", maximum - 10, maximum + 8);
			double mean = fr->Parameter(1);
			TAGM_Offset = mean;
		}
		outFile.open(prefix + "tagm_base_time.txt");
		outFile << tagm_base_time_adc - (TAGM_Offset-nominalTAGMTime) 
				<< " " << tagm_base_time_tdc - (TAGM_Offset-nominalTAGMTime) << endl;
		outFile.close();
		
	} else { // AssumeADCTDCAlign
	
		// START COUNTER
		this1DHist = ExtractTDCADCTimingNS::Get1DHistogram("HLDetectorTiming", "SC", "SCHit ADC time");
		if(this1DHist != NULL){
			//Gaussian
			Double_t maximum = this1DHist->GetBinCenter(this1DHist->GetMaximumBin());
			TFitResultPtr fr = this1DHist->Fit("gaus", "S", "", maximum - 5, maximum + 5);
			double mean = fr->Parameter(1);
			SC_ADC_Offset = mean;
		}
		this1DHist = ExtractTDCADCTimingNS::Get1DHistogram("HLDetectorTiming", "SC", "SCHit TDC time");
		if(this1DHist != NULL){
			//Gaussian
			Double_t maximum = this1DHist->GetBinCenter(this1DHist->GetMaximumBin());
			TFitResultPtr fr = this1DHist->Fit("gaus", "S", "", maximum - 5, maximum + 5);
			double mean = fr->Parameter(1);
			SC_TDC_Offset = mean;
		}
		outFile.open(prefix + "sc_base_time.txt");
		outFile << sc_base_time_adc - (SC_ADC_Offset-nominalSCTime) 
				<< " " << sc_base_time_tdc - (SC_TDC_Offset-nominalSCTime) << endl;
		outFile.close();

		// TOF
		this1DHist = ExtractTDCADCTimingNS::Get1DHistogram("HLDetectorTiming", "TOF", "TOFHit ADC time");
		if(this1DHist != NULL){
			Int_t firstBin = this1DHist->FindFirstBinAbove(1 , 1); // Find first bin with content above 1 in the histogram
			for (int i = 0; i <= 20; i++){
				if ((firstBin + i) > 0) this1DHist->SetBinContent((firstBin + i), 0);
			}
			//Gaussian
			Double_t maximum = this1DHist->GetBinCenter(this1DHist->GetMaximumBin());
			TFitResultPtr fr = this1DHist->Fit("gaus", "S", "", maximum - 4, maximum + 4);
			double mean = fr->Parameter(1);
			TOF_ADC_Offset = mean;
		}
		this1DHist = ExtractTDCADCTimingNS::Get1DHistogram("HLDetectorTiming", "TOF", "TOFHit TDC time");
		if(this1DHist != NULL){
			//Gaussian
			Double_t maximum = this1DHist->GetBinCenter(this1DHist->GetMaximumBin());
			TFitResultPtr fr = this1DHist->Fit("gaus", "S", "", maximum - 4, maximum + 4);
			double mean = fr->Parameter(1);
			TOF_TDC_Offset = mean;
		}
		outFile.open(prefix + "tof_base_time.txt");
		outFile << tof_base_time_adc - (TOF_ADC_Offset-nominalTOFTime) 
				<< " " << tof_base_time_tdc - (TOF_TDC_Offset-nominalTOFTime) << endl;
		outFile.close();

		// TAGH
		this1DHist = ExtractTDCADCTimingNS::Get1DHistogram("HLDetectorTiming", "TAGH", "TAGHHit ADC time");
		if(this1DHist != NULL){
			//Gaussian
			Double_t maximum = this1DHist->GetBinCenter(this1DHist->GetMaximumBin());
			TFitResultPtr fr = this1DHist->Fit("gaus", "S", "", maximum - 10, maximum + 10);
			double mean = fr->Parameter(1);
			TAGH_ADC_Offset = mean;
		}
		this1DHist = ExtractTDCADCTimingNS::Get1DHistogram("HLDetectorTiming", "TAGH", "TAGHHit TDC time");
		if(this1DHist != NULL){
			//Gaussian
			Double_t maximum = this1DHist->GetBinCenter(this1DHist->GetMaximumBin());
			TFitResultPtr fr = this1DHist->Fit("gaus", "S", "", maximum - 10, maximum + 10);
			double mean = fr->Parameter(1);
			TAGH_TDC_Offset = mean;
		}
		outFile.open(prefix + "tagh_base_time.txt");
		outFile << tagh_base_time_adc - (TAGH_ADC_Offset-nominalTAGHTime) 
				<< " " << tagh_base_time_tdc - (TAGH_TDC_Offset-nominalTAGHTime) << endl;
		outFile.close();

		// TAGM
		this1DHist = ExtractTDCADCTimingNS::Get1DHistogram("HLDetectorTiming", "TAGM", "TAGMHit ADC time");
		if(this1DHist != NULL){
			Int_t firstBin = this1DHist->FindFirstBinAbove( 1 , 1); // Find first bin with content above 1 in the histogram
			for (int i = 0; i <= 90; i++){
				if ((firstBin + i) > 0) this1DHist->SetBinContent((firstBin + i), 0);
			}
		   //Gaussian
			Double_t maximum = this1DHist->GetBinCenter(this1DHist->GetMaximumBin());
			TFitResultPtr fr = this1DHist->Fit("gaus", "S", "", maximum - 10, maximum + 8);
			double mean = fr->Parameter(1);
			TAGM_ADC_Offset = mean;
		}
		this1DHist = ExtractTDCADCTimingNS::Get1DHistogram("HLDetectorTiming", "TAGM", "TAGMHit TDC time");
		if(this1DHist != NULL){
			Int_t firstBin = this1DHist->FindFirstBinAbove( 1 , 1); // Find first bin with content above 1 in the histogram
			for (int i = 0; i <= 90; i++){
				if ((firstBin + i) > 0) this1DHist->SetBinContent((firstBin + i), 0);
			}
		   //Gaussian
			Double_t maximum = this1DHist->GetBinCenter(this1DHist->GetMaximumBin());
			TFitResultPtr fr = this1DHist->Fit("gaus", "S", "", maximum - 10, maximum + 8);
			double mean = fr->Parameter(1);
			TAGM_TDC_Offset = mean;
		}
		outFile.open(prefix + "tagm_base_time.txt");
		outFile << tagm_base_time_adc - (TAGM_ADC_Offset-nominalTAGMTime) 
				<< " " << tagm_base_time_tdc - (TAGM_TDC_Offset-nominalTAGMTime) << endl;
		outFile.close();
		
	}


    ExtractTDCADCTimingNS::thisFile->Write();
    return;
    }
