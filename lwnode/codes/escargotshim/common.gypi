{
  'variables': {
    'target_arch%': 'x64', # configure with --dest-cpu
    'target_os%': 'none',  # configure with --tizen
    'build_mode%': 'debug',
    'build_host%': '<(OS)',
  },
  'target_defaults': {
    'defines': [],
    'cflags': [
      '-Wall', '-Wextra', '-Wno-error',
      '-Wno-unused-variable',
      '-Wno-unused-function',
      '-Wno-unused-but-set-variable',
    ],
    'link_settings': {
      'libraries': [ '-ldl', '-lrt' ],
    },
    'configurations': {
      'Debug': {
        'cflags': [ '-g', '-O0' ],
      },
      'Release': {
        'defines': ['NDEBUG'],
        'cflags': [ '-Wfatal-errors' ],
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
          '-g', '-O0',
          '-fPIC', '-fPIE',
          '-fstack-protector-strong',
          '-D_FORTIFY_SOURCE=2',
        ],
      },
    }],
  ],
}
