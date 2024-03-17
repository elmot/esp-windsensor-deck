cmake_minimum_required(VERSION 3.28)
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_VERSION 1)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
find_program(SDCC_LOCATION sdcc REQUIRED)
set(CMAKE_C_COMPILER ${SDCC_LOCATION})
add_compile_options(-mstm8 -pstm8s003 --debug --all-callee-saves --stack-auto --fverbose-asm  --float-reent --no-peep -DSTM8S003)
add_link_options(-mstm8 -pstm8s003 --out-fmt-elf --debug --all-callee-saves --stack-auto --fverbose-asm  --float-reent --no-peep)

project(stm8_display C)

set(CMAKE_C_STANDARD 90)
set(STM8_SPL_PATH ${PROJECT_SOURCE_DIR}/framework-ststm8spl/Libraries/STM8S_StdPeriph_Driver CACHE FILEPATH "Patched STM8 Standard Peripheral Library")

include_directories(${STM8_SPL_PATH}/inc)

add_executable(stm8_display src/main.c src/stm8s_it.c src/ERM240128-2_DemoCode.c)
target_include_directories(stm8_display PRIVATE ${PROJECT_SOURCE_DIR}/src)

target_sources(stm8_display PRIVATE ${STM8_SPL_PATH}/src/stm8s_clk.c)
target_sources(stm8_display PRIVATE ${STM8_SPL_PATH}/src/stm8s_tim1.c)
target_sources(stm8_display PRIVATE ${STM8_SPL_PATH}/src/stm8s_gpio.c)
target_sources(stm8_display PRIVATE ${STM8_SPL_PATH}/src/stm8s_uart1.c)