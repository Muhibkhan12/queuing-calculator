    # ==========================================================================
    # CMakeLists.txt
    # --------------------------------------------------------------------------
    # Build configuration for the Queueing Model Calculator (Qt6 GUI version).
    # This project is built directly from VS Code using the CMake Tools
    # extension (NOT Qt Creator). It links against Qt 6.11.1 installed at:
    #     C:\Qt\6.11.1\mingw64
    # ==========================================================================

    cmake_minimum_required(VERSION 3.16)

    project(QueueingCalculator VERSION 1.0 LANGUAGES CXX)

    # Use C++17 standard
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)

    # Automatically handle Qt's MOC (Meta-Object Compiler), UIC, and RCC
    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTOUIC ON)
    set(CMAKE_AUTORCC ON)

    # --------------------------------------------------------------------------
    # Tell CMake where to find Qt 6.
    # IMPORTANT: If your Qt path is different, update this line accordingly.
    # This can also be set in VS Code's settings.json instead of hardcoding
    # here (see cmake.configureSettings in the setup instructions).
    # --------------------------------------------------------------------------
    set(CMAKE_PREFIX_PATH "C:/Qt/6.11.1/mingw_64")

    # Find the required Qt6 modules (Widgets pulls in Core + Gui automatically)
    find_package(Qt6 REQUIRED COMPONENTS Widgets)

    # --------------------------------------------------------------------------
    # Source files
    # --------------------------------------------------------------------------
    set(PROJECT_SOURCES
        main.cpp
        mainwindow.h
        mainwindow.cpp
        queue_models.h
        queue_models.cpp
        utilities.h
        utilities.cpp
    )

    add_executable(QueueingCalculator ${PROJECT_SOURCES})

    target_link_libraries(QueueingCalculator PRIVATE Qt6::Widgets)

    # --------------------------------------------------------------------------
    # On Windows, this copies required Qt DLLs next to the .exe automatically
    # after each build, so the program runs without "missing DLL" errors.
    # --------------------------------------------------------------------------
    if(WIN32)
        add_custom_command(TARGET QueueingCalculator POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "$<TARGET_FILE:Qt6::Core>"
                "$<TARGET_FILE:Qt6::Gui>"
                "$<TARGET_FILE:Qt6::Widgets>"
                $<TARGET_FILE_DIR:QueueingCalculator>
        )
    endif()