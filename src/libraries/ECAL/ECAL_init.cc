// $Id: FCAL_init.cc 5342 2009-07-09 15:46:12Z davidl $

#include <JANA/JEventLoop.h>
using namespace jana;

#include <ECAL/DECALTruthShower_factory.h>

#include "DECALDigiHit.h"
#include "DECALHit_factory.h"


#include "DECALRefDigiHit.h"

jerror_t ECAL_init(JEventLoop *loop)
{
	/// Create and register ECAL data factories
  	loop->AddFactory(new JFactory<DECALDigiHit>());
  	loop->AddFactory(new JFactory<DECALRefDigiHit>());
	loop->AddFactory(new DECALHit_factory());
	loop->AddFactory(new JFactory<DECALHit>("TRUTH"));
	loop->AddFactory(new DECALTruthShower_factory());

	return NOERROR;
}
