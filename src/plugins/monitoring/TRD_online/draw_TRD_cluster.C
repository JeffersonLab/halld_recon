
const Int_t NTRDplanes = 2;
const Int_t NEventsClusterMonitor = 10;

void draw_TRD_cluster(TString root_file = "/work/halld2/home/nseptian/TRD-DEV/hd_root.root") {
    
    TFile *f = TFile::Open(root_file);
    TDirectory *dir = (TDirectory*)f->Get("TRD");
    TDirectory *subdir = (TDirectory*)dir->Get("Cluster");

    TH2I *hClusterHits_TimeVsPos[NTRDplanes][NEventsClusterMonitor];
    TH2I *hCluster_TimeVsPos[NTRDplanes][NEventsClusterMonitor];
	TH2I *hDigiHit_TimeVsStrip[NTRDplanes][NEventsClusterMonitor];
	
    for (int i = 0; i < NTRDplanes; i++) {
        for (int j = 0; j < NEventsClusterMonitor; j++) {
            hClusterHits_TimeVsPos[i][j] = (TH2I*)subdir->Get(Form("ClusterHits_TimeVsPos_Plane%d_Event%d", i, j));
            hCluster_TimeVsPos[i][j] = (TH2I*)subdir->Get(Form("Cluster_TimeVsPos_Plane%d_Event%d", i, j));
			hDigiHit_TimeVsStrip[i][j] = (TH2I*)subdir->Get(Form("DigiHit_TimeVsStrip_Plane%d_Event%d", i, j));
        }
    }

    TCanvas *c = new TCanvas("c", "c", 1600, 800);
    c->Divide(2, 2);

    TString pdfFileName = "hd_root_TRDCluster_output_";
    pdfFileName += NEventsClusterMonitor;
    pdfFileName += "events.pdf";
    
    c->Print(pdfFileName + "[");

    gStyle->SetOptStat(0);
	gStyle->SetPalette(kRainBow);
    for (int i = 0; i < NEventsClusterMonitor; i++) {
        for (int j = 0; j < NTRDplanes; j++) {
            c->cd(j+(j+1));
            hClusterHits_TimeVsPos[j][i]->Draw("COLZ");
            hClusterHits_TimeVsPos[j][i]->GetXaxis()->SetTitle("Time (ns)");
            hClusterHits_TimeVsPos[j][i]->GetYaxis()->SetTitle("Position (cm)");
            
            hCluster_TimeVsPos[j][i]->Draw("SAME");
            hCluster_TimeVsPos[j][i]->SetMarkerStyle(kCircle);
            hCluster_TimeVsPos[j][i]->SetMarkerSize(2.0);
			
			c->cd(j+(j+2));
            hDigiHit_TimeVsStrip[j][i]->Draw("COLZ");
            hDigiHit_TimeVsStrip[j][i]->GetXaxis()->SetTitle("Time (ns)");
            hDigiHit_TimeVsStrip[j][i]->GetYaxis()->SetTitle("Strip");
    		
            // hCluster_TimeVsPos[j][i]->Draw("SAME");
            // hCluster_TimeVsPos[j][i]->SetMarkerStyle(kCircle);
            // hCluster_TimeVsPos[j][i]->SetMarkerSize(2.0);
        }

        c->Print(pdfFileName);
    }

    c->Print(pdfFileName + "]");
    f->Close();

}
