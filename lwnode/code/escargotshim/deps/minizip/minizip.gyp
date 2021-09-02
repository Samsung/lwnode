{
  'includes': ['../../common.gypi'],
  'targets': [
    {
      'target_name': 'minizip',
      'type': 'static_library',
      'sources': [
        'unzip.c',
        'ioapi.c',
      ],
      'defines': [
        'NOUNCRYPT',
      ],
      'link_settings': {
        'libraries': ['-lz'],
      },
      'direct_dependent_settings': {
        'defines': [
          'NOUNCRYPT',
        ],
        'include_dirs': [
          '.',
        ],
      },
    },
  ],
}
