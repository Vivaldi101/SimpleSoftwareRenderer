#include "shared.h"
#include "win_local.h"
int Sys_GetMilliseconds() {
	static DWORD s_sys_time_base = timeGetTime();
	return timeGetTime() - s_sys_time_base;
}