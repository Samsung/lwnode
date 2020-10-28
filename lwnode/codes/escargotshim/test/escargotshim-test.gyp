{
  'variables': {
  },
  
  'targets': [
    {
      'target_name': 'escargotshim_cctest',
      'type': 'executable',
      'dependencies': [
        '../escargotshim.gyp:escargotshim',
        '../../gtest/gtest.gyp:gtest',
       ],
       'defines': [
         'GTEST_DONT_DEFINE_TEST=1',
       ],
      'ldflags': ['-Wno-comment'],
      'includes': [ '../escargotshim.gypi'],
      'include_dirs': [
        '<@(escargot_include_dir)',
        '<@(escargotshim_include_dir)',
        './',
      ],
      'sources': [
        'cctest/cctest.cc',
        'cctest/test-api.cc',
        'cctest/internaltest.cc',
      ],
      'libraries': [
          '<(OBJ_DIR)/deps/escargotshim/libescargot.a',
      ],
    },
  ],
}
