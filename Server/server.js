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

}

new Server().initialize();