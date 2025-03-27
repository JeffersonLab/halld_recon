// The following are special comments used by RootSpy to know
// which histograms to fetch for the macro.
//

// hnamepath: /TRD/Hit/Hit_StripVsdE_Plane0
// hnamepath: /TRD/Hit/Hit_StripVsdE_Plane1
// hnamepath: /TRD/Hit/Hit_TimeVsStrip_Plane0
// hnamepath: /TRD/Hit/Hit_TimeVsStrip_Plane1

{
    TDirectory *dir = (TDirectory*)gDirectory->FindObjectAny("TRD/Hit");
    if(dir) dir->cd();

    TH2I* stripdE0 = (TH2I*)gDirectory->FindObjectAny("Hit_StripVsdE_Plane0");
    TH2I* stripTime0 = (TH2I*)gDirectory->FindObjectAny("Hit_TimeVsStrip_Plane0");
    TH2I* stripdE1 = (TH2I*)gDirectory->FindObjectAny("Hit_StripVsdE_Plane1");
    TH2I* stripTime1 = (TH2I*)gDirectory->FindObjectAny("Hit_TimeVsStrip_Plane1");

    if(gPad == NULL){
        TCanvas *c1 = new TCanvas("c1","TRD Hit Monitor",150,10,990,660);
        c1->cd(0);
        c1->Draw();
        c1->Update();
    }
    if(!gPad) return;
    TCanvas* c1 = gPad->GetCanvas();
    c1->Divide(2,2);
    double tsize = 0.0475;
    gStyle->SetOptStat("emr");

    if(stripdE0) {
	    c1->cd(1);
	    stripdE0->Draw("colz");
    }
    if(stripTime0) {
	    c1->cd(3);
	    stripTime0->Draw("colz");
    }
    if(stripdE1) {
	    c1->cd(2);
	    stripdE1->Draw("colz");
    }
    if(stripTime1) {
      c1->cd(4);
	    stripTime1->Draw("colz");
    }

}
