const net = require( 'net' );

const players = [];

class Player {

    socket = null;
    name   = '';

    constructor( socket ) {
        this.socket = socket;

        socket.on( 'data', ( data ) => {
            console.log( data );
        } );

        socket.on( 'error', ( error ) => {
            console.log( error );
        } );

        socket.write( 'gib_data_plz\0' );
    }

    update() {

    }

}

const server = net.createServer( function( socket ) {
    if ( players.length > 4 ) {
        return console.log( 'Cannot exceed four players :-(' ), socket.end();
    }

    players.push( new Player( socket ) );
} );

server.listen( 2137, 'localhost', () => {
    console.log( 'server started' );

    //setInterval( () => {
        
    //}, 5000 );
} );