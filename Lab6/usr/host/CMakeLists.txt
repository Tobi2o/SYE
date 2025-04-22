add_executable(threads.elf threads.c)

target_link_libraries(threads.elf c)
target_link_libraries(threads.elf pthread)