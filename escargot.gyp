{
  'includes': ['common.gypi'],
  'variables': {
    'target_arch%': '<(target_arch)',
    'target_os%': '<(target_os)',
    'build_host%': '<(build_host)',
    'asan%': '<(asan)',
    'escargot_dir%': 'deps/escargot',
    'escargot_build_mode%': 'release',
    "escargot_lib_type%": 'shared_lib', # static_lib | shared_lib
    'escargot_threading%': '<(escargot_threading)',
    'escargot_debugger%': '<(escargot_debugger)',
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
      ['target_os=="tizen"', {
        # tizen build host is fixed to gbs
        'build_host': 'tizen_obs'
      }],
    ],
  },
  'targets': [{
    'target_name': 'escargot',
    'type': 'none',
    'variables': {
      'configs': '<!(["find", "<(escargot_dir)", \
                              "-name", "CMakeLists.txt", "-o", "-name", "*.cmake"])',
      'sources': '<!(["find", "<(escargot_dir)/src", \
                              "<(escargot_dir)/third_party", \
                              "-name", "*.cpp", "-o", "-name", "*.cc"])',
      'output_dir': '<(SHARED_INTERMEDIATE_DIR)/escargot',
      'escargot_libs': [
        '<(output_dir)/libescargot<(lib_ext)',
        '<(output_dir)/third_party/GCutil/libgc-lib.a',
        '<(output_dir)/third_party/runtime_icu_binder/libruntime-icu-binder-static.a',
        '<(output_dir)/liblibbf.a',
      ],
      'escargot_configs': [
        '-DESCARGOT_SMALL_CONFIG=1',
        '-DESCARGOT_USE_CUSTOM_LOGGING=ON',
        '-DESCARGOT_USE_EXTENDED_API=ON',
        '-DESCARGOT_ARCH=<(target_arch)',
        '-DESCARGOT_HOST=<(build_host)',
        '-DESCARGOT_MODE=<(escargot_build_mode)',
        '-DESCARGOT_OUTPUT=<(escargot_lib_type)',
        '-DESCARGOT_THREADING=<(escargot_threading)',
        '-DESCARGOT_ASAN=<(asan)',
        '-DESCARGOT_DEBUGGER=<(escargot_debugger)',
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
        'ESCARGOT_USE_CUSTOM_LOGGING=1',
      ],
      'include_dirs': [
        '<(escargot_dir)/src/api',
        '<(escargot_dir)/third_party/GCutil',
        '<(escargot_dir)/third_party/GCutil/bdwgc/include',
      ],
      'conditions': [
        ['escargot_threading==1', {
          'defines':['ESCARGOT_THREADING']
        }],
      ],
    },
    'actions': [
      {
        'action_name': 'print configs',
        'inputs':  [],
        'outputs': ['<(SHARED_INTERMEDIATE_DIR)'],
        'action': ['printf', '%s\\n', '<@(escargot_configs)'],
      },
      {
        'action_name': 'config escargot',
        'inputs':  ['./escargot.gyp', '<@(configs)'],
        'outputs': ['<(output_dir)'],
        'action': [
          'cmake', '<(escargot_dir)', '-B<(output_dir)',
          '-GNinja',
          '<@(escargot_configs)',
        ],
      },
      {
        'action_name': 'build escargot',
        'inputs':  ['<(output_dir)', './escargot.gyp', '<@(sources)'],
        'outputs': ['<@(escargot_libs)'],
        'action': [
          'ninja', '-v', '-C', '<(output_dir)'
        ],
      },
    ],
  }],
}
