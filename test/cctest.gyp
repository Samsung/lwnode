{
  'includes': ['../common.gypi'],
  'targets': [
    {
      'target_name': 'cctest',
      'type': 'executable',
      'dependencies': [
        './cctest/gtest/gtest.gyp:gtest',
        '../escargotshim.gyp:escargotshim',
        # for using gc internals
        '../escargot.gyp:escargot',
       ],
      'defines': [
         'GTEST_DONT_DEFINE_TEST=1',
         'CCTEST_ENGINE_ESCARGOT=1',
       ],
      'cflags_cc': [
        '-Wno-unused-parameter',
        '-Wno-unused-result',
        '-Wno-comment',
        '-Wno-sign-compare',
        '-Wno-cast-function-type',
        '-Wno-maybe-uninitialized',
        '-std=c++14',
      ],
      'include_dirs': [
        './cctest',
        '../src',
      ],
      'sources': [
        'cctest/cctest.cc',
        'cctest/v14_test_1.cc',
        'cctest/v14_test_2.cc',
        'cctest/v14_test_3.cc',
        'cctest/v14_test-serialize.cc',
        'cctest/test-api.cc',
        'cctest/test-internal.cc',
        'cctest/test-strings.cc',
      ]
    },
  ],
}
