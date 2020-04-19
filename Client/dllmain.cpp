#include "pch.h"

const dword   INIT_FUNCTION_HOOK_POS  = 0x1000908E;
const dword   INIT_FUNCTION_RETN_POS  = 0x10009096;

const dword   GAME_AI_DISABLE         = 0x00004562;
const dword   GAME_VEHICLES_ARRAY     = 0x0016C1B4;
dword         GAME_RACE_LOOP_HOOK_POS = 0x000012E0;
dword         GAME_RACE_LOOP_RETN_POS = 0x000012E7;

const char*   MR2_DLL_NAME            = "mr2.dll";
dword         MR2_DLL_POS             = NULL;
dword         synchronized            = FALSE;
VehicleInfo** vehicles                = NULL;

dword       port;
dword       player_id;
std::string config_dir;
SOCKET      conn_socket;
char        player_name[ 32 ];
char        server_address[ 32 ];

dword disconnected = FALSE;

void handle_race_loop( RaceInfo* race_info ) {
    if ( disconnected == TRUE ) {
        return;
    }

    if ( race_info->status == 0 ) {
        synchronized = FALSE;
    } else if ( race_info->status == 1 && synchronized == FALSE ) {
        // send sync status, and wait for others
        dword sync_status = 0x00000002;

        int result = send( conn_socket, ( const char* )&sync_status, 2, MSG_OOB );

        if ( result != 2 ) {
            MessageBoxA( NULL, "result != 2", NULL, MB_OK );
        }

        do {
            result = recv( conn_socket, ( char* )&sync_status, 4, NULL );

            if ( result == 3 ) {
                break;
            } else if ( result == 0 ) {
                break;
            } else if ( result < 0 ) {
                int error = WSAGetLastError();

                if ( error == 10035 ) {
                    continue;
                }

                char buf[16]; _itoa_s( error, buf, 10 );
                MessageBoxA( NULL, "no cusz, rozlaczyl sie", buf, MB_OK );
                disconnected = TRUE;
                return;
            }
        } while( result >= 0 );

        
        //
        //char buf[ 128 ];

        //if ( result == 0 ) {
        //    return;
        ////} else if ( result < 0 ) {
        //    MessageBoxA( NULL, "recv zjebal", NULL, MB_OK );
        //}

        synchronized = TRUE;

        for ( int i = 1; i < 4; i++ ) {
            vehicles[ i ]->acceleration_strength = 0;
            vehicles[ i ]->handling_strength = 0;
            vehicles[ i ]->steering_strength = 0;
        }
    } else if ( race_info->status >= 2 && synchronized == TRUE ) {
        byte  buffer[ 128 ];
        int   result;
        dword xd;

        if ( race_info->status_time % 50 == 0 ) {
            buffer[ 0 ] = 0x03;
            memcpy( &buffer[ 1 ], &vehicles[ 0 ]->info->x, 4 );
            memcpy( &buffer[ 5 ], &vehicles[ 0 ]->info->y, 4 );
            memcpy( &buffer[ 9 ], &vehicles[ 0 ]->info->z, 4 );

            result = send( conn_socket, ( const char* )buffer, 14, NULL );

            if ( result != 14 ) {
                MessageBoxA( NULL, "send zjebal", NULL, MB_OK );
            }
        }

        result = recv( conn_socket, ( char* )buffer, 128, NULL );

        if ( result == 0 ) {
            return;
        } else if ( result < 0 ) {
            int error = WSAGetLastError();

            if ( error == 10035 ) {
                return;
            }

            char buf[16]; _itoa_s( error, buf, 10 );
            MessageBoxA( NULL, "no cusz, rozlaczyl sie", buf, MB_OK );
            disconnected = TRUE;

            closesocket( conn_socket );
            WSACleanup();
            return;
        } else if ( result > 0 && buffer[ 0 ] == 0x37 ) {
            dword players = buffer[ 1 ];

            for ( int i = 0; i < players; i++ ) {
                vehicles[ 1 + i ]->acceleration_strength = 0;
                vehicles[ 1 + i ]->handling_strength = 0;
                vehicles[ 1 + i ]->steering_strength = 0;

                memcpy( &vehicles[ 1 + i ]->info->x, &buffer[ ( i * 12 ) + 2 ], 4 );
                memcpy( &vehicles[ 1 + i ]->info->y, &buffer[ ( i * 12 ) + 2 + 4 ], 4 );
                memcpy( &vehicles[ 1 + i ]->info->z, &buffer[ ( i * 12 ) + 2 + 4 + 4 ], 4 );
            }
        }
    }
}

