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

inline bool FCALHit_E_cmp(const DFCALCluster_factory_Island::HitInfo a,
			  const DFCALCluster_factory_Island::HitInfo b){
  return (a.E>b.E);
}

inline bool peak_E_cmp(DFCALCluster_factory_Island::PeakInfo a,
		       DFCALCluster_factory_Island::PeakInfo b){
  return (a.E>b.E);
}

//------------------
// init
//------------------
jerror_t DFCALCluster_factory_Island::init(void)
{
  TIME_CUT=15.;
  gPARMS->SetDefaultParameter("FCAL:TIME_CUT",TIME_CUT,"time cut for associating FCAL hits together into a cluster");
  
  MAX_HITS_FOR_CLUSTERING = 250;  
  gPARMS->SetDefaultParameter("FCAL:MAX_HITS_FOR_CLUSTERING", MAX_HITS_FOR_CLUSTERING);
  MIN_EXCESS_SEED_ENERGY=35.*k_MeV;
  gPARMS->SetDefaultParameter("FCAL:MIN_EXCESS_SEED_ENERGY",
			      MIN_EXCESS_SEED_ENERGY);
  MIN_CLUSTER_SEED_ENERGY=35.*k_MeV;
  gPARMS->SetDefaultParameter("FCAL:MIN_CLUSTER_SEED_ENERGY",
			      MIN_CLUSTER_SEED_ENERGY);
  MIN_E_FRACTION=0.0;
  gPARMS->SetDefaultParameter("FCAL:MIN_E_FRACTION",MIN_E_FRACTION);

  SHOWER_WIDTH_PAR0=0.58;
  gPARMS->SetDefaultParameter("FCAL:SHOWER_WIDTH_PAR0",SHOWER_WIDTH_PAR0);  
  SHOWER_WIDTH_PAR1=0.0049;
  gPARMS->SetDefaultParameter("FCAL:SHOWER_WIDTH_PAR1",SHOWER_WIDTH_PAR1);
 
  INSERT_SHOWER_WIDTH_PAR0=0.347;
  gPARMS->SetDefaultParameter("FCAL:INSERT_SHOWER_WIDTH_PAR0",
			      INSERT_SHOWER_WIDTH_PAR0);
  INSERT_SHOWER_WIDTH_PAR1=0.00058;
  gPARMS->SetDefaultParameter("FCAL:INSERT_SHOWER_WIDTH_PAR1",
			      INSERT_SHOWER_WIDTH_PAR1);
  MIN_CUTDOWN_FRACTION=0.1;
  gPARMS->SetDefaultParameter("FCAL:MIN_CUTDOWN_FRACTION",
			      MIN_CUTDOWN_FRACTION);

  DEBUG_HISTS=false;
  gPARMS->SetDefaultParameter("FCAL:DEBUG_HISTS",DEBUG_HISTS);

  CHISQ_MARGIN=12.5;
  gPARMS->SetDefaultParameter("FCAL:CHISQ_MARGIN",CHISQ_MARGIN);

  SPLIT_PEAKS=true;
  gPARMS->SetDefaultParameter("FCAL:SPLIT_PEAKS",SPLIT_PEAKS);

  MERGE_HITS_AT_BOUNDARY=true;
  gPARMS->SetDefaultParameter("FCAL:MERGE_HITS_AT_BOUNDARY",MERGE_HITS_AT_BOUNDARY);

  APPLY_S_CURVE_CORRECTION=false;
  gPARMS->SetDefaultParameter("FCAL:APPLY_S_CURVE_CORRECTION",
			      APPLY_S_CURVE_CORRECTION);

  S_CURVE_PAR1=-0.374;
  gPARMS->SetDefaultParameter("FCAL:S_CURVE_PAR1",S_CURVE_PAR1);
  S_CURVE_PAR2=1.59;
  gPARMS->SetDefaultParameter("FCAL:S_CURVE_PAR2",S_CURVE_PAR2);
  S_CURVE_PAR3=1.55;
  gPARMS->SetDefaultParameter("FCAL:S_CURVE_PAR2",S_CURVE_PAR2); 
  INSERT_S_CURVE_PAR1=-0.203;
  gPARMS->SetDefaultParameter("FCAL:INSERT_S_CURVE_PAR1",INSERT_S_CURVE_PAR1);
  INSERT_S_CURVE_PAR2=2.982;
  gPARMS->SetDefaultParameter("FCAL:INSERT_S_CURVE_PAR2",INSERT_S_CURVE_PAR2);
  INSERT_S_CURVE_PAR3=0.0;
  gPARMS->SetDefaultParameter("FCAL:INSERT_S_CURVE_PAR3",INSERT_S_CURVE_PAR3);

  if (DEBUG_HISTS){
    if (HistdE==NULL) HistdE=new TH2D("HistdE",";E [GeV];#deltaE [GeV]",100,0,10,201,-0.25,0.25);
    if (HistProb==NULL) HistProb=new TH1D("HistProb",";CL",100,0,1);
  }
  return NOERROR;
}

