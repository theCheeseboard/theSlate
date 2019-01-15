TEMPLATE = subdirs

slateDir.depends = FileBackends
slateDir.subdir = slate

SUBDIRS = slateDir \
    FileBackends

macx {
    sign.target = sign
    sign.CONFIG = recursive

    QMAKE_EXTRA_TARGETS += sign
}
