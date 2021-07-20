cmake_minimum_required(VERSION 3.9 FATAL_ERROR)
project(GeneticOptimization VERSION 3.0 DESCRIPTION "GeneticOptimization v3.0 Windows/Linux" LANGUAGES C CXX)

set(DEFAULT_BUILD_TYPE "Debug")
set(CMAKE_BUILD_TYPE "Debug")

#STNDARD STUFF
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${FLAGS_COMMON}" )
if (UNIX)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lX11 -O3 -Wall" )
endif (UNIX)
if (WIN32)
endif (WIN32)
set(TARGET_NAME "GeneticOptimization")

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

macro(copy_json jsonname)
	configure_file(${jsonname} ${CMAKE_BINARY_DIR}/${jsonname})
	configure_file(${jsonname} ${CMAKE_BUILD_TYPE}/${jsonname})
	configure_file(${jsonname} ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/${jsonname})
endmacro()

copy_json("configs/config.json")
copy_json("configs/preprocess.json")

copy_json("gtest/test.json")
copy_json("gtest/test_addmultipledron.json")

copy_json("gtest_it/test_graph.json")
copy_json("gtest_it/test_graph_config.json")
copy_json("gtest_it/test_dataset.json")
copy_json("gtest_it/test_preprocess.json")
copy_json("gtest_it/test_graph_postprocessing.json")
copy_json("gtest_it/test_graph_config_postprocesssing.json")



#file(COPY test_images DESTINATION ${CMAKE_BINARY_DIR})

include_directories( "include" )
include_directories( "PostProcessingModules/include" )
include_directories( "ProcessingModules/include" )

if (WIN32)
	set(QT5_PATH  "E:\\Qt\\5.15.2\\msvc2019_64")
	list(APPEND CMAKE_PREFIX_PATH ${QT5_PATH})
	list(APPEND CMAKE_PREFIX_PATH "E:\\repo\\opencvBuild\\")
endif (WIN32)

#list(APPEND CMAKE_PREFIX_PATH "include")
#list(APPEND CMAKE_PREFIX_PATH "src")

if (UNIX)
find_package(X11 REQUIRED)
endif (UNIX)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
find_package(Qt5Core REQUIRED)
find_package(OpenCV REQUIRED)
find_package(Qt5Test REQUIRED)
find_package(Qt5Network REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})
find_package(OpenCV QUIET COMPONENTS core highgui imgproc imgcodecs videoio)


enable_testing()

