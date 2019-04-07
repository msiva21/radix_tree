find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

add_executable(test_radix_tree
    main.cpp
)
target_link_libraries(test_radix_tree
    ${GTEST_BOTH_LIBRARIES}
    radix_tree
)
target_compile_options(test_radix_tree PRIVATE
    -Wall -Wextra -Wconversion -Wsign-conversion -Werror
)
