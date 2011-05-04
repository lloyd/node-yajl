#include <v8.h>
#include <node.h>

#include "jsonwrap.h"

using namespace v8;
using namespace yajljs::internal;

Handle<Value> yajljs::internal::arrayWrap(JSONValue * v)
{
    HandleScope scope;
    Handle<Array> a = Array::New(v->v.u.array.len);

    for (int i = 0; i < v->v.u.array.len; i++) {
        a->Set(i, yajljs::internal::getHandle(v->v.u.array.values[i]));
    }
    return scope.Close(a);
}

class ObjectWrapper
{
public:
    ObjectWrapper(JSONValue * v) : m_v(v)
    {
        if (s_t.IsEmpty()) {
            s_t = Persistent<ObjectTemplate>::New(ObjectTemplate::New());
            s_t->SetNamedPropertyHandler(jsonGet, jsonSet, 0, 0, jsonEnum);
            s_t->SetInternalFieldCount(1);
        }
        m_v->handleManaged = (void *) this;
    }

    Handle<Value> instance()
    {
        if (m_handle.IsEmpty()) {
            m_handle = Persistent<Object>::New(s_t->NewInstance());
            m_handle->SetInternalField(0, External::New(m_v));
            m_handle.MakeWeak((void *) this, weakRefCallback);
        }
        return m_handle;
    }


private:
    JSONValue * m_v;
    Persistent<Object> m_handle;

    ~ObjectWrapper()
    {
        m_v->handleManaged = NULL;
        freeVal(m_v);
    }

    static void weakRefCallback(Persistent<Value> object, void * ctx) {
        ObjectWrapper * ow = (ObjectWrapper *) ctx;
        delete ow;
    }


    static Handle<Value> jsonSet(Local<String>, Local<Value>, const AccessorInfo &)
    {
        return Handle<Value>();
    }

    static Handle<Value> jsonGet(Local<String> name, const AccessorInfo &info)
    {
        HandleScope scope;

        Local<Object> self = info.Holder();
        Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
        JSONValue * v = (JSONValue *) wrap->Value();

        String::Utf8Value k(name);

        for (int i = 0; i < v->v.u.object.len; i++) {
            if (!strcmp(*k, v->v.u.object.keys[i])) {
                return yajljs::internal::getHandle(v->v.u.object.values[i]);
            }
        }
        return Handle<Value>();
    }

    static Handle<Array> jsonEnum(const AccessorInfo &info)
    {
        HandleScope scope;

        Local<Object> self = info.Holder();
        Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
        JSONValue * v = (JSONValue *) wrap->Value();

        Handle<Array> a = Array::New(v->v.u.array.len);
        for (int i = 0; i < v->v.u.object.len; i++) {
            a->Set(i, String::New(v->v.u.object.keys[i]));
        }
        return scope.Close(a);
    }

    static Persistent<ObjectTemplate> s_t;
};
Persistent<ObjectTemplate> ObjectWrapper::s_t;

Handle<Value> yajljs::internal::objectWrap(JSONValue * v)
{
    ObjectWrapper * ow = NULL;
    if (v->handleManaged) {
        ow = (ObjectWrapper *) v->handleManaged;
    } else {
        ow = new ObjectWrapper(v);
    }
    return ow->instance();
}
