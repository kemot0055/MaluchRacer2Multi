const Player = require( './player' );
const net    = require( 'net' );

class Server {

    constructor() {
        this.players_nb = 0;
        this.players    = {};
        this.server     = null;

        //this.player_sync_event = this.player_sync_event.bind( this );
    }

    initialize() {
        this.server = net.createServer( ( socket ) => {
            if ( this.players_nb >= 4 ) {
                return console.log( 'Cannot exceed four players :-(' ), socket.end(); // TODO: Return message to client
            }

            socket.setTimeout( 10000, socket.end );

            socket.on( 'error', ( error ) => {
                //idk how to handle
            } );

            socket.on( 'data', ( data ) => {
                if ( data.readUInt8() !== 0x01 ) {
                    return socket.end( Buffer.from( [ 0x7F, 0x00 ] ) );
                }

                this.players_nb++;
                let player_id = 0;

                while ( player_id in this.players === true ) {
                    player_id = ( player_id + 1 ) % 256;
                }

                ( this.players[ player_id ] = new Player( player_id, socket, this ) ).initialize( data.slice( 1 ) );
            } );
        } );

        this.server.listen( 2137, () => {
            console.log( 'server started' );
        } );
    }

    player_leaved( player_id ) {
        delete this.players[ player_id ], this.players_nb--;
    }

    player_update( player ) {
        for ( let player_id in this.players ) {
            const _player = this.players[ player_id ];

            if ( _player === player || _player.synchronized === false ) {
                continue;
            }

            const buffer = Buffer.alloc( 128 );
            buffer.writeUInt8( 0x37, 0 );
            let plr = 0;

            for ( let _player_id in this.players ) {
                const __player = this.players[ _player_id ];

                if ( __player === _player || __player.synchronized === false ) {
                    continue;
                }

                let offset = plr * 36;

                buffer.writeFloatLE( __player.x, 2 + offset + 0 );
                buffer.writeFloatLE( __player.y, 2 + offset + 4 );
                buffer.writeFloatLE( __player.z, 2 + offset + 8 );

                buffer.writeFloatLE( __player.rx, 2 + offset + 12 );
                buffer.writeFloatLE( __player.ry, 2 + offset + 16 );
                buffer.writeFloatLE( __player.rz, 2 + offset + 20 );

                buffer.writeFloatLE( __player.vx, 2 + offset + 24 );
                buffer.writeFloatLE( __player.vy, 2 + offset + 28 );
                buffer.writeFloatLE( __player.vz, 2 + offset + 32 );

                plr++;
            }

            if ( plr < 1 ) {
                continue;
            }

            buffer.writeUInt8( plr, 1 );                
            _player.socket.write( buffer );
        }
    }

    player_synchronize() {
        for ( let player in this.players ) {
            if ( this.players[ player ].synchronized === false ) {
                return;
            }
        }

        console.log( 'starting' );

        for ( let player in this.players ) {
            this.players[ player ].socket.write( Buffer.from( [ 0x01 ] ) );
        }
    }

}

new Server().initialize();