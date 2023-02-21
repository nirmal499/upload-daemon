add_executable(
  main
  main.cpp
)

add_executable(
  main_test
  main_test.cpp
)

target_link_libraries(main
    PRIVATE request
)

target_link_libraries(main_test
    PRIVATE request
)

