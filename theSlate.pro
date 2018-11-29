TEMPLATE = subdirs

slateDir.depends = SyntaxHighlightingPlugins FileBackends
slateDir.subdir = slate

SUBDIRS = slateDir \
    SyntaxHighlightingPlugins \
    FileBackends
