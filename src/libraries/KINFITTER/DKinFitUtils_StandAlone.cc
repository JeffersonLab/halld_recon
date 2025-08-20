#include "DKinFitUtils_StandAlone.h"

DKinFitUtils_StandAlone::DKinFitUtils_StandAlone(const string& coarseMeshMsgpackFileName, const string& fineMeshMsgpackFileName)
	: DKinFitUtils()
{
	dMagneticFieldMap = new DMagneticFieldMapFineMesh_StandAlone(coarseMeshMsgpackFileName, fineMeshMsgpackFileName);
}

DKinFitUtils_StandAlone::~DKinFitUtils_StandAlone()
{ }

bool DKinFitUtils_StandAlone::Get_IncludeBeamlineInVertexFitFlag() const
{
	return false;
}

TVector3 DKinFitUtils_StandAlone::Get_BField(const TVector3& locPosition) const
{
	if (dMagneticFieldMap == nullptr)
		return TVector3(0.0, 0.0, 0.0);

	double locBx, locBy, locBz;
	dMagneticFieldMap->GetField(locPosition.X(), locPosition.Y(), locPosition.Z(), locBx, locBy, locBz);
	return TVector3(locBx, locBy, locBz);
}

bool DKinFitUtils_StandAlone::Get_IsBFieldNearBeamline() const
{
	if(dMagneticFieldMap == nullptr)
		return false;

	return true;
}