_declspec( naked ) void hook_race_loop() {
    _asm push ecx
    _asm call handle_race_loop
    _asm pop ecx
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
    VirtualProtect( ( void* )MR2_DLL_POS, 0x6000, PAGE_EXECUTE_READWRITE, &old_protect );

    dword pos = ( dword )&hook_race_loop;
    pos = pos - ( ( dword )GAME_RACE_LOOP_HOOK_POS + 5 );

    *( byte* )( GAME_RACE_LOOP_HOOK_POS ) = 0xE9;
    *( dword* )( GAME_RACE_LOOP_HOOK_POS + 1 ) = pos;

    vehicles = ( VehicleInfo** )( MR2_DLL_POS + GAME_VEHICLES_ARRAY );

    *( byte* )( MR2_DLL_POS + GAME_AI_DISABLE ) = 0x7F;

    VirtualProtect( ( void* )MR2_DLL_POS, 0x6000, old_protect, &old_protect );
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
        char xd[ 12 ]; _itoa_s( WSAGetLastError(), xd, 10 );
        MessageBoxA( NULL, "Error=1", xd, MB_OK );
        WSACleanup();
        return FALSE;
    }

    sockaddr_in socket_address;
    socket_address.sin_addr.s_addr = inet_addr( server_address );
    socket_address.sin_port        = htons( port );
    socket_address.sin_family      = AF_INET;

    result = connect( conn_socket, ( SOCKADDR* ) &socket_address, sizeof( socket_address ) );

    if ( result == SOCKET_ERROR ) {
        char xd[ 12 ]; _itoa_s( WSAGetLastError(), xd, 10 );
        MessageBoxA( NULL, "Error=2", xd, MB_OK );

        closesocket( conn_socket );
        WSACleanup();
        return FALSE;
    }

    dword name_length = strlen( player_name );
    char buffer[ 128 ];

    buffer[ 0 ] = 0x01;
    buffer[ 1 ] = name_length;
    memcpy( &buffer[ 2 ], player_name, name_length );

    result = send( conn_socket, buffer, name_length + 3, NULL );

    if ( result == SOCKET_ERROR ) {
        char xd[ 12 ]; _itoa_s( WSAGetLastError(), xd, 10 );
        MessageBoxA( NULL, "Error=3", xd, MB_OK );

        closesocket( conn_socket );
        WSACleanup();
        return FALSE;
    }

    char recvbuf[ 16 ];

    do {
        result = recv( conn_socket, recvbuf, 16, NULL );

        if ( result < 1 || recvbuf[ 0 ] != 0x01 ) {
            char xd[ 12 ]; _itoa_s( WSAGetLastError(), xd, 10 );
            MessageBoxA( NULL, xd, xd, MB_OK );
            return FALSE;
        }

        player_id = recvbuf[ 1 ];
        break;
    } while ( result > 0 );

    dword sflags = 1;
    result = ioctlsocket( conn_socket, FIONBIO, &sflags );

    if ( result != NO_ERROR ) {
        char xd[ 12 ]; _itoa_s( WSAGetLastError(), xd, 10 );
        MessageBoxA( NULL, "Error=4", xd, MB_OK );

        closesocket( conn_socket );
        WSACleanup();
        return FALSE;
    }

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