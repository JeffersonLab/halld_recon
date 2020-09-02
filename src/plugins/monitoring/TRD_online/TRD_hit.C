// The following are special comments used by RootSpy to know
// which histograms to fetch for the macro.
//

// hnamepath: /TRD/DigiHit/DigiHit_Occupancy_WirePlane0
// hnamepath: /TRD/DigiHit/DigiHit_Occupancy_WirePlane1
// hnamepath: /TRD/DigiHit/DigiHit_Occupancy_GEMPlane2
// hnamepath: /TRD/DigiHit/DigiHit_Occupancy_GEMPlane3
// hnamepath: /TRD/DigiHit/DigiHit_Occupancy_PadGEM
// hnamepath: /TRD/DigiHit/DigiHit_TimeVsStrip_WirePlane0
// hnamepath: /TRD/DigiHit/DigiHit_TimeVsStrip_WirePlane1
// hnamepath: /TRD/DigiHit/DigiHit_TimeVsStrip_GEMPlane2
// hnamepath: /TRD/DigiHit/DigiHit_TimeVsStrip_GEMPlane3
// hnamepath: /TRD/DigiHit/DigiHit_TimeVsPad_PadGEM

{
    TDirectory *dir = (TDirectory*)gDirectory->FindObjectAny("TRD/DigiHit");
    if(dir) dir->cd();

    TH2I* occWire0 = (TH2I*)gDirectory->FindObjectAny("DigiHit_Occupancy_WirePlane0");
    TH2I* timeWire0 = (TH2I*)gDirectory->FindObjectAny("DigiHit_TimeVsStrip_WirePlane0");
    TH2I* occWire1 = (TH2I*)gDirectory->FindObjectAny("DigiHit_Occupancy_WirePlane1");
    TH2I* timeWire1 = (TH2I*)gDirectory->FindObjectAny("DigiHit_TimeVsStrip_WirePlane1");
    TH2I* occGEMX = (TH2I*)gDirectory->FindObjectAny("DigiHit_Occupancy_GEMPlane2");
    TH2I* timeGEMX = (TH2I*)gDirectory->FindObjectAny("DigiHit_TimeVsStrip_GEMPlane2");
    TH2I* occGEMY = (TH2I*)gDirectory->FindObjectAny("DigiHit_Occupancy_GEMPlane3");
    TH2I* timeGEMY = (TH2I*)gDirectory->FindObjectAny("DigiHit_TimeVsStrip_GEMPlane3");
    TH2I* occPadGEM = (TH2I*)gDirectory->FindObjectAny("DigiHit_Occupancy_PadGEM");
    TH2I* timePadGEM = (TH2I*)gDirectory->FindObjectAny("DigiHit_TimeVsPad_PadGEM");

    if(gPad == NULL){
        TCanvas *c1 = new TCanvas("c1","TRD DigiHit Monitor",150,10,1100,660);
        c1->cd(0);
        c1->Draw();
        c1->Update();
    }
    if(!gPad) return;
    TCanvas* c1 = gPad->GetCanvas();
    c1->Divide(5,2);
    double tsize = 0.0475;
    gStyle->SetOptStat("emr");

    if(occWire0) {
	    c1->cd(1);
	    occWire0->Draw();
    }
    if(timeWire0) {
	    c1->cd(6);
	    timeWire0->Draw("colz");
    }
    if(occWire1) {
	    c1->cd(2);
	    occWire1->Draw();
    }
    if(timeWire1) {
	    c1->cd(7);
	    timeWire1->Draw("colz");
    }
    if(occGEMX) {
	    c1->cd(3);
	    occGEMX->Draw();
    }
    if(timeGEMX) {
	    c1->cd(8);
	    timeGEMX->Draw("colz");
    }
    if(occGEMY) {
	    c1->cd(4);
	    occGEMY->Draw();
    }
    if(timeGEMY) {
	    c1->cd(9);
	    timeGEMY->Draw("colz");
    }
    if(occPadGEM) {
            c1->cd(5);
            occPadGEM->Draw();
    }
    if(timePadGEM) {
            c1->cd(10);
            timePadGEM->Draw("colz");
    }


}
