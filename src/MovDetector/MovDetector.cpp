#include <stdio.h>
#include <stdlib.h>
#include <opencv2/video/background_segm.hpp>

#include "MovDetector.hpp"

using namespace cv;
using namespace std;

CMoveDetector_mv::CMoveDetector_mv()
{
	int	i;
	m_bExit	 = FALSE;
	for(i=0; i<DETECTOR_NUM; i++){
		m_warnRoiVec[i].clear();
		m_warnLostTarget[i].clear();
		m_warnInvadeTarget[i].clear();
		m_movTarget[i].clear();
		m_edgeTarget[i].clear();
		m_warnTarget[i].clear();
		m_warnMode[i] = WARN_MOVEDETECT_MODE;
		m_bSelfDraw[i] = false;
		m_scaleX[i] = 1.0;
		m_scaleY[i] = 1.0;
	}
	m_notifyFunc = NULL;
	m_context = NULL;
}

CMoveDetector_mv::~CMoveDetector_mv()
{
	destroy();
}

int CMoveDetector::init(LPNOTIFYFUNC	notifyFunc, void *context)
{
	int	i, result = OSA_SOK;
	initModule_video();
	setUseOptimized(true);
	setNumThreads(4);

	for(i=0; i<DETECTOR_NUM; i++)
	{
		fgbg[i] = Algorithm::create<BackgroundSubtractorMOG2>("BackgroundSubtractor.MOG2");
//		fgbg[i] = Algorithm::create<BackgroundSubtractorGMG>("BackgroundSubtractor.GMG");
		if (fgbg[i].empty())
		{
			OSA_printf( "Failed to create BackgroundSubtractor.MOG2 Algorithm." );
			assert(0);
		}
	}
	m_notifyFunc	= notifyFunc;
	m_context = context;
	for(i=0; i<DETECTOR_NUM; i++)
	{
		result |= OSA_tskCreate(&m_maskDetectTsk[i], videoProcess_TskFncMaskDetect, 0, 0, 0, this);
	}
	return	result;
}

int CMoveDetector::destroy()
{
	int	i,	rtn = OSA_SOK;
	m_bExit = TRUE;
	for(i=0; i<DETECTOR_NUM; i++){
		rtn |= OSA_tskDelete(&m_maskDetectTsk[i]);
	}

	for(i=0; i<DETECTOR_NUM; i++)
	{
		m_postDetect[i].DestroyMD();
	}
	for(i=0; i<DETECTOR_NUM; i++){
		m_warnRoiVec[i].clear();
		m_warnLostTarget[i].clear();
		m_warnInvadeTarget[i].clear();
		m_movTarget[i].clear();
		m_edgeTarget[i].clear();
		m_warnTarget[i].clear();
		m_bSelfDraw[i] = false;
		m_scaleX[i] = 1.0;
		m_scaleY[i] = 1.0;
	}
	return rtn;
}

void	CMoveDetector::setFrame(cv::Mat	src, int chId /*= 0*/)
{
	CV_Assert(chId	< DETECTOR_NUM);
	if( !src.empty() ){

		cv::GaussianBlur(src, frame[chId],cv::Size(5,5), 0, 0);
		m_postDetect[chId].InitializedMD(src.cols,	src.rows, src.cols);
		OSA_tskSendMsg(&m_maskDetectTsk[chId], NULL, (Uint16)chId, NULL, 0);
	}
}

//void	CMoveDetector::setDetectRoi(cv::Rect roi, int chId/* = 0*/)
//{
//	m_detectRoi[chId] = roi;
//}


