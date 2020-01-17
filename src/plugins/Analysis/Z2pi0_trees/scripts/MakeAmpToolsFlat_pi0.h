//////////////////////////////////////////////////////////
// This class has been automatically generated on
// Wed Jan 16 11:29:50 2019 by ROOT version 6.08/06
// from TTree pi0pi0misspb208_TreeFlat/pi0pi0misspb208_TreeFlat
// found on file: treeFlat_DSelector_Z2pi0_trees_p1_signal_10000.root
//////////////////////////////////////////////////////////

#ifndef MakeAmpToolsFlat_pi0_h
#define MakeAmpToolsFlat_pi0_h

#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>

// Header file for the classes stored in the TTree if any.
#include "TLorentzVector.h"

class MakeAmpToolsFlat_pi0 {
public :
   TTree          *fChain;   //!pointer to the analyzed TTree or TChain
   Int_t           fCurrent; //!current Tree number in a TChain

// Fixed size dimensions of array or collections stored in the TTree if any.

   // Declaration of leaf types
   UInt_t          run;
   ULong64_t       event;
   Float_t         weight;
   ULong64_t       numtruepid_final;
   ULong64_t       truepids_decay;
   Bool_t          is_truetop;
   Bool_t          is_truecombo;
   Bool_t          is_bdtcombo;
   Bool_t          rftime;
   Float_t         kin_chisq;
   UInt_t          kin_ndf;
   UInt_t          beam_beamid;
   Bool_t          beam_isgen;
   TLorentzVector  *beam_x4_meas;
   TLorentzVector  *beam_p4_meas;
   TLorentzVector  *beam_x4_kin;
   TLorentzVector  *beam_p4_kin;
   TLorentzVector  *beam_x4_true;
   TLorentzVector  *beam_p4_true;
   TLorentzVector  *decaypi01_p4_kin;
   TLorentzVector  *decaypi02_p4_kin;
   TLorentzVector  *misspb208_p4_kin;
   UInt_t          g1_showid;
   TLorentzVector  *g1_x4_meas;
   TLorentzVector  *g1_p4_meas;
   TLorentzVector  *g1_x4_kin;
   TLorentzVector  *g1_p4_kin;
   Float_t         g1_true_fom;
   TLorentzVector  *g1_x4_true;
   TLorentzVector  *g1_p4_true;
   Float_t         g1_beta_time;
   Float_t         g1_chisq_time;
   UInt_t          g1_ndf_time;
   Float_t         g1_ebcal;
   Float_t         g1_eprebcal;
   Float_t         g1_efcal;
   Float_t         g1_bcal_delphi;
   Float_t         g1_bcal_delz;
   Float_t         g1_fcal_doca;
   UInt_t          g2_showid;
   TLorentzVector  *g2_x4_meas;
   TLorentzVector  *g2_p4_meas;
   TLorentzVector  *g2_x4_kin;
   TLorentzVector  *g2_p4_kin;
   Float_t         g2_true_fom;
   TLorentzVector  *g2_x4_true;
   TLorentzVector  *g2_p4_true;
   Float_t         g2_beta_time;
   Float_t         g2_chisq_time;
   UInt_t          g2_ndf_time;
   Float_t         g2_ebcal;
   Float_t         g2_eprebcal;
   Float_t         g2_efcal;
   Float_t         g2_bcal_delphi;
   Float_t         g2_bcal_delz;
   Float_t         g2_fcal_doca;
   UInt_t          g3_showid;
   TLorentzVector  *g3_x4_meas;
   TLorentzVector  *g3_p4_meas;
   TLorentzVector  *g3_x4_kin;
   TLorentzVector  *g3_p4_kin;
   Float_t         g3_true_fom;
   TLorentzVector  *g3_x4_true;
   TLorentzVector  *g3_p4_true;
   Float_t         g3_beta_time;
   Float_t         g3_chisq_time;
   UInt_t          g3_ndf_time;
   Float_t         g3_ebcal;
   Float_t         g3_eprebcal;
   Float_t         g3_efcal;
   Float_t         g3_bcal_delphi;
   Float_t         g3_bcal_delz;
   Float_t         g3_fcal_doca;
   UInt_t          g4_showid;
   TLorentzVector  *g4_x4_meas;
   TLorentzVector  *g4_p4_meas;
   TLorentzVector  *g4_x4_kin;
   TLorentzVector  *g4_p4_kin;
   Float_t         g4_true_fom;
   TLorentzVector  *g4_x4_true;
   TLorentzVector  *g4_p4_true;
   Float_t         g4_beta_time;
   Float_t         g4_chisq_time;
   UInt_t          g4_ndf_time;
   Float_t         g4_ebcal;
   Float_t         g4_eprebcal;
   Float_t         g4_efcal;
   Float_t         g4_bcal_delphi;
   Float_t         g4_bcal_delz;
   Float_t         g4_fcal_doca;
   Double_t        AccWeight;

