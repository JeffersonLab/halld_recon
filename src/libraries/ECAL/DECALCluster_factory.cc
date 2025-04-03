// $Id$
//
//    File: DECALCluster_factory.cc
// Created: Tue Mar 25 10:45:18 AM EDT 2025
// Creator: staylor (on Linux ifarm2401.jlab.org 5.14.0-503.19.1.el9_5.x86_64 x86_64)
//

#include <iostream>
#include <iomanip>

#include "DECALCluster_factory.h"
#include <JANA/JEvent.h>

//------------------
// Init
//------------------
void DECALCluster_factory::Init()
{
  auto app = GetApplication();

  TIME_CUT=15.;
  app->SetDefaultParameter("ECAL:TIME_CUT",TIME_CUT,"time cut for associating FCAL hits together into a cluster");
  
  MIN_EXCESS_SEED_ENERGY=0.035;
  app->SetDefaultParameter("ECAL:MIN_EXCESS_SEED_ENERGY",
			   MIN_EXCESS_SEED_ENERGY);
  MIN_CLUSTER_SEED_ENERGY=0.035;
  app->SetDefaultParameter("ECAL:MIN_CLUSTER_SEED_ENERGY",
			   MIN_CLUSTER_SEED_ENERGY);
  MIN_CUTDOWN_FRACTION=0.1;
  app->SetDefaultParameter("ECAL:MIN_CUTDOWN_FRACTION",
			      MIN_CUTDOWN_FRACTION);
  CHISQ_MARGIN=12.5;
  app->SetDefaultParameter("ECAL:CHISQ_MARGIN",CHISQ_MARGIN);

  SPLIT_PEAKS=true;
  app->SetDefaultParameter("ECAL:SPLIT_PEAKS",SPLIT_PEAKS);

  // Shower shape parameters
  app->SetDefaultParameter("ECAL:SHOWER_WIDTH_PAR0",SHOWER_WIDTH_PAR0);
  app->SetDefaultParameter("ECAL:SHOWER_WIDTH_PAR1",SHOWER_WIDTH_PAR1);
}

//------------------
// BeginRun
//------------------
void DECALCluster_factory::BeginRun(const std::shared_ptr<const JEvent>& event)
{
  event->GetSingle(dECALGeom);
}

