// $Id$
//
//    File: DFCALCluster_factory_Island.cc
// Created: Fri Dec  4 08:25:47 EST 2020
// Creator: staylor (on Linux ifarm1802.jlab.org 3.10.0-1062.4.1.el7.x86_64 x86_64)
//


#include <iostream>
#include <iomanip>
using namespace std;

#include "DFCALCluster_factory_Island.h"
using namespace jana;

inline bool FCALHit_E_cmp(const DFCALHit *a,const DFCALHit *b){
  return (a->E>b->E);
}


//------------------
// init
//------------------
jerror_t DFCALCluster_factory_Island::init(void)
{
  TIME_CUT=15.;
  gPARMS->SetDefaultParameter("FCAL:TIME_CUT",TIME_CUT,"time cut for associating FCAL hits together into a cluster");
  MIN_CLUSTER_SEED_ENERGY=0.035;
  gPARMS->SetDefaultParameter("FCAL:MIN_CLUSTER_SEED_ENERGY",
			      MIN_CLUSTER_SEED_ENERGY);
  SHOWER_WIDTH_PARAMETER=0.8;
  gPARMS->SetDefaultParameter("FCAL:SHOWER_WIDTH_PARAMETER",
			      SHOWER_WIDTH_PARAMETER);
  MIN_CUTDOWN_FRACTION=0.25;
  gPARMS->SetDefaultParameter("FCAL:MIN_CUTDOWN_FRACTION",
			      MIN_CUTDOWN_FRACTION);
  DEBUG_HISTS=false;
  gPARMS->SetDefaultParameter("FCAL:DEBUG_HISTS",DEBUG_HISTS);

  HistdE=new TH2D("HistdE",";E [GeV];#deltaE [GeV]",100,0,10,100,-1,1);
  HistProb=new TH1D("HistProb",";CL",100,0,1);
  
  return NOERROR;
}

//------------------
// brun
//------------------
jerror_t DFCALCluster_factory_Island::brun(jana::JEventLoop *eventLoop, int32_t runnumber)
{
  eventLoop->GetSingle(dFCALGeom);

  return NOERROR;
}

