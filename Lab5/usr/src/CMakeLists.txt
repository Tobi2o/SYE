
add_executable(sh.elf sh.c)
add_executable(ls.elf ls.c)
add_executable(echo.elf echo.c)
add_executable(cat.elf cat.c)
add_executable(sleep.elf sleep.c)


target_link_libraries(sh.elf c)
target_link_libraries(ls.elf c)
target_link_libraries(echo.elf c)
target_link_libraries(cat.elf c)
target_link_libraries(sleep.elf c)


