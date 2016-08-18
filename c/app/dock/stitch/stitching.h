#ifndef _STITCH_H
#define _STITCH_H

#ifdef _WIN32
	#define DLL_EXPORT __declspec(dllexport)
#else
	#define DLL_EXPORT
#endif

typedef struct
{
	size_t nCam;		// camera amount
	size_t nCamHeight;	// camera image height
	size_t nCamWidth;	// camera image width
	size_t nCamSize;	// camera image size (height * width)

	size_t nFinalHeight;	// final image height
	size_t nFinalWidth;		// final image width
	size_t nFinalSize;		// final image size (height * width)
}ImageParam;

enum
{
	STITCH_COARSE = 0,   //简略拼接（一般用于预览）
	STITCH_REFINED = 1   //精细拼接（一般用于正式播放）
};

#ifdef __cplusplus
extern "C" {
#endif
DLL_EXPORT const char* GetStitchingVer();
#ifdef __cplusplus
}
#endif

/////////////////////////////////////////////////////////////
// Parameter: char * ParamFolderPath	---- Parameter files path
// Remarks:   initialize handle parameter and buffer
/////////////////////////////////////////////////////////////
#ifdef __cplusplus
extern "C" {
#endif
DLL_EXPORT bool StitchInit(char* ParamFilePath, ImageParam* pIP, unsigned int ccFrequency, unsigned int vcFrequency, unsigned int ccInitialCounter, unsigned int vcInitialCounter, int flags);
#ifdef __cplusplus
}
#endif

/////////////////////////////////////////////////////////////
// Returns:   void
// Remarks:   free buffer
/////////////////////////////////////////////////////////////
#ifdef __cplusplus
extern "C" {
#endif
DLL_EXPORT void StitchUninit();
#ifdef __cplusplus
}
#endif

/////////////////////////////////////////////////////////////
// Parameter: UINT8 * pImg1		---- input image 1 data
// Parameter: UINT8 * pImg2		---- input image 2 data
// Parameter: UINT8 * pImg3		---- input image 3 data
// Parameter: UINT8 * pImg4		---- input image 4 data
// Parameter: UINT8 * pFinal	---- ouput image data
// Returns:   int				---- process result
// Remarks:   stitch specified amount image into 1 full image
/////////////////////////////////////////////////////////////
#ifdef __cplusplus
extern "C" {
#endif
DLL_EXPORT int StitchProcess(unsigned char* pImg1, unsigned char* pImg2, unsigned char* pImg3, unsigned char* pImg4, unsigned char* pFinal);
#ifdef __cplusplus
}
#endif

#endif