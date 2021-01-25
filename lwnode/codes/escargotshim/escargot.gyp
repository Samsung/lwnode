{
  'includes': ['common.gypi'],
  'variables': {
    'escargot_dir%': 'deps/escargot',
    'support_valgrind%': 'OFF',
    'lib_ext%': '.a',
  },
  'targets': [{
    'target_name': 'escargot',
    'type': 'none',
    'variables': {
      'output_dir': '<(SHARED_INTERMEDIATE_DIR)/escargot',
      'escargot_libs': [
        '<(output_dir)/libescargot<(lib_ext)',
        '<(output_dir)/third_party/GCutil/libgc-lib<(lib_ext)',
        '<(output_dir)/third_party/runtime_icu_binder/libruntime-icu-binder-static<(lib_ext)',
        '<(output_dir)/liblibbf<(lib_ext)',
      ],
    },
    'all_dependent_settings': {
      'libraries': [
        '-lpthread',
        '<@(escargot_libs)',
      ],
      'configurations': {
        'Debug': {
          'defines': ['_GLIBCXX_DEBUG', 'GC_DEBUG'],
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
          '-DESCARGOT_OUTPUT=static_lib',
          # TODO: use VALGRIND
          # '-DVALGRIND=<(support_valgrind)',
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
