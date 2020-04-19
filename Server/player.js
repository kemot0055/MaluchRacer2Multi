class Player {

    socket = null;
    server = null;

    initialized = false;
    id          = -1;
    name        = 'Player';

    constructor( id, socket, server ) {
        this.server = server;
        this.socket = socket;
        this.id     = id;

        socket.on( 'data', ( data ) => {
            console.log( data );

            if ( data.length <= 0 ) {
                return;
            }

            const message_id = data.readUInt8( 0 );

            if ( message_id === 0x01 ) {
                return this.initialize( data.slice( 1 ) );
            }
        } );

        socket.on( 'close', () => {
            console.log( `Player "${ this.name }" disconnected` );

            delete this.server.players[ this.id ];
            this.server.players_nb--;
        } );

        socket.on( 'error', ( error ) => {
            console.log( `Player ${ this.name } disconnected (error)`, error );
        } );
    }

    initialize( data ) {
        if ( this.initialized ) {
            return;
        }

        this.name = data.toString( 'utf-8', 1, data.readUInt8() + 1 );
        this.initialized = true;
    }

}

module.exports = Player;