//------------------
// Process
//------------------
void DECALCluster_factory::Process(const std::shared_ptr<const JEvent>& event)
{
  vector<const DECALHit*>ecal_hits;
  event->Get(ecal_hits);
 
  unsigned int nhits=ecal_hits.size();
  if (nhits==0) return;

  // LED events will have hits in nearly every channel. Do NOT
  // try clusterizing if more than 250 hits in ECAL
  if (nhits > 250) return;

  // Put hit information into local array
  vector<HitInfo>hits;
  for (unsigned int i=0;i<nhits;i++){
    const DECALHit *myhit=ecal_hits[i];
    DVector2 pos=dECALGeom->positionOnFace(myhit->row,myhit->column);
    hits.push_back(HitInfo(myhit->row,myhit->column,myhit->E,pos.X(),pos.Y(),myhit->t));
  }

  // Sort the hits according to energy.
  stable_sort(hits.begin(),hits.end(),
	      [&](const HitInfo a,const HitInfo b){return a.E>b.E;});
  
  // Associate groups of adjacent hits into cluster candidates
  vector<vector<HitInfo>>clusterCandidates;
  FindClusterCandidates(hits,clusterCandidates);
  
  for (unsigned int i=0;i<clusterCandidates.size();i++){
    // The list of hits in the cluster
    vector<HitInfo>clusterHits=clusterCandidates[i];
    unsigned int num_hits=clusterHits.size();
    
    // Do not attempt a fit if the cluster size is too small
    if (num_hits<4){
      // Create a new DECALCluster object and add it to the mData list
      DECALCluster *myCluster= new DECALCluster;
   
      double Etot=0.,t=0,x=0,y=0;
      for (unsigned int k=0;k<num_hits;k++){
	double E=clusterHits[k].E;
 	Etot+=E;
	t+=E*clusterHits[k].t;
	x+=E*clusterHits[k].x;
	y+=E*clusterHits[k].y;
      }
      x/=Etot;
      y/=Etot;
      t/=Etot;

      myCluster->E=Etot;
      myCluster->Efit=Etot;
      myCluster->t=t;
      myCluster->x=x;
      myCluster->y=y;
      myCluster->status=DECALCluster::SHOWER_FOUND;

      Insert(myCluster);
      
      continue;
    }
    
    //------------------------------------------------------------------------
    // Handle cluster candidates containing more than 3 hits
    //------------------------------------------------------------------------

    // Find the minimum and maximum row and column numbers
    int min_row=1000,min_col=1000,max_row=0,max_col=0;
    for (unsigned int j=0;j<num_hits;j++){
      if (clusterHits[j].row<min_row) min_row=clusterHits[j].row;
      if (clusterHits[j].column<min_col) min_col=clusterHits[j].column;
      if (clusterHits[j].row>max_row) max_row=clusterHits[j].row;
      if (clusterHits[j].column>max_col) max_col=clusterHits[j].column;
    }

    // Create arrays to represent the cluster of hits to aid in peak search
    int num_rows=max_row-min_row+3;
    int num_cols=max_col-min_col+3;
    vector<vector<double>>Emap(num_cols,vector<double>(num_rows));
    vector<vector<unsigned int>>imap(num_cols,vector<unsigned int>(num_rows));
    
    // Info for block with maximum energy
    double Emax=0.;
    int ir_max=0,ic_max=0;
    for (unsigned int j=0;j<num_hits;j++){
      int ic=clusterHits[j].column-min_col+1;
      int ir=clusterHits[j].row-min_row+1;
      Emap[ic][ir]=clusterHits[j].E;
      imap[ic][ir]=j;
      if (clusterHits[j].E>Emax){
	Emax=clusterHits[j].E;
	ir_max=ir;
	ic_max=ic;
      }
    }
    // Skip to the next cluster if Emax is less than a seed threshold
    if (Emax<MIN_CLUSTER_SEED_ENERGY) continue;
    
    // Create a vector of peaks and fill peak info for peak corresponding to
    // the maximimum energy
    vector<PeakInfo>peak_candidates;
    double Epeak=0;
    int lo_col=0,hi_col=0,lo_row=0,hi_row=0;
    // Maximum distance in row/column number from peak
    int idiff=1;
    GetRowColRanges(idiff,num_rows,num_cols,ir_max,ic_max,lo_row,hi_row,lo_col,
		    hi_col);
    for (int my_ir=lo_row;my_ir<=hi_row;my_ir++){
      for (int my_ic=lo_col;my_ic<=hi_col;my_ic++){
	double E=Emap[my_ic][my_ir];
	if (E>0) Epeak+=E;
      }
    }
    // x and y will be filled in later
    peak_candidates.push_back(PeakInfo(Epeak,0.,0.,ic_max,ir_max,0));
    
    // Loop over hits looking for more peaks
    if (num_hits>6){
      for (int ic=1;ic<num_cols-1;ic++){
	for (int ir=1;ir<num_rows-1;ir++){
	  if (ir==ir_max && ic==ic_max) continue;
	  
	  double E=Emap[ic][ir];
	  if (E>MIN_CLUSTER_SEED_ENERGY){
	    bool got_peak=true;
	    int nhits_in_peak=1;
	    GetRowColRanges(idiff,num_rows,num_cols,ir,ic,lo_row,hi_row,lo_col,
			    hi_col);
	      for (int j=lo_col;j<=hi_col;j++){
		for (int k=lo_row;k<=hi_row;k++){
		  if (j==ic && k==ir) continue;
		  
		  double Ejk=Emap[j][k];
		  if (Ejk<0.001) continue;
		  
		  nhits_in_peak++;
		  got_peak=(E-Ejk>0)&&got_peak;
		}
	      }
	      if (got_peak){
		// Skip small upward energy fluctuation at boundary if
		// the block with the local maximum energy is not
		// surrounded by enough blocks
		if (ic==1 || ir==1 || ic==num_cols-2 || ir==num_rows-2){
		  //cout << "At boundary " << nhits_in_peak << endl;
		  if (nhits_in_peak<3)
		    continue;
		}
		Epeak=0;
		GetRowColRanges(idiff,num_rows,num_cols,ir,ic,lo_row,
				hi_row,lo_col,hi_col);
		for (int my_ir=lo_row;my_ir<=hi_row;my_ir++){
		  for (int my_ic=lo_col;my_ic<=hi_col;my_ic++){
		    double E=Emap[my_ic][my_ir];
		    if (E>0) Epeak+=E;
		  }
		}
		// x and y will be filled in later
		peak_candidates.push_back(PeakInfo(Epeak,0.,0.,ic,ir,0));
		if (num_hits<=3*(peak_candidates.size()+1)) break;
	      }
	  }// cut on minimum energy of central block
	} // loop over rows
      } // loop over columns
    } // check that there are enough hits to justify looking for another peak
    
    // Sort peak candidates by energy
    if (peak_candidates.size()>0){
      sort(peak_candidates.begin(),peak_candidates.end(),
	   [&](const PeakInfo a,const PeakInfo b){return a.E>b.E;});
    }
    
    // Create weight matrix for hits, sum over all of the measured energy,
    // and find the average radial position of the cluster
    TMatrixD W(num_hits,num_hits);
    double Esum=0.;
    double R=0.;
    for (unsigned int j=0;j<num_hits;j++){
      const HitInfo hit=clusterHits[j];
      double E=hit.E;
      double varE=VAR_E_PAR0+VAR_E_PAR1*E+VAR_E_PAR2*E*E;
      W(j,j)=1./varE;
      Esum+=E;
      R+=sqrt(hit.x*hit.x+hit.y*hit.y);
    }
    R/=double(num_hits);
    
    // Compute estimate for shower shape parameter b
    double b=SHOWER_WIDTH_PAR0+SHOWER_WIDTH_PAR1*R;

    // Loop over peak candidates to perform the peak fit and add good 
    // candidates to the output list.
    vector<PeakInfo>peaks; // output list of peaks
    // Fit quality parameters
    double chisq=1e6,chisq_old=1e6;
    unsigned int ndf=1,ndf_old=1;
    for (unsigned int i=0;i<peak_candidates.size();i++){
      PeakInfo myPeak=peak_candidates[i];

      // Find the centroid for the peak candidate
      double x=0.,y=0.;
      GetRowColRanges(idiff,num_rows,num_cols,myPeak.ir,myPeak.ic,
		      lo_row,hi_row,lo_col,hi_col);
      for (int j=lo_col;j<=hi_col;j++){
	for (int k=lo_row;k<=hi_row;k++){
	  double E=Emap[j][k];
	  if (E>0.){
	    int index=imap[j][k];
	    // Subtract background due to the set of peaks already in the 
	    // list from the current peak
	    for (unsigned int m=0;m<peaks.size();m++){
	      double dE=peaks[m].E*CalcClusterEDeriv(b,clusterHits[index],
						     peaks[m]);
	      E-=dE;
	      myPeak.E-=dE;
	    }
	    x+=E*clusterHits[index].x;
	    y+=E*clusterHits[index].y;
	  }
	}
      }
      if (myPeak.E>MIN_CLUSTER_SEED_ENERGY){
	// Guesses for peak position parameters
	myPeak.x=x/myPeak.E;
	myPeak.y=y/myPeak.E;

	// Save the current peak list and peak guess
	vector<PeakInfo>saved_peaks=peaks;
	PeakInfo peak_guess=myPeak;
	
	// Fit the data to find the best current guesses for the shower 
	// parameters for each peak within this group of ECAL hits.	  
	chisq_old=chisq;
	ndf_old=ndf;
	bool good_fit=FitPeaks(W,b,clusterHits,peaks,myPeak,chisq,ndf);
	if (good_fit){
	  if (peaks.size()>0) myPeak.status=DECALCluster::EXTRA_PEAK;
	  peaks.push_back(myPeak);
	}
	else if (peaks.size()==0){
	  //cout << " Adding guess" << endl;
	  // Always add the first peak, even if the fit failed.
	  // Use the initial guess for the peak info.
	  peaks.push_back(peak_guess);
	  
	  // Compute chisq estimate just in case we need to make a split
	  chisq=0.;
	  ndf=num_hits-3;
	  for (unsigned int j=0;j<num_hits;j++){
	    double dE=clusterHits[j].E
	      -Esum*CalcClusterEDeriv(b,clusterHits[j],peak_guess);
	    chisq+=W(j,j)*dE*dE;
	  }
	}
	else{
	  // No improvement from adding the new peak. Restore the old list
	  peaks=saved_peaks;
	  chisq=chisq_old;
	  ndf=ndf_old;
	}
      } // check for minimum energy
    } // loop over peak candidates

    // Attempt to split the peaks to create new shower candidates
    if (SPLIT_PEAKS && num_hits>3*(peaks.size()+1)){
      // Subtract the energy due to the fitted peaks from the energy of each
      // hit to see if we have excess energy that has not been accounted for
      vector<double>Elist(clusterHits.size());
      for (unsigned int m=0;m<clusterHits.size();m++){
	Elist[m]=clusterHits[m].E;
	for (unsigned int k=0;k<peaks.size();k++){
	  Elist[m]-=peaks[k].E*CalcClusterEDeriv(b,clusterHits[m],peaks[k]);
	}
      }
      double Emax=0.;
      // Find the maximum of the peak-subtracted hit distribution
      unsigned int mmax=0;
      for (unsigned int m=0;m<Elist.size();m++){
	if (Elist[m]>Emax){
	  Emax=Elist[m];
	  mmax=m;
	}
      }
      if (Emax>MIN_EXCESS_SEED_ENERGY){
	// Attempt to make a peak candidate out of the excess energy in the
	// cluster of hits
	int ic=clusterHits[mmax].column-min_col+1;
	int ir=clusterHits[mmax].row-min_row+1;
	double excessE=0.;
	int idiff=2;
	GetRowColRanges(idiff,num_rows,num_cols,ir,ic,lo_row,hi_row,
			lo_col,hi_col);
	for (int j=lo_col;j<=hi_col;j++){
	  for (int k=lo_row;k<=hi_row;k++){
	    int index=imap[j][k];
	    if (Elist[index]>0){
	      excessE+=Elist[index];
	    }
	  }
	}

	double x=clusterHits[mmax].x;
	double y=clusterHits[mmax].y;
	PeakInfo myPeak(excessE,x,y,ic,ir,DECALCluster::SPLIT_CLUSTER);
	    
	// Save the current list of peaks
	vector<PeakInfo>saved_peaks=peaks;
	
	// Add the new peak to the fit to see if the fit quality improves
	chisq_old=chisq;
	ndf_old=ndf;
	bool good_fit=FitPeaks(W,b,clusterHits,peaks,myPeak,chisq,ndf);
	if (good_fit && chisq/ndf+CHISQ_MARGIN<chisq_old/ndf_old){
	  peaks.push_back(myPeak);
	}
	else {
	  // Chisq did not improve.  Restore the old list of peaks.
	  peaks=saved_peaks;
	  chisq=chisq_old;
	  ndf=ndf_old;
	}
      } // Check for minimum excess energy
    } // if peak splitting is enabled

    // Estimate fraction of energy for each peak
    unsigned int npeaks=peaks.size();
    vector<double>peak_fractions(npeaks);
    double Ecalc_sum=0.;
    for (unsigned int k=0;k<npeaks;k++){
      Ecalc_sum+=peaks[k].E;
    }
    for (unsigned int k=0;k<npeaks;k++){
      peak_fractions[k]=peaks[k].E/Ecalc_sum;
    }

    // Add the clusters to the output of the factory
    for (unsigned int k=0;k<npeaks;k++){
      DECALCluster *myCluster= new DECALCluster;
      
      myCluster->status=peaks[k].status;
      myCluster->x=peaks[k].x;
      myCluster->y=peaks[k].y;
      myCluster->Efit=peaks[k].E;
      // using the measured energy gives better energy resolution
      myCluster->E=peak_fractions[k]*Esum;

      // Compute energy-weighted time
      double fsum=0.,t=0.;
      for (unsigned int j=0;j<clusterHits.size();j++){
	double f=CalcClusterEDeriv(b,clusterHits[j],peaks[k]);
	t+=f*clusterHits[j].t;
	fsum+=f;
      }
      myCluster->t=t/fsum;

      Insert(myCluster);
    } // loop over peaks
  } // loop over cluster candidates
}

