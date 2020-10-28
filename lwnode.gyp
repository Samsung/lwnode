{
  'variables': {
  },

  'targets': [
    {
      'target_name': 'lwnode',
      'type': 'executable',
      'dependencies': [
        'node.gyp:<(node_lib_target_name)',
       ],
      'defines': [
        'NODE_ARCH="<(target_arch)"',
        'NODE_PLATFORM="<(OS)"',
        'NODE_WANT_INTERNALS=1',
      ],
      'includes': [
        'common.gypi',
        'config.gypi',
        'node.gypi',
        'node.gyp',
      ],
      'include_dirs': [
        'src',
        'deps/v8/include',
      ],
      'sources': [
        'src/node_main.cc'
      ],
      'libraries': [
      ],
    },
  ],
  'conditions': [
  ],
}
