// $Id$
//
//    File: JEventProcessor_npp_ana.h
// Created: Mon Sep 29 02:32:05 PM EDT 2025
// Creator: ilarin (on Linux ifarm2401.jlab.org 5.14.0-570.33.2.el9_6.x86_64 x86_64)
//

/// For more information on the syntax changes between JANA1 and JANA2, visit: https://jeffersonlab.github.io/JANA2/#/jana1to2/jana1-to-jana2

#ifndef _JEventProcessor_npp_ana_
#define _JEventProcessor_npp_ana_

#ifdef SKIM_MODE
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#pragma GCC diagnostic ignored "-Wdeprecated"
#include <evio_writer/DEventWriterEVIO.h>
#pragma GCC diagnostic pop
#endif

#include <JANA/JEventProcessor.h>
#include <JANA/Services/JLockService.h> // Required for accessing services

#include "DANA/DEvent.h"
#include <TRIGGER/DL1Trigger.h>
#include <PID/DEventRFBunch.h>
#include <PID/DBeamPhoton.h>
#include <PID/DChargedTrack.h>
#include <PID/DNeutralShower.h>
#include <TOF/DTOFPoint.h>


//  #define MCMODE
#ifdef MCMODE
#include <TRACKING/DMCThrown.h>
#include <PID/DMCReaction.h>
#endif

#include <TH1.h>
#include <TH2.h>
#include <TTree.h>
#include <TDirectory.h>
#include <TDirectoryFile.h>

#include <vector>
#include <cmath>
#include <optional>
#include <functional>
#include <mutex>

#include <TMinuit.h>

using namespace jana;
using namespace std;

static mutex minuit_mutex;

const int mshowers  =   7;
const int mbeam     = 200;
static const double mpi0    =  0.1349766;
static const double clight  = 29.9792458;
static const double SHOWER_TO_SHOWER_DIST = 15.;
static const double SHOWER_TO_TRACK_DIST = 6.;
static const double SHOWER_TO_TOFPOINT_DIST = 12.;
static const double SHOWER_TO_TOFPOINT_TIME = 3.;
static const double PI0_MASS_CUT = 0.025;
static const double ACCEL_FREQ = 0.499;

struct FitParams {double e1, e2, s1, s2, m0;};
thread_local FitParams currentParams;

void chi2func1(int& npar, double* grad, double& fval, double* par, int flag) {
  double ec1 = par[0];
  const FitParams& p = currentParams;
  double ec2 = mpi0*mpi0/p.m0/p.m0 * p.e1*p.e2/ec1;
  double chi2 = (p.e1-ec1)*(p.e1-ec1)/p.s1/p.s1 + (p.e2-ec2)*(p.e2-ec2)/p.s2/p.s2;
  fval = chi2;
}

class JEventProcessor_npp_ana:public JEventProcessor{
    public:
        JEventProcessor_npp_ana();
        ~JEventProcessor_npp_ana();
        const char* className(void){return "JEventProcessor_npp_ana";}

    double caleres(double e, DetectorSystem_t det) {

      double a, b;
      switch(det) {
        case SYS_FCAL:  a = 3.6; b = 6.8; break;
        case SYS_BCAL:  a = 3.8; b = 5.7; break;
        default:  cerr << "caleres error" << endl;
                  throw JException("Unknown detector in caleres()");
      }
      double sig = sqrt(a*a + b*b/e);
      sig *= e*1.e-2;
      return  sig;
    }

    void fitm01(double e1, double e2, double s1, double s2, double m0,
               double& ec1, double& ec2, int &status) {

      unique_lock<mutex> lock(minuit_mutex); // lock acquired
      currentParams = {e1, e2, s1, s2, m0};

      minuit->SetFCN(chi2func1);
      minuit->mncler(); // clear previous minimization state
      minuit->DefineParameter(0, "ec1", e1, 1e-4*e1, 0.4*e1, 2.5*e1);

      status = minuit->Migrad();
      double err;
      minuit->GetParameter(0, ec1, err);
      ec2 = mpi0*mpi0/m0/m0 * e1*e2/ec1;
    }

