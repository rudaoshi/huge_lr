


include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../vowpalwabbit/)

find_package(Boost REQUIRED COMPONENTS program_options filesystem system)
if(Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})

  add_executable (scale_stat scale_stat.cpp)
  target_link_libraries (scale_stat LINK_PUBLIC vw ${Boost_LIBRARIES} z)

  add_executable (view_sample view_sample.cpp)
  target_link_libraries (view_sample LINK_PUBLIC vw ${Boost_LIBRARIES} z)

  add_executable (scale_samples scale_samples.cpp)
  target_link_libraries (scale_samples LINK_PUBLIC vw ${Boost_LIBRARIES}  z pthread)

  add_executable (group_stat group_stat.cpp)
  target_link_libraries (group_stat LINK_PUBLIC vw ${Boost_LIBRARIES}  z pthread)

  add_executable (group_update group_update.cpp)
  target_link_libraries (group_update LINK_PUBLIC vw ${Boost_LIBRARIES}  z pthread)

  add_executable (predict predict.cpp)
  target_link_libraries (predict LINK_PUBLIC vw ${Boost_LIBRARIES}  z pthread)

  add_executable (sparse_to_dense sparse_to_dense.cpp)
  target_link_libraries (sparse_to_dense LINK_PUBLIC vw ${Boost_LIBRARIES}  z pthread)

endif()

# Create a library called "Hello" which includes the source file "hello.cxx".
# The extension is already found. Any number of sources could be listed here.


