#include "pch.h"

const dword INIT_FUNCTION_HOOK_POS  = 0x1000908E;
const dword INIT_FUNCTION_RETN_POS  = 0x10009096;

dword       GAME_RACE_LOOP_HOOK_POS = 0x000012E0;
dword       GAME_RACE_LOOP_RETN_POS = 0x000012E7;

const char* MR2_DLL_NAME            = "mr2.dll";
dword       MR2_DLL_POS             = NULL;

void handle_race_loop( RaceInfo* race_info ) {
    race_info->field_00 = 0x1D;

    // if status == 0
    // this is loading status
    // if status == 1
    // send sync status
    // and wait for others
    // if status >= 2
    // set player positions according to server
}

_declspec( naked ) void hook_race_loop() {
    _asm push ecx
    _asm call handle_race_loop
    _asm add esp, 4
    _asm push esi
    _asm mov esi, ecx
    _asm push edi
    _asm mov edi, [esi + 0x28]
    _asm jmp GAME_RACE_LOOP_RETN_POS
}

void handle_init_function( const char* DLL_NAME, dword DLL_POS ) {
    if ( strcmp( DLL_NAME, "mr2.dll" ) != 0 ) {
        return;
    }

    MR2_DLL_POS = DLL_POS;
    GAME_RACE_LOOP_HOOK_POS += MR2_DLL_POS;
    GAME_RACE_LOOP_RETN_POS += MR2_DLL_POS;

    dword old_protect;
    VirtualProtect( ( void* )MR2_DLL_POS, 0x3000, PAGE_EXECUTE_READWRITE, &old_protect );

    dword pos = ( dword )&hook_race_loop;
    pos = pos - ( ( dword )GAME_RACE_LOOP_HOOK_POS + 5 );

    *( byte* )( GAME_RACE_LOOP_HOOK_POS ) = 0xE9;
    *( dword* )( GAME_RACE_LOOP_HOOK_POS + 1 ) = pos;

    VirtualProtect( ( void* )MR2_DLL_POS, 0x3000, old_protect, &old_protect );
}

__declspec( naked ) void hook_init_function() {
    _asm push edx
    _asm push eax
    _asm push esi
    _asm call handle_init_function
    _asm pop esi
    _asm pop eax
    _asm pop edx
    _asm cmp eax, ebx
    _asm mov [esp + 0x50], eax
    _asm jmp INIT_FUNCTION_RETN_POS
}

#define INI_FILE_NAME "mr2_multi.ini"

dword       port;
dword       player_id;
std::string config_dir;
SOCKET      conn_socket;
char        player_name[ 32 ];
char        server_address[ 32 ];

dword validate() {
    int   size    = GetCurrentDirectoryA( NULL, NULL );
    char* ini_dir = new char[ size ];

    GetCurrentDirectoryA( size, ini_dir );

    config_dir += ini_dir;
    
    if ( config_dir.back() != '\\' ) {
        config_dir += '\\';
    }

    config_dir += INI_FILE_NAME;

    delete[] ini_dir;
    
    ini_dir = ( char* )config_dir.c_str();

    port = GetPrivateProfileIntA( "Config", "Port", -1, ini_dir );
    GetPrivateProfileStringA( "Config", "Name", "Player", player_name, 32, ini_dir );
    GetPrivateProfileStringA( "Config", "Address", "127.0.0.1", server_address, 32, ini_dir );

    if ( port == -1 ) {
        MessageBoxA( NULL, "Please provide port", "Result", MB_OK );
        return FALSE;
    } else if ( port > 65535 ) {
        MessageBoxA( NULL, "Port cannot exceed 65535", "Result", MB_OK );
        return FALSE;
    } else if ( port < 0 ) {
        MessageBoxA( NULL, "Port cannot be negative", "Result", MB_OK );
        return FALSE;
    }

    WSAData data;
    int result = WSAStartup( MAKEWORD( 2, 2 ), &data );

    if ( result != 0 ) {
        MessageBoxA( NULL, "Error=0", NULL, MB_OK );
        return FALSE;
    }

    conn_socket = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );

    if ( conn_socket == INVALID_SOCKET ) {
        MessageBoxA( NULL, "Error=1", NULL, MB_OK );
        WSACleanup();
        return FALSE;
    }

    sockaddr_in socket_address;
    socket_address.sin_addr.s_addr = inet_addr( server_address );
    socket_address.sin_port        = htons( port );
    socket_address.sin_family      = AF_INET;

    result = connect( conn_socket, ( SOCKADDR* ) &socket_address, sizeof( socket_address ) );

    if ( result == SOCKET_ERROR ) {
        MessageBoxA( NULL, "Error=2", NULL, MB_OK );

        closesocket( conn_socket );
        WSACleanup();
        return FALSE;
    }

    dword name_length = strlen( player_name );
    char buffer[ 128 ];

    buffer[ 0 ] = 0x01;
    buffer[ 1 ] = name_length;
    memcpy( &buffer[ 2 ], player_name, name_length );

    result = send( conn_socket, buffer, name_length + 3, MSG_OOB );

    if ( result == SOCKET_ERROR ) {
        MessageBoxA( NULL, "Error=3", NULL, MB_OK );

        closesocket( conn_socket );
        WSACleanup();
        return FALSE;
    }

    char recvbuf[ 16 ];

    do {
        result = recv( conn_socket, recvbuf, 16, MSG_PEEK );

        if ( result < 1 || recvbuf[ 0 ] != 0x01 ) {
            return FALSE;
        }

        player_id = recvbuf[ 1 ];
        break;
    } while ( result > 0 );

    return TRUE;
}

dword __stdcall DllMain( dword handle, dword reason, void* reserved ) {
    if ( reason != DLL_PROCESS_ATTACH ) {
        return FALSE;
    }

    MessageBoxA( NULL, "Hello", "Hello World", MB_OK );

    if ( validate() == FALSE ) {
        return FALSE;
    }

    dword old_protect;

    VirtualProtect( ( void* )0x10000000, 0x10000, PAGE_EXECUTE_READWRITE, &old_protect );

    dword pos = ( dword )&hook_init_function;
    pos = pos - ( ( dword )INIT_FUNCTION_HOOK_POS + 5 );

    *( byte* )( INIT_FUNCTION_HOOK_POS ) = 0xE9;
    *( dword* )( INIT_FUNCTION_HOOK_POS + 1 ) = pos;

    VirtualProtect( ( void* )0x10000000, 0x10000, old_protect, &old_protect );
    return TRUE;
}