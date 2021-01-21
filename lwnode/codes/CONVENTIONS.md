# Conventions



## Naming apis

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

- If posssible, avoid using a brand name (e.g lwnode) to name apis. Because the brand name could be changed in future.

- The postfix, `Ref`, stands for `Reference`. Thus, don't use it as a pointer `Ref*`.

```diff
- typedef Escargot::ContextRef* JsContextRef; // (x)
```

* API Levels
  * API level 1 : v8 apis
  * API level 2 : EscargotShim apis (Wrapped Values)
  * API level 3 : Escargot apis
* In `api.cc` if there are same concepts among the API levels, use the following conventions:
  * name a value of API level 3 (Escargot Value) with `__` .
  * name a value of API level 2 (Wrapped Value) with `_`.
  * name v8 Value without `_`.



## Sources

### Source regions

|       Level 0       |        Level 1         |         Level 2         |       Level 3        |
| :-----------------: | :--------------------: | :---------------------: | :------------------: |
| User apps (Node.js) | v8 apis (`src/api.cc`) | lwnode apis (`src/api`) | js engine (escargot) |

* In the level 1, `ValueWrap` should be used for type convertion between v8 and lwnode apis.

  

## Type conversion between lwnode world and v8 world

`api.cc` is like a middle land. Any value moving out from lwnode world should be wrapped using `ValueWrap`.



### New
- Wrap any value using `ValueWrap` before returning it.

```c++
// Escargot::ValueRef* inherited
auto string = StringRef::createFromUTF8(...);
auto value = new ValueWrap(string);

// Others 
// e.g) Escargot::Script
auto value = ValueWrap::createScript(...);
```



### Use

- Wrap any value using `ValueWrap` before use it.

```c++
#define VAL(that) reinterpret_cast<const ValueWrap*>(that)

// Usage:
VAL(this); // = ValueWrap(reinterpret_cast<ValueWrap*>(this));

// e.g) access StringRef*
VAL(*source->source_string)->value()->asString();

// e.g) access ContextWrap::Enter();
VAL(this)->context()->Enter();
```



### Return

* Retrun a `ValueWrap` using a `Local`

```c++
auto string = StringRef::createFromUTF8(...);

// Usage:   
return Local<String>::New(isolate, new ValueWrap(string));
```



### Exception Handling

* `IsolateWrap::IsExecutionTerminating` returns true if a pending exception that should be thrown exists.
* Scheduling throwing an excpetion can be set through `IsolateWrap::scheduleThrow`.



## Relations

- IsolateWrap *-> ContextWrap
- IsolateWrap *-> HandleScope
- ContextWrap 1-> IsolateWrap

