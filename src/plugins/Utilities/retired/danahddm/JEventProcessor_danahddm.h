// $Id$
//
//    File: JEventProcessor_danahddm.h
// Created: Mon Mar 15 09:08:37 EDT 2010
// Creator: wolin (on Linux stan.jlab.org 2.6.18-164.el5 x86_64)
//

#ifndef _JEventProcessor_danahddm_
#define _JEventProcessor_danahddm_

#include <string>
#include <fstream>
using namespace std;

#include <HDDM/hddm_s.hpp>

#include <JANA/JEventProcessor.h>
#include <JANA/JEvent.h>



class JEventProcessor_danahddm : public JEventProcessor {

   public:

      JEventProcessor_danahddm();
      ~JEventProcessor_danahddm();

      void Init() override;
      void BeginRun(const std::shared_ptr<const JEvent>& event) override;
      void Process(const std::shared_ptr<const JEvent>& event) override;
      void EndRun() override;
      void Finish() override;


   private:

      std::ofstream *file;
      hddm_s::ostream *fout;
      unsigned long Nevents_written;

      bool HDDM_USE_COMPRESSION;
      bool HDDM_USE_INTEGRITY_CHECKS;

      void Add_DTrackTimeBased(const std::shared_ptr<const JEvent>& loop, 
                               hddm_s::ReconViewList::iterator riter);
      
      string DMatrixDSymToString(const DMatrixDSym &mat);
};


#endif // _JEventProcessor_danahddm_
