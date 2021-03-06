CMAKE_MINIMUM_REQUIRED (VERSION 2.8)
PROJECT (leechcraft_monocle_pdf)
INCLUDE (InitLCPlugin OPTIONAL)

SET (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
FIND_PACKAGE (PopplerQt4 REQUIRED)

INCLUDE (${QT_USE_FILE})
INCLUDE_DIRECTORIES (
	${POPPLER_QT4_INCLUDE_DIR}
	${POPPLER_QT4_INCLUDE_DIR}/../cpp/
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
SET (SRCS
	pdf.cpp
	document.cpp
	links.cpp
	fields.cpp
	)

SET (PDF_RESOURCES pdfresources.qrc)
QT4_ADD_RESOURCES (PDF_RCCS ${PDF_RESOURCES})

ADD_LIBRARY (leechcraft_monocle_pdf SHARED
	${SRCS}
	${MOC_SRCS}
	${PDF_RCCS}
	)
TARGET_LINK_LIBRARIES (leechcraft_monocle_pdf
	${POPPLER_QT4_LIBRARIES}
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
INSTALL (TARGETS leechcraft_monocle_pdf DESTINATION ${LC_PLUGINS_DEST})
IF (UNIX AND NOT APPLE)
	INSTALL (FILES freedesktop/leechcraft-monocle-pdf.desktop DESTINATION share/applications)
ENDIF (UNIX AND NOT APPLE)
