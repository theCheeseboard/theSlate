#ifndef BUILDJOBISSUESMODEL_H
#define BUILDJOBISSUESMODEL_H

#include <QAbstractListModel>
#include <QStyledItemDelegate>
#include <project/buildjob.h>
#include <tpaintcalculator.h>

struct BuildJobIssuesModelPrivate;
class BuildJobIssuesModel : public QAbstractListModel {
        Q_OBJECT

    public:
        explicit BuildJobIssuesModel(BuildJobPtr buildJob, QObject* parent = nullptr);
        ~BuildJobIssuesModel();

        enum Roles {
            IssueType = Qt::UserRole,
            IssueMessage,
            EditorUrl,
            FileName
        };

        // Basic functionality:
        int rowCount(const QModelIndex& parent = QModelIndex()) const override;
        QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    private:
        BuildJobIssuesModelPrivate* d;
};

class BuildJobIssuesDelegate : public QStyledItemDelegate {
        Q_OBJECT

    public:
        explicit BuildJobIssuesDelegate(QObject* parent = nullptr);

    private:
        tPaintCalculator paintCalculator(const QStyleOptionViewItem& option, const QModelIndex& index, QPainter* painter = nullptr) const;

        // QAbstractItemDelegate interface
    public:
        void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
};

#endif // BUILDJOBISSUESMODEL_H
