// $Id$
//
//    File: DEventProcessor_MilleKs.cc
// Created: Mon Feb 22 18:20:38 EST 2021
// Creator: keigo (on Linux ifarm1901.jlab.org 3.10.0-1062.4.1.el7.x86_64 x86_64)
//

#include "DEventProcessor_MilleKs.h"

// Routine used to create our DEventProcessor

extern "C" {
void InitPlugin(JApplication* locApplication) {
  InitJANAPlugin(locApplication);
  locApplication->AddProcessor(new DEventProcessor_MilleKs());  // register this plugin
  locApplication->AddFactoryGenerator(new DFactoryGenerator_MilleKs());  // register the factory generator
}
}  // "C"

jerror_t DEventProcessor_MilleKs::init(void) {
  // This is called once at program startup.
  gPARMS->SetParameter("TRKFIT:ALIGNMENT", true);

  string output_filename;
  gPARMS->GetParameter("OUTPUT_FILENAME", output_filename);
  int ext_pos = output_filename.rfind(".root");
  if (ext_pos != (int)output_filename.size() - 5) {
    jerr << "[MilleKs] Invalid output filename." << endl;
    japp->Quit();
  }
  output_filename.replace(ext_pos, 5, ".mil");
  milleWriter = new Mille(output_filename.data());

  return NOERROR;
}

jerror_t DEventProcessor_MilleKs::brun(jana::JEventLoop* locEventLoop, int32_t locRunNumber) {
  // This is called whenever the run number changes
  // Check for magnetic field
  bfield = NULL;
  DApplication *dapp = dynamic_cast<DApplication *>(locEventLoop->GetJApplication());
  bool dIsNoFieldFlag = (dynamic_cast<const DMagneticFieldMapNoField *>(
                             dapp->GetBfield(locRunNumber)) != nullptr);

  // This plugin is designed for field on data. If this is used for field off
  // data, Abort...
  if (dIsNoFieldFlag) {
    jerr << " ********No Field******** Plugin MilleKs is for use with "
            "magnetic field!!! Aborting "
         << endl;
    japp->Quit();
  }
  bfield = dapp->GetBfield(locRunNumber);


  tree_ = new TTree("MilleKs", "MilleKs");
  tree_->SetAutoSave(100);
  tree_->Branch("m2", &m2_, "m2/D");
  tree_->Branch("m2_measured", &m2_measured_, "m2_measured/D");
  tree_->Branch("m", &m_, "m/D");
  tree_->Branch("m_measured", &m_measured_, "m_measured/D");
  tree_->Branch("conf_lv", &conf_lv_, "conf_lv/D");

  return NOERROR;
}

