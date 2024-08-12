#include "JEventProcessor_FCALgains.h"
#include <TLorentzVector.h>
#include "TMath.h"
#include "DANA/DEvent.h"
#include "FCAL/DFCALShower.h"
#include "FCAL/DFCALCluster.h"
#include "FCAL/DFCALHit.h"
#include "ANALYSIS/DAnalysisUtilities.h"
#include "PID/DVertex.h"
#include "GlueX.h"
#include <vector>
#include <deque>
#include <string>
#include <iostream>
#include <algorithm>
#include <stdio.h>
#include <stdlib.h>

static TH1F* InvMass1 = NULL;
static TH1F* InvMass2 = NULL;




extern "C"
{
  void InitPlugin(JApplication *locApplication)
  {
    InitJANAPlugin(locApplication);
    locApplication->Add(new JEventProcessor_FCALgains()); //register this plugin
  }
} // "C"

//------------------
// Init
//------------------
void JEventProcessor_FCALgains::Init()
{
  // This is called once at program startup. If you are creating
  // and filling historgrams in this plugin, you should lock the
  // ROOT mutex like this:
  auto app = GetApplication();
  lockService = app->GetService<JLockService>();
  lockService->RootWriteLock();

  if(InvMass1 && InvMass2 != NULL){
    lockService->RootUnLock();
    return;
  }

  n_channels = 2800;
  m_nElements = n_channels;
  MASS_CUT_LO = 0.1;
    MASS_CUT_HI = 0.17;

      InvMass2 = new TH1F("InvMass2","FCAL diphoton mass (Cluster E > 1000 MeV)",500,0.0,0.5);
      InvMass2->GetXaxis()->SetTitle("invariant mass [GeV]");
      InvMass2->GetYaxis()->SetTitle("Counts / 1 MeV");
      

      h2D_mC = new TH2F( "h2D_mC", "C Matrix as TH2F",
			 n_channels, 0., n_channels, n_channels,0.,n_channels );
      h1D_mD = new TH1F( "h1D_mD", "D Matrix as TH1F",n_channels, 0., n_channels );
      h1D_mL = new TH1F( "h1D_mL", "L Matrix as TH1F",n_channels, 0., n_channels );
      h1D_massbias = new TH1F( "h1D_massbias", "Mass Bias Value (in bin 2)",5,0.,5.);

      h1D_mPi0 = new TH1F( "h1D_mPi0", "Reconstructed Pi0 Mass (pre-cuts)",54,0.03,0.3);
      h1D_mPi0->SetXTitle("GeV/c^2");
      h1D_mPi0->SetYTitle("Counts / 5 MeV");

      h1D_massDiff = new TH1F( "h1D_massDiff", "Mass Reconst^2 - Mass Pi0^2",50,-0.15,0.15);
	h1D_massDiff->SetXTitle("GeV^2");
	h1D_massDiff->SetYTitle("Counts / 0.006 GeV^2");

	h1D_mPi0cuts = new TH1F( "h1D_mPi0cuts", "Reconstructed Pi0 Mass (post-cuts)",54,0.03,0.3);
	h1D_mPi0cuts->SetXTitle("GeV/c^2");
	h1D_mPi0cuts->SetYTitle("Counts / 5 MeV");

	  h1D_mPi0_window = new TH1F( "h1D_mPi0_window", "Reconstructed Pi0 Mass (post-cuts) actually used",54,0.03,0.3);
	  h1D_mPi0_window->SetXTitle("GeV/c^2");
	  h1D_mPi0_window->SetYTitle("Counts / 5 MeV");

	    h1D_nhits_unordered = new TH1F("h1D_nhits_unordered", "Number of hits at channel" ,100,0,100);
	      h1D_nhits_unordered->SetXTitle("Number of hits");
			      
		h1D_nhits = new TH1F("h1D_nhits", "Number of hits as function of channel number" ,n_channels,0,2799);
		  h1D_nhits->SetXTitle("Channel Number");
		  h1D_nhits->SetYTitle("Number of Hits");
				  
			m_fcalgeom = new DFCALGeometry();
			m_pi0mass = 0.1349766;
			m_etamass = 0.54751;
			m_nmesons = 0;
			m_mesonmass = 0.;
			m_massbias = 0.;

			// set up matrices for calibration...
			m_nhits.ResizeTo(m_nElements,1);
			m_mD.ResizeTo(m_nElements,1);
			m_mC.ResizeTo(m_nElements,m_nElements);
			m_mL.ResizeTo(m_nElements,1);
			m_mLt.ResizeTo(m_nElements,1);
			  
			  m_nhits.Zero();
			  m_mD.Zero();
			  m_mC.Zero();
			  m_mL.Zero();
			  m_mLt.Zero();
			  m_event = 0;
			  m_TotPastCuts = 0;

			  lockService->RootUnLock();
}
int JEventProcessor_FCALgains::XYtoAbsNum(int my_x, int my_y)
{
  return m_fcalgeom->channel( my_y/4 +29, my_x/4 +29 );
}


