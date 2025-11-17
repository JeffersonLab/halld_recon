// hnamepath:  /cal_cal/fcal_time_1
// hnamepath:  /cal_cal/fcal_ctime_1
// hnamepath:  /cal_cal/fcal_mgg_v_blk_2
{
  
  TDirectory *dir = (TDirectory*)gDirectory->FindObjectAny("cal_cal");
  if(dir) dir->cd();
  /*
  Double_t smallBetween1 = .1;
  Double_t smallBetween2 = .1;
  Double_t smallBetween3 = .1;
  Double_t smallBetween4 = .1;
  gROOT->SetStyle("Bold");
  gStyle->SetCanvasColor(0);
  gStyle->SetLabelColor(1,"X");
  gStyle->SetLabelColor(1,"Y");
  gStyle->SetLabelSize(0.04,"X");
  gStyle->SetLabelSize(0.04,"Y");
  gStyle->SetLabelFont(42,"X");
  gStyle->SetLabelFont(42,"Y");
  gStyle->SetHistLineColor(1); 
  gStyle->SetHistLineWidth(2); 
  gStyle->SetNdivisions(504,"X");
  gStyle->SetNdivisions(504,"Y");
  //gROOT->Macro("setcolor2.c");
  gStyle->SetFrameLineWidth(2);
  gStyle->SetTitleFontSize(0.04);
  gStyle->SetTitleBorderSize(0);
  gStyle->SetHistFillColor(999);
  gROOT->SetStyle("Plain");  // white as bg
  gStyle->SetOptStat("111111");
  gStyle->SetLabelSize(0.04, "XYZ");
  gStyle->SetLabelFont(42, "XYZ");
  gStyle->SetTitleSize(0.05, "XYZ");
  gStyle->SetNdivisions(505, "XYZ");
  gStyle->SetTitleOffset(1.0, "X");
  gStyle->SetTitleOffset(1.2, "Y");
  */
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
  
  bool m_activeBlock[kBlocksTall][kBlocksWide];
  TVector2 m_positionOnFace[kBlocksTall+40][kBlocksWide+40];
  
  TH2I* fcal_time2D = (TH2I*)gDirectory->FindObjectAny("fcal_time_1");
  TH2I* fcal_ctime2D = (TH2I*)gDirectory->FindObjectAny("fcal_ctime_1");
  TH2I* fcal_mass2D = (TH2I*)gDirectory->FindObjectAny("fcal_mgg_v_blk_2");
  TH1I* fcal_time = (TH1I*) fcal_time2D->ProjectionY("fcal_time");
  TH1I* fcal_ctime = (TH1I*) fcal_ctime2D->ProjectionY("fcal_ctime");
  TH1I* fcal_mass = (TH1I*) fcal_mass2D->ProjectionY("fcal_mass");
  
  TH2F * time_map = new TH2F("time_map", "#font[42]{Events within |t_{FCAL} - t_{REF}| #leq 3.004ns};#font[42]{Row #};#font[42]{Column #};#font[42]{Events #}", 59, -30, 30, 59, -30, 30);
  TH2F * mass_map = new TH2F("mass_map", "#font[42]{Events within 110 #leq m_{#gamma#gamma} #leq 160 MeV};#font[42]{Row #};#font[42]{Column #};#font[42]{Events #}", 59, -30, 30, 59, -30, 30);
  int m_numActiveBlocks = 0;
  std::vector <int> bad_blk, real_bad_blk;
  for (int row = 0; row < kBlocksTall; row ++) {
    for (int col = 0; col < kBlocksWide; col ++) {
      m_positionOnFace[row][col] =TVector2(  ( col - kMidBlock ) * blockSize,( row - kMidBlock ) * blockSize );
      double thisRadius = m_positionOnFace[row][col].Mod();
      if (innerRadius < thisRadius && thisRadius < radius) {
	double co = fcal_time2D->Integral(m_numActiveBlocks + 1, m_numActiveBlocks + 1, fcal_time2D->GetYaxis()->FindBin(-3.004), fcal_time2D->GetYaxis()->FindBin(3.004));
	time_map->Fill(row - 29, col - 29, co);
	if (co == 0) {
	  bad_blk.push_back(m_numActiveBlocks);
	  co = fcal_time2D->Integral(m_numActiveBlocks + 1, m_numActiveBlocks + 1, fcal_time2D->GetYaxis()->FindBin(-200.004), fcal_time2D->GetYaxis()->FindBin(200.004));
	  if (co == 0) real_bad_blk.push_back(m_numActiveBlocks);
	}
	co = fcal_mass2D->Integral(m_numActiveBlocks + 1, m_numActiveBlocks + 1, fcal_mass2D->GetYaxis()->FindBin(0.11), fcal_mass2D->GetYaxis()->FindBin(0.16));
	mass_map->Fill(row - 29, col - 29, co);
	m_numActiveBlocks++;
      }
    }
  }
  TCanvas *c1 = NULL;
  if(gPad == NULL){
    c1 = new TCanvas( "c1", "fcal_calib_monitiring", 800, 800 );
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
  
  if( fcal_time ){

    fcal_time->SetStats(0);
    c1->cd(1);
    smallBetween2 = .05;
    gPad->SetLeftMargin(smallBetween1);
    gPad->SetRightMargin(smallBetween2);
    gPad->SetTopMargin(smallBetween3);
    gPad->SetBottomMargin(smallBetween4);
    fcal_time->GetXaxis()->SetRangeUser(-9.99, 9.99);
    fcal_time->Draw();
    fcal_ctime->SetLineColor(2);
    fcal_ctime->Draw("same");
    TLegend * legend=new TLegend(0.7, 0.755, 0.8, 0.885);
    legend->AddEntry(fcal_time,  "#font[42]{t^{Shower}_{FCAL} - t_{RF}}","l");
    legend->AddEntry(fcal_ctime, "#font[42]{t^{Hit}_{FCAL} - t_{RF}}","l");
    legend->SetFillColor(0);
    legend->SetTextFont(22);
    legend->SetTextSize(.04);
    legend->SetLineColor(0);
    legend->Draw("same");

    //gPad->SetLogy();
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
 
  if( fcal_mass ){
    
    fcal_mass->SetStats(0);
    c1->cd(3);
    smallBetween2 = .05;
    gPad->SetLeftMargin(smallBetween1);
    gPad->SetRightMargin(smallBetween2);
    gPad->SetTopMargin(smallBetween3);
    gPad->SetBottomMargin(smallBetween4);
    fcal_mass->GetXaxis()->SetRangeUser(0.07, 0.299);
    fcal_mass->Draw();
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
  std::cout << " bad blk " << bad_blk.size()
	    << " real bad blk " << real_bad_blk.size()
	    << std::endl;
  for (int i = 0; i < (int) bad_blk.size(); i ++) std::cout << bad_blk[i] << std::endl;
}
