// $Id$
//
//    File: JEventProcessor_epem_ml_skim.cc
// Updated: JANA2 style, ee/ππ skims with EVIO, histograms, locks

#include "JEventProcessor_epem_ml_skim.h"

// JANA
#include <JANA/JApplication.h>
#include "DANA/DEvent.h"  // GetLockService helper

// GlueX objects
#include "evio_writer/DEventWriterEVIO.h"
#include "FMWPC/DCPPEpEm.h"
#include "PID/DChargedTrack.h"
#include "TRACKING/DTrackTimeBased.h"

// ROOT
#include <TH1.h>
#include <TDirectory.h>
#include <TMath.h>
#include <TLorentzVector.h>
#include <TString.h>

// C++
#include <vector>
#include <cmath>
#include <iostream>

// -----------------------------------------------------------------------------
// Plugin boilerplate (matches your working track2skim)
extern "C" {
void InitPlugin(JApplication *app) {
    InitJANAPlugin(app);
    app->Add(new JEventProcessor_epem_ml_skim());
}
} // "C"


// -----------------------------------------------------------------------------
// ctor/dtor
JEventProcessor_epem_ml_skim::JEventProcessor_epem_ml_skim() {
    // JANA2 pattern: set name here
    SetTypeName("JEventProcessor_epem_ml_skim");
}

JEventProcessor_epem_ml_skim::~JEventProcessor_epem_ml_skim() {}

// -----------------------------------------------------------------------------
// Init: parameters and histogram booking
void JEventProcessor_epem_ml_skim::Init() {
    auto app = GetApplication();

    // Parameters as in your JANA1 version
    app->SetDefaultParameter("epemML:EPEM_EVIO_FILE",   epem_evio_bool,
                             "Write ML-classified e+e- events to EVIO skim");
    app->SetDefaultParameter("epemML:PIPIM_EVIO_FILE",  pippim_evio_bool,
                             "Write ML-classified pi+pi- events to EVIO skim");
    app->SetDefaultParameter("epemML:MIN_EOVERP_CUT",   min_EoverP_cut,
                             "Minimum E_FCAL / |P| cut (applied where relevant)");
    app->SetDefaultParameter("epemML:EE_THRESHOLD_MLP", ee_threshold_MLP,
                             "Minimum MLP response to classify track as e+/e-");
    app->SetDefaultParameter("epemML:PI_THRESHOLD_MLP", pi_threshold_MLP,
                             "Maximum MLP response to classify track as pi+/pi-");

    // ROOT directory layout & histos (single-thread Init is safe)
    TDirectory* main = gDirectory;
    gDirectory->mkdir("epem_ml_skim")->cd();

    gDirectory->mkdir("Tracks")->cd();
    htrack_mom     = new TH1D("htrack_mom",     ";p (GeV/c)",       200, 0, 10);
    htrack_mom_ok  = new TH1D("htrack_mom_ok",  ";p (GeV/c)",       200, 0, 10);
    htrack_chi2    = new TH1D("htrack_chi2",    ";#chi^{2}/NDF",    500, 0.0, 1.0);
    htrack_chi2_ok = new TH1D("htrack_chi2_ok", ";#chi^{2}/NDF",    500, 0.0, 1.0);
    htrack_theta   = new TH1D("htrack_theta",   ";Track #theta",    500, 0.0, 50);
    htrack_theta_ok= new TH1D("htrack_theta_ok",";Track #theta",    500, 0.0, 50);
    htrack_phi     = new TH1D("htrack_phi",     ";Track #phi",      360, -180, 180);
    htrack_phi_ok  = new TH1D("htrack_phi_ok",  ";Track #phi",      360, -180, 180);

    gDirectory->cd("/epem_ml_skim");
    gDirectory->mkdir("Reconstructed")->cd();
    hpimem_ML_classifier = new TH1D("hpimem_ML_classifier",
                                    "ML classifier for #pi^{-}/e^{-};classifier value", 200, 0.0, 1.0);
    hpipep_ML_classifier = new TH1D("hpipep_ML_classifier",
                                    "ML classifier for #pi^{+}/e^{+};classifier value", 200, 0.0, 1.0);


    constexpr int NBINS = 200;
    const double EDGES[NBINS + 1] = {
      0.0000, 0.0002, 0.0006, 0.0013, 0.002, 0.003, 0.0042, 0.0055, 0.007, 0.009, 0.011,
      0.013, 0.015, 0.017, 0.019, 0.021, 0.023, 0.025, 0.027, 0.029, 0.031, 0.033, 0.035, 0.037, 0.039, 0.041, 0.043,
      0.045, 0.047, 0.049, 0.051, 0.053, 0.055, 0.057, 0.059, 0.061, 0.063, 0.065, 0.067, 0.069, 0.071, 0.073, 0.075,
      0.077, 0.079, 0.081, 0.083, 0.085, 0.087, 0.089, 0.091, 0.093, 0.095, 0.097, 0.099, 0.101, 0.103, 0.105, 0.107,
      0.109, 0.111, 0.113, 0.115, 0.117, 0.119, 0.121, 0.123, 0.125, 0.127, 0.129, 0.131, 0.133, 0.135, 0.137, 0.139,
      0.141, 0.143, 0.145, 0.147, 0.149, 0.151, 0.153, 0.155, 0.157, 0.159, 0.161, 0.163, 0.165, 0.167, 0.169, 0.171,
      0.173, 0.175, 0.177, 0.179, 0.181, 0.183, 0.185, 0.187, 0.189, 0.191, 0.193, 0.195, 0.197, 0.199, 0.201, 0.203,
      0.205, 0.207, 0.209, 0.211, 0.213, 0.215, 0.217, 0.219, 0.221, 0.223, 0.225, 0.227, 0.229, 0.231, 0.233, 0.235,
      0.237, 0.239, 0.241, 0.243, 0.245, 0.247, 0.249, 0.251, 0.253, 0.255, 0.257, 0.259, 0.261, 0.263, 0.265, 0.267,
      0.269, 0.271, 0.273, 0.275, 0.277, 0.279, 0.281, 0.283, 0.285, 0.287, 0.289, 0.291, 0.293, 0.295, 0.297, 0.299,
      0.301, 0.303, 0.305, 0.307, 0.309, 0.311, 0.313, 0.315, 0.317, 0.319, 0.321, 0.323, 0.325, 0.327, 0.329, 0.331,
      0.333, 0.335, 0.337, 0.339, 0.341, 0.343, 0.345, 0.347, 0.349, 0.351, 0.353, 0.355, 0.357, 0.359, 0.361, 0.363,
      0.365, 0.367, 0.369, 0.371, 0.373, 0.375, 0.377, 0.379, 0.381, 0.383, 0.385, 0.387, 0.389, 0.391
    };

    static const char* Models[nModels] = {"MLP"};
    static const char* PID[nPID]       = {"ee","pi","noPID"};
    
    for (int i = 0; i < nModels; ++i) {
        for (int j = 0; j < nPID; ++j) {
            hJTphi[i][j]          = new TH1D(Form("JTphi_%s_%s", Models[i], PID[j]),
                                             "#vec{J}_{T}.#phi() ;#phi (degrees)", 90, -180, 180);
            hinvmass[i][j]        = new TH1D(Form("invmass_%s_%s", Models[i], PID[j]),
                                             "Inv. Mass e^{+}e^{-} ;inv. mass (GeV/c^{2})", 234, 0.0, 1.0);
            hFCALElasticity[i][j] = new TH1D(Form("FCALElasticity_%s_%s", Models[i], PID[j]),
                                             ";(E^{FCAL}_{1}+E^{FCAL}_{2})/E_{0}", 200, 0, 1.5);
            hq2_varWidth[i][j]    = new TH1D(Form("q2varWidth_%s_%s", Models[i], PID[j]),
                                             ";q^{2} (GeV/c)^{2}", NBINS, EDGES);
        }
    }
    main->cd();
}

