{
  'variables': {
    'target_arch%': 'x64', # configure with --dest-cpu
    'target_os%': 'none',  # configure with --tizen
    'build_host%': '<(OS)',
    'asan%': '0',
  },
  'target_defaults': {
    'defines': [ 'LWNODE=1' ],
    'cflags!': [ '-Wno-error' ],
    'cflags': [
      '-Wall', '-Wextra', '-Werror',
      '-Wno-unused-variable',
      '-Wno-unused-function',
      '-Wno-unused-but-set-variable',
      '-fPIC',
      '-ggdb', # all builds include debug symbols, which will be stripped before packaging
      '-fdata-sections',     # for gc-sections
      '-ffunction-sections', # for gc-sections
    ],
    'ldflags': [
      '-Wl,--gc-sections',
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
    'all_dependent_settings': {
      'configurations': {
        'Release': {
          'defines': ['NDEBUG'],
        },
      },
    },
    'conditions': [
      ['target_os=="tizen"', {
        'target_defaults': {
          'defines': ['HOST_TIZEN'],
          'cflags': [
            '-Os',
            '-fPIE',
            '-fstack-protector-strong',
            '-D_FORTIFY_SOURCE=2',
          ],
          'ldflags': [
            '-pie',
            '-Wl,-z,relro,-z,now',
          ],
        },
      }],
      ['asan==1', {
        'cflags+':    [ '-fsanitize=address', '-fno-omit-frame-pointer', '-fno-common', '-D_FORTIFY_SOURCE=2' ],
        'cflags_cc+': [ '-fsanitize=address', '-fno-omit-frame-pointer', '-fno-common', '-D_FORTIFY_SOURCE=2' ],
        'cflags!': [ '-fomit-frame-pointer' ],
        'ldflags': [ '-fsanitize=address' ],
        'libraries': [ '-lasan' ],
      }],
    ],
  },
}