    bool run_is_empty(int run) {
      const int mrun = 185;
      const int runlist[mrun] = { \
      100652, 100653, 100654, 100826, 100827, 100829, 100830, 100831, 100832, 100833, 100834, 100835, 100836, 100837, 100841,
      100842, 101045, 101046, 101047, 101048, 101049, 101050, 101051, 101052, 101053, 101054, 101055, 101056, 101057, 101058,
      101060, 101061, 101062, 101063, 101064, 101065, 101066, 101067, 101068, 101069, 101070, 101071, 101072, 101073, 101167,
      101168, 101170, 101171, 101172, 101182, 101183, 101184, 101185, 101186, 101187, 101189, 101190, 101191, 101192, 101193,
      101194, 101195, 101196, 101197, 101198, 101199, 101200, 101201, 101202, 101203, 101204, 101205, 101206, 101207, 101208,
      101209, 101210, 101211, 101212, 101213, 101214, 101215, 101216, 101217, 101218, 101219, 101220, 101221, 101222, 101223,
      101224, 101225, 101226, 101227, 101228, 101229, 101230, 101231, 101232, 101234, 101235, 101236, 101237, 101238, 101239,
      101240, 101241, 101242, 101243, 101244, 101245, 101246, 101247, 101248, 101249, 101250, 101251, 101253, 101254, 101393,
      101394, 101395, 101396, 101397, 101398, 101399, 101400, 101401, 101402, 101403, 101404, 101405, 101406, 101407, 101408,
      101409, 101410, 101411, 101412, 101413, 101414, 101415, 101416, 101417, 101418, 101420, 101421, 101422, 101423, 101424,
      101425, 101426, 101427, 101428, 101429, 101430, 101431, 101432, 101433, 101434, 101436, 101437, 101438, 101597, 101598,
      101600, 101601, 101602, 101603, 101604, 101605, 101606, 101607, 101608, 101609, 101610, 101611, 101615, 101616, 101617,
      101618, 101619, 101620, 101621, 101622 };

      for(int i = 0; i<mrun; ++i) if(run==runlist[i]) return true;

      return false;
    }

