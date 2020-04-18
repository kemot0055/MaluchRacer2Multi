#include "pch.h"

dword __stdcall DllMain( dword handle, dword reason, void* reserved ) {
    if ( reason != DLL_PROCESS_ATTACH ) {
        return FALSE;
    }

    //VirtualProtect(;

    MessageBoxA( NULL, "Hello", "Hello World", MB_OK );

    return TRUE;
}