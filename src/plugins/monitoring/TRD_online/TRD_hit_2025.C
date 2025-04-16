// The following are special comments used by RootSpy to know
// which histograms to fetch for the macro.
//

// hnamepath: /TRD/Hit/Hit_TimeVsStrip_Plane0
// hnamepath: /TRD/Hit/Hit_TimeVsStrip_Plane1
// hnamepath: /TRD/Hit/Hit_Occupancy_Plane0
// hnamepath: /TRD/Hit/Hit_Occupancy_Plane1
// hnamepath: /TRD/Hit/Hit_PulseHeight_Plane0
// hnamepath: /TRD/Hit/Hit_PulseHeight_Plane1

{
    TDirectory *dir = (TDirectory*)gDirectory->FindObjectAny("TRD/Hit");
    if(dir) dir->cd();

    TH2I* stripTime0 = (TH2I*)gDirectory->FindObjectAny("Hit_TimeVsStrip_Plane0");
	TH1I* stripOcc0 = (TH1I*)gDirectory->FindObjectAny("Hit_Occupancy_Plane0");
    TH1I* totalPulse0 = (TH1I*)gDirectory->FindObjectAny("Hit_PulseHeight_Plane0");
    TH2I* stripTime1 = (TH2I*)gDirectory->FindObjectAny("Hit_TimeVsStrip_Plane1");
	TH1I* stripOcc1 = (TH1I*)gDirectory->FindObjectAny("Hit_Occupancy_Plane1");
	TH1I* totalPulse1 = (TH1I*)gDirectory->FindObjectAny("Hit_PulseHeight_Plane1");
	
	
    if(gPad == NULL){
        TCanvas *c1 = new TCanvas("c1","TRD Hit Monitor",150,10,990,660);
        c1->cd(0);
        c1->Draw();
        c1->Update();
    }
    if(!gPad) return;
    TCanvas* c1 = gPad->GetCanvas();
    c1->Divide(3,2);
    double tsize = 0.0475;
    gStyle->SetOptStat("emr");

    if(stripOcc0) {
	    c1->cd(1);
	    stripOcc0->Draw("");
    }
	if(stripOcc1) {
        c1->cd(2);
        stripOcc1->Draw("");
    }
    if(stripTime0) {
	    c1->cd(3);
	    stripTime0->Draw("colz");
    }
    if(stripTime1) {
      c1->cd(4);
	    stripTime1->Draw("colz");
    }
	if(totalPulse0) {
        c1->cd(5);
        totalPulse0->Draw("");
    }
    if(totalPulse1) {
        c1->cd(6);
        totalPulse1->Draw("");
    }


}
