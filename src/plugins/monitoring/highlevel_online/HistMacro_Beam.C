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
  // for run 100370, an amorphous target run.
  // The data in the table below was created with a macro in:
  //   $HALLD_RECON_HOME/src/plugins/monitoring/highlevel_online/AmorphousNormalization
  //
  // The working directory used was:
  //     ~hdops/2022.06.11.amorphous_normalization
  //
  string amorphous_label = "Normalized to Amorphous run 100370";

  Double_t amorphous_data[] = {
    0.0,         0.0,         0.0,         0.0,         0.0,         0.0,         0.0,         0.0,         0.0,         0.0,
    0.0,         0.0,         0.0,         0.0,         0.0,         0.0,         0.0,         0.0,         0.0,         0.0,
    0.0,         0.0,         0.0,         0.0,         0.0,         0.0,         0.0,         0.0,         0.0,         0.0,
    0.0,         0.0,         0.0,         0.0,         0.0,         0.0,         0.0,         0.0,         0.0,         0.0,
    0.0,         0.0,         0.0,         0.0,         0.0,         0.0,         0.0,         0.0,         0.0,         0.0,
    0.0,         0.0,         0.0,         0.0,         0.0,         0.0,         0.0,         0.0,     72469.0,    196480.0,
    253890.0,         0.0,    260959.0,    264791.0,    256267.0,    237171.0,    254741.0,    230914.0,         0.0,    202612.0,
    226976.0,    238157.0,    225021.0,    219379.0,    189509.0,         0.0,    223089.0,    207603.0,    192709.0,    199170.0,
    182148.0,    189056.0,         0.0,         0.0,         0.0,         0.0,         0.0,         0.0,    162739.0,    160563.0,
    166491.0,    124665.0,    349816.0,    343725.0,    421730.0,    343827.0,    177223.0,    281793.0,    158143.0,    293367.0,
    217019.0,    323556.0,    258121.0,    311897.0,    357686.0,    146992.0,    294879.0,    300721.0,    338019.0,    285861.0,
    291403.0,    331158.0,    278235.0,    209134.0,    191333.0,    289778.0,    345277.0,    289232.0,    339960.0,    287044.0,
    115239.0,    113069.0,     86953.0,         0.0,    102634.0,    133684.0,    124256.0,    160576.0,         0.0,    119654.0,
    152433.0,    113890.0,    116965.0,    137952.0,         0.0,    143712.0,    134109.0,    131343.0,    128409.0,    125444.0,
    0.0,    104705.0,    104614.0,    102123.0,    101386.0,     74635.0,         0.0,    146856.0,     95812.0,     95168.0,
    93487.0,     84390.0,     88833.0,     69064.0,    200416.0,    301262.0,    248375.0,    190163.0,     13197.0,         0.0,
    0.0,         0.0,         0.0,         0.0,         0.0,         0.0,         0.0,         0.0,         0.0,         0.0,
    0.0,         0.0,         0.0,         0.0,         0.0,         0.0,         0.0,         0.0,         0.0,    136237.0,
    204611.0,    130583.0,    189679.0,    131753.0,    183961.0,    121014.0,    173538.0,    118559.0,    167766.0,    171569.0,
    164989.0,    108874.0,    155636.0,    155265.0,    149056.0,    147024.0,    198403.0,    144283.0,    127034.0,    138084.0,
    134694.0,    127391.0,    128853.0,    183168.0,    151274.0,    129977.0,    139662.0,    136582.0,    131152.0,    123411.0,
    185263.0,    117335.0,    111947.0,    160350.0,    186460.0,     74290.0,    150247.0,    139817.0,    131895.0,    124428.0,
    177064.0,    106577.0,    144877.0,    101701.0,    158489.0,     91064.0,    115849.0,    130068.0,    102657.0,     79550.0,
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
		if(!beamenergypad) beamenergypad = new TPad("beamenergypad", "", 0.45, 0.65, 0.885, 0.895);
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

