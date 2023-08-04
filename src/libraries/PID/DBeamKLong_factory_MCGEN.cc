#include <iostream>
#include <iomanip>
using namespace std;

#include "DBeamKLong_factory_MCGEN.h"

using namespace jana;

//------------------
// brun
//------------------
jerror_t DBeamKLong_factory_MCGEN::brun(jana::JEventLoop *locEventLoop, int32_t runnumber)
{
	//Setting this flag makes it so that JANA does not delete the objects in _data.  This factory will manage this memory. 
		//This is because some/all of these pointers are just copied from earlier objects, and should not be deleted.  
	bool locIsRESTEvent = locEventLoop->GetJEvent().GetStatusBit(kSTATUS_REST);
	if(!locIsRESTEvent) //If REST, will grab from file: IS owner
		SetFactoryFlag(NOT_OBJECT_OWNER);
	return NOERROR;
}

//------------------
// evnt
//------------------
jerror_t DBeamKLong_factory_MCGEN::evnt(jana::JEventLoop *locEventLoop, uint64_t eventnumber)
{
// 	if(!_data.empty())
// 	{
// 		if(_data[0]->dSystem == SYS_NULL)
// 			delete _data[0]; //CREATED BY THIS FACTORY //else copied from TRUTH factory!
// 		_data.clear();
// 	}

	//Check if MC
	vector<const DMCReaction*> locMCReactions;
	locEventLoop->Get(locMCReactions);
	if(locMCReactions.empty())
		return NOERROR; //Not a thrown event


	//Create a beam object from the DMCReaction
	auto *locBeam = new DBeamKLong;
	*(DKinematicData*)locBeam = locMCReactions[0]->beam;
	_data.push_back(locBeam);

	return NOERROR;
}

//------------------
// fini
//------------------
jerror_t DBeamKLong_factory_MCGEN::fini(void)
{
// 	if(!_data.empty())
// 	{
// 		if(_data[0]->dSystem == SYS_NULL)
// 			delete _data[0]; //CREATED BY THIS FACTORY //else copied from TRUTH factory!
// 		_data.clear();
// 	}

	return NOERROR;
}

