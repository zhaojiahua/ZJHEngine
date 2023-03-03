#pragma once

#if defined(_WIN32)
#include "../../../simple_library/public/simple_library.h"
#define ZLog(format,...) log_log(format,__VA_ARGS__)
#define ZLog_sucess(format,...) log_success(format,__VA_ARGS__)
#define ZLog_error(format,...) log_error(format,__VA_ARGS__)
#define ZLog_warning(format,...) log_warning(format,__VA_ARGS__)

#elif
#define ZLog(format,...) 
#define ZLog_sucess(format,...) 
#define ZLog_error(format,...) 
#define ZLog_warning(format,...) 
#endif