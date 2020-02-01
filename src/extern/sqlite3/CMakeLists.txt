cmake_minimum_required (VERSION 3.8)

target_include_directories(${ProjectName}
	PRIVATE 
	${CMAKE_CURRENT_SOURCE_DIR}
)


set( SOURCE_FILES
   ${CMAKE_CURRENT_SOURCE_DIR}/sqlite3.c
)
set( HEADE_FILES
   ${CMAKE_CURRENT_SOURCE_DIR}/sqlite3.h
)

set(LOCAL_INCL_DIR
    ${CMAKE_CURRENT_LIST_DIR}
)

target_sources(${ProjectName}
    PRIVATE
		${SOURCE_FILES}
	PRIVATE
		${HEADE_FILES}
)