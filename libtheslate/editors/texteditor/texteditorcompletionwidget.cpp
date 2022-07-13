#include "texteditorcompletionwidget.h"

#include "lsp/languageserverprocess.h"
#include <QKeyEvent>
#include <QPainter>
#include <QScrollBar>
#include <QTimer>
#include <libcontemporary_global.h>
#include <texteditor.h>

struct TextEditorCompletionWidgetPrivate {
        TextEditor* editor;
        LanguageServerProcess* lsp;

        QScrollBar* verticalScrollBar;

        QList<LanguageServerProcess::CompletionItem> items;
        int currentSelection = 0;
        int xOffset = 0;

        int characterDistance = 0;
};

TextEditorCompletionWidget::TextEditorCompletionWidget(TextEditor* editor, LanguageServerProcess* lsp, QWidget* parent) :
    QWidget{parent} {
    d = new TextEditorCompletionWidgetPrivate();
    //    lsp->completion(d->editor->editor()->currentFile(), anchorStart);
    d->editor = editor;
    d->lsp = lsp;

    d->verticalScrollBar = new QScrollBar(Qt::Vertical, this);
    d->verticalScrollBar->setFixedWidth(this->style()->pixelMetric(QStyle::PM_ScrollBarExtent));
    connect(d->verticalScrollBar, &QScrollBar::valueChanged, this, QOverload<>::of(&TextEditor::update));
    d->verticalScrollBar->show();

    connect(d->editor->verticalScrollBar(), &QScrollBar::valueChanged, this, &TextEditorCompletionWidget::updatePosition);
    connect(d->editor->horizontalScrollBar(), &QScrollBar::valueChanged, this, &TextEditorCompletionWidget::updatePosition);
    d->editor->installEventFilter(this);

    this->setAutoFillBackground(true);
    this->updateCompletions();
    this->show();
}

TextEditorCompletionWidget::~TextEditorCompletionWidget() {
    delete d;
}

QCoro::Task<> TextEditorCompletionWidget::updateCompletions() {
    auto [isIncomplete, items] = co_await d->lsp->completion(d->editor->currentFile(), d->editor->caretAnchorStart(0));
    d->items = items;
    if (d->items.isEmpty()) {
        this->deleteLater();
        co_return;
    }

    d->currentSelection = 0;
    for (auto i = 0; i < d->items.length(); i++) {
        if (d->items.at(i).preselect) {
            d->currentSelection = i;
            break;
        }
    }

    this->updatePosition();
    this->update();
}

void TextEditorCompletionWidget::updatePosition() {
    auto anchorStart = d->editor->caretAnchorStart(0);
    anchorStart.rx() -= d->characterDistance;

    QPoint editorTopLeft;
    auto characterRect = d->editor->characterRect(anchorStart);
    editorTopLeft.setY(characterRect.bottom());
    editorTopLeft.setX(0);
    this->move(this->parentWidget()->mapFromGlobal(d->editor->mapToGlobal(editorTopLeft)));
    this->resize(d->editor->width(), qMin(300, this->itemHeight() * d->items.length()));

    d->verticalScrollBar->move(this->width() - d->verticalScrollBar->width(), 0);
    d->verticalScrollBar->setFixedHeight(this->height());
    d->verticalScrollBar->setPageStep(this->height());
    d->verticalScrollBar->setMaximum(this->itemHeight() * d->items.length() - this->height());

    d->xOffset = characterRect.left();
}

void TextEditorCompletionWidget::commitCompletion(int index) {
    auto completion = d->items.at(index);
    d->editor->replaceText(completion.acceptReplaceStart, completion.acceptReplaceEnd, completion.acceptText);
    this->deleteLater();
}

int TextEditorCompletionWidget::itemHeight() {
    return this->fontMetrics().height() + SC_DPI_W(6, this);
}

void TextEditorCompletionWidget::ensureSelectedVisible() {
    if ((d->currentSelection + 1) * this->itemHeight() - d->verticalScrollBar->value() > this->height()) {
        d->verticalScrollBar->setValue((d->currentSelection + 1) * this->itemHeight());
    }
    if (d->currentSelection * this->itemHeight() - d->verticalScrollBar->value() < 0) {
        d->verticalScrollBar->setValue(d->currentSelection * this->itemHeight() - this->height());
    }
}

bool TextEditorCompletionWidget::editorKeyPress(QKeyEvent* event) {
    if (event->key() == Qt::Key_Escape) {
        this->deleteLater();
        return true;
    } else if (event->key() == Qt::Key_Up) {
        d->currentSelection--;
        if (d->currentSelection < 0) d->currentSelection = d->items.length() - 1;
        this->ensureSelectedVisible();
        this->update();
        return true;
    } else if (event->key() == Qt::Key_Down) {
        d->currentSelection++;
        if (d->currentSelection >= d->items.length()) d->currentSelection = 0;
        this->ensureSelectedVisible();
        this->update();
        return true;
    } else if (event->key() == Qt::Key_Backspace) {
        d->characterDistance--;
        if (d->characterDistance < 0) {
            this->deleteLater();
        } else {
            QTimer::singleShot(0, this, &TextEditorCompletionWidget::updateCompletions);
        }
        return false;
    } else if (!event->text().isEmpty() && d->lsp->completionCommitCharacters().contains(event->text().at(0))) {
        commitCompletion(d->currentSelection);
        return true;
    } else {
        d->characterDistance += event->text().length();
        QTimer::singleShot(0, this, &TextEditorCompletionWidget::updateCompletions);
        return false;
    }
}

