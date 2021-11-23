// $Id$
//
//    File: DFactoryGenerator_1p2pi.h

#ifndef _DFactoryGenerator_1p2pi_
#define _DFactoryGenerator_1p2pi_

#include <JANA/jerror.h>
#include <JANA/JFactoryGenerator.h>

#include "DFactory_1p2pi.h"

class DFactoryGenerator_1p2pi : public jana::JFactoryGenerator
{
	public:
		virtual const char* className(void){return static_className();}
		static const char* static_className(void){return "DFactoryGenerator_1p2pi";}
		
		jerror_t GenerateFactories(jana::JEventLoop* locEventLoop)
		{
			locEventLoop->AddFactory(new DFactory_1p2pi());
			return NOERROR;
		}
};

#endif // _DFactoryGenerator_1p2pi_

