{
  'variables': {
    'target_arch%': 'x64', # configure with --dest-cpu
    'target_os%': 'none',  # configure with --tizen
    'build_mode%': 'debug',
    'build_host%': '<(OS)',
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
  'conditions': [
    ['target_os=="tizen"', {
      'target_defaults': {
        'defines': [
          'HOST_TIZEN',
        ],
        'ldflags': [
          '-mthumb',
          '-pie',
          '-Wl,-z,relro,-z,now',
        ],
        'cflags': [
          '-fPIC', '-fPIE',
          '-fstack-protector-strong',
          '-D_FORTIFY_SOURCE=2',
        ],
      },
    }],
  ],
}
