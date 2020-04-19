class Player {

    socket = null;
    server = null;

    initialized  = false;
    id           = -1;
    name         = 'Player';
    synchronized = false;

    x = 0;
    y = 0;
    z = 0;

    xd = 0;

    constructor( id, socket, server ) {
        this.server = server;
        this.socket = socket;
        this.id     = id;

        socket.on( 'data', ( data ) => {
            this.xd++;

            if ( data.length <= 0 ) {
                return;
            }

            const message_id = data.readUInt8( 0 );

            if ( message_id === 0x01 ) {
                return this.initialize( data.slice( 1 ) );
            } else if ( message_id === 0x02 ) {
                this.synchronized = true;
                this.server.player_sync_event( this );
            } else if ( message_id === 0x03 ) {
                this.x = data.readFloatLE( 1 );
                this.y = data.readFloatLE( 5 );
                this.z = data.readFloatLE( 9 );

                console.log( `Player "${ this.name }" position: {x = ${ this.x }, y = ${ this.y }, z = ${ this.z }}  ${ this.xd }` );
            }

            this.socket.write( Buffer.from( [ 0x00 ] ) );
        } );

        socket.on( 'close', ( had_error ) => {
            console.log( `Player "${ this.name }" disconnected ${ had_error ? '(error)' : '' }` );

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