    bool run_is_lead(int run) {
      const int mrun = 604;
      const int runlist[mrun] = { \
      100531, 100532, 100533, 100534, 100535, 100536, 100539, 100540, 100541, 100542, 100543, 100545, 100546, 100547, 100548,
      100549, 100550, 100551, 100552, 100553, 100554, 100555, 100556, 100557, 100558, 100559, 100569, 100570, 100571, 100572,
      100573, 100574, 100575, 100576, 100581, 100582, 100583, 100584, 100585, 100586, 100660, 100661, 100668, 100669, 100670,
      100671, 100672, 100674, 100675, 100715, 100716, 100717, 100718, 100719, 100720, 100721, 100722, 100723, 100725, 100726,
      100731, 100732, 100733, 100734, 100735, 100736, 100738, 100739, 100740, 100741, 100742, 100743, 100744, 100745, 100746,
      100747, 100748, 100760, 100761, 100762, 100763, 100764, 100765, 100766, 100767, 100768, 100770, 100771, 100772, 100773,
      100774, 100775, 100776, 100777, 100778, 100779, 100782, 100783, 100784, 100785, 100786, 100787, 100788, 100789, 100790,
      100791, 100792, 100793, 100794, 100795, 100797, 100798, 100799, 100817, 100818, 100819, 100820, 100821, 100822, 100823,
      100824, 100825, 100843, 100844, 100845, 100846, 100847, 100861, 100862, 100863, 100864, 100865, 100867, 100868, 100869,
      100870, 100871, 100872, 100873, 100874, 100875, 100876, 100877, 100878, 100879, 100880, 100881, 100882, 100883, 100884,
      100898, 100899, 100900, 100901, 100902, 100903, 100904, 100907, 100908, 100909, 100910, 100911, 100912, 100913, 100914,
      100915, 100944, 100947, 100948, 100949, 100950, 100951, 100952, 100953, 100954, 100955, 100956, 100957, 100958, 100959,
      100961, 100962, 100963, 100964, 100965, 100966, 100967, 100968, 100969, 100970, 100971, 100972, 100973, 100974, 100975,
      100976, 100977, 100978, 100979, 100980, 100981, 100982, 100983, 100984, 100985, 100986, 100987, 100988, 100989, 100990,
      100991, 100993, 100994, 100995, 100996, 100997, 100998, 100999, 101000, 101001, 101002, 101003, 101004, 101005, 101006,
      101007, 101008, 101009, 101010, 101011, 101012, 101013, 101014, 101015, 101016, 101018, 101019, 101020, 101021, 101022,
      101023, 101024, 101025, 101026, 101027, 101028, 101029, 101030, 101031, 101032, 101033, 101034, 101035, 101036, 101037,
      101038, 101039, 101040, 101041, 101042, 101043, 101044, 101076, 101077, 101078, 101079, 101080, 101081, 101082, 101083,
      101084, 101085, 101086, 101087, 101088, 101089, 101090, 101093, 101094, 101095, 101096, 101097, 101098, 101099, 101100,
      101102, 101103, 101104, 101105, 101106, 101107, 101108, 101109, 101110, 101111, 101112, 101113, 101114, 101115, 101116,
      101117, 101118, 101119, 101120, 101121, 101122, 101123, 101124, 101125, 101126, 101127, 101128, 101130, 101137, 101142,
      101143, 101145, 101146, 101147, 101148, 101149, 101150, 101151, 101152, 101153, 101154, 101155, 101156, 101157, 101164,
      101165, 101166, 101255, 101256, 101257, 101258, 101259, 101260, 101261, 101262, 101263, 101264, 101265, 101266, 101267,
      101268, 101269, 101270, 101271, 101272, 101273, 101274, 101275, 101276, 101277, 101278, 101279, 101280, 101281, 101282,
      101283, 101284, 101285, 101286, 101287, 101288, 101289, 101290, 101291, 101292, 101293, 101294, 101295, 101296, 101297,
      101298, 101299, 101300, 101301, 101302, 101303, 101304, 101305, 101306, 101307, 101308, 101309, 101311, 101312, 101313,
      101314, 101315, 101316, 101317, 101318, 101319, 101320, 101321, 101322, 101323, 101324, 101325, 101326, 101327, 101328,
      101329, 101330, 101331, 101332, 101333, 101334, 101335, 101336, 101337, 101338, 101340, 101341, 101342, 101343, 101344,
      101345, 101346, 101347, 101348, 101349, 101350, 101351, 101352, 101353, 101354, 101355, 101357, 101358, 101359, 101360,
      101361, 101362, 101363, 101364, 101365, 101366, 101367, 101368, 101369, 101370, 101371, 101372, 101373, 101374, 101376,
      101377, 101379, 101380, 101381, 101382, 101383, 101384, 101385, 101387, 101388, 101389, 101390, 101391, 101392, 101439,
      101440, 101441, 101442, 101444, 101445, 101446, 101447, 101448, 101449, 101450, 101451, 101452, 101453, 101454, 101455,
      101456, 101457, 101458, 101459, 101460, 101462, 101463, 101464, 101466, 101467, 101468, 101469, 101470, 101471, 101472,
      101473, 101474, 101475, 101476, 101477, 101478, 101479, 101480, 101481, 101482, 101483, 101484, 101485, 101486, 101487,
      101488, 101489, 101490, 101491, 101492, 101493, 101494, 101495, 101496, 101497, 101498, 101499, 101500, 101501, 101502,
      101503, 101504, 101505, 101506, 101507, 101508, 101509, 101510, 101511, 101512, 101513, 101514, 101515, 101516, 101517,
      101518, 101519, 101520, 101521, 101522, 101523, 101524, 101525, 101526, 101527, 101528, 101529, 101530, 101531, 101532,
      101533, 101534, 101535, 101536, 101537, 101538, 101539, 101540, 101541, 101542, 101547, 101548, 101549, 101550, 101551,
      101552, 101553, 101554, 101555, 101557, 101558, 101559, 101560, 101561, 101562, 101563, 101564, 101565, 101566, 101567,
      101568, 101569, 101570, 101571, 101572, 101573, 101574, 101575, 101576, 101578, 101579, 101580, 101581, 101582, 101583,
      101584, 101585, 101586, 101587 };

      for(int i = 0; i<mrun; ++i) if(run==runlist[i]) return true;

      return false;
    }

