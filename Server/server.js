const Player = require( './player' );
const net    = require( 'net' );

class Server {

    server     = null;
    players    = {};
    players_nb = 0;

    initialize() {
        this.server = net.createServer( ( socket ) => {
            if ( this.players_nb >= 4 ) {
                return console.log( 'Cannot exceed four players :-(' ), socket.end(); // TODO: Return message to client
            }

            let player_id = 0;

            while ( player_id in this.players === true ) {
                player_id = ( player_id + 1 ) % 255;
            }

            this.players[ player_id ] = new Player( player_id, socket, this );
            this.players_nb++;

            socket.write( Buffer.from( [ 0x01, player_id ] ) );
        } );

        this.server.listen( 2137, 'localhost', () => {
            console.log( 'server started' );
        } );
    }

    player_sync_event( player ) {
        for ( let player in this.players ) {
            if ( this.players[ player ].synchronized === false ) {
                return;
            }
        }

        let buffer = Buffer.from( [ 0x01, 0xDE, 0xCA, 0xFA ] );

        for ( let player in this.players ) {
            this.players[ player ].socket.write( buffer ); // not works for some reason...
        }

        setInterval( () => {
            for ( let player in this.players ) {
                player = this.players[ player ];

                const buffer = Buffer.alloc( 64 );
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

                    let offset = plr * 12;

                    buffer.writeFloatLE( _player.x, offset + 2 );
                    buffer.writeFloatLE( _player.y, offset + 2 + 4 );
                    buffer.writeFloatLE( _player.z, offset + 2 + 4 + 4 );

                    plr++;
                }

                if ( plr <= 0 ) {
                    continue;
                }

                buffer.writeUInt8( plr, 1 );
                //console.log( buffer );

                player.socket.write( buffer );
            }
        }, 1000 / 10 );
    }

}

new Server().initialize();