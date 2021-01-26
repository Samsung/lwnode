{
  'includes': ['../common.gypi'],
  'targets': [
    {
      'target_name': 'cctest',
      'type': 'executable',
      'dependencies': [
        './cctest/gtest/gtest.gyp:gtest',
        '../escargotshim.gyp:escargotshim',
       ],
      'defines': [
         'GTEST_DONT_DEFINE_TEST=1',
       ],
      'cflags_cc': [
        '-Wno-unused-parameter',
      ],
      'include_dirs': [
        './cctest'
      ],
      'sources': [
        'cctest/cctest.cc',
        'cctest/v14_test_1.cc',
      ]
    },
  ],
}
