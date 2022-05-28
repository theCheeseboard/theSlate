#include "buildjobissuesmodel.h"

#include <QFileInfo>
#include <QPainter>
#include <QUrl>

struct BuildJobIssuesModelPrivate {
        BuildJobPtr buildJob;
};

BuildJobIssuesModel::BuildJobIssuesModel(BuildJobPtr buildJob, QObject* parent) :
    QAbstractListModel(parent) {
    d = new BuildJobIssuesModelPrivate();
    d->buildJob = buildJob;

    connect(buildJob.data(), &BuildJob::buildIssuesAppendedTo, this, [=] {
        emit dataChanged(index(0), index(rowCount()));
    });
}

BuildJobIssuesModel::~BuildJobIssuesModel() {
    delete d;
}

int BuildJobIssuesModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid())
        return 0;

    return d->buildJob->buildIssues().length();
}

QVariant BuildJobIssuesModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid())
        return QVariant();

    auto buildIssue = d->buildJob->buildIssues().at(index.row());
    switch (role) {
        case Qt::DisplayRole:
        case IssueMessage:
            return buildIssue.message;
        case IssueType:
            return buildIssue.issueType;
        case EditorUrl:
            {
                auto url = QUrl::fromLocalFile(buildIssue.file);
                return url;
            }
        case FileName:
            return QFileInfo(buildIssue.file).fileName();
    }

    return QVariant();
}

BuildJobIssuesDelegate::BuildJobIssuesDelegate(QObject* parent) :
    QStyledItemDelegate(parent) {
}

tPaintCalculator BuildJobIssuesDelegate::paintCalculator(const QStyleOptionViewItem& option, const QModelIndex& index, QPainter* painter) const {
    tPaintCalculator paintCalculator;
    paintCalculator.setPainter(painter);
    paintCalculator.setDrawBounds(option.rect);
    paintCalculator.setLayoutDirection(option.direction);

    QRect severityRect = option.rect;
    severityRect.setWidth(SC_DPI_W(3, option.widget));
    paintCalculator.addRect(severityRect, [=](QRectF drawBounds) {
        QColor col;
        switch (index.data(BuildJobIssuesModel::IssueType).value<BuildJob::BuildIssue::Type>()) {
            case BuildJob::BuildIssue::Informational:
                col = Qt::transparent;
                break;
            case BuildJob::BuildIssue::Warning:
                col = QColor(255, 200, 0);
                break;
            case BuildJob::BuildIssue::Error:
                col = QColor(200, 0, 0);
                break;
        }

        painter->fillRect(drawBounds, col);
    });

    QString message = index.data(BuildJobIssuesModel::IssueMessage).toString();
    QRect messageRect = option.rect;
    messageRect.setLeft(severityRect.right() + SC_DPI_W(6, option.widget));
    messageRect.setWidth(painter->fontMetrics().horizontalAdvance(message));
    paintCalculator.addRect(messageRect, [=](QRectF drawBounds) {
        painter->setPen(option.palette.color(QPalette::WindowText));
        painter->drawText(drawBounds, Qt::AlignLeft | Qt::AlignVCenter, message);
    });

    QString filename = index.data(BuildJobIssuesModel::FileName).toString();
    QRect filenameRect = option.rect;
    filenameRect.setLeft(messageRect.right() + SC_DPI_W(6, option.widget));
    filenameRect.setWidth(painter->fontMetrics().horizontalAdvance(filename));
    paintCalculator.addRect(filenameRect, [=](QRectF drawBounds) {
        painter->setPen(option.palette.color(QPalette::Disabled, QPalette::WindowText));
        painter->drawText(drawBounds, Qt::AlignLeft | Qt::AlignVCenter, filename);
    });

    return paintCalculator;
}

void BuildJobIssuesDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const {
    painter->save();
    option.widget->style()->drawPrimitive(QStyle::PE_PanelItemViewItem, &option, painter, option.widget);
    painter->restore();

    painter->save();
    this->paintCalculator(option, index, painter).performPaint();
    painter->restore();
}
