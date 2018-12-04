TEMPLATE = subdirs

slateDir.depends = SyntaxHighlightingPlugins FileBackends
slateDir.subdir = slate

SUBDIRS = slateDir \
    SyntaxHighlightingPlugins \
    FileBackends

macx {
    sign.target = sign
    sign.CONFIG = recursive

    QMAKE_EXTRA_TARGETS += sign
}
