#!/usr/bin/env python
#
# Copyright 2006-2008 the V8 project authors. All rights reserved.
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:
#
#     * Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above
#       copyright notice, this list of conditions and the following
#       disclaimer in the documentation and/or other materials provided
#       with the distribution.
#     * Neither the name of Google Inc. nor the names of its
#       contributors may be used to endorse or promote products derived
#       from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

# This is a utility for converting JavaScript source code into C-style
# char arrays. It is used for embedded JavaScript code in the V8
# library.

import os
import re
import sys
import string

def ToCArray(elements, step=10):
  slices = (elements[i:i+step] for i in xrange(0, len(elements), step))
  slices = map(lambda s: ','.join(str(x) for x in s), slices)
  return ',\n'.join(slices)


def ToCString(contents):
  return ToCArray(map(ord, contents), step=20)


def ReadFile(filename):
  file = open(filename, "rt")
  try:
    lines = file.read()
  finally:
    file.close()
  return lines


def ReadLines(filename):
  result = []
  for line in open(filename, "rt"):
    if '#' in line:
      line = line[:line.index('#')]
    line = line.strip()
    if len(line) > 0:
      result.append(line)
  return result


def ExpandConstants(lines, constants):
  for key, value in constants.items():
    lines = lines.replace(key, str(value))
  return lines


def ExpandMacros(lines, macros):
  for name, macro in macros.items():
    start = lines.find(name + '(', 0)
    while start != -1:
      # Scan over the arguments
      assert lines[start + len(name)] == '('
      height = 1
      end = start + len(name) + 1
      last_match = end
      arg_index = 0
      mapping = { }
      def add_arg(str):
        # Remember to expand recursively in the arguments
        replacement = ExpandMacros(str.strip(), macros)
        mapping[macro.args[arg_index]] = replacement
      while end < len(lines) and height > 0:
        # We don't count commas at higher nesting levels.
        if lines[end] == ',' and height == 1:
          add_arg(lines[last_match:end])
          last_match = end + 1
        elif lines[end] in ['(', '{', '[']:
          height = height + 1
        elif lines[end] in [')', '}', ']']:
          height = height - 1
        end = end + 1
      # Remember to add the last match.
      add_arg(lines[last_match:end-1])
      result = macro.expand(mapping)
      # Replace the occurrence of the macro with the expansion
      lines = lines[:start] + result + lines[end:]
      start = lines.find(name + '(', start)
  return lines


class TextMacro:
  def __init__(self, args, body):
    self.args = args
    self.body = body
  def expand(self, mapping):
    result = self.body
    for key, value in mapping.items():
        result = result.replace(key, value)
    return result

class PythonMacro:
  def __init__(self, args, fun):
    self.args = args
    self.fun = fun
  def expand(self, mapping):
    args = []
    for arg in self.args:
      args.append(mapping[arg])
    return str(self.fun(*args))

CONST_PATTERN = re.compile('^const\s+([a-zA-Z0-9_]+)\s*=\s*([^;]*);$')
MACRO_PATTERN = re.compile('^macro\s+([a-zA-Z0-9_]+)\s*\(([^)]*)\)\s*=\s*([^;]*);$')
PYTHON_MACRO_PATTERN = re.compile('^python\s+macro\s+([a-zA-Z0-9_]+)\s*\(([^)]*)\)\s*=\s*([^;]*);$')

