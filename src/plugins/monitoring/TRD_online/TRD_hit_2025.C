// The following are special comments used by RootSpy to know
// which histograms to fetch for the macro.
//

// hnamepath: /TRD/DigiHit/DigiHit_Occupancy_Plane0
// hnamepath: /TRD/DigiHit/DigiHit_Occupancy_Plane1
// hnamepath: /TRD/DigiHit/DigiHit_TimeVsStrip_Plane0
// hnamepath: /TRD/DigiHit/DigiHit_TimeVsStrip_Plane1

{
    TDirectory *dir = (TDirectory*)gDirectory->FindObjectAny("TRD/DigiHit");
    if(dir) dir->cd();

    TH2I* occWire0 = (TH2I*)gDirectory->FindObjectAny("DigiHit_Occupancy_Plane0");
    TH2I* timeWire0 = (TH2I*)gDirectory->FindObjectAny("DigiHit_TimeVsStrip_Plane0");
    TH2I* occWire1 = (TH2I*)gDirectory->FindObjectAny("DigiHit_Occupancy_Plane1");
    TH2I* timeWire1 = (TH2I*)gDirectory->FindObjectAny("DigiHit_TimeVsStrip_Plane1");

    if(gPad == NULL){
        TCanvas *c1 = new TCanvas("c1","TRD DigiHit Monitor",150,10,990,660);
        c1->cd(0);
        c1->Draw();
        c1->Update();
    }
    if(!gPad) return;
    TCanvas* c1 = gPad->GetCanvas();
    c1->Divide(2,2);
    double tsize = 0.0475;
    gStyle->SetOptStat("emr");

    if(occWire0) {
	    c1->cd(1);
	    occWire0->Draw();
    }
    if(timeWire0) {
	    c1->cd(3);
	    timeWire0->Draw("colz");
    }
    if(occWire1) {
	    c1->cd(2);
	    occWire1->Draw();
    }
    if(timeWire1) {
      c1->cd(4);
	    timeWire1->Draw("colz");
    }

}
