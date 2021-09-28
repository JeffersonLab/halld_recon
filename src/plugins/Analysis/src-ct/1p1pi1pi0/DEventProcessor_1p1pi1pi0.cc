// 
//
//    File: DEventProcessor_1p1pi1pi0.cc
// Created: Mon Apr  3 11:38:03 EDT 2006
// Creator: davidl (on Darwin swire-b241.jlab.org 8.4.0 powerpc)
//

#include <map>
using namespace std;

#include "DEventProcessor_1p1pi1pi0.h"

#include <DANA/DApplication.h>

// Tree variables
Int_t eventNumber, nPhotonCandidates;
Double_t beamEnergy[100], beamT[100];
Double_t E_shower[2], x_shower[2], y_shower[2], z_shower[2], t_shower[2];
Bool_t BCAL_shower[2];
Int_t nHyp;
Double_t pX_Proton, pY_Proton, pZ_Proton;
Double_t pX_PiMinus, pY_PiMinus, pZ_PiMinus;
Double_t pX_Photon0, pY_Photon0, pZ_Photon0;
Double_t pX_Photon1, pY_Photon1, pZ_Photon1;
Double_t CL;
Double_t vertex_X, vertex_Y, vertex_Z, vertex_T;

Int_t N_thrown, type_thrown[19], pdgtype_thrown[19], myid_thrown[19], parentid_thrown[19];
Double_t beamEnergy_thrown, pX_thrown[19], pY_thrown[19], pZ_thrown[19], E_thrown[19];

Bool_t trigger;

// Histograms

TH2D * h_BCAL;
TH2D * h_FCAL;
TH1D * h_m2gamma;
TH1D * h_m2pi;

double c = 29.98;
double mN = 0.93892;
double mpip = 0.139570;

// Routine used to create our DEventProcessor
extern "C"{
  void InitPlugin(JApplication *app){
    InitJANAPlugin(app);
    app->AddProcessor(new DEventProcessor_1p1pi1pi0());
  }
} // "C"


//------------------
// init
//------------------
jerror_t DEventProcessor_1p1pi1pi0::init(void)
{
 
  tree1 = new TTree( "tree1", "tree1" );

  tree1->Branch("eventNumber",&eventNumber,"eventNumber/I");
  tree1->Branch("nPhotonCandidates",&nPhotonCandidates,"nPhotonCandidates/I");
  tree1->Branch("beamEnergy",beamEnergy,"beamEnergy[nPhotonCandidates]/D");
  tree1->Branch("beamT",beamT,"beamT[nPhotonCandidates]/D");

  tree1->Branch("x_shower",x_shower,"x_shower[2]/D");
  tree1->Branch("y_shower",y_shower,"y_shower[2]/D");
  tree1->Branch("z_shower",z_shower,"z_shower[2]/D");
  tree1->Branch("E_shower",E_shower,"E_shower[2]/D");
  tree1->Branch("t_shower",t_shower,"t_shower[2]/D");
  tree1->Branch("BCAL_shower",BCAL_shower,"BCAL_shower[2]/O");

  tree1->Branch("pX_Proton",&pX_Proton,"pX_Proton/D");
  tree1->Branch("pY_Proton",&pY_Proton,"pY_Proton/D");
  tree1->Branch("pZ_Proton",&pZ_Proton,"pZ_Proton/D");
  tree1->Branch("pX_PiMinus",&pX_PiMinus,"pX_PiMinus/D");
  tree1->Branch("pY_PiMinus",&pY_PiMinus,"pY_PiMinus/D");
  tree1->Branch("pZ_PiMinus",&pZ_PiMinus,"pZ_PiMinus/D");
  tree1->Branch("pX_Photon0",&pX_Photon0,"pX_Photon0/D");
  tree1->Branch("pY_Photon0",&pY_Photon0,"pY_Photon0/D");
  tree1->Branch("pZ_Photon0",&pZ_Photon0,"pZ_Photon0/D");
  tree1->Branch("pX_Photon1",&pX_Photon1,"pX_Photon1/D");
  tree1->Branch("pY_Photon1",&pY_Photon1,"pY_Photon1/D");
  tree1->Branch("pZ_Photon1",&pZ_Photon1,"pZ_Photon1/D");

  tree1->Branch("CL",&CL,"CL/D");

  tree1->Branch("vertex_X",&vertex_X,"vertex_X/D");
  tree1->Branch("vertex_Y",&vertex_Y,"vertex_Y/D");
  tree1->Branch("vertex_Z",&vertex_Z,"vertex_Z/D");
  tree1->Branch("vertex_T",&vertex_T,"vertex_T/D");

  tree1->Branch("N_thrown",&N_thrown,"N_thrown/I");
  tree1->Branch("pX_thrown",pX_thrown,"pX_thrown[N_thrown]/D");
  tree1->Branch("pY_thrown",pY_thrown,"pY_thrown[N_thrown]/D");
  tree1->Branch("pZ_thrown",pZ_thrown,"pZ_thrown[N_thrown]/D");
  tree1->Branch("E_thrown",E_thrown,"E_thrown[N_thrown]/D");
  tree1->Branch("type_thrown",type_thrown,"type_thrown[N_thrown]/I");
  tree1->Branch("pdgtype_thrown",pdgtype_thrown,"pdgtype_thrown[N_thrown]/I");
  tree1->Branch("myid_thrown",myid_thrown,"myid_thrown[N_thrown]/I");
  tree1->Branch("parentid_thrown",parentid_thrown,"parentid_thrown[N_thrown]/I");

  h_BCAL = new TH2D("h_BCAL","BCAL;z [cm];r [cm]",100,-200.,800.,100,0.,200.);
  h_FCAL = new TH2D("h_FCAL","FCAL;z [cm];r [cm]",100,-200.,800.,100,0.,200.);
  h_m2gamma = new TH1D("h_m2gamma",";m2gamma [GeV];",100,0.,1.);
  h_m2pi = new TH1D("h_m2pi",";m2pi [GeV];",250,0.,5.);

  return NOERROR;
}

