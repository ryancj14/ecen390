add_executable(lasertag.elf
m3_t2_main.c
filter.c
filterTest.c
histogram.c
sound.c
timer_ps.c
trigger.c
isr.c
hitLedTimer.c
lockoutTimer.c
transmitter.c
detector.c
# runningModes.c
)

add_subdirectory(sounds)
target_link_libraries(lasertag.elf ${330_LIBS} sounds lasertag_libs queue_lib)
set_target_properties(lasertag.elf PROPERTIES LINKER_LANGUAGE CXX)
