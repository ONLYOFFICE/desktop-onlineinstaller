# Copyright (C) Ascensio System SIA, 2009-2026
#
# This program is a free software product. You can redistribute it and/or
# modify it under the terms of the GNU Affero General Public License (AGPL)
# version 3 as published by the Free Software Foundation, together with the
# additional terms provided in the LICENSE file.
#
# This program is distributed WITHOUT ANY WARRANTY; without even the implied
# warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. For
# details, see the GNU AGPL at: https://www.gnu.org/licenses/agpl-3.0.html
#
# You can contact Ascensio System SIA by email at info@onlyoffice.com
# or by postal mail at 20A-6 Ernesta Birznieka-Upisha Street, Riga,
# LV-1050, Latvia, European Union.
#
# The interactive user interfaces in modified versions of the Program
# are required to display Appropriate Legal Notices in accordance with
# Section 5 of the GNU AGPL version 3.
#
# No trademark rights are granted under this License.
#
# All non-code elements of the Product, including illustrations,
# icon sets, and technical writing content, are licensed under the
# Creative Commons Attribution-ShareAlike 4.0 International License:
# https://creativecommons.org/licenses/by-sa/4.0/legalcode
#
# This license applies only to such non-code elements and does not
# modify or replace the licensing terms applicable to the Program's
# source code, which remains licensed under the GNU Affero General
# Public License v3.
#
# SPDX-License-Identifier: AGPL-3.0-only

TARGET  = online-installer
CONFIG  += c++11 utf8_source static_runtime
CONFIG  -= app_bundle
CONFIG  -= qt
CONFIG  -= debug_and_release debug_and_release_target

TEMPLATE = app

CORE_ROOT_DIR = $$PWD/../core
UICLASSES = $$PWD/src/uiclasses

DEFINES += VISUALUI_STATIC
DEFINES += VISUALUI_SIMPLIFIED
CONFIG += core_no_dst
include($$CORE_ROOT_DIR/Common/base.pri)

INCLUDEPATH += $$PWD/src \
               $$UICLASSES \
               $$UICLASSES/core \
               $$UICLASSES/common \
               $$UICLASSES/graphics \
               $$UICLASSES/layout \
               $$UICLASSES/widgets \
               $$UICLASSES/dialogs

INCLUDEPATH += $$PWD/../desktop-apps/win-linux/src/prop

HEADERS += $$PWD/src/version.h \
           $$PWD/src/resource.h \
           $$PWD/src/mainwindow.h \
           $$PWD/src/cdownloader.h \
           $$PWD/src/translator.h \
           $$PWD/src/cjson_p.h \
           $$PWD/src/cjson.h \
           $$PWD/src/utils.h \
           $$UICLASSES/common/uidefines.h \
           $$UICLASSES/common/uiplatformtypes.h \
           $$UICLASSES/common/uiutils.h \
           $$UICLASSES/common/uicommon.h \
           $$UICLASSES/graphics/uimetrics.h \
           $$UICLASSES/graphics/uipalette.h \
           $$UICLASSES/graphics/uidrawningengine.h \
           $$UICLASSES/graphics/uidrawingsurface.h \
           $$UICLASSES/graphics/uistyle.h \
           # $$UICLASSES/graphics/uigeometryanimation.h \
           # $$UICLASSES/graphics/uiopacityanimation.h \
           $$UICLASSES/graphics/uiconhandler.h \
           # $$UICLASSES/graphics/uidraghandler.h \
           # $$UICLASSES/graphics/uitooltiphandler.h \
           $$UICLASSES/graphics/uipixmap.h \
           $$UICLASSES/core/uiobject.h \
           # $$UICLASSES/core/uicursor.h \
           $$UICLASSES/core/uiapplication.h \
           $$UICLASSES/core/uithread.h \
           $$UICLASSES/core/uifontmetrics.h \
           $$UICLASSES/core/uitimer.h \
           # $$UICLASSES/core/uieventloop.h \
           $$UICLASSES/core/uixmldocument.h \
           # $$UICLASSES/dialogs/uipopupmessage.h \
           # $$UICLASSES/dialogs/uidialog.h \
           # $$UICLASSES/dialogs/uifiledialog.h \
           $$UICLASSES/widgets/uiwindow.h \
           $$UICLASSES/widgets/uiabstractwindow.h \
           # $$UICLASSES/widgets/uiabstractpopup.h \
           # $$UICLASSES/widgets/uitooltip.h \
           # $$UICLASSES/widgets/uimenu.h \
           $$UICLASSES/widgets/uiwidget.h \
           $$UICLASSES/widgets/uilabel.h \
           $$UICLASSES/widgets/uicaption.h \
           $$UICLASSES/widgets/uiabstractbutton.h \
           $$UICLASSES/widgets/uibutton.h \
           $$UICLASSES/widgets/uicheckbox.h \
           $$UICLASSES/widgets/uiradiobutton.h \
           # $$UICLASSES/widgets/uitogglebutton.h \
           $$UICLASSES/widgets/uiprogressbar.h \
           # $$UICLASSES/widgets/uilineedit.h \
           $$UICLASSES/layout/uispacer.h \
           $$UICLASSES/layout/uilayoutitem.h \
           $$UICLASSES/layout/uilayout.h \
           $$UICLASSES/layout/uiboxlayout.h

