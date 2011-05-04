/*
 * Written by Lloyd Hilaiel in 2011 and placed into the public domain,
 * With the idea that the upstream maintainers of node.js yajl bindings
 * may integrate the implementation, and license it as they've licensed
 * the rest of their projects.  Probably giving me credit somewhere, but
 * really, life is too short.
 */

#ifndef YAJLJS_STREAM_H
#define YAJLJS_STREAM_H

#include <v8.h>
#include <node.h>
#include <node/node_events.h>
#include <yajl/yajl_parse.h>
#include <stack>
#include <string>
#include "jsonwrap.h"

namespace yajljs
{
    class Stream : public node::ObjectWrap
    {
      public:
        static void Initialize ( v8::Handle<v8::Object> config );

        static v8::Handle<v8::Value> New (const v8::Arguments& args);
        static v8::Handle<v8::Value> Update( const v8::Arguments& args );
        static v8::Handle<v8::Value> Complete( const v8::Arguments& args );
        static v8::Handle<v8::Value> Reset( const v8::Arguments& args );

      protected:
        Stream( yajl_option opt );
        ~Stream();

      private:
        void Update(const unsigned char*, int);
        v8::Handle<v8::Value> Complete();
        yajl_handle yhand;
        std::stack< internal::JSONValue * > tokstack;
        internal::JSONValue * topval;

        static int yajljs_null(void * ctx);
        static int yajljs_boolean(void * ctx, int boolean);
        static int yajljs_integer(void * ctx, long long int i);
        static int yajljs_double(void * ctx, double d);
        static int yajljs_number(void * ctx, const char * s, size_t l);
        static int yajljs_string(void * ctx, const unsigned char * stringVal,
                                 size_t stringLen);
        static int yajljs_map_key(void * ctx, const unsigned char * stringVal,
                                  size_t stringLen);
        static int yajljs_start_map(void * ctx);
        static int yajljs_end_map(void * ctx);
        static int yajljs_start_array(void * ctx);
        static int yajljs_end_array(void * ctx);
    };
}

#endif
