/*
 * Written by Lloyd Hilaiel in 2011 and placed into the public domain,
 * With the idea that the upstream maintainers of node.js yajl bindings
 * may integrate the implementation, and license it as they've licensed
 * the rest of their projects.  Probably giving me credit somewhere, but
 * really, life is too short.
 */

#include <v8.h>
#include <node.h>
#include <node/node_buffer.h>
#include <yajl/yajl_parse.h>

#include "stream.h"
#include <stdlib.h>
#include <string.h>
#include <string>

#include "jsonwrap.h"

using namespace v8;
using namespace node;
using namespace yajljs::internal;

#define PUSHTOK(s, val)                                         \
    if ((s)->topval == NULL) {                                  \
        (s)->topval = val;                                      \
    } else if ((s)->tokstack.top()->v.type == yajl_t_array) {   \
        arrayAdd((s)->tokstack.top(), (val));                   \
    } else if ((s)->tokstack.top()->v.type == yajl_t_object) {  \
        mapAddValue((s)->tokstack.top(), (val));                \
    }

int
yajljs::Stream::yajljs_null(void * ctx)
{
    yajljs::Stream * s = (yajljs::Stream *) ctx;
    JSONValue * v = (JSONValue *) calloc(1, sizeof(struct JSONValue));
    v->v.type = yajl_t_null;
    PUSHTOK(s, v);
    return 1;
}

int
yajljs::Stream::yajljs_boolean(void * ctx, int boolean)
{
    yajljs::Stream * s = (yajljs::Stream *) ctx;
    JSONValue * v = (JSONValue *) calloc(1, sizeof(struct JSONValue));
    v->v.type = boolean ? yajl_t_true : yajl_t_false;
    PUSHTOK(s, v);
    return 1;
}


int yajljs::Stream::yajljs_integer(void * ctx, long long i)
{
    yajljs::Stream * s = (yajljs::Stream *) ctx;
    JSONValue * v = (JSONValue *) calloc(1, sizeof(struct JSONValue));
    v->v.type = yajl_t_number;
    v->v.u.number.flags = YAJL_NUMBER_INT_VALID;
    v->v.u.number.i = i;
    PUSHTOK(s, v);
    return 1;
}

int yajljs::Stream::yajljs_double(void * ctx, double d)
{
    yajljs::Stream * s = (yajljs::Stream *) ctx;
    JSONValue * v = (JSONValue *) calloc(1, sizeof(struct JSONValue));
    v->v.type = yajl_t_number;
    v->v.u.number.flags = YAJL_NUMBER_DOUBLE_VALID;
    v->v.u.number.d = d;
    PUSHTOK(s, v);
    return 1;
}

int yajljs::Stream::yajljs_string(void * ctx, const unsigned char * stringVal,
                                  size_t stringLen)
{
    yajljs::Stream * s = (yajljs::Stream *) ctx;
    JSONValue * v = (JSONValue *) calloc(1, sizeof(struct JSONValue));
    v->v.type = yajl_t_string;
    v->v.u.string = strndup((char *) stringVal, stringLen);
    PUSHTOK(s, v);
    return 1;
}

int yajljs::Stream::yajljs_map_key(void * ctx, const unsigned char * stringVal,
                                   size_t stringLen)
{
    yajljs::Stream * s = (yajljs::Stream *) ctx;
    mapAddKey((s)->tokstack.top(), (char *) stringVal, stringLen);
    return 1;
}

int yajljs::Stream::yajljs_start_map(void * ctx)
{
    yajljs::Stream * s = (yajljs::Stream *) ctx;
    JSONValue * v = (JSONValue *) calloc(1, sizeof(struct JSONValue));
    v->v.type = yajl_t_object;
    PUSHTOK(s, v);
    (s)->tokstack.push(v);
    return 1;
}

int yajljs::Stream::yajljs_end_map(void * ctx)
{
    yajljs::Stream * s = (yajljs::Stream *) ctx;
    (s)->tokstack.pop();
    return 1;
}

int yajljs::Stream::yajljs_start_array(void * ctx)
{
    yajljs::Stream * s = (yajljs::Stream *) ctx;
    JSONValue * v = (JSONValue *) calloc(1, sizeof(struct JSONValue));
    v->v.type = yajl_t_array;
    PUSHTOK(s, v);
    (s)->tokstack.push(v);
    return 1;
}

int yajljs::Stream::yajljs_end_array(void * ctx)
{
    yajljs::Stream * s = (yajljs::Stream *) ctx;
    (s)->tokstack.pop();
    return 1;
}