def ReadMacros(lines):
  constants = { }
  macros = { }
  for line in lines:
    hash = line.find('#')
    if hash != -1: line = line[:hash]
    line = line.strip()
    if len(line) is 0: continue
    const_match = CONST_PATTERN.match(line)
    if const_match:
      name = const_match.group(1)
      value = const_match.group(2).strip()
      constants[name] = value
    else:
      macro_match = MACRO_PATTERN.match(line)
      if macro_match:
        name = macro_match.group(1)
        args = map(string.strip, macro_match.group(2).split(','))
        body = macro_match.group(3).strip()
        macros[name] = TextMacro(args, body)
      else:
        python_match = PYTHON_MACRO_PATTERN.match(line)
        if python_match:
          name = python_match.group(1)
          args = map(string.strip, python_match.group(2).split(','))
          body = python_match.group(3).strip()
          fun = eval("lambda " + ",".join(args) + ': ' + body)
          macros[name] = PythonMacro(args, fun)
        else:
          raise Exception("Illegal line: " + line)
  return (constants, macros)


TEMPLATE_BASE = """
#include "v8.h"
#include "v8utils.h"
#include "escargotutil.h"

#include <sys/types.h>
#include <unistd.h>
#include <limits.h>
#include <string>
"""

TEMPLATE_EXTERNAL_BASE = """
static void* loadBuiltinSource(void* data)
{{
  char pathDest[PATH_MAX];
  memset(pathDest,0,sizeof(pathDest));
  if (readlink("/proc/self/exe", pathDest, PATH_MAX) == -1) {{
    NESCARGOT_ASSERT_SHOULD_NOT_BE_HERE();
  }}

  std::string filePath = pathDest;
  filePath = filePath.substr(0, filePath.rfind('/') + 1);
  filePath += (const char*)data;

  FILE* fp = fopen(filePath.data(), "rb");
  if (!fp) {{
    fprintf(stderr, "can not open %s\\n", filePath.data());
    NESCARGOT_ASSERT_SHOULD_NOT_BE_HERE();
  }}
  fseek(fp, 0, SEEK_END);
  auto fsize = ftell(fp);
  fseek(fp, 0, SEEK_SET);

  void *dest = malloc(fsize);
  auto size = fread(dest, 1, fsize, fp);
  NESCARGOT_ASSERT((long)size == (long)fsize);
  fclose(fp);

  return dest;
}}

static void unloadBuiltinSource(void* memoryPtr, void* callbackData)
{{
  free(memoryPtr);
}}
"""

TEMPLATE_FOR_NAMESPACE = """
namespace {namespace} {{
{definitions}
}} // namespace {namespace}
"""

TEMPLATE = """
#include "node.h"
#include "node_javascript.h"
#include "env.h"
#include "env-inl.h"

namespace node {{

{definitions}

v8::Local<v8::String> MainSource(Environment* env) {{
  return internal_bootstrap_node_value.ToStringChecked(env->isolate());
}}

void DefineJavaScript(Environment* env, v8::Local<v8::Object> target) {{
  {initializers}
}}

}}  // namespace node
"""

ONE_BYTE_STRING = """
static const uint8_t raw_{var}[] = {{ {data} }};
static struct : public v8::String::ExternalOneByteStringResource {{
  const char* data() const override {{
    return reinterpret_cast<const char*>(raw_{var});
  }}
  size_t length() const override {{ return sizeof(raw_{var}); }}
  void Dispose() override {{ /* Default calls `delete this`. */ }}
  v8::Local<v8::String> ToStringChecked(v8::Isolate* isolate) {{
    return v8::String::NewExternalOneByte(isolate, this).ToLocalChecked();
  }}
}} {var};
"""

ONE_BYTE_STRING_EXTERNAL_SOURCE = """
static struct : public v8::String::ExternalOneByteStringResource {{
  const char* data() const override {{
    NESCARGOT_ASSERT_SHOULD_NOT_BE_HERE();
    return nullptr;
  }}
  size_t length() const override {{ NESCARGOT_ASSERT_SHOULD_NOT_BE_HERE(); return 0; }}
  void Dispose() override {{ /* Default calls `delete this`. */ }}
  v8::Local<v8::String> ToStringChecked(v8::Isolate* isolate) {{
    auto vmInstance = EscargotShim::IsolateShim::FromIsolate(isolate)->VMInstanceRef();
    auto jsStringRef = Escargot::StringRef::createReloadableString(vmInstance, true,
      {source_length}, (void*)("{source_filename}"), loadBuiltinSource, unloadBuiltinSource);
    return v8::Utils::ToLocal<v8::String>(reinterpret_cast<v8::String*>(jsStringRef));
  }}
}} {var};
"""

