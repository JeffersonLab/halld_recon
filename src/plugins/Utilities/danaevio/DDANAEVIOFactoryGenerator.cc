// Author: Elliott Wolin 19-Mar-2010


#include <JANA/JApplication.h>

#include "DDANAEVIOFactoryGenerator.h"
#include "DDANAEVIO_factory.h"
#include "JEventProcessor_danaevio.h"


void DDANAEVIOFactoryGenerator::GenerateFactories(JFactorySet* fs) {
  fs->Add(new DDANAEVIO_factory());
}



//----------------------------------------------------------------------------


// for initializing plugins
extern "C" {

  void InitPlugin(JApplication *app) {

    // Initialize plugin system
    InitJANAPlugin(app);


    // add DANAEVIO factory
    app->Add(new DDANAEVIOFactoryGenerator());


    // Add DANAEVIO event processor to write out events
    // default for writing is true, the normal case if factory is included
    // to turn off specify -PEVIO::WRITEOUT=0
    bool evioWriteOut =true;
    app->SetDefaultParameter("EVIO:WRITEOUT",evioWriteOut);
    if(evioWriteOut) {
      app->Add(new JEventProcessor_danaevio());
    } else {
      jout << endl << endl << "    *** No EVIO output file will be generated ***" << endl << endl << endl;
    }

  }

} // "extern C"


//----------------------------------------------------------------------------