void	CMoveDetector::setWarningRoi(std::vector<cv::Point2i>	warnRoi,	int chId	/*= 0*/)
{
	CV_Assert(chId	< DETECTOR_NUM);
	int	k,	npoint	= warnRoi.size();
	CV_Assert(npoint	> 2);
	if(npoint	> 2){
		m_warnRoiVec[chId].resize(npoint);
		for(k=0; k<npoint;	k++){
			m_warnRoiVec[chId][k]	= cv::Point2i(warnRoi[k]);
		}
		m_postDetect[chId].setWarningRoi(warnRoi);
	}else{
		OSA_printf("%s: warning	roi	point	num=%d < 3\n", __func__,	npoint);
	}
}

void	CMoveDetector::clearWarningRoi(int chId	/*= 0*/)
{
	CV_Assert(chId	< DETECTOR_NUM);
	m_warnRoiVec[chId].clear();
}

void	CMoveDetector::setTrkThred(TRK_THRED	trkThred,	int chId/*	= 0*/)
{
	CV_Assert(chId	< DETECTOR_NUM);
	m_postDetect[chId].setTrkThred(trkThred);
}

void	CMoveDetector::setDrawOSD(cv::Mat	dispOSD, int chId /*= 0*/)
{
	CV_Assert(chId	< DETECTOR_NUM);
	disframe[chId]	= dispOSD;
}

void	CMoveDetector::setWarnMode(WARN_MODE	warnMode,	int chId /*= 0*/)
{
	CV_Assert(chId	< DETECTOR_NUM);
	m_warnMode[chId]	= warnMode;
}

void	CMoveDetector::enableSelfDraw(bool	bEnable, int chId/* = 0*/)
{
	CV_Assert(chId	< DETECTOR_NUM);
	m_bSelfDraw[chId] = bEnable;
}

void   CMoveDetector::setROIScalXY(float scaleX /*= 1.0*/, float scaleY /*= 1.0*/, int chId /*= 0*/)
{
	CV_Assert(chId	< DETECTOR_NUM);
	m_scaleX[chId] = scaleX;
	m_scaleY[chId] = scaleY;
}

static void _copyTarget(std::vector<TRK_RECT_INFO> srcTarget, std::vector<TRK_RECT_INFO> &dstTarget)
{
	dstTarget.clear();
	int	i,	nsize = srcTarget.size();
	if(nsize >0){
		dstTarget.resize(nsize);
		for(i=0; i<nsize; i++){
			dstTarget[i] = srcTarget[i];
		}
	}
}

void	CMoveDetector::getLostTarget(std::vector<TRK_RECT_INFO>	&resTarget,	int chId /*= 0*/)
{
	CV_Assert(chId	<DETECTOR_NUM);
	_copyTarget(m_warnLostTarget[chId], resTarget);
}

void	CMoveDetector::getInvadeTarget(std::vector<TRK_RECT_INFO>	&resTarget,	int chId /*= 0*/)
{
	CV_Assert(chId	<DETECTOR_NUM);
	_copyTarget(m_warnInvadeTarget[chId], resTarget);
}

void	CMoveDetector::getMoveTarget(std::vector<TRK_RECT_INFO>	&resTarget,	int chId /*= 0*/)
{
	CV_Assert(chId	<DETECTOR_NUM);
	_copyTarget(m_movTarget[chId], resTarget);
}

void	CMoveDetector::getBoundTarget(std::vector<TRK_RECT_INFO>	&resTarget,	int chId /*= 0*/)
{
	CV_Assert(chId	<DETECTOR_NUM);
	_copyTarget(m_edgeTarget[chId], resTarget);
}

void	CMoveDetector::getWarnTarget(std::vector<TRK_RECT_INFO>	&resTarget,	int chId	/*= 0*/)
{
	CV_Assert(chId	<DETECTOR_NUM);
		_copyTarget(m_warnTarget[chId], resTarget);
}

