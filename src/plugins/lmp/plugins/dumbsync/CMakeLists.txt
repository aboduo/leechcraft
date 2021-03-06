CMAKE_MINIMUM_REQUIRED (VERSION 2.8)
PROJECT (leechcraft_lmp_dumbsync)
INCLUDE (InitLCPlugin OPTIONAL)

INCLUDE_DIRECTORIES (${LMP_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)

SET (DUMBSYNC_SRCS
	dumbsync.cpp
	xmlsettingsmanager.cpp
	)

CreateTrs ("lmp_dumbsync" "en;ru_RU" DUMBSYNC_COMPILED_TRANSLATIONS)
CreateTrsUpTarget("lmp_dumbsync" "en;ru_RU" "${DUMBSYNC_SRCS}" "${DUMBSYNC_FORMS}" "lmpdumbsyncsettings.xml")

ADD_LIBRARY (leechcraft_lmp_dumbsync SHARED
	${DUMBSYNC_COMPILED_TRANSLATIONS}
	${DUMBSYNC_SRCS}
	)
TARGET_LINK_LIBRARIES (leechcraft_lmp_dumbsync
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)

INSTALL (TARGETS leechcraft_lmp_dumbsync DESTINATION ${LC_PLUGINS_DEST})
INSTALL (FILES lmpdumbsyncsettings.xml DESTINATION ${LC_SETTINGS_DEST})
INSTALL (FILES ${DUMBSYNC_COMPILED_TRANSLATIONS} DESTINATION ${LC_TRANSLATIONS_DEST})