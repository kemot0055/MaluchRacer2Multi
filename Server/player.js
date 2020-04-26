const PLAYER_INITIALIZED  = 0x00000001;
const PLAYER_SYNCHRONIZED = 0x00000002;
const PLAYER_PLAYING      = 0x00000004;

class Player {

    constructor( id = -1, socket = null, server = null ) {
        this.server       = server;
        this.socket       = socket;
        this.id           = id;

        this.name         = 'Player';
        this.packets      = 0;

        this.synchronized = false;
        this.initialized  = false;

        this.x            = 0;
        this.y            = 0;
        this.z            = 0;

        this.rx           = 0;
        this.ry           = 0;
        this.rz           = 0;

        this.process      = this.process.bind( this );
    }

    process( data ) {
        const message_id = data.readUInt8();

        if ( message_id === 0x02 ) {
            this.synchronized = true, this.server.player_synchronize();
        } else if ( message_id === 0x03 ) {
            this.x = data.readFloatLE( 1 + 0 );
            this.y = data.readFloatLE( 1 + 4 );
            this.z = data.readFloatLE( 1 + 8 );

            this.rx = data.readFloatLE( 1 + 12 );
            this.ry = data.readFloatLE( 1 + 16 );
            this.rz = data.readFloatLE( 1 + 20 );

            console.log( `${ this.name }: x = ${ this.x }, y = ${ this.y }, z = ${ this.z }, delta = ${ Date.now() - this.last_packet }` );
        }

        this.packets++;
        this.last_packet = Date.now();
    }

    initialize( data ) {
        if ( this.initialized === true ) {
            return false;
        }

        this.socket.setTimeout( 0 );

        for ( const id of [ 'data', 'error' ] ) {
            this.socket.removeAllListeners( id );
        }

        this.socket.on( 'error', ( error ) => {
            console.log( `Player ${ this.name } disconnected (${ error.code })` );
        } );

        this.socket.on( 'close', ( had_error ) => {
            this.server.player_leaved( this.id );

            if ( had_error === true ) {
                return;
            }

            console.log( `Player ${ this.name } disconnected` );
        } );

        this.socket.on( 'data', this.process );

        this.name        = data.toString( 'utf-8', 1, 1 + data.readUInt8() );
        this.initialized = true;

        return this.socket.write( Buffer.from( [ 0x01, this.id ] ) ), console.log( `Player ${ this.name } connected` ), true;
    }

}

module.exports = Player;