void TextEditorCompletionWidget::paintEvent(QPaintEvent* event) {
    QPainter painter(this);

    int yOffset = -d->verticalScrollBar->value();
    for (auto i = 0; i < d->items.length(); i++) {
        painter.save();
        painter.setPen(this->palette().color(QPalette::WindowText));

        auto item = d->items.at(i);

        QRect rect;
        rect.setHeight(this->itemHeight());
        rect.setWidth(this->width());
        rect.moveLeft(0);
        rect.moveTop(yOffset);

        if (d->currentSelection == i) {
            painter.fillRect(rect, this->palette().color(QPalette::Highlight));
            painter.setPen(this->palette().color(QPalette::HighlightedText));
        }

        QString iconName;
        switch (item.kind) {
            case LanguageServerProcess::CompletionItem::Kind::Text:
                break;
            case LanguageServerProcess::CompletionItem::Kind::Method:
            case LanguageServerProcess::CompletionItem::Kind::Function:
                iconName = "code-function";
                break;
            case LanguageServerProcess::CompletionItem::Kind::Constructor:
                break;
            case LanguageServerProcess::CompletionItem::Kind::Field:
            case LanguageServerProcess::CompletionItem::Kind::Variable:
            case LanguageServerProcess::CompletionItem::Kind::Property:
                iconName = "code-variable";
                break;
            case LanguageServerProcess::CompletionItem::Kind::Class:
            case LanguageServerProcess::CompletionItem::Kind::Interface:
            case LanguageServerProcess::CompletionItem::Kind::Module:
                break;
            case LanguageServerProcess::CompletionItem::Kind::Unit:
                break;
            case LanguageServerProcess::CompletionItem::Kind::Value:
                break;
            case LanguageServerProcess::CompletionItem::Kind::Enum:
                break;
            case LanguageServerProcess::CompletionItem::Kind::Keyword:
                break;
            case LanguageServerProcess::CompletionItem::Kind::Snippet:
                break;
            case LanguageServerProcess::CompletionItem::Kind::Color:
                break;
            case LanguageServerProcess::CompletionItem::Kind::File:
                break;
            case LanguageServerProcess::CompletionItem::Kind::Reference:
                break;
            case LanguageServerProcess::CompletionItem::Kind::Folder:
                break;
            case LanguageServerProcess::CompletionItem::Kind::EnumMember:
                break;
            case LanguageServerProcess::CompletionItem::Kind::Constant:
                break;
            case LanguageServerProcess::CompletionItem::Kind::Struct:
                break;
            case LanguageServerProcess::CompletionItem::Kind::Event:
                break;
            case LanguageServerProcess::CompletionItem::Kind::Operator:
                break;
            case LanguageServerProcess::CompletionItem::Kind::TypeParameter:
                break;
        }

        QRect iconRect;
        iconRect.setSize(SC_DPI_WT(QSize(16, 16), QSize, this));
        iconRect.moveCenter(rect.center());
        iconRect.moveRight(d->xOffset - SC_DPI_W(6, this));

        auto iconImage = QIcon::fromTheme(iconName).pixmap(iconRect.size()).toImage();
        libContemporaryCommon::tintImage(iconImage, painter.pen().color());
        painter.drawImage(iconRect, iconImage);

        if (item.tags.contains(LanguageServerProcess::CompletionItem::Tag::Deprecated)) {
            QFont font = painter.font();
            font.setStrikeOut(true);
            painter.setFont(font);
        }

        QRect textRect;
        textRect.setHeight(painter.fontMetrics().height());
        textRect.moveCenter(rect.center());
        textRect.moveLeft(d->xOffset);
        textRect.setWidth(painter.fontMetrics().horizontalAdvance(item.label));
        painter.drawText(textRect, Qt::AlignLeft | Qt::AlignCenter, item.label);

        if (d->currentSelection == i) {
            painter.setPen(this->palette().color(QPalette::HighlightedText));
        } else {
            painter.setPen(this->palette().color(QPalette::Disabled, QPalette::WindowText));
        }

        QRect detailRect;
        detailRect.setHeight(painter.fontMetrics().height());
        detailRect.moveCenter(rect.center());
        detailRect.moveLeft(textRect.right() + SC_DPI_W(9, this));
        detailRect.setWidth(painter.fontMetrics().horizontalAdvance(item.detail));
        painter.drawText(detailRect, Qt::AlignLeft | Qt::AlignCenter, item.detail);

        yOffset = rect.bottom() + 1;

        painter.restore();
    }

    painter.setPen(libContemporaryCommon::lineColor(this->palette().color(QPalette::WindowText)));
    painter.drawLine(0, this->height() - 1, this->width(), this->height() - 1);
}

bool TextEditorCompletionWidget::eventFilter(QObject* watched, QEvent* event) {
    if (watched == d->editor) {
        if (event->type() == QEvent::KeyPress) {
            return this->editorKeyPress(static_cast<QKeyEvent*>(event));
        } else if (event->type() == QEvent::MouseButtonPress) {
            this->deleteLater();
            return false;
        }
    }
    return false;
}
