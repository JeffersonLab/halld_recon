
const Int_t NEventsPointSegmentMonitor = 10;

void draw_TRD_points_segments(TString root_file = "/work/halld2/home/nseptian/TRD-DEV/hd_root.root") {
    
    TFile *f = TFile::Open(root_file);
    TDirectory *dir = (TDirectory*)f->Get("TRD");
    TDirectory *subdir = (TDirectory*)dir->Get("Point");

    TH2I *hPoint_XvsZ[NEventsPointSegmentMonitor];
    TH2I *hPoint_YvsZ[NEventsPointSegmentMonitor];
    TH2I *hSegment_Members[NEventsPointSegmentMonitor];
    TF1 *track_path_x[NEventsPointSegmentMonitor];
    TF1 *track_path_y[NEventsPointSegmentMonitor];
	
    for (int i = 0; i < NEventsPointSegmentMonitor; i++) {
        hPoint_XvsZ[i] = (TH2I*)subdir->Get(Form("Point_ZVsX_Event%d", i));
        hPoint_YvsZ[i] = (TH2I*)subdir->Get(Form("Point_ZVsY_Event%d", i));
        hSegment_Members[i] = (TH2I*)subdir->Get(Form("Segment_Members_Event%d", i));
        double x = hSegment_Members[i]->GetBinContent(1);
        double y = hSegment_Members[i]->GetBinContent(2);
        double tx = hSegment_Members[i]->GetBinContent(3);
        double ty = hSegment_Members[i]->GetBinContent(4);
        double var_x = hSegment_Members[i]->GetBinContent(5);
        double var_y = hSegment_Members[i]->GetBinContent(6);
        double var_tx = hSegment_Members[i]->GetBinContent(7);
        double var_ty = hSegment_Members[i]->GetBinContent(8);

        cout << "Event " << i << ": x=" << x << ", y=" << y << ", tx=" << tx << ", ty=" << ty
             << ", var_x=" << var_x << ", var_y=" << var_y
             << ", var_tx=" << var_tx << ", var_ty=" << var_ty << endl;

        track_path_x[i] = new TF1("track_path_x", "[0]*(x-532.5)+[1]", 528, 533);
        track_path_x[i]->SetParameter(0, tx);
        track_path_x[i]->SetParameter(1, x);

        track_path_y[i] = new TF1("track_path_y", "[0]*(x-532.5)+[1]", 528, 533);
        track_path_y[i]->SetParameter(0, ty);
        track_path_y[i]->SetParameter(1, y);
    }

    TCanvas *c = new TCanvas("c", "c", 1600, 800);
    c->Divide(2, 1);

    TString pdfFileName = "hd_root_TRDPointSegment_output_";
    pdfFileName += NEventsPointSegmentMonitor;
    pdfFileName += "events.pdf";
    
    c->Print(pdfFileName + "[");

    gStyle->SetOptStat(0);
	gStyle->SetPalette(kRainBow);
    for (int i = 0; i < NEventsPointSegmentMonitor; i++) {
        c->cd(1);
        hPoint_XvsZ[i]->Draw("P");
        hPoint_XvsZ[i]->SetMarkerStyle(kCircle);
        track_path_x[i]->Draw("SAME");
        track_path_x[i]->SetLineStyle(2);
        track_path_x[i]->SetLineWidth(1);

        c->cd(2);
        hPoint_YvsZ[i]->Draw("P");
        hPoint_YvsZ[i]->SetMarkerStyle(kCircle);
        track_path_y[i]->Draw("SAME");
        track_path_y[i]->SetLineStyle(2);
        track_path_y[i]->SetLineWidth(1);

        c->Print(pdfFileName);
    }

    c->Print(pdfFileName + "]");
    f->Close();

}
