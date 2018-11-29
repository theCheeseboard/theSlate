#include "mergetool.h"
#include "ui_mergetool.h"

#include <QBuffer>

MergeTool::MergeTool(QString unmergedFile, MainWindow* mainWindow, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MergeTool)
{
    ui->setupUi(this);

    #ifdef Q_OS_MAC
        ui->cancelButton->setVisible(false);
        ui->acceptButton->setVisible(false);
    #else
        ui->windowControlsMac->setVisible(false);
    #endif

    this->resize(mainWindow->width() - 20, mainWindow->height() - 50);

    source = new TextEditor(mainWindow);
    remote = new TextEditor(mainWindow);
    endFile = new TextEditor(mainWindow);
    source->lockScrolling(remote);
    source->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    source->setReadOnly(true);
    remote->setReadOnly(true);

    source->setFrameStyle(QFrame::NoFrame);
    remote->setFrameStyle(QFrame::NoFrame);
    endFile->setFrameStyle(QFrame::NoFrame);

    ui->localChangesLayout->addWidget(source);
    ui->remoteChangesLayout->addWidget(remote);
    ((QBoxLayout*) this->layout())->insertWidget(5, endFile);

    //Parse the file line by line
    int state = 0;
    /* 0: normal
     * 1: reading source
     * 2: reading remote */

    MergeLines currentMergeMetadata;
    int currentLine = 0;
    QByteArray unmergedFileBytes = unmergedFile.toUtf8();
    QString sourceStr, sourceMergeContents;
    QString remoteStr, remoteMergeContents;
    QString mergedStr;
    int linesInSource = 0;
    int linesInRemote = 0;
    QBuffer buf(&unmergedFileBytes);
    buf.open(QBuffer::ReadOnly);
    while (!buf.atEnd()) {
        QString line = buf.readLine();
        switch (state) {
            case 0: {
                if (line.startsWith("<<<<<<<")) {
                    state = 1;
                    mergedStr.append(QString("======= %1\n").arg(mergeLines.count() + 1));
                    ui->localChangesTitle->setText(tr("%1 (Current changes)").arg(line.mid(8).trimmed()));
                    remoteMergeContents.clear();
                    sourceMergeContents.clear();
                    currentMergeMetadata.startLine = currentLine;
                } else {
                    sourceStr.append(line);
                    remoteStr.append(line);
                    mergedStr.append(line);
                    currentLine++;
                }
                break;
            }
            case 1: {
                if (line.startsWith("=======")) {
                    state = 2;
                } else {
                    sourceStr.append(line);
                    sourceMergeContents.append(line);
                    linesInSource++;
                }
                break;
            }
            case 2: {
                if (line.startsWith(">>>>>>>")) {
                    state = 0;
                    ui->remoteChangesTitle->setText(tr("%1 (Incoming changes)").arg(line.mid(8).trimmed()));

                    int linesChanged = qMax(linesInSource, linesInRemote);
                    currentMergeMetadata.length = linesChanged;

                    if (remoteMergeContents.length() == 0) {
                        currentMergeMetadata.remote = "";
                    } else {
                        currentMergeMetadata.remote = remoteMergeContents.chopped(1);
                    }

                    if (sourceMergeContents.length() == 0) {
                        currentMergeMetadata.source = "";
                    } else {
                        currentMergeMetadata.source = sourceMergeContents.chopped(1);
                    }

                    mergeLines.append(currentMergeMetadata);
                    currentLine += linesChanged;

                    //Now determine the number of lines to add to each
                    if (linesInSource < linesInRemote) {
                        int padding = linesInRemote - linesInSource;
                        for (int i = 0; i < padding; i++) {
                            sourceStr.append("\n");
                        }
                    } else if (linesInSource > linesInRemote) {
                        int padding = linesInSource - linesInRemote;
                        for (int i = 0; i < padding; i++) {
                            remoteStr.append("\n");
                        }
                    }

                    linesInSource = 0;
                    linesInRemote = 0;
                } else {
                    remoteStr.append(line);
                    remoteMergeContents.append(line);
                    linesInRemote++;
                }
            }
        }
    }

    source->openFileFake(sourceStr);
    remote->openFileFake(remoteStr);
    mergedDocument = mergedStr;

    updateMergedFile();
    connect(source, SIGNAL(mergeDecision(MergeLines,bool)), this, SLOT(updateMergedFile()));
    connect(remote, SIGNAL(mergeDecision(MergeLines,bool)), this, SLOT(updateMergedFile()));

    source->setMergedLines(mergeLines);
    remote->setMergedLines(mergeLines);
}

MergeTool::~MergeTool()
{
    delete ui;
}

void MergeTool::setTitle(QString title) {
    this->setWindowTitle(title);
    ui->titleLabel->setText(title);
}

void MergeTool::updateMergedFile() {
    QString mergedStr = mergedDocument;
    int current = 1;
    for (MergeLines mergeData : mergeLines) {
        QStringList line;
        if (source->mergedLineIsAccepted(mergeData)) {
            line.append(mergeData.source);
        }

        if (remote->mergedLineIsAccepted(mergeData)) {
            line.append(mergeData.remote);
        }

        if (line.count() == 0) {
            mergedStr = mergedStr.replace(QString("======= %1").arg(current), tr("[Awaiting merge decision]"));
        } else {
            mergedStr = mergedStr.replace(QString("======= %1").arg(current), line.join("\n"));
        }
        current++;
    }
    endFile->openFileFake(mergedStr);
}