//------------------
// EndRun
//------------------
void DECALCluster_factory::EndRun()
{
}

//------------------
// Finish
//------------------
void DECALCluster_factory::Finish()
{
}

// Make a list of potential clusters, each consisting of a "central" block 
// and all adjacent blocks, starting with the block with the highest energy.
// Each successive cluster candidate starts with the block with the next 
// highest energy that has not already been included in another cluster.
void DECALCluster_factory::FindClusterCandidates(vector<HitInfo>&hits,
						 vector<vector<HitInfo>>&clusterCandidates) const {
  vector<HitInfo>clusterCandidate;
  clusterCandidate.push_back(hits[0]);
  
  vector<bool>used_hits(hits.size());
  unsigned int nhits=hits.size();
  used_hits[0]=true;

  int hits_left=nhits-1;
  int begin_i=1; 
  while (hits_left>0){
    for (unsigned int i=begin_i;i<nhits;i++){
      if (used_hits[i]==true) continue;
      
      double dt=clusterCandidate[0].t-hits[i].t;
      if (fabs(dt)>TIME_CUT) continue;

      int drow=clusterCandidate[0].row-hits[i].row;
      int dcol=clusterCandidate[0].column-hits[i].column;
      if (abs(drow)<=1 && abs(dcol)<=1){
	clusterCandidate.push_back(hits[i]);

	used_hits[i]=true;
	hits_left--;
      }
    }
    if (clusterCandidate.size()>1){
      clusterCandidates.push_back(clusterCandidate);
    }
    clusterCandidate.clear();
    for (unsigned int i=begin_i;i<nhits;i++){
      if (used_hits[i]==false){
	begin_i=i+1;
	clusterCandidate.push_back(hits[i]);
	
	used_hits[i]=true;
	hits_left--;

	break;
      }
    }
  }
  if (clusterCandidate.size()>1){
    clusterCandidates.push_back(clusterCandidate);
  }

   // At this point we do not necessarily have complete clusters.
  // Merge cluster candidates that appear to belong to a single cluster, again
  // looking for adjacent hits.
  vector<vector<HitInfo>>::iterator iter=clusterCandidates.begin();
  while (iter!=clusterCandidates.end()){
    bool matched=false;
    vector<vector<HitInfo>>::iterator iter2=iter+1;
    for (;iter2!=clusterCandidates.end();iter2++){     
      for (unsigned int i=0;i<(*iter).size();i++){
	for (unsigned int j=0;j<(*iter2).size();j++){
	  double dt=(*iter)[i].t-(*iter2)[j].t;
	  if (fabs(dt)>TIME_CUT) continue;

	  int row1=(*iter)[i].row;
	  int col1=(*iter)[i].column;
	  int row2=(*iter2)[j].row;
	  int col2=(*iter2)[j].column;

	  if (abs(row1-row2)<=1 && abs(col1-col2)<=1){
	    matched=true;
	    break;
	  }
	}
	if (matched){
	  // Add the hits from this cluster to the cluster to which it is 
	  // matched and erase it from the list.
	  for (unsigned int j=0;j<(*iter2).size();j++){
	    (*iter).push_back((*iter2)[j]);
	  }
	  clusterCandidates.erase(iter2);
	  break;
	}
      }
      if (matched==true){
	iter=clusterCandidates.begin();
	break;
      }
    }
    if (matched==false) iter++;
  }
}