//returns in (x,y) coordinates in cm NOT in single step units or in row,column form
pair<int,int> JEventProcessor_FCALgains::AbsNumtoXY(int channel)
{
  int row =    4*(m_fcalgeom->row(channel) -29);
  int column = 4*(m_fcalgeom->column(channel) -29);
  pair<int,int> xy = make_pair(column,row);
  return xy;
}


//------------------
// BeginRun
//------------------
void JEventProcessor_FCALgains::BeginRun(const std::shared_ptr<const JEvent> &locEvent)
{
  double z_target; double z_fcal;
  const DGeometry* dgeom = GetDGeometry(locEvent);

  dgeom->GetTargetZ(z_target);
  dgeom->GetFCALZ(z_fcal);

  z_diff = z_fcal - z_target;
}


////////




void JEventProcessor_FCALgains::Process(const std::shared_ptr<const JEvent> &locEvent)
{

  // This is called for every event. Use of common resources like writing
  // to a file or filling a histogram should be mutex protected. Using
  // locEvent->Get(...) to get reconstructed objects (and thereby activating the
  // reconstruction algorithm) should be done outside of any mutex lock
  // since multiple threads may call this method at the same time.
  //
  // Here's an example:
  //
  // vector<const MyDataClass*> mydataclasses;
  // locEvent->Get(mydataclasses);
  //
  // GetLockService(locEvent)->RootWriteLock();
  //  ... fill historgrams or trees ...
  // GetLockService(locEvent)->RootUnLock();

  // DOCUMENTATION:
  // ANALYSIS library: https://halldweb1.jlab.org/wiki/index.php/GlueX_Analysis_Software

    vector< const DFCALHit*> hits;
    locEvent->Get( hits );

  vector< const DFCALShower* > locFCALShowers;
  vector< const DVertex* > kinfitVertex;
    vector< const DTrackTimeBased* > locTrackTimeBased;
    if( hits.size() <= 500 ){  // only form clusters and showers if there aren't too many hits

  locEvent->Get(locFCALShowers);
    locEvent->Get(kinfitVertex);
      locEvent->Get(locTrackTimeBased);
}
  
  vector < const DFCALShower * > matchedShowers;
  vector < const DTrackTimeBased* > matchedTracks;
  vector <const DChargedTrackHypothesis*> locParticles;
  
    
  
  
  Double_t kinfitVertexX = 0.0, kinfitVertexY = 0.0, kinfitVertexZ = 0.0;
  
  for (unsigned int i = 0 ; i < kinfitVertex.size(); i++)
    {
      kinfitVertexX = kinfitVertex[i]->dSpacetimeVertex.X();
      kinfitVertexY = kinfitVertex[i]->dSpacetimeVertex.Y();
      kinfitVertexZ = kinfitVertex[i]->dSpacetimeVertex.Z();
    }

  DVector3 norm(0.0,0.0,-1);
  DVector3 pos,mom;
 
  
  for (unsigned int i=0; i < locTrackTimeBased.size() ; ++i){
    for (unsigned int j=0; j< locFCALShowers.size(); ++j){

      Double_t x = locFCALShowers[j]->getPosition().X();
      Double_t y = locFCALShowers[j]->getPosition().Y();
      
      DVector3 pos_FCAL(0,0,638);
      if (locTrackTimeBased[i]->rt->GetIntersectionWithPlane(pos_FCAL,norm,pos,mom,NULL,NULL,NULL,SYS_FCAL)==NOERROR)
	{
	  Double_t trkmass = locTrackTimeBased[i]->mass();
	  Double_t FOM = TMath::Prob(locTrackTimeBased[i]->chisq, locTrackTimeBased[i]->Ndof);
	  Double_t dRho = sqrt(((pos.X() - x)*(pos.X() - x)) + ((pos.Y() - y)* (pos.Y() - y)));
	  if(trkmass < 0.15 && dRho < 5 && FOM > 0.01 ) {  
	    matchedShowers.push_back(locFCALShowers[j]);
	 
	  }
	}

    }
  }
  
    GetLockService(locEvent)->RootWriteLock();
  
    if (locFCALShowers.size() >=2) {
    
  for(unsigned int i=0; i<locFCALShowers.size(); i++)
    {
     if (find(matchedShowers.begin(), matchedShowers.end(),locFCALShowers[i]) != matchedShowers.end()) continue;
     
      const DFCALShower *s1 = locFCALShowers[i];
     
      vector<const DFCALCluster*> associated_clusters1;
     
      s1->Get(associated_clusters1);
      Double_t dx1 = s1->getPosition().X() - kinfitVertexX;
      Double_t dy1 = s1->getPosition().Y() - kinfitVertexY;
      Double_t dz1 = s1->getPosition().Z() - kinfitVertexZ;

      Double_t R1 = sqrt(dx1*dx1 + dy1*dy1 + dz1*dz1);
      Double_t  E1 = s1->getEnergy();
      Double_t  t1 = s1->getTime();
      TLorentzVector sh1_p(E1*dx1/R1, E1*dy1/R1, E1*dz1/R1, E1);
			
      for(unsigned int j=i+1; j<locFCALShowers.size(); j++){
	const DFCALShower *s2 = locFCALShowers[j];
	if (find(matchedShowers.begin(), matchedShowers.end(),s2) != matchedShowers.end()) continue;
	
	vector<const DFCALCluster*> associated_clusters2;
	s2->Get(associated_clusters2);
	Double_t dx2 = s2->getPosition().X() - kinfitVertexX;
	Double_t dy2 = s2->getPosition().Y() - kinfitVertexY;
	Double_t dz2 = s2->getPosition().Z() - kinfitVertexZ;

	Double_t R2 = sqrt(dx2*dx2 + dy2*dy2 + dz2*dz2);
	Double_t E2 = s2->getEnergy();
	Double_t  t2 = s2->getTime();
	
	TLorentzVector sh2_p(E2*dx2/R2, E2*dy2/R2, E2*dz2/R2, E2);
	TLorentzVector ptot = sh1_p+sh2_p;
	Double_t inv_mass = ptot.M();

	if(E1>1&&E2>1 && s1->getPosition().Pt() > 20*k_cm && s2->getPosition().Pt() > 20*k_cm  && (fabs (t1-t2) < 10)) InvMass2->Fill(inv_mass);

	h1D_mPi0->Fill(inv_mass);
        h1D_massDiff->Fill(inv_mass*inv_mass-m_pi0mass*m_pi0mass);
	///        
	if(E1 < 1 || E2 < 1) continue;
	//if(E1 < 0.5 || E2 < 0.5) continue;
	  if(fabs(t1-t2) >= 10.) continue;
	    if(s1->getPosition().Pt() <= 20. || s2->getPosition().Pt() <=20.) continue;
	     if(s1->getPosition().Pt() >= 108. || s2->getPosition().Pt() >=108.) continue;
	      if(inv_mass < 0.03 || inv_mass > 0.3) continue;
		h1D_mPi0cuts->Fill(inv_mass);
		  if(inv_mass < MASS_CUT_LO || inv_mass > MASS_CUT_HI) continue;
		    h1D_mPi0_window->Fill(inv_mass);
        
		      
		      m_massbias   += (inv_mass*inv_mass - m_pi0mass*m_pi0mass);
		      // if(m_event % 1000 == 0) cout << "Current mass bias is " <<  m_massbias << endl;
		      h1D_massbias->SetBinContent(2,m_massbias);
		      
		      for(unsigned int loc_j = 0; loc_j < associated_clusters1.size(); loc_j++)
			{
			  for(unsigned int loc_jj = 0; loc_jj < associated_clusters2.size(); loc_jj++)
			    {
			     
			      vector< DFCALCluster::DFCALClusterHit_t > hits1 = associated_clusters1[loc_j]->GetHits();
			      vector< DFCALCluster::DFCALClusterHit_t > hits2 = associated_clusters2[loc_jj]->GetHits();
    
			      //cout << "Size of hits1: " <<hits1.size()   << endl;
			      //cout << "Size of hits2: " <<hits2.size()   << endl;

			      vector< std::pair <double, pair<int,int> > > frac_en;

			      double e_sum1=0; double e_sum2=0;

			      
			      Int_t numhits_per_cluster1 = associated_clusters1[loc_j]->GetNHits();
			      Int_t numhits_per_cluster2 = associated_clusters2[loc_jj]->GetNHits();
	    
			      // cout << " numhits_per_cluster1: " << numhits_per_cluster1 << endl;
			      // cout << " numhits_per_cluster2: " << numhits_per_cluster2 << endl;
	    
			      if(numhits_per_cluster1 == 1 || numhits_per_cluster2 == 1) continue;
			      for(  int i = 0; i < numhits_per_cluster1; ++i ){
				
				//Get (x,y) for each cluster, convert to a scale factor
				//double hiten1 = hits1[i].E;
				int my_channel = XYtoAbsNum(hits1[i].x,hits1[i].y);

				
				if(NHITS_CUT!=0) {
				  if(nhits_vec[my_channel] < NHITS_CUT) continue;
				    }
				e_sum1+=hits1[i].E;
			      }

			      for(  int i = 0; i < numhits_per_cluster2; ++i ){
				
				//double hiten2 = hits2[i].E;
				int my_channel = XYtoAbsNum(hits2[i].x,hits2[i].y);

				
				
				  if(NHITS_CUT!=0) {
				    if(nhits_vec[my_channel] < NHITS_CUT) continue;
				      }
				  

				    e_sum2+=hits2[i].E;
			      }

			      //cout << " e_sum1: "<< e_sum1   << "E1: " <<E1<< endl;
			      //cout << " e_sum2: "<< e_sum2   << "E2: " <<E2<< endl;

  
			      //Calculate fractional energies, fill matrices
			      for(  int i = 0; i < numhits_per_cluster1; ++i ){
				int my_x = hits1[i].x;
				int my_y = hits1[i].y;
				double my_E = hits1[i].E;
				m_nhits[XYtoAbsNum(my_x,my_y)]+=1;
				// hits2D->Fill(my_x/4,my_y/4,1);
				
				frac_en.push_back(make_pair( (my_E / e_sum1), make_pair(my_x,my_y)));
			      }

			      for(  int i = 0; i < numhits_per_cluster2; ++i ){
				int my_x = hits2[i].x ; 
				int my_y = hits2[i].y ;
				double my_E = hits2[i].E;
				  m_nhits[XYtoAbsNum(my_x,my_y)]+=1;
				  frac_en.push_back(make_pair( (my_E / e_sum2), make_pair(my_x,my_y)));
			      }

			      for(unsigned int i = 0; i < frac_en.size(); ++i) {
				int my_x1 = (frac_en[i].second).first  ;
				int my_y1 = (frac_en[i].second).second  ;
				int my_index1 = XYtoAbsNum(my_x1, my_y1);

				m_mD[my_index1][0] += -(inv_mass * inv_mass - m_pi0mass*m_pi0mass)*(inv_mass * inv_mass)*frac_en[i].first;
				m_mL[my_index1][0] += (inv_mass * inv_mass)*frac_en[i].first;

				for(unsigned int ii = 0; ii < frac_en.size(); ++ii) {
				  int my_x2 = (frac_en[ii].second).first;
				  int my_y2 = (frac_en[ii].second).second;
				  int my_index2 = XYtoAbsNum(my_x2, my_y2);

				  m_mC[my_index1][my_index2] += (inv_mass*inv_mass)*(inv_mass*inv_mass)*(frac_en[i].first)*(frac_en[ii].first);
				}
			      }
				
				for(int p=0; p<n_channels; p++){
				  h1D_mD->SetBinContent(p+1,m_mD[p][0]);
				  h1D_mL->SetBinContent(p+1,m_mL[p][0]);
				    h1D_nhits->SetBinContent(p+1,m_nhits[p][0]);
				      h1D_nhits_unordered->Fill(m_nhits[p][0]);
				     
				      for(int j=0; j<n_channels; j++){
					h2D_mC->SetBinContent(p+1,j+1,m_mC[p][j]);
				      }
				}
				  
				  //Done filling matrices				  

				  }
			}
      }
    }
}
  lockService->RootUnLock();

  //	japp->RootFillUnLock(this); //RELEASE ROOT FILL LOCK
}

//------------------
// EndRun
//------------------
void JEventProcessor_FCALgains::EndRun()
{
  // This is called whenever the run number changes, before it is
  // changed to give you a chance to clean up before processing
  // events from the next run number.

}



//------------------
// Finish
//------------------
void JEventProcessor_FCALgains::Finish()
{
  // Called before program exit after event processing is finished.
}


