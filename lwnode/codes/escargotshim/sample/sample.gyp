{
  'variables': {
  },

  'targets': [
    {
      'target_name': 'escargotshim_sample',
      'type': 'executable',
      'dependencies': [
        './../escargotshim.gyp:escargotshim',
       ],
      'includes': [ './../escargotshim.gypi'],
      'include_dirs': [
        '<@(escargot_include_dir)',
        '<@(escargotshim_include_dir)',
      ],
      'sources': [
        'hello-world.cc',
        'escargot-sample.cc',
      ],
      'libraries': [
          '<(OBJ_DIR)/deps/escargotshim/libescargot.a',
      ],
    },
  ],
}
