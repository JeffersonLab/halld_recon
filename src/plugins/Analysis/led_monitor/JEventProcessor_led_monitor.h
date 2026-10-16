// $Id$
//
//    File: JEventProcessor_led_monitor.h
// Created: Mon Jun  2 03:58:40 PM EDT 2025
// Creator: somov (on Linux ifarm2401.jlab.org 5.14.0-503.19.1.el9_5.x86_64 x86_64)
//

/// For more information on the syntax changes between JANA1 and JANA2, visit: https://jeffersonlab.github.io/JANA2/#/jana1to2/jana1-to-jana2

#ifndef _JEventProcessor_led_monitor_
#define _JEventProcessor_led_monitor_

#include <JANA/JEventProcessor.h>
#include <JANA/Services/JLockService.h> // Required for accessing services

#include <TLorentzVector.h>

#include <TRIGGER/DL1Trigger.h>

#include <DAQ/Df250PulseIntegral.h>
#include <DAQ/Df250PulseData.h>
#include <DAQ/Df250WindowRawData.h>
#include <DAQ/Df250PulsePedestal.h>

#include <ECAL/DECALDigiHit.h>
#include <ECAL/DECALRefDigiHit.h>

#include <TDirectory.h>
#include <RootHeader.h>
#include <TLatex.h>





class JEventProcessor_led_monitor:public JEventProcessor{
    public:
        JEventProcessor_led_monitor();
        ~JEventProcessor_led_monitor();
        const char* className(void){return "JEventProcessor_led_monitor";}

    private:
        void Init() override;                       ///< Called once at program start.
        void BeginRun(const std::shared_ptr<const JEvent>& event) override; ///< Called everytime a new run number is detected.
        void Process(const std::shared_ptr<const JEvent>& event) override;  ///< Called every event.
        void EndRun() override;                     ///< Called everytime run number changes, provided BeginRun has been called.
        void Finish() override;                     ///< Called after last event of last event source has been processed.

        std::shared_ptr<JLockService> lockService; //Used to access all the services, its value should be set inside Init()

        TH1F  *htrig_bit,  *htrig_bit_fp;
  
        TH1F *htime;
        TProfile *ecal_wave;

        TApplication *app1;

        TCanvas *my_canvas;
        TCanvas *my_canvas1;

        TH1F *ecal_wave_ind[40][40];

        int par_test;

        TLatex*latexMax;
  
};

#endif // _JEventProcessor_led_monitor_

