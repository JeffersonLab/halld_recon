// hnamepath:  /FCAL2_invmass/h_2gamma_ECAL_ECAL
// hnamepath:  /FCAL2_invmass/h_2gamma_FCAL_FCAL
// hnamepath:  /FCAL2_invmass/h_2gamma_BCAL_BCAL
{
  TDirectory *dir = (TDirectory*)gDirectory->FindObjectAny("FCAL2_invmass");
  if(dir) dir->cd();
  else return;
  
  TCanvas *c1=NULL;
  if(gPad == NULL){
    c1=new TCanvas("c1","Two photon mass distributions",900,600);
    c1->cd(0);
    c1->Draw();
    c1->Update();
  }
  else {
    c1 = gPad->GetCanvas();
  }
  if( !gPad ) return;

  c1->Divide(3,1);
  
  TF1 *f1=new TF1("f1","gaus(0)+expo(3)",0.09,0.17);
  f1->SetParNames("height","mean","sigma");
  f1->SetParameters(100,0.125,0.004,2,-10,0);
 
  TH1F *h1=(TH1F*)gDirectory->FindObjectAny("h_2gamma_ECAL_ECAL");
  if (h1){
    c1->cd(1);
    h1->GetXaxis()->SetRangeUser(0.08,0.18);
    h1->SetStats(0);
    f1->SetParameter(0,h1->GetMaximum());
    f1->SetParameter(1,h1->GetBinCenter(h1->GetMaximumBin()));
    f1->SetParameter(2,h1->GetRMS());
    h1->Fit("f1");
    h1->Draw();

    TLine *line=new TLine(0.135,0,0.135,1.05*h1->GetMaximum());
    line->SetLineStyle(10);
    line->Draw();
    
    TPaveText *pt1 = new TPaveText(0.6, 0.65, 0.99, 0.89, "NDC");
    pt1->SetFillColor(0);
    double mean=f1->GetParameter(1)*1000;
    double sigma=fabs(f1->GetParameter(2))*1000;
    pt1->AddText(Form("M_{#pi^{0}} = %.3f MeV",mean));
    pt1->AddText(Form("#sigma_{#pi^{0}} = %.3f MeV",sigma));
    pt1->AddText(Form("#sigma/M = %.3f %%",sigma/mean*100));
    pt1->Draw();
  }
  TH1F *h2=(TH1F*)gDirectory->FindObjectAny("h_2gamma_FCAL_FCAL");
  if (h2){
    c1->cd(2);
    h2->GetXaxis()->SetRangeUser(0.08,0.18);
    h2->SetStats(0);
    f1->SetParameter(3,2);
    f1->SetParameter(4,-10);
    f1->SetParameter(2,0.008);
    f1->SetParameter(1,0.135);
    f1->SetParameter(0,h2->GetMaximum());
    f1->SetParameter(1,h2->GetBinCenter(h2->GetMaximumBin()));
    f1->SetParameter(2,h2->GetRMS());
    h2->Fit("f1");
    h2->Draw();

    TLine *line=new TLine(0.135,0,0.135,1.05*h2->GetMaximum());
    line->SetLineStyle(10);
    line->Draw();
    
    TPaveText *pt2 = new TPaveText(0.6, 0.65, 0.99, 0.89, "NDC");
    pt2->SetFillColor(0);
    double mean=f1->GetParameter(1)*1000;
    double sigma=fabs(f1->GetParameter(2))*1000;
    pt2->AddText(Form("M_{#pi^{0}} = %.3f MeV",mean));
    pt2->AddText(Form("#sigma_{#pi^{0}} = %.3f MeV",sigma));
    pt2->AddText(Form("#sigma/M = %.3f %%",sigma/mean*100));
    pt2->Draw();
  }
  TH1F *h3=(TH1F*)gDirectory->FindObjectAny("h_2gamma_BCAL_BCAL");
  if (h3){
    c1->cd(3);
    f1->SetParameter(1,0.135);
    f1->SetParameter(2,0.01);
    f1->SetParameter(3,2);
    f1->SetParameter(4,-10);
    f1->SetParameter(0,h3->GetMaximum());
    f1->SetParameter(1,h3->GetBinCenter(h3->GetMaximumBin()));
    f1->SetParameter(2,h3->GetRMS());
    h3->GetXaxis()->SetRangeUser(0.08,0.18);
    h3->SetStats(0);
    h3->Fit("f1");
    h3->Draw();

    TLine *line=new TLine(0.135,0,0.135,1.05*h3->GetMaximum());
    line->SetLineStyle(10);
    line->Draw();

    TPaveText *pt3 = new TPaveText(0.6, 0.65, 0.99, 0.89, "NDC");
    pt3->SetFillColor(0);
    double mean=f1->GetParameter(1)*1000;
    double sigma=fabs(f1->GetParameter(2))*1000;
    pt3->AddText(Form("M_{#pi^{0}} = %.3f MeV",mean));
    pt3->AddText(Form("#sigma_{#pi^{0}} = %.3f MeV",sigma));
    pt3->AddText(Form("#sigma/M = %.3f %%",sigma/mean*100));
    pt3->Draw();
  }
}
