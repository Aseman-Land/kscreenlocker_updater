QT += gui widgets
CONFIG += c++11
SOURCES += main.cpp
TARGET = kscreenlocker_updater

isEmpty(PREFIX) {
    isEmpty(INSTALL_BINS_PREFIX): INSTALL_BINS_PREFIX = $$[QT_INSTALL_BINS]
} else {
    isEmpty(INSTALL_BINS_PREFIX): INSTALL_BINS_PREFIX = $$PREFIX/bin
}

target = $$TARGET
target.path = $$INSTALL_BINS_PREFIX

message(Project will be install to: $$INSTALL_BINS_PREFIX/$$TARGET)

INSTALLS += target
