void slice_tagh(){


  int plot_fit = 0;

  // Number of counters inside the PS energy range

  int max_count = 200;

  // ID = 2 is the first counter

  char calib_file_adc[120];
  char calib_file_tdc[120];


  sprintf(calib_file_adc,"adc_time_offsets.txt");
  sprintf(calib_file_tdc,"tdc_time_offsets.txt");


  FILE *file1 = fopen(calib_file_adc,"r");
  FILE *file2 = fopen(calib_file_tdc,"r");


  TFile *file = new TFile ("hd_root.root","R");

  gStyle->SetOptFit(1);


  double calib_time  =  0.;

  double  time_tagh[300];
  float   adc_time_db_tagh[300];
  float   tdc_time_db_tagh[300];

  double  cnt_numb[300];

  Double_t  ex[300];


  memset(ex,0,sizeof(ex));

  memset(time_tagh,0.,sizeof(time_tagh));

  memset(adc_time_db_tagh,0,sizeof(adc_time_db_tagh));
  memset(tdc_time_db_tagh,0,sizeof(tdc_time_db_tagh));

  memset(cnt_numb,0,sizeof(cnt_numb));


  Int_t ii = 0;

  char str1[] = "#";

  int ch;
  
  int cnt_tmp;


  while (!feof(file1)){
  
    if ((ch = getc(file1)) != EOF){
  
       if (ch == '#')
         while (getc(file1) != '\n');     
        else
       	  ungetc(ch,file1);
    }
  
  
    int a = fscanf(file1,"%d  %f \n",&cnt_tmp, &adc_time_db_tagh[ii]);

    cout << "ii = " << ii <<  " CNT  = " << cnt_tmp  << "  DB time =  " << adc_time_db_tagh[ii]  << endl;

   
    ii++;

  }
 

  ii = 0;

  while (!feof(file2)){
  
    if ((ch = getc(file2)) != EOF){
  
       if (ch == '#')
         while (getc(file2) != '\n');     
        else
       	  ungetc(ch,file2);
    }
  
  
    int a = fscanf(file2,"%d  %f \n",&cnt_tmp, &tdc_time_db_tagh[ii]);

    cout << "ii = " << ii <<  " CNT  = " << cnt_tmp  << "  DB time  TDC =  " << tdc_time_db_tagh[ii]  << endl;

   
    ii++;

  }


  TH2F *htagh_hit_time   =  (TH2F*)file->Get("TAGH_timewalk/Offsets/tagh_time_rf");


  TH1 *htagh[max_count + 2];


  TCanvas *c1 = new TCanvas("c1","c1", 200, 10, 800, 300);


  for(int ii = 1; ii < max_count + 2; ii++){
    
    char title[30];
    sprintf(title,"htagh%d",ii);
    htagh[ii]    = htagh_hit_time->ProjectionY(title,ii,ii);

    cnt_numb[ii] = ii;

  }



  TF1 *f0 = new TF1("f0","gaus",-200.,200.);
  

  for(int ii = 2; ii < max_count + 2; ii++){

    Int_t nbins   = htagh[ii]->GetNbinsX();

    Int_t entries = htagh[ii]->GetEntries();

    cout << " ID = " << ii - 1 << "  " << entries << endl;

    if(entries > 0){

      // I. Search for peak
      Int_t max_bin = 0;
      Int_t max_amp = 0;

      for(int jj = 1; jj < nbins - 1 ; jj++){
	Int_t cont = htagh[ii]->GetBinContent(jj);
	if(cont > max_amp){
	  max_amp = cont;
	  max_bin = jj;
	}
      }

      double xbin = htagh[ii]->GetXaxis()->GetBinCenter(max_bin);

      double xmin = xbin - 1.;
      double xmax = xbin + 1.;
     

      f0->SetParameters(entries,xbin,1.2);

      htagh[ii]->Fit("f0","","",xmin,xmax);

      double mean = f0->GetParameter(1);	



      time_tagh[ii] = mean;


      //      cout << "Mean = " << mean << " Entries =  " << entries << 
      //	" X max = " << xbin <<  endl;


      if(plot_fit){

	htagh[ii]->Draw();

	c1->Update();	

	//	getchar();

      }
      
    }  // Entries > 0
    else {

      cout << " NO ENTRIES" << endl;
      //      getchar();
    }

  }    // Loop over counters
  
  
  

  
  TGraphErrors *gr1  = new TGraphErrors(max_count + 2,cnt_numb,time_tagh,ex,ex);
  
  gr1->SetMarkerStyle(20);
  gr1->SetMarkerSize(0.6);
  gr1->SetMarkerColor(4);
  
  gr1->SetTitle("PS Hit Time, Arm A");
  gr1->GetXaxis()->SetTitle("PS counter");
  gr1->GetYaxis()->SetTitle("Time (ns)");
  
  gr1->SetTitle("");
  
  gr1->Draw("AP");
  


  char name[200];
  char name1[200];
  
  sprintf(name,"adc_time_offsets_calib.txt");
  sprintf(name1,"tdc_time_offsets_calib.txt");


  FILE *data_file1 = fopen(name,"w");
  FILE *data_file2 = fopen(name1,"w");

  // Write PS times and new values of the adc_time_offsets table to files
  
  cout << endl << endl;
  
  int bad_ch = 0;
  
  for(int ii = 0; ii <  274; ii++){
    
    int counter = ii + 1;
    
    if(counter < max_count){  // Update counters
      
      double new_db_time_adc = adc_time_db_tagh[ii] - (calib_time - time_tagh[ii + 2]);
      double new_db_time_tdc = tdc_time_db_tagh[ii] - (calib_time - time_tagh[ii + 2]);
      
      double dt_tagh = calib_time - time_tagh[ii + 2];
      
      
      if(fabs(dt_tagh) > 2){
	cout << "TAGH:    " <<  counter << "   " <<   dt_tagh <<   "  Old DB   " <<  adc_time_db_tagh[ii] << 
	  "   New DB  " <<  new_db_time_adc  << endl;
	bad_ch++;
      }
      
    
      if(fabs(new_db_time_adc) > 1000.) new_db_time_adc = 0.;
      if(fabs(new_db_time_tdc) > 1000.) new_db_time_tdc = 0.;
            
      fprintf(data_file1, "%d    %12.5f   \n", counter, new_db_time_adc); 
      fprintf(data_file2, "%d    %12.5f   \n", counter, new_db_time_tdc);    

    } else {

      fprintf(data_file1, "%d    %12.5f   \n", counter, adc_time_db_tagh[ii]); 
      fprintf(data_file2, "%d    %12.5f   \n", counter, tdc_time_db_tagh[ii]);  
    }
    
  }

  cout << endl << "Total number of bad channels = " << bad_ch << endl; 

  fclose(data_file1);
  fclose(data_file2);
 
}
  
