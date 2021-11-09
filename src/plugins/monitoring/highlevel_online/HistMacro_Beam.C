// hnamepath: /highlevel/RFBeamBunchPeriod
// hnamepath: /highlevel/RFBeamBunchPeriod_DFT
// hnamepath: /highlevel/BeamEnergy
//
// e-mail: davidl@jlab.org
// e-mail: staylor@jlab.org
// e-mail: sdobbs@jlab.org
// e-mail: tbritton@jlab.org
//

{
  //--------------------------------------------------------------------
  // This taken from the bin contents of: DBeamPhoton::E of a janaroot file
  // for run 90038, an amorphous target run.
  // The data in the table below was created with a macro in:
  //   $HALLD_RECON_HOME/src/plugins/monitoring/highlevel_online/AmorphousNormalization
  //
  // The working directory used was:
  //     ~hdops/2021.11.09.amorphous_normalization
  //
  string amorphous_label = "Normalized to Amorphous run 90038";

  Double_t amorphous_data[] = {
    0.0,         0.0,         0.0,         0.0,         0.0,         0.0,         0.0,         0.0,         0.0,         0.0,
    0.0,         0.0,         0.0,         0.0,         0.0,         0.0,         0.0,         0.0,         0.0,         0.0,
    0.0,         0.0,         0.0,         0.0,         0.0,         0.0,         0.0,         0.0,         0.0,         0.0,
    0.0,         0.0,         0.0,         0.0,         0.0,         0.0,         0.0,         0.0,         0.0,         0.0,
    0.0,         0.0,         0.0,         0.0,         0.0,         0.0,         0.0,         0.0,         0.0,         0.0,
    0.0,         0.0,         0.0,         0.0,         0.0,         0.0,         0.0,         0.0,         0.0,         0.0,
    0.0,         0.0,         0.0,         0.0,         0.0,         0.0,         0.0,         0.0,         0.0,         0.0,
    0.0,         0.0,         0.0,         0.0,         0.0,         0.0,     62185.0,     61005.0,         0.0,     56670.0,
    55721.0,         0.0,     66858.0,     55938.0,     47567.0,     49324.0,     37043.0,     33747.0,     36532.0,         0.0,
    33606.0,     26166.0,     35353.0,     27285.0,     26988.0,     39640.0,     25160.0,         0.0,     28508.0,     27473.0,
    24755.0,     23198.0,     23792.0,     21331.0,     28276.0,         0.0,     24505.0,     22995.0,     22821.0,     21355.0,
    22009.0,     21732.0,     20734.0,     17623.0,         0.0,     16946.0,     17717.0,     17777.0,     19672.0,     16864.0,
    15139.0,     14525.0,     25275.0,         0.0,     22471.0,     35193.0,     20476.0,     34439.0,     19277.0,     20165.0,
    27743.0,         0.0,     31152.0,     26972.0,     26481.0,     24415.0,     23428.0,         0.0,     16411.0,     17298.0,
    18043.0,     15746.0,      9217.0,     34201.0,     14197.0,     15528.0,         0.0,     14520.0,     11243.0,     12871.0,
    33374.0,     33792.0,     50408.0,     38945.0,    197127.0,    554321.0,    704490.0,    483578.0,    708765.0,    617016.0,
    599424.0,    625045.0,    635360.0,    630786.0,    607040.0,    584349.0,    567135.0,    621522.0,    611239.0,    728565.0,
    601340.0,    735471.0,    636548.0,    729000.0,    495340.0,     19796.0,     24417.0,     17998.0,     22175.0,     20859.0,
    13498.0,     18095.0,     17998.0,     10842.0,     17347.0,     15462.0,     14500.0,     13890.0,     13330.0,     12528.0,
    12446.0,     21006.0,     17178.0,     14591.0,     15288.0,     14003.0,     13546.0,     18655.0,     12191.0,     16948.0,
    12829.0,     14459.0,     21263.0,     12676.0,     11941.0,     15806.0,      9648.0,     13508.0,      7957.0,     13285.0,
    16105.0,     14330.0,     12528.0,     11422.0,     14148.0,      8283.0,     10518.0,      5672.0,     10370.0,      8305.0,
    5309.0,      3978.0,      3088.0,      2260.0,         0.0,         0.0,         0.0,         0.0,         0.0,         0.0,
    0.0,         0.0,         0.0,         0.0,         0.0,         0.0,         0.0,         0.0,         0.0,         0.0,
    0.0};
  //--------------------------------------------------------------------

	TDirectory *locTopDirectory = gDirectory;

	//Goto Beam Path
	TDirectory *locDirectory = (TDirectory*)gDirectory->FindObjectAny("highlevel");
	if(!locDirectory)
		return;
	locDirectory->cd();

	TH1* locHist_RFBeamBunchPeriod = (TH1*)gDirectory->Get("RFBeamBunchPeriod");
	TH1* locHist_RFBeamBunchPeriod_DFT = (TH1*)gDirectory->Get("RFBeamBunchPeriod_DFT");
	TH1* locHist_BeamEnergy = (TH1*)gDirectory->Get("BeamEnergy");

	//Get/Make Canvas
	TCanvas *locCanvas = NULL;
	if(TVirtualPad::Pad() == NULL)
		locCanvas = new TCanvas("Beam", "Beam", 1200, 600); //for testing
	else
		locCanvas = gPad->GetCanvas();
	locCanvas->Divide(2, 1);

	TLatex latex;
	latex.SetTextSize(0.04);
	char str[256];

	//Draw
	locCanvas->cd(1);
	gPad->SetTicks();
	gPad->SetGrid();
	if( (locHist_RFBeamBunchPeriod_DFT != NULL) && (locHist_RFBeamBunchPeriod != NULL))
	{
		double max = locHist_RFBeamBunchPeriod_DFT->GetMaximum()*2.0;
		locHist_RFBeamBunchPeriod_DFT->GetXaxis()->SetTitleSize(0.05);
		locHist_RFBeamBunchPeriod_DFT->GetXaxis()->SetLabelSize(0.05);
		locHist_RFBeamBunchPeriod_DFT->GetYaxis()->SetLabelSize(0.03);
		locHist_RFBeamBunchPeriod_DFT->SetFillStyle(3001);
		locHist_RFBeamBunchPeriod_DFT->SetFillColor(kGreen);
		locHist_RFBeamBunchPeriod_DFT->SetLineColor(kGreen-2);
		locHist_RFBeamBunchPeriod_DFT->SetLineWidth(2);
		locHist_RFBeamBunchPeriod_DFT->SetStats(0);
		locHist_RFBeamBunchPeriod_DFT->GetYaxis()->SetRangeUser(0.0, max);
		locHist_RFBeamBunchPeriod_DFT->Draw();

		sprintf(str, "%g entries", (double)locHist_RFBeamBunchPeriod->GetEntries());
		latex.DrawLatex(300.0, 1.02*max, str);
		
		// Plot RFBeamBunchPeriod as inset
		TPad *p = (TPad*)gDirectory->FindObjectAny("RF_DFT");
		if(!p) p = new TPad("RF_DFT", "insert", 0.16, 0.6, 0.89, 0.9);
		p->Draw();
		p->cd();

		locHist_RFBeamBunchPeriod->GetXaxis()->SetRangeUser(30.0, 90.0);
		locHist_RFBeamBunchPeriod->GetXaxis()->SetTitleSize(0.05);
		locHist_RFBeamBunchPeriod->GetXaxis()->SetLabelSize(0.05);
		locHist_RFBeamBunchPeriod->GetYaxis()->SetLabelSize(0.03);
		locHist_RFBeamBunchPeriod->SetFillStyle(3001);
		locHist_RFBeamBunchPeriod->SetFillColor(kMagenta);
		locHist_RFBeamBunchPeriod->SetStats(0);
		locHist_RFBeamBunchPeriod->Draw();
		
	}

	locCanvas->cd(2);
	gPad->SetTicks();
	gPad->SetGrid();
	if(locHist_BeamEnergy != NULL)
	{
		// Create normalized histogram
		TH1D* locHist_BeamEnergy_norm = (TH1D*)gDirectory->Get("BeamEnergy_norm");
		if(!locHist_BeamEnergy_norm){
			locHist_BeamEnergy_norm = new TH1D("BeamEnergy_norm", "Reconstructed Photon Beam Energy;Beam #gamma energy (GeV)", 240, 0.0, 12.0);
		}
		if(locHist_BeamEnergy_norm){

			locHist_BeamEnergy_norm->Reset();

			// Normalize to amorphous baseline 
			double scale = 0.0;
			for(int ibin=1; ibin<=locHist_BeamEnergy_norm->GetNbinsX(); ibin++){
				Double_t norm = amorphous_data[ibin-1];
				if( norm < 10000.0) continue;

				Double_t v = (Double_t)locHist_BeamEnergy->GetBinContent(ibin);
				v /= norm;
				locHist_BeamEnergy_norm->SetBinContent(ibin, v);
				if(v > scale) scale = v;
			}

			// Find maximum and scale histogram so peak is at 1
			if(scale != 0.0) locHist_BeamEnergy_norm->Scale(1.0/scale);		

			// Find leftmost non-zero bin 
			double left_scale = 0.0;
			for(int ibin=1; ibin<=locHist_BeamEnergy_norm->GetNbinsX(); ibin++){
				if( amorphous_data[ibin-1] < 10000.0) continue;
				Double_t v = (Double_t)locHist_BeamEnergy_norm->GetBinContent(ibin);
				if(v>0.1){
					left_scale = v;
					break;
				}
			}
			
			// Set min and max of plot to show range
			double diff = 1.0 - left_scale;
			double min = left_scale - diff*0.1;
			double max = left_scale + diff*1.2;
			if( (max-min) < 0.5 ){
				min = 0.25;
				if(max<1.50) max =1.4;
			}
			
			// max = 1.50;
			// min = 0.25;
			
			locHist_BeamEnergy_norm->GetXaxis()->SetTitleSize(0.05);
			locHist_BeamEnergy_norm->GetXaxis()->SetLabelSize(0.05);
			locHist_BeamEnergy_norm->GetYaxis()->SetLabelSize(0.03);
			locHist_BeamEnergy_norm->SetFillStyle(3001);
			locHist_BeamEnergy_norm->SetFillColor(kOrange);
			locHist_BeamEnergy_norm->SetLineColor(kRed-2);
			locHist_BeamEnergy_norm->SetLineWidth(2);
			locHist_BeamEnergy_norm->SetStats(0);		
			locHist_BeamEnergy_norm->GetXaxis()->SetRangeUser(4, 12);
			locHist_BeamEnergy_norm->GetYaxis()->SetRangeUser(min, max);
			locHist_BeamEnergy_norm->Draw("HIST");
			
			// If diff is > 0.15 then assume this is not an amorphous run
			// and draw a label of the peak energy
			if(diff > 0.15){
				double Epeak = locHist_BeamEnergy_norm->GetBinCenter(locHist_BeamEnergy_norm->GetMaximumBin());
				sprintf(str, "Epeak: %3.2f GeV", Epeak);
				latex.SetTextAlign(12);
				latex.DrawLatex(4.5, (min+max)/2.0, str);
			}

			sprintf(str, "%g entries", (double)locHist_BeamEnergy->GetEntries());
			latex.SetTextAlign(22);
			latex.DrawLatex(6.0, 1.035*(max-min)+min, str);

			latex.SetTextAngle(270);
			latex.DrawLatex(12.5, (min+max)/2.0, amorphous_label.c_str());
		}

		TPad *beamenergypad = (TPad*)gDirectory->FindObjectAny("beamenergypad");
		if(!beamenergypad) beamenergypad = new TPad("beamenergypad", "", 0.105, 0.65, 0.5, 0.895);
		beamenergypad->SetTicks();
		beamenergypad->Draw();
		beamenergypad->cd();
	
		//locHist_BeamEnergy->Rebin(5);
		locHist_BeamEnergy->GetXaxis()->SetTitleSize(0.05);
		locHist_BeamEnergy->GetXaxis()->SetLabelSize(0.05);
		locHist_BeamEnergy->GetYaxis()->SetLabelSize(0.03);
		locHist_BeamEnergy->SetFillStyle(3001);
		locHist_BeamEnergy->SetFillColor(kCyan);
		locHist_BeamEnergy->SetLineColor(kBlue);
		locHist_BeamEnergy->SetLineWidth(2);
		locHist_BeamEnergy->SetStats(0);
		locHist_BeamEnergy->Draw();
	}
}