   // List of branches
   TBranch        *b_run;   //!
   TBranch        *b_event;   //!
   TBranch        *b_weight;   //!
   TBranch        *b_numtruepid_final;   //!
   TBranch        *b_truepids_decay;   //!
   TBranch        *b_is_truetop;   //!
   TBranch        *b_is_truecombo;   //!
   TBranch        *b_is_bdtcombo;   //!
   TBranch        *b_rftime;   //!
   TBranch        *b_kin_chisq;   //!
   TBranch        *b_kin_ndf;   //!
   TBranch        *b_beam_beamid;   //!
   TBranch        *b_beam_isgen;   //!
   TBranch        *b_beam_x4_meas;   //!
   TBranch        *b_beam_p4_meas;   //!
   TBranch        *b_beam_x4_kin;   //!
   TBranch        *b_beam_p4_kin;   //!
   TBranch        *b_beam_x4_true;   //!
   TBranch        *b_beam_p4_true;   //!
   TBranch        *b_decaypi01_p4_kin;   //!
   TBranch        *b_decaypi02_p4_kin;   //!
   TBranch        *b_misspb208_p4_kin;   //!
   TBranch        *b_g1_showid;   //!
   TBranch        *b_g1_x4_meas;   //!
   TBranch        *b_g1_p4_meas;   //!
   TBranch        *b_g1_x4_kin;   //!
   TBranch        *b_g1_p4_kin;   //!
   TBranch        *b_g1_true_fom;   //!
   TBranch        *b_g1_x4_true;   //!
   TBranch        *b_g1_p4_true;   //!
   TBranch        *b_g1_beta_time;   //!
   TBranch        *b_g1_chisq_time;   //!
   TBranch        *b_g1_ndf_time;   //!
   TBranch        *b_g1_ebcal;   //!
   TBranch        *b_g1_eprebcal;   //!
   TBranch        *b_g1_efcal;   //!
   TBranch        *b_g1_bcal_delphi;   //!
   TBranch        *b_g1_bcal_delz;   //!
   TBranch        *b_g1_fcal_doca;   //!
   TBranch        *b_g2_showid;   //!
   TBranch        *b_g2_x4_meas;   //!
   TBranch        *b_g2_p4_meas;   //!
   TBranch        *b_g2_x4_kin;   //!
   TBranch        *b_g2_p4_kin;   //!
   TBranch        *b_g2_true_fom;   //!
   TBranch        *b_g2_x4_true;   //!
   TBranch        *b_g2_p4_true;   //!
   TBranch        *b_g2_beta_time;   //!
   TBranch        *b_g2_chisq_time;   //!
   TBranch        *b_g2_ndf_time;   //!
   TBranch        *b_g2_ebcal;   //!
   TBranch        *b_g2_eprebcal;   //!
   TBranch        *b_g2_efcal;   //!
   TBranch        *b_g2_bcal_delphi;   //!
   TBranch        *b_g2_bcal_delz;   //!
   TBranch        *b_g2_fcal_doca;   //!
   TBranch        *b_g3_showid;   //!
   TBranch        *b_g3_x4_meas;   //!
   TBranch        *b_g3_p4_meas;   //!
   TBranch        *b_g3_x4_kin;   //!
   TBranch        *b_g3_p4_kin;   //!
   TBranch        *b_g3_true_fom;   //!
   TBranch        *b_g3_x4_true;   //!
   TBranch        *b_g3_p4_true;   //!
   TBranch        *b_g3_beta_time;   //!
   TBranch        *b_g3_chisq_time;   //!
   TBranch        *b_g3_ndf_time;   //!
   TBranch        *b_g3_ebcal;   //!
   TBranch        *b_g3_eprebcal;   //!
   TBranch        *b_g3_efcal;   //!
   TBranch        *b_g3_bcal_delphi;   //!
   TBranch        *b_g3_bcal_delz;   //!
   TBranch        *b_g3_fcal_doca;   //!
   TBranch        *b_g4_showid;   //!
   TBranch        *b_g4_x4_meas;   //!
   TBranch        *b_g4_p4_meas;   //!
   TBranch        *b_g4_x4_kin;   //!
   TBranch        *b_g4_p4_kin;   //!
   TBranch        *b_g4_true_fom;   //!
   TBranch        *b_g4_x4_true;   //!
   TBranch        *b_g4_p4_true;   //!
   TBranch        *b_g4_beta_time;   //!
   TBranch        *b_g4_chisq_time;   //!
   TBranch        *b_g4_ndf_time;   //!
   TBranch        *b_g4_ebcal;   //!
   TBranch        *b_g4_eprebcal;   //!
   TBranch        *b_g4_efcal;   //!
   TBranch        *b_g4_bcal_delphi;   //!
   TBranch        *b_g4_bcal_delz;   //!
   TBranch        *b_g4_fcal_doca;   //!
   TBranch        *b_AccWeight;   //!