    private:
        void Init() override;                       ///< Called once at program start.
        void BeginRun(const std::shared_ptr<const JEvent>& event) override; ///< Called everytime a new run number is detected.
        void Process(const std::shared_ptr<const JEvent>& event) override;  ///< Called every event.
        void EndRun() override;                     ///< Called everytime run number changes, provided BeginRun has been called.
        void Finish() override;                     ///< Called after last event of last event source has been processed.

        TMinuit* minuit = nullptr;  // Persistent Minuit instance

        std::shared_ptr<JLockService> lockService; //Used to access all the services, its value should be set inside Init()

        TH1D        *h_u, *h_trk_dt, *h_de[2], *h_de_acc[2];
        TTree       *tree;
        TDirectory  *dir;

        Int_t   nt_run, nt_event, nt_tgt, nt_nchtrk, nt_nfcal, nt_nbcal, nt_nneu, nt_cbeam,
                nt_det1, nt_det2, nt_det3, nt_det4, nt_ovlp1, nt_ovlp2, nt_ovlp3, nt_ovlp4,
                nt_nbpair, nt_rankpair;

        Float_t nt_rftime, nt_rftime2, nt_eneu, nt_eneu5, nt_tbeam, nt_ebeam,
                nt_eg1, nt_eg2, nt_eg3, nt_eg4, nt_epi01, nt_epi02, nt_dmpi01, nt_dmpi02, nt_tpi01, nt_tpi02,
                nt_epair, nt_ecpair, nt_mpair, nt_mcpair, nt_tcpair, nt_ccpair, nt_ebpair, nt_efpair, nt_thpair, nt_phipair;

        Bool_t  nt_mbeam;

        DVector3 target_position;
        vector <int> block_to_square;
        int runtype;

        struct ShowerInfo {
          int center_id;
          int dim;
          int ovlp;
          DetectorSystem_t det;
          DVector3 pos;
          double e;
          double t;
        };

// Define the vector type for convenience
        using ShowerInfoVector = std::vector<ShowerInfo>;

        void checkOverlaps(std::vector<ShowerInfo>& si, double threshold) {
          for(size_t i = 0; i < si.size(); ++i) {
            if(si[i].det != SYS_FCAL) continue;
            for(size_t j = i+1; j < si.size(); ++j) {
              if(si[j].det != SYS_FCAL) continue;
              if((si[i].pos-si[j].pos).Pt()<threshold) {si[i].ovlp++; si[j].ovlp++;}
            }
          }
        }

        DVector3 ShowerMomentum(const ShowerInfo& s) {
          return (s.pos - target_position).Unit() * s.e;
        }

        double TwoShowersInvariantMass(const ShowerInfo& s1, const ShowerInfo& s2) {
          DVector3 n1 = (s1.pos - target_position).Unit();
          DVector3 n2 = (s2.pos - target_position).Unit();

          DLorentzVector n41(n1,1.);
          DLorentzVector n42(n2,1.);
          return sqrt(s1.e*s2.e)*(n41+n42).M();
        }

        struct ShowerPair {
          size_t i;  // index of first ShowerInfo object
          size_t j;  // index of second ShowerInfo
          DVector3 momentum;
          DVector3 momentumc;
          double energy;
          double energyc;
          double mass;
          double time;
          double chi2;
          double ebcal;
          double efcal;
          int    nbcal;
        };

        struct Pi0Pair {
          size_t i;  // index of first ShowerPair object
          size_t j;  // index of second ShowerPair
          DVector3 momentum;
          DVector3 momentumc;
          double energy;
          double energyc;
          double mass;
          double massc;
          double time;
          double chi2;
          int    nbcal;
          double ebcal;
          double efcal;
          int duplication_rank = 0;  // 0 = unique; >0 = rank inside duplicate group
        };

        struct BeamPi0Pair {
          size_t pi0_index;     // index of Pi0Pair in Pi0Pairs
          double chi2;          // Pi0Pair chi2 (for sorting/quality)
          double de;
          double ebeam;
          double tbeam;
          double cbeam;         // beam counter
          bool   mbeam;         // if beam counter in microscope
        };

};

#endif // _JEventProcessor_npp_ana_
