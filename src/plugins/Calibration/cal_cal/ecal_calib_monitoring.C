// hnamepath:  /cal_cal/ecal_time_1
// hnamepath:  /cal_cal/ecal_ctime_1
// hnamepath:  /cal_cal/ecal_mgg_v_blk_2

{
  
  TDirectory *dir = (TDirectory*)gDirectory->FindObjectAny("cal_cal");
  if(dir) dir->cd();

  Double_t smallBetween1 = .1;
  Double_t smallBetween2 = .1;
  Double_t smallBetween3 = .1;
  Double_t smallBetween4 = .1;
  gROOT->SetStyle("Bold");
  gStyle->SetCanvasColor(0);
  gStyle->SetLabelColor(1,"X");
  gStyle->SetLabelColor(1,"Y");
  gStyle->SetLabelSize(0.4,"X");
  gStyle->SetLabelSize(0.4,"Y");
  gStyle->SetLabelFont(42,"X");
  gStyle->SetLabelFont(42,"Y");
  gStyle->SetHistLineColor(1); 
  gStyle->SetHistLineWidth(2); 
  gStyle->SetNdivisions(504,"X");
  gStyle->SetNdivisions(504,"Y");
  //gROOT->Macro("setcolor2.c");
  gStyle->SetHistFillColor(999);
  gROOT->SetStyle("Plain");  // white as bg
  gStyle->SetOptStat("111111");
  gStyle->SetFrameLineWidth(2);
  gStyle->SetTitleFontSize(0.1);
  gStyle->SetTitleBorderSize(0);
  gStyle->SetLabelSize(0.04, "XYZ");
  gStyle->SetLabelFont(42, "XYZ");
  gStyle->SetTitleSize(0.05, "XYZ");
  gStyle->SetNdivisions(505, "XYZ");
  gStyle->SetTitleOffset(1.0, "X");
  gStyle->SetTitleOffset(1.2, "Y");
  
  const int kBlocksWide = 59;
  const int kBlocksTall = 59;
  int kMaxChannels = kBlocksWide * kBlocksTall;
  double kMidBlock = ( kBlocksWide - 1 ) / 2;
  double kBeamHoleSize = 3;
  
  double blockSize=4.0157;
  double radius=1.20471*100.;
  double blockLength=45.;
  
  double innerRadius = ( kBeamHoleSize - 1 ) / 2. * blockSize * sqrt(2.)*1.01;
  
  bool   m_activeBlock[kBlocksTall][kBlocksWide];
  TVector2 m_positionOnFace[kBlocksTall+40][kBlocksWide+40];
  double insertBlockSize = 2.09;
  int insert_row_size = 40;
  int m_insertMidBlock=(insert_row_size-1)/2;

  TH2I* ecal_time2D = (TH2I*)gDirectory->FindObjectAny("ecal_time_1");
  TH2I* ecal_ctime2D = (TH2I*)gDirectory->FindObjectAny("ecal_ctime_1");
  TH2I* ecal_mass2D = (TH2I*)gDirectory->FindObjectAny("ecal_mgg_v_blk_2");
  TH1I* ecal_time = (TH1I*) ecal_time2D->ProjectionY("ecal_time");
  TH1I* ecal_ctime = (TH1I*) ecal_ctime2D->ProjectionY("ecal_ctime");
  TH1I* ecal_mass = (TH1I*) ecal_mass2D->ProjectionY("ecal_mass");
  
  TH2F * time_map = new TH2F("time_map", "#font[42]{Events within |t_{RF} - t_{ECAL}| #leq 2.004ns};#font[42]{Row #};#font[42]{Column #};#font[42]{Events #}", 40, -20, 20, 40, -20, 20);
  TH2F * mass_map = new TH2F("mass_map", "#font[42]{Events within 110 #leq m_{#gamma#gamma} #leq 160 MeV};#font[42]{Row #};#font[42]{Column #};#font[42]{Events #}", 40, -20, 20, 40, -20, 20);
  int m_numActiveBlocks = 0;
  for( int row = 0; row < insert_row_size; row++ ){
    for( int col = 0; col < insert_row_size; col++ ){
      // transform to beam axis
      int row_index=row+kBlocksTall;
      int col_index=col+kBlocksWide;
      m_positionOnFace[row_index][col_index] = 
	TVector2(  ( col - m_insertMidBlock -0.5) * insertBlockSize,
		   ( row - m_insertMidBlock -0.5) * insertBlockSize );
      //cout << "radius " << m_positionOnFace[row_index][col_index].Mod() << endl; 
      if( fabs(m_positionOnFace[row_index][col_index].X())>insertBlockSize
	  || fabs(m_positionOnFace[row_index][col_index].Y())>insertBlockSize
	  ){
	double co = ecal_ctime2D->Integral(m_numActiveBlocks + 1, m_numActiveBlocks + 1, ecal_ctime2D->GetYaxis()->FindBin(-2.004), ecal_ctime2D->GetYaxis()->FindBin(2.004));
	time_map->Fill(row - 20, col - 20, co);
	co = ecal_mass2D->Integral(m_numActiveBlocks + 1, m_numActiveBlocks + 1, ecal_mass2D->GetYaxis()->FindBin(0.11), ecal_mass2D->GetYaxis()->FindBin(0.16));
	mass_map->Fill(row - 20, col - 20, co);
	m_numActiveBlocks ++;
      }
    }
  }
  
  TCanvas *c1 = NULL;
  if(gPad == NULL){
    c1 = new TCanvas( "c1", "ecal_calib_monitiring", 800, 800 );
    c1->cd(0);
    c1->Draw();
    c1->Update();
  } else {
      c1 = gPad->GetCanvas();
  }

  smallBetween1 = .15;
  smallBetween2 = .05;
  smallBetween3 = .05;
  smallBetween4 = .15;
  
  if( !gPad ) return;
  c1->Divide(2,2);
  
  if( ecal_time ){

    ecal_time->SetStats(0);
    c1->cd(1);
    smallBetween2 = .05;
    gPad->SetLeftMargin(smallBetween1);
    gPad->SetRightMargin(smallBetween2);
    gPad->SetTopMargin(smallBetween3);
    gPad->SetBottomMargin(smallBetween4);
    ecal_time->GetXaxis()->SetRangeUser(-9.99, 9.99);
    ecal_time->Draw();
    ecal_ctime->SetLineColor(2);
    ecal_ctime->Draw("same");
    //gPad->SetLogy();
    TLegend * legend=new TLegend(0.7, 0.755, 0.8, 0.885);
    legend->AddEntry(ecal_time,  "#font[42]{t_{RF}-t^{Shower}_{ECAL}}","l");
    legend->AddEntry(ecal_ctime, "#font[42]{t_{RF}-t^{Hit}_{ECAL}}","l");
    legend->SetFillColor(0);
    legend->SetTextFont(22);
    legend->SetTextSize(.04);
    legend->SetLineColor(0);
    legend->Draw("same");
  }

  if( time_map ){
    
    time_map->SetStats(0);
    c1->cd(2);
    smallBetween2 = .15;
    smallBetween3 = .175;
    gPad->SetLeftMargin(smallBetween1);
    gPad->SetRightMargin(smallBetween2);
    gPad->SetTopMargin(smallBetween3);
    gPad->SetBottomMargin(smallBetween4);
    time_map->Draw("colz");
    gPad->SetLogz();
  }
 
  if( ecal_mass ){
    
    ecal_mass->SetStats(0);
    c1->cd(3);
    smallBetween2 = .05;
    gPad->SetLeftMargin(smallBetween1);
    gPad->SetRightMargin(smallBetween2);
    gPad->SetTopMargin(smallBetween3);
    gPad->SetBottomMargin(smallBetween4);
    ecal_mass->GetXaxis()->SetRangeUser(0.07, 0.299);
    ecal_mass->Draw();
    //gPad->SetLogy();
  }

  if( mass_map ){
    
    mass_map->SetStats(0);
    c1->cd(4);
    smallBetween2 = .15;
    smallBetween3 = .175;
    gPad->SetLeftMargin(smallBetween1);
    gPad->SetRightMargin(smallBetween2);
    gPad->SetTopMargin(smallBetween3);
    gPad->SetBottomMargin(smallBetween4);
    mass_map->Draw("colz");
    gPad->SetLogz();
  }
}
