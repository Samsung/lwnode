{
  'variables': {
  },
  'includes': [
    '../../common.gypi'
  ],
  'targets': [
    {
      'target_name': 'node_bindings',
      'type': 'static_library',
      'dependencies': [
        '../../../../../deps/uv/uv.gyp:libuv',
       ],
      'defines': [
      ],
      'includes': [
      ],
      'include_dirs': [
        'include',
      ],
      'sources': [
        'src/gmainloop_node_bindings.cc'
      ],
      'cflags': [
        '<!@(pkg-config --cflags glib-2.0)',
        '-Wno-missing-field-initializers',
      ],
      'libraries': [
        '<!@(pkg-config --libs glib-2.0)',
      ],
      'all_dependent_settings': {
        'defines': [
        ],
        'include_dirs': [
          'include',
        ],
        'cflags': [
          '<!@(pkg-config --cflags glib-2.0)',
        ],
        'cflags_cc!': ['-fno-exceptions'],
        'cflags_cc': [
          '-fexceptions',
        ],
        'libraries': [
          '<!@(pkg-config --libs glib-2.0)',
        ],
      }
    },
  ],
  'conditions': [
  ],
}
