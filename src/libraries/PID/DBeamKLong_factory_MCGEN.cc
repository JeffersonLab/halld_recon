#include <iostream>
#include <iomanip>
using namespace std;

#include "DBeamKLong_factory_MCGEN.h"

#include "DANA/DStatusBits.h"
#include "DANA/DEvent.h"

using namespace jana;

DBeamKLong_factory_MCGEN::DBeamKLong_factory_MCGEN(void) { 
	SetTag("MCGEN");
}


//------------------
// BeginRun
//------------------
void DBeamKLong_factory_MCGEN::BeginRun(const std::shared_ptr<const JEvent>& event)
{
	//Setting this flag makes it so that JANA does not delete the objects in _data.  This factory will manage this memory. 
		//This is because some/all of these pointers are just copied from earlier objects, and should not be deleted.  
	bool locIsRESTEvent = event->GetSingleStrict<DStatusBits>()->GetStatusBit(kSTATUS_REST);
	if(!locIsRESTEvent) //If REST, will grab from file: IS owner
		SetFactoryFlag(NOT_OBJECT_OWNER);

}

//------------------
// Process
//------------------
void DBeamKLong_factory_MCGEN::Process(const std::shared_ptr<const JEvent>& event)
{
// 	if(!_data.empty())
// 	{
// 		if(_data[0]->dSystem == SYS_NULL)
// 			delete _data[0]; //CREATED BY THIS FACTORY //else copied from TRUTH factory!
// 		_data.clear();
// 	}

	//Check if MC
	vector<const DMCReaction*> locMCReactions;
	event->Get(locMCReactions);
	if(locMCReactions.empty())
		return; //Not a thrown event


	//Create a beam object from the DMCReaction
	auto *locBeam = new DBeamKLong;
	*(DKinematicData*)locBeam = locMCReactions[0]->beam;
	Insert(locBeam);

}

// //------------------
// // EndRun
// //------------------
// void DBeamKLong_factory_MCGEN::EndRun(void)
// {
// 
// 
// }
// 
// 
// //------------------
// // Finish
// //------------------
// void DBeamKLong_factory_MCGEN::Finish(void)
// {
// 
// 
// }
// 
