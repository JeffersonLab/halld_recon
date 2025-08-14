// $Id$
//
//    File: JEventProcessor_ecal_waveforms.h
// Created: Fri May 30 04:30:17 PM EDT 2025
// Creator: somov (on Linux ifarm2401.jlab.org 5.14.0-503.19.1.el9_5.x86_64 x86_64)
//

/// For more information on the syntax changes between JANA1 and JANA2, visit: https://jeffersonlab.github.io/JANA2/#/jana1to2/jana1-to-jana2

#ifndef _JEventProcessor_ecal_waveforms_
#define _JEventProcessor_ecal_waveforms_

#include <JANA/JEventProcessor.h>
#include <JANA/Services/JLockService.h> // Required for accessing services

#include <TLorentzVector.h>

#include <DAQ/Df250PulseIntegral.h>
#include <DAQ/Df250PulseData.h>
#include <DAQ/Df250WindowRawData.h>
#include <DAQ/Df250PulsePedestal.h>

#include <ECAL/DECALDigiHit.h>
#include <ECAL/DECALRefDigiHit.h>

#include <TDirectory.h>
#include <RootHeader.h>
#include <TLatex.h>



class JEventProcessor_ecal_waveforms:public JEventProcessor{
    public:
        JEventProcessor_ecal_waveforms();
        ~JEventProcessor_ecal_waveforms();
        const char* className(void){return "JEventProcessor_ecal_waveforms";}

    private:
        void Init() override;                       ///< Called once at program start.
        void BeginRun(const std::shared_ptr<const JEvent>& event) override; ///< Called everytime a new run number is detected.
        void Process(const std::shared_ptr<const JEvent>& event) override;  ///< Called every event.
        void EndRun() override;                     ///< Called everytime run number changes, provided BeginRun has been called.
        void Finish() override;                     ///< Called after last event of last event source has been processed.
        
        std::shared_ptr<JLockService> lockService; //Used to access all the services, its value should be set inside Init()

        TApplication *app1;

        TCanvas *my_canvas;

        TProfile *ecal_wave_ind[40][40], *ecal_ref1, *ecal_ref2;

        int par_col, par_row;

        int par_plot_ref;  
  
};

#endif // _JEventProcessor_ecal_waveforms_

