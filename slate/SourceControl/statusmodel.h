#ifndef STATUSMODEL_H
#define STATUSMODEL_H

#include <QAbstractListModel>
#include <QAbstractItemDelegate>

struct StatusModelPrivate;
class GitIntegration;
class StatusModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit StatusModel(GitIntegration* integration, QObject *parent = nullptr);
    ~StatusModel() override;

    struct Status {
        QString file;
        QString status;
        bool staged;
    };

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    void reloadStatus();
    void setIsMerging(bool merging);

private:
    StatusModelPrivate* d;

    bool canChangeCheckState(int row) const;
};
Q_DECLARE_METATYPE(StatusModel::Status);

class StatusModelDelegate : public QAbstractItemDelegate
{
    Q_OBJECT

public:
    explicit StatusModelDelegate(GitIntegration* integration, QObject* parent = nullptr) : QAbstractItemDelegate(parent) {
        gi = integration;
    }

private:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
    bool editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem &option, const QModelIndex &index);

    QRect checkboxRect(const QStyleOptionViewItem &option) const;

    GitIntegration* gi;
};

#endif // STATUSMODEL_H