SOURCES += $$PWD/src/main.cpp \
           $$PWD/src/mainwindow.cpp \
           $$PWD/src/cdownloader.cpp \
           $$PWD/src/translator.cpp \
           $$PWD/src/cjson.cpp \
           $$PWD/src/utils.cpp \
           $$UICLASSES/common/uiutils.cpp \
           $$UICLASSES/common/uicommon.cpp \
           $$UICLASSES/graphics/uimetrics.cpp \
           $$UICLASSES/graphics/uipalette.cpp \
           $$UICLASSES/graphics/uidrawningengine.cpp \
           $$UICLASSES/graphics/uidrawingsurface.cpp \
           $$UICLASSES/graphics/uistyle.cpp \
           # $$UICLASSES/graphics/uigeometryanimation.cpp \
           # $$UICLASSES/graphics/uiopacityanimation.cpp \
           $$UICLASSES/graphics/uiconhandler.cpp \
           # $$UICLASSES/graphics/uidraghandler.cpp \
           # $$UICLASSES/graphics/uitooltiphandler.cpp \
           $$UICLASSES/graphics/uipixmap.cpp \
           $$UICLASSES/core/uiobject.cpp \
           # $$UICLASSES/core/uicursor.cpp \
           $$UICLASSES/core/uiapplication.cpp \
           $$UICLASSES/core/uifontmetrics.cpp \
           $$UICLASSES/core/uitimer.cpp \
           # $$UICLASSES/core/uieventloop.cpp \
           $$UICLASSES/core/uixmldocument.cpp \
           # $$UICLASSES/dialogs/uipopupmessage.cpp \
           # $$UICLASSES/dialogs/uidialog.cpp \
           # $$UICLASSES/dialogs/uifiledialog.cpp \
           $$UICLASSES/widgets/uiwindow.cpp \
           $$UICLASSES/widgets/uiabstractwindow.cpp \
           # $$UICLASSES/widgets/uiabstractpopup.cpp \
           # $$UICLASSES/widgets/uitooltip.cpp \
           # $$UICLASSES/widgets/uimenu.cpp \
           $$UICLASSES/widgets/uiwidget.cpp \
           $$UICLASSES/widgets/uilabel.cpp \
           $$UICLASSES/widgets/uicaption.cpp \
           $$UICLASSES/widgets/uiabstractbutton.cpp \
           $$UICLASSES/widgets/uibutton.cpp \
           $$UICLASSES/widgets/uicheckbox.cpp \
           $$UICLASSES/widgets/uiradiobutton.cpp \
           # $$UICLASSES/widgets/uitogglebutton.cpp \
           $$UICLASSES/widgets/uiprogressbar.cpp \
           # $$UICLASSES/widgets/uilineedit.cpp \
           $$UICLASSES/layout/uispacer.cpp \
           $$UICLASSES/layout/uilayoutitem.cpp \
           $$UICLASSES/layout/uilayout.cpp \
           $$UICLASSES/layout/uiboxlayout.cpp

OTHER_FILES += $$PWD/res/version.rc \
               $$PWD/res/styles/styles.xml \
               $$PWD/res/styles/themes.xml \
               $$PWD/res/manifest/online-installer.exe.manifest

ENV_PRODUCT_VERSION = $$(PRODUCT_VERSION)
!isEmpty(ENV_PRODUCT_VERSION) {
    FULL_PRODUCT_VERSION = $${ENV_PRODUCT_VERSION}.$$(BUILD_NUMBER)
    DEFINES += VER_PRODUCT_VERSION=$$FULL_PRODUCT_VERSION \
               VER_PRODUCT_VERSION_COMMAS=$$replace(FULL_PRODUCT_VERSION, \., ",")
}

CONFIG -= embed_manifest_exe
RC_FILE = $$PWD/res/version.rc
QMAKE_CXXFLAGS += -D_UNICODE

contains(QMAKE_TARGET.arch, x86_64):{
    QMAKE_LFLAGS_WINDOWS = /SUBSYSTEM:WINDOWS,5.02
} else {
    QMAKE_LFLAGS_WINDOWS = /SUBSYSTEM:WINDOWS,5.01
}

core_release:DESTDIR = $$DESTDIR/build
core_debug:DESTDIR = $$DESTDIR/build/debug

!isEmpty(OO_BUILD_BRANDING) {
    DESTDIR = $$DESTDIR/$$OO_BUILD_BRANDING
}

DESTDIR = $$DESTDIR/$$CORE_BUILDS_PLATFORM_PREFIX

build_xp {
    DESTDIR = $$DESTDIR/xp
    DEFINES += __OS_WIN_XP
}

DEFINES -= NOMINMAX

LIBS += -luser32 \
        -lshell32 \
        -lshlwapi \
        -lwinhttp \
        -lwintrust \
        -lgdi32 \
        -lgdiplus \
        -ladvapi32 \
        -lrpcrt4 \
        -lole32 \
        -lmsi \
        -lwinmm \
        -lcomctl32

OBJECTS_DIR = $$DESTDIR/obj
MOC_DIR = $$DESTDIR/moc
RCC_DIR = $$DESTDIR/rcc
