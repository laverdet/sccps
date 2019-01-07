{
	'target_defaults': {
		'configurations': {
			'Debug': {
				'cflags_cc': [ '-std=c++17', '-fno-rtti', '-Wno-invalid-offsetof' ],
				'cflags_cc!': [ '-fno-exceptions' ],
				'xcode_settings': {
					'GCC_ENABLE_CPP_EXCEPTIONS': 'YES',
					'CLANG_CXX_LANGUAGE_STANDARD': 'c++17',
					'GCC_ENABLE_CPP_RTTI': 'YES',
					'MACOSX_DEPLOYMENT_TARGET': '10.14',
					'WARNING_CFLAGS': [ '-Wno-invalid-offsetof' ],
				},
				'msvs_settings': {
					'VCCLCompilerTool': {
						'AdditionalOptions': [ '/GR' ],
						'ExceptionHandling': '1',
						'RuntimeTypeInfo': 'true',
					},
				},
				'conditions': [
					[ 'OS == "win"', { 'defines': [ 'NOMINMAX' ] } ],
				],
			},
		},
	},
	'targets': [
		{
			'target_name': 'module',
			'include_dirs': [
				'<!(node -e "require(\'nan\')")',
				'<!(node -e "require(\'isolated-vm/include\')")',
				'../../../include'
			],
			'libraries': [ '../../module.a' ],
			'sources': SOURCES,
		},
	],
}
