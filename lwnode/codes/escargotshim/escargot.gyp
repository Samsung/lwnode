{
  'variables': {
    'support_valgrind%': 'OFF',
    'conditions': [
    ],
  },
  'targets': [
    {
      'target_name': 'escargot',
      'type': 'none',
      'variables': {
        'escargot_output_dir': './out/<(escargot_os)/<(host_arch)/<(build_mode)',
        'escargot_binaries': '<(escargot_output_dir)/libescargot.a',
        'gc_binaries': '<(escargot_output_dir)/third_party/GCutil/libgc-lib.a',
        'icu_binder': '<(escargot_output_dir)/third_party/runtime_icu_binder/libruntime-icu-binder-static.a',
      },
      'includes': [
        'escargotshim.gypi',
      ],
      'actions': [
        {
          'action_name': 'cmake_escargot',
          'inputs': [
          ],
          'outputs': [
            './out',
          ],
          'action': [
            'cmake', '.', '-Bout',
            '-DMODE=<(build_mode)',
            '-DARCH=<(host_arch)',
            '-DHOST=<(escargot_os)',
            '-DVALGRIND=<(support_valgrind)',
            '-GNinja',
          ],
        },
        {
          'action_name': 'build_escargot',
          'inputs': [
            './out'
          ],
          'outputs': [
            '<(escargot_binaries)',
            '<(gc_binaries)',
            '<(icu_binder)',
          ],
          'action': [
              'ninja', '-v', '-C', 'out'
          ],
        },
      ],
      'copies': [
        {
          'destination': '<(OBJ_DIR)/deps/escargotshim/',
          'files': [
            '<(escargot_binaries)',
            '<(gc_binaries)',
            '<(icu_binder)',
          ],
        },
      ],

      'direct_dependent_settings': {
      },
    }, # end escargot
  ],
}
