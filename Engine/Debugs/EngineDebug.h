#pragma once

#define ANALYSIS_HRESULT(inValue) \
{\
	HRESULT handleResult = inValue;\
	if (FAILED(handleResult)) \
	{\
		ZLog_error("error = %i",(int)handleResult);\
	}\
	else if (SUCCEEDED(handleResult))\
	{\
		ZLog_sucess("success !");\
	}\
}
