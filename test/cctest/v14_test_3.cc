/*
 * Copyright (c) 2021-present Samsung Electronics Co., Ltd
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "cctest.h"
#include "v14_test.h"
#include "v8.h"

using namespace v8;

// New internal TC
TEST(StringNewFromUtf8Literal) {
  SETUP();

  v8::TryCatch try_catch(isolate);
  CHECK_EQ(v8::String::NewFromUtf8Literal(
               isolate, "UTF8 String Test", v8::NewStringType::kNormal)
               .IsEmpty(),
           false);

  CHECK_EQ(v8::String::NewFromUtf8Literal(
               isolate, "UTF8 String Test", v8::NewStringType::kNormal)
               ->Utf8Length(isolate),
           static_cast<int>(strlen("UTF8 String Test")));

  CHECK_EQ(v8::String::NewFromUtf8Literal(
               isolate, "한글", v8::NewStringType::kNormal)
               ->Utf8Length(isolate), 6);

  CHECK_EQ(v8::String::NewFromUtf8Literal(
               isolate, "UTF8 String Test", v8::NewStringType::kNormal)
               ->Length(),
           static_cast<int>(strlen("UTF8 String Test")));

  CHECK_EQ(v8::String::NewFromUtf8Literal(
               isolate, "한글", v8::NewStringType::kNormal)
               ->Length(), 2);

  CHECK_EQ(v8::String::NewFromUtf8Literal(
               isolate, "UTF8 String Test", v8::NewStringType::kNormal)
               ->ContainsOnlyOneByte(),
           true);

  CHECK_EQ(v8::String::NewFromUtf8Literal(
               isolate, "한글", v8::NewStringType::kNormal)
               ->ContainsOnlyOneByte(),
           false);

  CHECK_EQ(v8::String::NewFromUtf8Literal(
               isolate, "UTF8 String Test", v8::NewStringType::kNormal)
               ->IsOneByte(),
           true);

  CHECK_EQ(v8::String::NewFromUtf8Literal(
               isolate, "한글", v8::NewStringType::kNormal)
               ->IsOneByte(),
           false);

  TEARDOWN();
}

// from node v8 internal
TEST(WriteOneByte) {
  SETUP();

  v8::TryCatch try_catch(isolate);

  v8::Local<String> str = v8_str("abcde");
  char buf[100];
  int len;

  memset(buf, 0x1, sizeof(buf));
  len = str->WriteOneByte(isolate, reinterpret_cast<uint8_t*>(buf));
  CHECK_EQ(5, len);
  CHECK_EQ(0, strcmp("abcde", buf));

  memset(buf, 0x1, sizeof(buf));
  len = str->WriteOneByte(isolate, reinterpret_cast<uint8_t*>(buf), 0, 4);
  CHECK_EQ(4, len);
  CHECK_EQ(0, strncmp("abcd\1", buf, 5));

  memset(buf, 0x1, sizeof(buf));
  len = str->WriteOneByte(isolate, reinterpret_cast<uint8_t*>(buf), 0, 5);
  CHECK_EQ(5, len);
  CHECK_EQ(0, strncmp("abcde\1", buf, 6));

  memset(buf, 0x1, sizeof(buf));
  len = str->WriteOneByte(isolate, reinterpret_cast<uint8_t*>(buf), 0, 6);
  CHECK_EQ(5, len);
  CHECK_EQ(0, strcmp("abcde", buf));

  memset(buf, 0x1, sizeof(buf));
  len = str->WriteOneByte(isolate, reinterpret_cast<uint8_t*>(buf), 4, -1);
  CHECK_EQ(1, len);
  CHECK_EQ(1, len);
  CHECK_EQ(0, strcmp("e", buf));

  memset(buf, 0x1, sizeof(buf));
  len = str->WriteOneByte(isolate, reinterpret_cast<uint8_t*>(buf), 4, 6);
  CHECK_EQ(1, len);
  CHECK_EQ(1, len);
  CHECK_EQ(0, strcmp("e", buf));

  memset(buf, 0x1, sizeof(buf));
  len = str->WriteOneByte(isolate, reinterpret_cast<uint8_t*>(buf), 4, 1);
  CHECK_EQ(1, len);
  CHECK_EQ(0, strncmp("e\1", buf, 2));

  memset(buf, 0x1, sizeof(buf));
  len = str->WriteOneByte(isolate, reinterpret_cast<uint8_t*>(buf), 3, 1);
  CHECK_EQ(1, len);
  CHECK_EQ(1, len);
  CHECK_EQ(0, strncmp("d\1", buf, 2));

  TEARDOWN();
}

static int StrCmp16(uint16_t* a, uint16_t* b) {
  while (true) {
    if (*a == 0 && *b == 0) return 0;
    if (*a != *b) return 0 + *a - *b;
    a++;
    b++;
  }
}

static int StrNCmp16(uint16_t* a, uint16_t* b, int n) {
  while (true) {
    if (n-- == 0) return 0;
    if (*a == 0 && *b == 0) return 0;
    if (*a != *b) return 0 + *a - *b;
    a++;
    b++;
  }
}

int GetUtf8Length(v8::Isolate* isolate, Local<String> str) {
  int len = str->Utf8Length(isolate);
//   if (len < 0) {
//     i::Isolate* i_isolate = reinterpret_cast<i::Isolate*>(isolate);
//     i::Handle<i::String> istr(v8::Utils::OpenHandle(*str));
//     i::String::Flatten(i_isolate, istr);
//     len = str->Utf8Length(isolate);
//   }
  return len;
}

// from node 14's test-api.cc
THREADED_TEST(StringWrite) {
  SETUP();

  // LocalContext context;
  // v8::Isolate* isolate = context->GetIsolate();
  // v8::HandleScope scope(isolate);
  v8::Local<String> str = v8_str("abcde");
  // abc<Icelandic eth><Unicode snowman>.
  v8::Local<String> str2 = v8_str("abc\xC3\xB0\xE2\x98\x83");
  v8::Local<String> str3 =
      v8::String::NewFromUtf8Literal(context->GetIsolate(), "abc\0def");
  // "ab" + lead surrogate + "wx" + trail surrogate + "yz"
  uint16_t orphans[8] = {0x61, 0x62, 0xD800, 0x77, 0x78, 0xDC00, 0x79, 0x7A};
  v8::Local<String> orphans_str =
      v8::String::NewFromTwoByte(context->GetIsolate(), orphans,
                                 v8::NewStringType::kNormal, 8)
          .ToLocalChecked();
  // single lead surrogate
  uint16_t lead[1] = {0xD800};
  v8::Local<String> lead_str =
      v8::String::NewFromTwoByte(context->GetIsolate(), lead,
                                 v8::NewStringType::kNormal, 1)
          .ToLocalChecked();
  // single trail surrogate
  uint16_t trail[1] = {0xDC00};
  v8::Local<String> trail_str =
      v8::String::NewFromTwoByte(context->GetIsolate(), trail,
                                 v8::NewStringType::kNormal, 1)
          .ToLocalChecked();
  // surrogate pair
  uint16_t pair[2] = {0xD800, 0xDC00};
  v8::Local<String> pair_str =
      v8::String::NewFromTwoByte(context->GetIsolate(), pair,
                                 v8::NewStringType::kNormal, 2)
          .ToLocalChecked();
  const int kStride = 4;  // Must match stride in for loops in JS below.

  // CompileRun(
  //     "var left = '';"
  //     "for (var i = 0; i < 0xD800; i += 4) {"
  //     "  left = left + String.fromCharCode(i);"
  //     "}");
  // CompileRun(
  //     "var right = '';"
  //     "for (var i = 0; i < 0xD800; i += 4) {"
  //     "  right = String.fromCharCode(i) + right;"
  //     "}");
  // v8::Local<v8::Object> global = context->Global();
  // Local<String> left_tree = global->Get(context.local(), v8_str("left"))
  //                               .ToLocalChecked()
  //                               .As<String>();
  // Local<String> right_tree = global->Get(context.local(), v8_str("right"))
  //                                .ToLocalChecked()
  //                                .As<String>();

  // CHECK_EQ(5, str2->Length());
  // CHECK_EQ(0xD800 / kStride, left_tree->Length());
  // CHECK_EQ(0xD800 / kStride, right_tree->Length());

  char buf[100];
  char utf8buf[0xD800 * 3];
  uint16_t wbuf[100];
  int len;
  int charlen;

  memset(utf8buf, 0x1, 1000);
  len = v8::String::Empty(isolate)->WriteUtf8(isolate, utf8buf, sizeof(utf8buf),
                                              &charlen);
  CHECK_EQ(1, len);
  CHECK_EQ(0, charlen);
  CHECK_EQ(0, strcmp(utf8buf, ""));

  memset(utf8buf, 0x1, 1000);
  len = str2->WriteUtf8(isolate, utf8buf, sizeof(utf8buf), &charlen);
  CHECK_EQ(9, len);
  CHECK_EQ(5, charlen);
  CHECK_EQ(0, strcmp(utf8buf, "abc\xC3\xB0\xE2\x98\x83"));

  memset(utf8buf, 0x1, 1000);
  len = str2->WriteUtf8(isolate, utf8buf, 8, &charlen);
  CHECK_EQ(8, len);
  CHECK_EQ(5, charlen);
  CHECK_EQ(0, strncmp(utf8buf, "abc\xC3\xB0\xE2\x98\x83\x01", 9));

  memset(utf8buf, 0x1, 1000);
  len = str2->WriteUtf8(isolate, utf8buf, 7, &charlen);
  CHECK_EQ(5, len);
  CHECK_EQ(4, charlen);
  CHECK_EQ(0, strncmp(utf8buf, "abc\xC3\xB0\x01", 5));

  memset(utf8buf, 0x1, 1000);
  len = str2->WriteUtf8(isolate, utf8buf, 6, &charlen);
  CHECK_EQ(5, len);
  CHECK_EQ(4, charlen);
  CHECK_EQ(0, strncmp(utf8buf, "abc\xC3\xB0\x01", 5));

  memset(utf8buf, 0x1, 1000);
  len = str2->WriteUtf8(isolate, utf8buf, 5, &charlen);
  CHECK_EQ(5, len);
  CHECK_EQ(4, charlen);
  CHECK_EQ(0, strncmp(utf8buf, "abc\xC3\xB0\x01", 5));

  memset(utf8buf, 0x1, 1000);
  len = str2->WriteUtf8(isolate, utf8buf, 4, &charlen);
  CHECK_EQ(3, len);
  CHECK_EQ(3, charlen);
  CHECK_EQ(0, strncmp(utf8buf, "abc\x01", 4));

  memset(utf8buf, 0x1, 1000);
  len = str2->WriteUtf8(isolate, utf8buf, 3, &charlen);
  CHECK_EQ(3, len);
  CHECK_EQ(3, charlen);
  CHECK_EQ(0, strncmp(utf8buf, "abc\x01", 4));

  memset(utf8buf, 0x1, 1000);
  len = str2->WriteUtf8(isolate, utf8buf, 2, &charlen);
  CHECK_EQ(2, len);
  CHECK_EQ(2, charlen);
  CHECK_EQ(0, strncmp(utf8buf, "ab\x01", 3));

  // allow orphan surrogates by default
  memset(utf8buf, 0x1, 1000);
  len = orphans_str->WriteUtf8(isolate, utf8buf, sizeof(utf8buf), &charlen);
  CHECK_EQ(13, len);
  CHECK_EQ(8, charlen);
  CHECK_EQ(0, strcmp(utf8buf, "ab\xED\xA0\x80wx\xED\xB0\x80yz"));

  // replace orphan surrogates with Unicode replacement character
  memset(utf8buf, 0x1, 1000);
  len = orphans_str->WriteUtf8(isolate, utf8buf, sizeof(utf8buf), &charlen,
                               String::REPLACE_INVALID_UTF8);
  CHECK_EQ(13, len);
  CHECK_EQ(8, charlen);
  CHECK_EQ(0, strcmp(utf8buf, "ab\xEF\xBF\xBDwx\xEF\xBF\xBDyz"));

  // replace single lead surrogate with Unicode replacement character
  memset(utf8buf, 0x1, 1000);
  len = lead_str->WriteUtf8(isolate, utf8buf, sizeof(utf8buf), &charlen,
                            String::REPLACE_INVALID_UTF8);
  CHECK_EQ(4, len);
  CHECK_EQ(1, charlen);
  CHECK_EQ(0, strcmp(utf8buf, "\xEF\xBF\xBD"));

  // replace single trail surrogate with Unicode replacement character
  memset(utf8buf, 0x1, 1000);
  len = trail_str->WriteUtf8(isolate, utf8buf, sizeof(utf8buf), &charlen,
                             String::REPLACE_INVALID_UTF8);
  CHECK_EQ(4, len);
  CHECK_EQ(1, charlen);
  CHECK_EQ(0, strcmp(utf8buf, "\xEF\xBF\xBD"));

  // do not replace / write anything if surrogate pair does not fit the buffer
  // space
  memset(utf8buf, 0x1, 1000);
  len = pair_str->WriteUtf8(isolate, utf8buf, 3, &charlen,
                            String::REPLACE_INVALID_UTF8);
  CHECK_EQ(0, len);
  CHECK_EQ(0, charlen);

  // memset(utf8buf, 0x1, sizeof(utf8buf));
  // len = GetUtf8Length(isolate, left_tree);
  // int utf8_expected =
  //     (0x80 + (0x800 - 0x80) * 2 + (0xD800 - 0x800) * 3) / kStride;
  // CHECK_EQ(utf8_expected, len);
  // len = left_tree->WriteUtf8(isolate, utf8buf, utf8_expected, &charlen);
  // CHECK_EQ(utf8_expected, len);
  // CHECK_EQ(0xD800 / kStride, charlen);
  // CHECK_EQ(0xED, static_cast<unsigned char>(utf8buf[utf8_expected - 3]));
  // CHECK_EQ(0x9F, static_cast<unsigned char>(utf8buf[utf8_expected - 2]));
  // CHECK_EQ(0xC0 - kStride,
  //          static_cast<unsigned char>(utf8buf[utf8_expected - 1]));
  // CHECK_EQ(1, utf8buf[utf8_expected]);

  // memset(utf8buf, 0x1, sizeof(utf8buf));
  // len = GetUtf8Length(isolate, right_tree);
  // CHECK_EQ(utf8_expected, len);
  // len = right_tree->WriteUtf8(isolate, utf8buf, utf8_expected, &charlen);
  // CHECK_EQ(utf8_expected, len);
  // CHECK_EQ(0xD800 / kStride, charlen);
  // CHECK_EQ(0xED, static_cast<unsigned char>(utf8buf[0]));
  // CHECK_EQ(0x9F, static_cast<unsigned char>(utf8buf[1]));
  // CHECK_EQ(0xC0 - kStride, static_cast<unsigned char>(utf8buf[2]));
  // CHECK_EQ(1, utf8buf[utf8_expected]);

  memset(buf, 0x1, sizeof(buf));
  memset(wbuf, 0x1, sizeof(wbuf));
  len = str->WriteOneByte(isolate, reinterpret_cast<uint8_t*>(buf));
  CHECK_EQ(5, len);
  len = str->Write(isolate, wbuf);
  CHECK_EQ(5, len);
  CHECK_EQ(0, strcmp("abcde", buf));
  uint16_t answer1[] = {'a', 'b', 'c', 'd', 'e', '\0'};
  CHECK_EQ(0, StrCmp16(answer1, wbuf));

  memset(buf, 0x1, sizeof(buf));
  memset(wbuf, 0x1, sizeof(wbuf));
  len = str->WriteOneByte(isolate, reinterpret_cast<uint8_t*>(buf), 0, 4);
  CHECK_EQ(4, len);
  len = str->Write(isolate, wbuf, 0, 4);
  CHECK_EQ(4, len);
  CHECK_EQ(0, strncmp("abcd\x01", buf, 5));
  uint16_t answer2[] = {'a', 'b', 'c', 'd', 0x101};
  CHECK_EQ(0, StrNCmp16(answer2, wbuf, 5));

  memset(buf, 0x1, sizeof(buf));
  memset(wbuf, 0x1, sizeof(wbuf));
  len = str->WriteOneByte(isolate, reinterpret_cast<uint8_t*>(buf), 0, 5);
  CHECK_EQ(5, len);
  len = str->Write(isolate, wbuf, 0, 5);
  CHECK_EQ(5, len);
  CHECK_EQ(0, strncmp("abcde\x01", buf, 6));
  uint16_t answer3[] = {'a', 'b', 'c', 'd', 'e', 0x101};
  CHECK_EQ(0, StrNCmp16(answer3, wbuf, 6));

  memset(buf, 0x1, sizeof(buf));
  memset(wbuf, 0x1, sizeof(wbuf));
  len = str->WriteOneByte(isolate, reinterpret_cast<uint8_t*>(buf), 0, 6);
  CHECK_EQ(5, len);
  len = str->Write(isolate, wbuf, 0, 6);
  CHECK_EQ(5, len);
  CHECK_EQ(0, strcmp("abcde", buf));
  uint16_t answer4[] = {'a', 'b', 'c', 'd', 'e', '\0'};
  CHECK_EQ(0, StrCmp16(answer4, wbuf));

  memset(buf, 0x1, sizeof(buf));
  memset(wbuf, 0x1, sizeof(wbuf));
  len = str->WriteOneByte(isolate, reinterpret_cast<uint8_t*>(buf), 4, -1);
  CHECK_EQ(1, len);
  len = str->Write(isolate, wbuf, 4, -1);
  CHECK_EQ(1, len);
  CHECK_EQ(0, strcmp("e", buf));
  uint16_t answer5[] = {'e', '\0'};
  CHECK_EQ(0, StrCmp16(answer5, wbuf));

  memset(buf, 0x1, sizeof(buf));
  memset(wbuf, 0x1, sizeof(wbuf));
  len = str->WriteOneByte(isolate, reinterpret_cast<uint8_t*>(buf), 4, 6);
  CHECK_EQ(1, len);
  len = str->Write(isolate, wbuf, 4, 6);
  CHECK_EQ(1, len);
  CHECK_EQ(0, strcmp("e", buf));
  CHECK_EQ(0, StrCmp16(answer5, wbuf));

  memset(buf, 0x1, sizeof(buf));
  memset(wbuf, 0x1, sizeof(wbuf));
  len = str->WriteOneByte(isolate, reinterpret_cast<uint8_t*>(buf), 4, 1);
  CHECK_EQ(1, len);
  len = str->Write(isolate, wbuf, 4, 1);
  CHECK_EQ(1, len);
  CHECK_EQ(0, strncmp("e\x01", buf, 2));
  uint16_t answer6[] = {'e', 0x101};
  CHECK_EQ(0, StrNCmp16(answer6, wbuf, 2));

  memset(buf, 0x1, sizeof(buf));
  memset(wbuf, 0x1, sizeof(wbuf));
  len = str->WriteOneByte(isolate, reinterpret_cast<uint8_t*>(buf), 3, 1);
  CHECK_EQ(1, len);
  len = str->Write(isolate, wbuf, 3, 1);
  CHECK_EQ(1, len);
  CHECK_EQ(0, strncmp("d\x01", buf, 2));
  uint16_t answer7[] = {'d', 0x101};
  CHECK_EQ(0, StrNCmp16(answer7, wbuf, 2));

  memset(wbuf, 0x1, sizeof(wbuf));
  wbuf[5] = 'X';
  len = str->Write(isolate, wbuf, 0, 6, String::NO_NULL_TERMINATION);
  CHECK_EQ(5, len);
  CHECK_EQ('X', wbuf[5]);
  uint16_t answer8a[] = {'a', 'b', 'c', 'd', 'e'};
  uint16_t answer8b[] = {'a', 'b', 'c', 'd', 'e', '\0'};
  CHECK_EQ(0, StrNCmp16(answer8a, wbuf, 5));
  CHECK_NE(0, StrCmp16(answer8b, wbuf));
  wbuf[5] = '\0';
  CHECK_EQ(0, StrCmp16(answer8b, wbuf));

  memset(buf, 0x1, sizeof(buf));
  buf[5] = 'X';
  len = str->WriteOneByte(isolate, reinterpret_cast<uint8_t*>(buf), 0, 6,
                          String::NO_NULL_TERMINATION);
  CHECK_EQ(5, len);
  CHECK_EQ('X', buf[5]);
  CHECK_EQ(0, strncmp("abcde", buf, 5));
  CHECK_NE(0, strcmp("abcde", buf));
  buf[5] = '\0';
  CHECK_EQ(0, strcmp("abcde", buf));

  memset(utf8buf, 0x1, sizeof(utf8buf));
  utf8buf[8] = 'X';
  len = str2->WriteUtf8(isolate, utf8buf, sizeof(utf8buf), &charlen,
                        String::NO_NULL_TERMINATION);
  CHECK_EQ(8, len);
  CHECK_EQ('X', utf8buf[8]);
  CHECK_EQ(5, charlen);
  CHECK_EQ(0, strncmp(utf8buf, "abc\xC3\xB0\xE2\x98\x83", 8));
  CHECK_NE(0, strcmp(utf8buf, "abc\xC3\xB0\xE2\x98\x83"));
  utf8buf[8] = '\0';
  CHECK_EQ(0, strcmp(utf8buf, "abc\xC3\xB0\xE2\x98\x83"));

  memset(utf8buf, 0x1, sizeof(utf8buf));
  utf8buf[5] = 'X';
  len = str->WriteUtf8(isolate, utf8buf, sizeof(utf8buf), &charlen,
                       String::NO_NULL_TERMINATION);
  CHECK_EQ(5, len);
  CHECK_EQ('X', utf8buf[5]);  // Test that the sixth character is untouched.
  CHECK_EQ(5, charlen);
  utf8buf[5] = '\0';
  CHECK_EQ(0, strcmp(utf8buf, "abcde"));

  memset(buf, 0x1, sizeof(buf));
  len = str3->WriteOneByte(isolate, reinterpret_cast<uint8_t*>(buf));
  CHECK_EQ(7, len);
  CHECK_EQ(0, strcmp("abc", buf));
  CHECK_EQ(0, buf[3]);
  CHECK_EQ(0, strcmp("def", buf + 4));

  CHECK_EQ(0, str->WriteOneByte(isolate, nullptr, 0, 0,
                                String::NO_NULL_TERMINATION));
  CHECK_EQ(0, str->WriteUtf8(isolate, nullptr, 0, nullptr,
                             String::NO_NULL_TERMINATION));
  CHECK_EQ(0, str->Write(isolate, nullptr, 0, 0, String::NO_NULL_TERMINATION));

  TEARDOWN();
}
