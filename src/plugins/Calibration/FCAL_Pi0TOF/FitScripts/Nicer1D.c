void Nicer1D(TH1F *MyHisto, 
	     Double_t Size, 
	     Int_t TFONT, 
	     Int_t NDIV,
	     Double_t OffsetTitleX,
	     Double_t OffsetTitleY)
{
  //MyHisto->SetTitle("");
  MyHisto->SetLabelSize(Size,"X");
  MyHisto->SetLabelSize(Size,"Y");
  MyHisto->SetLabelFont(TFONT,"X");
  MyHisto->SetLabelFont(TFONT,"Y");
  MyHisto->SetTitleSize(Size,"X");
  MyHisto->SetTitleSize(Size,"Y");
  MyHisto->SetNdivisions(NDIV,"X");
  MyHisto->SetNdivisions(NDIV,"Y");
  MyHisto->SetTitleOffset(OffsetTitleX,"X");
  MyHisto->SetTitleOffset(OffsetTitleY,"Y");
  MyHisto->GetXaxis()->CenterTitle(kTRUE);
  MyHisto->GetYaxis()->CenterTitle(kTRUE);
}


