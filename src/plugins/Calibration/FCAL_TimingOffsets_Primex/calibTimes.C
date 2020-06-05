

void plotFCALChannels( TH2I *h2, TCanvas *c2, vector<int> chanVec ) {
	
	
	gStyle->SetOptStat(0);
	
	int n_chans  = (int)chanVec.size();	
	
	const int kBlocksTall    = 59;
	const int kBlocksWide    = 59;
	const int kBeamHoleSize  =  3;
	
	int kMaxChannels = kBlocksWide * kBlocksTall;
	int kMidBlock    = ( kBlocksWide - 1 ) / 2;
	
	const double blockSize  = 4.0157;
	const double radius     = 120.471;
	
	bool     m_activeBlock[kBlocksTall][kBlocksWide];
	
	int      m_channelNumber[kBlocksTall][kBlocksWide];
	int      m_row[kMaxChannels];
	int      m_column[kMaxChannels];
	
	TVector2 m_positionOnFace[kBlocksTall][kBlocksWide];
	
	
	double innerRadius = ( kBeamHoleSize - 1 ) / 2. * blockSize * sqrt(2.);
	innerRadius *= 1.01;
	
	int m_numActiveBlocks = 0;
	
	for( int row = 0; row < kBlocksTall; row++ ){
		for( int col = 0; col < kBlocksWide; col++ ){
			
			// transform to beam axis
			m_positionOnFace[row][col] = 
			   TVector2(  ( col - kMidBlock ) * blockSize,
					     ( row - kMidBlock ) * blockSize );
			
			double thisRadius = m_positionOnFace[row][col].Mod();
			
			if( ( thisRadius < radius ) && ( thisRadius > innerRadius ) ){

				m_activeBlock[row][col] = true;
				
				// build the "channel map"
				m_channelNumber[row][col] = m_numActiveBlocks;
				m_row[m_numActiveBlocks] = row;
				m_column[m_numActiveBlocks] = col;

				m_numActiveBlocks++;
			}
			else{
				
				m_activeBlock[row][col] = false;
			}
		}
	}
	
	
	
	c2->cd();
	h2->Draw();
	
	
	
	for( int irow = 0; irow<59; irow++ ) {
		for( int icol = 0; icol<59; icol++ ) {
			
			if( !m_activeBlock[irow][icol] ) continue;
			TBox *b = new TBox( icol, irow, icol+1, irow+1 );
			b->SetFillColor(kGray);
   			b->Draw();
			
		}
	}
	
	
	
	
	for( int irow = 0; irow<59; irow++ ) {
		for( int icol = 0; icol<59; icol++ ) {
			
			if( !m_activeBlock[irow][icol] ) continue;
			
			int chanNum   =  m_channelNumber[irow][icol];
			
			int find_flag = 0;
			for( int ic = 0; ic < n_chans; ic++ ) {
				if( chanVec[ic] == chanNum ) {
					find_flag = 1;
					break;
				}
			}
			
			if( find_flag ) {
				
				TBox *b = new TBox( icol, irow, icol+1, irow+1 );
				b->SetFillColor( kBlue+2 );
   				b->Draw();
			}
		}
	}
	
	
	
	
	
	
	for( int irow = 0; irow<59; irow++ ) {
		
		if( irow == 28 || irow==29 || irow==30 ) continue;
		
		int xmin = 0;
		for( int icol = 0; icol<59; icol++ ) {
			if( m_activeBlock[irow][icol] ) {
				xmin = icol; break;
			}
		}
		int xmax = 0;
		for( int icol = xmin; icol<70; icol++ ) {
			if( !m_activeBlock[irow][icol] ) {
				xmax = icol; break;
			}
		}
		if( xmin==xmax ) { xmax = 59; }
		
		
		if( irow < 28 ) {
			TLine *l1 = new TLine( xmin, irow, xmax, irow ); 
			l1->SetLineColor( kBlack ); l1->Draw();			
		} else {
			TLine *l1 = new TLine( xmin, irow+1, xmax, irow+1 ); 
			l1->SetLineColor( kBlack ); l1->Draw();
		}
	}
	
	
	
	for( int icol = 0; icol<59; icol++ ) {
		
		if( icol==28 || icol==29 || icol==30 ) continue;
		
		int ymin = 0;
		for( int irow = 0; irow<59; irow++ ) {
			if( m_activeBlock[irow][icol] ) {
				ymin = irow; break;
			}
		}
		int ymax = 0;
		for( int irow = ymin; irow<59; irow++ ) {
			if( !m_activeBlock[irow][icol] ) {
				ymax = irow; break;
			}
		}
		if( ymin==ymax ) { ymax = 59; }
		
		
		if( icol < 28 ) {
			TLine *l1 = new TLine( icol, ymin, icol, ymax ); 
			l1->SetLineColor( kBlack ); l1->Draw();
		} else {
			TLine *l1 = new TLine( icol+1, ymin, icol+1, ymax ); 
			l1->SetLineColor( kBlack ); l1->Draw();
		}
	}
	
	
	
	TLine *l0  = new TLine(  0, 28, 59, 28 ); l0->SetLineColor( kBlack ); l0->Draw();	
	TLine *l1  = new TLine(  0, 29, 28, 29 ); l1->SetLineColor( kBlack ); l1->Draw();
	TLine *l2  = new TLine( 31, 29, 59, 29 ); l2->SetLineColor( kBlack ); l2->Draw();	
	TLine *l3  = new TLine(  0, 30, 28, 30 ); l3->SetLineColor( kBlack ); l3->Draw();
	TLine *l4  = new TLine( 31, 30, 59, 30 ); l4->SetLineColor( kBlack ); l4->Draw();	
	TLine *l5  = new TLine(  0, 31, 59, 31 ); l5->SetLineColor( kBlack ); l5->Draw();
	
	
	TLine *l6  = new TLine( 28,  0, 28, 59 ); l6->SetLineColor( kBlack ); l6->Draw();	
	TLine *l7  = new TLine( 29,  0, 29, 28 ); l7->SetLineColor( kBlack ); l7->Draw();
	TLine *l8  = new TLine( 29, 31, 29, 59 ); l8->SetLineColor( kBlack ); l8->Draw();	
	TLine *l9  = new TLine( 30,  0, 30, 28 ); l9->SetLineColor( kBlack ); l9->Draw();
	TLine *l10 = new TLine( 30, 31, 30, 59 ); l10->SetLineColor( kBlack ); l10->Draw();	
	TLine *l11 = new TLine( 31,  0, 31, 59 ); l11->SetLineColor( kBlack ); l11->Draw();
	
	
	
	return;
}










