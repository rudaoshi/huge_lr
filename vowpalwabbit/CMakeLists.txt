

file(GLOB vw_SRC
    "*.h"
    "*.cc"
)

find_package(Boost REQUIRED COMPONENTS program_options)
if(Boost_FOUND)

    include_directories(${Boost_INCLUDE_DIRS})
# Create a library called "Hello" which includes the source file "hello.cxx".
# The extension is already found. Any number of sources could be listed here.
    add_library (vw ${vw_SRC})

# Make sure the compiler can find include files for our Hello library
# when other libraries or executables link to Hello
    target_include_directories (vw PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} )

    target_link_libraries (vw LINK_PUBLIC ${Boost_LIBRARIES}  z pthread)

endif()