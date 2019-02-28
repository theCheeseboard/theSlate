#ifndef COMMITSMODEL_H
#define COMMITSMODEL_H

#include <QAbstractListModel>
#include <QAbstractItemDelegate>

struct CommitsModelPrivate;
class GitIntegration;
class CommitsModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit CommitsModel(GitIntegration* integration, QObject *parent = nullptr);
    ~CommitsModel() override;

    struct Rail {
        QString endCommit;
        QColor color;
        bool dot = false;
        bool expand = false;
    };

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

public slots:
    void reloadData();

private:
    CommitsModelPrivate* d;
};
Q_DECLARE_METATYPE(QList<CommitsModel::Rail>);

class CommitsModelDelegate : public QAbstractItemDelegate
{
    Q_OBJECT

public:
    explicit CommitsModelDelegate(GitIntegration* integration, QObject* parent = nullptr) : QAbstractItemDelegate(parent) {
        gi = integration;
    }

private:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;

    GitIntegration* gi;
};

#endif // COMMITSMODEL_H
