{
  'includes': [
    '../../../node.gypi',
  ],

  'dependencies': [
    '<(node_lib_target_name)',
    'deps/histogram/histogram.gyp:histogram',
    'deps/uvwasi/uvwasi.gyp:uvwasi',
    'node_dtrace_header',
    'node_dtrace_ustack',
    'node_dtrace_provider',
    # lwnode
    '<(lwnode_jsengine_path)/escargotshim.gyp:escargotshim',
    '<(lwnode_jsengine_path)/escargot.gyp:escargot',
  ],

  'include_dirs': [
    '../../../src',
    '../../../tools/msvs/genfiles',
    '../../../<(lwnode_jsengine_path)/include',
    '../../../deps/cares/include',
    '../../../deps/uv/include',
    '../../../deps/uvwasi/include',
    'test/cctest',
    # lwnode
    '../../../<(lwnode_jsengine_path)/src',
  ],

  'defines': [
    'NODE_ARCH="<(target_arch)"',
    'NODE_PLATFORM="<(OS)"',
    'NODE_WANT_INTERNALS=1',
  ],

  'sources': [
    '../../../src/node_snapshot_stub.cc',
    '../../../src/node_code_cache_stub.cc',
    'test/cctest/gtest/gtest-all.cc',
    'test/cctest/gtest/gtest_main.cc',
    'test/cctest/node_test_fixture.cc',
    'test/cctest/node_test_fixture.h',
    # 'test/cctest/test_aliased_buffer.cc',
    'test/cctest/test_base64.cc',
    'test/cctest/test_base_object_ptr.cc',
    # 'test/cctest/test_node_postmortem_metadata.cc',
    'test/cctest/test_environment.cc',
    'test/cctest/test_linked_binding.cc',
    # 'test/cctest/test_per_process.cc',
    # 'test/cctest/test_platform.cc',
    'test/cctest/test_json_utils.cc',
    'test/cctest/test_sockaddr.cc',
    # 'test/cctest/test_traced_value.cc',
    'test/cctest/test_util.cc',
    'test/cctest/test_url.cc',
  ],

  'conditions': [
    [ 'node_use_openssl=="true"', {
      'defines': [
        'HAVE_OPENSSL=1',
      ],
    }],
    ['v8_enable_inspector==1', {
      'sources': [
        'test/cctest/test_inspector_socket.cc',
        'test/cctest/test_inspector_socket_server.cc'
      ],
      'defines': [
        'HAVE_INSPECTOR=1',
      ],
    }, {
        'defines': [
          'HAVE_INSPECTOR=0',
        ]
    }],
  ],
} # cctest
