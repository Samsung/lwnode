{
  'targets': [
    {
      'target_name': 'cctest_v8',
      'type': 'executable',
      'dependencies': [
        './cctest/gtest/gtest.gyp:gtest',
        '../../../../tools/v8_gypfiles/v8.gyp:v8'
       ],
      'defines': [
         'GTEST_DONT_DEFINE_TEST=1',
         'CCTEST_ENGINE_V8=1',
       ],
      'cflags_cc': [
        '-Wno-unused-parameter',
        '-Wno-comment',
        '-Wno-sign-compare',
        '-std=gnu++11',
      ],
      'include_dirs': [
        './cctest',
        '../src',
      ],
      'sources': [
        'cctest/cctest.cc',
        # 'cctest/v14_test_1.cc',
        'cctest/v14_test_2.cc',
        # 'cctest/v14_test_3.cc',
        'cctest/test-api.cc',
      ]
    },
  ],
}
