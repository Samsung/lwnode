{
  'includes': ['common.gypi'],
  'variables': {
    'escargot_dir%': 'deps/escargot',
    "escargot_lib_type%": 'shared_lib', # static_lib | shared_lib
    'build_asan%': '<(build_asan)',
    'target_arch%': '<(target_arch)',
#    'escargot_arch%': 'x64',
    'conditions': [
      ['escargot_lib_type=="shared_lib"', {
        'lib_ext': '.so'
      }, {
        'lib_ext': '.a'
      }],
      ['target_arch=="arm64"', {
        'target_arch': 'aarch64'
      }],
      ['target_arch=="x32"', {
        'target_arch': 'i686'
      }],
    ],
  },
  'targets': [{
    'target_name': 'escargot',
    'type': 'none',
    'variables': {
      'output_dir': '<(SHARED_INTERMEDIATE_DIR)/escargot',
      'escargot_libs': [
        '<(output_dir)/libescargot<(lib_ext)',
        '<(output_dir)/third_party/GCutil/libgc-lib.a',
        '<(output_dir)/third_party/runtime_icu_binder/libruntime-icu-binder-static.a',
        '<(output_dir)/liblibbf.a',
      ],
    },
    'all_dependent_settings': {
      'libraries': [
        '-lpthread',
        '<@(escargot_libs)',
        '-Wl,-rpath,\$$ORIGIN/<(output_dir)',
        '-Wl,-rpath,../lib',
        '-Wl,-rpath,\$$ORIGIN',
      ],
      'cflags': [ '-pthread' ],
      'ldflags': [ '-pthread' ],
      'configurations': {
        'Debug': {
          'defines': [
            '_GLIBCXX_DEBUG',
            'GC_DEBUG',
          ],
        },
      },
    },
    'direct_dependent_settings': {
      'defines': [
        'ESCARGOT_ENABLE_TYPEDARRAY=1',
        'ESCARGOT_ENABLE_PROMISE=1',
      ],
      'include_dirs': [
        '<(escargot_dir)/src/api',
        '<(escargot_dir)/third_party/GCutil',
        '<(escargot_dir)/third_party/GCutil/bdwgc/include',
      ],
    },
    'actions': [
      {
        'action_name': 'config escargot',
        'inputs': [],
        'outputs': ['<(output_dir)'],
        'action': [
          'cmake', '<(escargot_dir)', '-B<(output_dir)',
          '-GNinja',
          '-DESCARGOT_ARCH=<(target_arch)',
          '-DESCARGOT_MODE=<(build_mode)',
          '-DESCARGOT_HOST=<(build_host)',
          '-DESCARGOT_OUTPUT=<(escargot_lib_type)',
          '-DESCARGOT_ASAN=<(build_asan)',
        ],
      },
      {
        'action_name': 'build escargot',
        'inputs': ['<(output_dir)'],
        'outputs': ['<@(escargot_libs)'],
        'action': [
          'ninja', '-v', '-C', '<(output_dir)'
        ],
      },
    ],
  }],
}
