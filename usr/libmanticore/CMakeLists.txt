project(libmanticore LANGUAGES C)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3")

add_library(manticore
    src/atomic-ring-buffer.c
    src/io_queue.c
    src/syscall.c
    src/syscalls/console_print.c
    src/syscalls/exit.c
    src/syscalls/get_io_queue.c
    src/syscalls/getevents.c
    src/syscalls/subscribe.c
    src/syscalls/wait.c
)

target_include_directories(manticore PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../include/uapi>
    $<INSTALL_INTERFACE:include>
    PRIVATE src)
