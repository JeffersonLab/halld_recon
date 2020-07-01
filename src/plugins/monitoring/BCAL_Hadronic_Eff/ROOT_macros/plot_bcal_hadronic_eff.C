void plot_bcal_hadronic_eff(void)
{
// read output of Read_bcal_hadronic_eff2 dat files with efficiency information and plot vs run number.
// to obtain the list of run numbers run plot_bcal_hadronic_eff.py to obtain the plot_bcal_hadronic_eff.list file
//

#include <TRandom.h>

gROOT->Reset();
//TTree *Bfield = (TTree *) gROOT->FindObject("Bfield");
gStyle->SetPalette(1,0);
// gStyle->SetOptStat(kFALSE);
// gStyle->SetOptStat(11111111);
gStyle->SetOptFit(111111);

gStyle->SetPadRightMargin(0.15);
gStyle->SetPadLeftMargin(0.15);
gStyle->SetPadBottomMargin(0.15);
//

   char string[256];
    Int_t const maxruns=1000;
    
    /*vector <float> eff_up1(maxruns,0.);
    vector <float> eff_up2(maxruns,0.);
    vector <float> eff_up3(maxruns,0.);
    vector <float> eff_up4(maxruns,0.);
    
    vector <float> eff_down1(maxruns,0.);
    vector <float> eff_down2(maxruns,0.);
    vector <float> eff_down3(maxruns,0.);
    vector <float> eff_down4(maxruns,0.);*/
    
    Float_t eff_up1[maxruns]={0,0,0,0,0,0};
    Float_t eff_up2[maxruns]={0,0,0,0,0,0};
    Float_t eff_up3[maxruns]={0,0,0,0,0,0};
    Float_t eff_up4[maxruns]={0,0,0,0,0,0};
    
    Float_t eff_down1[maxruns]={0,0,0,0,0,0};
    Float_t eff_down2[maxruns]={0,0,0,0,0,0};
    Float_t eff_down3[maxruns]={0,0,0,0,0,0};
    Float_t eff_down4[maxruns]={0,0,0,0,0,0};
    
    /*Float_t runnum[maxruns]={10492,10498,10590,10598,10659,10778,10873,10907,11082,11128,11157,11264,11300,11405,11436,11529,11659,
        11366,11384,11407,11432,11436,11450,11473,11483,11510,11520,11553,
        30734,30801,30890,30830,30833,30834,30835,30836,30838,30900,30902,30903,30926,30927,30928,30930,
        30274,30280.30300,30350,30405,30450,30497,30592,30610,30650,30961,31000,31029,31056};
    
    map<Int_t, TString> runs;
    runs[0]="010492";
    runs[1]="010498";
    runs[2]="010590";
    runs[3]="010598";
    runs[4]="010659";
    runs[5]="010778";
    runs[6]="010873";
    runs[7]="010907";
    runs[8]="011082";
    runs[9]="011128";
    runs[10]="011157";
    runs[11]="011264";
    runs[12]="011300";
    runs[13]="011405";
    runs[14]="011436";
    runs[15]="011529";
    runs[16]="011659";
    runs[17]="011366";
    runs[18]="011384";
    runs[19]="011407";
    runs[20]="011432";
    runs[21]="011436";
    runs[22]="011450";
    runs[23]="011473";
    runs[24]="011483";
    runs[25]="011510";
    runs[26]="011520";
    runs[27]="011553";
    runs[28]="030734";
    runs[29]="030801";
    runs[30]="030890";
    runs[31]="030830";
    runs[32]="030833";
    runs[33]="030834";
    runs[34]="030835";
    runs[35]="030836";
    runs[36]="030838";
    runs[37]="030900";
    runs[38]="030902";
    runs[39]="030903";
    runs[40]="030926";
    runs[41]="030927";
    runs[42]="030928";
    runs[43]="030930";
    runs[44]="030274";
    runs[45]="030280";
    runs[46]="030300";
    runs[47]="030350";
    runs[48]="030405";
    runs[49]="030450";
    runs[50]="030497";
    runs[51]="030592";
    runs[52]="030610";
    runs[53]="030650";
    runs[54]="030961";
    runs[55]="031000";
    runs[56]="031029";
    runs[57]="031056";*/

    // Get run numbers and efficiency from file

    TString runlistfile="plot_bcal_hadronic_eff.list";
    ifstream runlist;
    runlist.open (runlistfile.Data());   // open list of run numbers
    if (!runlist) {
      cout << "ERROR: Failed to open data file= " << runlistfile.Data() << endl;
       return;
    }
    else { 
      cout << "Open data file= " << runlistfile.Data() << endl;
       }
    Float_t runnum[maxruns];
    
    map<Int_t, TString> runs;
    
    Int_t ndx=0;
    Int_t nruns;
    TString line;
   while (line.ReadLine(runlist)){
       TObjArray *tokens = line.Tokenize(" ");
       Int_t ntokens = tokens->GetEntries();
       cout << " ndx=" << ndx << endl;
       if (ndx == 0) {
           nruns = ((TObjString*)tokens->At(0))->GetString().Atof();
           // cout << " nruns=" << nruns << endl;
	   ndx++;
	   continue;
       }
       TString run = ((TObjString*)tokens->At(0))->GetString();
       runs[ndx-1] = run;
       // cout << " run=" << run << endl;
       ndx++;
   }


   if (nruns > maxruns) {
     cout << "*** plot_bcal_hadronic_eff ***nruns=" << nruns << " maxruns=" << maxruns << endl;
     exit(1);
   }

    
    Int_t layer;
    Int_t coinc_cut=3;     // nominal is 3
  
    TString datfile;
    
    for (Int_t jrun=0; jrun<nruns; jrun++) {
    
    
    for (layer=1; layer<5; layer++) {
    
      datfile = "dat/R0"+runs[jrun]+"_layer"+TString::Itoa(layer,10)+"_cut"+TString::Itoa(coinc_cut,10)+".dat";
      // datfile = "dat/R"+runs[jrun]+"_layer"+TString::Itoa(layer,10)+"_cut"+TString::Itoa(coinc_cut,10)+".dat";
    
    cout << "Opening file: " << datfile.Data() << endl;

   ifstream file;
   file.open (datfile.Data());
   if (!file) {
       cout << "ERROR: Failed to open data file= " << datfile.Data() << endl;
       return;
       }
        
    ndx=0;
   while (line.ReadLine(file)){
       
       cout << "line=" << line << endl;
       
       TObjArray *tokens = line.Tokenize(" ");
       Int_t ntokens = tokens->GetEntries();
       // Float_t  = (((TObjString*)tokens->At(1))->GetString()).Atof();
       // cout << " token1=" << ((TObjString*)tokens->At(0))->GetString()  << " token2=" << ((TObjString*)tokens->At(1))->GetString() << endl;
       if (ndx == 0) {
           Float_t run = ((TObjString*)tokens->At(1))->GetString().Atof();
           Float_t xlayer = ((TObjString*)tokens->At(3))->GetString().Atof();
           Float_t xcoinc_cut = ((TObjString*)tokens->At(5))->GetString().Atof();
           cout << " run=" << run << " xlayer=" << xlayer << " xcoinc_cut=" << xcoinc_cut << endl;
           runnum[jrun] = run;
       }
       if (ndx == 1) {
           Float_t eff_up = ((TObjString*)tokens->At(7))->GetString().Atof();
           Float_t eff_down = ((TObjString*)tokens->At(9))->GetString().Atof();
           cout << " eff_up=" << eff_up << " eff_down=" << eff_down << endl;
           
           switch (layer) {
               case 1:
                   eff_up1[jrun] = eff_up;
                   eff_down1[jrun] = eff_down;
                   break;
               case 2:
                   eff_up2[jrun] = eff_up;
               		eff_down2[jrun] = eff_down;
                   break;
               case 3:
                   eff_up3[jrun] = eff_up;
                   eff_down3[jrun] = eff_down;
                   break;
               case 4:
                   eff_up4[jrun] = eff_up;
                   eff_down4[jrun] = eff_down;
                   break;
               default:
                   cout << "*** Illegal layer=" << layer << endl;
                   return;
           }  // end switch
       } // end if
       
       ndx++;
       
   }   // end reading over dat file

   file.close();
        
    }
    }
    
    cout << "up1[0]=" << eff_up1[0] << endl;
    cout << "up1[1]=" << eff_up1[1] << endl;
    cout << "up1[2]=" << eff_up1[2] << endl;
    cout << " nruns=" << nruns << endl;

    for (j=0; j<nruns; j++) {
      // cout << "j=" << j << " runnum=" << runnum[j] << " eff up   =" << eff_up1[j] << " " << eff_up2[j] << " " << eff_up3[j] << " " << eff_up4[j] <<  endl;
      // cout << "j=" << j << " runnum=" << runnum[j] << " eff downn=" << eff_down1[j] << " " << eff_down2[j] << " " << eff_down3[j] << " " << eff_down4[j] <<  endl;
      cout << "j=" << j << " runnum=" << runnum[j] << " eff diff =" << eff_up1[j]-eff_down1[j] << " " << eff_up2[j]-eff_down2[j] << " " << eff_up3[j]-eff_down3[j] << " " << eff_up4[j]-eff_down4[j] <<  endl;
    }
    
    TGraph *gr_eff_up1 = new TGraph(nruns,runnum,eff_up1);
    TGraph *gr_eff_up2 = new TGraph(nruns,runnum,eff_up2);
    TGraph *gr_eff_up3 = new TGraph(nruns,runnum,eff_up3);
    TGraph *gr_eff_up4 = new TGraph(nruns,runnum,eff_up4);
    
    TGraph *gr_eff_down1 = new TGraph(nruns,runnum,eff_down1);
    TGraph *gr_eff_down2 = new TGraph(nruns,runnum,eff_down2);
    TGraph *gr_eff_down3 = new TGraph(nruns,runnum,eff_down3);
    TGraph *gr_eff_down4 = new TGraph(nruns,runnum,eff_down4);
    
    
    TCanvas *c0 = new TCanvas("c0", "c0",200,10,1000,700);
    c0->SetGridx();
    c0->SetGridy();


    ymin = 0.4;
    ymax = 1.0;
       
    gr_eff_up1->SetTitle("");
    // gr_eff_up1->GetXaxis()->SetRangeUser(xmin,xmax);
    gr_eff_up1->GetYaxis()->SetRangeUser(ymin,ymax);
    gr_eff_up1->GetXaxis()->SetTitleSize(0.05);
    gr_eff_up1->GetYaxis()->SetTitleSize(0.05);
    gr_eff_up1->GetYaxis()->SetTitle("Efficiency");
    gr_eff_up1->GetXaxis()->SetTitle("Run Number");
    gr_eff_up1->SetMarkerColor(4);
    gr_eff_up1->SetMarkerStyle(20);
    gr_eff_up1->Draw("Ap");
    
    gr_eff_up2->SetMarkerColor(2);
    gr_eff_up2->SetMarkerStyle(20);
    gr_eff_up2->Draw("psame");
    
    gr_eff_up3->SetMarkerColor(1);
    gr_eff_up3->SetMarkerStyle(20);
    gr_eff_up3->Draw("psame");
    
    gr_eff_up4->SetMarkerColor(3);
    gr_eff_up4->SetMarkerStyle(20);
    gr_eff_up4->Draw("psame");
    
    TLegend *leg = new TLegend(0.65,0.45,0.8,0.6);
    leg->AddEntry(gr_eff_up1,"Up Layer 1","p");
    leg->AddEntry(gr_eff_up2,"Up Layer 2","p");
    leg->AddEntry(gr_eff_up3,"Up Layer 3","p");
    leg->AddEntry(gr_eff_up4,"Up Layer 4","p");
    leg->Draw();

    
    TCanvas *c20 = new TCanvas("c20", "c20",200,10,1000,700);
    c20->SetGridx();
    c20->SetGridy();


    ymin = 0.4;
    ymax = 1.0;
       
    gr_eff_down1->SetTitle("");
    // gr_eff_down1->GetXaxis()->SetRangeUser(xmin,xmax);
    gr_eff_down1->GetYaxis()->SetRangeUser(ymin,ymax);
    gr_eff_down1->GetXaxis()->SetTitleSize(0.05);
    gr_eff_down1->GetYaxis()->SetTitleSize(0.05);
    gr_eff_down1->GetYaxis()->SetTitle("Efficiency");
    gr_eff_down1->GetXaxis()->SetTitle("Run Number");
    gr_eff_down1->SetMarkerColor(4);
    gr_eff_down1->SetMarkerStyle(20);
    gr_eff_down1->Draw("Ap");
    
    gr_eff_down2->SetMarkerColor(2);
    gr_eff_down2->SetMarkerStyle(20);
    gr_eff_down2->Draw("psame");
    
    gr_eff_down3->SetMarkerColor(1);
    gr_eff_down3->SetMarkerStyle(20);
    gr_eff_down3->Draw("psame");
    
    gr_eff_down4->SetMarkerColor(3);
    gr_eff_down4->SetMarkerStyle(20);
    gr_eff_down4->Draw("psame");
    
    TLegend *leg20 = new TLegend(0.65,0.45,0.8,0.6);
    leg20->AddEntry(gr_eff_down1,"Down Layer 1","p");
    leg20->AddEntry(gr_eff_down2,"Down Layer 2","p");
    leg20->AddEntry(gr_eff_down3,"Down Layer 3","p");
    leg20->AddEntry(gr_eff_down4,"Down Layer 4","p");
    leg20->Draw();
    
    
    
    TCanvas *c1 = new TCanvas("c1", "c1",200,10,1000,700);
    
    TGraph *gr_eff_up1_copy = (TGraph*)gr_eff_up1->Clone("gr_eff_up1_copy");
    c1->SetGridx();
    c1->SetGridy();

    // 2017 ranges    
    // xmin = 10000;
    // xmax = 12000;

    // 2018 ranges    
    xmin = 40600;
    xmax = 41700;
    ymin = 0.9;
    ymax = 1.0;

    // 2019 ranges    
    xmin = 71300;
    xmax = 72500;
    ymin = 0.9;
    ymax = 1.0;
    
    gr_eff_up1_copy->GetYaxis()->SetRangeUser(0.9,1.0);
    gr_eff_up1_copy->GetXaxis()->SetRangeUser(xmin,xmax);
    // gr_eff_up1_copy->GetXaxis()->SetRangeUser(30200,31050);
    
    gr_eff_up1_copy->Draw("Ap");
    gr_eff_up2->Draw("psame");
    gr_eff_up3->Draw("psame");
    gr_eff_up4->Draw("psame");

    TLegend *leg1 = new TLegend(0.7,0.75,0.85,0.9);
    leg1->AddEntry(gr_eff_up1,"Up Layer 1","p");
    leg1->AddEntry(gr_eff_up2,"Up Layer 2","p");
    leg1->AddEntry(gr_eff_up3,"Up Layer 3","p");
    leg1->AddEntry(gr_eff_up4,"UP Layer 4","p");
    leg1->Draw();
    
    
    TLine *linea = new TLine(41200,ymin,41200,ymax);
    linea->SetLineWidth(2);
    linea->Draw();
    
    TLatex *t1 = new TLatex(40900,1.001,"ver07");    // t1->SetNDC();
    t1->SetTextSize(0.03);
    t1->Draw();
    t1->DrawLatex(41400,1.001,"ver11");
    t1->Draw();

    linea = new TLine(40700,0.95,41700,0.95);

    // linea->Draw();


   
    
    TCanvas *c21 = new TCanvas("c21", "c21",200,10,1000,700);
    
    TGraph *gr_eff_down1_copy = (TGraph*)gr_eff_down1->Clone("gr_eff_down1_copy");
    c21->SetGridx();
    c21->SetGridy();

    // 2017 ranges    
    // xmin = 10000;
    // xmax = 12000;

    // 2018 ranges    
    xmin = 40600;
    xmax = 41700;
    ymin = 0.9;
    ymax = 1.0;

    // 2019 ranges    
    xmin = 71300;
    xmax = 72500;
    ymin = 0.9;
    ymax = 1.0;
    
    gr_eff_down1_copy->GetYaxis()->SetRangeUser(0.9,1.0);
    gr_eff_down1_copy->GetXaxis()->SetRangeUser(xmin,xmax);
    // gr_eff_down1_copy->GetXaxis()->SetRangeUser(30200,31050);
    
    gr_eff_down1_copy->Draw("Ap");
    gr_eff_down2->Draw("psame");
    gr_eff_down3->Draw("psame");
    gr_eff_down4->Draw("psame");

    TLegend *leg21 = new TLegend(0.7,0.75,0.85,0.9);
    leg21->AddEntry(gr_eff_down1,"Down Layer 1","p");
    leg21->AddEntry(gr_eff_down2,"Down Layer 2","p");
    leg21->AddEntry(gr_eff_down3,"Down Layer 3","p");
    leg21->AddEntry(gr_eff_down4,"Dowb Layer 4","p");
    leg21->Draw();
    
    
    /*TLine *linea = new TLine(41200,ymin,41200,ymax);
    linea->SetLineWidth(2);
    linea->Draw();
    
    TLatex *t1 = new TLatex(40900,1.001,"ver07");    // t1->SetNDC();
    t1->SetTextSize(0.03);
    t1->Draw();
    t1->DrawLatex(41400,1.001,"ver11");
    t1->Draw();*/
    


    TCanvas *c11 = new TCanvas("c11", "c11",200,10,1000,1000);
    
    TGraph *gr_eff_up1_copy2 = (TGraph*)gr_eff_up1->Clone("gr_eff_up1_copy2");
    c11->Divide(1,3);

    c11->cd(1);
    gPad->SetGridx();
    gPad->SetGridy();
    gr_eff_up1_copy2->GetYaxis()->SetRangeUser(0.925,0.945);
    gr_eff_up1_copy2->GetXaxis()->SetRangeUser(xmin,xmax);
    gr_eff_up1_copy2->GetXaxis()->SetTitleSize(0.07);
    gr_eff_up1_copy2->GetYaxis()->SetTitleSize(0.07);
    gr_eff_up1_copy2->GetXaxis()->SetLabelSize(0.07);
    gr_eff_up1_copy2->GetYaxis()->SetLabelSize(0.07);
    gr_eff_up1_copy2->GetYaxis()->SetNdivisions(505);
    gr_eff_up1_copy2->Fit("pol1");
    gr_eff_up1_copy2->Draw("Ap");

    TLegend *leg11 = new TLegend(0.2,0.75,0.35,0.9);
    leg11->AddEntry(gr_eff_up1_copy2,"Up Layer 1","p");
    leg11->Draw();

    TGraph *gr_eff_up2_copy2 = (TGraph*)gr_eff_up2->Clone("gr_eff_up2_copy2");
    c11->cd(2);
    gPad->SetGridx();
    gPad->SetGridy();
    gr_eff_up2_copy2->GetYaxis()->SetRangeUser(0.955,0.975);
    gr_eff_up2_copy2->GetXaxis()->SetRangeUser(xmin,xmax);
    gr_eff_up2_copy2->GetXaxis()->SetTitleSize(0.07);
    gr_eff_up2_copy2->GetYaxis()->SetTitleSize(0.07);
    gr_eff_up2_copy2->GetXaxis()->SetLabelSize(0.07);
    gr_eff_up2_copy2->GetYaxis()->SetLabelSize(0.07);
    gr_eff_up2_copy2->GetYaxis()->SetNdivisions(505);
    gr_eff_up2_copy2->Fit("pol1");
    gr_eff_up2_copy2->Draw("Ap");

    TLegend *leg12 = new TLegend(0.2,0.75,0.35,0.9);
    leg12->AddEntry(gr_eff_up2_copy2,"Up Layer 2","p");
    leg12->Draw();

    TGraph *gr_eff_up3_copy2 = (TGraph*)gr_eff_up3->Clone("gr_eff_up3_copy2");
    c11->cd(3);
    gPad->SetGridx();
    gPad->SetGridy();
    gr_eff_up3_copy2->GetYaxis()->SetRangeUser(0.95,0.97);
    gr_eff_up3_copy2->GetXaxis()->SetRangeUser(xmin,xmax);
    gr_eff_up3_copy2->GetXaxis()->SetTitleSize(0.07);
    gr_eff_up3_copy2->GetYaxis()->SetTitleSize(0.07);
    gr_eff_up3_copy2->GetXaxis()->SetLabelSize(0.07);
    gr_eff_up3_copy2->GetYaxis()->SetLabelSize(0.07);
    gr_eff_up3_copy2->GetYaxis()->SetNdivisions(505);
    gr_eff_up3_copy2->Fit("pol1");
    gr_eff_up3_copy2->Draw("Ap");

    TLegend *leg13 = new TLegend(0.2,0.75,0.35,0.9);
    leg13->AddEntry(gr_eff_up3_copy2,"Up Layer 3","p");
    leg13->Draw();


    TCanvas *c22 = new TCanvas("c22", "c22",200,10,1000,1000);
    
    TGraph *gr_eff_down1_copy2 = (TGraph*)gr_eff_down1->Clone("gr_eff_down1_copy2");
    c22->Divide(1,3);

    c22->cd(1);
    gPad->SetGridx();
    gPad->SetGridy();
    gr_eff_down1_copy2->GetYaxis()->SetRangeUser(0.925,0.945);
    gr_eff_down1_copy2->GetXaxis()->SetRangeUser(xmin,xmax);
    gr_eff_down1_copy2->GetXaxis()->SetTitleSize(0.07);
    gr_eff_down1_copy2->GetYaxis()->SetTitleSize(0.07);
    gr_eff_down1_copy2->GetXaxis()->SetLabelSize(0.07);
    gr_eff_down1_copy2->GetYaxis()->SetLabelSize(0.07);
    gr_eff_down1_copy2->GetYaxis()->SetNdivisions(505);
    gr_eff_down1_copy2->Fit("pol1");
    gr_eff_down1_copy2->Draw("Ap");

    TLegend *leg22a = new TLegend(0.2,0.75,0.35,0.9);
    leg22a->AddEntry(gr_eff_down1_copy2,"Down Layer 1","p");
    leg22a->Draw();

    TGraph *gr_eff_down2_copy2 = (TGraph*)gr_eff_down2->Clone("gr_eff_down2_copy2");
    c22->cd(2);
    gPad->SetGridx();
    gPad->SetGridy();
    gr_eff_down2_copy2->GetYaxis()->SetRangeUser(0.955,0.975);
    gr_eff_down2_copy2->GetXaxis()->SetRangeUser(xmin,xmax);
    gr_eff_down2_copy2->GetXaxis()->SetTitleSize(0.07);
    gr_eff_down2_copy2->GetYaxis()->SetTitleSize(0.07);
    gr_eff_down2_copy2->GetXaxis()->SetLabelSize(0.07);
    gr_eff_down2_copy2->GetYaxis()->SetLabelSize(0.07);
    gr_eff_down2_copy2->GetYaxis()->SetNdivisions(505);
    gr_eff_down2_copy2->Fit("pol1");
    gr_eff_down2_copy2->Draw("Ap");

    TLegend *leg22b = new TLegend(0.2,0.75,0.35,0.9);
    leg22b->AddEntry(gr_eff_down2_copy2,"Down Layer 2","p");
    leg22b->Draw();

    TGraph *gr_eff_down3_copy2 = (TGraph*)gr_eff_down3->Clone("gr_eff_down3_copy2");
    c22->cd(3);
    gPad->SetGridx();
    gPad->SetGridy();
    gr_eff_down3_copy2->GetYaxis()->SetRangeUser(0.95,0.97);
    gr_eff_down3_copy2->GetXaxis()->SetRangeUser(xmin,xmax);
    gr_eff_down3_copy2->GetXaxis()->SetTitleSize(0.07);
    gr_eff_down3_copy2->GetYaxis()->SetTitleSize(0.07);
    gr_eff_down3_copy2->GetXaxis()->SetLabelSize(0.07);
    gr_eff_down3_copy2->GetYaxis()->SetLabelSize(0.07);
    gr_eff_down3_copy2->GetYaxis()->SetNdivisions(505);
    gr_eff_down3_copy2->Fit("pol1");
    gr_eff_down3_copy2->Draw("Ap");

    TLegend *leg22c = new TLegend(0.2,0.75,0.35,0.9);
    leg22c->AddEntry(gr_eff_down3_copy2,"Down Layer 3","p");
    leg22c->Draw();




    /* 2016 ranges

    TCanvas *c2 = new TCanvas("c2", "c2",200,10,1000,700);
    
    TGraph *gr_eff_up1_copy2 = (TGraph*)gr_eff_up1->Clone("gr_eff_up1_copy");
    gr_eff_up1_copy2->GetYaxis()->SetRangeUser(ymin,ymax);
    gr_eff_up1_copy2->GetXaxis()->SetRangeUser(xmin,xmax);
    
    gr_eff_up1_copy2->Draw("Ap");
    gr_eff_up2->Draw("psame");
    gr_eff_up3->Draw("psame");
    gr_eff_up4->Draw("psame");
    leg->Draw();
    
    
    TLine *line1 = new TLine(11059,ymin,11059,ymax);
    line1->Draw();
    TLine *line2 = new TLine(11366,ymin,11366,ymax);
    line2->Draw();
    TLine *line3 = new TLine(11555,ymin,11555,ymax);
    line3->Draw();

    t1->DrawLatex(11059,1.001,"CDC Hole");    // t1->SetNDC();
    t1->SetTextSize(0.03);
    t1->Draw();
    t1->DrawLatex(11366,1.001,"Golden");
    t1->DrawLatex(11655,1.001,"low rate");
    t1->DrawLatex(10495,1.001,"2016");*/
    
    c0->SaveAs("plot_bcal_hadronic_eff.pdf(");
    c1->SaveAs("plot_bcal_hadronic_eff.pdf");
    c11->SaveAs("plot_bcal_hadronic_eff.pdf");
    c20->SaveAs("plot_bcal_hadronic_eff.pdf");
    c21->SaveAs("plot_bcal_hadronic_eff.pdf");
    c22->SaveAs("plot_bcal_hadronic_eff.pdf)");
    
}

