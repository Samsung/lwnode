{
  'variables': {
    'target_arch%': 'x64', # configure with --dest-cpu
    'target_os%': 'none',  # configure with --tizen
    'build_mode%': 'release',
    'build_host%': '<(OS)',
    'build_asan%': '0',
  },
  'target_defaults': {
    'defines': [],
    'cflags!': [ '-Wno-error' ],
    'cflags': [
      '-Wall', '-Wextra', '-Werror', '-ggdb',
      '-Wno-unused-variable',
      '-Wno-unused-function',
      '-Wno-unused-but-set-variable',
    ],
    'link_settings': {
      'libraries': [ '-ldl', '-lrt' ],
    },
    'configurations': {
      'Debug': {
        'cflags': [ '-O0' ],
      },
      'Release': {
        'defines': ['NDEBUG'],
        'cflags': [ '-Wfatal-errors', '-Os' ],
      },
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
            '-ggdb', '-Os',
            '-fPIC', '-fPIE',
            '-fstack-protector-strong',
            '-D_FORTIFY_SOURCE=2',
          ],
        },
      }],
      ['build_asan==1', {
        'cflags+':    [ '-fsanitize=address', '-fno-omit-frame-pointer', '-fno-common', '-D_FORTIFY_SOURCE=2' ],
        'cflags_cc+': [ '-fsanitize=address', '-fno-omit-frame-pointer', '-fno-common', '-D_FORTIFY_SOURCE=2' ],
        'cflags!': [ '-fomit-frame-pointer' ],
        'ldflags': [ '-fsanitize=address' ],
        'libraries': [ '-lasan' ],
      }],
    ],
  },
}
