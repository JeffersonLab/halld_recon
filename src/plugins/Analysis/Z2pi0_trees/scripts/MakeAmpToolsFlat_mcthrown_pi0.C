#define MakeAmpToolsFlat_mcthrown_pi0_cxx
#include "MakeAmpToolsFlat_mcthrown_pi0.h"
#include <TH2.h>
#include <TStyle.h>
#include <TCanvas.h>
#include <iostream>

void MakeAmpToolsFlat_mcthrown_pi0::Loop()
{
//   In a ROOT session, you can do:
//      root> .L MakeAmpToolsFlat_mcthrown_pi0.C
//      root> MakeAmpToolsFlat_mcthrown_pi0 t
//      root> t.GetEntry(12); // Fill t data members with entry number 12
//      root> t.Show();       // Show values of entry 12
//      root> t.Show(16);     // Read and show values of entry 16
//      root> t.Loop();       // Loop on all entries
//

//     This is the loop skeleton where:
//    jentry is the global entry number in the chain
//    ientry is the entry number in the current Tree
//  Note that the argument to GetEntry must be:
//    jentry for TChain::GetEntry
//    ientry for TTree::GetEntry and TBranch::GetEntry
//
//       To read only selected branches, Insert statements like:
// METHOD1:
//    fChain->SetBranchStatus("*",0);  // disable all branches
//    fChain->SetBranchStatus("branchname",1);  // activate branchname
// METHOD2: replace line
//    fChain->GetEntry(jentry);       //read all branches
//by  b_branchname->GetEntry(ientry); //read only this branch
   if (fChain == 0) return;

   Long64_t nentries = fChain->GetEntriesFast();

   // Initialize output Tree

   outFile = new TFile("AmpToolsInputTree.root", "RECREATE");
   m_OutTree = new TTree("kin", "kin2");

   static size_t locNumFinalStateParticles = 3;

   m_OutTree->Branch("Weight", new float, "Weight/F");
   m_OutTree->Branch("E_Beam", new float, "E_Beam/F");
   m_OutTree->Branch("Px_Beam", new float, "Px_Beam/F");
   m_OutTree->Branch("Py_Beam", new float, "Py_Beam/F");
   m_OutTree->Branch("Pz_Beam", new float, "Pz_Beam/F");
   m_OutTree->Branch("Target_Mass", new float, "Target_Mass/F");
   m_OutTree->Branch("NumFinalState", new int, "NumFinalState/I");
   m_OutTree->Branch("PID_FinalState", new int[locNumFinalStateParticles], "PID_FinalState[NumFinalState]/I");
   m_OutTree->Branch("E_FinalState", new float[locNumFinalStateParticles], "E_FinalState[NumFinalState]/F");
   m_OutTree->Branch("Px_FinalState", new float[locNumFinalStateParticles], "Px_FinalState[NumFinalState]/F");
   m_OutTree->Branch("Py_FinalState", new float[locNumFinalStateParticles], "Py_FinalState[NumFinalState]/F");
   m_OutTree->Branch("Pz_FinalState", new float[locNumFinalStateParticles], "Pz_FinalState[NumFinalState]/F");



   m_OutTree->SetBranchAddress("NumFinalState", &m_nPart);
   m_nPart = 3;

   m_OutTree->SetBranchAddress("Target_Mass", &m_TargetMass);
   m_TargetMass = 208*0.931494;          // Pb mass in GeV.

   m_OutTree->SetBranchAddress("PID_FinalState", m_PID);
   m_PID[0] = 7; m_PID[1] = 7; m_PID[2] = 111;

   m_OutTree->SetBranchAddress("E_FinalState", m_e);
   m_OutTree->SetBranchAddress("Px_FinalState", m_px);
   m_OutTree->SetBranchAddress("Py_FinalState", m_py);
   m_OutTree->SetBranchAddress("Pz_FinalState", m_pz);
   m_OutTree->SetBranchAddress("E_Beam", &m_eBeam);
   m_OutTree->SetBranchAddress("Px_Beam", &m_pxBeam);
   m_OutTree->SetBranchAddress("Py_Beam", &m_pyBeam);
   m_OutTree->SetBranchAddress("Pz_Beam", &m_pzBeam);
   m_OutTree->SetBranchAddress("Weight", &m_weight);


  // Process entries in Tree
  TLorentzVector *decaypi01;
  TLorentzVector *decaypi02;


   Long64_t nbytes = 0, nb = 0;
   for (Long64_t jentry=0; jentry<nentries;jentry++) {
      Long64_t ientry = LoadTree(jentry);
      if (ientry < 0) break;
      cout << "Get next jentry=" << jentry << " nentries="  << nentries << " ientry="  << ientry << " NumThrown=" << NumThrown << endl;
      if (jentry==2995 || jentry==3893 || jentry==8475 || jentry==11106 || jentry==37008) continue;   // skip events that bomb
      nb = fChain->GetEntry(jentry);   nbytes += nb;
      cout << "Get next event=" << jentry << " nb=" << nb << endl;
      // if (Cut(ientry) < 0) continue;
      TLorentzVector *pb208 = (TLorentzVector *)Thrown__P4->At(0);

      if (NumThrown == 7) {
	decaypi01 = (TLorentzVector *)Thrown__P4->At(5);
       decaypi02 = (TLorentzVector *)Thrown__P4->At(6);
      }
      else if (NumThrown == 8) {
        decaypi01 = (TLorentzVector *)Thrown__P4->At(6);
	decaypi02 = (TLorentzVector *)Thrown__P4->At(7);
      }
      else {
	cout << "**** MakeAmpToolsFlat_mcthrown_pi0 - illegal NumThrown=" << NumThrown << endl; 
	continue;
      }

      cout << endl << " RunNumber=" << RunNumber << " EventNumber=" << EventNumber << " NumPIDThrown_FinalState=" << NumPIDThrown_FinalState 
      << " PIDThrown_Decaying=" << PIDThrown_Decaying << " NumThrown=" << NumThrown << endl;
      cout << " mass=" <<  ThrownBeam__P4->M() <<  " PID= " << ThrownBeam__PID << " "; ThrownBeam__P4->Print();
      cout << " mass=" << pb208->M() << " "; pb208->Print();
      cout << " mass=" << decaypi01->M() << " "; decaypi01->Print();
      cout << " mass=" << decaypi02->M() << " "; decaypi02->Print();

      TLorentzVector Target(0,0,0,m_TargetMass);

      TLorentzVector Sum = Target + *ThrownBeam__P4 - *decaypi01 - *decaypi02 - *pb208;

      cout<< "jentry=" << jentry << " px=" << Sum.Px()<< " py=" << Sum.Py()<< " pz=" << Sum.Pz()<< " E=" << Sum.E() << endl;

      m_e[0] = decaypi01->E();
       m_px[0] = decaypi01->Px();
       m_py[0] = decaypi01->Py();
       m_pz[0] = decaypi01->Pz();
       m_e[1] = decaypi02->E();
       m_px[1] = decaypi02->Px();
       m_py[1] = decaypi02->Py();
       m_pz[1] = decaypi02->Pz();
       m_e[2] = pb208->E();
       m_px[2] = pb208->Px();
       m_py[2] = pb208->Py();
       m_pz[2] = pb208->Pz();

       m_eBeam = ThrownBeam__P4->E();
       m_pxBeam = ThrownBeam__P4->Px();
       m_pyBeam = ThrownBeam__P4->Py();
       m_pzBeam = ThrownBeam__P4->Pz();
       m_weight = 1;

       m_OutTree->Fill();

   }

   // write out tree

   cout << "Completed loop: nbytes =" << nbytes << " nentries=" << nentries << endl;
   m_OutTree->Write();
   outFile->Close();

   cout << " nentries=" << nentries << " nb=" << nb << " nbytes=" << nbytes << endl;
}



/* Long64_t nbytes = 0, nb = 0;
   for (Long64_t jentry=0; jentry<nentries;jentry++) {
      Long64_t ientry = LoadTree(jentry);
      if (ientry < 0) break;
      nb = fChain->GetEntry(jentry);   nbytes += nb;
      // if (Cut(ientry) < 0) continue;
   }
   }*/
