#ifndef UNSAVEDCHANGESPOPOVER_H
#define UNSAVEDCHANGESPOPOVER_H

#include <QWidget>

namespace Ui {
    class UnsavedChangesPopover;
}

class AbstractPage;
struct UnsavedChangesPopoverPrivate;
class UnsavedChangesPopover : public QWidget {
        Q_OBJECT

    public:
        explicit UnsavedChangesPopover(QList<AbstractPage*> unsavedPages, QWidget* parent = nullptr);
        ~UnsavedChangesPopover();

        void processNextFile();

    signals:
        void accepted();
        void rejected();
        void hide();
        void show();

    private slots:
        void on_titleLabel_backButtonClicked();

        void on_saveAllChangesButton_clicked();

        void on_discardAllChangesButton_clicked();

    private:
        Ui::UnsavedChangesPopover* ui;
        UnsavedChangesPopoverPrivate* d;

        void updateState();
};

#endif // UNSAVEDCHANGESPOPOVER_H
