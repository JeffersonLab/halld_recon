#ifndef _DBCALSiPMSpectrum_
#define _DBCALSiPMSpectrum_

#include <JANA/JObject.h>

#include <DHistogram.h>
#include <BCAL/DBCALGeometry.h>

class DBCALSiPMSpectrum: public JObject {

	/// This class holds the signal at the BCAL SiPM as provided by GEANT
	/// (before summing and electronic response).

	public:
		JOBJECT_PUBLIC(DBCALSiPMSpectrum);

		DBCALSiPMSpectrum() : spectrum(4000, -100.0, 300.0) {};

		int module;
		int layer; //This is SiPM layer (ranges from 1-10), not ADC layer (1-4)
		int sector;
		DBCALGeometry::End end;

		DHistogram spectrum;

		int incident_id;

		void Summarize(JObjectSummary& summary) const override {
			summary.add(module, "module", "%d");
			summary.add(layer, "layer", "%d");
			summary.add(sector, "sector", "%d");
			summary.add(end==0 ? "upstream":"downstream" , "end", "%s");
			summary.add(incident_id, "incident_id", "%d");
		}
};

#endif // _DBCALSiPMSpectrum_
