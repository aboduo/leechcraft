CMAKE_MINIMUM_REQUIRED (VERSION 2.8)
PROJECT (leechcraft_monocle)
INCLUDE (InitLCPlugin OPTIONAL)

INCLUDE (${QT_USE_FILE})
INCLUDE_DIRECTORIES (
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
SET (SRCS
	monocle.cpp
	documenttab.cpp
	core.cpp
	)
SET (HEADERS
	monocle.h
	documenttab.h
	core.h
	)
SET (FORMS
	documenttab.ui
	)
IF (NOT LC_NO_MOC)
	QT4_WRAP_CPP (MOC_SRCS ${HEADERS})
ENDIF (NOT LC_NO_MOC)
QT4_WRAP_UI (UIS_H ${FORMS})

ADD_LIBRARY (leechcraft_monocle SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${MOC_SRCS}
	${UIS_H}
	)
TARGET_LINK_LIBRARIES (leechcraft_monocle
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
INSTALL (TARGETS leechcraft_monocle DESTINATION ${LC_PLUGINS_DEST})

OPTION (ENABLE_MONOCLE_PDF "Enable PDF backend for Monocle" ON)

IF (ENABLE_MONOCLE_PDF)
	ADD_SUBDIRECTORY (plugins/pdf)
ENDIF (ENABLE_MONOCLE_PDF)