void MergeTool::on_cancelButton_clicked()
{
    this->close();
}

void MergeTool::on_acceptButton_clicked()
{
    QString finalFile = endFile->toPlainText();
    if (finalFile.contains(tr("[Awaiting merge decision]"))) {
        QMessageBox* messageBox = new QMessageBox(this);
        #ifdef Q_OS_MAC
            messageBox->setText(tr("Unresolved Merge Conflicts"));
            messageBox->setInformativeText(tr("You still have merge conflicts for which you have not yet selected a resolution. Are you sure you still want to accept this resolution?"));
        #else
            messageBox->setWindowTitle(tr("Unresolved Merge Conflicts"));
            messageBox->setText(tr("You still have merge conflicts for which you have not yet selected a resolution. Are you sure you still want to accept this resolution?"));
        #endif
        messageBox->setIcon(QMessageBox::Warning);
        messageBox->setWindowFlags(Qt::Sheet);
        messageBox->setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        messageBox->setDefaultButton(QMessageBox::Yes);
        if (messageBox->exec() == QMessageBox::No) return;
    }
    finalFile.remove(tr("[Awaiting merge decision]") + "\n");

    acceptResolution(finalFile);
    this->close();
}

QString MergeTool::getUnmergedFile(QString original, QString edited, QString oTitle, QString eTitle, bool* mergeResolutionRequired) {
    //QString file;
    *mergeResolutionRequired = false;

    QTextStream ostr(&original);
    QTextStream estr(&edited);

    QLinkedList<QString> oList, eList;
    QStringList output;
    while (!ostr.atEnd()) oList.append(ostr.readLine());
    while (!estr.atEnd()) eList.append(estr.readLine());

    int mergeStage = 0;
    QStringList oMerge, eMerge;
    while (oList.count() != 0 || eList.count() != 0) {
        if (mergeStage == 0) {
            if ((oList.count() > 0 && eList.count() > 0) && oList.first() == eList.first()) {
                //Lines are the same; nothing to do here
                output.append(oList.takeFirst());
                eList.takeFirst();
            } else {
                //Lines are different; switch to merging left side mode
                mergeStage = 1;
            }
        } else if (mergeStage == 1) {
            if (oList.count() > 0) oMerge.append(oList.takeFirst());
            if (eList.count() > 0) eMerge.append(eList.takeFirst());

            if (oMerge.count() == 0) {
                //Automatically merge by just adding the text
                output.append(eMerge.takeFirst());
                mergeStage = 0;
            } else if (eMerge.count() == 0) {
                //Automatically merge by just adding the text
                output.append(oMerge.takeFirst());
                mergeStage = 0;
            } else if (oMerge.contains(eMerge.last())) {
                //Found the merging point!
                int oIndex = oMerge.indexOf(eMerge.last());
                if (oIndex == 0) {
                    //We can automatically merge this part
                    while (eMerge.count() != 1) {
                        output.append(eMerge.takeFirst());
                    }

                    //Push all the text back
                    while (oMerge.count() != 0) oList.prepend(oMerge.takeLast());
                    while (eMerge.count() != 0) eList.prepend(eMerge.takeLast());

                    mergeStage = 0;
                } else {
                    //This part will require manual intervention
                    output.append("<<<<<<< " + oTitle);
                    while (oIndex != 0) {
                        output.append(oMerge.takeFirst());
                        oIndex--;
                    }

                    output.append("=======");

                    while (eMerge.count() != 1) {
                        output.append(eMerge.takeFirst());
                    }

                    mergeStage = 2;
                }
            } else if (eMerge.contains(oMerge.last())) {
                //Found the merging point!
                int eIndex = eMerge.indexOf(oMerge.last());
                if (eIndex == 0) {
                    //We can automatically merge this part
                    while (oMerge.count() != 1) {
                        output.append(oMerge.takeFirst());
                    }

                    //Push all the text back
                    while (oMerge.count() != 0) oList.prepend(oMerge.takeLast());
                    while (eMerge.count() != 0) eList.prepend(eMerge.takeLast());

                    mergeStage = 0;
                } else {
                    //This part will require manual intervention
                    output.append("<<<<<<< " + oTitle);
                    while (oMerge.count() != 1) {
                        output.append(oMerge.takeFirst());
                    }

                    output.append("=======");

                    while (eIndex != 0) {
                        output.append(eMerge.takeFirst());
                        eIndex--;
                    }

                    mergeStage = 2;
                }
            }

            if (mergeStage == 2) {
                //Push all the text back
                while (oMerge.count() != 0) oList.prepend(oMerge.takeLast());
                while (eMerge.count() != 0) eList.prepend(eMerge.takeLast());
                output.append(">>>>>>> " + eTitle);
                *mergeResolutionRequired = true;
                mergeStage = 0;
            }
        }
    }

    if (mergeStage != 0) {
        //Push everything else to the file
        output.append("<<<<<<< " + oTitle);
        output.append(oMerge);
        output.append("=======");
        output.append(eMerge);
        output.append(">>>>>>> " + eTitle);
        *mergeResolutionRequired = true;
    }

    return output.join("\n");
}
