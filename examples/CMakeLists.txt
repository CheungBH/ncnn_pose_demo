macro(ncnn_add_example name)
    add_executable(${name} ${name}.cpp)
    target_include_directories(${name} PRIVATE ${OpenCV_INCLUDE_DIRS} )
    target_link_libraries(${name} PRIVATE ncnn ${OpenCV_LIBRARIES})

    # add test to a virtual project group
    set_property(TARGET ${name} PROPERTY FOLDER "examples")
endmacro()

include_directories("include")

# Add source files
file(GLOB SOURCE_FILES
	"*.cpp"
        "*.h")

set(CMAKE_CXX_STANDARD 17)
#find_package(Torch REQUIRED)
find_package(OpenCV 3 REQUIRED)
#find_package(OpenCV 3.4.6 QUIET COMPONENTS opencv_world)
# for opencv 2.4 on ubuntu 16.04, there is no opencv_world but OpenCV_FOUND will be TRUE
if("${OpenCV_LIBRARIES}" STREQUAL "")
    set(OpenCV_FOUND FALSE)
endif()
if(NOT OpenCV_FOUND)
    find_package(OpenCV QUIET COMPONENTS core highgui imgproc imgcodecs videoio)
endif()
if(NOT OpenCV_FOUND)
    find_package(OpenCV QUIET COMPONENTS core highgui imgproc)
endif()

if(NOT OpenCV_FOUND)
    message(WARNING "OpenCV not found, examples won't be built")
elseif(NOT NCNN_PIXEL)
    message(WARNING "NCNN_PIXEL not enabled, examples won't be built")
else()
    message(STATUS "OpenCV library: ${OpenCV_INSTALL_PATH}")
    message(STATUS "    version: ${OpenCV_VERSION}")
    message(STATUS "    libraries: ${OpenCV_LIBRARIES}")
    message(STATUS "    include path: ${OpenCV_INCLUDE_DIRS}")

    if(${OpenCV_VERSION_MAJOR} GREATER 3)
        set(CMAKE_CXX_STANDARD 17)
    endif()
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../src)
    add_executable(underwater_st ${SOURCE_FILES})
    target_include_directories(underwater_st PRIVATE ${OpenCV_INCLUDE_DIRS})
    target_link_libraries(underwater_st PRIVATE ncnn ${TORCH_LIBRARIES} ${OpenCV_LIBRARIES})
endif()
