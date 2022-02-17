{
  const int nplanes = 24;
  TH1D *hh[nplanes][2];
  for (int i = 0; i < nplanes; ++i) {
    hh[i][0] = (TH1D *)gDirectory->Get(Form("TrackingPulls/FDCPulls_Plane%02d/wire_residual_right", i + 1));
    hh[i][1] = (TH1D *)gDirectory->Get(Form("TrackingPulls/FDCPulls_Plane%02d/wire_residual_left", i + 1));
    hh[i][0]->SetLineColor(2);
    hh[i][1]->SetLineColor(3);
    hh[i][0]->SetLineWidth(2);
    hh[i][1]->SetLineWidth(2);
  }

  TCanvas *c1 = new TCanvas("c1", "c1", 2400, 1600);
  c1->Divide(6, 4);
  TH1 *frame[nplanes];
  TLine *line[nplanes];
  TLatex *latex = new TLatex();
  latex->SetTextSize(0.2);
  for (int i = 0; i < nplanes; ++i) {
    double ymax = TMath::Max(hh[i][0]->GetMaximum(), hh[i][1]->GetMaximum()) * 1.05;
    if (ymax < 1.0) ymax = 1.0;
    frame[i] = c1->cd(i + 1)->DrawFrame(-0.09999, 0.0, 0.1, ymax);
    frame[i]->GetXaxis()->SetNdivisions(504);
    frame[i]->GetXaxis()->SetTitle("(cm)");
    latex->DrawLatexNDC(0.15, 0.7, Form("%d", i + 1));
    line[i] = new TLine(0.0, 0.0, 0.0, ymax);
    line[i]->SetLineStyle(3);
    line[i]->Draw();
    for (int j = 0; j < 2; ++j) hh[i][j]->Draw("same");
  }
  c1->cd();
  latex->SetTextSize(0.05);
  latex->DrawLatexNDC(0.25, 0.48, "FDC wire residuals for each plane");
}