jerror_t DEventProcessor_MilleKs::evnt(jana::JEventLoop* locEventLoop, uint64_t locEventNumber) {
  // This is called for every event. Use of common resources like writing
  // to a file or filling a histogram should be mutex protected. Using
  // locEventLoop->Get(...) to get reconstructed objects (and thereby activating
  // the reconstruction algorithm) should be done outside of any mutex lock
  // since multiple threads may call this method at the same time.
  //
  // Here's an example:
  //
  // vector<const MyDataClass*> mydataclasses;
  // locEventLoop->Get(mydataclasses);
  //
  // japp->RootFillLock(this);
  //  ... fill historgrams or trees ...
  // japp->RootFillUnLock(this);

  // DOCUMENTATION:
  // ANALYSIS library:
  // https://halldweb1.jlab.org/wiki/index.php/GlueX_Analysis_Software

  /** REQUIRED **/

  // REQUIRED: To run an analysis, You MUST call one at least of the below code
  // fragments. JANA is on-demand, so if you don't call one of these, then your
  // analysis won't run.

  /*
  //Recommended: Write surviving particle combinations (if any) to output ROOT TTree
          //If no cuts are performed by the analysis actions added to a DReaction, then this saves all of its particle combinations.
          //The event writer gets the DAnalysisResults objects from JANA, performing the analysis.
  // string is DReaction factory tag: will fill trees for all DReactions that are defined in the specified factory const DEventWriterROOT*
  locEventWriterROOT = NULL; locEventLoop->GetSingle(locEventWriterROOT);
  locEventWriterROOT->Fill_DataTrees(locEventLoop, "MilleKs");
  */

  // Optional: Get the analysis results for all DReactions.
  // Getting these objects triggers the analysis, if it wasn't performed
  // already. These objects contain the DParticleCombo objects that survived the
  // DAnalysisAction cuts that were added to the DReactions
  vector<const DAnalysisResults*> locAnalysisResultsVector;
  locEventLoop->Get(locAnalysisResultsVector);
  // locAnalysisResultsVector.size() must be 2 for "KsKpPim" and "KsKmPip".

  /** OPTIONAL: FURTHER ANALYSIS **/
  /*
  //Optional: Get all particle combinations for all DReactions.
          //If kinematic fits were requested, these contain both the measured and kinematic-fit track parameters
          //No cuts from DAnalysisActions are placed on these combos
  vector<const DParticleCombo*> locParticleCombos;
  locEventLoop->Get(locParticleCombos);
  for(size_t loc_i = 0; loc_i < locParticleCombos.size(); ++loc_i)
  {
          const DParticleCombo* locParticleCombo = locParticleCombos[loc_i];
          if(locParticleCombo->Get_Reaction()->Get_ReactionName() != "MilleKs") continue; // particle combination was for a different reaction
          //perform further analysis steps here...
  }
  */

  // Optional: Perform further cuts on the particle combos in the analysis results.
  for (size_t loc_i = 0; loc_i < locAnalysisResultsVector.size(); ++loc_i) {
    const DAnalysisResults* locAnalysisResults = locAnalysisResultsVector[loc_i];
    // if (locAnalysisResults->Get_Reaction()->Get_ReactionName() != "KsKpPim") continue;  // analysis results were for a different reaction
    // get the DParticleCombo objects for this DReaction that survived all of
    // the DAnalysisAction cuts
    deque<const DParticleCombo*> locPassedParticleCombos;
    locAnalysisResults->Get_PassedParticleCombos(locPassedParticleCombos);
    for (size_t loc_j = 0; loc_j < locPassedParticleCombos.size(); ++loc_j) {
      const DParticleCombo* locPassedParticleCombo = locPassedParticleCombos[loc_j];
      conf_lv_ = locPassedParticleCombo->Get_KinFitResults()->Get_ConfidenceLevel();
      if (conf_lv_ < 0.05) continue;
      // perform further analysis steps here...
      // locPassedParticleCombo->Get_NumParticleComboSteps() must be 2.
      // 0: g p -> Ks K pi p
      // 1: Ks -> pi+ pi-
      const DParticleComboStep* locParticleComboStep = locPassedParticleCombo->Get_ParticleComboStep(1);

      // see libraries/include/particleType.h

      // locParticleComboStep->Get_NumFinalParticles() must be 2 (pi+ & pi-).
      const DKinematicData* locPiPlus = locParticleComboStep->Get_FinalParticle(0);
      const DKinematicData* locPiMinus = locParticleComboStep->Get_FinalParticle(1);

      const DChargedTrack* pip_track0 = static_cast<const DChargedTrack*>(locParticleComboStep->Get_FinalParticle_SourceObject(0));
      const DChargedTrack* pim_track0 = static_cast<const DChargedTrack*>(locParticleComboStep->Get_FinalParticle_SourceObject(1));
      auto pip_track = pip_track0->Get_Hypothesis(PiPlus)->Get_TrackTimeBased();
      auto pim_track = pim_track0->Get_Hypothesis(PiMinus)->Get_TrackTimeBased();

      // Prepare derivatives.
      // CMS NOTE-2007/032 (5-Oct-2007)
      double DqPlus_Dv[5][3], DqMinus_Dv[5][3];  // \frac{\partial q^+}{\partial v}
      double DqPlus_Dp[5][3], DqMinus_Dp[5][3];  // \frac{\partial q^+}{\partial p^+}
      double DpPlus_Dz[3][5], DpMinus_Dz[3][5];  // \frac{\partial p^+}{\partial z}
      double DqPlus_Dz[5][5], DqMinus_Dz[5][5];  // Products

      double px = locPiPlus->px() + locPiMinus->px();
      double py = locPiPlus->py() + locPiMinus->py();
      double pz = locPiPlus->pz() + locPiMinus->pz();
      double pT = sqrt(px * px + py * py);
      double p = sqrt(pT * pT + pz * pz);
      double m = ParticleMass(PiPlus);
      double M = ParticleMass(KShort);
      double alpha = M / (2 * m);
      DMatrix3x3 R(px * pz / pT / p, -py / pT, px / p,
                   py * pz / pT / p,  px / pT, py / p,
                            -pT / p,      0.0, pz / p);
      double p3 = p * p * p;
      double pT3 = pT * pT * pT;
      DMatrix3x3 DR_Dpx(pz / pT / p - px * px * pz * (p * p + pT * pT) / pT3 / p3,  px * py / pT3, (p * p - px * px) / p3,
                                     -px * py * pz * (p * p + pT * pT) / pT3 / p3,  py * py / pT3,          -px * py / p3,
                                                          -px * pz * pz / pT / p3,            0.0,          -px * pz / p3);
      DMatrix3x3 DR_Dpy(             -px * py * pz * (p * p + pT * pT) / pT3 / p3, -px * px / pT3,          -px * py / p3,
                        pz / pT / p - py * py * pz * (p * p + pT * pT) / pT3 / p3, -px * py / pT3, (p * p - py * py) / p3,
                                                          -py * pz * pz / pT / p3,            0.0,          -py * pz / p3);
      DMatrix3x3 DR_Dpz(px * pT / p3, 0.0, -px * pz / p3,
                        py * pT / p3, 0.0, -py * pz / p3,
                        pz * pT / p3, 0.0,  pT * pT / p3);

      DMatrix3x1 pPlus(locPiPlus->px(), locPiPlus->py(), locPiPlus->pz());
      DMatrix3x1 pMinus(locPiMinus->px(), locPiMinus->py(), locPiMinus->pz());
      DMatrix3x1 p0p = R.Invert() * pPlus;
      DMatrix3x1 p0m = R.Invert() * pMinus;
      double sinthcosphi = p0p(0) / m / sqrt(alpha * alpha - 1);
      double sinthsinphi = p0p(1) / m / sqrt(alpha * alpha - 1);
      double costh = (p0p(2) - p / 2.0) * 2 / sqrt((alpha * alpha - 1) * (p * p + M * M) / alpha / alpha);
      double sinth = sqrt(1.0 - costh * costh);
      double cosphi = sinthcosphi / sinth;
      double sinphi = sinthsinphi / sinth;

      // p_i should be multiplied to get \frac{\partial p_0^+}{\partial p_i} (i = x, y, z)
      DMatrix3x1 dp0p(0.0, 0.0, 0.5 / p + sqrt(alpha * alpha - 1) * costh / 2.0 / alpha / sqrt(p * p + M * M));
      DMatrix3x1 dp0m(0.0, 0.0, 0.5 / p - sqrt(alpha * alpha - 1) * costh / 2.0 / alpha / sqrt(p * p + M * M));

      DMatrix3x1 DpPlus_Dpx = DR_Dpx * p0p + R * (px * dp0p);
      DMatrix3x1 DpPlus_Dpy = DR_Dpy * p0p + R * (py * dp0p);
      DMatrix3x1 DpPlus_Dpz = DR_Dpz * p0p + R * (pz * dp0p);
      DMatrix3x1 DpMinus_Dpx = DR_Dpx * p0m + R * (px * dp0m);
      DMatrix3x1 DpMinus_Dpy = DR_Dpy * p0m + R * (py * dp0m);
      DMatrix3x1 DpMinus_Dpz = DR_Dpz * p0m + R * (pz * dp0m);

      // \frac{\partial}{\partial\theta}
      DMatrix3x1 DpPlus_Dtheta0(m * sqrt(alpha * alpha - 1) * costh * cosphi,
                                m * sqrt(alpha * alpha - 1) * costh * sinphi,
                                -0.5 * sqrt((alpha * alpha - 1) * (p * p + M * M) / alpha / alpha) * sinth);
      DMatrix3x1 DpPlus_Dtheta = R * DpPlus_Dtheta0;
      DMatrix3x1 DpMinus_Dtheta0(-m * sqrt(alpha * alpha - 1) * costh * cosphi,
                                 -m * sqrt(alpha * alpha - 1) * costh * sinphi,
                                 0.5 * sqrt((alpha * alpha - 1) * (p * p + M * M) / alpha / alpha) * sinth);
      DMatrix3x1 DpMinus_Dtheta = R * DpMinus_Dtheta0;

      // \frac{\partial}{\partial\phi}
      DMatrix3x1 DpPlus_Dphi0(m * sqrt(alpha * alpha - 1) * sinth * (-sinphi),
                              m * sqrt(alpha * alpha - 1) * sinth * cosphi, 0.0);
      DMatrix3x1 DpPlus_Dphi = R * DpPlus_Dphi0;
      DMatrix3x1 DpMinus_Dphi0(-m * sqrt(alpha * alpha - 1) * sinth * (-sinphi),
                               -m * sqrt(alpha * alpha - 1) * sinth * cosphi, 0.0);
      DMatrix3x1 DpMinus_Dphi = R * DpMinus_Dphi0;

      for (int i = 0; i < 3; ++i) {
        DpPlus_Dz[i][0] = DpPlus_Dpx(i);
        DpPlus_Dz[i][1] = DpPlus_Dpy(i);
        DpPlus_Dz[i][2] = DpPlus_Dpz(i);
        DpPlus_Dz[i][3] = DpPlus_Dtheta(i);
        DpPlus_Dz[i][4] = DpPlus_Dphi(i);
        DpMinus_Dz[i][0] = DpMinus_Dpx(i);
        DpMinus_Dz[i][1] = DpMinus_Dpy(i);
        DpMinus_Dz[i][2] = DpMinus_Dpz(i);
        DpMinus_Dz[i][3] = DpMinus_Dtheta(i);
        DpMinus_Dz[i][4] = DpMinus_Dphi(i);
      }

      for (int i = 0; i < 5; ++i) {
        for (int j = 0; j < 3; ++j) {
          if ((i == 0 && j == 0) || (i == 1 && j == 1)) {
            DqPlus_Dv[i][j]  = 1.0;
            DqMinus_Dv[i][j] = 1.0;
          } else if (i == 0 && j == 2) {
            DqPlus_Dv[i][j]  = -pip_track->px() / pip_track->pz();
            DqMinus_Dv[i][j] = -pim_track->px() / pim_track->pz();
          } else if (i == 1 && j == 2) {
            DqPlus_Dv[i][j]  = -pip_track->py() / pip_track->pz();
            DqMinus_Dv[i][j] = -pim_track->py() / pim_track->pz();
          } else {
            DqPlus_Dv[i][j] = 0.0;
            DqMinus_Dv[i][j] = 0.0;
          }
        }
      }

      // Fill DqPlus_Dp[5][3]  (\frac{\partial q^+}{\partial p^+})
      double a = -0.00299792458 * abs(bfield->GetBz(locPiPlus->x(), locPiPlus->y(), locPiPlus->z()));  // for pi+ (positively charged)
      double delta_z = pip_track->z() - locPiPlus->z();
      double px_tmp = locPiPlus->px();
      double py_tmp = locPiPlus->px();
      double pz_tmp = locPiPlus->pz();
      double pz_tmp2 = pz_tmp * pz_tmp;
      double pz_tmp3 = pz_tmp2 * pz_tmp;
      double cos_tmp = cos(a * delta_z / pz_tmp);
      double sin_tmp = sin(a * delta_z / pz_tmp);
      double p_tmp = locPiPlus->pmag();
      double p_tmp3 = p_tmp * p_tmp * p_tmp;

      DqPlus_Dp[0][0] = sin_tmp / a;
      DqPlus_Dp[0][1] = (cos_tmp - 1.0) / a;
      DqPlus_Dp[0][2] = -delta_z * (px_tmp * cos_tmp - py_tmp * sin_tmp) / pz_tmp2;
      DqPlus_Dp[1][0] = -(cos_tmp - 1.0) / a;
      DqPlus_Dp[1][1] = sin_tmp / a;
      DqPlus_Dp[1][2] = -delta_z * (px_tmp * sin_tmp + py_tmp * cos_tmp) / pz_tmp2;
      DqPlus_Dp[2][0] = cos_tmp / pz_tmp;
      DqPlus_Dp[2][1] = -sin_tmp / pz_tmp;
      DqPlus_Dp[2][2] = (a * delta_z * py_tmp / pz_tmp3 - px_tmp / pz_tmp2) * cos_tmp + (a * delta_z * px_tmp / pz_tmp3 + py_tmp / pz_tmp2) * sin_tmp;
      DqPlus_Dp[3][0] = sin_tmp / pz_tmp;
      DqPlus_Dp[3][1] = cos_tmp / pz_tmp;
      DqPlus_Dp[3][2] = -(a * delta_z * px_tmp / pz_tmp3 + py_tmp / pz_tmp2) * cos_tmp + (a * delta_z * py_tmp / pz_tmp3 - px_tmp / pz_tmp2) * sin_tmp;
      // Be careful. The following terms depend on charge.
      DqPlus_Dp[4][0] = -px_tmp / p_tmp3;
      DqPlus_Dp[4][1] = -py_tmp / p_tmp3;
      DqPlus_Dp[4][2] = -pz_tmp / p_tmp3;

      // Fill DqMinus_Dp[5][3]  (\frac{\partial q^-}{\partial p^-})
      a *= -1.0;  // for pi- (negatively charged)
      delta_z = pim_track->z() - locPiMinus->z();
      px_tmp = locPiMinus->px();
      py_tmp = locPiMinus->px();
      pz_tmp = locPiMinus->pz();
      pz_tmp2 = pz_tmp * pz_tmp;
      pz_tmp3 = pz_tmp2 * pz_tmp;
      cos_tmp = cos(a * delta_z / pz_tmp);
      sin_tmp = sin(a * delta_z / pz_tmp);
      p_tmp = locPiMinus->pmag();
      p_tmp3 = p_tmp * p_tmp * p_tmp;

      DqMinus_Dp[0][0] = sin_tmp / a;
      DqMinus_Dp[0][1] = (cos_tmp - 1.0) / a;
      DqMinus_Dp[0][2] = -delta_z * (px_tmp * cos_tmp - py_tmp * sin_tmp) / pz_tmp2;
      DqMinus_Dp[1][0] = -(cos_tmp - 1.0) / a;
      DqMinus_Dp[1][1] = sin_tmp / a;
      DqMinus_Dp[1][2] = -delta_z * (px_tmp * sin_tmp + py_tmp * cos_tmp) / pz_tmp2;
      DqMinus_Dp[2][0] = cos_tmp / pz_tmp;
      DqMinus_Dp[2][1] = -sin_tmp / pz_tmp;
      DqMinus_Dp[2][2] = (a * delta_z * py_tmp / pz_tmp3 - px_tmp / pz_tmp2) * cos_tmp + (a * delta_z * px_tmp / pz_tmp3 + py_tmp / pz_tmp2) * sin_tmp;
      DqMinus_Dp[3][0] = sin_tmp / pz_tmp;
      DqMinus_Dp[3][1] = cos_tmp / pz_tmp;
      DqMinus_Dp[3][2] = -(a * delta_z * px_tmp / pz_tmp3 + py_tmp / pz_tmp2) * cos_tmp + (a * delta_z * py_tmp / pz_tmp3 - px_tmp / pz_tmp2) * sin_tmp;
      // Be careful. The following terms depend on charge.
      DqMinus_Dp[4][0] = px_tmp / p_tmp3;
      DqMinus_Dp[4][1] = py_tmp / p_tmp3;
      DqMinus_Dp[4][2] = pz_tmp / p_tmp3;

      // Calculate products.
      for (int i = 0; i < 5; ++i) {
        for (int j = 0; j < 5; ++j) {
          DqPlus_Dz[i][j]  = 0.0;
          DqMinus_Dz[i][j] = 0.0;
          for (int k = 0; k < 3; ++k) {
            DqPlus_Dz[i][j] += DqPlus_Dp[i][k] * DpPlus_Dz[k][j];
            DqMinus_Dz[i][j] += DqMinus_Dp[i][k] * DpMinus_Dz[k][j];
          }
        }
      }


      // Sets TTree variables
      m2_ = (locPiPlus->lorentzMomentum() + locPiMinus->lorentzMomentum()).Mag2();
      double E_pip = sqrt(ParticleMass(PiPlus) * ParticleMass(PiPlus) + pip_track->pmag2());
      double E_pim = sqrt(ParticleMass(PiMinus) * ParticleMass(PiMinus) + pim_track->pmag2());
      m2_measured_ = (E_pip + E_pim) * (E_pip + E_pim) - (pip_track->momentum() + pim_track->momentum()).Mag2();
      m_ = (m2_ < 0.0 ? -1.0 : sqrt(m2_));
      m_measured_ = (m2_measured_ < 0.0 ? -1.0 : sqrt(m2_measured_));

      japp->RootWriteLock();
      tree_->Fill();
      japp->RootUnLock();

      vector<DTrackFitter::pull_t> pip_pulls = pip_track->pulls;
      vector<DTrackFitter::pull_t> pim_pulls = pim_track->pulls;

      // Checks the pull quality.
      bool contains_bad_pulls = false;
      for (size_t iPull = 0; iPull < pip_pulls.size(); ++iPull) {
        float resi = pip_pulls[iPull].resi;  // residual of measurement
        float err = pip_pulls[iPull].err;
        float resic = pip_pulls[iPull].resic;  // residual for FDC cathode
        float errc = pip_pulls[iPull].errc;
        if (resi != resi || err != err || resic != resic || errc != errc || !isfinite(resi) || !isfinite(resic)) {
          contains_bad_pulls = true;
          break;
        }
        // length unit: (cm)
        if (fabs(resi) > 10.0 || err > 10.0 || fabs(resic) > 10.0 || errc > 10.0) {
          contains_bad_pulls = true;
          break;
        }
      }
      for (size_t iPull = 0; iPull < pim_pulls.size(); ++iPull) {
        float resi = pim_pulls[iPull].resi;  // residual of measurement
        float err = pim_pulls[iPull].err;
        float resic = pim_pulls[iPull].resic;  // residual for FDC cathode
        float errc = pim_pulls[iPull].errc;
        if (resi != resi || err != err || resic != resic || errc != errc || !isfinite(resi) || !isfinite(resic)) {
          contains_bad_pulls = true;
          break;
        }
        // length unit: (cm)
        if (fabs(resi) > 10.0 || err > 10.0 || fabs(resic) > 10.0 || errc > 10.0) {
          contains_bad_pulls = true;
          break;
        }
      }
      if (contains_bad_pulls) continue;

      japp->RootWriteLock();  // Just use the root lock as a temporary
      int straw_offset[29] = {0,    0,    42,   84,   138,  192,  258,  324,
                              404,  484,  577,  670,  776,  882,  1005, 1128,
                              1263, 1398, 1544, 1690, 1848, 2006, 2176, 2346,
                              2528, 2710, 2907, 3104, 3313};
      for (size_t iPull = 0; iPull < pip_pulls.size(); ++iPull) {
        float resi = pip_pulls[iPull].resi;  // residual of measurement
        float err = pip_pulls[iPull].err;    // estimated error of measurement
        const DCDCTrackHit *cdc_hit = pip_pulls[iPull].cdc_hit;
        const DFDCPseudo *fdc_hit = pip_pulls[iPull].fdc_hit;
        float resic = pip_pulls[iPull].resic;  // residual for FDC cathode measurement
        float errc = pip_pulls[iPull].errc;

        vector<double> der = pip_pulls[iPull].trackDerivatives;

        if (fdc_hit != nullptr && fdc_hit->status == 6) {
          // Add fdc hit
          DFDCPseudo *hit = const_cast<DFDCPseudo *>(fdc_hit);

          vector<double> pseudo_der = hit->GetFDCPseudoAlignmentDerivatives();
          vector<double> strip_der = hit->GetFDCStripPitchDerivatives();

          int label_layer_offset = 100000 + hit->wire->layer * 1000;

          // For wire measurement.
          const int NLC = 8;
          const int NGL_W = 6;
          float derLc_W[NLC];
          float derGl_W[NGL_W];
          int label_W[NGL_W];

          double Df_Dq_W[5] = {
              der[FDCTrackD::dDOCAW_dx],
              der[FDCTrackD::dDOCAW_dy],
              der[FDCTrackD::dDOCAW_dtx],
              der[FDCTrackD::dDOCAW_dty],
              0.0
          };

          for (int i = 0; i < 3; ++i) {
            derLc_W[i] = 0.0;
            for (int j = 0; j < 5; ++j) derLc_W[i] += Df_Dq_W[j] * DqPlus_Dv[j][i];
          }
          for (int i = 0; i < 5; ++i) {
            derLc_W[i + 3] = 0.0;
            for (int j = 0; j < 5; ++j) derLc_W[i + 3] += Df_Dq_W[j] * DqPlus_Dz[j][i];
          }

          derGl_W[0] = der[FDCTrackD::dDOCAW_dDeltaX];
          derGl_W[1] = der[FDCTrackD::dDOCAW_dDeltaPhiX];
          derGl_W[2] = der[FDCTrackD::dDOCAW_dDeltaPhiY];
          derGl_W[3] = der[FDCTrackD::dDOCAW_dDeltaPhiZ];
          derGl_W[4] = der[FDCTrackD::dDOCAW_dDeltaZ];
          derGl_W[5] = -der[FDCTrackD::dW_dt0];

          label_W[0] = label_layer_offset + 1;
          label_W[1] = label_layer_offset + 2;
          label_W[2] = label_layer_offset + 3;
          label_W[3] = label_layer_offset + 4;
          label_W[4] = label_layer_offset + 5;
          if (fdc_hit->wire->wire <= 48) {
            label_W[5] = label_layer_offset + 997;
          } else {
            label_W[5] = label_layer_offset + 998;
          }

          milleWriter->mille(NLC, derLc_W, NGL_W, derGl_W, label_W, resi, err);

          // For cathode measurement.
          const int NGL_C = 20;
          float derLc_C[NLC];
          float derGl_C[NGL_C];
          int label_C[NGL_C];

          double Df_Dq_C[5] = {
              der[FDCTrackD::dDOCAC_dx],
              der[FDCTrackD::dDOCAC_dy],
              der[FDCTrackD::dDOCAC_dtx],
              der[FDCTrackD::dDOCAC_dty],
              0.0
          };

          for (int i = 0; i < 3; ++i) {
            derLc_C[i] = 0.0;
            for (int j = 0; j < 5; ++j) derLc_C[i] += Df_Dq_C[j] * DqPlus_Dv[j][i];
          }
          for (int i = 0; i < 5; ++i) {
            derLc_C[i + 3] = 0.0;
            for (int j = 0; j < 5; ++j) derLc_C[i + 3] += Df_Dq_C[j] * DqPlus_Dz[j][i];
          }

          derGl_C[0] = -1.0;
          derGl_C[1] = der[FDCTrackD::dDOCAC_dDeltaX];
          derGl_C[2] = der[FDCTrackD::dDOCAC_dDeltaPhiX];
          derGl_C[3] = der[FDCTrackD::dDOCAC_dDeltaPhiY];
          derGl_C[4] = der[FDCTrackD::dDOCAC_dDeltaPhiZ];
          derGl_C[5] = der[FDCTrackD::dDOCAC_dDeltaZ];

          label_C[0] = label_layer_offset + 100;
          label_C[1] = label_layer_offset + 1;
          label_C[2] = label_layer_offset + 2;
          label_C[3] = label_layer_offset + 3;
          label_C[4] = label_layer_offset + 4;
          label_C[5] = label_layer_offset + 5;

          // Cathode U and V offsets
          derGl_C[6] = -pseudo_der[FDCPseudoD::dSddeltaU];
          derGl_C[7] = -pseudo_der[FDCPseudoD::dSddeltaV];
          derGl_C[8] = -pseudo_der[FDCPseudoD::dSddeltaPhiU];
          derGl_C[9] = -pseudo_der[FDCPseudoD::dSddeltaPhiV];

          label_C[6] = label_layer_offset + 101;
          label_C[7] = label_layer_offset + 102;
          label_C[8] = label_layer_offset + 103;
          label_C[9] = label_layer_offset + 104;

          // Strip Pitch Calibration
          derGl_C[10] = -pseudo_der[FDCPseudoD::dSddeltaU] * strip_der[0];
          derGl_C[11] = -pseudo_der[FDCPseudoD::dSddeltaU] * strip_der[1];
          derGl_C[12] = -pseudo_der[FDCPseudoD::dSddeltaU] * strip_der[2];
          derGl_C[13] = -pseudo_der[FDCPseudoD::dSddeltaU] * strip_der[3];
          derGl_C[14] = -pseudo_der[FDCPseudoD::dSddeltaU] * strip_der[4];
          derGl_C[15] = -pseudo_der[FDCPseudoD::dSddeltaV] * strip_der[5];
          derGl_C[16] = -pseudo_der[FDCPseudoD::dSddeltaV] * strip_der[6];
          derGl_C[17] = -pseudo_der[FDCPseudoD::dSddeltaV] * strip_der[7];
          derGl_C[18] = -pseudo_der[FDCPseudoD::dSddeltaV] * strip_der[8];
          derGl_C[19] = -pseudo_der[FDCPseudoD::dSddeltaV] * strip_der[9];

          label_C[10] = label_layer_offset + 200;
          label_C[11] = label_layer_offset + 201;
          label_C[12] = label_layer_offset + 202;
          label_C[13] = label_layer_offset + 203;
          label_C[14] = label_layer_offset + 204;
          label_C[15] = label_layer_offset + 205;
          label_C[16] = label_layer_offset + 206;
          label_C[17] = label_layer_offset + 207;
          label_C[18] = label_layer_offset + 208;
          label_C[19] = label_layer_offset + 209;

          milleWriter->mille(NLC, derLc_C, NGL_C, derGl_C, label_C, resic, errc);
        }

        if (cdc_hit != nullptr) {
          DCDCWire *wire = const_cast<DCDCWire *>(cdc_hit->wire);

          vector<double> wire_der = wire->GetCDCWireDerivatives();

          const int NLC = 8;
          const int NGL = 10;
          float derLc[NLC];
          float derGl[NGL];
          int label[NGL];

          double Df_Dq[5] = {
              der[CDCTrackD::dDOCAdS0],
              der[CDCTrackD::dDOCAdS1],
              der[CDCTrackD::dDOCAdS2],
              der[CDCTrackD::dDOCAdS3],
              der[CDCTrackD::dDOCAdS4]
          };

          for (int i = 0; i < 3; ++i) {
            derLc[i] = 0.0;
            for (int j = 0; j < 5; ++j) derLc[i] += Df_Dq[j] * DqPlus_Dv[j][i];
          }
          for (int i = 0; i < 5; ++i) {
            derLc[i + 3] = 0.0;
            for (int j = 0; j < 5; ++j) derLc[i + 3] += Df_Dq[j] * DqPlus_Dz[j][i];
          }

          derGl[0] =
              der[CDCTrackD::dDOCAdOriginX] * wire_der[CDCWireD::dOriginXddeltaX] +
              der[CDCTrackD::dDOCAdOriginY] * wire_der[CDCWireD::dOriginYddeltaX] +
              der[CDCTrackD::dDOCAdOriginZ] * wire_der[CDCWireD::dOriginZddeltaX] +
              der[CDCTrackD::dDOCAdDirX] * wire_der[CDCWireD::dDirXddeltaX] +
              der[CDCTrackD::dDOCAdDirY] * wire_der[CDCWireD::dDirYddeltaX] +
              der[CDCTrackD::dDOCAdDirZ] * wire_der[CDCWireD::dDirZddeltaX];

          derGl[1] =
              der[CDCTrackD::dDOCAdOriginX] * wire_der[CDCWireD::dOriginXddeltaY] +
              der[CDCTrackD::dDOCAdOriginY] * wire_der[CDCWireD::dOriginYddeltaY] +
              der[CDCTrackD::dDOCAdOriginZ] * wire_der[CDCWireD::dOriginZddeltaY] +
              der[CDCTrackD::dDOCAdDirX] * wire_der[CDCWireD::dDirXddeltaY] +
              der[CDCTrackD::dDOCAdDirY] * wire_der[CDCWireD::dDirYddeltaY] +
              der[CDCTrackD::dDOCAdDirZ] * wire_der[CDCWireD::dDirZddeltaY];

          derGl[2] =
              der[CDCTrackD::dDOCAdOriginX] * wire_der[CDCWireD::dOriginXddeltaZ] +
              der[CDCTrackD::dDOCAdOriginY] * wire_der[CDCWireD::dOriginYddeltaZ] +
              der[CDCTrackD::dDOCAdOriginZ] * wire_der[CDCWireD::dOriginZddeltaZ] +
              der[CDCTrackD::dDOCAdDirX] * wire_der[CDCWireD::dDirXddeltaZ] +
              der[CDCTrackD::dDOCAdDirY] * wire_der[CDCWireD::dDirYddeltaZ] +
              der[CDCTrackD::dDOCAdDirZ] * wire_der[CDCWireD::dDirZddeltaZ];

          derGl[3] =
              der[CDCTrackD::dDOCAdOriginX] * wire_der[CDCWireD::dOriginXddeltaPhiX] +
              der[CDCTrackD::dDOCAdOriginY] * wire_der[CDCWireD::dOriginYddeltaPhiX] +
              der[CDCTrackD::dDOCAdOriginZ] * wire_der[CDCWireD::dOriginZddeltaPhiX] +
              der[CDCTrackD::dDOCAdDirX] * wire_der[CDCWireD::dDirXddeltaPhiX] +
              der[CDCTrackD::dDOCAdDirY] * wire_der[CDCWireD::dDirYddeltaPhiX] +
              der[CDCTrackD::dDOCAdDirZ] * wire_der[CDCWireD::dDirZddeltaPhiX];

          derGl[4] =
              der[CDCTrackD::dDOCAdOriginX] * wire_der[CDCWireD::dOriginXddeltaPhiY] +
              der[CDCTrackD::dDOCAdOriginY] * wire_der[CDCWireD::dOriginYddeltaPhiY] +
              der[CDCTrackD::dDOCAdOriginZ] * wire_der[CDCWireD::dOriginZddeltaPhiY] +
              der[CDCTrackD::dDOCAdDirX] * wire_der[CDCWireD::dDirXddeltaPhiY] +
              der[CDCTrackD::dDOCAdDirY] * wire_der[CDCWireD::dDirYddeltaPhiY] +
              der[CDCTrackD::dDOCAdDirZ] * wire_der[CDCWireD::dDirZddeltaPhiY];

          derGl[5] =
              der[CDCTrackD::dDOCAdOriginX] * wire_der[CDCWireD::dOriginXddeltaPhiZ] +
              der[CDCTrackD::dDOCAdOriginY] * wire_der[CDCWireD::dOriginYddeltaPhiZ] +
              der[CDCTrackD::dDOCAdOriginZ] * wire_der[CDCWireD::dOriginZddeltaPhiZ] +
              der[CDCTrackD::dDOCAdDirX] * wire_der[CDCWireD::dDirXddeltaPhiZ] +
              der[CDCTrackD::dDOCAdDirY] * wire_der[CDCWireD::dDirYddeltaPhiZ] +
              der[CDCTrackD::dDOCAdDirZ] * wire_der[CDCWireD::dDirZddeltaPhiZ];

          derGl[6] =
              der[CDCTrackD::dDOCAdOriginX] * wire_der[CDCWireD::dOriginXddeltaXu] +
              der[CDCTrackD::dDOCAdOriginY] * wire_der[CDCWireD::dOriginYddeltaXu] +
              der[CDCTrackD::dDOCAdOriginZ] * wire_der[CDCWireD::dOriginZddeltaXu] +
              der[CDCTrackD::dDOCAdDirX] * wire_der[CDCWireD::dDirXddeltaXu] +
              der[CDCTrackD::dDOCAdDirY] * wire_der[CDCWireD::dDirYddeltaXu] +
              der[CDCTrackD::dDOCAdDirZ] * wire_der[CDCWireD::dDirZddeltaXu];

          derGl[7] =
              der[CDCTrackD::dDOCAdOriginX] * wire_der[CDCWireD::dOriginXddeltaYu] +
              der[CDCTrackD::dDOCAdOriginY] * wire_der[CDCWireD::dOriginYddeltaYu] +
              der[CDCTrackD::dDOCAdOriginZ] * wire_der[CDCWireD::dOriginZddeltaYu] +
              der[CDCTrackD::dDOCAdDirX] * wire_der[CDCWireD::dDirXddeltaYu] +
              der[CDCTrackD::dDOCAdDirY] * wire_der[CDCWireD::dDirYddeltaYu] +
              der[CDCTrackD::dDOCAdDirZ] * wire_der[CDCWireD::dDirZddeltaYu];

          derGl[8] =
              der[CDCTrackD::dDOCAdOriginX] * wire_der[CDCWireD::dOriginXddeltaXd] +
              der[CDCTrackD::dDOCAdOriginY] * wire_der[CDCWireD::dOriginYddeltaXd] +
              der[CDCTrackD::dDOCAdOriginZ] * wire_der[CDCWireD::dOriginZddeltaXd] +
              der[CDCTrackD::dDOCAdDirX] * wire_der[CDCWireD::dDirXddeltaXd] +
              der[CDCTrackD::dDOCAdDirY] * wire_der[CDCWireD::dDirYddeltaXd] +
              der[CDCTrackD::dDOCAdDirZ] * wire_der[CDCWireD::dDirZddeltaXd];

          derGl[9] =
              der[CDCTrackD::dDOCAdOriginX] * wire_der[CDCWireD::dOriginXddeltaYd] +
              der[CDCTrackD::dDOCAdOriginY] * wire_der[CDCWireD::dOriginYddeltaYd] +
              der[CDCTrackD::dDOCAdOriginZ] * wire_der[CDCWireD::dOriginZddeltaYd] +
              der[CDCTrackD::dDOCAdDirX] * wire_der[CDCWireD::dDirXddeltaYd] +
              der[CDCTrackD::dDOCAdDirY] * wire_der[CDCWireD::dDirYddeltaYd] +
              der[CDCTrackD::dDOCAdDirZ] * wire_der[CDCWireD::dDirZddeltaYd];

          label[0] = 1;
          label[1] = 2;
          label[2] = 3;
          label[3] = 4;
          label[4] = 5;
          label[5] = 6;
          label[6] = 1001 + (straw_offset[wire->ring] + wire->straw - 1) * 4;
          label[7] = 1002 + (straw_offset[wire->ring] + wire->straw - 1) * 4;
          label[8] = 1003 + (straw_offset[wire->ring] + wire->straw - 1) * 4;
          label[9] = 1004 + (straw_offset[wire->ring] + wire->straw - 1) * 4;

          milleWriter->mille(NLC, derLc, NGL, derGl, label, resi, err);
        }
      }
      for (size_t iPull = 0; iPull < pim_pulls.size(); ++iPull) {
        float resi = pim_pulls[iPull].resi;  // residual of measurement
        float err = pim_pulls[iPull].err;    // estimated error of measurement
        const DCDCTrackHit *cdc_hit = pim_pulls[iPull].cdc_hit;
        const DFDCPseudo *fdc_hit = pim_pulls[iPull].fdc_hit;
        float resic = pim_pulls[iPull].resic;  // residual for FDC cathode measurement
        float errc = pim_pulls[iPull].errc;

        vector<double> der = pim_pulls[iPull].trackDerivatives;

        if (fdc_hit != nullptr && fdc_hit->status == 6) {
          // Add fdc hit
          DFDCPseudo *hit = const_cast<DFDCPseudo *>(fdc_hit);

          vector<double> pseudo_der = hit->GetFDCPseudoAlignmentDerivatives();
          vector<double> strip_der = hit->GetFDCStripPitchDerivatives();

          int label_layer_offset = 100000 + hit->wire->layer * 1000;

          // For wire measurement.
          const int NLC = 8;
          const int NGL_W = 6;
          float derLc_W[NLC];
          float derGl_W[NGL_W];
          int label_W[NGL_W];

          double Df_Dq_W[5] = {
              der[FDCTrackD::dDOCAW_dx],
              der[FDCTrackD::dDOCAW_dy],
              der[FDCTrackD::dDOCAW_dtx],
              der[FDCTrackD::dDOCAW_dty],
              0.0
          };

          for (int i = 0; i < 3; ++i) {
            derLc_W[i] = 0.0;
            for (int j = 0; j < 5; ++j) derLc_W[i] += Df_Dq_W[j] * DqMinus_Dv[j][i];
          }
          for (int i = 0; i < 5; ++i) {
            derLc_W[i + 3] = 0.0;
            for (int j = 0; j < 5; ++j) derLc_W[i + 3] += Df_Dq_W[j] * DqMinus_Dz[j][i];
          }

          derGl_W[0] = der[FDCTrackD::dDOCAW_dDeltaX];
          derGl_W[1] = der[FDCTrackD::dDOCAW_dDeltaPhiX];
          derGl_W[2] = der[FDCTrackD::dDOCAW_dDeltaPhiY];
          derGl_W[3] = der[FDCTrackD::dDOCAW_dDeltaPhiZ];
          derGl_W[4] = der[FDCTrackD::dDOCAW_dDeltaZ];
          derGl_W[5] = -der[FDCTrackD::dW_dt0];

          label_W[0] = label_layer_offset + 1;
          label_W[1] = label_layer_offset + 2;
          label_W[2] = label_layer_offset + 3;
          label_W[3] = label_layer_offset + 4;
          label_W[4] = label_layer_offset + 5;
          if (fdc_hit->wire->wire <= 48) {
            label_W[5] = label_layer_offset + 997;
          } else {
            label_W[5] = label_layer_offset + 998;
          }

          milleWriter->mille(NLC, derLc_W, NGL_W, derGl_W, label_W, resi, err);

          // For cathode measurement.
          const int NGL_C = 20;
          float derLc_C[NLC];
          float derGl_C[NGL_C];
          int label_C[NGL_C];

          double Df_Dq_C[5] = {
              der[FDCTrackD::dDOCAC_dx],
              der[FDCTrackD::dDOCAC_dy],
              der[FDCTrackD::dDOCAC_dtx],
              der[FDCTrackD::dDOCAC_dty],
              0.0
          };

          for (int i = 0; i < 3; ++i) {
            derLc_C[i] = 0.0;
            for (int j = 0; j < 5; ++j) derLc_C[i] += Df_Dq_C[j] * DqMinus_Dv[j][i];
          }
          for (int i = 0; i < 5; ++i) {
            derLc_C[i + 3] = 0.0;
            for (int j = 0; j < 5; ++j) derLc_C[i + 3] += Df_Dq_C[j] * DqMinus_Dz[j][i];
          }

          derGl_C[0] = -1.0;
          derGl_C[1] = der[FDCTrackD::dDOCAC_dDeltaX];
          derGl_C[2] = der[FDCTrackD::dDOCAC_dDeltaPhiX];
          derGl_C[3] = der[FDCTrackD::dDOCAC_dDeltaPhiY];
          derGl_C[4] = der[FDCTrackD::dDOCAC_dDeltaPhiZ];
          derGl_C[5] = der[FDCTrackD::dDOCAC_dDeltaZ];

          label_C[0] = label_layer_offset + 100;
          label_C[1] = label_layer_offset + 1;
          label_C[2] = label_layer_offset + 2;
          label_C[3] = label_layer_offset + 3;
          label_C[4] = label_layer_offset + 4;
          label_C[5] = label_layer_offset + 5;

          // Cathode U and V offsets
          derGl_C[6] = -pseudo_der[FDCPseudoD::dSddeltaU];
          derGl_C[7] = -pseudo_der[FDCPseudoD::dSddeltaV];
          derGl_C[8] = -pseudo_der[FDCPseudoD::dSddeltaPhiU];
          derGl_C[9] = -pseudo_der[FDCPseudoD::dSddeltaPhiV];

          label_C[6] = label_layer_offset + 101;
          label_C[7] = label_layer_offset + 102;
          label_C[8] = label_layer_offset + 103;
          label_C[9] = label_layer_offset + 104;

          // Strip Pitch Calibration
          derGl_C[10] = -pseudo_der[FDCPseudoD::dSddeltaU] * strip_der[0];
          derGl_C[11] = -pseudo_der[FDCPseudoD::dSddeltaU] * strip_der[1];
          derGl_C[12] = -pseudo_der[FDCPseudoD::dSddeltaU] * strip_der[2];
          derGl_C[13] = -pseudo_der[FDCPseudoD::dSddeltaU] * strip_der[3];
          derGl_C[14] = -pseudo_der[FDCPseudoD::dSddeltaU] * strip_der[4];
          derGl_C[15] = -pseudo_der[FDCPseudoD::dSddeltaV] * strip_der[5];
          derGl_C[16] = -pseudo_der[FDCPseudoD::dSddeltaV] * strip_der[6];
          derGl_C[17] = -pseudo_der[FDCPseudoD::dSddeltaV] * strip_der[7];
          derGl_C[18] = -pseudo_der[FDCPseudoD::dSddeltaV] * strip_der[8];
          derGl_C[19] = -pseudo_der[FDCPseudoD::dSddeltaV] * strip_der[9];

          label_C[10] = label_layer_offset + 200;
          label_C[11] = label_layer_offset + 201;
          label_C[12] = label_layer_offset + 202;
          label_C[13] = label_layer_offset + 203;
          label_C[14] = label_layer_offset + 204;
          label_C[15] = label_layer_offset + 205;
          label_C[16] = label_layer_offset + 206;
          label_C[17] = label_layer_offset + 207;
          label_C[18] = label_layer_offset + 208;
          label_C[19] = label_layer_offset + 209;

          milleWriter->mille(NLC, derLc_C, NGL_C, derGl_C, label_C, resic, errc);
        }

        if (cdc_hit != nullptr) {
          DCDCWire *wire = const_cast<DCDCWire *>(cdc_hit->wire);

          vector<double> wire_der = wire->GetCDCWireDerivatives();

          const int NLC = 8;
          const int NGL = 10;
          float derLc[NLC];
          float derGl[NGL];
          int label[NGL];

          double Df_Dq[5] = {
              der[CDCTrackD::dDOCAdS0],
              der[CDCTrackD::dDOCAdS1],
              der[CDCTrackD::dDOCAdS2],
              der[CDCTrackD::dDOCAdS3],
              der[CDCTrackD::dDOCAdS4]
          };

          for (int i = 0; i < 3; ++i) {
            derLc[i] = 0.0;
            for (int j = 0; j < 5; ++j) derLc[i] += Df_Dq[j] * DqMinus_Dv[j][i];
          }
          for (int i = 0; i < 5; ++i) {
            derLc[i + 3] = 0.0;
            for (int j = 0; j < 5; ++j) derLc[i + 3] += Df_Dq[j] * DqMinus_Dz[j][i];
          }

          derGl[0] =
              der[CDCTrackD::dDOCAdOriginX] * wire_der[CDCWireD::dOriginXddeltaX] +
              der[CDCTrackD::dDOCAdOriginY] * wire_der[CDCWireD::dOriginYddeltaX] +
              der[CDCTrackD::dDOCAdOriginZ] * wire_der[CDCWireD::dOriginZddeltaX] +
              der[CDCTrackD::dDOCAdDirX] * wire_der[CDCWireD::dDirXddeltaX] +
              der[CDCTrackD::dDOCAdDirY] * wire_der[CDCWireD::dDirYddeltaX] +
              der[CDCTrackD::dDOCAdDirZ] * wire_der[CDCWireD::dDirZddeltaX];

          derGl[1] =
              der[CDCTrackD::dDOCAdOriginX] * wire_der[CDCWireD::dOriginXddeltaY] +
              der[CDCTrackD::dDOCAdOriginY] * wire_der[CDCWireD::dOriginYddeltaY] +
              der[CDCTrackD::dDOCAdOriginZ] * wire_der[CDCWireD::dOriginZddeltaY] +
              der[CDCTrackD::dDOCAdDirX] * wire_der[CDCWireD::dDirXddeltaY] +
              der[CDCTrackD::dDOCAdDirY] * wire_der[CDCWireD::dDirYddeltaY] +
              der[CDCTrackD::dDOCAdDirZ] * wire_der[CDCWireD::dDirZddeltaY];

          derGl[2] =
              der[CDCTrackD::dDOCAdOriginX] * wire_der[CDCWireD::dOriginXddeltaZ] +
              der[CDCTrackD::dDOCAdOriginY] * wire_der[CDCWireD::dOriginYddeltaZ] +
              der[CDCTrackD::dDOCAdOriginZ] * wire_der[CDCWireD::dOriginZddeltaZ] +
              der[CDCTrackD::dDOCAdDirX] * wire_der[CDCWireD::dDirXddeltaZ] +
              der[CDCTrackD::dDOCAdDirY] * wire_der[CDCWireD::dDirYddeltaZ] +
              der[CDCTrackD::dDOCAdDirZ] * wire_der[CDCWireD::dDirZddeltaZ];

          derGl[3] =
              der[CDCTrackD::dDOCAdOriginX] * wire_der[CDCWireD::dOriginXddeltaPhiX] +
              der[CDCTrackD::dDOCAdOriginY] * wire_der[CDCWireD::dOriginYddeltaPhiX] +
              der[CDCTrackD::dDOCAdOriginZ] * wire_der[CDCWireD::dOriginZddeltaPhiX] +
              der[CDCTrackD::dDOCAdDirX] * wire_der[CDCWireD::dDirXddeltaPhiX] +
              der[CDCTrackD::dDOCAdDirY] * wire_der[CDCWireD::dDirYddeltaPhiX] +
              der[CDCTrackD::dDOCAdDirZ] * wire_der[CDCWireD::dDirZddeltaPhiX];

          derGl[4] =
              der[CDCTrackD::dDOCAdOriginX] * wire_der[CDCWireD::dOriginXddeltaPhiY] +
              der[CDCTrackD::dDOCAdOriginY] * wire_der[CDCWireD::dOriginYddeltaPhiY] +
              der[CDCTrackD::dDOCAdOriginZ] * wire_der[CDCWireD::dOriginZddeltaPhiY] +
              der[CDCTrackD::dDOCAdDirX] * wire_der[CDCWireD::dDirXddeltaPhiY] +
              der[CDCTrackD::dDOCAdDirY] * wire_der[CDCWireD::dDirYddeltaPhiY] +
              der[CDCTrackD::dDOCAdDirZ] * wire_der[CDCWireD::dDirZddeltaPhiY];

          derGl[5] =
              der[CDCTrackD::dDOCAdOriginX] * wire_der[CDCWireD::dOriginXddeltaPhiZ] +
              der[CDCTrackD::dDOCAdOriginY] * wire_der[CDCWireD::dOriginYddeltaPhiZ] +
              der[CDCTrackD::dDOCAdOriginZ] * wire_der[CDCWireD::dOriginZddeltaPhiZ] +
              der[CDCTrackD::dDOCAdDirX] * wire_der[CDCWireD::dDirXddeltaPhiZ] +
              der[CDCTrackD::dDOCAdDirY] * wire_der[CDCWireD::dDirYddeltaPhiZ] +
              der[CDCTrackD::dDOCAdDirZ] * wire_der[CDCWireD::dDirZddeltaPhiZ];

          derGl[6] =
              der[CDCTrackD::dDOCAdOriginX] * wire_der[CDCWireD::dOriginXddeltaXu] +
              der[CDCTrackD::dDOCAdOriginY] * wire_der[CDCWireD::dOriginYddeltaXu] +
              der[CDCTrackD::dDOCAdOriginZ] * wire_der[CDCWireD::dOriginZddeltaXu] +
              der[CDCTrackD::dDOCAdDirX] * wire_der[CDCWireD::dDirXddeltaXu] +
              der[CDCTrackD::dDOCAdDirY] * wire_der[CDCWireD::dDirYddeltaXu] +
              der[CDCTrackD::dDOCAdDirZ] * wire_der[CDCWireD::dDirZddeltaXu];

          derGl[7] =
              der[CDCTrackD::dDOCAdOriginX] * wire_der[CDCWireD::dOriginXddeltaYu] +
              der[CDCTrackD::dDOCAdOriginY] * wire_der[CDCWireD::dOriginYddeltaYu] +
              der[CDCTrackD::dDOCAdOriginZ] * wire_der[CDCWireD::dOriginZddeltaYu] +
              der[CDCTrackD::dDOCAdDirX] * wire_der[CDCWireD::dDirXddeltaYu] +
              der[CDCTrackD::dDOCAdDirY] * wire_der[CDCWireD::dDirYddeltaYu] +
              der[CDCTrackD::dDOCAdDirZ] * wire_der[CDCWireD::dDirZddeltaYu];

          derGl[8] =
              der[CDCTrackD::dDOCAdOriginX] * wire_der[CDCWireD::dOriginXddeltaXd] +
              der[CDCTrackD::dDOCAdOriginY] * wire_der[CDCWireD::dOriginYddeltaXd] +
              der[CDCTrackD::dDOCAdOriginZ] * wire_der[CDCWireD::dOriginZddeltaXd] +
              der[CDCTrackD::dDOCAdDirX] * wire_der[CDCWireD::dDirXddeltaXd] +
              der[CDCTrackD::dDOCAdDirY] * wire_der[CDCWireD::dDirYddeltaXd] +
              der[CDCTrackD::dDOCAdDirZ] * wire_der[CDCWireD::dDirZddeltaXd];

          derGl[9] =
              der[CDCTrackD::dDOCAdOriginX] * wire_der[CDCWireD::dOriginXddeltaYd] +
              der[CDCTrackD::dDOCAdOriginY] * wire_der[CDCWireD::dOriginYddeltaYd] +
              der[CDCTrackD::dDOCAdOriginZ] * wire_der[CDCWireD::dOriginZddeltaYd] +
              der[CDCTrackD::dDOCAdDirX] * wire_der[CDCWireD::dDirXddeltaYd] +
              der[CDCTrackD::dDOCAdDirY] * wire_der[CDCWireD::dDirYddeltaYd] +
              der[CDCTrackD::dDOCAdDirZ] * wire_der[CDCWireD::dDirZddeltaYd];

          label[0] = 1;
          label[1] = 2;
          label[2] = 3;
          label[3] = 4;
          label[4] = 5;
          label[5] = 6;
          label[6] = 1001 + (straw_offset[wire->ring] + wire->straw - 1) * 4;
          label[7] = 1002 + (straw_offset[wire->ring] + wire->straw - 1) * 4;
          label[8] = 1003 + (straw_offset[wire->ring] + wire->straw - 1) * 4;
          label[9] = 1004 + (straw_offset[wire->ring] + wire->straw - 1) * 4;

          milleWriter->mille(NLC, derLc, NGL, derGl, label, resi, err);
        }
      }
      milleWriter->end();
      japp->RootUnLock();
    }
  }

  return NOERROR;
}

jerror_t DEventProcessor_MilleKs::erun(void) {
  // This is called whenever the run number changes, before it is
  // changed to give you a chance to clean up before processing
  // events from the next run number.
  return NOERROR;
}

jerror_t DEventProcessor_MilleKs::fini(void) {
  // Called before program exit after event processing is finished.
  delete milleWriter;
  return NOERROR;
}
