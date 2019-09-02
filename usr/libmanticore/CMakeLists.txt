project(libmanticore LANGUAGES C)

add_library(manticore
    src/syscall.c
    src/syscalls/console_print.c
    src/syscalls/exit.c
    src/syscalls/subscribe.c
    src/syscalls/wait.c
)

target_include_directories(manticore PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
    PRIVATE src)
