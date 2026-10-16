// $Id$
//
//    File: DEventProcessor_mcthrown_tree.h
// Created: Mon Apr  3 11:38:03 EDT 2006
// Creator: pmatt (on Darwin swire-b241.jlab.org 8.4.0 powerpc)
//

#ifndef _DEventProcessor_mcthrown_tree_
#define _DEventProcessor_mcthrown_tree_

#include "JANA/JEventProcessor.h"
#include <ANALYSIS/DEventWriterROOT.h>


class DEventProcessor_mcthrown_tree : public JEventProcessor
{
	public:
		DEventProcessor_mcthrown_tree(){
			SetTypeName("DEventProcessor_mcthrown_tree");
		};
		~DEventProcessor_mcthrown_tree(){};

	private:
		void Init() override;
		void Process(const std::shared_ptr<const JEvent>& event) override;
		void Finish() override;

		bool dTagCheck;
		int numgoodevents;
};

#endif // _DEventProcessor_mcthrown_tree_