TWO_BYTE_STRING = """
static const uint16_t raw_{var}[] = {{ {data} }};
static struct : public v8::String::ExternalStringResource {{
  const uint16_t* data() const override {{ return raw_{var}; }}
  size_t length() const override {{ return sizeof(raw_{var}) / 2; }}
  void Dispose() override {{ /* Default calls `delete this`. */ }}
  v8::Local<v8::String> ToStringChecked(v8::Isolate* isolate) {{
    return v8::String::NewExternalTwoByte(isolate, this).ToLocalChecked();
  }}
}} {var};
"""

TWO_BYTE_STRING_EXTERNAL_SOURCE = """
static struct : public v8::String::ExternalStringResource {{
   const uint16_t* data() const override {{
    NESCARGOT_ASSERT_SHOULD_NOT_BE_HERE();
    return nullptr;
  }}
  size_t length() const override {{ NESCARGOT_ASSERT_SHOULD_NOT_BE_HERE(); return 0; }}
  void Dispose() override {{ /* Default calls `delete this`. */ }}
  v8::Local<v8::String> ToStringChecked(v8::Isolate* isolate) {{
    auto vmInstance = EscargotShim::IsolateShim::FromIsolate(isolate)->VMInstanceRef();
    auto jsStringRef = Escargot::StringRef::createReloadableString(vmInstance, false,
      {source_length}, (void*)("{source_filename}"), loadBuiltinSource, unloadBuiltinSource);
    return v8::Utils::ToLocal<v8::String>(reinterpret_cast<v8::String*>(jsStringRef));
  }}
}} {var};
"""

INITIALIZER = """\
CHECK(target->Set(env->context(),
                  {key}.ToStringChecked(env->isolate()),
                  {value}.ToStringChecked(env->isolate())).FromJust());
"""

# Taken from https://stackoverflow.com/a/600612/119527
def mkdir_p(path):
  import os, os.path
  import errno

  try:
    os.makedirs(path)
  except OSError as exc: # Python >2.5
    if exc.errno == errno.EEXIST and os.path.isdir(path):
      pass
    else: raise

def safe_open_w(path, mode="w"):
  ''' Open "path" for writing, creating any parent directories as needed.
  '''
  mkdir_p(os.path.dirname(path))
  return open(path, mode)

def Render(var, data, source_filename, is_js_source):
  # Treat non-ASCII as UTF-8 and convert it to UTF-16.
  if any(ord(c) > 127 for c in data):
    if enable_external_script is True and is_js_source is True:
      template = TWO_BYTE_STRING_EXTERNAL_SOURCE
    else:
      template = TWO_BYTE_STRING
    org_data = data
    data = map(ord, org_data.decode('utf-8').encode('utf-16be'))
    data = [data[i] * 256 + data[i+1] for i in xrange(0, len(data), 2)]
    data = ToCArray(data)
    raw_data = map(ord, org_data.decode('utf-8').encode('utf-16be'))
    raw_data = [raw_data[i] * 256 + raw_data[i+1] for i in xrange(0, len(raw_data), 2)]
  else:
    if enable_external_script is True and is_js_source is True:
      template = ONE_BYTE_STRING_EXTERNAL_SOURCE
    else:
      template = ONE_BYTE_STRING
    raw_data = data
    data = ToCString(data)

  if enable_external_script is True and is_js_source is True:
    source_file_path = output_path + '/builtin/' + source_filename
    try:
      if isinstance(raw_data, str):
        builtin_file = safe_open_w(source_file_path)
        builtin_file.write(raw_data)
        builtin_file.close()
      else:
        import struct
        builtin_file = safe_open_w(source_file_path, "wb")
        for item in raw_data:
          builtin_file.write(struct.pack("<h", item))
        builtin_file.close()
    except Exception as e:
      print(e)

  return template.format(var=var, raw_data=raw_data, data=data, source_filename='builtin/' + source_filename, source_length=len(raw_data))



