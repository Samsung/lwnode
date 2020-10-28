{
  'variables': {
    'library_files': [
      'lib/escargot_shim.js',
    ],
  },
  'includes': [ 'escargotshim.gypi' ],
  'target_defaults': {
    'defines': [],
    'cflags': [
      # FIXME if possible
      '-Wno-unused-variable',
      '-Wno-unused-function',
      '-Wno-sign-compare',
      '-Wno-nonnull-compare',
      '-Wno-maybe-uninitialized',
      '-Wno-cast-function-type',
      '-Wno-stringop-truncation',
    ]
  },
  'targets': [
    {
      'target_name': 'escargotshim',
      'type': '<(library)',
      'dependencies': [
        'escargotshim_js2c#host',
        'escargot.gyp:escargot',
      ],
      'include_dirs': [
        '<@(escargotshim_include_dir)',
        '<@(escargot_include_dir)',
        '<(SHARED_INTERMEDIATE_DIR)',
      ],
      'sources': [
        'include/libplatform/libplatform.h',
        'include/v8.h',
        'include/v8config.h',
        'include/v8-debug.h',
        'include/v8-platform.h',
        'include/v8-profiler.h',
        'include/v8-version.h',
        '<(escargotshim_src_path)/escargotisolateshim.cc',
        '<(escargotshim_src_path)/escargotcontextshim.cc',
        '<(escargotshim_src_path)/escargotutil.cc',
        '<(escargotshim_src_path)/escargotplatform.cc',
        '<(escargotshim_src_path)/v8utils.cc',
        '<(escargotshim_src_path)/v8isolate.cc',
        '<(escargotshim_src_path)/v8context.cc',
        '<(escargotshim_src_path)/v8promise.cc',
        '<(escargotshim_src_path)/v8v8.cc',
        '<(escargotshim_src_path)/v8default-platform.cc',
        '<(escargotshim_src_path)/v8debug.cc',
        '<(escargotshim_src_path)/v8array.cc',
        '<(escargotshim_src_path)/v8symbol.cc',
        '<(escargotshim_src_path)/v8symbolobject.cc',
        '<(escargotshim_src_path)/v8private.cc',
        '<(escargotshim_src_path)/v8external.cc',
        '<(escargotshim_src_path)/v8script.cc',
        '<(escargotshim_src_path)/v8string.cc',
        '<(escargotshim_src_path)/v8value.cc',
        '<(escargotshim_src_path)/v8number.cc',
        '<(escargotshim_src_path)/v8global.cc',
        '<(escargotshim_src_path)/v8boolean.cc',
        '<(escargotshim_src_path)/v8booleanobject.cc',
        '<(escargotshim_src_path)/v8integer.cc',
        '<(escargotshim_src_path)/v8arraybuffer.cc',
        '<(escargotshim_src_path)/v8typedarray.cc',
        '<(escargotshim_src_path)/v8handlescope.cc',
        '<(escargotshim_src_path)/v8template.cc',
        '<(escargotshim_src_path)/v8object.cc',
        '<(escargotshim_src_path)/v8objecttemplate.cc',
        '<(escargotshim_src_path)/v8function.cc',
        '<(escargotshim_src_path)/v8functiontemplate.cc',
        '<(escargotshim_src_path)/v8trycatch.cc',
        '<(escargotshim_src_path)/v8stacktrace.cc',
        '<(escargotshim_src_path)/v8propertydescriptor.cc',
        '<(escargotshim_src_path)/v8exception.cc',
        '<(escargotshim_src_path)/v8message.cc',
        '<(escargotshim_src_path)/v8signature.cc',
        '<(escargotshim_src_path)/v8name.cc',
        '<(escargotshim_src_path)/v8int32.cc',
        '<(escargotshim_src_path)/v8uint32.cc',
        '<(escargotshim_src_path)/v8proxy.cc',
        '<(escargotshim_src_path)/v8resolver.cc',
        '<(escargotshim_src_path)/v8sharedarraybuffer.cc',
        '<(escargotshim_src_path)/v8valuedeserializer.cc',
        '<(escargotshim_src_path)/v8valueserializer.cc',
        '<(escargotshim_src_path)/v8persitent.cc',
        '<(escargotshim_src_path)/v8cpuprofiler.cc',
        '<(escargotshim_src_path)/v8heap.cc',
        '<(escargotshim_src_path)/v8snapshotcreator.cc',
        '<(escargotshim_src_path)/v8resourceconstraints.cc',
        '<(escargotshim_src_path)/v8map.cc',
        '<(escargotshim_src_path)/v8primitivearray.cc',
        '<(escargotshim_src_path)/v8scriptcompiler.cc',
        '<(escargotshim_src_path)/v8wasmmoduleobject.cc',
        '<(escargotshim_src_path)/v8microtasksscope.cc',
        '<(escargotshim_src_path)/v8module.cc',
        '<(escargotshim_src_path)/v8bigint.cc',
        '<(escargotshim_src_path)/v8set.cc',
        '<(escargotshim_src_path)/v8date.cc',
        '<(escargotshim_src_path)/v8dataview.cc',
        '<(escargotshim_src_path)/v8json.cc',
        '<(escargotshim_src_path)/v8tracing.cc',
        '<(escargotshim_src_path)/v8trace-writer.cc',
        '<(escargotshim_src_path)/jsutils.cc',
        '<(escargotshim_src_path)/base/programoptions.cc',
      ],
      'conditions': [
        ['target_os=="tizen"', {
          'variables': {
            'pkg-config': 'pkg-config'
          },
          'sources': [
            '<(tizen_device_api_dir)/src/Extension.cpp',
            '<(tizen_device_api_dir)/src/ExtensionAdapter.cpp',
            '<(tizen_device_api_dir)/src/ExtensionManager.cpp',
            '<(tizen_device_api_dir)/src/TizenDeviceAPILoaderForEscargot.cpp',
          ],
          'include_dirs': [
            '<(tizen_dir)/src',
            '<(tizen_device_api_dir)/src',
          ],
          'defines': [
            'TIZEN_DEVICE_API',
          ],
          'cflags': [
            '<!@(<(pkg-config) --cflags dlog glib-2.0)',
            # FIXME
            '-Wno-invalid-offsetof',
            '-Wno-error=format=',
          ],
          'cflags_cc': [],
          'libraries': [
            '<!@(<(pkg-config) --libs dlog glib-2.0)',
          ],
          }, {
          },
        ]
      ],
      'direct_dependent_settings': {
        'libraries': [
          '<(OBJ_DIR)/deps/escargotshim/libgc-lib.a',
          '<(OBJ_DIR)/deps/escargotshim/libescargot.a',
          '<(OBJ_DIR)/deps/escargotshim/libruntime-icu-binder-static.a',
        ],
        'defines': [
          'ESCARGOT_ENABLE_TYPEDARRAY=1',
          'ESCARGOT_ENABLE_PROMISE=1',
        ],
        'cflags': [
        ],
        'include_dirs': [
          'src', # FIXME remove src from include_dirs
          'include',
          'escargot/src/api',
          'escargot/third_party/GCutil',
          'escargot/third_party/GCutil/bdwgc/include',
        ],
      },
    }, # escargotshim
    {
      'target_name': 'escargotshim_js2c',
      'type': 'none',
      'toolsets': ['host'],
      'msvs_disabled_warnings': [4091],
      'actions': [
        {
          'action_name': 'escargotshim_js2c',
          'inputs': [
            '<@(library_files)'
          ],
          'options': [],
          'outputs': [
            '<(SHARED_INTERMEDIATE_DIR)/escargot_natives.h',
          ],
          'conditions': [
            [ 'enable_external_builtin_scripts=="true"', {
                'options': [ '--enable-external-builtin-scripts' ]
              }
            ]
          ],
          'action': [
            '<(python)',
            './../../tools/js2c_escargot.py',
            '<@(_options)',
            '--output-path=<(PRODUCT_DIR)',
            '--namespace=EscargotShim',
            '<@(_outputs)',
            '<@(_inputs)',
          ],
        },
      ],
    }, # end escargotshim_js2c
  ],
}
