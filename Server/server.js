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

        setInterval( () => {
            for ( let player in this.players ) {
                player = this.players[ player ];

                if ( player.synchronized === false ) {
                    continue;
                }

                const buffer = Buffer.alloc( 128 );
                buffer.writeUInt8( 0x37, 0 );
                let plr = 0;

                for ( let _player in this.players ) {
                    _player = this.players[ _player ];

                    if ( player === _player ) {
                        continue;
                    }

                    if ( _player.synchronized === false ) {
                        continue;
                    }

                    let offset = plr * 24;

                    buffer.writeFloatLE( _player.x, 2 + offset + 0 );
                    buffer.writeFloatLE( _player.y, 2 + offset + 4 );
                    buffer.writeFloatLE( _player.z, 2 + offset + 8 );

                    buffer.writeFloatLE( _player.rx, 2 + offset + 12 );
                    buffer.writeFloatLE( _player.ry, 2 + offset + 16 );
                    buffer.writeFloatLE( _player.rz, 2 + offset + 20 );

                    plr++;
                }

                if ( plr < 1 ) {
                    continue;
                }

                buffer.writeUInt8( plr, 1 );                
                player.socket.write( buffer );
            }
         }, 1000 / 20 );
    }

}

new Server().initialize();