// Fit peaks within a cluster containing a list of hits contained in hitList
bool DECALCluster_factory::FitPeaks(const TMatrixD &W,double b,
				    vector<HitInfo>&hitList,
				    vector<PeakInfo>&peaks,
				    PeakInfo &myNewPeak,double &chisq,
				    unsigned int &ndf
				    ) const {  
  unsigned int nhits=hitList.size();
  unsigned int npeaks=peaks.size();  
  // Save the current peak info
  vector<PeakInfo>saved_peaks=peaks;
  PeakInfo saved_new_peak=myNewPeak; 

  // Iterate to find best shower energy and position
  chisq=1e6;
  unsigned int min_iter=5,max_iter=50;
  double min_frac=MIN_CUTDOWN_FRACTION;
  double cutdown_scale=(1.-min_frac)/double(min_iter); //for cut-down for parameter adjustment
  for (unsigned int k=0;k<max_iter;k++){
    //    _DBG_ << "E " << myNewPeak.E << endl;
    // Make sure the energy is positive!
    if (myNewPeak.E<0){
      return false;
    }
    //Check that the new peak positions are still within the fiducial area of
    // the detector
    if (dECALGeom->isFiducial(myNewPeak.x,myNewPeak.y)==false){
      //_DBG_ << myNewPeak.x << " " << myNewPeak.y << endl;
      return false;
    }
    // Matrix of per-block differences between measured and calculated energies
    TMatrixD dE(nhits,1);
    // Matrix containing partical derivatives of the shower profile function
    // with respect to the three parameters (E, xc, yc): Jacobian matrix
    TMatrixD A(nhits,3*npeaks+3);
    // Loop over all the hits to fill dE and A
    for (unsigned int i=0;i<nhits;i++){
      double Ecalc=0.,df_dE=0;
      for (unsigned int j=0;j<npeaks;j++){
	PeakInfo myPeakInfo=peaks[j];
	
	// Make sure the energy is positive!
	if (myPeakInfo.E<0){
	  return false;
	}

	//Check that the new peak positions are still within the fiducial 
	//area of the detector 
	if (dECALGeom->isFiducial(myPeakInfo.x,myPeakInfo.y)==false){
	  //_DBG_<< myPeakInfo.x << " " << myPeakInfo.y << endl;
	  return false;
	}
	// Compute the Jacobian matrix elements
	df_dE=CalcClusterEDeriv(b,hitList[i],myPeakInfo);
	A(i,3*j+0)=df_dE;
	A(i,3*j+1)=CalcClusterXYDeriv(true,b,hitList[i],myPeakInfo);
	A(i,3*j+2)=CalcClusterXYDeriv(false,b,hitList[i],myPeakInfo);

	Ecalc+=myPeakInfo.E*df_dE;
      }
      // Add contributions from the peak we wish to add
      df_dE=CalcClusterEDeriv(b,hitList[i],myNewPeak);
      A(i,3*npeaks+0)=df_dE;
      A(i,3*npeaks+1)=CalcClusterXYDeriv(true,b,hitList[i],myNewPeak);
      A(i,3*npeaks+2)=CalcClusterXYDeriv(false,b,hitList[i],myNewPeak);
      
      Ecalc+=myNewPeak.E*df_dE;

      double Ediff=hitList[i].E-Ecalc;
      dE(i,0)=Ediff;
      //cout << " Ediff " << Ediff << endl;
    }
    // Compute chi^2 for this iteration
    double chisq_new=0.;
    for (unsigned int i=0;i<nhits;i++) chisq_new+=W(i,i)*dE(i,0)*dE(i,0);
    // cout << endl;
    //cout << k << " chisq "<< chisq_new << " " << (chisq_new-chisq)/chisq <<endl;
    //cout << endl; 
    if (k>=min_iter){
      if (fabs(chisq_new-chisq)/chisq<0.0001) break;
      if (chisq_new>chisq) break;
    }
    
    // Save the current value of chisq
    chisq=chisq_new;
        
    // Save the current best peak values
    saved_new_peak=myNewPeak;
    saved_peaks.assign(peaks.begin(),peaks.end());
    
    // Determine the set of corrections needed to bring the computed shower 
    // shape closer to the measurements
    TMatrixD A_T(TMatrixD::kTransposed,A);
    TMatrixD AT_A=A_T*W*A;
    TMatrixD InvATA(TMatrixD::kInverted,AT_A);
    // The function describing the cluster profile is rather nastily non-linear,
    // so we perform a cut-down on the maximum shift vector.
    double frac=(k<min_iter)?(min_frac+cutdown_scale*double(k)):1.;
    TMatrixD dPar=frac*(InvATA*A_T*W*dE);
    
    // Update the peak parameters
    for (unsigned int i=0;i<npeaks;i++){
      peaks[i].E+=dPar(3*i+0,0);
      peaks[i].x+=dPar(3*i+1,0);
      peaks[i].y+=dPar(3*i+2,0);
    } 
    myNewPeak.E+=dPar(3*npeaks+0,0);
    myNewPeak.x+=dPar(3*npeaks+1,0);
    myNewPeak.y+=dPar(3*npeaks+2,0);
  }

  // Number of degrees of freedom
  ndf=nhits-3*(peaks.size()+1);

  // Peak info for output of the routine.  At this stage myNewPeak, for example,
  // has been adjusted away from the solution with the best chisq value.
  myNewPeak=saved_new_peak;
  peaks.assign(saved_peaks.begin(),saved_peaks.end());

  return true;
}

