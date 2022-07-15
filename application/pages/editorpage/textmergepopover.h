#ifndef TEXTMERGEPOPOVER_H
#define TEXTMERGEPOPOVER_H

#include <QWidget>

namespace Ui {
    class TextMergePopover;
}

class TextMergePopover : public QWidget {
        Q_OBJECT

    public:
        explicit TextMergePopover(QString file1, QString file2, QWidget* parent = nullptr);
        ~TextMergePopover();

    signals:
        void finished(bool ok, QString resolution);

    private slots:
        void on_titleLabel_backButtonClicked();

        void on_acceptButton_clicked();

    private:
        Ui::TextMergePopover* ui;
};

#endif // TEXTMERGEPOPOVER_H
