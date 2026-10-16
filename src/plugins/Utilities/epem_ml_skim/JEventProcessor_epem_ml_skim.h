#ifndef _JEventProcessor_epem_ml_skim_
#define _JEventProcessor_epem_ml_skim_

#include <JANA/JEventProcessor.h>
#include <JANA/JEvent.h>
#include <memory>

// Forward decls keep the header light
class TH1D;

class JEventProcessor_epem_ml_skim : public JEventProcessor {
public:
    JEventProcessor_epem_ml_skim();
    ~JEventProcessor_epem_ml_skim();

    const char* className(void) { return "JEventProcessor_epem_ml_skim"; }

private:
    // JANA2 lifecycle
    void Init() override;
    void BeginRun(const std::shared_ptr<const JEvent>& event) override;
    void Process(const std::shared_ptr<const JEvent>& event) override;
    void EndRun() override;
    void Finish() override;

    // Parameters / counters
    long int events_read = 0;
    long int ee_events_written = 0;
    long int pi_events_written = 0;

    bool epem_evio_bool = true;
    bool pippim_evio_bool = false;

    double min_EoverP_cut = 0.0;
    double ee_threshold_MLP = 0.8;
    double pi_threshold_MLP = 0.4;

    // Histograms
    TH1D* htrack_mom = nullptr;
    TH1D* htrack_mom_ok = nullptr;
    TH1D* htrack_chi2 = nullptr;
    TH1D* htrack_chi2_ok = nullptr;
    TH1D* htrack_theta = nullptr;
    TH1D* htrack_theta_ok = nullptr;
    TH1D* htrack_phi = nullptr;
    TH1D* htrack_phi_ok = nullptr;

    TH1D* hpimem_ML_classifier = nullptr;
    TH1D* hpipep_ML_classifier = nullptr;

    // MLP only for now; PID buckets: 0=ee, 1=pi, 2=noPID
    static constexpr int nModels = 1;   // {"MLP"}
    static constexpr int nPID    = 3;   // {"ee","pi","noPID"}

    TH1D* hJTphi[nModels][nPID] = {{nullptr}};
    TH1D* hinvmass[nModels][nPID] = {{nullptr}};
    TH1D* hFCALElasticity[nModels][nPID] = {{nullptr}};
    TH1D* hq2_varWidth[nModels][nPID] = {{nullptr}};
};

#endif // _JEventProcessor_epem_ml_skim_