//------------------
// brun
//------------------
jerror_t DEventProcessor_1p1pi1pi0::brun(JEventLoop *eventLoop, int32_t runnumber)
{

  return NOERROR;
}

//------------------
// evnt
//------------------
jerror_t DEventProcessor_1p1pi1pi0::evnt(JEventLoop *loop, uint64_t eventnumber)
{

  // Pulling detector information
  vector<const DBeamPhoton*> beam_ph;

  vector<const DChargedTrack*>ch_tracks;

  vector<const DNeutralShower*> showers;

  vector<const DMCThrown*> mcthrown;

  vector<const DMCReaction*> reaction;

  loop->Get(beam_ph);

  loop->Get(ch_tracks);

  loop->Get(showers);

  loop->Get(mcthrown);

  loop->Get(reaction);

  japp->RootWriteLock();

  eventNumber = eventnumber;

  // Clearing output arrays                                                                             
  for (unsigned int ii = 0; ii < 100; ii++)
    {
      beamEnergy[ii] = 0;
      beamT[ii] = 0;
    }                
  for (unsigned int ii = 0; ii < 2; ii++)
    {
      E_shower[ii] = 0;
      x_shower[ii] = 0;
      y_shower[ii] = 0;
      z_shower[ii] = 0;
      t_shower[ii] = 0;
      BCAL_shower[ii] = false;
    }

  pX_Proton = 0;
  pY_Proton = 0;
  pZ_Proton = 0;
  pX_PiMinus = 0;
  pY_PiMinus = 0;
  pZ_PiMinus = 0;
  pX_Photon0 = 0;
  pY_Photon0 = 0;
  pZ_Photon0 = 0;
  pX_Photon1 = 0;
  pY_Photon1 = 0;
  pZ_Photon1 = 0;
  
  CL = 0;
  vertex_X = 0;
  vertex_Y = 0;
  vertex_Z = 0;
  vertex_T = 0;
  
  for (unsigned int ii = 0; ii < 19; ii++)
    {
      type_thrown[ii] = 0;
      pX_thrown[ii] = 0;
      pY_thrown[ii] = 0;
      pZ_thrown[ii] = 0;
      E_thrown[ii] = 0;
    }

  beamEnergy_thrown = 0;

  N_thrown = mcthrown.size();

  if ((N_thrown > 0) and (N_thrown < 19))
    {

      beamEnergy_thrown = reaction[0]->beam.energy();
      
      for (unsigned int ii = 0; ii < N_thrown; ii++)
	{
	  if (N_thrown != mcthrown.size())
	    cout << "\n"<< N_thrown <<"\t"<< mcthrown.size()<<"\t"<<ii<<"\ta\n"; 
	  type_thrown[ii] = mcthrown[ii]->type;
	  pX_thrown[ii] = mcthrown[ii]->momentum().X();
	  pY_thrown[ii] = mcthrown[ii]->momentum().Y();
	  pZ_thrown[ii] = mcthrown[ii]->momentum().Z();
	  E_thrown[ii] = mcthrown[ii]->energy();
          pdgtype_thrown[ii] = mcthrown[ii]->pdgtype;
          myid_thrown[ii] = mcthrown[ii]->myid;
          parentid_thrown[ii] = mcthrown[ii]->parentid;

	}

    }

  N_thrown = mcthrown.size();  

  fcal_ncl    = 0;
  fcal_en_cl  = 0;
  fcal_x_cl   = 0;
  fcal_y_cl   = 0;

  if ((ch_tracks.size() != 2) || (showers.size() != 2))
    {
      japp->RootUnLock();

      return NOERROR;
    }

  map<Particle_t, int> targetParticles = {
    {Proton,1},
    {PiMinus,1}
  };

  map<Particle_t, vector<const DChargedTrackHypothesis*> > emptyHypothesis;
  vector<map<Particle_t, vector<const DChargedTrackHypothesis*> > > hypothesisList;
  
  GetHypotheses(ch_tracks,targetParticles,emptyHypothesis,hypothesisList);
  
  nHyp = hypothesisList.size();
  
  nPhotonCandidates = beam_ph.size();
  
  if(beam_ph.size() > 0)
    {
      
      for(unsigned int ii = 0; ii < beam_ph.size(); ii++)
	{
	  beamEnergy[ii] = beam_ph[ii]->momentum().Mag();
	  beamT[ii] = beam_ph[ii]->time();
	}
      
    }
  
  for (unsigned int ii = 0; ii<2; ii++)
    {
      
      const DNeutralShower *shower = showers[ii];
      
      E_shower[ii] = shower->dEnergy;
      x_shower[ii] = shower->dSpacetimeVertex.X();
      y_shower[ii] = shower->dSpacetimeVertex.Y();
      z_shower[ii] = shower->dSpacetimeVertex.Z();
      t_shower[ii] = shower->dSpacetimeVertex.T();
      BCAL_shower[ii] = (shower->dDetectorSystem == SYS_BCAL);
      
      // Filling shower positions                                                              
      if (BCAL_shower[ii])
	{
	  h_BCAL->Fill(z_shower[ii],shower->dSpacetimeVertex.Perp());
	}
      else
	{
	  h_FCAL->Fill(z_shower[ii],shower->dSpacetimeVertex.Perp());
	}
      
    }
  
  if (nHyp == 0)
    {
      japp->RootUnLock();
      
      return NOERROR;
    }
  
  for (unsigned int ii = 0; ii < nHyp; ii++)
    {
      
      map<Particle_t, vector<const DChargedTrackHypothesis*> > thisHyp = hypothesisList[ii];
      const DChargedTrackHypothesis * hyp_pr = thisHyp[Proton][0];
      const DChargedTrackHypothesis * hyp_pim = thisHyp[PiMinus][0];
      
      DKinFitUtils_GlueX *dKinFitUtils = new DKinFitUtils_GlueX(loop);
      DKinFitter *dKinFitter = new DKinFitter(dKinFitUtils);
      
      dKinFitUtils->Reset_NewEvent();
      dKinFitter->Reset_NewEvent();
      dKinFitter->Reset_NewFit();
      
      set<shared_ptr<DKinFitParticle> > FinalParticles, FinalPhotons, NoParticles;
      
      shared_ptr<DKinFitParticle> myProton = dKinFitUtils->Make_DetectedParticle(hyp_pr->Get_TrackTimeBased());
      shared_ptr<DKinFitParticle> myPiMinus = dKinFitUtils->Make_DetectedParticle(hyp_pim->Get_TrackTimeBased());
      FinalParticles.insert(myProton);
      FinalParticles.insert(myPiMinus);
      
      shared_ptr<DKinFitParticle> myPhoton0 = dKinFitUtils->Make_DetectedShower(showers[0],Gamma);
      shared_ptr<DKinFitParticle> myPhoton1 = dKinFitUtils->Make_DetectedShower(showers[1],Gamma);
      FinalParticles.insert(myPhoton0);
      FinalParticles.insert(myPhoton1);
      FinalPhotons.insert(myPhoton0);
      FinalPhotons.insert(myPhoton1);
      
      shared_ptr<DKinFitParticle> myPion = dKinFitUtils->Make_DecayingParticle(Pi0,NoParticles,FinalPhotons);
      
      shared_ptr<DKinFitConstraint_Vertex> locProductionVertexConstraint = dKinFitUtils->Make_VertexConstraint(FinalParticles,NoParticles,hyp_pr->Get_TrackTimeBased()->position());
      dKinFitter->Add_Constraint(locProductionVertexConstraint);
      
      shared_ptr<DKinFitConstraint_Mass> pionMassConstraint = dKinFitUtils->Make_MassConstraint(myPion);
      dKinFitter->Add_Constraint(pionMassConstraint);
            
      dKinFitter->Fit_Reaction();
      
      CL = dKinFitter->Get_ConfidenceLevel();
            
      if (CL == 0)
	continue;
      
      TVector3 vertex;
      double vertex_x, vertex_y, vertex_z, vertex_t;

      shared_ptr<DKinFitParticle> fitProton = NULL;
      shared_ptr<DKinFitParticle> fitPiMinus = NULL;
      shared_ptr<DKinFitParticle> fitPhoton0 = NULL;
      shared_ptr<DKinFitParticle> fitPhoton1 = NULL;
      
      // Loop over particles
      set<shared_ptr<DKinFitParticle> >myParticles = dKinFitter->Get_KinFitParticles();
      set<shared_ptr<DKinFitParticle> >::iterator locParticleIterator=myParticles.begin();
      for(; locParticleIterator != myParticles.end(); ++locParticleIterator)
        {

          int pid = (*locParticleIterator)->Get_PID();

          switch (pid)
            {
            case 2212:
              fitProton = (*locParticleIterator);
              vertex = fitProton->Get_Position();
              vertex_x = vertex.X();
              vertex_y = vertex.Y();
              vertex_z = vertex.Z();
              vertex_t = fitProton->Get_Time();
              break;
	    case -211:
              fitPiMinus = (*locParticleIterator);
              break;
            case 22:
              if (fitPhoton0 == NULL)
                {
                  fitPhoton0 = (*locParticleIterator);
                }
              else
                {
                  fitPhoton1 = (*locParticleIterator);
                }
              break;
            case 111:
              break;
            default:
              cout << pid << "\n\n";
              break;
            }

        }
      
      TVector3 vertexPos(vertex_x,vertex_y,vertex_z);
      
      vertex_X = vertex_x;
      vertex_Y = vertex_y;
      vertex_Z = vertex_z;
      vertex_T = vertex_t;
      
      // Showers using fit vertex
      TVector3 p_shower0(x_shower[0]-vertex_x,y_shower[0]-vertex_y,z_shower[0]-vertex_z);
      p_shower0 = p_shower0.Unit()*E_shower[0];
      
      TVector3 p_shower1(x_shower[1]-vertex_x,y_shower[1]-vertex_y,z_shower[1]-vertex_z);
      p_shower1 = p_shower1.Unit()*E_shower[1];
      
      // Filling invariant mass without fitting
      double m2gammaSq = 2.*(p_shower1.Mag()*p_shower0.Mag() - p_shower1.Dot(p_shower0));
      h_m2gamma->Fill(sqrt(m2gammaSq));
      
      TLorentzVector vProton(fitProton->Get_Momentum(),sqrt(fitProton->Get_Momentum().Mag2() + mN*mN));
      TLorentzVector vPiMinus(fitPiMinus->Get_Momentum(),sqrt(fitPiMinus->Get_Momentum().Mag2() + mpip*mpip));
      TLorentzVector vPhoton1(fitPhoton1->Get_Momentum(),fitPhoton1->Get_Momentum().Mag());
      TLorentzVector vPhoton0(fitPhoton0->Get_Momentum(),fitPhoton0->Get_Momentum().Mag());
      TLorentzVector vPi0 = vPhoton0 + vPhoton1;
      h_m2pi->Fill((vPi0+vPiMinus).M());
      
      pX_Proton = vProton.X();
      pY_Proton = vProton.Y();
      pZ_Proton = vProton.Z();
      pX_PiMinus = vPiMinus.X();
      pY_PiMinus = vPiMinus.Y();
      pZ_PiMinus = vPiMinus.Z();
      pX_Photon0 = vPhoton0.X();
      pY_Photon0 = vPhoton0.Y();
      pZ_Photon0 = vPhoton0.Z();
      pX_Photon1 = vPhoton1.X();
      pY_Photon1 = vPhoton1.Y();
      pZ_Photon1 = vPhoton1.Z();
      
    }
  
  tree1->Fill();
  
  japp->RootUnLock();
  
  return NOERROR;

}

