project(external)

if(WIN32)
    list(APPEND ADDITIONAL_LIBRARIES ws2_32)
endif()

add_subdirectory(accel-macros)
list(APPEND ADDITIONAL_LIBRARIES accel-macros)

set(ADDITIONAL_LIBRARIES ${ADDITIONAL_LIBRARIES} PARENT_SCOPE)