// $Id$
//
//    File: DVertex_factory_THROWN.h
// Created: Tue Apr  6 17:01:54 EDT 2010
// Creator: pmatt (on Darwin Amelia.local 9.8.0 i386)
//

#ifndef _DVertex_factory_THROWN_
#define _DVertex_factory_THROWN_

#include <JANA/JFactoryT.h>
#include "DVertex.h"
#include <TRACKING/DMCThrown.h>

class DVertex_factory_THROWN : public JFactoryT<DVertex>{
	public:
	DVertex_factory_THROWN() {
		SetTag("THROWN");
	}
	~DVertex_factory_THROWN() override = default;

	private:
		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;
		void Process(const std::shared_ptr<const JEvent>& event) override;
		void EndRun() override;
		void Finish() override;
};

#endif // _DVertex_factory_THROWN_

