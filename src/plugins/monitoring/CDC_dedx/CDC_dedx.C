
// The following are special comments used by RootSpy to know
// which histograms to fetch for the macro.
//
// hnamepath: /CDC_dedx/dedx_p_pos
// hnamepath: /CDC_dedx/dedx_p_neg
// hnamepath: /CDC_dedx/dedx_i_p_pos
// hnamepath: /CDC_dedx/dedx_i_p_neg


// By default, use the dE/dx from amplitude histos
// Set integral=1 to switch to integral instead 


void CDC_dedx(int integral=0) {

  gStyle->SetCanvasDefW(1000);
  gStyle->SetCanvasDefH(1000);
  gStyle->SetOptStat(0);
  gStyle->SetOptFit(0);
  gStyle->SetFuncWidth(1);
  gStyle->SetFuncColor(6);


 TDirectory *CDCdir = (TDirectory*)gDirectory->FindObjectAny("CDC_dedx");
  if (!CDCdir) printf("Cannot find directory CDC_dedx\n"); 
  if (!CDCdir) return;
  CDCdir->cd();

  char whichdedx[20];
  char hname_pos[20];
  char hname_neg[20];

  if (integral == 0) {
    sprintf(whichdedx,"");
    sprintf(hname_pos,"dedx_p_pos");
    sprintf(hname_neg,"dedx_p_neg");
  } else {
    sprintf(whichdedx," (integ)");
    sprintf(hname_pos,"intdedx_p_pos");
    sprintf(hname_neg,"intdedx_p_neg");
  }  

  TH2I *h = (TH2I*)CDCdir->Get(hname_pos);
  if (!h) printf("Cannot find histogram %s\n",hname_pos);
  if (!h) return;

  TH2I *hn = (TH2I*)CDCdir->Get(hname_neg);
  if (!hn) printf("Cannot find histogram %s\n",hname_neg);
  if (!hn) return;

  if(gPad == NULL){
    TCanvas *c1 = new TCanvas("c1");
    c1->cd(0);
    c1->Draw();
    c1->Update();
  }
	
  if(!gPad) return;

  TCanvas *c1 = gPad->GetCanvas();
  c1->Divide(2,2);

  const float ymin=0;
  const float ymax=12;


  c1->cd(1);

  gPad->SetLogz();
  h->Draw("colz");
 
  c1->cd(2);

  gPad->SetLogz();
  hn->Draw("colz");

  c1->cd(4);

  TF1 *g = new TF1("gaus","gaus",0,ymax);

  double pars[3];
  double res;
  double resp = 0;
  double respi = 0; 


  // draw cut through histo at p=1.5 GeV/c

  float pcut = 1.5;
  int pbin = h->GetXaxis()->FindBin(pcut);

  TH1D *p = h->ProjectionY("p1",pbin,pbin);

  p->SetTitle(Form("CDC q+ dE/dx%s at %.2f GeV/c",whichdedx,pcut));
  p->GetXaxis()->SetRangeUser(ymin,ymax);
  p->DrawCopy("");

  double fitstat = p->Fit(g,"Q0WE");  // fitstat=0 is good
  double mean = 0;
  
  if (fitstat == 0) {
    g->GetParameters(&pars[0]);
    res = 2.0*pars[2]/pars[1];
    mean = pars[1];

    g->DrawCopy("same");

    TPaveText *txt1 = new TPaveText(0.4,0.75,0.8,0.85,"NDC");
    txt1->AddText(Form("Mean %.2f  width %.2f  res %.2f",pars[1],2*pars[2],res));
    txt1->SetBorderSize(0);
    txt1->SetFillStyle(0);
    txt1->Draw();

  }

  
  c1->cd(3);

  // draw cut through histo at p=0.5 GeV/c

  pcut = 0.5;
  pbin = h->GetXaxis()->FindBin(pcut);
  p = h->ProjectionY("p1",pbin,pbin);

  p->SetTitle(Form("CDC q+ dE/dx%s at %.2f GeV/c",whichdedx,pcut));
  p->GetXaxis()->SetRangeUser(ymin,ymax);
  p->DrawCopy("");

  if (mean ==0) return; // don't try to fit these if the earlier fit failed

  TPaveText *txt2 = new TPaveText(0.4,0.75,0.8,0.85,"NDC");
  txt2->SetBorderSize(0);
  txt2->SetFillStyle(0);


  fitstat = p->Fit(g,"Q0WE","",0,2*mean);  

  if (fitstat == 0) {
    g->GetParameters(&pars[0]);
    respi = 2.0*pars[2]/pars[1];

    g->DrawCopy("same");

    txt2->AddText(Form("Mean %.2f  width %.2f  res %.2f",pars[1],2*pars[2],respi));

  }


  fitstat = p->Fit(g,"Q0WE","",2*mean,ymax);

  if (fitstat == 0) {
    g->GetParameters(&pars[0]);
    resp = 2.0*pars[2]/pars[1];

    g->DrawCopy("same");

    txt2->AddText("");
    txt2->AddText(Form("Mean %.2f  width %.2f  res %.2f",pars[1],2*pars[2],resp));

  }


  txt2->Draw();


}