// -----------------------------------------------------------------------------
// BeginRun: place to fetch run-dependent services if needed
void JEventProcessor_epem_ml_skim::BeginRun(const std::shared_ptr<const JEvent>& /*event*/) {
    // No-op for now
}

// -----------------------------------------------------------------------------
// Process: main per-event logic
void JEventProcessor_epem_ml_skim::Process(const std::shared_ptr<const JEvent>& event) {
    ++events_read;

    // Collections
    auto locCPPEpEms        = event->Get<DCPPEpEm>();
    auto chargedTrackVector = event->Get<DChargedTrack>();
    auto detMatches         = event->GetSingle<DDetectorMatches>();

    // Track QA
    for (auto* ctrack : chargedTrackVector) {
        auto* bestHyp   = ctrack->Get_BestTrackingFOM();
        if (bestHyp == nullptr) continue;
        auto* tbt       = bestHyp->Get_TrackTimeBased();
        if (tbt == nullptr) continue;

        // Fill basic kinematics
        {
            auto ls = DEvent::GetLockService(event);
            ls->RootWriteLock();
            htrack_mom->Fill(tbt->pmag());
            htrack_chi2->Fill(tbt->FOM);
            htrack_theta->Fill(tbt->momentum().Theta() * TMath::RadToDeg());
            htrack_phi->Fill(tbt->momentum().Phi()   * TMath::RadToDeg());
            ls->RootUnLock();
        }

        // Require match to TOF or FCAL
        if (detMatches != nullptr) {
            bool matched = detMatches->Get_IsMatchedToDetector(tbt, SYS_TOF)
                        || detMatches->Get_IsMatchedToDetector(tbt, SYS_FCAL);
            if (!matched) continue;
        }

        {
            auto ls = DEvent::GetLockService(event);
            ls->RootWriteLock();
            htrack_mom_ok->Fill(tbt->pmag());
            htrack_chi2_ok->Fill(tbt->FOM);
            htrack_theta_ok->Fill(tbt->momentum().Theta() * TMath::RadToDeg());
            htrack_phi_ok->Fill(tbt->momentum().Phi()   * TMath::RadToDeg());
            ls->RootUnLock();
        }
    }

    // Helper to fill PID bucket histos
    auto fill_all = [&](int pid_index,
                        double Jphi, double FCALElasticity, double W_2e, double q2, double weight) {
        auto ls = DEvent::GetLockService(event);
        ls->RootWriteLock();
        for (int i = 0; i < nModels; ++i) {
            hJTphi[i][pid_index]->Fill(Jphi, weight);
            hFCALElasticity[i][pid_index]->Fill(FCALElasticity, weight);
            hinvmass[i][pid_index]->Fill(W_2e, weight);
            hq2_varWidth[i][pid_index]->Fill(q2, weight);
        }
        ls->RootUnLock();
    };

    // EVIO writer
    const DEventWriterEVIO* locEventWriterEVIO = nullptr;
    event->GetSingle(locEventWriterEVIO);

    // Physics loop over CPP e+e− candidates
    for (auto* cppepem : locCPPEpEms) {
        const double Ebeam  = cppepem->Ebeam;
        const double weight = cppepem->weight;

        const TLorentzVector& ep = cppepem->ep_v4;
        const TLorentzVector& em = cppepem->em_v4;

        const auto* ep_FCAL = cppepem->PositronShower;
        const auto* em_FCAL = cppepem->ElectronShower;

        const double ep_Efcal = ep_FCAL ? ep_FCAL->getEnergy() : 0.0;
        const double em_Efcal = em_FCAL ? em_FCAL->getEnergy() : 0.0;
        const double FCALElasticity = (Ebeam > 0.0) ? (ep_Efcal + em_Efcal) / Ebeam : 0.0;

        // Invariant mass of e+e−, with cut to reject rho^0
        const double W_2e = (ep + em).M();
        if (W_2e > 0.621) continue;

        // J_T phi (optimized a bit)
        const double Ep   = ep.E();
        const double Em   = em.E();
        const double Pep  = ep.P();
        const double Pem  = em.P();
        const double cthp = std::cos(ep.Theta());
        const double cthm = std::cos(em.Theta());

        const double denom_ep = Ep - Pep*cthp;
        const double denom_em = Em - Pem*cthm;
        const double fac_ep   = (denom_ep != 0.0) ? 2.0*Em/denom_ep : 0.0;
        const double fac_em   = (denom_em != 0.0) ? 2.0*Ep/denom_em : 0.0;

        const double JTx  = ep.X()*fac_ep + em.X()*fac_em;
        const double JTy  = ep.Y()*fac_ep + em.Y()*fac_em;
        const double Jphi = std::atan2(JTy, JTx) * (180.0 / TMath::Pi());

        // Classifier values
        const double MLPClassifierMinus = cppepem->pimem_ML_classifier;
        const double MLPClassifierPlus  = cppepem->pipep_ML_classifier;

        {
            auto ls = DEvent::GetLockService(event);
            ls->RootWriteLock();
            hpimem_ML_classifier->Fill(MLPClassifierMinus);
            hpipep_ML_classifier->Fill(MLPClassifierPlus);
            ls->RootUnLock();
        }

        // q2 for histogram: if DCPPEpEm has a q2 member, you may prefer that instead
        // const double q2 = cppepem->q2; // if available
        const double q2 = (ep + em).M2(); // placeholder: W^2; replace with true q^2 if desired

        // ee bucket
        if (MLPClassifierPlus > ee_threshold_MLP && MLPClassifierMinus > ee_threshold_MLP) {
            fill_all(0, Jphi, FCALElasticity, W_2e, q2, weight);
            if (epem_evio_bool && locEventWriterEVIO) {
                // JANA2 writer accepts the shared_ptr<const JEvent>&
                locEventWriterEVIO->Write_EVIOEvent(event, "epem_selection");
                ++ee_events_written;
            }
        }
        // pi bucket (mutually exclusive from ee by thresholds)
        else if (MLPClassifierPlus < pi_threshold_MLP && MLPClassifierMinus < pi_threshold_MLP) {
            fill_all(1, Jphi, FCALElasticity, W_2e, q2, weight);
            if (pippim_evio_bool && locEventWriterEVIO) {
                locEventWriterEVIO->Write_EVIOEvent(event, "pippim_selection");
                ++pi_events_written;
            }
        }

        // noPID bucket always filled
        fill_all(2, Jphi, FCALElasticity, W_2e, q2, weight);
    }
}

// -----------------------------------------------------------------------------
// EndRun
void JEventProcessor_epem_ml_skim::EndRun() {
    // cleanup per-run if needed
}

// -----------------------------------------------------------------------------
// Finish
void JEventProcessor_epem_ml_skim::Finish() {
    std::cout << "[epem_ml_skim] read " << events_read
              << " events; wrote ee=" << ee_events_written
              << ", pi=" << pi_events_written << std::endl;
}
