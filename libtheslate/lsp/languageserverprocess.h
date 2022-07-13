#ifndef LANGUAGESERVERPROCESS_H
#define LANGUAGESERVERPROCESS_H

#include "libtheslate_global.h"
#include <QCoroTask>
#include <QJsonObject>
#include <QPoint>
#include <QProcess>
#include <tuple>

struct LanguageServerProcessPrivate;
class LIBTHESLATE_EXPORT LanguageServerProcess : public QProcess {
        Q_OBJECT

    public:
        explicit LanguageServerProcess(QString languageServerType, QObject* parent = nullptr);
        ~LanguageServerProcess();

        struct Diagnostic {
                enum class Severity : int {
                    Error = 1,
                    Warning = 2,
                    Information = 3,
                    Hint = 4
                };

                QString code;
                QString message;
                Severity severity;
                QPoint start;
                QPoint end;
        };

        struct TextDocumentContentChangeEvent {
                QPoint start;
                QPoint end;
                QString text;
        };

        QList<QChar> completionTriggerCharacters();
        QList<QChar> completionCommitCharacters();
        bool prefersIncrementalSync();

        QCoro::Task<> startLanguageServer(QJsonObject extraInitialisationOptions = QJsonObject());
        static QString serverTypeForFileName(QString fileName);

        QCoro::Task<QJsonValue> call(QString method, QJsonValue params);
        void notify(QString method, QJsonValue params);

        void addWorkspaceFolder(QUrl documentUri, QString name);
        void didOpen(QUrl documentUri, QString languageId, int version, QString text);
        void didChange(QUrl documentUri, int version, QString text);
        void didChange(QUrl documentUri, int version, QList<TextDocumentContentChangeEvent> events);
        void didClose(QUrl documentUri);

        struct HoverResponse {
                QPoint start;
                QPoint end;
                QString text;
        };

        struct CompletionItem {
                enum class Kind : int {
                    Text = 1,
                    Method = 2,
                    Function = 3,
                    Constructor = 4,
                    Field = 5,
                    Variable = 6,
                    Class = 7,
                    Interface = 8,
                    Module = 9,
                    Property = 10,
                    Unit = 11,
                    Value = 12,
                    Enum = 13,
                    Keyword = 14,
                    Snippet = 15,
                    Color = 16,
                    File = 17,
                    Reference = 18,
                    Folder = 19,
                    EnumMember = 20,
                    Constant = 21,
                    Struct = 22,
                    Event = 23,
                    Operator = 24,
                    TypeParameter = 25
                };

                enum class Tag : int {
                    Deprecated = 1
                };

                QString detail;
                QString filterText;
                Kind kind;
                QString label;
                QString sortText;
                bool preselect;
                QList<Tag> tags;

                QString acceptText;
                QPoint acceptReplaceStart;
                QPoint acceptReplaceEnd;
        };

        QCoro::Task<HoverResponse> hover(QUrl documentUri, QPoint position);
        QCoro::Task<std::tuple<bool, QList<CompletionItem>>> completion(QUrl documentUri, QPoint position);

        QList<Diagnostic> diagnostics(QUrl url);

    signals:
        void jsonRpcResponse(QJsonObject response);
        void jsonRpcNotification(QJsonObject notification);

        void publishDiagnostics(QUrl url);

    private:
        LanguageServerProcessPrivate* d;

        void writeJsonRpc(QJsonObject object);
        void handleJsonRpcNotification(QJsonObject notification);

        QJsonObject joinObject(QList<QJsonObject> objects);

        QJsonObject encodeTextDocumentPositionParams(QUrl uri, QPoint position);
        QJsonObject encodeTextDocumentIdentifier(QUrl uri);
        QJsonObject encodeVersionedTextDocumentIdentifier(QUrl uri, int version);

        QPoint decodePosition(QJsonObject position);
        QJsonObject encodePosition(QPoint position);

        std::tuple<QPoint, QPoint> decodeRange(QJsonObject range);
        QJsonObject encodeRange(QPoint start, QPoint end);
};

#endif // LANGUAGESERVERPROCESS_H