void CMoveDetector::maskDetectProcess(OSA_MsgHndl *pMsg)
{
		int chId;
		chId	=	pMsg->cmd ;
		CV_Assert(chId < DETECTOR_NUM);
		if(m_bExit)
			return;

		if(m_warnRoiVec[chId].size() == 0)
		{
			m_movTarget[chId].clear();
			m_warnLostTarget[chId].clear();
			m_warnInvadeTarget[chId].clear();
			m_warnTarget[chId].clear();
			m_edgeTarget[chId].clear();
			if(m_notifyFunc != NULL)
			{
				(*m_notifyFunc)(m_context, chId);
			}
			return;
		}
		bool update_bg_model = true;
		if(!frame[chId].empty())
		{
			(*fgbg[chId])(frame[chId], fgmask[chId], update_bg_model ? -1 : 0);

			if(m_postDetect[chId].GetMoveDetect(fgmask[chId].data, fgmask[chId].cols, fgmask[chId].rows, fgmask[chId].cols))
			{
//				OSA_printf("num=%d\n", m_postDetect[chId].m_patternnum);
/*				m_warnLostTarget[chId].clear();
				m_warnInvadeTarget[chId].clear();
				m_movTarget[chId].clear();
				m_edgeTarget[chId].clear();
				m_warnTarget[chId].clear();
*/
				if(	(m_warnMode[chId] & WARN_MOVEDETECT_MODE)	)	//move target detect
				{
					m_postDetect[chId].MovTargetDetect(m_scaleX[chId],	m_scaleY[chId]);
					m_postDetect[chId].getMoveTarget(m_movTarget[chId]);

					if(m_bSelfDraw[chId] && !disframe[chId].empty() )
					{
						int	npoint	= m_warnRoiVec[chId].size();
						for(int i=0; i<npoint; i++)
						{
							line(disframe[chId], m_warnRoiVec[chId][i], m_warnRoiVec[chId][(i+1)%npoint], cvScalar(0,0,255,255), 4, 8);
						}
						m_postDetect[chId].MovTargetDraw(disframe[chId]);
					}
				}
				else if(	(m_warnMode[chId] & WARN_INVADE_MODE)  || (m_warnMode[chId] & WARN_LOST_MODE)	 || (m_warnMode[chId] & WARN_INVAD_LOST_MODE))//lost or invade detect
				{
					m_postDetect[chId].warnTargetSelect(m_scaleX[chId],	m_scaleY[chId]);
					m_postDetect[chId].SetTargetBGFGTrk();
					m_postDetect[chId].WarnTargetBGFGTrk();
					m_postDetect[chId].TargetBGFGAnalyse();
					m_postDetect[chId].GetBGFGTarget(m_warnLostTarget[chId], m_warnInvadeTarget[chId], m_warnTarget[chId]);

					if(m_bSelfDraw[chId] && !disframe[chId].empty())
					{
						int	npoint	= m_warnRoiVec[chId].size();
						for(int i=0; i<npoint; i++)
						{
							line(disframe[chId], m_warnRoiVec[chId][i], m_warnRoiVec[chId][(i+1)%npoint], cvScalar(0,0,255,255), 4, 8);
						}
						m_postDetect[chId].DrawBGFGTarget(disframe[chId]);
					}
				}
				else if(	(m_warnMode[chId] & WARN_BOUNDARY_MODE)	)//edge target detect
				{
						m_postDetect[chId].edgeTargetDetect(m_scaleX[chId],	m_scaleY[chId]);
						m_postDetect[chId].getEdgeTarget(m_edgeTarget[chId]);

						if(m_bSelfDraw[chId] && !disframe[chId].empty())
						{
							int	npoint	= m_warnRoiVec[chId].size();
							for(int i=0; i<npoint; i++)
							{
								line(disframe[chId], m_warnRoiVec[chId][i], m_warnRoiVec[chId][(i+1)%npoint], cvScalar(0,0,255,255), 4, 8);
							}
							m_postDetect[chId].edgeTargetDraw(disframe[chId]);
						}
				}
				if(m_notifyFunc != NULL)
				{
					(*m_notifyFunc)(m_context, chId);
				}
		}
	}
	
}
