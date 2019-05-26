TEMPLATE = subdirs

slateDir.depends = FileBackends AuxiliaryPanes
slateDir.subdir = slate

SUBDIRS = slateDir \
    AuxiliaryPanes \
    FileBackends

macx {
    sign.target = sign
    sign.CONFIG = recursive

    QMAKE_EXTRA_TARGETS += sign
}
