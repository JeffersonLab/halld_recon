// $Id$
//
//    File: DCppPid_factory.h
// Created: Thu Mar  5 02:05:32 PM EST 2026
// Creator: staylor (on Linux ifarm2401.jlab.org 5.14.0-611.30.1.el9_7.x86_64 x86_64)
//

/// For more information on the syntax changes between JANA1 and JANA2, visit: https://jeffersonlab.github.io/JANA2/#/jana1to2/jana1-to-jana2


#ifndef _DCppPid_factory_
#define _DCppPid_factory_

#include <JANA/JFactoryT.h>
#include "DCppPid.h"

class DCppPid_factory:public JFactoryT<DCppPid>{
	public:
		DCppPid_factory(){
			SetTag("");
		}
		~DCppPid_factory(){}


	private:
		void Init() override;                       ///< Called once at program start.
		void BeginRun(const std::shared_ptr<const JEvent>& event) override; ///< Called everytime a new run number is detected.
		void Process(const std::shared_ptr<const JEvent>& event) override;  ///< Called every event.
		void EndRun() override;                     ///< Called everytime run number changes, provided BeginRun has been called.
		void Finish() override;                     ///< Called after last event of last event source has been processed.

};

#endif // _DCppPid_factory_

