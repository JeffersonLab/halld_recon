// Author: David Lawrence  June 25, 2004
//
//
// MyProcessor.h
//
/// Example program for a Hall-D analyzer which uses DANA
///

#include <JANA/JEventProcessor.h>
#include <JANA/JEvent.h>

#include <TFile.h>
#include <TTree.h>

extern std::vector<std::string> toprint;
extern bool ACTIVATE_ALL;

class MyProcessor:public JEventProcessor
{
	public:
		MyProcessor();
		~MyProcessor();
	
		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;
		void Process(const std::shared_ptr<const JEvent>& event) override;
		void EndRun() override {};
		void Finish() override;

		typedef struct{
			std::string dataClassName;
			std::string tag;
		}factory_info_t;
		std::vector<factory_info_t> fac_info;

		TFile *ROOTfile;
};
