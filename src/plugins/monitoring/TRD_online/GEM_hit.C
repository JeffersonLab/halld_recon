// The following are special comments used by RootSpy to know
// which histograms to fetch for the macro.
//

// hnamepath: /TRD/GEMHit/GEMHit_SampleVsStrip_Plane0
// hnamepath: /TRD/GEMHit/GEMHit_SampleVsStrip_Plane1
// hnamepath: /TRD/GEMHit/GEMHit_SampleVsStrip_Plane2
// hnamepath: /TRD/GEMHit/GEMHit_SampleVsStrip_Plane3
// hnamepath: /TRD/GEMHit/GEMHit_SampleVsStrip_Plane8
// hnamepath: /TRD/GEMHit/GEMHit_SampleVsStrip_Plane9

{
    TDirectory *dir = (TDirectory*)gDirectory->FindObjectAny("TRD/GEMHit");
    if(dir) dir->cd();
    gDirectory->ls();

    const int maxplane = 10;
    TH2I* gemHit_Plane[maxplane];
    for(int i=0; i<=maxplane; i++)
	    gemHit_Plane[i] = (TH2I*)gDirectory->FindObjectAny(Form("GEMHit_SampleVsStrip_Plane%d", i));

    if(gPad == NULL){
        TCanvas *c1 = new TCanvas("c1","TRD Hit Monitor",150,10,990,660);
        c1->cd(0);
        c1->Draw();
        c1->Update();
    }
    if(!gPad) return;
    TCanvas* c1 = gPad->GetCanvas();
    c1->Divide(3,2);
    c1->cd(1);
    double tsize = 0.0475;
    gStyle->SetOptStat("emr");
    int icanvas = 1;
    
    // RunPeriod-2019-01 plane orientation
    if(gemHit_Plane[0] && gemHit_Plane[0]->Integral() > 0.) {
	    for(int i=0; i<=maxplane; i++) {
		    if(gemHit_Plane[i] && !(i>3 && i<8)) {
			    c1->cd(icanvas);
			    gemHit_Plane[i]->Draw("colz");
			    icanvas++;
		    }
	    }
    }
    else if(gemHit_Plane[4] && gemHit_Plane[4]->Integral() > 0.) {
	    for(int i=4; i<=7; i++) {
		    if(gemHit_Plane[i]) {
			    c1->cd(icanvas);
			    gemHit_Plane[i]->Draw("colz");
			    icanvas++;
		    }
	    }
    }

}
