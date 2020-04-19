#include "pch.h"

const dword INIT_FUNCTION_HOOK_POS = 0x1000908E;
const dword INIT_FUNCTION_RETN_POS = 0x10009096;

dword MR2_DLL_POS = NULL;

__declspec( naked ) void hook_init_function() {
    _asm mov [ MR2_DLL_POS ], eax
    _asm cmp eax, ebx
    _asm mov [esp + 0x50], eax
    _asm jmp INIT_FUNCTION_RETN_POS
}

#define INI_FILE_NAME "mr2_multi.ini"

int         port = -1;
char        name[ 32 ];
char        address[ 32 ];
std::string dir;

#pragma comment(lib, "Ws2_32.lib")
void testuj() {
    int   size    = GetCurrentDirectoryA( NULL, NULL );
    char* ini_dir = new char[ size ];

    GetCurrentDirectoryA( size, ini_dir );

    dir += ini_dir;
    
    if ( dir.back() != '\\' ) {
        dir += '\\';
    }

    dir += INI_FILE_NAME;

    delete[] ini_dir;
    
    ini_dir = ( char* )dir.c_str();

    port = GetPrivateProfileIntA( "Config", "Port", -1, ini_dir );
    GetPrivateProfileStringA( "Config", "Name", "Player", name, 32, ini_dir );
    GetPrivateProfileStringA( "Config", "Address", "127.0.0.1", address, 32, ini_dir );

    if ( port == -1 ) {
        MessageBoxA( NULL, "Please provide port", "Result", MB_OK );
    } else if ( port > 65535 ) {
        MessageBoxA( NULL, "Port cannot exceed 65535", "Result", MB_OK );
    } else if ( port < 0 ) {
        MessageBoxA( NULL, "Port cannot be negative", "Result", MB_OK );
    }

    MessageBoxA( NULL, name, "Result", MB_OK );
    MessageBoxA( NULL, address, "Result", MB_OK );

    WSAData data;
    int result = WSAStartup( MAKEWORD( 2, 2 ), &data );

    if ( result != 0 ) {
        MessageBoxA( NULL, "Error=0", NULL, MB_OK );
        return;
    }

    SOCKET s = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );

    if ( s == INVALID_SOCKET ) {
        MessageBoxA( NULL, "Error=1", NULL, MB_OK );
        WSACleanup();
        return;
    }

    sockaddr_in sa;
    sa.sin_family      = AF_INET;
    sa.sin_addr.s_addr = inet_addr( address );
    sa.sin_port        = htons( port );

    result = connect( s, ( SOCKADDR* ) &sa, sizeof( sa ) );

    if ( result == SOCKET_ERROR ) {
        closesocket( s );
        WSACleanup();

        MessageBoxA( NULL, "Error=2", NULL, MB_OK );
        return;
    }

    //const char testowe[] = { 0x21, 0x37, 0x21, 0x37 };

    //result = send( s, testowe, 5, MSG_OOB );

    /*if ( result == SOCKET_ERROR ) {
        MessageBoxA( NULL, "Chujnia panie", "4", MB_OK );

        result = closesocket( s );

        if ( result == SOCKET_ERROR ) {
            MessageBoxA( NULL, "Chujnia panie", "5", MB_OK );
        }

        WSACleanup();
        return;
    }*/

    //char recvbuf[ 16 ];

    //do {
    //    result = recv( s, recvbuf, 16, 0 );

    //    if ( result > 0 && strcmp( "gib_data_plz", recvbuf ) == 0 ) {
    //        MessageBoxA( NULL, recvbuf, "6", MB_OK );
    //    }
    //} while ( result > 0 );

    closesocket( s );
    WSACleanup();
}

dword __stdcall DllMain( dword handle, dword reason, void* reserved ) {
    if ( reason != DLL_PROCESS_ATTACH ) {
        return FALSE;
    }

    MessageBoxA( NULL, "Hello", "Hello World", MB_OK );

    testuj();

    dword old_protect = NULL;

    VirtualProtect( ( void* )0x10000000, 0x10000, PAGE_EXECUTE_READWRITE, &old_protect );

    dword pos = ( dword )&hook_init_function;
    pos = pos - ( ( dword )INIT_FUNCTION_HOOK_POS + 5 );

    *( byte* )( INIT_FUNCTION_HOOK_POS ) = 0xE9;
    *( dword* )( INIT_FUNCTION_HOOK_POS + 1 ) = pos;

    VirtualProtect( ( void* )0x10000000, 0x10000, old_protect, &old_protect );
    return TRUE;
}