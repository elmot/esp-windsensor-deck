cmake_minimum_required(VERSION 3.28)
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_VERSION 1)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
find_program(SDCC_LOCATION sdcc)
if (NOT EXISTS ${SDCC_LOCATION})
    SET(FAIL_MSG "SDCC is not in system PATH")
    if (WIN32 AND EXISTS "C:/Program Files/SDCC/bin/sdcc.exe")
        SET(FAIL_MSG "${FAIL_MSG}\r\nDetected location: C:\\Program Files\\SDCC\\bin")
    endif ()
    if (WIN32 AND EXISTS "C:/Program Files(x86)/SDCC/bin/sdcc.exe")
        SET(FAIL_MSG "${FAIL_MSG}\r\nDetected location: C:\\Program Files(x86)\\SDCC\\bin")
    endif ()
    MESSAGE(FATAL_ERROR ${FAIL_MSG})
endif ()
set(CMAKE_C_COMPILER ${SDCC_LOCATION})
#add_compile_options(-mstm8 -pstm8s003 --stack-auto --fverbose-asm  --no-peep -DSTM8S003)
#add_link_options(-mstm8 -pstm8s003 --out-fmt-elf  --stack-auto --fverbose-asm --no-peep)
add_compile_options(-mstm8 -pstm8s003 --debug --stack-auto --fverbose-asm --float-reent --no-peep -DSTM8S003)
add_link_options(-mstm8 -pstm8s003 --out-fmt-elf --debug --stack-auto --fverbose-asm --float-reent --no-peep)

project(stm8_display C)

set(CMAKE_C_STANDARD 90)
set(STM8_SPL_PATH ${PROJECT_SOURCE_DIR}/framework-ststm8spl/Libraries/STM8S_StdPeriph_Driver CACHE FILEPATH "Patched STM8 Standard Peripheral Library")

include_directories(${STM8_SPL_PATH}/inc)

set(CMAKE_EXECUTABLE_SUFFIX .elf)

add_executable(stm8_display src/main.c src/stm8s_it.c src/ERM240128-2_DemoCode.c)
target_include_directories(stm8_display PRIVATE ${PROJECT_SOURCE_DIR}/src)

target_sources(stm8_display PRIVATE src/lib/stm8s_clk.c src/lib/stm8s_tim1.c src/lib/stm8s_gpio.c src/lib/stm8s_uart1.c)

add_custom_target(FLASH_OPENOCD_STLINKV2
        DEPENDS stm8_display
        COMMAND C:/tools/openocd-v0.12.0-i686-w64-mingw32/bin/openocd.exe -f interface/stlink-dap.cfg -f target/stm8s003.cfg -c "init ; reset halt ; load_image $<TARGET_FILE:stm8_display> ; shutdown"
        USES_TERMINAL)
