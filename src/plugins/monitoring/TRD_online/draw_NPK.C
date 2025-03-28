const Bool_t verbose = 0;

void draw_NPK(TString root_file = "/work/halld2/home/lkasper/dsoft/halldrecon/halldrecon/src/hd_root.root") {
	
	TFile *f = TFile::Open(root_file);
	TDirectory *dir = (TDirectory*)f->Get("TRD");
	if (verbose) cout<<"First directory: "<<dir->GetName()<<endl;
	TCanvas *c1 = new TCanvas("c1","c1",1600,800);
    c1->Divide(2,1);
    TString pdfFile = "hd_root_NPK_output.pdf";
	if (verbose) cout<<"Output file: "<<pdfFile<<endl;
	
	TDirectory *Digidir = (TDirectory*)dir->Get("DigiHit");
	TH1I *hDigiHit_NPKs;
	if (verbose) cout<<"First subdirectory: "<<Digidir->GetName()<<endl;
	hDigiHit_NPKs = (TH1I*)Digidir->Get("DigiHit_NPKs");
	
	for (int np=15; np>0; np--) {
        int cont=hDigiHit_NPKs->GetBinContent(np);
        for (int i=1; i<np; i++) {
            int cont_i=hDigiHit_NPKs->GetBinContent(i);
            if (verbose) cout<<"DIGIHIT***  np, i, cont, cont_i= "<<np<<" "<<i<<" "<<cont<<" "<<cont_i<<endl;
            hDigiHit_NPKs->SetBinContent(i,cont_i-cont);
        }
    }   
    
    c1->cd(1);
	gPad->SetLogy(1);
	gStyle->SetOptStat(1111111111);
    hDigiHit_NPKs->GetXaxis()->SetTitle("Num Peaks");
	hDigiHit_NPKs->SetMinimum(1);
    hDigiHit_NPKs->GetYaxis()->SetTitle("Events");
    hDigiHit_NPKs->SetTitle("DigiHit # Peaks");
    hDigiHit_NPKs->Draw();
	
	TDirectory *Hitdir = (TDirectory*)dir->Get("Hit");
	TH1I *hHit_NPKs;
	if (verbose) cout<<"Second subdirectory: "<<Hitdir->GetName()<<endl;
	hHit_NPKs = (TH1I*)Hitdir->Get("Hit_NPKs");
	
	for (int np=15; np>0; np--) {
        int cont=hHit_NPKs->GetBinContent(np);
        for (int i=1; i<np; i++) {
            int cont_i=hHit_NPKs->GetBinContent(i);
            if (verbose) cout<<"HIT***  np, i, cont, cont_i= "<<np<<" "<<i<<" "<<cont<<" "<<cont_i<<endl;
            hHit_NPKs->SetBinContent(i,cont_i-cont);
        }
    }
    
    c1->cd(2);
	gPad->SetLogy(1);
    hHit_NPKs->GetXaxis()->SetTitle("Num Peaks");
	hHit_NPKs->SetMinimum(1);
    hHit_NPKs->GetYaxis()->SetTitle("Events");
    hHit_NPKs->SetTitle("Hit # Peaks");
    hHit_NPKs->Draw();
	
	c1->Print(pdfFile);
	f->Close();
	
}
