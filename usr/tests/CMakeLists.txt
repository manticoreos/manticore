project(tst)
set(CMAKE_C_FLAGS "-Wall -O3 -g -fno-stack-protector")
set(CMAKE_EXE_LINKER_FLAGS "-static -Wl,--gc-sections -nostdlib")

add_executable(tst-console_print tst-console_print.c)
target_link_libraries(tst-console_print manticore)
target_link_libraries(tst-console_print linux)

add_executable(tst-vmspace_alloc tst-vmspace_alloc.c)
target_link_libraries(tst-vmspace_alloc manticore)
target_link_libraries(tst-vmspace_alloc linux)
