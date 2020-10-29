{
  'includes': ['common.gypi'],
  'variables': {
    'escargot_dir%': 'deps/escargot',
    'support_valgrind%': 'OFF',
    'conditions': [],
  },
  'targets': [{
    'target_name': 'escargot',
    'type': 'none',
    'variables': {
      'escargot_lib': 'libescargot.a',
      'gc_lib': 'third_party/GCutil/libgc-lib.a',
      'icu_binder_lib': 'third_party/runtime_icu_binder/libruntime-icu-binder-static.a',
    },
    'all_dependent_settings': {
      'libraries': [
        '<(INTERMEDIATE_DIR)/<(escargot_lib)',
        '<(INTERMEDIATE_DIR)/<(gc_lib)',
        '<(INTERMEDIATE_DIR)/<(icu_binder_lib)',
      ],
    },
    'direct_dependent_settings': {
      'defines': [
        'ESCARGOT_ENABLE_TYPEDARRAY=1',
        'ESCARGOT_ENABLE_PROMISE=1',
      ],
      'cflags': [],
      'include_dirs': [
        '<(escargot_dir)/src/api',
        '<(escargot_dir)/third_party/GCutil',
        '<(escargot_dir)/third_party/GCutil/bdwgc/include',
      ],
      'conditions': [
        ['build_mode=="debug"', {
          'defines': ['_GLIBCXX_DEBUG', 'GC_DEBUG'],
        }],
      ],
    },
    'actions': [
      {
        'action_name': 'config escargot',
        'inputs': [],
        'outputs': ['<(INTERMEDIATE_DIR)'],
        'action': [
          'cmake', '<(escargot_dir)', '-B<(INTERMEDIATE_DIR)',
          '-GNinja',
          '-DESCARGOT_MODE=<(build_mode)',
          '-DESCARGOT_ARCH=<(host_arch)',
          '-DESCARGOT_HOST=<(escargot_os)',
          '-DESCARGOT_OUTPUT=static_lib',
          # TODO: use VALGRIND
          # '-DVALGRIND=<(support_valgrind)',
        ],
      },
      {
        'action_name': 'build escargot',
        'inputs': ['<(INTERMEDIATE_DIR)'],
        'outputs': [
          '<(INTERMEDIATE_DIR)/<(escargot_lib)',
          '<(INTERMEDIATE_DIR)/<(gc_lib)',
          '<(INTERMEDIATE_DIR)/<(icu_binder_lib)',
        ],
        'action': [
          'ninja', '-v', '-C', '<(INTERMEDIATE_DIR)'
        ],
      },
    ],
  }],
}
