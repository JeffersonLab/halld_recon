// $Id$
//
//    File: DDIRCLut_factory.h
//

#ifndef _DDIRCLut_factory_
#define _DDIRCLut_factory_

#include <JANA/JFactoryT.h>
#include "DDIRCLut.h"

class DDIRCLut_factory:public JFactoryT<DDIRCLut> {

public:
	
	DDIRCLut_factory(){};
	~DDIRCLut_factory(){};

	DDIRCLut *dirclut=nullptr;

	void BeginRun(const std::shared_ptr<const JEvent>& event) override {
		
		assert( mData.size() == 0 );

		SetFactoryFlag(NOT_OBJECT_OWNER);
		ClearFactoryFlag(WRITE_TO_OUTPUT);

		delete dirclut;
		dirclut = new DDIRCLut();
		dirclut->Init(event);
	}

	void EndRun() override {
		delete dirclut;
		dirclut = nullptr;
	}

	void Process(const std::shared_ptr<const JEvent>& event) override {
		// Reuse existing DDIRCLut object
		if( dirclut ) Insert( dirclut );
	}
};

#endif // _DDIRCLut_factory_

