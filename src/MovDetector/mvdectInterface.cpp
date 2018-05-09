#include "mvdectInterface.hpp"
#include "MovDetector.hpp"


CMvDectInterface *MvDetector_Create()
{
	CMvDectInterface	*pMvObj = NULL;

	pMvObj = (CMvDectInterface*)new CMoveDetector_mv;
	CV_Assert(pMvObj != NULL);

	return (CMvDectInterface*)pMvObj;
}

void MvDetector_Destory(CMvDectInterface *obj)
{
	CMoveDetector_mv	*pMvObj = (CMoveDetector_mv*)obj;
	if(pMvObj != NULL){
		delete pMvObj;
		obj = NULL;
	}
}
