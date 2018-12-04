TEMPLATE = subdirs

SUBDIRS += \
    LocalFileBackend \
    HttpBackend

macx {
    sign.target = sign
    QMAKE_EXTRA_TARGETS += sign
}