NODE_NATIVES_MAP = """\
  V({escaped_id}) \\
"""


SOURCES = """\
static const uint8_t {escaped_id}_name[] = {{
{name}}};
static const uint8_t {escaped_id}_data[] = {{
{data}}};
"""

def JS2C(source, target, namespace):
  modules = []
  consts = {}
  macros = {}
  macro_lines = []

  for s in source:
    if (os.path.split(str(s))[1]).endswith('macros.py'):
      macro_lines.extend(ReadLines(str(s)))
    else:
      modules.append(s)

  # Process input from all *macro.py files
  (consts, macros) = ReadMacros(macro_lines)

  # Build source code lines
  definitions = []
  initializers = []

  for name in modules:
    lines = ReadFile(str(name))
    lines = ExpandConstants(lines, consts)
    lines = ExpandMacros(lines, macros)

    lines = lines.splitlines()
    new_lines = []
    for line in lines:
        if line.strip().startswith("//") is False and len(line) is not 0:
            new_lines.append(line)

    lines = '\n'.join(new_lines)

    source_filename = str(name)
    # On Windows, "./foo.bar" in the .gyp file is passed as "foo.bar"
    # so don't assume there is always a slash in the file path.
    if '/' in name or '\\' in name:
      name = '/'.join(re.split('/|\\\\', name)[1:])

    name = name.split('.', 1)[0]
    var = name.replace('-', '_').replace('/', '_')
    key = '%s_key' % var
    value = '%s_value' % var

    definitions.append(Render(key, name, source_filename, False))
    definitions.append(Render(value, lines, source_filename, True))

    initializers.append(INITIALIZER.format(key=key, value=value))

  # Emit result
  output = open(str(target[0]), "w")
  output.write(HEADER_TEMPLATE.format(definitions=''.join(definitions),
                               initializers=''.join(initializers),
                               namespace=namespace))
  output.close()


NAMESPACE_SWITCH = "--namespace="
OUTPUT_PATH_SWITCH = "--output-path="
ENABLE_EXTERNAL_SCRIPT_SWITCH = "--enable-external-builtin-scripts"

def main():
  global HEADER_TEMPLATE
  global enable_external_script
  global output_path
  enable_external_script = False
  output_path = ""
  i = 1
  namespace = 'node'

  if sys.argv[i].startswith(ENABLE_EXTERNAL_SCRIPT_SWITCH):
    enable_external_script = True
    i += 1

  if enable_external_script is True:
    HEADER_TEMPLATE = TEMPLATE_BASE + TEMPLATE_EXTERNAL_BASE + TEMPLATE
  else:
    HEADER_TEMPLATE = TEMPLATE_BASE + TEMPLATE

  if sys.argv[i].startswith(OUTPUT_PATH_SWITCH):
    output_path = sys.argv[i][len(OUTPUT_PATH_SWITCH):]
    i += 1

  if sys.argv[i].startswith(NAMESPACE_SWITCH):
    if enable_external_script is True:
      HEADER_TEMPLATE = TEMPLATE_BASE + TEMPLATE_EXTERNAL_BASE + TEMPLATE_FOR_NAMESPACE
    else:
      HEADER_TEMPLATE = TEMPLATE_BASE + TEMPLATE_FOR_NAMESPACE
    namespace = sys.argv[i][len(NAMESPACE_SWITCH):]
    i += 1

  natives = sys.argv[i]
  source_files = sys.argv[(i + 1):]
  JS2C(source_files, [natives], namespace)

if __name__ == "__main__":
  main()
