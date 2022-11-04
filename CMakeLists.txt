if ( PROJECT_NAME )
	set( SUBPROJECT TRUE )
endif()

cmake_policy( SET CMP0048 NEW )

project( message_broker
	VERSION
		1.0.0
	LANGUAGES
		CXX
	DESCRIPTION
		"type-safe message_broker"
)

cmake_minimum_required( VERSION 3.20 )

add_library( message_broker INTERFACE )

target_include_directories( message_broker
	INTERFACE
		${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features( message_broker
	INTERFACE
		cxx_std_20
)

if ( NOT SUBPROJECT )

	include( FetchContent )

	FetchContent_Declare( Catch2
		SYSTEM
		GIT_REPOSITORY
			https://github.com/catchorg/Catch2.git
		GIT_TAG
			v3.1.1
	)

	FetchContent_MakeAvailable( Catch2 )

	add_executable( message_broker_test
		main.cpp
	)

	target_link_libraries( message_broker_test
		PRIVATE
			message_broker
			Catch2WithMain
	)

	enable_testing()

	add_test( NAME mbt COMMAND message_broker_test )

endif()