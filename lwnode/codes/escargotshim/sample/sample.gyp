{
  'includes': ['../common.gypi'],
  'targets': [
    {
      'target_name': 'sample',
      'type': 'executable',
      'dependencies': [
        '../escargot.gyp:escargot',
        '../escargotshim.gyp:escargotshim',
       ],
      'cflags_cc': [
        '-Wno-unused-parameter',
      ],
      'sources': [
        'hello-world.cc',
        'escargot-sample.cc',
      ]
    },
  ],
}