//------------------
// erun
//------------------
jerror_t DEventProcessor_1p1pi1pi0::erun(void)
{
  // Any final calculations on histograms (like dividing them)
  // should be done here. This may get called more than once.
  return NOERROR;
}

//------------------
// fini
//------------------
jerror_t DEventProcessor_1p1pi1pi0::fini(void)
{

  return NOERROR;
}

// Recursive function for determining possible particle assignments
void DEventProcessor_1p1pi1pi0::GetHypotheses(vector<const DChargedTrack *> &tracks,
						map<Particle_t, int> &particles,
						map<Particle_t, vector<const DChargedTrackHypothesis*> > &assignmentHypothesis,
						vector<map<Particle_t, vector<const DChargedTrackHypothesis*> > > &hypothesisList
						) const
{

  const DChargedTrack * firstTrack = tracks.front();
  vector<const DChargedTrack *> otherTracks(tracks);
  otherTracks.erase(otherTracks.begin());
  map<Particle_t, int>::iterator partIt;

  for (partIt = particles.begin(); partIt != particles.end(); partIt++)
    {
      if (partIt->second > 0)
	{

	  Particle_t particle = partIt->first;

	  const DChargedTrackHypothesis *hyp=NULL;
	  
	  if ((hyp = firstTrack->Get_Hypothesis(particle)) != NULL)
	    {
	      
	      // Using dEdX for PID
	      double prob = TMath::Prob(hyp->Get_ChiSq_DCdEdx(),hyp->Get_NDF_DCdEdx());
	      
	      if (prob > 1E-3)
		{
		  
		  map<Particle_t, vector<const DChargedTrackHypothesis*> > newHypothesis = assignmentHypothesis;
		  
		  if (assignmentHypothesis.find(particle) == assignmentHypothesis.end())
		    {
		      vector<const DChargedTrackHypothesis*> newVector;
		      newHypothesis[particle] = newVector;
		    }
		  
		  newHypothesis[particle].push_back(hyp);		    
		  
		  if (otherTracks.empty())
		    {
		      hypothesisList.push_back(newHypothesis);
		    }
		  else
		    {
		      map<Particle_t, int> otherParticles(particles);
		      otherParticles[particle]--;
		      GetHypotheses(otherTracks,
				    otherParticles,
				    newHypothesis,
				    hypothesisList
				    );

		    }
		  
		}
	      
	    }

	}

    }

}
