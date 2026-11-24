 project(log_sentinel
        VERSION 0.1.0
        LANGUAGES CXX
    )

    # Definizione eseguibile e sorgenti
    add_executable(log_sentinel
        src/main.cpp
        src/log_parser.cpp
        src/analyzer.cpp
    )

    # Standard C++17 vincolato al target
    target_compile_features(log_sentinel PRIVATE cxx_std_17)

    # Directory include privata per questo target
    target_include_directories(log_sentinel PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    )

    # Warning di compilazione confinati solo al target log_sentinel
    target_compile_options(log_sentinel PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )
