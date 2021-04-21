# Conventions



--------

### directives

* **SHALL** is used to express mandatory requirements. (negative form: **SHALL NOT**)
  * Do not use MUST as an alternative to SHALL.
* **SHOULD** is used to express recommendations. (negative form: **SHOULD NOT**)
* **MAY** is used to express permissions .  (negative form: **NEED NOT**)
  * do not use CAN as an alternative to MAY.

---------



## Naming

- If a corresponding class or a simlar concept exists in v8, name it with a postfix, `Wrap`.
```
  - Context (v8)
  - ContextWrap (lwnode)
  - Isolate (v8)
  - IsolateWrap (lwnode)
```

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

* API Levels
  * API level 1 : v8 apis
  * API level 2 : EscargotShim apis (Wrapped Values)
  * API level 3 : Escargot apis
* Regarding Variable Names,
* In Level 1, if there are same concepts among the API levels, it **SHOULD** use the following conventions:
  * name a value of API level 3 (Escargot Value) with `es` .
  * name a value of API level 2 (Wrapped Value) with `lw`.
  * name v8 Value with no prefix or `v8`.
  * It **SHALL** use the above especially when using `auto` type inference.
* In Level 3, it **NEEDS NOT** to meet the above conventions.



## Sources

### Source regions

|       Level 0       |        Level 1         |         Level 2         |       Level 3        |
| :-----------------: | :--------------------: | :---------------------: | :------------------: |
| User apps (Node.js) | v8 apis (`src/api.cc`) | lwnode apis (`src/api`) | js engine (escargot) |

* In the level 1, `ValueWrap` **SHALL** be used for type convertion between v8 and lwnode apis.



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
  - Class data member (TBD)
    - 1) m_value
    - 2) value_ (node, v8)
  - Common data (TBD)
    - 1) stringValue
    - 2) string_value (v8)
- File Names
  - Filenames should be all lowercase and may include dashes (-).

