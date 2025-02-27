// $Id$

#ifndef _DBCALShower_factory_
#define _DBCALShower_factory_

#include <JANA/JFactoryT.h>
#include <JANA/JEvent.h>
#include <BCAL/DBCALShower.h>

class DBCALShower_factory:public JFactoryT<DBCALShower>{
	public:
		DBCALShower_factory(){};
		~DBCALShower_factory(){};

	private:
		void Process(const std::shared_ptr<const JEvent>& event){

			// This is a trivial factory that simply implements the
			// IU tagged factory as the default. It is here so 
			// that the default can be changed easily by simply
			// changing the tag here or on the command line.
			auto showers = event->Get<DBCALShower>("IU");

			for(unsigned int i=0; i<showers.size(); i++){
				Insert(const_cast<DBCALShower*>(showers[i]));
				// TODO: const-cast
			}

			SetFactoryFlag(NOT_OBJECT_OWNER);
		}
};

#endif // _DBCALShower_factory_
