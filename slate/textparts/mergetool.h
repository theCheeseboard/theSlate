#ifndef MERGETOOL_H
#define MERGETOOL_H

#include <QDialog>
#include "texteditor.h"
#include "mainwindow.h"

class MainWindow;

namespace Ui {
    class MergeTool;
}

struct MergeLines {
    int startLine;
    int length;

    QString source, remote;

    friend bool operator<(const MergeLines &l, const MergeLines &r) {
        return l.startLine < r.startLine;
    }
};

class MergeTool : public QDialog
{
        Q_OBJECT

    public:
        explicit MergeTool(QString unmergedFile, MainWindow* parentWindow, QWidget *parent = nullptr);
        ~MergeTool();

        static QString getUnmergedFile(QString original, QString edited, QString oTitle, QString eTitle, bool* mergeResolutionRequired);
        void setTitle(QString title);

    private slots:
        void on_cancelButton_clicked();
        void updateMergedFile();

        void on_acceptButton_clicked();

    signals:
        void acceptResolution(QString revisedDocument);

    private:
        Ui::MergeTool *ui;

        TextEditor *source, *remote, *endFile;
        QList<MergeLines> mergeLines;
        QString mergedDocument;
};

#endif // MERGETOOL_H
