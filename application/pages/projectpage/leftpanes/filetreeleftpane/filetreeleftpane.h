#ifndef FILETREELEFTPANE_H
#define FILETREELEFTPANE_H

#include "../abstractleftpane/abstractleftpane.h"
#include <QUrl>
#include <project.h>

namespace Ui {
    class FileTreeLeftPane;
}

struct FileTreeLeftPanePrivate;
class FileTreeLeftPane : public AbstractLeftPane {
        Q_OBJECT

    public:
        explicit FileTreeLeftPane(ProjectPtr project, QWidget* parent = nullptr);
        ~FileTreeLeftPane();

    signals:
        void requestFileOpen(QUrl url);

    private:
        Ui::FileTreeLeftPane* ui;
        FileTreeLeftPanePrivate* d;

        // AbstractLeftPane interface
    public:
        tWindowTabberButton* tabButton();
    private slots:
        void on_fileSystemView_clicked(const QModelIndex& index);
};

#endif // FILETREELEFTPANE_H