//------------------
// brun
//------------------
jerror_t DFCALCluster_factory_Island::brun(jana::JEventLoop *eventLoop, int32_t runnumber)
{
  DApplication *dapp = dynamic_cast<DApplication*>(eventLoop->GetJApplication());
  const DGeometry *geom = dapp->GetDGeometry(runnumber);

  double targetZ=0.;
  geom->GetTargetZ(targetZ);
  eventLoop->GetSingle(dFCALGeom);
  m_zdiff=dFCALGeom->fcalFrontZ()-targetZ;

  m_insert_Eres[0]=0.00025;
  m_insert_Eres[1]=0.00025;
  m_insert_Eres[2]=4.4e-5;

  m_Eres[0]=0.0005;
  m_Eres[1]=0.0025;
  m_Eres[2]=0.0009;

  return NOERROR;
}

//------------------
// evnt
//------------------
jerror_t DFCALCluster_factory_Island::evnt(JEventLoop *loop, uint64_t eventnumber)
{
  vector<const DFCALHit*>fcal_hits;
  loop->Get(fcal_hits);
  vector<const DECALHit*>ecal_hits;
  loop->Get(ecal_hits);
 
  unsigned int total_hits=fcal_hits.size()+ecal_hits.size();
  if (total_hits==0) return OBJECT_NOT_AVAILABLE;

  // LED events will have hits in nearly every channel. Do NOT
  // try clusterizing if more than 250 hits in FCAL
  if (total_hits > MAX_HITS_FOR_CLUSTERING) return VALUE_OUT_OF_RANGE;

  // Put hit information into local array
  vector<HitInfo>hits;
  for (size_t i=0;i<fcal_hits.size();i++){
    const DFCALHit *myhit=fcal_hits[i];
    hits.push_back(HitInfo(myhit->id,myhit->row,myhit->column,myhit->E,myhit->x,
			   myhit->y,myhit->t));
  }
  for (size_t i=0;i<ecal_hits.size();i++){
    const DECALHit *myhit=ecal_hits[i];
    int row=100+myhit->row;
    int col=100+myhit->column;
    DVector2 pos=dFCALGeom->positionOnFace(row,col);
    hits.push_back(HitInfo(myhit->id,row,col,myhit->E,pos.X(),pos.Y(),
			   myhit->t));
  }

  // Sort the hits according to energy.
  stable_sort(hits.begin(),hits.end(),FCALHit_E_cmp);
  
  // Associate groups of adjacent hits into cluster candidates
  vector<vector<HitInfo>>clusterCandidates;
  FindClusterCandidates(hits,clusterCandidates);

  for (unsigned int i=0;i<clusterCandidates.size();i++){
    // Skip over single hit "clusterCandidates"
    if (clusterCandidates[i].size()==1) continue;

    // Mininum number of hits to make a shower = 2
    if (clusterCandidates[i].size()<4){
      // Create a new DFCALCluster object and add it to the _data list
      DFCALCluster *myCluster= new DFCALCluster(0);
      vector<HitInfo>clusterCandidate=clusterCandidates[i];
      
      double Etot=0.,t=0,x=0,y=0;
      for (unsigned int k=0;k<clusterCandidate.size();k++){
	double E=clusterCandidate[k].E;
	Etot+=E;
	t+=E*clusterCandidate[k].t;
	x+=E*clusterCandidate[k].x;
	y+=E*clusterCandidate[k].y;

	int channel=dFCALGeom->channel(clusterCandidate[k].row,
				       clusterCandidate[k].column);
	myCluster->addHit(clusterCandidate[k].id,channel,E,
			  clusterCandidate[k].x,clusterCandidate[k].y,
			  clusterCandidate[k].t);
      }
      x/=Etot;
      y/=Etot;
      t/=Etot;
	
      myCluster->setEnergy(Etot);
      myCluster->setTimeEWeight(t);
      myCluster->setStatus(DFCALCluster::SHOWER_FOUND);
     
      int channel=dFCALGeom->channel(clusterCandidate[0].row,
				     clusterCandidate[0].column);
      myCluster->setChannelEmax(channel);

      if (APPLY_S_CURVE_CORRECTION){
	DVector2 blockPos=dFCALGeom->positionOnFace(channel);
	double dx=blockPos.X()-x;
	double dy=blockPos.Y()-y;
	if (dFCALGeom->inInsert(channel)==false){ 
	  x-=0.12*dx*(4.0157*4.0157/4-dx*dx);  
	  y-=0.12*dy*(4.0157*4.0157/4-dy*dy);
	}
	else{
	  double diffx=2.05*2.05/4-dx*dx;
	  double diffy=2.05*2.05/4-dy*dy;
	  x-=1.055*dx*diffx*diffx;
	  y-=1.055*dy*diffy*diffy;
	}
      }

      myCluster->setCentroid(x,y);
      
      _data.push_back(myCluster);

      continue;
    }

    //------------------------------------------------------------------------
    // Handle cluster candidates containing more than 3 hits
    //------------------------------------------------------------------------

    vector<HitInfo>clusterHits=clusterCandidates[i];
    vector<PeakInfo>peaks;

    // Create weight matrix for hits
    unsigned int num_hits=clusterHits.size();
    TMatrixD W(num_hits,num_hits);
    double Esum=0.;
    double R=0.; // simple average radius of cluster
    for (unsigned int j=0;j<num_hits;j++){
      const HitInfo hit=clusterHits[j];
      double E=hit.E;
      double varE=0.;
      if (dFCALGeom->isInsertBlock(hit.row,hit.column)){
	varE=m_insert_Eres[0]+m_insert_Eres[1]*E+m_insert_Eres[2]*E*E;
      }
      else{
	varE=m_Eres[0]+m_Eres[1]*E+m_Eres[2]*E*E;
      }
      W(j,j)=1./varE;
      Esum+=E;
      R+=sqrt(hit.x*hit.x+hit.y*hit.y);
    }
    R/=double(num_hits);

    double chisq=1e6,chisq_old=1e6;
    unsigned int ndf=1,ndf_old=1;
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
    unsigned int jmax=0;
    double Emax=0.;
    for (unsigned int j=0;j<num_hits;j++){
      int ic=clusterHits[j].column-min_col+1;
      int ir=clusterHits[j].row-min_row+1;
      Emap[ic][ir]=clusterHits[j].E;
      imap[ic][ir]=j;
      if (clusterHits[j].E>Emax){
	Emax=clusterHits[j].E;
	jmax=j;
      }
    }

    // Compute estimate for shower shape parameter b
    double b=0.;
    if (dFCALGeom->isInsertBlock(clusterHits[jmax].row,
				 clusterHits[jmax].column)){
      b=INSERT_SHOWER_WIDTH_PAR0+INSERT_SHOWER_WIDTH_PAR1*R;
    }
    else{
      b=SHOWER_WIDTH_PAR0+SHOWER_WIDTH_PAR1*R;
    }
    
    if (MERGE_HITS_AT_BOUNDARY && min_row<100 && max_row>=100
	&& Emax>MIN_CLUSTER_SEED_ENERGY){
      // Handle the interface between the insert and the lead glass blocks
      PeakInfo myPeak(Esum,clusterHits[jmax].x,clusterHits[jmax].y,0,0,
		      DFCALCluster::AT_BOUNDARY);
      bool good_fit=FitPeaks(W,b,clusterHits,peaks,myPeak,chisq,ndf);
      if (good_fit){
	peaks.push_back(myPeak);
      }
      else{
	// Use energy-weighted average for position and put myPeak in peak 
	// list
	myPeak.x=0.;
	myPeak.y=0.;
	for (unsigned int j=0;j<num_hits;j++){
	  double Ej=clusterHits[j].E;
	  myPeak.x+=Ej*clusterHits[j].x;
	  myPeak.y+=Ej*clusterHits[j].y;
	}
	myPeak.x/=Esum;
	myPeak.y/=Esum;
	peaks.push_back(myPeak);
	
	// Compute chisq estimate just in case we need to make a split
	chisq=0.;
	ndf=num_hits-3;
	for (unsigned int j=0;j<num_hits;j++){
	  double dE=clusterHits[j].E
	    -Esum*CalcClusterEDeriv(b,clusterHits[j],myPeak);
	  chisq+=W(j,j)*dE*dE;
	}
      }
    }
    else {
      // Handle clusters fully in insert or in lead glass region   
      // Loop over hits looking for peaks.
      vector<PeakInfo>peak_candidates;
      double Emax=0.;
      int ir_max=0,ic_max=0;
     // First find the peak corresponding with the highest energy block
      for (int ic=1;ic<num_cols-1;ic++){
	for (int ir=1;ir<num_rows-1;ir++){
	  double E=Emap[ic][ir];
	  if (E>Emax){
	    Emax=E;
	    ir_max=ir;
	    ic_max=ic;
	  }
	}
      }
      double Epeak=0;
      int lo_col=0,hi_col=0,lo_row=0,hi_row=0;
      // Maximum distance in row/column number from peak 
      int idiff=(min_row<100)?1:2;
      idiff=1;
      GetRowColRanges(idiff,num_rows,num_cols,ir_max,ic_max,lo_row,
		      hi_row,lo_col,hi_col);
      for (int my_ir=lo_row;my_ir<=hi_row;my_ir++){
	for (int my_ic=lo_col;my_ic<=hi_col;my_ic++){
	  double E=Emap[my_ic][my_ir];
	  if (E>0) Epeak+=E;
	}
      }
      // x and y will be filled in later
      //cout << "Max ic " << ic_max << " ir " << ir_max << endl;
      peak_candidates.push_back(PeakInfo(Epeak,0.,0.,ic_max,ir_max,0));
      
      // Look for additional peaks
      if (num_hits>6){
	for (int ic=1;ic<num_cols-1;ic++){
	  for (int ir=1;ir<num_rows-1;ir++){
	    if (ir==ir_max && ic==ic_max) continue;
	    
	    double E=Emap[ic][ir];
	    if (E>MIN_CLUSTER_SEED_ENERGY){
	      bool got_peak=true;
	      int lo_col=ic-1;
	      int hi_col=ic+1;
	      int lo_row=ir-1;
	      int hi_row=ir+1;
	      int nhits_in_peak=1;
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
		//cout << "ic " << ic << " ir " << ir << endl;
		peak_candidates.push_back(PeakInfo(Epeak,0.,0.,ic,ir,0));
		if (num_hits<=3*(peak_candidates.size()+1)) break;
	      }
	    }// cut on minimum energy of central block
	  } // loop over rows
	} // loop over columns
      } // check that there are enough hits to justify looking for another peak
      
      // Sort peak candidates by energy
      //cout << "---> Candidates:   "<< peak_candidates.size() << endl;
      if (peak_candidates.size()>0){
	sort(peak_candidates.begin(),peak_candidates.end(),peak_E_cmp);
      }
    
      // Loop over peak candidates to perform the peak fit and add good 
      // candidates to the output list.      
      for (size_t i=0;i<peak_candidates.size();i++){
	PeakInfo myPeak=peak_candidates[i];
	
	int lo_col=0,hi_col=0,lo_row=0,hi_row=0;
	GetRowColRanges(idiff,num_rows,num_cols,myPeak.ir,myPeak.ic,
			lo_row,hi_row,lo_col,hi_col);
	// Find the centroid for the peak candidate
	double x=0.,y=0.;
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
	// Guesses for peak position parameters
	myPeak.x=x/myPeak.E;
	myPeak.y=y/myPeak.E;
	
	if (myPeak.E>MIN_CLUSTER_SEED_ENERGY){
	  // Save the current peak list
	  vector<PeakInfo>saved_peaks=peaks;
	  PeakInfo peak_guess=myPeak;

	  // Fit the data to find the best current guesses for the shower 
	  // parameters for each peak within this group of FCAL hits.	  
	  chisq_old=chisq;
	  ndf_old=ndf;
	  bool good_fit=FitPeaks(W,b,clusterHits,peaks,myPeak,chisq,ndf);
	  //cout << "Main fits " << chisq/ndf << " " << chisq_old/ndf_old<<endl;
	  if (good_fit){
	    if (peaks.size()>0) myPeak.status=DFCALCluster::EXTRA_PEAK;
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
	    // No improvement from adding the new peak. Restore the old
	    // list
	    peaks=saved_peaks;
	    chisq=chisq_old;
	    ndf=ndf_old;
	  }
	} // check threshold
      } // loop over peak candidates
    }
  
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
	int lo_col=0,hi_col=0,lo_row=0,hi_row=0;
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
	if (excessE/Esum>MIN_E_FRACTION){	  
	  double x=clusterHits[mmax].x;
	  double y=clusterHits[mmax].y;
	  PeakInfo myPeak(excessE,x,y,ic,ir,DFCALCluster::SPLIT_CLUSTER);
	    
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
	}
      } // Check for minimum excess energy
    } // if peak splitting is enabled

    if (DEBUG_HISTS&&num_hits>3&&peaks.size()==1){
      HistProb->Fill(TMath::Prob(chisq,num_hits-3));
      
      PeakInfo myPeakInfo=peaks[0];
      for (unsigned int k=0;k<clusterHits.size();k++){
	double Ecalc=0.,E=clusterHits[k].E;
	Ecalc+=myPeakInfo.E*CalcClusterEDeriv(b,clusterHits[k],myPeakInfo);
	HistdE->Fill(E,E-Ecalc);
      }
    }
    
    // Estimate fraction of "seen" energy for each peak
    size_t npeaks=peaks.size();
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
      DFCALCluster *myCluster= new DFCALCluster(0);
      myCluster->setStatus(peaks[k].status);
      // using the measured energy gives slightly better energy resolution
      double E=peak_fractions[k]*Esum;
      //cout << "E " << E << " " << peaks[k].E << " status " << peaks[k].status << endl;
      myCluster->setEnergy(E);
      //myCluster->setEnergy(peaks[k].E);

      // Compute energy-weighted time and find channel corresponding to 
      // peak position and add hits to the cluster object
      unsigned int jmax=0.;
      double fsum=0.,fmax=0.,t=0.;
      for (unsigned int j=0;j<clusterHits.size();j++){
	double f=CalcClusterEDeriv(b,clusterHits[j],peaks[k]);
	t+=f*clusterHits[j].t;
	fsum+=f;
	if (f>fmax){
	  fmax=f;
	  jmax=j;
	}
      }

      myCluster->setTimeEWeight(t/fsum);
      int channel=dFCALGeom->channel(clusterHits[jmax].row,
				     clusterHits[jmax].column);
      myCluster->setChannelEmax(channel);
      
      double xc=peaks[k].x,yc=peaks[k].y;
      if (APPLY_S_CURVE_CORRECTION){
	DVector2 blockPos=dFCALGeom->positionOnFace(channel);
	double dx=blockPos.X()-xc;
	double dy=blockPos.Y()-yc;	  
	if (dFCALGeom->inInsert(channel)==false){
	  xc+=S_CURVE_PAR1*sin(S_CURVE_PAR2*(dx-S_CURVE_PAR3));
	  yc+=S_CURVE_PAR1*sin(S_CURVE_PAR2*(dy-S_CURVE_PAR3));
	}
	else{
	  xc+=INSERT_S_CURVE_PAR1*sin(INSERT_S_CURVE_PAR2*dx);
	  yc+=INSERT_S_CURVE_PAR1*sin(INSERT_S_CURVE_PAR2*dy);
	}
      }
      myCluster->setCentroid(xc,yc);
      
      // Add hits to the cluster object as associated objects
      for (unsigned int j=0;j<clusterHits.size();j++){
	int channel=dFCALGeom->channel(clusterHits[j].row,
				       clusterHits[j].column);
	if (npeaks==1){
	  myCluster->addHit(clusterHits[j].id,channel,clusterHits[j].E,
			    clusterHits[j].x,clusterHits[j].y,
			    clusterHits[j].t);
	}
	else{
	  // Output hits surrounding peak position
	  double dx=clusterHits[j].x-clusterHits[jmax].x;
	  double dy=clusterHits[j].y-clusterHits[jmax].y;
	  double d=dFCALGeom->blockSize();
	  if (dFCALGeom->inInsert(channel)){
	    d=dFCALGeom->insertBlockSize();
	  }
	  double dcut=2.5*d;
	  if (fabs(dx)<dcut && fabs(dy)<dcut){
	    myCluster->addHit(clusterHits[j].id,channel,clusterHits[j].E,
			      clusterHits[j].x,clusterHits[j].y,
			      clusterHits[j].t);
	  }
	}
      }
      _data.push_back(myCluster);
    } // loop over peaks
  } // looper over cluster candidates

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
void DFCALCluster_factory_Island::FindClusterCandidates(vector<HitInfo>&fcal_hits,
							vector<vector<HitInfo>>&clusterCandidates) const {
  vector<HitInfo>clusterCandidate;
  clusterCandidate.push_back(fcal_hits[0]);
  
  vector<bool>used_hits(fcal_hits.size());
  used_hits[0]=true;

  int hits_left=fcal_hits.size()-1;
  int begin_i=1; 
  while (hits_left>0){
    for (size_t i=begin_i;i<fcal_hits.size();i++){
      if (used_hits[i]==true) continue;
      
      double dt=clusterCandidate[0].t-fcal_hits[i].t;
      if (fabs(dt)>TIME_CUT) continue;

      int drow=clusterCandidate[0].row-fcal_hits[i].row;
      int dcol=clusterCandidate[0].column-fcal_hits[i].column;
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
  double borderCut=0.5*dFCALGeom->blockSize()+dFCALGeom->insertBlockSize();
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

	  // look for adjacent clusters between the lead glass and the insert,
	  // if present
	  if (MERGE_HITS_AT_BOUNDARY
	      && dFCALGeom->hitPairHasInsertHit(row1,row2)){
	    double dx=(*iter)[i].x-(*iter2)[j].x;
	    double dy=(*iter)[i].y-(*iter2)[j].y;
	    if (fabs(dx)<borderCut && fabs(dy)<borderCut){
	      matched=true;
	      break;
	    }
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
  //cout << "Finished finding clusters" << endl;
}

// Fit peaks within a cluster containing a list of hits contained in hitList
bool DFCALCluster_factory_Island::FitPeaks(const TMatrixD &W,double b,
					   vector<HitInfo>&hitList,
					   vector<PeakInfo>&peaks,
					   PeakInfo &myNewPeak,double &chisq,
					   unsigned int &ndf
					   ) const {  
  size_t nhits=hitList.size();
  size_t npeaks=peaks.size();  
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
    if (dFCALGeom->isFiducial(myNewPeak.x,myNewPeak.y)==false){
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
	if (dFCALGeom->isFiducial(myPeakInfo.x,myPeakInfo.y)==false){
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
    double frac=min_frac+cutdown_scale*double(k);
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
double DFCALCluster_factory_Island::CalcClusterXYDeriv(bool isXDeriv,double b,
						       const HitInfo &hit,
						       const PeakInfo &myPeakInfo) const {
  double sign1[4]={1,1,-1,-1};
  double sign2[4]={1,-1,-1,1};
  double half_block=0.5*dFCALGeom->sensitiveBlockSize();
  if (dFCALGeom->isInsertBlock(hit.row,hit.column)){
    half_block=0.5*dFCALGeom->insertSensitiveBlockSize();
  }
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
double DFCALCluster_factory_Island::CalcClusterEDeriv(double b,
						      const HitInfo &hit,
						      const PeakInfo &myPeakInfo) const {
  double sign1[4]={1,1,-1,-1};
  double sign2[4]={1,-1,-1,1};
  double half_block=0.5*dFCALGeom->sensitiveBlockSize();
  if (dFCALGeom->isInsertBlock(hit.row,hit.column)){
    half_block=0.5*dFCALGeom->insertSensitiveBlockSize();
  }
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
