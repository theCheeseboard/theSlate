#ifndef BRANCHESMODEL_H
#define BRANCHESMODEL_H

#include <QAbstractListModel>
#include <QAbstractItemDelegate>

struct BranchesModelPrivate;
class GitIntegration;
class BranchesModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit BranchesModel(GitIntegration* integration, bool includeActions = true, QObject *parent = nullptr);
    ~BranchesModel() override;

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

public slots:
    void reloadData();

private:
    BranchesModelPrivate* d;
};

class BranchesModelDelegate : public QAbstractItemDelegate
{
    Q_OBJECT

public:
    explicit BranchesModelDelegate(GitIntegration* integration, QObject* parent = nullptr) : QAbstractItemDelegate(parent) {
        gi = integration;
    }

private:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;

    GitIntegration* gi;
};

#endif // BRANCHESMODEL_H
