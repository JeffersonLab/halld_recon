void Nice1D(TH1F *MyHisto, 
	    Double_t SizeLabel, 
	    Double_t SizeTitle, 
	    Int_t TFONT, 
	    Int_t NDIV,
	    Double_t OffsetTitleX,
	    Double_t OffsetTitleY,
	    TString NameTitle, 
	    TString NameXTitle,
	    TString NameYTitle)
{
  MyHisto->SetTitle(NameTitle);
  MyHisto->SetLabelSize(SizeLabel,"X");
  MyHisto->SetLabelSize(SizeLabel,"Y");
  MyHisto->SetLabelFont(TFONT,"X");
  MyHisto->SetLabelFont(TFONT,"Y");
  MyHisto->SetTitleSize(SizeTitle,"X");
  MyHisto->SetTitleSize(SizeTitle,"Y");
  MyHisto->SetNdivisions(NDIV,"X");
  MyHisto->SetNdivisions(NDIV,"Y");
  MyHisto->SetTitleOffset(OffsetTitleX,"X");
  MyHisto->SetTitleOffset(OffsetTitleY,"Y");
  MyHisto->GetXaxis()->CenterTitle(kTRUE);
  MyHisto->GetYaxis()->CenterTitle(kTRUE);
  MyHisto->SetXTitle(NameXTitle);
  MyHisto->SetYTitle(NameYTitle);
}


