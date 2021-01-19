# Conventions



## Naming in lwnode apis

- If a corresponding class or a simlar concept exists in v8, name it with a postfix, `Wrap`.
```
  - Context (v8)
  - ContextWrap (lwnode)
  - Isolate (v8)
  - IsolateWrap (lwnode)
```

- If there is a corresponding functions in v8, use the exact name.
```
  ContextWrap::Enter (lwnode)
  Context::Enter (v8)
```

- If not, use the name starting with lower letter.
```
  IsolateWrap::pushHandleScope (lwnode internal)
```

- Use lwnode apis under the namespace or use unique api name without the namespace.

- Avoid using a brand name to name lwnode if possible. The brand name could be changed in future.

- The postfix, `Ref`, stands for `Reference`. Thus, don't use it as a pointer `Ref*`.

```
typedef Escargot::ContextRef* JsContextRef; // (x)
```





## Sources

### Source regions
```
User apps (Node.js) -- v8 apis (`src/api.cc`) -- lwnode apis (`src/api`)
```

- In v8 apis, `api.cc`, `ValueWrap` should be used for type convertion between v8 and lwnode apis.
- In lwnode apis,
  - Use `Escargot type` for input params as many as possible.
  - Use `ValueWrap` for input params isn't preferred.
  - Use `v8 type` is allowed.



## Type conversion between lwnode world and v8 world

`api.cc` is like a middle land. Any value moving out from lwnode world should be wrapped using `ValueWrap`.



### New
- Wrap any value using `ValueWrap` before returning it.

```c++
// ValueWrap<T, V8>
// @tparam V8 V8 type
// @tparam T corresponding internal type

// Usage:
auto _context = ContextWrap::New(isolate);
auto value = ValueWrap<ContextWrap, Context>::New(_context);

// dynamic allocations are protected. So the following doesn't work.
// auto value = new ValueWrap<ContextWrap, Context>(_context);
```



### Use

- Wrap any value using `ValueWrap` before use it.

```c++
// template ValueWrap<T, V8> 
// @tparam V8 V8 type
// @tparam T corresponding internal type

// Usage 1:
ValueWrap<ContextWrap, Context>::fromV8(this)->Enter();

// Usage 2:
ValueWrap<ContextWrap, Context>(this).get()->Enter();

// Usage 3:
ValueWrap<ContextWrap, Context> _value(this);
_value.get()->Enter();

// Usage 4:
ValueWrap<ContextWrap, Context>* that 
     = reinterpret_cast<ValueWrap<ContextWrap, Context>*>(this);
ContextWrap* _context = that->get();
assert(_context->type() == v8::Context);
_context->Enter();
```

- Use the same parameter name with the prefix, `_` (underscore), to help identifying casted values.

``` c++
MaybeLocal<Script> Script::Compile(Local<Context> context,
                                   Local<String> source,
                                   ScriptOrigin* origin) {

  auto _context = ValueWrap<ContextWrap, Context>(*context).get(); // not anything like `ctx`
  auto _soruce = ValueWrap<StringRef, String>(*source).get(); // not anything like `src`
```
* Use the prefix, `_` (underscore), to help identifying internal values.

```diff
++ auto _isolate = IsolateWrap::currentIsolate(); (O)
-- auto isolate = IsolateWrap::currentIsolate(); (X)
```

* Name `that` for the exact value of `this` casted.

```diff
Local<Script> UnboundScript::BindToCurrentContext() {
++   ScriptRef* that = ValueRef<ScriptRef, UnboundScript>::fromV8(this); (O)
--   ScriptRef* script = ValueRef<ScriptRef, UnboundScript>::fromV8(this); (X)
}
```



### Return

* Retrun a `ValueWrap` using a `Local`

```c++
auto value = ValueWrap<ContextWrap, Context>::New(_context);

....

// Usage 1
return value->toLocal();

// Usage 2
return Local<Context>::New(external_isolate, value);
```





## Relations

- IsolateWrap *-> ContextWrap
- IsolateWrap *-> HandleScope
- ContextWrap 1-> IsolateWrap

