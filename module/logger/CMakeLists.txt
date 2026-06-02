add_library(logger STATIC logger.h logger.cc)

add_executable(logger_example logger_example.cc)
target_link_libraries(logger_example PRIVATE logger)
