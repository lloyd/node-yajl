// Will be a complete reformatter in javascript similar to the Yajl reformatter

require.paths.unshift( __dirname + '/build/default' );

var yajl = require('yajl');
var fs = require('fs');

var n = 5000, v, start_t, end_t;

var td = fs.readFileSync("testdata.json");
{
/*
    var s = new yajl.Stream({allowMultipleValues: true});
    s.update("null");
    console.log(s.complete());

    var s = new yajl.Stream({allowMultipleValues: true});
    s.update("true");
    console.log(s.complete());
    var s = new yajl.Stream({allowMultipleValues: true});
    s.update("false");
    console.log(s.complete());

    var s = new yajl.Stream({allowMultipleValues: true});
    s.update("47");
    console.log(s.complete());

    var s = new yajl.Stream({allowMultipleValues: true});
    s.update("3.1415");
    console.log(s.complete());

    var s = new yajl.Stream({allowMultipleValues: true});
    s.update("\"hi mom!\"");
    console.log(s.complete());

    var s = new yajl.Stream({allowMultipleValues: true});
    s.update("[\"hi mom!\", 1, 2, true, false ]");
    console.log(s.complete());

    var s = new yajl.Stream({allowMultipleValues: true});
    s.update("[\"hi mom!\", 1, 2, true, false, \"fuck\", [null, 3.1415]]");
    console.log(s.complete());

    var s = new yajl.Stream({allowMultipleValues: true});
    s.update("{ \"foo\": \"hi mom!\", \"bar\": 1, \"boom\": { \"baz\": null, \"bing\": 3.1415} }");
    console.log(s.complete());
*/

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

