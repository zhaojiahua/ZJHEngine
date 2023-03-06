#pragma once

#define ANALYSIS_HRESULT(inValue) \
{\
	HRESULT handleResult = inValue;\
	if (FAILED(inValue)) \
	{\
		ZLog_error("error = %i",(int)inValue);\
	}\
	else if (SUCCEEDED(inValue))\
	{\
		ZLog_sucess("success !");\
	}\
}
