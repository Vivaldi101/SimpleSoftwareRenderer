#include "shared.h"
#include "win_local.h"
int Sys_GetMilliseconds() {
	static DWORD sys_time_base = timeGetTime();
	return timeGetTime() - sys_time_base;
}