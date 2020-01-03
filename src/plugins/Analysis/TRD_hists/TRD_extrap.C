// The following are special comments used by RootSpy to know
// which histograms to fetch for the macro.
//

// hnamepath: /TRD_hists/StraightTracks/WireTRDPoint_TrackX
// hnamepath: /TRD_hists/StraightTracks/WireTRDPoint_TrackY
// hnamepath: /TRD_hists/StraightTracks/WireTRDPoint_DeltaXY
// hnamepath: /TRD_hists/StraightTracks/WireTRDPoint_Time

{
    TDirectory *dir = (TDirectory*)gDirectory->FindObjectAny("TRD_hists/StraightTracks");
    if(dir) dir->cd();

    TH2I* trackX = (TH2I*)gDirectory->FindObjectAny("WireTRDPoint_TrackX");
    TH2I* trackY = (TH2I*)gDirectory->FindObjectAny("WireTRDPoint_TrackY");
    TH2I* deltaXY = (TH2I*)gDirectory->FindObjectAny("WireTRDPoint_DeltaXY");
    TH2I* t = (TH2I*)gDirectory->FindObjectAny("WireTRDPoint_Time");
    
    if(gPad == NULL){
        TCanvas *c1 = new TCanvas("c1","TRD Extapolated Track Monitor",150,10,990,660);
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
    if(t) {
	    c1->cd(4);
	    t->Draw();
    }
}
