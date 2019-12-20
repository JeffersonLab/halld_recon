// The following are special comments used by RootSpy to know
// which histograms to fetch for the macro.
//

// hnamepath: /TRD_hists/StraightTracks/GEMSRSPoint_TrackX_4
// hnamepath: /TRD_hists/StraightTracks/GEMSRSPoint_TrackY_4
// hnamepath: /TRD_hists/StraightTracks/GEMSRSPoint_DeltaXY_4

{
    TDirectory *dir = (TDirectory*)gDirectory->FindObjectAny("TRD_hists");
    if(dir) dir->cd();

    int plane = 4;
    TH2I* trackX = (TH2I*)gDirectory->FindObjectAny(Form("StraightTracks/GEMSRSPoint_TrackX_%d",plane));
    TH2I* trackY = (TH2I*)gDirectory->FindObjectAny(Form("StraightTracks/GEMSRSPoint_TrackY_%d",plane));
    TH2I* deltaXY = (TH2I*)gDirectory->FindObjectAny(Form("StraightTracks/GEMSRSPoint_DeltaXY_%d",plane));
    
    if(gPad == NULL){
        TCanvas *c1 = new TCanvas("c1","TRD Extapolated Track GEM Monitor",150,10,990,660);
        c1->cd(0);
        c1->Draw();
        c1->Update();
    }
    if(!gPad) return;
    TCanvas* c1 = gPad->GetCanvas();
    c1->Divide(2,2);
    double tsize = 0.0475;
    gStyle->SetOptStat("emr");

    if(trackX) {
	    c1->cd(1);
	    trackX->Draw("colz");
    }
    if(trackY) {
	    c1->cd(2);
	    trackY->Draw("colz");
    }
    if(deltaXY) {
	    c1->cd(3);
	    deltaXY->Draw("colz");
    }
}