void calibTimes() {
	
	
	
	gStyle->SetOptFit(1);
	gStyle->SetOptStat(0);
	
	
	
	//----------------------    ADJUST THESE PARAMETERS   ----------------------//
	
	
	const int DRAW_FITS        =   0; 
	const int MINIMUM_ENTRIES  = 300; // minimum number of entries for calibrating each channel
	
	
	const int CALIB_WITH_BEAM  =   0;
		/*
		if set to 1, this will calibrate the fcal timing offsets according to the 
		beam photon time instead of the RF time
		*/
	
	
	
	const double MIN_TIME_DIFF =  0.;
		/*
		Set this parameter to the minimum (absolute) value that the mean of the FCAL - RF 
		time histogram should be to change the already existing constants.
		
		For example, if the mean value is 0.001 ns the current timing_offsets value for 
		that channel is probably good as is.
		*/
	
	
	
	
	char old_offset_fname[256] = "fcal_times_old.dat";
		// pathname to file with current /FCAL/timing_offsets
	
	
	char new_offset_fname[256] = "fcal_times_new.dat";
		// pathname to file where the new /FCAL/timing_offsets will be written
	
	
	char root_fname[256]       = "Full.root";
		// pathname to the root file used for calibration:
	
	
	
	
	//----------------------      Read Initial Offsets    ----------------------//
	
	
	double old_offsets[2800];
	
	ifstream infile( old_offset_fname );
	for( int i=0; i<2800; i++ ) {
		
		double a; infile >> a;
		old_offsets[i] = a;
		
	}
	infile.close();
	
	
	
	
	//----------------------         Read ROOT File       ----------------------//
	
	
	
	cout << "\n\n";
	cout << "FCAL Timing Offsets Calibration: " << endl;
	cout << "   Old offsets stored in:          " << old_offset_fname << endl;
	cout << "   New offsets will be written to: " << new_offset_fname << endl;
	cout << "   ROOT File used for calibration: " << root_fname << endl;
	cout << "\n\n";
	
	
	
	TFile *fIn = new TFile( root_fname, "READ" );
	TH2I  *h2;
	
	if( CALIB_WITH_BEAM )
		h2 = (TH2I*)fIn->Get( "FCAL_TimingOffsets/fcal_beam_dt_VS_chan" );
	else 
		h2 = (TH2I*)fIn->Get( "FCAL_TimingOffsets/fcal_rf_dt_VS_chan" );
	
	
	
	// Setup our fit function:
	
	TF1 *fGaus = new TF1( "fGaus", "gaus", 0., 1. );
	fGaus->SetLineColor( kRed );
	
	
	
	// Canvas for drawing fits to:
	
	TCanvas *cFits = new TCanvas( "cFits", "cFits", 1000, 1000 );
	cFits->Divide( 6,6 );
	
	TPad *pFits[36];
	for( int i=0; i<36; i++ ) {
		pFits[i] = (TPad*)cFits->cd(i+1);
		pFits[i]->SetTickx(); pFits[i]->SetTicky();
	}
	
	
	
	double new_offsets[2800];
	
	vector< int > chanVec;
	vector< double > timeVec;
	
	vector< int > skippedChanVec;
	
	
	// Loop over all channels:
	
	int counter = 0;
	
	for( int ich = 0; ich < 2800; ich++ ) {
		
		
		TH1I *h1 = (TH1I*)h2->ProjectionY( Form("h1_%d",ich), ich+1, ich+1 );
		if( h1->GetEntries() < 150 ) h1->Rebin( 2 );
		h1->GetXaxis()->SetRangeUser( -6., 6. );
		h1->SetTitle( Form("Channel %d",ich) );
		h1->GetXaxis()->SetTitle( "t_{FCAL} - t_{RF} [ns]" );
		
		
		
		// skip channels that don't have enough statistics:
		
		if( h1->GetEntries() < MINIMUM_ENTRIES ) {
			
        		new_offsets[ich] = old_offsets[ich];
			
			pFits[ich%36]->cd();
			h1->Draw();
			
			if( DRAW_FITS ) {
				if( (ich+1)%36 == 0 ) {
					//cFits->SaveAs(Form("c_%.2i.png",ich/36));
					if( ich==35 ) cFits->Print( "cFits.pdf(", "pdf" );
					else 	      cFits->Print( "cFits.pdf",  "pdf" );
					cFits->Clear("D");
         			}
			}
			
			skippedChanVec.push_back( ich );
			
         		continue;
      		}
		
		
		
		
		
		// Find the peak  --->  fit using a Gaussian with a very narrow fit range:
		
		double loc_amp, loc_time, loc_width;
		
		loc_amp   =  (double)h1->GetMaximum();
		loc_time  =  h1->GetBinCenter( h1->GetMaximumBin() );
		loc_width =  0.2;
		
		fGaus->SetParameters( loc_amp, loc_time, loc_width );
		fGaus->SetRange( loc_time-0.5, loc_time+0.5 );
		
		
		/*
		if( ich==9 ) {
			fGaus->SetParameters( 600., 1.6, 0.3 );
			fGaus->SetRange( 1.1, 2.5 );
		} else if( ich==  73 ) {
			fGaus->SetParameters( 2500., 0.75, 0.3 );
			fGaus->SetRange( 0.3, 1.5 );
		} else if( ich== 472 ) {
			fGaus->SetParameters( 700., 1.5, 0.3 );
			fGaus->SetRange( 1.0, 2.0 );
		} else if( ich== 553 ) {
			fGaus->SetParameters( 8000., 1.2, 0.3 );
			fGaus->SetRange( 0.7, 2.0 );
		} else if( ich== 703 ) {
			fGaus->SetParameters( 7000., 0., 0.3 );
			fGaus->SetRange( -0.5, 0.7 );
		} else if( ich== 948 ) {
			fGaus->SetParameters( 2500., 0., 0.3 );
			fGaus->SetRange( -0.4, 0.7 );
		} else if( ich== 961 ) {
			fGaus->SetParameters( 40., 0., 0.3 );
			fGaus->SetRange( -0.7, 0.7 );
		} else if( ich==1896 ) {
			fGaus->SetParameters( 350., 2., 0.3 );
			fGaus->SetRange( 1.0, 3.0 );
		} else if( ich==2591 ) {
			fGaus->SetParameters( 1800., 0., 0.3 );
			fGaus->SetRange( -0.4, 1.0 );
		}
		*/
		
		
		h1->Fit( "fGaus", "R0Q" );
		
		loc_time  =  fGaus->GetParameter(1);
		loc_width =  fGaus->GetParameter(2);
		
		fGaus->SetRange( loc_time - 1.*loc_width, loc_time + 2.*loc_width );
		
		/*
		if( ich==9 ) {
			fGaus->SetRange(  1.1, 2.5 );
		} else if( ich==  73 ) {
			fGaus->SetRange(  0.4, 1.5 );
		} else if( ich== 553 ) {
			fGaus->SetRange(  0.7, 2.0 );
		} else if( ich== 703 ) {
			fGaus->SetRange( -0.5, 0.7 );
		} else if( ich==2591 ) {
			fGaus->SetRange( -0.4, 1.0 );
		}
		*/
		
		TFitResultPtr result = h1->Fit( "fGaus", "SR0QL" );
		
		
		
		
		
		// Make sure the fit result converged:
		
		if( (Int_t)result == 0 ) {
			
			// fit converged, check chi^2/ndof:
			
			double reducedChi2 = result->Chi2() / result->Ndf();
			
			if( reducedChi2 < 100. ) {
				
				double loc_shift = fGaus->GetParameter(1);
				
				if( fabs(loc_shift) > MIN_TIME_DIFF ) {
					new_offsets[ich] = old_offsets[ich] + loc_shift;
				} else {
					new_offsets[ich] = old_offsets[ich];
				}
				
				chanVec.push_back( ich );
				timeVec.push_back( loc_shift );
				
			} else {
				
				/*
				this doesn't really do anything new, so if a channel get's
				to this point, then the fit will probably not converge in the end
				*/
				
				
				loc_time  = h1->GetBinCenter( h1->GetMaximumBin() );
				loc_width = 0.3;
				fGaus->SetRange( loc_time - 1.*loc_width, loc_time + 2.*loc_width );
				
				TFitResultPtr new_result = h1->Fit( "fGaus", "SR0Q" );
				double newChi2 = new_result->Chi2() / new_result->Ndf();
				
				if( newChi2 < 100. ) {
					
				  	double loc_shift = fGaus->GetParameter(1);
					
					if( fabs(loc_shift) > MIN_TIME_DIFF ) {
						new_offsets[ich] = old_offsets[ich] + loc_shift;
					} else {
						new_offsets[ich] = old_offsets[ich];
					}
					
					chanVec.push_back( ich );
					timeVec.push_back( loc_shift );
					
				} else {
					
					new_offsets[ich] = old_offsets[ich];
					cout << "Channel " << ich << " : CHI2 TOO LARGE ";
					cout << "  ( " << result->Chi2() << " / " << result->Ndf() 
						<< " )" << endl;
					
					skippedChanVec.push_back( ich );
					
				}
				
			}
			
		} else {
			
			new_offsets[ich] = old_offsets[ich];
			cout << "Channel " << ich << " : FIT DOES NOT CONVERGE" << endl;
			
			skippedChanVec.push_back( ich );
			
		}
		
		
		
		// Draw the results:
		
		pFits[ich%36]->cd();
		h1->GetXaxis()->SetRangeUser( loc_time-2., loc_time+2. );
		h1->Draw();
		
		TF1 *fDraw = (TF1*)fGaus->Clone( Form("fDraw_%d",ich) );
		fDraw->SetLineColor( kRed );
		fDraw->SetRange( loc_time-2., loc_time+2. );
		fDraw->SetLineStyle( 2 );
		fDraw->Draw( "same" );
		
		/*
		if( ich==   9 ) cFits->Update();
		if( ich==  73 ) cFits->Update();
		if( ich== 553 ) cFits->Update();
		if( ich== 703 ) cFits->Update();
		if( ich== 472 ) cFits->Update();
		if( ich== 948 ) cFits->Update();
		if( ich== 961 ) cFits->Update();
		if( ich==1896 ) cFits->Update();
		if( ich==2591 ) cFits->Update();
		*/
		if( ich== 9  ) cFits->Update();
		
		if( DRAW_FITS ) {
			if( (ich+1)%36 == 0 ) {
				//cFits->SaveAs(Form("c_%.2i.png",ich/36));
				if( ich==35 ) cFits->Print( "cFits.pdf(", "pdf" );
				else 	      cFits->Print( "cFits.pdf",  "pdf" );
				cFits->Clear("D");
			}
		}
		
	}
	
	cout << "\n\n";
	//if( DRAW_FITS ) cFits->SaveAs("c_final.png");
	if( DRAW_FITS ) cFits->Print( "cFits.pdf)",  "pdf" );
	
	
	
	
	//----------------------         Draw Results        ----------------------//
	
	
	int nchans    = static_cast<int>( chanVec.size() );
	double *chans = new double[nchans];
	double *times = new double[nchans];
	
	for( int i=0; i<nchans; i++ ) {
		
	 	chans[i] = static_cast<double>( chanVec[i] );
		times[i] = timeVec[i];
		
	}
	
	
	
	TGraph *gr = new TGraph( nchans, chans, times );
	gr->SetMarkerStyle(7);
	gr->SetMarkerColor(kBlue+2);
	gr->GetXaxis()->SetTitle( "FCAL Channel Number" );
	gr->GetYaxis()->SetTitle( "t_{FCAL} - t_{RF} [ns]" );
	gr->SetTitle( "FCAL Timing Shifts" );
	
	TCanvas *c1 = new TCanvas( "c1", "c1", 1200, 600 );
	c1->SetTickx(); c1->SetTicky();
	gr->Draw( "AP" );
	
	
	
	// write the udpated /FCAL/timing_offsets to a text file:
	
	ofstream outfile( new_offset_fname );
	for( int i=0; i<2800; i++ ) {
		outfile << new_offsets[i] << "\n";
	}
	outfile.close();
	
	
	
	
	// plot the FCAL channels which were NOT calibrated 
	
	TCanvas *c2 = new TCanvas( "c2", "c2", 800, 800 );
	
	TH2I *h_FCAL_unCalib = new TH2I( "FCAL_unCalib", "Uncalibrated FCAL Channels", 
		59, 0, 59, 59, 0, 59 );
	h_FCAL_unCalib->GetXaxis()->SetTitle( "FCAL Column" );
	h_FCAL_unCalib->GetYaxis()->SetTitle( "FCAL Row"    );
	h_FCAL_unCalib->SetDirectory( 0 );
	
	
	plotFCALChannels( h_FCAL_unCalib, c2, skippedChanVec );
	
	
	
	
	
	
	
	
	//fIn->Close();
	
	
	
	
	
	return;
}
