// Will be a complete reformatter in javascript similar to the Yajl reformatter

require.paths.unshift( __dirname + '/build/default' );

var yajl = require('yajl');
var fs = require('fs');

var n = 5000, v, start_t, end_t;

var td = fs.readFileSync("testdata.json");
{
    start_t = (new Date).getTime();
    var s = new yajl.Stream({allowMultipleValues: true});
    for ( var i = 0; i < n; i++ ) {
        s.update(td);
        v = s.complete();
    }
    end_t = (new Date).getTime();
    console.log( "YAJL Stream parse: " + n + " t1 values: " + ( end_t - start_t ) + " ms ( ~" + Math.round( 1000 * n / ( end_t - start_t ) ) + "/s )" );

    start_t = (new Date).getTime();
    for ( var i = 0; i < n; i++ ) {
        v = JSON.parse(td);
    }
    end_t = (new Date).getTime();
    console.log( "JSON parse: " + n + " t1 values: " + ( end_t - start_t ) + " ms ( ~" + Math.round( 1000 * n / ( end_t - start_t ) ) + "/s )" );
}
