#include <TCanvas.h>
#include <TH1D.h>
#include <TH1F.h>
#include <TH1.h>
#include <TFile.h>
#include <TList.h>
#include <TString.h>
#include <iostream>
#include <TLegend.h>

TH1 *px;
TH1 *py;
TH1 *sx;
TH1 *sy;

void TRD2025_efficiency() {
  	
	// --- SET FILE NAME ---
	//TFile *file = TFile::Open("hd_root_TRDTrack_140540_011-013.root"); //min track P 0.5GeV, track within 1.5cm of TRD X and TRD Y
	//TFile *file = TFile::Open("hd_root_TRDTrack_140539_016-020.root"); //min track P 0.5GeV, track within 1.5cm of TRD X and TRD Y
	//TFile *file = TFile::Open("hd_root_TRDTrack_140540_000-009.root"); //min track P 0.5GeV, track within 1.5cm of TRD X and TRD Y
	//TFile *file = TFile::Open("hd_root_TRDTrack_140539_016-020_minP0.8.root"); //min track P 0.8GeV, track within 1.5cm of TRD X and TRD Y
	//TFile *file = TFile::Open("hd_root_TRDTrack_140540_011-013_withFlags.root"); //min track P 1GeV, track within 1.5cm of TRD X, 1.8cm of TRD Y
    //TFile *file = TFile::Open("hd_root_TRDTrack_140539_016-020_withFlags.root"); //min track P 1GeV, track within 1.5cm of TRD X, 1.8cm of TRD Y
    //TFile *file = TFile::Open("hd_root_TRDTrack_140540_000-009_withFlags.root"); //min track P 1GeV, track within 1.5cm of TRD X, 1.8cm of TRD Y
	//TFile *file = TFile::Open("hd_root_TRDTrack_140668_003_0.8P.root");
	//TFile *file = TFile::Open("hd_root_TRDTrack_140669_019_and_022_0.8P.root");
	//TFile *file = TFile::Open("hd_root_TRDTrack_140669_011_0.8P.root");
	//TFile *file = TFile::Open("hd_root_TRDTrack_140668_all_0.8P.root");
	TFile *file = TFile::Open("hd_root_TRDTrack_140706_012-021_0.8P.root");
	
	if (!file || file->IsZombie()) {
      	std::cerr << "Error: Could not open file" << std::endl;
    }
	
	//--For fitting the divided TGraphs
	TF1 *p0 = new TF1("p0","pol0",-90.,0.);
	p0->SetParameter(0,0.4);
	char fileNameTitleX[256];
	sprintf(fileNameTitleX,"#splitline{%s}{GEMTRD X Plane Point_Hit Efficiency Estimate}",file->GetName());
	char fileNameTitleY[256];
    sprintf(fileNameTitleY,"#splitline{%s}{GEMTRD Y Plane Point_Hit Efficiency Estimate}",file->GetName());
	
	TList *xList = new TList;
    TList *yList = new TList;
    int nxList=0, nyList=0;
	TCanvas *c1 = new TCanvas("c1",file->GetName(),1600,1200);
    c1->Divide(2,1);
    c1->GetPad(1)->SetLeftMargin(0.15);
    c1->GetPad(2)->SetLeftMargin(0.15);
	
	TObject *objx_p = file->Get("TRDTrack/Misc_Monitoring/ExtrapsX");
	if (objx_p && objx_p->InheritsFrom("TH1")) {
    	TH1 *xPass = (TH1*)objx_p;
		xPass->SetDirectory(0);
		xList->Add(xPass);
		nxList++;
    } else {
      	std::cerr << "Error: Object 'TRDTrack/Misc_Monitoring/ExtrapsX' is not a TH1 histogram or is missing." << std::endl;
    }
	
	TObject *objy_p = file->Get("TRDTrack/Misc_Monitoring/ExtrapsY");
    if (objy_p && objy_p->InheritsFrom("TH1")) {
        TH1 *yPass = (TH1*)objy_p;
		yPass->SetDirectory(0);
		yList->Add(yPass);
		nyList++;
    } else {
        std::cerr << "Error: Object 'TRDTrack/Misc_Monitoring/ExtrapsY' is not a TH1 histogram or is missing." << std::endl;
    }
	
	TObject *objx_s = file->Get("TRDTrack/Misc_Monitoring/seenPointsSingleX");
	//TObject *objx_s = file->Get("TRDTrack/Misc_Monitoring/seenHitsSingleX");
    if (objx_s && objx_s->InheritsFrom("TH1")) {
        TH1 *xSeen = (TH1*)objx_s;
		xSeen->SetDirectory(0);
		xList->Add(xSeen);
		nxList++;
    } else {
        std::cerr << "Error: Object 'TRDTrack/Misc_Monitoring/seenPointsSingleX' is not a TH1 histogram or is missing." << std::endl;
    }
	
	TObject *objy_s = file->Get("TRDTrack/Misc_Monitoring/seenPointsSingleY");
	//TObject *objy_s = file->Get("TRDTrack/Misc_Monitoring/seenHitsSingleY");
    if (objy_s && objy_s->InheritsFrom("TH1")) {
        TH1 *ySeen = (TH1*)objy_s;
		ySeen->SetDirectory(0);
		yList->Add(ySeen);
		nyList++;
    } else {
        std::cerr << "Error: Object 'TRDTrack/Misc_Monitoring/seenPointsSingleY' is not a TH1 histogram or is missing." << std::endl;
    }
	
	file->Close();

	c1->cd(1);
	px = (TH1 *)xList->FindObject("ExtrapsX");
	sx = (TH1 *)xList->FindObject("seenPointsSingleX");
	//sx = (TH1 *)xList->FindObject("seenHitsSingleX");
	if (px && sx) {
		for (int i=0; i<=sx->GetNbinsX(); i++) {
            if (sx->GetBinContent(i) > px->GetBinContent(i)) {
                sx->SetBinContent(i,0);
                px->SetBinContent(i,0);
            }
        }
		
		TGraphAsymmErrors *EFFX = new TGraphAsymmErrors();
		EFFX->Divide(sx,px,"cl=0.683 b(1,1) mode");
		EFFX->SetMarkerStyle(7);
		EFFX->SetMaximum(1);
		EFFX->GetXaxis()->SetTitle("X Position (in GlueX Coordinates) [cm]");
        EFFX->GetYaxis()->SetTitle("(Track Extrap. Seen) / (Track Extrap. Expected)");
		EFFX->SetTitle(fileNameTitleX);
		EFFX->Fit(p0);
		EFFX->Draw("ap");
    	float c=p0->GetParameter(0);
    	float ec=p0->GetParError(0);
    	cout<<" effX = "<<c<<" +/- "<<ec<<endl;
		
		TPaveText* ptx = new TPaveText(0.5,0.75,0.9,0.9,"NDC");
		ptx->SetFillColor(0);
		ptx->SetTextColor(kRed);
		ptx->SetTextSize(0.035);
		ptx->SetBorderSize(1);
		ptx->AddText(Form("Fit const. = %.3f #pm %.3f",c,ec));
		ptx->AddText(Form("#chi^{2} = %.2f   NDF = %d",p0->GetChisquare(),p0->GetNDF()));
		ptx->Draw("same");
		
    } else {
		std::cerr << "Error: Cannot divide X plane histos!" << std::endl;
	}
	
	c1->cd(2);
    py = (TH1 *)yList->FindObject("ExtrapsY");
    sy = (TH1 *)yList->FindObject("seenPointsSingleY");
	//sy = (TH1 *)yList->FindObject("seenHitsSingleY");
    if (py && sy) {
		for (int i=0; i<=sy->GetNbinsX(); i++) {
			if (sy->GetBinContent(i) > py->GetBinContent(i)) {
				sy->SetBinContent(i,0);
				py->SetBinContent(i,0);
			}
		}
		
		TGraphAsymmErrors *EFFY = new TGraphAsymmErrors();
		EFFY->Divide(sy,py,"cl=0.683 b(1,1) mode");
        EFFY->SetMarkerStyle(7);
		EFFY->SetMaximum(1);
        EFFY->GetXaxis()->SetTitle("Y Position (in GlueX Coordinates) [cm]");
        EFFY->GetYaxis()->SetTitle("(Track Extrap. Seen) / (Track Extrap. Expected)");
        EFFY->SetTitle(fileNameTitleY);
		EFFY->Fit(p0);
        EFFY->Draw("ap");
        float c=p0->GetParameter(0);
        float ec=p0->GetParError(0);
        cout<<" effY = "<<c<<" +/- "<<ec<<endl;

        TPaveText* pty = new TPaveText(0.5,0.75,0.9,0.9,"NDC");
        pty->SetFillColor(0);
        pty->SetTextColor(kRed);
        pty->SetTextSize(0.035);
		pty->SetBorderSize(1);
        pty->AddText(Form("Fit const. = %.3f #pm %.3f",c,ec));
        pty->AddText(Form("#chi^{2} = %.2f   NDF = %d",p0->GetChisquare(),p0->GetNDF()));
		pty->Draw("same");
		
    } else {
		std::cerr << "Error: Cannot divide Y plane histos!" << std::endl;
	}
	
	char pdfName[256];
	sprintf(pdfName,"efficiencyCheck_%s.pdf",file->GetName());
	c1->SaveAs(pdfName);
	
}

