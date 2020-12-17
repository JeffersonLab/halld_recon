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
    if (clusterCandidates[i].size()==2){
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
    vector<ClusterInfo>clustersToKeep;

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
    double old_E=1e6;
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
      double x=Ec*clusterCandidate[potential_peaks[n]]->x;
      double y=Ec*clusterCandidate[potential_peaks[n]]->y;
      double t=Ec*clusterCandidate[potential_peaks[n]]->t;

      bool got_peak=true;
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
	    E+=Ejk;
	    x+=Ejk*clusterCandidate[imap[j][k]]->x;
	    y+=Ejk*clusterCandidate[imap[j][k]]->y;
	    t+=Ejk*clusterCandidate[imap[j][k]]->t;
	  }
	}
	if (got_peak==false) break;
      }
      if (got_peak){
	x/=E;
	y/=E;
	t/=E;
	clustersToKeep.push_back(ClusterInfo(row_index,col_index,E,x,y,t));
      }
    }
    // Fit the data to find the best current guesses for the shower parameters
    // for each peak within this group of FCAL hits.
    FitPeaks(clusterCandidate,clustersToKeep);

    for (unsigned int k=0;k<clustersToKeep.size();k++){
      DFCALCluster *myCluster= new DFCALCluster(0);
      
      myCluster->setEnergy(clustersToKeep[k].E);
      myCluster->setTimeEWeight(clustersToKeep[k].t);
      myCluster->setCentroid(clustersToKeep[k].x,clustersToKeep[k].y);
      myCluster->setChannelEmax(dFCALGeom->channel(clustersToKeep[k].row_index+min_row,
						   clustersToKeep[k].col_index+min_col));
      
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


void DFCALCluster_factory_Island::FitPeaks(vector<const DFCALHit*>&hitList,
					  vector<ClusterInfo>&clustersToKeep)
  const {
  
  for (unsigned int i=0;i<hitList.size();i++){
    double Ecalc=CalcClusterFunction(hitList[i],clustersToKeep);
    double Ediff=hitList[i]->E-Ecalc;
    cout << Ediff << endl;
  }


}

double DFCALCluster_factory_Island::CalcClusterDeriv(bool isXDeriv,
						     const DFCALHit *hit,
						     vector<ClusterInfo>&clustersToKeep) const {
  double sign1[4]={1,1,-1,-1};
  double sign2[4]={1,-1,-1,1};
  double half_block=0.5*4.;
  double b=0.125; // cm
  double b2=b*b;
  double f=0.;
  for (unsigned int j=0;j<clustersToKeep.size();j++){
    double dxc=hit->x-clustersToKeep[j].x;
    double dyc=hit->y-clustersToKeep[j].y;
    double Ec=clustersToKeep[j].E;
    for (int i=0;i<4;i++){
      double dx=dxc+sign1[i]*half_block;
      double dy=dyc+sign2[i]*half_block;
      double sign3=(i%2)?-1:1;
      double dx2=dx*dx;
      double dy2=dy*dy;
      double temp=b2+dx2+dy2;
      double factor=sign3*Ec*b/(2*M_PI)/sqrt(temp)/(b2*temp+dx2*dy2);
      if (isXDeriv){
	f-=factor*dy*(b2+dy2);
      }
      else {
	f-=factor*dx*(b2+dx2);
      }
    }
  }
  return f;
}
		  

double DFCALCluster_factory_Island::CalcClusterFunction(const DFCALHit *hit,
							vector<ClusterInfo>&clustersToKeep) const {
  double sign1[4]={1,1,-1,-1};
  double sign2[4]={1,-1,-1,1};
  double half_block=0.5*4.;
  double b=0.125; // cm
  double f=0.;
  for (unsigned int j=0;j<clustersToKeep.size();j++){
    double dxc=hit->x-clustersToKeep[j].x;
    double dyc=hit->y-clustersToKeep[j].y;
    double Ec=clustersToKeep[j].E;
    for (int i=0;i<4;i++){
      double dx=dxc+sign1[i]*half_block;
      double dy=dyc+sign2[i]*half_block;
      double sign3=(i%2)?-1:1;
      f+=sign3*Ec/(2*M_PI)*atan(dx*dy/(b*sqrt(b*b+dx*dx+dy*dy)));
    }
  }
  return f;
}
