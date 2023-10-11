// Author: David Lawrence  June 25, 2004
//
//
// MyProcessor.h
//
/// Example program for a Hall-D analyzer which uses DANA
///

#include <string>

#include <JANA/JEventProcessor.h>
#include <JANA/Compatibility/JLockService.h>

#include <HDDM/hddm_s.hpp>
#include <fstream>


class MyProcessor: public JEventProcessor
{
   public:
      void Init() override;
      void BeginRun(const std::shared_ptr<const JEvent>& event) override {};
      void Process(const std::shared_ptr<const JEvent>& event) override;
      void EndRun() override {};
      void Finish() override;

      std::shared_ptr<JLockService> lockService;

      std::string filename;
      std::ofstream *ofs;
      hddm_s::ostream *fout;
      unsigned long Nevents_written;

   private:
      bool HDDM_USE_COMPRESSION;
      bool HDDM_USE_INTEGRITY_CHECKS;
};
