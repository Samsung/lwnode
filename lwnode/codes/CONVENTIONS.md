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
// ValueWrap<T, V8>
// @tparam V8 V8 type
// @tparam T corresponding internal type

// Usage:
ValueWrap<ContextWrap, Context> value(this);
value.get()->Enter();

ValueWrap<ContextWrap, Context>(this).get()->Enter(); // one-line

// The above one-line is equal as the following:

ValueWrap* value = reinterpret_cast<ValueWrap*>(this);
ContextWrap* _context = value->get();
assert(_context->type() == v8::Context);
_context->Enter();
```

- Use the same parameter name with the prefix, `_` (underscore), to help  identifying casted values.

``` c++
MaybeLocal<Script> Script::Compile(Local<Context> context,
                                   Local<String> source,
                                   ScriptOrigin* origin) {

  auto _context = ValueWrap<ContextWrap, Context>(*context).get(); // not anything like `ctx`
  auto _soruce = ValueWrap<StringRef, String>(*source).get(); // not anything like `src`
```
## Relations

- IsolateWrap *-> ContextWrap
- IsolateWrap *-> HandleScope
- ContextWrap 1-> IsolateWrap
