{
  'includes': ['common.gypi'],
  'variables': {
    'source_dir%': 'deps/message-port',
  },
  'targets': [
    {
      'target_name': 'nd-es-utils',
      'type': 'static_library',
      'include_dirs': [
        '<(source_dir)/nd',
      ],
      'dependencies': ['escargot.gyp:escargot'],
      'cflags_cc': [
        '-fexceptions',
        '-Wno-maybe-uninitialized', # EscargotPublic.h:366:24
      ],
      'sources': [
        '<(source_dir)/nd/es.cc',
        '<(source_dir)/nd/es-helper.cc',
        '<(source_dir)/nd/nd-debug.cc',
        '<(source_dir)/nd/nd-logger.cc',
      ],
      'all_dependent_settings': {
        'include_dirs': [
          '<(source_dir)/nd',
          '<(source_dir)/nd/utils',
        ],
      },
    },
    {
      'target_name': 'nd-message-port',
      'type': 'static_library',
      'dependencies': ['nd-es-utils'],
      'defines': [],
      'cflags_cc': [
        '-fexceptions',
        '-Wno-unused-parameter',
        '-Wno-maybe-uninitialized',
      ],
      'include_dirs': [
        '<(source_dir)'
        '/../node/deps/uv/include'
      ],
      'sources': [
        '<(source_dir)/message-port.cc',
        '<(source_dir)/async-uv.cc',
        '<(source_dir)/debug-mem-trace.cc',
      ],
      'all_dependent_settings': {
        'include_dirs': [
          '<(source_dir)/',
          'deps/node/deps/uv/include'
        ],
      },
    },
  ]
}
