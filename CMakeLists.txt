cmake_minimum_required(VERSION 2.6)
project(snag2)

set(CMAKE_BUILD_TYPE Release)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")

set(BCM_HOST_INCLUDE_DIRS /opt/vc/include)
set(BCM_HOST_LIBRARY_DIRS /opt/vc/lib)
set(BCM_HOST_LIBRARIES bcm_host)

include_directories(${BCM_HOST_INCLUDE_DIRS})

link_directories(${BCM_HOST_LIBRARY_DIRS})

add_executable(${PROJECT_NAME}
	snag2.c
)

target_link_libraries(${PROJECT_NAME}
	${BCM_HOST_LIBRARIES}
)

set_property(TARGET ${PROJECT_NAME} PROPERTY SKIP_BUILD_RPATH TRUE)
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
