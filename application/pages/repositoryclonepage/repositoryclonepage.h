#ifndef REPOSITORYCLONEPAGE_H
#define REPOSITORYCLONEPAGE_H

#include "../passthroughpage/passthroughpage.h"
#include <objects/forward_declares.h>

namespace Ui {
    class RepositoryClonePage;
}

struct RepositoryClonePagePrivate;
class RepositoryClonePage : public PassthroughPage {
        Q_OBJECT

    public:
        explicit RepositoryClonePage(RepositoryPtr repo, QWidget* parent = nullptr);
        ~RepositoryClonePage();

    private:
        Ui::RepositoryClonePage* ui;
        RepositoryClonePagePrivate* d;

        // AbstractPage interface
    public:
};

#endif // REPOSITORYCLONEPAGE_H