//------------------
// evnt
//------------------
jerror_t DFCALCluster_factory_Island::evnt(JEventLoop *loop, uint64_t eventnumber)
{
  vector<const DFCALHit*>fcal_hits;
  loop->Get(fcal_hits);
  if (fcal_hits.size()==0) return OBJECT_NOT_AVAILABLE;
  
  // Sort the hits according to energy.
  stable_sort(fcal_hits.begin(),fcal_hits.end(),FCALHit_E_cmp);
  
  // Associate groups of adjacent hits into cluster candidates
  vector<vector<const DFCALHit*>>clusterCandidates;
  FindClusterCandidates(fcal_hits,clusterCandidates);
  
  for (unsigned int i=0;i<clusterCandidates.size();i++){
    // Skip over single hit "clusterCandidates"
    if (clusterCandidates[i].size()==1) continue;

    // Mininum number of hits to make a shower = 2
    if (clusterCandidates[i].size()<4){
      // Create a new DFCALCluster object and add it to the _data list
      DFCALCluster *myCluster= new DFCALCluster(0);
      vector<const DFCALHit*>clusterCandidate=clusterCandidates[i];
      
      double E1=clusterCandidate[0]->E;
      double E2=clusterCandidate[1]->E;
      double Etot=E1+E2;

      double t1=clusterCandidate[0]->t;
      double t2=clusterCandidate[1]->t;

      double x1=clusterCandidate[0]->x;
      double x2=clusterCandidate[1]->x;
      double y1=clusterCandidate[0]->y;
      double y2=clusterCandidate[1]->y;
	
      myCluster->setEnergy(Etot);
      myCluster->setTimeEWeight((t1*E1+t2*E2)/Etot);
      myCluster->setCentroid((x1*E1+x2*E2)/Etot,(y1*E1+y2*E2)/Etot);
      myCluster->setChannelEmax(dFCALGeom->channel(clusterCandidate[0]->row,
						   clusterCandidate[0]->column));

      myCluster->AddAssociatedObject(clusterCandidate[0]);
      myCluster->AddAssociatedObject(clusterCandidate[1]);

      _data.push_back(myCluster);

      continue;
    }
    
    //--------------------------------------------------------------------------
    // Handle cluster candidates containing more than 2 hits
    //--------------------------------------------------------------------------
    vector<const DFCALHit*>clusterCandidate=clusterCandidates[i];
    vector<PeakInfo>peaks;

    unsigned int num_hits=clusterCandidate.size();
    int min_row=1000,min_col=1000,max_row=0,max_col=0;
    for (unsigned int j=0;j<num_hits;j++){
      if (clusterCandidate[j]->row<min_row) min_row=clusterCandidate[j]->row;
      if (clusterCandidate[j]->column<min_col) min_col=clusterCandidate[j]->column;
      if (clusterCandidate[j]->row>max_row) max_row=clusterCandidate[j]->row;
      if (clusterCandidate[j]->column>max_col) max_col=clusterCandidate[j]->column;
    }
    // Creat arrays to represent the cluster of hits to aid in peak search
    int num_rows=max_row-min_row+1;
    int num_cols=max_col-min_col+1;
    vector<vector<double>>Emap(max_col,vector<double>(max_row));
    vector<vector<unsigned int>>imap(max_col,vector<unsigned int>(max_row));
    vector<int>potential_peaks;
    // By construction the first entry in the cluster hit list is a peak 
    // candidate
    potential_peaks.push_back(0);
    double old_E=1e6,chisq=1e6;
    // Loop over hits looking for more peaks.  The groups of hits were added to
    // the cluster candidate ordered by energy, so look for increases in E from 
    // one hit to the next.
    for (unsigned int j=0;j<num_hits;j++){
      double E=clusterCandidate[j]->E;
      if (E>old_E){
	potential_peaks.push_back(j);
	old_E=1e6;
      }
      else old_E=E;

      int ic=clusterCandidate[j]->column-min_col;
      int ir=clusterCandidate[j]->row-min_row;
      Emap[ic][ir]=E;
      imap[ic][ir]=j;      
    }
    // Loop over the potential peaks.  Some of these "peaks" are merely parts 
    // of a larger cluster.
    for (unsigned int n=0;n<potential_peaks.size();n++){
      // Look in a grid of 1 unit in x and y about a central cell.  If all
      // of the energy differences relative this central cell are positive,
      // we have a good peak.
      int row_index=clusterCandidate[potential_peaks[n]]->row-min_row;
      int col_index=clusterCandidate[potential_peaks[n]]->column-min_col;
      int lo_row=(row_index>0)?(row_index-1):row_index;
      int hi_row=(row_index<num_rows-1)?(row_index+1):row_index;
      int lo_col=(col_index>0)?(col_index-1):col_index;
      int hi_col=(col_index<num_cols-1)?(col_index+1):col_index;
      double Ec=clusterCandidate[potential_peaks[n]]->E;
      
      // Initialize some variables needed for finding a guess for the cluster
      // energy, position and time.
      double E=Ec;
      if (E>MIN_CLUSTER_SEED_ENERGY){
	double x=Ec*clusterCandidate[potential_peaks[n]]->x;
	double y=Ec*clusterCandidate[potential_peaks[n]]->y;
	double t=Ec*clusterCandidate[potential_peaks[n]]->t;
	
	bool got_peak=true;
	int nhits_in_peak=1;
	for (int j=lo_col;j<=hi_col;j++){
	  for (int k=lo_row;k<=hi_row;k++){
	    if (k==row_index&&j==col_index) continue;
	    
	    double Ejk=Emap[j][k];
	    if (Ec-Ejk<0){
	      got_peak=false;
	      break;
	    }
	    else{
	      // Accumulate energy and energy weighted position variables
	      if (Ejk>0.){
		E+=Ejk;
		x+=Ejk*clusterCandidate[imap[j][k]]->x;
		y+=Ejk*clusterCandidate[imap[j][k]]->y;
		t+=Ejk*clusterCandidate[imap[j][k]]->t;
		nhits_in_peak++;
	      }
	    }
	  }
	  if (got_peak==false) break;
	}
	if (got_peak){
	  x/=E;
	  y/=E;
	  t/=E;
	  PeakInfo myPeak(row_index,col_index,E,x,y,t);

	  // Subtract background due to the set of peaks already in the list
	  // from the current peak
	  for (unsigned int m=0;m<peaks.size();m++){
	    double frac=0.;
	    for (int j=lo_col;j<=hi_col;j++){
	      for (int k=lo_row;k<=hi_row;k++){
		if (Emap[j][k]>0.) {
		  frac+=CalcClusterEDeriv(clusterCandidate[imap[j][k]],peaks[m]);
		}
	      }
	    }
	    myPeak.E-=peaks[m].E*frac;
	  }
	  // Fit the data to find the best current guesses for the shower 
	  // parameters for each peak within this group of FCAL hits.
	  if (nhits_in_peak>2){
	    FitPeaks(clusterCandidate,peaks,myPeak,chisq);
	  }
	  else {
	    // Don't try to fit with 2 hit peak candidates -- just add them
	    // to the list.
	    peaks.push_back(myPeak);
	  }
	}
      } // cut on minimum energy of central block
    }

    for (unsigned int k=0;k<peaks.size();k++){
      DFCALCluster *myCluster= new DFCALCluster(0);
      
      myCluster->setEnergy(peaks[k].E);
      myCluster->setTimeEWeight(peaks[k].t);
      myCluster->setCentroid(peaks[k].x,peaks[k].y);
      myCluster->setChannelEmax(dFCALGeom->channel(peaks[k].row_index+min_row,
						   peaks[k].col_index+min_col));

      // For now attach all the hits in this hit group to each cluster
      for (unsigned int n=0;n<clusterCandidate.size();n++){
	myCluster->AddAssociatedObject(clusterCandidate[n]);
      }
      _data.push_back(myCluster);
    }
  }

  return NOERROR;
}

