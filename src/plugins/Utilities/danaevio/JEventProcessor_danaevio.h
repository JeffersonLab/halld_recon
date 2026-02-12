// $Id$
//
//    File: JEventProcessor_danaevio.h
// Created: Mon Mar 15 09:08:37 EDT 2010
// Creator: wolin (on Linux stan.jlab.org 2.6.18-164.el5 x86_64)
//

#ifndef _JEventProcessor_danaevio_
#define _JEventProcessor_danaevio_


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>


#include <JANA/JApplication.h>
#include <JANA/JEventProcessor.h>
#include <JANA/JEvent.h>
#include <JANA/JFactoryT.h>

#include <evioFileChannel.hxx>


using namespace std;
using namespace evio;


//----------------------------------------------------------------------------


class JEventProcessor_danaevio : public JEventProcessor {

 public:

  JEventProcessor_danaevio();
  ~JEventProcessor_danaevio();


 private:
  void BeginRun(const std::shared_ptr<const JEvent>& event) override;
  void Process(const std::shared_ptr<const JEvent>& event) override;
  

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
};


#endif // _JEventProcessor_danaevio_
