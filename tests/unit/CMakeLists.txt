
add_executable(unit_tests
    test_protocol.cpp
)

target_link_libraries(unit_tests
    PRIVATE
    tcp_core
    GTest::gtest
    GTest::gtest_main
)

add_test(NAME unit_tests COMMAND unit_tests)
