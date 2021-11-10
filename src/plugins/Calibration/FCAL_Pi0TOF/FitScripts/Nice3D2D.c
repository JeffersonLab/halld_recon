void Nice3D2D(TH2F *MyHisto,
	      Double_t SizeLabel,
	      Double_t SizeTitle,
	      Int_t TFONT,
	      Int_t NDIV,
	      Double_t OffsetTitleX,
	      Double_t OffsetTitleY,
	      Double_t OffsetTitleZ,
	      TString NameTitle, 
	      TString NameXTitle,
	      TString NameYTitle,
	      TString NameZTitle)
{
  MyHisto->SetTitle(NameTitle);
  MyHisto->SetLabelSize(SizeLabel,"X");
  MyHisto->SetLabelSize(SizeLabel,"Y");
  MyHisto->SetLabelSize(SizeLabel,"Z");
  MyHisto->SetLabelFont(TFONT,"X");
  MyHisto->SetLabelFont(TFONT,"Y");
  MyHisto->SetLabelFont(TFONT,"Z");
  MyHisto->SetTitleSize(SizeTitle);
  MyHisto->SetTitleSize(SizeTitle,"X");
  MyHisto->SetTitleSize(SizeTitle,"Y");
  MyHisto->SetTitleSize(SizeTitle,"Z");
  MyHisto->SetNdivisions(NDIV,"X");
  MyHisto->SetNdivisions(NDIV,"Y");
  MyHisto->SetNdivisions(NDIV,"Z");
  MyHisto->SetTitleOffset(OffsetTitleX,"X");
  MyHisto->SetTitleOffset(OffsetTitleY,"Y");
  MyHisto->SetTitleOffset(OffsetTitleZ,"Z");
  MyHisto->GetXaxis()->CenterTitle(kTRUE);
  MyHisto->GetYaxis()->CenterTitle(kTRUE);
  MyHisto->GetZaxis()->CenterTitle(kTRUE);
  MyHisto->SetXTitle(NameXTitle);
  MyHisto->SetYTitle(NameYTitle);
  MyHisto->SetZTitle(NameZTitle);
}


