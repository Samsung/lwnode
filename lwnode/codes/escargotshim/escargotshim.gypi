{
  'variables': {
    'sample_build%': 'false',
    'library%': 'static_library',
    'OBJ_DIR%': '<(PRODUCT_DIR)/obj',
    'escargot_dir%': 'escargot',
    'escargot_os%': '<(OS)',
    'escargotshim_dir': '<(PRODUCT_DIR)/../../deps/escargotshim',
    'tizen_dir': '<(PRODUCT_DIR)/../../deps/tizen',
    'tizen_device_api_dir': '<(escargotshim_dir)/tizen-device-api',
    'escargotshim_src_path': 'src',
    'escargotshim_include_dir%': [
      '<(escargotshim_dir)',
      '<(escargotshim_dir)/src',
      '<(escargotshim_dir)/include',
      '<(tizen_dir)/src',
    ],
    'escargot_include_dir%': [
      '<(escargotshim_dir)/escargot/src/api',
      '<(escargotshim_dir)/escargot/third_party/GCutil',
      '<(escargotshim_dir)/escargot/third_party/GCutil/bdwgc/include',
    ],
  },
  'target_defaults': {
    'defines': [],
    'cflags': [
      '-fno-omit-frame-pointer', '-fstack-protector',
      '-fdata-sections', '-ffunction-sections',
      # V8
      '-Wno-expansion-to-defined',
    ],
    'link_settings': {
      'libraries': [ '-ldl', '-lrt' ],
    },
    'configurations': {
      'Debug': {
        'defines': [
          '_GLIBCXX_DEBUG', 'GC_DEBUG'
        ],
      },
      'Release': {
        'defines': ['NDEBUG'],
      },
    }
  },
  'conditions': [
    ['escargot_os=="tizen_obs"', {
      'libraries': [
          '-ldlog',
        ],
    }],
    ['enable_escargotshim_asan==1', {
      'cflags+': [ '-fsanitize=address', '-fno-omit-frame-pointer' ],
      'cflags!': [ '-fomit-frame-pointer' ],
      'ldflags': [ '-fsanitize=address' ],
      'libraries': [
          '-lasan',
        ],
      # 'defines': ['-DESCARGOT_ASAN=1'],
    }],
  ],
}
