# TODO(vtl): lots of stuff
{
  'target_defaults': {
    'conditions': [
      ['OS=="win"', {
        'defines': [ 'WIN32', ],
        'sources/': [ ['exclude', '_unix(_unittest)?\\.(cpp|h)$'] ],
      }, {
        'sources/': [ ['exclude', '_win(_unittest)?\\.(cpp|h)$'], ],
      }],  # OS=="win"
    ],
  },
  'targets': [
    {
      'target_name': 'ipc_lib',
      'type': 'static_library',
      'msvs_guid': '61E911C1-F921-4F20-BA75-5F49424FCE79',
      'sources': [
        'src/ipc_channel.h',
        'src/ipc_codec.h',
        'src/ipc_msg_dispatch.h',
        'src/ipc_wire_types.h',
        'src/os_includes.h',
        'src/pipe_unix.cpp',
        'src/pipe_unix.h',
        'src/pipe_win.cpp',
        'src/pipe_win.h',
      ],
    },
    {
      'target_name': 'unit_test',
      'type': 'executable',
      'msvs_guid': 'CC82D835-A81E-4578-9329-FFC85FED63DE',
      'dependencies': [
        'ipc_lib',
      ],
      'sources': [
        'test/ipc_codec_unittest.cpp',
        'test/ipc_dispatch_unnitest.cpp',
        'test/ipc_roundtrip_unittest.cpp',
        'test/ipc_test_helpers.h',
        'test/ipc_transport_unix_unittest.cpp',
        'test/ipc_transport_win_unittest.cpp',
        'test/test_main.cpp',
      ],
      'include_dirs': [
        'src',
      ],
    },
  ],
}
