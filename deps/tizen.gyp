{
  'targets': [
    {
      'target_name': 'dlog',
      'type': 'none',
      'all_dependent_settings': {
        'cflags': [ '<!@(pkg-config --cflags dlog)'],
        'libraries': [
          '<!@(pkg-config --libs dlog)',
        ],
      },
    },
    {
      'target_name': 'appcommon',
      'type': 'none',
      'all_dependent_settings': {
        'cflags': [ '<!@(pkg-config --cflags aul bundle capi-appfw-app-common)'],
        'libraries': [
          '<!@(pkg-config --libs aul bundle capi-appfw-app-common)',
        ],
      },
    },
    {
      'target_name': 'system',
      'type': 'none',
      'all_dependent_settings': {
        'cflags': [
          '<!@(pkg-config --cflags capi-system-info capi-system-system-settings)',
        ],
        'libraries': [
          '<!@(pkg-config --libs capi-system-info capi-system-system-settings)',
        ],
      },
    },
  ],
}
