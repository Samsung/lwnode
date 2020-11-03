{
  'includes': ['../common.gypi'],
  'targets': [
    {
      'target_name': 'escargotshim_sample',
      'type': 'executable',
      'dependencies': [
        '../escargot.gyp:escargot',
        '../escargotshim.gyp:escargotshim',
       ],
      'sources': [
        'hello-world.cc',
        'escargot-sample.cc',
      ]
    },
  ],
}
