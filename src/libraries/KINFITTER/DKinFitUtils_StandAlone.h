#ifndef _DKinFitUtils_StandAlone_
#define _DKinFitUtils_StandAlone_

#include "DKinFitUtils.h"
#include "DMagneticFieldMapFineMesh_StandAlone.h"

using namespace std;

class DKinFitUtils_StandAlone : public DKinFitUtils
{
	public:

		DKinFitUtils_StandAlone(const string& coarseMeshMsgpackFileName, const string& fineMeshMsgpackFileName);
		virtual ~DKinFitUtils_StandAlone();

	protected:

		/******************************************************* OVERRIDE BASE CLASS FUNCTIONS ******************************************************/

		bool Get_IncludeBeamlineInVertexFitFlag() const override;
		TVector3 Get_BField(const TVector3& locPosition) const override;  // must return in units of Tesla!!
		bool Get_IsBFieldNearBeamline() const override;

	private:
		//PRIVATE DEFAULT CONSTRUCTOR
		DKinFitUtils_StandAlone() {}  // ensure that object is constructed with file names of magnetic field-maps as argument

		const DMagneticFieldMapFineMesh_StandAlone* dMagneticFieldMap;
};

#endif // _DKinFitUtils_StandAlone_
