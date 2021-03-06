#ifndef		_MOV_DETECTOR_H_
#define		_MOV_DETECTOR_H_

#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "osa_sem.h"
#include "osa_tsk.h"
#include "mvdectInterface.hpp"
#include "postDetector.hpp"

using namespace cv;
using namespace std;


typedef struct _mog2_para_t{
	bool	bShadowDetection;//阴影去除，默认打开
	 int		history;//背景建模历史帧数(背景模型影响帧数)，默认为500,	Learning rate; alpha = 1/defaultHistory2
	 double varThreshold;//模板匹配阈值4.0f*4.0f
	 float	fTau;//阴影阈值0.5f
	 float	varThresholdGen;//新模型匹配阈值3.0f*3.0f;
	 unsigned char nShadowDetection;//前景中模型参数，设置为0表示背景，255表示前景，默认值为127
																		//do shadow detection - insert this value as the detection result - 127 default value

	 float	backgroundRatio;//背景阈值设定:0.9f; backgroundRatio*history; // threshold sum of weights for background test
	 int		nmixtures;///高斯混合模型组件数量5; // maximal number of Gaussians in mixture
	 float	fVarInit;//每个高斯组件的初始方差15.0f; // initial variance for new components
	 float	fVarMin;//4.0f
	 float	fVarMax;//5*defaultVarInit2;

	 float fCT;//CT - complexity reduction prior:0.05f

}MOG2_PARAM;


class CMoveDetector_mv	:	public CMvDectInterface
{
public:
	CMoveDetector_mv();
	virtual	~CMoveDetector_mv();

public:
	int		init(LPNOTIFYFUNC	notifyFunc, void *context);
	int		destroy();
	void	setFrame(cv::Mat	src, 	int chId = 0);

	void	clearWarningRoi(int chId	= 0);//清除警戒区
	void	setWarningRoi(std::vector<cv::Point2i>	warnRoi,	int chId	= 0);
	void	setTrkThred(TRK_THRED		trkThred,	int chId	= 0);
	void	setDrawOSD(cv::Mat	dispOSD, int chId = 0);
	void	setWarnMode(WARN_MODE	warnMode,	int chId	= 0);
	void	enableSelfDraw(bool	bEnable, int chId = 0);
	void   setROIScalXY(float scaleX = 1.0, float scaleY = 1.0, int chId = 0);


	void	getLostTarget(std::vector<TRK_RECT_INFO>	&resTarget,	int chId	= 0);//丢失目标
	void	getInvadeTarget(std::vector<TRK_RECT_INFO>	&resTarget,	int chId	= 0);//入侵目标
	void	getMoveTarget(std::vector<TRK_RECT_INFO>	&resTarget,	int chId	= 0);//移动目标
	void	getBoundTarget(std::vector<TRK_RECT_INFO>	&resTarget,	int chId	= 0);//越界目标
	void	getWarnTarget(std::vector<TRK_RECT_INFO>	&resTarget,	int chId	= 0);//警戒区周边所有目标

public:
	MOG2_PARAM	m_mogParam;
	cv::Mat	frame[DETECTOR_NUM];
	cv::Mat 	fgmask[DETECTOR_NUM];
	cv::Mat	disframe[DETECTOR_NUM];
	CPostDetect_mv		m_postDetect[DETECTOR_NUM];

	OSA_TskHndl m_maskDetectTsk[DETECTOR_NUM];
	BOOL			m_bExit;

	std::vector<cv::Point2i>		m_warnRoiVec[DETECTOR_NUM];

protected:

	std::vector<TRK_RECT_INFO>		m_warnLostTarget[DETECTOR_NUM];//丢失目标检测
	std::vector<TRK_RECT_INFO>		m_warnInvadeTarget[DETECTOR_NUM];//入侵目标检测
	std::vector<TRK_RECT_INFO>		m_movTarget[DETECTOR_NUM];//移动目标检测
	std::vector<TRK_RECT_INFO>		m_edgeTarget[DETECTOR_NUM];//越界目标检测
	std::vector<TRK_RECT_INFO>		m_warnTarget[DETECTOR_NUM];//警戒区周边所有目标

protected:

	unsigned int m_DetectCount;
	static int videoProcess_TskFncMaskDetect(OSA_TskHndl *pTsk, OSA_MsgHndl *pMsg, Uint32 curState )
	{
		((CMoveDetector_mv*)pTsk->appData)->maskDetectProcess(pMsg);
		return 0;
	}
	void maskDetectProcess(OSA_MsgHndl *pMsg);

private:

	Ptr<BackgroundSubtractorMOG2> fgbg[DETECTOR_NUM];
//	Ptr<BackgroundSubtractorGMG>  fgbg[DETECTOR_NUM];
	LPNOTIFYFUNC	m_notifyFunc;
	void	*m_context;
	WARN_MODE		m_warnMode[DETECTOR_NUM];
	bool		m_bSelfDraw[DETECTOR_NUM];
	float		m_scaleX[DETECTOR_NUM];
	float		m_scaleY[DETECTOR_NUM];

};

#endif
