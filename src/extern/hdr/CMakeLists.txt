cmake_minimum_required (VERSION 3.8)

target_include_directories(${ProjectName}
	PRIVATE 
	${CMAKE_CURRENT_SOURCE_DIR}
)

set( SOURCE_FILES
)
set( HEADE_FILES
   ${CMAKE_CURRENT_SOURCE_DIR}/sqlite_modern_cpp.h
)

target_sources(${ProjectName}
    PUBLIC
		${SOURCE_FILES}
	PUBLIC
		${HEADE_FILES}
)

