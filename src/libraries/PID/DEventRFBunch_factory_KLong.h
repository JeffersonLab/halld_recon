// $Id$
//
//    File: DEventRFBunch_factory_KLong.h
//

#ifndef _DEventRFBunch_factory_KLong_
#define _DEventRFBunch_factory_KLong_

#include <iostream>
#include <iomanip>

#include <JANA/JFactoryT.h>
#include <PID/DEventRFBunch.h>
#include <PID/DBeamPhoton.h>
#include <RF/DRFTime.h>
#include <PID/DVertex.h>

using namespace std;
using namespace jana;

class DEventRFBunch_factory_KLong : public JFactoryT<DEventRFBunch>
{
	public:
		DEventRFBunch_factory_KLong();
		//~DEventRFBunch_factory_KLong() {}

	private:
// 		void Init() override;
// 		void BeginRun(const std::shared_ptr<const JEvent>& event) override; 
		void Process(const std::shared_ptr<const JEvent>& event) override; 
// 		void EndRun() override; 
// 		void Finish() override; 
};

#endif // _DEventRFBunch_factory_KLong_