   MakeAmpToolsFlat_pi0(TTree *tree=0);
   virtual ~MakeAmpToolsFlat_pi0();
   virtual Int_t    Cut(Long64_t entry);
   virtual Int_t    GetEntry(Long64_t entry);
   virtual Long64_t LoadTree(Long64_t entry);
   virtual void     Init(TTree *tree);
   virtual void     Loop(Int_t foption);
   virtual Bool_t   Notify();
   virtual void     Show(Long64_t entry = -1);


   int m_nPart;
   int m_PID[3];
   float m_e[3];
   float m_px[3];
   float m_py[3];
   float m_pz[3];
   float m_eBeam;
   float m_pxBeam;
   float m_pyBeam;
   float m_pzBeam;
   float m_weight;
   float m_TargetMass;

   TTree *m_OutTree;
   TFile *outFile;
   TTree *m_OutTreeInTime;
   TFile *outFileInTime;
   TTree *m_OutTreeOutTime;
   TFile *outFileOutTime;


};

#endif

#ifdef MakeAmpToolsFlat_pi0_cxx
MakeAmpToolsFlat_pi0::MakeAmpToolsFlat_pi0(TTree *tree) : fChain(0) 
{
// if parameter tree is not specified (or zero), connect the file
// used to generate this class and read the Tree.

      if (_file0) {
	tree = (TTree *) _file0->Get("pi0pi0misspb208_TreeFlat");    // require  input file if provided!
      }
   if (tree == 0) {
      TFile *f = (TFile*)gROOT->GetListOfFiles()->FindObject("treeFlat_DSelector_Z2pi0_trees_p1_signal_10000.root");
      if (!f || !f->IsOpen()) {
         f = new TFile("treeFlat_DSelector_Z2pi0_trees_p1_signal_10000.root");
      }
      f->GetObject("pi0pi0misspb208_TreeFlat",tree);

   }
   Init(tree);
}

MakeAmpToolsFlat_pi0::~MakeAmpToolsFlat_pi0()
{
   if (!fChain) return;
   delete fChain->GetCurrentFile();
}

Int_t MakeAmpToolsFlat_pi0::GetEntry(Long64_t entry)
{
// Read contents of entry.
   if (!fChain) return 0;
   return fChain->GetEntry(entry);
}
Long64_t MakeAmpToolsFlat_pi0::LoadTree(Long64_t entry)
{
// Set the environment to read one entry
   if (!fChain) return -5;
   Long64_t centry = fChain->LoadTree(entry);
   if (centry < 0) return centry;
   if (fChain->GetTreeNumber() != fCurrent) {
      fCurrent = fChain->GetTreeNumber();
      Notify();
   }
   return centry;
}

