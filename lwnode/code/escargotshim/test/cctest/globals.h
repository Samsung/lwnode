#ifndef __ESCARGOTSHIM_GLOBALS__
#define __ESCARGOTSHIM_GLOBALS__

// FUNCTION_ADDR(f) gets the address of a C function f.
#define FUNCTION_ADDR(f)                                                       \
  (reinterpret_cast<v8::internal::Address>(reinterpret_cast<intptr_t>(f)))

namespace v8 {

namespace internal {

typedef uint8_t byte;
// typedef byte* Address;

template <typename T>
T* NewArray(size_t size) {
  T* result = new T[size];
  // TODO(gwolny) implement out of memory handler
  // if (result == NULL) FatalProcessOutOfMemory("NewArray");
  return result;
}

template <typename T>
void DeleteArray(T* array) {
  delete[] array;
}

static char* StrDup(const char* str) {
  int len = strlen(str);
  char* buf = new char[len + 1];
  strncpy(buf, str, len);
  buf[len] = '\0';
  return buf;
}

}  // namespace internal
}  // namespace v8

namespace i = v8::internal;

#endif