//------------------
// erun
//------------------
jerror_t DFCALCluster_factory_Island::erun(void)
{
  return NOERROR;
}

//------------------
// fini
//------------------
jerror_t DFCALCluster_factory_Island::fini(void)
{
  return NOERROR;
}

// Make a list of potential clusters, each consisting of a "central" block 
// and all adjacent blocks, starting with the block with the highest energy.
// Each successive cluster candidate starts with the block with the next 
// highest energy that has not already been included in another cluster.
void DFCALCluster_factory_Island::FindClusterCandidates(vector<const DFCALHit*>&fcal_hits,
					       vector<vector<const DFCALHit*>>&clusterCandidates) const {
  vector<const DFCALHit*>clusterCandidate;
  clusterCandidate.push_back(fcal_hits[0]);
  
  vector<bool>used_hits(fcal_hits.size());
  used_hits[0]=true;

  int hits_left=fcal_hits.size()-1;
  int begin_i=1; 
  while (hits_left>0){
    for (size_t i=begin_i;i<fcal_hits.size();i++){
      if (used_hits[i]==true) continue;
      
      double dt=clusterCandidate[0]->t-fcal_hits[i]->t;
      if (fabs(dt)>TIME_CUT) continue;

      double drow=clusterCandidate[0]->row-fcal_hits[i]->row;
      double dcol=clusterCandidate[0]->column-fcal_hits[i]->column;
      if (abs(drow)<=1 && abs(dcol)<=1){
	clusterCandidate.push_back(fcal_hits[i]);

	used_hits[i]=true;
	hits_left--;
      }
    }

    clusterCandidates.push_back(clusterCandidate);
    clusterCandidate.clear();
    for (size_t i=begin_i;i<fcal_hits.size();i++){
      if (used_hits[i]==false){
	begin_i=i+1;
	clusterCandidate.push_back(fcal_hits[i]);
	
	used_hits[i]=true;
	hits_left--;

	break;
      }
    }
  }
  if (clusterCandidate.size()>0) clusterCandidates.push_back(clusterCandidate);

  // At this point we do not necessarily have complete clusters.
  // Merge cluster candidates that appear belong to a single cluster, again
  // looking for adjacent hits.
  vector<vector<const DFCALHit*>>::iterator iter=clusterCandidates.begin();
  while (iter!=clusterCandidates.end()){
    bool matched=false;
    vector<vector<const DFCALHit*>>::iterator iter2=iter+1;
    for (;iter2!=clusterCandidates.end();iter2++){     
      for (unsigned int i=0;i<(*iter).size();i++){
	for (unsigned int j=0;j<(*iter2).size();j++){
	  double drow=(*iter)[i]->row-(*iter2)[j]->row;
	  double dcol=(*iter)[i]->column-(*iter2)[j]->column;
	  double dt=(*iter)[i]->t-(*iter2)[j]->t;
	  if (abs(drow)<=1 && abs(dcol)<=1 && fabs(dt)<TIME_CUT){
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
void DFCALCluster_factory_Island::FitPeaks(vector<const DFCALHit*>&hitList,
					     vector<PeakInfo>&peaks,
					     PeakInfo &myNewPeak,double &chisq) const {  
  size_t nhits=hitList.size();
  size_t npeaks=peaks.size();

  // Save the current peak list
  vector<PeakInfo>saved_peaks=peaks;
  // Save the current info for the new peak
  PeakInfo saved_peak=myNewPeak;

  TMatrixD W(nhits,nhits);  // Weights for each hit
  for (unsigned int i=0;i<nhits;i++){
    double E=hitList[i]->E;
    double varE=0.001+0.001225*E+0.0009*E*E;
    W(i,i)=1./varE;
  }

  // Iterate to find best shower energy and position
  double chisq_old=1e6,chisq_new=0.;
  unsigned int max_iter=100;
  double min_frac=MIN_CUTDOWN_FRACTION;
  double cutdown_scale=(1.-min_frac)/double(max_iter); //for cut-down for parameter adjustment
  for (unsigned int k=0;k<max_iter;k++){
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
	// Compute the Jacobian matrix elements
	df_dE=CalcClusterEDeriv(hitList[i],myPeakInfo);
	A(i,3*j+0)=df_dE;
	A(i,3*j+1)=CalcClusterXYDeriv(true,hitList[i],myPeakInfo);
	A(i,3*j+2)=CalcClusterXYDeriv(false,hitList[i],myPeakInfo);

	Ecalc+=myPeakInfo.E*df_dE;
      }
      // Add contributions from the peak we wish to add
      df_dE=CalcClusterEDeriv(hitList[i],myNewPeak);
      A(i,3*npeaks+0)=df_dE;
      A(i,3*npeaks+1)=CalcClusterXYDeriv(true,hitList[i],myNewPeak);
      A(i,3*npeaks+2)=CalcClusterXYDeriv(false,hitList[i],myNewPeak);
      
      Ecalc+=myNewPeak.E*df_dE;

      double Ediff=hitList[i]->E-Ecalc;
      dE(i,0)=Ediff;
    }
    // Compute chi^2 for this iteration
    chisq_new=0.;
    for (unsigned int i=0;i<nhits;i++) chisq_new+=W(i,i)*dE(i,0)*dE(i,0);
    // cout << endl;
    //cout << k << " chisq "<< chisq_new << " " << (chisq_new-chisq_old)/chisq_old <<endl;
    //cout << endl;
    if (chisq_new>chisq_old){
      // Restore the "best" version before the convergence failure
      chisq_new=chisq_old;
      myNewPeak=saved_peak;
      break;
    }
    if (fabs(chisq_new-chisq_old)/chisq_old<0.0001) break;
    
    // Determine the set of corrections needed to bring the computed shower 
    // shape closer to the measurements
    TMatrixD A_T(TMatrixD::kTransposed,A);
    TMatrixD AT_A=A_T*W*A;
    TMatrixD InvATA(TMatrixD::kInverted,AT_A);
    // The function describing the cluster profile is rather nastily non-linear,
    // so we perform a cut-down on the maximum shift vector.
    double frac=min_frac+cutdown_scale*double(k);
    TMatrixD dPar=frac*(InvATA*A_T*W*dE);
    
    // Update the peak parameters
    for (unsigned int i=0;i<npeaks;i++){
      peaks[i].E+=dPar(3*i+0,0);
      peaks[i].x+=dPar(3*i+1,0);
      peaks[i].y+=dPar(3*i+2,0);
    } 
    saved_peak=myNewPeak;

    myNewPeak.E+=dPar(3*npeaks+0,0);
    myNewPeak.x+=dPar(3*npeaks+1,0);
    myNewPeak.y+=dPar(3*npeaks+2,0);
    
    chisq_old=chisq_new;
  }
  if (chisq_new<chisq){
    peaks.push_back(myNewPeak);
    chisq=chisq_new;
  }
  else {
    // No improvement from adding the new peak. Restore the old list
    peaks=saved_peaks;
  }
  if (DEBUG_HISTS && nhits>3){
    HistProb->Fill(TMath::Prob(chisq_new,nhits-3.*peaks.size()));
    if (peaks.size()==1){
      double Ecalc=0.,E=hitList[0]->E;
      PeakInfo myPeakInfo=peaks[0];
      Ecalc+=myPeakInfo.E*CalcClusterEDeriv(hitList[0],myPeakInfo);
      HistdE->Fill(E,E-Ecalc);
    }
  }
}

// Routine to compute the partial derivatives of the shower profile function 
// with respect to the peak position parameters (xpeak,ypeak)
double DFCALCluster_factory_Island::CalcClusterXYDeriv(bool isXDeriv,
						       const DFCALHit *hit,
						       const PeakInfo &myPeakInfo) const {
  double sign1[4]={1,1,-1,-1};
  double sign2[4]={1,-1,-1,1};
  double half_block=0.5*4.;
  double b=SHOWER_WIDTH_PARAMETER; // cm
  double b2=b*b;
  double f=0.;
  double dxc=hit->x-myPeakInfo.x;
  double dyc=hit->y-myPeakInfo.y;
  double Ec=myPeakInfo.E;
  for (int i=0;i<4;i++){
    double dx=dxc+sign1[i]*half_block;
    double dy=dyc+sign2[i]*half_block;
    double dx2=dx*dx;
    double dy2=dy*dy;
    double factor=-sign1[i]*sign2[i]*Ec*b/(2*M_PI)/sqrt(b2*dx2+dy2);
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
double DFCALCluster_factory_Island::CalcClusterEDeriv(const DFCALHit *hit,
						      const PeakInfo &myPeakInfo) const {
  double sign1[4]={1,1,-1,-1};
  double sign2[4]={1,-1,-1,1};
  double half_block=0.5*4.;
  double b=SHOWER_WIDTH_PARAMETER; // cm
  double f=0.;
  double dxc=hit->x-myPeakInfo.x;
  double dyc=hit->y-myPeakInfo.y;
  for (int i=0;i<4;i++){
    double dx=dxc+sign1[i]*half_block;
    double dy=dyc+sign2[i]*half_block;
    f+=sign1[i]*sign2[i]/(2*M_PI)*atan(dx*dy/(b*sqrt(b*b+dx*dx+dy*dy)));
  }
  return f;
}