void yajljs::Stream::Initialize ( v8::Handle<v8::Object> target )
{
    v8::HandleScope scope;

    v8::Local<v8::FunctionTemplate> t = v8::FunctionTemplate::New(New);
    t->InstanceTemplate()->SetInternalFieldCount(1);
    t->SetClassName(String::NewSymbol("Stream"));

    NODE_SET_PROTOTYPE_METHOD( t, "update", Update );
    NODE_SET_PROTOTYPE_METHOD( t, "complete", Complete );
    NODE_SET_PROTOTYPE_METHOD( t, "reset", Reset );

    target->Set( v8::String::NewSymbol( "Stream"), t->GetFunction() );
}

v8::Handle<v8::Value> yajljs::Stream::New (const v8::Arguments& args)
{
    v8::HandleScope scope;

    int opt = 0;

    if( args.Length() >= 1 && args[0]->IsObject() )
    {
        Local<Object> obj = args[0]->ToObject();

        opt |= ( obj->Get( String::New( "allowComments" ) )->ToInteger()->Value()       ) ? yajl_allow_comments         : 0;
        opt |= ( obj->Get( String::New( "dontValidateStrings" ) )->ToInteger()->Value() ) ? yajl_dont_validate_strings  : 0;
        opt |= ( obj->Get( String::New( "allowTrailingGarbage" ) )->ToInteger()->Value()) ? yajl_allow_trailing_garbage : 0;
        opt |= ( obj->Get( String::New( "allowMultipleValues" ) )->ToInteger()->Value() ) ? yajl_allow_multiple_values  : 0;
        opt |= ( obj->Get( String::New( "allowPartialValues" ) )->ToInteger()->Value()  ) ? yajl_allow_partial_values   : 0;
    }

    yajljs::Stream *handle = new yajljs::Stream((yajl_option) opt);
    handle->Wrap( args.This() );
    return args.This();
}

yajljs::Stream::Stream( yajl_option opt )
{
    static yajl_callbacks s_callbacks = {
        yajljs::Stream::yajljs_null,
        yajljs::Stream::yajljs_boolean,
        yajljs::Stream::yajljs_integer,
        yajljs::Stream::yajljs_double,
        NULL, // yajljs::Stream::yajljs_number,
        yajljs::Stream::yajljs_string,
        yajljs::Stream::yajljs_start_map,
        yajljs::Stream::yajljs_map_key,
        yajljs::Stream::yajljs_end_map,
        yajljs::Stream::yajljs_start_array,
        yajljs::Stream::yajljs_end_array
    };

    yhand = yajl_alloc( &s_callbacks, NULL, this );
    yajl_config( yhand, yajl_allow_comments, opt & yajl_allow_comments );
    yajl_config( yhand, yajl_dont_validate_strings, opt & yajl_dont_validate_strings);
    yajl_config( yhand, yajl_allow_trailing_garbage, opt & yajl_allow_trailing_garbage);
    yajl_config( yhand, yajl_allow_multiple_values, opt & yajl_allow_multiple_values);
    yajl_config( yhand, yajl_allow_partial_values, opt & yajl_allow_partial_values);

    topval = NULL;
}

yajljs::Stream::~Stream()
{
    yajl_free(yhand);
    yhand = NULL;
}

void
yajljs::Stream::Update(const unsigned char * str, int len)
{
    yajl_status s = yajl_parse(yhand, str, len);
    if (s != yajl_status_ok) {
        ThrowException(
            Exception::SyntaxError(
                String::New((const char *) yajl_get_error(yhand, 0, str, len))));
    }
}

v8::Handle<Value>
yajljs::Stream::Complete( void )
{
    yajl_status s = yajl_complete_parse(yhand);
    if (s != yajl_status_ok) {
        ThrowException(
            Exception::SyntaxError(
                String::New((const char *) yajl_get_error(yhand, 0, NULL, 0))));
    }

    if (topval == NULL) return Handle<Value>();
    Handle<Value> v(getHandle(topval));
    freeVal(topval);
    topval = NULL;
    return v;
}

v8::Handle<Value>
yajljs::Stream::Update( const Arguments& args )
{
    Stream *yh = Unwrap<Stream>( args.This() );

    for (int i = 0; i < args.Length(); i++){
        if (args[i]->IsString() ) {
            String::Utf8Value str(args[i]);
            yh->Update((const unsigned char *) *str, strlen((const char *) *str));
        } else if (node::Buffer::HasInstance(args[i]) ) {
            Local<Object> o = Object::Cast(*(args[i]));
            yh->Update((const unsigned char *) node::Buffer::Data(o),
                       node::Buffer::Length(o));
        } else {
            return ThrowException(
                Exception::TypeError(
                    String::New("Arguments must be strings or buffers")));
        }
    }

    return v8::Handle<Value>();
}

v8::Handle<Value>
yajljs::Stream::Complete( const Arguments& args )
{
    Stream *yh = Unwrap<Stream>( args.This() );
    return yh->Complete();
}

v8::Handle<Value>
yajljs::Stream::Reset( const Arguments& args )
{
    return v8::Handle<Value>();
}

