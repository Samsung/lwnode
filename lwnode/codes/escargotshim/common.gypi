{
  'variables': {
    'host_arch%': 'x64',
    'build_mode%': 'debug',
    'target_os%': 'none',
    'escargot_os%': '<(OS)',
  },
  'target_defaults': {
    'defines': [],
    'cflags': [],
    'link_settings': {
      'libraries': [ '-ldl', '-lrt' ],
    },
    'configurations': {
      'Debug': {},
      'Release': {
        'defines': ['NDEBUG'],
      },
    }
  },
  'conditions': [],
}
