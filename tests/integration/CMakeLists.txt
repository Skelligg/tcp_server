
add_executable(integration_ping_pong
    ping_pong_test.cpp
)

target_link_libraries(integration_ping_pong
    PRIVATE tcp_core
)

# Optional: make it visible via `ctest`
add_test(
    NAME integration_ping_pong
    COMMAND integration_ping_pong
)