macro(clone_build libname)
	configure_file(clone_build/${libname}.in ${libname}-download/CMakeLists.txt)
	execute_process(COMMAND "${CMAKE_COMMAND}" -G "${CMAKE_GENERATOR}" .
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/${libname}-download")
	execute_process(COMMAND "${CMAKE_COMMAND}" --build .
	WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/${libname}-download")
	add_subdirectory("${CMAKE_BINARY_DIR}/${libname}-src"
                 "${CMAKE_BINARY_DIR}/${libname}-build")
endmacro()

clone_build(googletest)
# Prevent GoogleTest from overriding our compiler/linker options
# when building with Visual Studio
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

clone_build(spdlog)

set(SRC_MAIN src/main.cc)

set(SRC
	src/genetic/data.cc
	src/genetic/loaddata.cc
	src/genetic/case.cc
	src/genetic/genetic.cc
	src/genetic/geneticoperation.cc
	src/genetic/mainloop.cc
	src/genetic/graph.cc
	)

set(HEADERS
	include/genetic/data.h
	include/genetic/loaddata.h
	include/genetic/case.h
	include/genetic/genetic.h
	include/genetic/geneticoperation.h
	include/genetic/mainloop.h
	include/genetic/structures.h
	include/genetic/graph.h
	)

file(GLOB_RECURSE SRC_UTILS Utils/src/*.cc)
file(GLOB_RECURSE HEADERS_UTILS Utils/include/*.h)

set(SRC_GRAPH
	ProcessingGraph/src/graph.cc
)
set(HEADERS_GRAPH
	ProcessingGraph/include/graph.h
)

set(SRC_DATA
	DataManagement/src/data.cc
	DataManagement/src/loaddata.cc
)
set(HEADERS_DATA
	DataManagement/include/data.h
	DataManagement/include/loaddata.h
)

set(SRC_TEST_MEDIAN2
	test_median2/main.cc)

file(GLOB_RECURSE SRC_PROCESSING ProcessingModules/*.cc)
file(GLOB_RECURSE HEADERS_PROCESSING ProcessingModules/*.h)

file(GLOB_RECURSE SRC_POST_PROCESSING PostProcessingModules/*.cc)
file(GLOB_RECURSE HEADERS_POST_PROCESSING PostProcessingModules/*.h)

add_library(${PROJECT_NAME}_lib STATIC
	${SRC} ${HEADERS} 
	${SRC_PROCESSING} ${HEADERS_PROCESSING}
	${SRC_POST_PROCESSING} ${HEADERS_POST_PROCESSING})

add_library(utils_lib STATIC
	${SRC_UTILS} ${HEADERS_UTILS})

add_library(data_lib STATIC
	${SRC_DATA} ${HEADERS_DATA})

add_library(graph_lib STATIC
	${SRC_GRAPH} ${HEADERS_GRAPH})

add_executable(${PROJECT_NAME}
    ${SRC_MAIN})

add_executable(median2 ${SRC_TEST_MEDIAN2})

message(" cv_libs:     " ${OpenCV_LIBS})
message(" cv_includes: " ${OPENCV_INCLUDE_DIRS})

file(GLOB_RECURSE SRC_HEADERS_TEST gtest/*.cc gtest/*.h)
file(GLOB_RECURSE SRC_HEADERS_TEST_IT gtest_it/*.cc gtest_it/*.h)

add_executable(g_test ${SRC_HEADERS_TEST})
add_executable(g_test_it ${SRC_HEADERS_TEST_IT})

target_link_libraries(utils_lib ${QT_LIBRARIES} spdlog Qt5::Core)
target_link_libraries(data_lib ${QT_LIBRARIES} spdlog Qt5::Core ${OpenCV_LIBS})
target_link_libraries(graph_lib ${QT_LIBRARIES} spdlog Qt5::Core ${OpenCV_LIBS})

target_link_libraries(${PROJECT_NAME}_lib ${QT_LIBRARIES} ${OpenCV_LIBS} Threads::Threads utils_lib data_lib graph_lib)
target_link_libraries(${PROJECT_NAME}_lib Qt5::Core Qt5::Network)

if (UNIX)
target_link_libraries(${PROJECT_NAME}_lib X11)
endif (UNIX)

target_link_libraries(${PROJECT_NAME} ${PROJECT_NAME}_lib)
target_link_libraries(g_test ${PROJECT_NAME}_lib gtest gmock )
target_link_libraries(g_test_it ${PROJECT_NAME}_lib gtest gmock)
target_link_libraries(median2 ${PROJECT_NAME}_lib)


message("CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE} ")

if (CMAKE_BUILD_TYPE STREQUAL "Debug" AND WIN32)
	set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
	set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
endif()

if (CMAKE_BUILD_TYPE STREQUAL "Release" AND WIN32)
	set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
	set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
endif()


if (WIN32)
	message("CMAKE_BINARY_DIR: ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE} ")

	message("Copy OpenCV file:" )
	if(${CMAKE_BUILD_TYPE} STREQUAL  "Release")
		set(OPENCV_PREFIX_BUILD_TYPE "")
	endif()
	if(${CMAKE_BUILD_TYPE} STREQUAL  "Debug")
		set(OPENCV_PREFIX_BUILD_TYPE "d")
	endif()

	add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_if_different  ${QT5_PATH}/bin/Qt5Test${OPENCV_PREFIX_BUILD_TYPE}.dll   ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/
	COMMAND ${CMAKE_COMMAND} -E copy_if_different  ${QT5_PATH}/bin/Qt5Gui${OPENCV_PREFIX_BUILD_TYPE}.dll   ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/
	#COMMAND ${CMAKE_COMMAND} -E copy_if_different  ${QT5_PATH}/bin/Qt5Widgets${OPENCV_PREFIX_BUILD_TYPE}.dll   ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/
	COMMAND ${CMAKE_COMMAND} -E copy_if_different  ${OpenCV_DIR}/bin/${CMAKE_BUILD_TYPE}/opencv_stitching450${OPENCV_PREFIX_BUILD_TYPE}.dll   ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/
	COMMAND ${CMAKE_COMMAND} -E copy_if_different  ${OpenCV_DIR}/bin/${CMAKE_BUILD_TYPE}/opencv_bgsegm450${OPENCV_PREFIX_BUILD_TYPE}.dll   ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/
	COMMAND ${CMAKE_COMMAND} -E copy_if_different  ${OpenCV_DIR}/bin/${CMAKE_BUILD_TYPE}/opencv_tracking450${OPENCV_PREFIX_BUILD_TYPE}.dll   ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/
	COMMAND ${CMAKE_COMMAND} -E copy_if_different  ${OpenCV_DIR}/bin/${CMAKE_BUILD_TYPE}/opencv_videoio450${OPENCV_PREFIX_BUILD_TYPE}.dll   ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/
	COMMAND ${CMAKE_COMMAND} -E copy_if_different  ${OpenCV_DIR}/bin/${CMAKE_BUILD_TYPE}/opencv_video450${OPENCV_PREFIX_BUILD_TYPE}.dll   ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/
	COMMAND ${CMAKE_COMMAND} -E copy_if_different  ${OpenCV_DIR}/bin/${CMAKE_BUILD_TYPE}/opencv_imgcodecs450${OPENCV_PREFIX_BUILD_TYPE}.dll   ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/
	COMMAND ${CMAKE_COMMAND} -E copy_if_different  ${OpenCV_DIR}/bin/${CMAKE_BUILD_TYPE}/opencv_imgproc450${OPENCV_PREFIX_BUILD_TYPE}.dll   ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/
	COMMAND ${CMAKE_COMMAND} -E copy_if_different  ${OpenCV_DIR}/bin/${CMAKE_BUILD_TYPE}/opencv_core450${OPENCV_PREFIX_BUILD_TYPE}.dll   ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/
	COMMAND ${CMAKE_COMMAND} -E copy_if_different  ${OpenCV_DIR}/bin/${CMAKE_BUILD_TYPE}/opencv_calib3d450${OPENCV_PREFIX_BUILD_TYPE}.dll   ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/
	COMMAND ${CMAKE_COMMAND} -E copy_if_different  ${OpenCV_DIR}/bin/${CMAKE_BUILD_TYPE}/opencv_features2d450${OPENCV_PREFIX_BUILD_TYPE}.dll   ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/
	COMMAND ${CMAKE_COMMAND} -E copy_if_different  ${OpenCV_DIR}/bin/${CMAKE_BUILD_TYPE}/opencv_flann450${OPENCV_PREFIX_BUILD_TYPE}.dll   ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/
	COMMAND ${CMAKE_COMMAND} -E copy_if_different  ${OpenCV_DIR}/bin/${CMAKE_BUILD_TYPE}/opencv_dnn450${OPENCV_PREFIX_BUILD_TYPE}.dll   ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/
	COMMAND ${CMAKE_COMMAND} -E copy_if_different  ${OpenCV_DIR}/bin/${CMAKE_BUILD_TYPE}/opencv_highgui450${OPENCV_PREFIX_BUILD_TYPE}.dll   ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/
	COMMAND ${CMAKE_COMMAND} -E copy_if_different  ${OpenCV_DIR}/bin/${CMAKE_BUILD_TYPE}/opencv_highgui450${OPENCV_PREFIX_BUILD_TYPE}.dll   ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/
	COMMAND ${CMAKE_COMMAND} -E copy_if_different  ${OpenCV_DIR}/bin/${CMAKE_BUILD_TYPE}/opencv_gapi450${OPENCV_PREFIX_BUILD_TYPE}.dll   ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/
	)
endif(WIN32)
