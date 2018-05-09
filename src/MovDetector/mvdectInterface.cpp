#include "mvdectInterface.hpp"
#include "MovDetector.hpp"


CMvDectInterface *MvDetector_Create()
{
	CMvDectInterface	*pMvObj = NULL;

	pMvObj = (CMvDectInterface*)new CMoveDetector;
	CV_Assert(pMvObj != NULL);

	return (CMvDectInterface*)pMvObj;
}

void MvDetector_Destory(CMvDectInterface *obj)
{
	CMoveDetector	*pMvObj = (CMoveDetector*)obj;
	if(pMvObj != NULL){
		delete pMvObj;
		obj = NULL;
	}
}
