#ifndef _JSONWRAP_H
#define _JSONWRAP_H

#include <stdlib.h>
#include <string.h>

#include <yajl/yajl_tree.h>

namespace yajljs { namespace internal {

struct JSONValue
{
    struct {
        yajl_type type;
        union
        {
            char * string;
            struct {
                long long i;
                double  d;
                char   *r;
                unsigned int flags;
            } number;
            struct {
                const char **keys;
                struct JSONValue **values;
                size_t len;
            } object;
            struct {
                struct JSONValue **values;
                size_t len;
            } array;
        } u;
    } v;
    void * handleManaged;
};

v8::Handle<v8::Value> arrayWrap(JSONValue * v);
v8::Handle<v8::Value> objectWrap(JSONValue * v);

static v8::Handle<v8::Value> getHandle(JSONValue * v) {
    switch(v->v.type) {
        case yajl_t_null:   return v8::Null();
        case yajl_t_true:   return v8::True();
        case yajl_t_false:  return v8::False();
        case yajl_t_number: {
            if (YAJL_IS_DOUBLE(&(v->v))) return v8::Number::New(v->v.u.number.d);
            else return v8::Integer::New(v->v.u.number.i);
        }
        case yajl_t_string: return v8::String::New(v->v.u.string);
        case yajl_t_array:  return arrayWrap(v);
        case yajl_t_object: return objectWrap(v);
        default: return v8::Handle<v8::Value>();
    }
}

static void freeVal(JSONValue * v) {
    if (NULL == v->handleManaged) {
        switch (v->v.type) {
            case yajl_t_string:
                if (v->v.u.string) free(v->v.u.string);
                break;
            case yajl_t_object: {
                for (int i = 0; i < v->v.u.object.len; i++) {
                    free((void *) v->v.u.object.keys[i]);
                    if (v->v.u.object.values[i])
                        freeVal(v->v.u.object.values[i]);
                }
                if (v->v.u.object.keys) {
                    free(v->v.u.object.keys);
                    free(v->v.u.object.values);
                }
                break;
            }
            case yajl_t_array: {
                for (int i = 0; i < v->v.u.array.len; i++) {
                    freeVal(v->v.u.array.values[i]);
                }
                if (v->v.u.array.values) free(v->v.u.array.values);
                break;
            }
            default:
                break;
        }
        free(v);
    }
}

static void arrayAdd(JSONValue * v, JSONValue * i) {
    // chunked allocation affords ~10% perf improvment
    int cur = (v->v.u.array.len + 7) & ~0x7;
    if (v->v.u.array.len >= cur) {
        cur += 8;
        v->v.u.array.values = (JSONValue **) realloc((void *) v->v.u.array.values, sizeof(JSONValue *) * (cur));
    }
    v->v.u.array.values[v->v.u.array.len] = i;
    v->v.u.array.len++;
}

static void mapAddKey(JSONValue * m, char * s, size_t l) {
    // chunked allocation affords ~10% perf improvment
    int cur = (m->v.u.object.len + 7) & ~0x7;
    if (m->v.u.object.len >= cur) {
        cur += 8;
        m->v.u.object.keys = (const char **) realloc((void *) m->v.u.object.keys,
                                                     sizeof(char *) * (cur));
        m->v.u.object.values = (JSONValue **) realloc((void *) m->v.u.object.values,
                                                      sizeof(JSONValue *) * (cur));
    }

    m->v.u.object.keys[m->v.u.object.len] = strndup(s,l);
    m->v.u.object.values[m->v.u.object.len] = NULL; // to be set by subsequent call to mapAddValue
    m->v.u.object.len++;
}

static void mapAddValue(JSONValue * m, JSONValue * v) {
    m->v.u.object.values[m->v.u.object.len - 1] = v;
}

} }

#endif
