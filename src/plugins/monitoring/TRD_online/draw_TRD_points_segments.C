
const Int_t NEventsPointSegmentMonitor = 10;
const Int_t NMaxExtrapolations = 5;
const Int_t NMaxSegments = 5;

struct Segment {
    double x;
    double y;
    double tx;
    double ty;
    double var_x;
    double var_y;
    double var_tx;
    double var_ty;
    TF1 *segment_path_x;
    TF1 *segment_path_y;
    Segment(double x, double y, double tx, double ty, double var_x, double var_y, double var_tx, double var_ty) :
        x(x), y(y), tx(tx), ty(ty), var_x(var_x), var_y(var_y), var_tx(var_tx), var_ty(var_ty) {
            segment_path_x = new TF1("segment_path_x", "[0]*(x-532.4)+[1]", 528, 533);
            segment_path_x->SetParameter(0, tx);
            segment_path_x->SetParameter(1, x);
            segment_path_y = new TF1("segment_path_y", "[0]*(x-532.4)+[1]", 528, 533);
            segment_path_y->SetParameter(0, ty);
            segment_path_y->SetParameter(1, y);
        };
};

struct Extrapolation {
    double x;
    double y;
    double dxdz;
    double dydz;
    TF1 *extrapolation_path_x;
    TF1 *extrapolation_path_y;
    Extrapolation(double x, double y, double dxdz, double dydz) :
        x(x), y(y), dxdz(dxdz), dydz(dydz) {
            extrapolation_path_x = new TF1("extrapolation_path_x", "[0]*(x-532.4)+[1]", 528, 533);
            extrapolation_path_x->SetParameter(0, dxdz);
            extrapolation_path_x->SetParameter(1, x);
            extrapolation_path_y = new TF1("extrapolation_path_y", "[0]*(x-532.4)+[1]", 528, 533);
            extrapolation_path_y->SetParameter(0, dydz);
            extrapolation_path_y->SetParameter(1, y);
        };
};

void draw_TRD_points_segments(TString root_file = "/work/halld2/home/nseptian/TRD-DEV/hd_root.root") {
    
    TFile *f = TFile::Open(root_file);
    TDirectory *dir = (TDirectory*)f->Get("TRD");
    TDirectory *subdir = (TDirectory*)dir->Get("Point");

    TH2I *hPoint_XvsZ[NEventsPointSegmentMonitor];
    TH2I *hPoint_YvsZ[NEventsPointSegmentMonitor];
    TH2D *hSegment_Members[NEventsPointSegmentMonitor][NMaxSegments];
    TH2D *hExtrapolation_Members[NEventsPointSegmentMonitor][NMaxExtrapolations];

    vector<vector<Segment*>> v_segments;
    vector<vector<Extrapolation*>> v_extrapolations;
	
    for (unsigned int i = 0; i < NEventsPointSegmentMonitor; i++) {
        vector<Segment*> segments;
        vector<Extrapolation*> extrapolations;
        
        hPoint_XvsZ[i] = (TH2I*)subdir->Get(Form("Point_ZVsX_Event%d", i));
        hPoint_YvsZ[i] = (TH2I*)subdir->Get(Form("Point_ZVsY_Event%d", i));
        for (unsigned int j = 0; j < NMaxSegments; j++) {
            hSegment_Members[i][j] = (TH2D*)subdir->Get(Form("Segment_Members_Event%d_Segment%d", i, j));
            
            Segment *segment = new Segment(
                hSegment_Members[i][j]->GetBinContent(1),
                hSegment_Members[i][j]->GetBinContent(2),
                hSegment_Members[i][j]->GetBinContent(3),
                hSegment_Members[i][j]->GetBinContent(4),
                hSegment_Members[i][j]->GetBinContent(5),
                hSegment_Members[i][j]->GetBinContent(6),
                hSegment_Members[i][j]->GetBinContent(7),
                hSegment_Members[i][j]->GetBinContent(8)
            );
            
            if (segment->x ==0.) {
                delete segment;
                continue;
            } else segments.push_back(segment);    
        }
        v_segments.push_back(segments);

        for (unsigned int j = 0; j < NMaxExtrapolations; j++) {
            hExtrapolation_Members[i][j] = (TH2D*)subdir->Get(Form("Extrapolation_Members_Event%d_Extrapolation%d", i, j));
            
            Extrapolation *extrapolation = new Extrapolation(
                hExtrapolation_Members[i][j]->GetBinContent(1),
                hExtrapolation_Members[i][j]->GetBinContent(2),
                hExtrapolation_Members[i][j]->GetBinContent(3),
                hExtrapolation_Members[i][j]->GetBinContent(4)
            );
            if (extrapolation->x ==0.) {
                delete extrapolation;
                continue;
            } else extrapolations.push_back(extrapolation);
            v_extrapolations.push_back(extrapolations);
        }

        
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
        for (unsigned int j = 0; j < v_segments[i].size(); j++) {
            v_segments[i][j]->segment_path_x->Draw("SAME");
            v_segments[i][j]->segment_path_x->SetLineStyle(2);
            v_segments[i][j]->segment_path_x->SetLineWidth(1);
            v_segments[i][j]->segment_path_x->SetLineColor(kRed);
        }
        for (unsigned int j = 0; j < v_extrapolations[i].size(); j++) {
            v_extrapolations[i][j]->extrapolation_path_x->Draw("SAME");
            v_extrapolations[i][j]->extrapolation_path_x->SetLineStyle(3);
            v_extrapolations[i][j]->extrapolation_path_x->SetLineWidth(1);
            v_extrapolations[i][j]->extrapolation_path_x->SetLineColor(kBlack);
        }

        c->cd(2);
        hPoint_YvsZ[i]->Draw("P");
        hPoint_YvsZ[i]->SetMarkerStyle(kCircle);
        for (unsigned int j = 0; j < v_segments[i].size(); j++) {
            v_segments[i][j]->segment_path_y->Draw("SAME");
            v_segments[i][j]->segment_path_y->SetLineStyle(2);
            v_segments[i][j]->segment_path_y->SetLineWidth(1);
            v_segments[i][j]->segment_path_y->SetLineColor(kRed);
        }

        for (unsigned int j = 0; j < v_extrapolations[i].size(); j++) {
            v_extrapolations[i][j]->extrapolation_path_y->Draw("SAME");
            v_extrapolations[i][j]->extrapolation_path_y->SetLineStyle(3);
            v_extrapolations[i][j]->extrapolation_path_y->SetLineWidth(1);
            v_extrapolations[i][j]->extrapolation_path_y->SetLineColor(kBlack);
        }

        c->Print(pdfFileName);
    }

    c->Print(pdfFileName + "]");
    f->Close();

}
