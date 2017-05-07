//#include "common.h"

Debug_FreeFile(DebugFreeFile) {
	if (fi->data) {
		Sys_Print("\nClosing file\n");
		VirtualFree(fi->data, 0, MEM_RELEASE);
		fi->size = 0;
	} else {
		Sys_Print("\nAttempting to free null file\n");
	}
}

Debug_ReadFile(DebugReadFile) {
	FileInfo fi = {};
	HANDLE fh = CreateFile(file_name, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);

	if (fh != INVALID_HANDLE_VALUE) {
		LARGE_INTEGER file_size;
		if (GetFileSizeEx(fh, &file_size)) {
			u32 file_size_32 = truncateI64(file_size.QuadPart);
			fi.data = VirtualAlloc(0, file_size_32, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
			if (fi.data) {
				DWORD bytes_read = 0;
				if (ReadFile(fh, fi.data, file_size_32, &bytes_read, 0) &&
					(file_size_32 == bytes_read)) {
					Sys_Print("\nOpening file\n");
					fi.size = bytes_read;
				} else {
					DebugFreeFile(&fi);
					Sys_Print("\nCouldn't read the file\n");
				}
			} else {
				Sys_Print("\nCouldn't allocate memory for the file\n");
			}
		} else {
			Sys_Print("\nCouldn't get size of the file\n");
		}
		CloseHandle(fh);
	} else {
		Sys_Print("\nInvalid file handle\n");
	}
	return fi;
}

Debug_WriteFile(DebugWriteFile) {
	b32 success = false;
	HANDLE fh	= CreateFile(file_name, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
	if (fh != INVALID_HANDLE_VALUE) {
		DWORD bytes_written;
		if (WriteFile(fh, memory, memory_size, &bytes_written, 0)) {
			Sys_Print("\nWriting file\n");
			success = (memory_size == bytes_written);
		} else {
			Sys_Print("\nCouldn't write the file\n");
		}
		CloseHandle(fh);
	} else {
		Sys_Print("\nInvalid file handle\n");
	}
	return success;
}
