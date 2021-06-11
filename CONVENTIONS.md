# Conventions

### Directives

* **SHALL** is used to express mandatory requirements. (negative form: **SHALL NOT**)
  * Do not use MUST as an alternative to SHALL.
* **SHOULD** is used to express recommendations. (negative form: **SHOULD NOT**)
* **MAY** is used to express permissions .  (negative form: **NEED NOT**)
  * do not use CAN as an alternative to MAY.

---------

## Naming

- If a corresponding class or a similar concept exists in v8, name it with a postfix, `Wrap`.
The following table shows how a v8 class and its matching lwnode class are named.

| v8      | lwnode      |
|---------|-------------|
| Context | ContextWrap |
| Isolate | IsolateWrap |


- If there is a corresponding functions in v8, it **SHALL** use the exact name.
```
  ContextWrap::Enter (lwnode)
  Context::Enter (v8)
```

- If not, it **SHALL** use the name starting with a small letter.
```
  IsolateWrap::pushHandleScope (lwnode internal)
```

- It **SHOULD NOT** use a brand name (e.g lwnode) to name apis. Because the brand name could be changed in future.

- The postfix, `Ref`, stands for `Reference`. Thus, it **SHOULD NOT** use it as a pointer `Ref*`.

```diff
- typedef Escargot::ContextRef* JsContextRef; // (x)
```

* EscargotShim works as a bridge between V8 and Escargot. It is responsible for
  transforming a V8 value to an Escargot value, and vice versa. This results in
  three types of "modules" as described by the following diagram.

```
+----+      +--------------+      +----------+
| V8 |------| EscargotShim |------| Escargot |
+----+      +--------------+      +----------+
```

  * We deal with a three sets of APIs, it is often confusing which value, out of three,
    a variable refers to. To prevent this, we add a prefix when a variable is named as described below:

| API          | Prefix                                  |
|--------------|-----------------------------------------|
| V8           | v8 (can be omitted if context is clear) |
| EscargotShim | lw                                      |
| Escargot     | es                                      |

* This naming rules **SHALL** be applied to all ``api-*.cc`` files.

* Within EscargotShim, `ValueWrap` **SHALL** be used for type convertion between v8 and lwnode apis.


## Type conversion between lwnode world and v8 world

`api.cc` is like a middle land. Any value moving out from lwnode world **SHALL** be wrapped using `ValueWrap`.



### New
- Wrap any value using `ValueWrap` before returning it.

```c++
// Escargot::ValueRef* inherited
auto esString = StringRef::createFromUTF8(...);
auto lwValue = ValueWrap::createValue(string);
// auto value = new ValueWrap(string); // deprecated

// Others
// e.g) Escargot::Script
auto lwValue = ValueWrap::createScript(...);
```



### Use

- Wrap any value using `ValueWrap` before use it.

```c++
#define VAL(that) reinterpret_cast<ValueWrap*>(that)
#define CVAL(that) reinterpret_cast<const ValueWrap*>(that)

// Usage:
VAL(this); // = ValueWrap(reinterpret_cast<ValueWrap*>(this));

// e.g) use ValueRef* from `Local<String> name`
VAL(*name)->value()->asString();

// e.g) use StringRef* from `Local<String> source_string of Source*`
VAL(*source->source_string)->value()->asString();

// e.g) access ContextWrap::Enter();
VAL(this)->context()->Enter();
```

### Return

* Retrun a `ValueWrap` using a `Local`

```c++
auto esString = StringRef::createFromUTF8(...);

return Utils::NewLocal<String>(isolate, esString);
```

### Exception Handling

* `IsolateWrap::IsExecutionTerminating` returns true if a pending exception that should be thrown exists.
* Scheduling throwing an excpetion can be set through `IsolateWrap::scheduleThrow`.
* If a javascript operation could make an exception (e.g `v8::Object::SetPrototype`), it should do exception handling.



# Language styles

- Type Names
  - Type names start with a capital letter and have a capital letter for each new word: `MyClass`, `MyEnum`.
- Variable Names
  - Class data member
    - ``myValue_`` for a member variable
    - ``g_myValue`` for a global variable
    - ``s_myValue`` for a static variable
    - ``kMyConstValue`` for a (global) constant value
  - Common data (TBD)
    - 1) stringValue
    - 2) string_value (v8)
- File Names
  - Filenames should be all lowercase and may include dashes (-).
