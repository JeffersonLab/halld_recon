// The following are special comments used by RootSpy to know
// which histograms to fetch for the macro.
//

// hnamepath: /TRD/DigiHit/

{
    TDirectory *dir = (TDirectory*)gDirectory->FindObjectAny("TRD/DigiHit");
    if(dir) dir->cd();
    gDirectory->ls();

    TH2I* occWire0 = (TH2I*)gDirectory->FindObjectAny("DigiHit_Occupancy_WirePlane0");
    TH2I* timeWire0 = (TH2I*)gDirectory->FindObjectAny("DigiHit_TimeVsStrip_WirePlane0");
    TH2I* occWire1 = (TH2I*)gDirectory->FindObjectAny("DigiHit_Occupancy_WirePlane1");
    TH2I* timeWire1 = (TH2I*)gDirectory->FindObjectAny("DigiHit_TimeVsStrip_WirePlane1");
    TH2I* occGEMX = (TH2I*)gDirectory->FindObjectAny("DigiHit_Occupancy_GEMPlane2");
    TH2I* timeGEMX = (TH2I*)gDirectory->FindObjectAny("DigiHit_TimeVsStrip_GEMPlane2");
    TH2I* occGEMY = (TH2I*)gDirectory->FindObjectAny("DigiHit_Occupancy_GEMPlane3");
    TH2I* timeGEMY = (TH2I*)gDirectory->FindObjectAny("DigiHit_TimeVsStrip_GEMPlane3");

    if(gPad == NULL){
        TCanvas *c1 = new TCanvas("c1","TRD DigiHit Monitor",150,10,990,660);
        c1->cd(0);
        c1->Draw();
        c1->Update();
    }
    if(!gPad) return;
    TCanvas* c1 = gPad->GetCanvas();
    c1->Divide(4,2);
    double tsize = 0.0475;
    gStyle->SetOptStat("emr");

    if(occWire0) {
	    c1->cd(1);
	    occWire0->Draw();
    }
    if(timeWire0) {
	    c1->cd(5);
	    timeWire0->Draw("colz");
    }
    if(occWire1) {
	    c1->cd(2);
	    occWire1->Draw();
    }
    if(timeWire1) {
	    c1->cd(6);
	    timeWire1->Draw("colz");
    }
    if(occGEMX) {
	    c1->cd(3);
	    occGEMX->Draw();
    }
    if(timeGEMX) {
	    c1->cd(7);
	    timeGEMX->Draw("colz");
    }
    if(occGEMY) {
	    c1->cd(4);
	    occGEMY->Draw();
    }
    if(timeGEMY) {
	    c1->cd(8);
	    timeGEMY->Draw("colz");
    }

}
