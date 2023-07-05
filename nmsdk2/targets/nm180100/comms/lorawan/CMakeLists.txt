add_library(lorawan STATIC)

set(LORAWAN_DIR ${PROJECT_SOURCE_DIR}/comms/lorawan)
set(LORAWAN_TARGET_DIR ${CMAKE_CURRENT_LIST_DIR})

target_compile_definitions(
    lorawan PRIVATE
	-D"REGION_AS923"
	-D"REGION_AU915"
	-D"REGION_EU868"
	-D"REGION_US915"
	-D"REGION_KR920"
	-D"REGION_IN865"
	-DLORAMAC_CLASSB_ENABLED
	-DSOFT_SE
	-DCONTEXT_MANAGEMENT_ENABLED
)

set (
	LORAWAN_INCLUDES
	${LORAWAN_DIR}/src/radio
	${LORAWAN_DIR}/src/radio/sx126x
	${LORAWAN_DIR}/src/boards
	${LORAWAN_DIR}/src/mac
	${LORAWAN_DIR}/src/mac/region
	${LORAWAN_DIR}/src/peripherals/soft-se
	${LORAWAN_DIR}/src/system
    ${LORAWAN_TARGET_DIR}/src/apps/LoRaMac/common
    ${LORAWAN_TARGET_DIR}/src/apps/LoRaMac/common/LmHandler
    ${LORAWAN_TARGET_DIR}/src/apps/LoRaMac/common/LmHandler/packages
    ${LORAWAN_TARGET_DIR}/src/boards/nm180100
    ${LORAWAN_TARGET_DIR}
    ${LORAWAN_TARGET_DIR}/../../utils
	CACHE PATH "LORAWAN includes"
)

target_include_directories(
    lorawan PRIVATE
    ${HAL_INCLUDES}
	${LORAWAN_INCLUDES}
)

target_sources(
    lorawan PRIVATE 

	${LORAWAN_DIR}/src/radio/sx126x/radio.c
	${LORAWAN_DIR}/src/radio/sx126x/sx126x.c
 
	${LORAWAN_DIR}/src/boards/mcu/utilities.c

	${LORAWAN_DIR}/src/mac/LoRaMacAdr.c
	${LORAWAN_DIR}/src/mac/LoRaMac.c
	${LORAWAN_DIR}/src/mac/LoRaMacClassB.c
	${LORAWAN_DIR}/src/mac/LoRaMacCommands.c
	${LORAWAN_DIR}/src/mac/LoRaMacConfirmQueue.c
	${LORAWAN_DIR}/src/mac/LoRaMacCrypto.c
	${LORAWAN_DIR}/src/mac/LoRaMacParser.c
	${LORAWAN_DIR}/src/mac/LoRaMacSerializer.c

	${LORAWAN_DIR}/src/mac/region/RegionAS923.c
	${LORAWAN_DIR}/src/mac/region/RegionAU915.c
	${LORAWAN_DIR}/src/mac/region/RegionBaseUS.c
	${LORAWAN_DIR}/src/mac/region/Region.c
#	${LORAWAN_DIR}/src/mac/region/RegionCN470.c
#	${LORAWAN_DIR}/src/mac/region/RegionCN779.c
	${LORAWAN_DIR}/src/mac/region/RegionCommon.c
#	${LORAWAN_DIR}/src/mac/region/RegionEU433.c
	${LORAWAN_DIR}/src/mac/region/RegionEU868.c
	${LORAWAN_DIR}/src/mac/region/RegionIN865.c
	${LORAWAN_DIR}/src/mac/region/RegionKR920.c
	${LORAWAN_DIR}/src/mac/region/RegionRU864.c
	${LORAWAN_DIR}/src/mac/region/RegionUS915.c

	${LORAWAN_DIR}/src/system/delay.c
	${LORAWAN_DIR}/src/system/nvmm.c
	${LORAWAN_DIR}/src/system/timer.c
	${LORAWAN_DIR}/src/system/systime.c

	${LORAWAN_TARGET_DIR}/src/boards/nm180100/board.c
	${LORAWAN_TARGET_DIR}/src/boards/nm180100/delay-board.c
	${LORAWAN_TARGET_DIR}/src/boards/nm180100/eeprom-board.c
	${LORAWAN_TARGET_DIR}/src/boards/nm180100/rtc-board.c
	${LORAWAN_TARGET_DIR}/src/boards/nm180100/sx1262-board.c

	${LORAWAN_TARGET_DIR}/src/boards/nm180100/lorawan_power.c

	${LORAWAN_TARGET_DIR}/src/apps/LoRaMac/common/LmHandlerMsgDisplay.c
	${LORAWAN_TARGET_DIR}/src/apps/LoRaMac/common/NvmDataMgmt.c
	${LORAWAN_TARGET_DIR}/src/apps/LoRaMac/common/LmHandler/LmHandler.c
	${LORAWAN_TARGET_DIR}/src/apps/LoRaMac/common/LmHandler/packages/FragDecoder.c
	${LORAWAN_TARGET_DIR}/src/apps/LoRaMac/common/LmHandler/packages/LmhpClockSync.c
	${LORAWAN_TARGET_DIR}/src/apps/LoRaMac/common/LmHandler/packages/LmhpCompliance.c
	${LORAWAN_TARGET_DIR}/src/apps/LoRaMac/common/LmHandler/packages/LmhpFragmentation.c
	${LORAWAN_TARGET_DIR}/src/apps/LoRaMac/common/LmHandler/packages/LmhpRemoteMcastSetup.c

    ${LORAWAN_TARGET_DIR}/../../utils/eeprom_emulation.c
)

install (TARGETS lorawan DESTINATION ${PROJECT_SOURCE_DIR}/lib/${TARGET})