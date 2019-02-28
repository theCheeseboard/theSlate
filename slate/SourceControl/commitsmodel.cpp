#include "commitsmodel.h"

#include "gitintegration.h"
#include <QPainter>

struct CommitsModelPrivate {
    GitIntegration* gi;

    GitIntegration::CommitList shownCommits;
};

CommitsModel::CommitsModel(GitIntegration* integration, QObject *parent)
    : QAbstractListModel(parent)
{
    d = new CommitsModelPrivate();
    d->gi = integration;

    connect(d->gi, &GitIntegration::commitsChanged, this, &CommitsModel::reloadData);
    connect(d->gi, &GitIntegration::commitInformationAvailable, this, [=](QString commitHash) {
        for (int i = 0; i < d->shownCommits.count(); i++) {
            if (d->shownCommits.at(i)->hash == commitHash) {
                emit dataChanged(index(i), index(i));
                return;
            }
        }
    });

    reloadData();
}

CommitsModel::~CommitsModel() {
    delete d;
}

int CommitsModel::rowCount(const QModelIndex &parent) const
{
    // For list models only the root node (an invalid parent) should return the list's size. For all
    // other (valid) parents, rowCount() should return 0 so that it does not become a tree model.
    if (parent.isValid()) {
        return 0;
    }

    return d->shownCommits.count();
}

QVariant CommitsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    GitIntegration::CommitPointer commit = d->shownCommits.at(index.row());
    switch (role) {
        case Qt::DisplayRole:
            return commit->hash;
        case Qt::UserRole:
            return QVariant::fromValue(commit);
    }
    return QVariant();
}

void CommitsModel::reloadData() {
    d->shownCommits.clear();
    emit dataChanged(index(0), index(rowCount()));

    if (!d->gi->needsInit()) {
        //Cache the HEAD commit
        d->gi->getCommit("HEAD", true, true);

        d->gi->commits()->then([=](GitIntegration::CommitList commits) {
            d->shownCommits = commits;
            emit dataChanged(index(0), index(rowCount()));
        });
    }
}

QSize CommitsModelDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const {
    QSize size;
    size.setWidth(option.rect.width());

    int height = 6; //padding
    height += option.fontMetrics.height() * 2;
    size.setHeight(height);
    return size;
}

void CommitsModelDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    GitIntegration::CommitPointer commit = index.data(Qt::UserRole).value<GitIntegration::CommitPointer>();

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
    informationRect.setX(3);
    informationRect.moveTop(informationRect.top() + 3);
    informationRect.setHeight(informationRect.height() - 6);
    informationRect.setWidth(informationRect.width() - 3);

    //Draw short hash
    QString shortHash = commit->hash.left(7);
    QRect shortHashRect;
    shortHashRect.setTop(informationRect.top());
    shortHashRect.setWidth(fm.width(shortHash));
    shortHashRect.moveRight(informationRect.right());
    shortHashRect.setHeight(fm.height());

    painter->setFont(f);
    painter->setPen(grayCol);
    painter->drawText(shortHashRect, shortHash);

    //Make sure the commit is populated further before trying to draw anything else
    if (!commit->populated) {
        //Populate the commit in the background
        commit = gi->getCommit(commit->hash, true, false);
        //return;
    }

    //Draw committer's name
    QRect nameRect;
    nameRect.setTopLeft(informationRect.topLeft());
    nameRect.setWidth(informationRect.width());
    nameRect.setHeight(fm.height());

    QFont boldFont = f;
    boldFont.setBold(true);
    painter->setPen(foregroundCol);
    painter->setFont(boldFont);
    painter->drawText(nameRect, QFontMetrics(boldFont).elidedText(commit->committer, Qt::ElideRight, nameRect.width()));

    //Draw commit message
    QRect commitMessageRect;
    commitMessageRect.setTop(nameRect.bottom());
    commitMessageRect.setLeft(informationRect.left());
    commitMessageRect.setWidth(informationRect.width());
    commitMessageRect.setHeight(fm.height());

    painter->setFont(f);
    painter->drawText(commitMessageRect, fm.elidedText(commit->message, Qt::ElideRight, commitMessageRect.width()));
}