// Routine to compute the partial derivatives of the shower profile function 
// with respect to the peak position parameters (xpeak,ypeak)
double DECALCluster_factory::CalcClusterXYDeriv(bool isXDeriv,double b,
						const HitInfo &hit,
						const PeakInfo &myPeakInfo) const {
  double sign1[4]={1,1,-1,-1};
  double sign2[4]={1,-1,-1,1};
  double half_block=0.5*dECALGeom->sensitiveBlockSize();
  double b2=b*b;
  double f=0.;
  double dxc=hit.x-myPeakInfo.x;
  double dyc=hit.y-myPeakInfo.y;
  double Ec=myPeakInfo.E;
  for (int i=0;i<4;i++){
    double dx=dxc+sign1[i]*half_block;
    double dy=dyc+sign2[i]*half_block;
    double dx2=dx*dx;
    double dy2=dy*dy;
    double factor=-sign1[i]*sign2[i]*Ec*b/(2*M_PI)/sqrt(b2+dx2+dy2);
    if (isXDeriv){
      f+=factor*dy/(b2+dx2);
    }
    else {
      f+=factor*dx/(b2+dy2);
    }
  }
  return f;
}

// Shower profile function from Bland, et al., Instruments and Experimental 
// Techniques, 2008, Vol. 51, No. 3, pp. 342-350, eq. 6.  Note that the 
// normalization of this function in this equation does not appear to be 
// correct (it's off by 1/sqrt(2pi)).
double DECALCluster_factory::CalcClusterEDeriv(double b,
					       const HitInfo &hit,
					       const PeakInfo &myPeakInfo) const {
  double sign1[4]={1,1,-1,-1};
  double sign2[4]={1,-1,-1,1};
  double half_block=0.5*dECALGeom->sensitiveBlockSize();
  double f=0.;
  double dxc=hit.x-myPeakInfo.x;
  double dyc=hit.y-myPeakInfo.y;
  for (int i=0;i<4;i++){
    double dx=dxc+sign1[i]*half_block;
    double dy=dyc+sign2[i]*half_block;
    f+=sign1[i]*sign2[i]/(2*M_PI)*atan(dx*dy/(b*sqrt(b*b+dx*dx+dy*dy)));
  }
  return f;
}