void MakeAmpToolsFlat_pi0::Init(TTree *tree)
{
   // The Init() function is called when the selector needs to initialize
   // a new tree or chain. Typically here the branch addresses and branch
   // pointers of the tree will be set.
   // It is normally not necessary to make changes to the generated
   // code, but the routine can be extended by the user if needed.
   // Init() will be called many times when running on PROOF
   // (once per file to be processed).

   // Set object pointer
   beam_x4_meas = 0;
   beam_p4_meas = 0;
   beam_x4_kin = 0;
   beam_p4_kin = 0;
   beam_x4_true = 0;
   beam_p4_true = 0;
   decaypi01_p4_kin = 0;
   decaypi02_p4_kin = 0;
   misspb208_p4_kin = 0;
   g1_x4_meas = 0;
   g1_p4_meas = 0;
   g1_x4_kin = 0;
   g1_p4_kin = 0;
   g1_x4_true = 0;
   g1_p4_true = 0;
   g2_x4_meas = 0;
   g2_p4_meas = 0;
   g2_x4_kin = 0;
   g2_p4_kin = 0;
   g2_x4_true = 0;
   g2_p4_true = 0;
   g3_x4_meas = 0;
   g3_p4_meas = 0;
   g3_x4_kin = 0;
   g3_p4_kin = 0;
   g3_x4_true = 0;
   g3_p4_true = 0;
   g4_x4_meas = 0;
   g4_p4_meas = 0;
   g4_x4_kin = 0;
   g4_p4_kin = 0;
   g4_x4_true = 0;
   g4_p4_true = 0;
   // Set branch addresses and branch pointers
   if (!tree) return;
   fChain = tree;
   fCurrent = -1;
   fChain->SetMakeClass(1);

   fChain->SetBranchAddress("run", &run, &b_run);
   fChain->SetBranchAddress("event", &event, &b_event);
   fChain->SetBranchAddress("weight", &weight, &b_weight);
   fChain->SetBranchAddress("numtruepid_final", &numtruepid_final, &b_numtruepid_final);
   fChain->SetBranchAddress("truepids_decay", &truepids_decay, &b_truepids_decay);
   fChain->SetBranchAddress("is_truetop", &is_truetop, &b_is_truetop);
   fChain->SetBranchAddress("is_truecombo", &is_truecombo, &b_is_truecombo);
   fChain->SetBranchAddress("is_bdtcombo", &is_bdtcombo, &b_is_bdtcombo);
   fChain->SetBranchAddress("rftime", &rftime, &b_rftime);
   fChain->SetBranchAddress("kin_chisq", &kin_chisq, &b_kin_chisq);
   fChain->SetBranchAddress("kin_ndf", &kin_ndf, &b_kin_ndf);
   fChain->SetBranchAddress("beam_beamid", &beam_beamid, &b_beam_beamid);
   fChain->SetBranchAddress("beam_isgen", &beam_isgen, &b_beam_isgen);
   fChain->SetBranchAddress("beam_x4_meas", &beam_x4_meas, &b_beam_x4_meas);
   fChain->SetBranchAddress("beam_p4_meas", &beam_p4_meas, &b_beam_p4_meas);
   fChain->SetBranchAddress("beam_x4_kin", &beam_x4_kin, &b_beam_x4_kin);
   fChain->SetBranchAddress("beam_p4_kin", &beam_p4_kin, &b_beam_p4_kin);
   fChain->SetBranchAddress("beam_x4_true", &beam_x4_true, &b_beam_x4_true);
   fChain->SetBranchAddress("beam_p4_true", &beam_p4_true, &b_beam_p4_true);
   fChain->SetBranchAddress("decaypi01_p4_kin", &decaypi01_p4_kin, &b_decaypi01_p4_kin);
   fChain->SetBranchAddress("decaypi02_p4_kin", &decaypi02_p4_kin, &b_decaypi02_p4_kin);
   fChain->SetBranchAddress("misspb208_p4_kin", &misspb208_p4_kin, &b_misspb208_p4_kin);
   fChain->SetBranchAddress("g1_showid", &g1_showid, &b_g1_showid);
   fChain->SetBranchAddress("g1_x4_meas", &g1_x4_meas, &b_g1_x4_meas);
   fChain->SetBranchAddress("g1_p4_meas", &g1_p4_meas, &b_g1_p4_meas);
   fChain->SetBranchAddress("g1_x4_kin", &g1_x4_kin, &b_g1_x4_kin);
   fChain->SetBranchAddress("g1_p4_kin", &g1_p4_kin, &b_g1_p4_kin);
   fChain->SetBranchAddress("g1_true_fom", &g1_true_fom, &b_g1_true_fom);
   fChain->SetBranchAddress("g1_x4_true", &g1_x4_true, &b_g1_x4_true);
   fChain->SetBranchAddress("g1_p4_true", &g1_p4_true, &b_g1_p4_true);
   fChain->SetBranchAddress("g1_beta_time", &g1_beta_time, &b_g1_beta_time);
   fChain->SetBranchAddress("g1_chisq_time", &g1_chisq_time, &b_g1_chisq_time);
   fChain->SetBranchAddress("g1_ndf_time", &g1_ndf_time, &b_g1_ndf_time);
   fChain->SetBranchAddress("g1_ebcal", &g1_ebcal, &b_g1_ebcal);
   fChain->SetBranchAddress("g1_eprebcal", &g1_eprebcal, &b_g1_eprebcal);
   fChain->SetBranchAddress("g1_efcal", &g1_efcal, &b_g1_efcal);
   fChain->SetBranchAddress("g1_bcal_delphi", &g1_bcal_delphi, &b_g1_bcal_delphi);
   fChain->SetBranchAddress("g1_bcal_delz", &g1_bcal_delz, &b_g1_bcal_delz);
   fChain->SetBranchAddress("g1_fcal_doca", &g1_fcal_doca, &b_g1_fcal_doca);
   fChain->SetBranchAddress("g2_showid", &g2_showid, &b_g2_showid);
   fChain->SetBranchAddress("g2_x4_meas", &g2_x4_meas, &b_g2_x4_meas);
   fChain->SetBranchAddress("g2_p4_meas", &g2_p4_meas, &b_g2_p4_meas);
   fChain->SetBranchAddress("g2_x4_kin", &g2_x4_kin, &b_g2_x4_kin);
   fChain->SetBranchAddress("g2_p4_kin", &g2_p4_kin, &b_g2_p4_kin);
   fChain->SetBranchAddress("g2_true_fom", &g2_true_fom, &b_g2_true_fom);
   fChain->SetBranchAddress("g2_x4_true", &g2_x4_true, &b_g2_x4_true);
   fChain->SetBranchAddress("g2_p4_true", &g2_p4_true, &b_g2_p4_true);
   fChain->SetBranchAddress("g2_beta_time", &g2_beta_time, &b_g2_beta_time);
   fChain->SetBranchAddress("g2_chisq_time", &g2_chisq_time, &b_g2_chisq_time);
   fChain->SetBranchAddress("g2_ndf_time", &g2_ndf_time, &b_g2_ndf_time);
   fChain->SetBranchAddress("g2_ebcal", &g2_ebcal, &b_g2_ebcal);
   fChain->SetBranchAddress("g2_eprebcal", &g2_eprebcal, &b_g2_eprebcal);
   fChain->SetBranchAddress("g2_efcal", &g2_efcal, &b_g2_efcal);
   fChain->SetBranchAddress("g2_bcal_delphi", &g2_bcal_delphi, &b_g2_bcal_delphi);
   fChain->SetBranchAddress("g2_bcal_delz", &g2_bcal_delz, &b_g2_bcal_delz);
   fChain->SetBranchAddress("g2_fcal_doca", &g2_fcal_doca, &b_g2_fcal_doca);
   fChain->SetBranchAddress("g3_showid", &g3_showid, &b_g3_showid);
   fChain->SetBranchAddress("g3_x4_meas", &g3_x4_meas, &b_g3_x4_meas);
   fChain->SetBranchAddress("g3_p4_meas", &g3_p4_meas, &b_g3_p4_meas);
   fChain->SetBranchAddress("g3_x4_kin", &g3_x4_kin, &b_g3_x4_kin);
   fChain->SetBranchAddress("g3_p4_kin", &g3_p4_kin, &b_g3_p4_kin);
   fChain->SetBranchAddress("g3_true_fom", &g3_true_fom, &b_g3_true_fom);
   fChain->SetBranchAddress("g3_x4_true", &g3_x4_true, &b_g3_x4_true);
   fChain->SetBranchAddress("g3_p4_true", &g3_p4_true, &b_g3_p4_true);
   fChain->SetBranchAddress("g3_beta_time", &g3_beta_time, &b_g3_beta_time);
   fChain->SetBranchAddress("g3_chisq_time", &g3_chisq_time, &b_g3_chisq_time);
   fChain->SetBranchAddress("g3_ndf_time", &g3_ndf_time, &b_g3_ndf_time);
   fChain->SetBranchAddress("g3_ebcal", &g3_ebcal, &b_g3_ebcal);
   fChain->SetBranchAddress("g3_eprebcal", &g3_eprebcal, &b_g3_eprebcal);
   fChain->SetBranchAddress("g3_efcal", &g3_efcal, &b_g3_efcal);
   fChain->SetBranchAddress("g3_bcal_delphi", &g3_bcal_delphi, &b_g3_bcal_delphi);
   fChain->SetBranchAddress("g3_bcal_delz", &g3_bcal_delz, &b_g3_bcal_delz);
   fChain->SetBranchAddress("g3_fcal_doca", &g3_fcal_doca, &b_g3_fcal_doca);
   fChain->SetBranchAddress("g4_showid", &g4_showid, &b_g4_showid);
   fChain->SetBranchAddress("g4_x4_meas", &g4_x4_meas, &b_g4_x4_meas);
   fChain->SetBranchAddress("g4_p4_meas", &g4_p4_meas, &b_g4_p4_meas);
   fChain->SetBranchAddress("g4_x4_kin", &g4_x4_kin, &b_g4_x4_kin);
   fChain->SetBranchAddress("g4_p4_kin", &g4_p4_kin, &b_g4_p4_kin);
   fChain->SetBranchAddress("g4_true_fom", &g4_true_fom, &b_g4_true_fom);
   fChain->SetBranchAddress("g4_x4_true", &g4_x4_true, &b_g4_x4_true);
   fChain->SetBranchAddress("g4_p4_true", &g4_p4_true, &b_g4_p4_true);
   fChain->SetBranchAddress("g4_beta_time", &g4_beta_time, &b_g4_beta_time);
   fChain->SetBranchAddress("g4_chisq_time", &g4_chisq_time, &b_g4_chisq_time);
   fChain->SetBranchAddress("g4_ndf_time", &g4_ndf_time, &b_g4_ndf_time);
   fChain->SetBranchAddress("g4_ebcal", &g4_ebcal, &b_g4_ebcal);
   fChain->SetBranchAddress("g4_eprebcal", &g4_eprebcal, &b_g4_eprebcal);
   fChain->SetBranchAddress("g4_efcal", &g4_efcal, &b_g4_efcal);
   fChain->SetBranchAddress("g4_bcal_delphi", &g4_bcal_delphi, &b_g4_bcal_delphi);
   fChain->SetBranchAddress("g4_bcal_delz", &g4_bcal_delz, &b_g4_bcal_delz);
   fChain->SetBranchAddress("g4_fcal_doca", &g4_fcal_doca, &b_g4_fcal_doca);
   fChain->SetBranchAddress("AccWeight", &AccWeight, &b_AccWeight);
   Notify();




}

Bool_t MakeAmpToolsFlat_pi0::Notify()
{
   // The Notify() function is called when a new file is opened. This
   // can be either for a new TTree in a TChain or when when a new TTree
   // is started when using PROOF. It is normally not necessary to make changes
   // to the generated code, but the routine can be extended by the
   // user if needed. The return value is currently not used.

   return kTRUE;
}

void MakeAmpToolsFlat_pi0::Show(Long64_t entry)
{
// Print contents of entry.
// If entry is not specified, print current entry
   if (!fChain) return;
   fChain->Show(entry);
}
Int_t MakeAmpToolsFlat_pi0::Cut(Long64_t entry)
{
// This function may be called from Loop.
// returns  1 if entry is accepted.
// returns -1 otherwise.
   return 1;
}
#endif // #ifdef MakeAmpToolsFlat_pi0_cxx
