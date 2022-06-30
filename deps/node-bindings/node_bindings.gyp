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
      'defines': [
      ],
      'includes': [
      ],
      'include_dirs': [
        'include',
        '../../deps/node/deps/uv/include'
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
