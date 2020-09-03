// The following are special comments used by RootSpy to know
// which histograms to fetch for the macro.
//

// hnamepath: /TRD_hists/StraightTracks/GEMTRDPoint_TrackX
// hnamepath: /TRD_hists/StraightTracks/GEMTRDPoint_TrackY
// hnamepath: /TRD_hists/StraightTracks/GEMTRDPoint_DeltaXY

{
    TDirectory *dir = (TDirectory*)gDirectory->FindObjectAny("TRD_hists/StraightTracks");
    if(dir) dir->cd();

    TH2I* trackX = (TH2I*)gDirectory->FindObjectAny("GEMTRDPoint_TrackX");
    TH2I* trackY = (TH2I*)gDirectory->FindObjectAny("GEMTRDPoint_TrackY");
    TH2I* deltaXY = (TH2I*)gDirectory->FindObjectAny("GEMTRDPoint_DeltaXY");
    
    if(gPad == NULL){
        TCanvas *c1 = new TCanvas("c1","GEM TRD Extapolated Track Monitor",150,10,990,660);
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
