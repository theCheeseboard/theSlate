#include "branchesmodel.h"
#include "gitintegration.h"

#include <QPainter>

extern void tintImage(QImage &image, QColor tint);

struct BranchesModelPrivate {
    GitIntegration* gi;
    bool includeActions;

    GitIntegration::BranchList shownBranches;

    struct Action {
        QString text;
        QIcon icon;
        QString key;
    };
    QList<Action> actions;
};

BranchesModel::BranchesModel(GitIntegration* integration, bool includeActions, QObject *parent)
    : QAbstractListModel(parent)
{
    d = new BranchesModelPrivate();
    d->gi = integration;
    d->includeActions = includeActions;

    if (includeActions) {
        d->actions.append({tr("New Branch"), QIcon::fromTheme("list-add", QIcon(":/icons/list-add.svg")), "new"});
    }

    connect(d->gi, &GitIntegration::branchesChanged, this, &BranchesModel::reloadData);
    reloadData();
}

BranchesModel::~BranchesModel() {
    delete d;
}

int BranchesModel::rowCount(const QModelIndex &parent) const
{
    // For list models only the root node (an invalid parent) should return the list's size. For all
    // other (valid) parents, rowCount() should return 0 so that it does not become a tree model.
    if (parent.isValid()) {
        return 0;
    }

    return d->shownBranches.count() + d->actions.count();
}

QVariant BranchesModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.row() < d->actions.count()) {
        BranchesModelPrivate::Action action = d->actions.at(index.row());
        switch (role) {
            case Qt::DisplayRole:
                return action.text;
            case Qt::DecorationRole:
                return action.icon;
            case Qt::UserRole:
                return action.key;
        }
    } else {
        GitIntegration::BranchPointer branch = d->shownBranches.at(index.row() - d->actions.count());
        switch (role) {
            case Qt::DisplayRole:
                return branch->name;
            case Qt::UserRole:
                if (branch->upstream.isNull()) {
                    return "";
                } else {
                    return branch->upstream->name;
                }
            case Qt::UserRole + 1:
                return QVariant::fromValue(branch);
        }
    }
    return QVariant();
}

void BranchesModel::reloadData() {
    d->shownBranches.clear();
    emit dataChanged(index(0), index(rowCount()));

    if (!d->gi->needsInit()) {
        //Get the branches we need to show
        d->shownBranches = d->gi->branches();
        emit dataChanged(index(0), index(rowCount()));
    }
}

QSize BranchesModelDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const {
    QSize size = option.rect.size();
    size.setHeight(24 * theLibsGlobal::getDPIScaling());
    return size;
}

void BranchesModelDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    GitIntegration::BranchPointer branch = index.data(Qt::UserRole + 1).value<GitIntegration::BranchPointer>();
    QColor foregroundCol;
    QColor grayCol;
    QColor backgroundCol;

    if (option.state & QStyle::State_Selected) {
        foregroundCol = option.palette.color(QPalette::HighlightedText);
        grayCol = option.palette.color(QPalette::Disabled, QPalette::HighlightedText);
        backgroundCol = option.palette.color(QPalette::Highlight);
    } else {
        foregroundCol = option.palette.color(QPalette::Text);
        grayCol = option.palette.color(QPalette::Disabled, QPalette::Text);
        backgroundCol = option.palette.color(QPalette::Base);
    }

    QFont f = option.font;
    QFontMetrics fm = option.fontMetrics;

    painter->setBrush(backgroundCol);
    painter->setPen(Qt::transparent);
    painter->drawRect(option.rect);
    painter->setBrush(Qt::transparent);

    QRect informationRect = option.rect;
    informationRect.setX(informationRect.height());
    informationRect.moveTop(informationRect.top() + 3);
    informationRect.setHeight(informationRect.height() - 6);
    informationRect.setWidth(informationRect.width() - 3);

    //Draw any decorations
    QIcon icon = index.data(Qt::DecorationRole).value<QIcon>();
    if (!icon.isNull()) {
        QRect iconRect = option.rect;
        iconRect.setWidth(informationRect.left());

        QRect realRect;
        realRect.setSize(QSize(16, 16) * theLibsGlobal::getDPIScaling());
        realRect.moveCenter(iconRect.center());

        QImage image = icon.pixmap(realRect.size()).toImage();
        tintImage(image, foregroundCol);
        painter->drawImage(realRect, image);
    }

    //Draw branch name
    QString name = index.data(Qt::DisplayRole).toString();
    QRect nameRect;
    nameRect.setWidth(informationRect.width());
    nameRect.setHeight(fm.height());
    nameRect.moveCenter(informationRect.center());
    nameRect.moveLeft(informationRect.left());

    painter->setPen(foregroundCol);
    painter->setFont(f);
    painter->drawText(nameRect, fm.elidedText(name, Qt::ElideRight, nameRect.width()));

    //Draw upstream branch if available
    QString upstream = index.data(Qt::UserRole).toString();
    if (upstream != "" && !branch.isNull()) {
        QRect upstreamRect;
        upstreamRect.setWidth(fm.width(upstream) + 6);
        upstreamRect.setHeight(option.rect.height());
        upstreamRect.moveTopRight(option.rect.topRight());

        painter->setPen(Qt::transparent);
        painter->setBrush(QColor(0, 150, 0));
        painter->drawRect(upstreamRect);

        QPolygon triangle;
        triangle.append(upstreamRect.topLeft());
        triangle.append(QPoint(upstreamRect.left(), upstreamRect.y() + upstreamRect.height()));
        triangle.append(QPoint(upstreamRect.left() - upstreamRect.height() / 2, upstreamRect.center().y()));
        painter->drawPolygon(triangle);

        QRect upstreamTextRect;
        upstreamTextRect.setWidth(fm.width(upstream) + 1);
        upstreamTextRect.setHeight(fm.height());
        upstreamTextRect.moveCenter(upstreamRect.center());

        painter->setPen(Qt::white);
        painter->setBrush(Qt::transparent);
        painter->drawText(upstreamTextRect, upstream);
    }

    //Draw current branch indicator
    if (gi->branch() == branch) {
        QRect indicatorRect;
        indicatorRect.setWidth(3 * theLibsGlobal::getDPIScaling());
        indicatorRect.setHeight(option.rect.height());
        indicatorRect.moveTopLeft(option.rect.topLeft());

        painter->setPen(Qt::transparent);
        painter->setBrush(QColor(0, 150, 0));
        painter->drawRect(indicatorRect);
    }
}
