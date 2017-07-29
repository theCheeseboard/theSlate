#ifndef EXCEPTION_H
#define EXCEPTION_H

#include <QString>

struct Exception {
    QString errorType;
    QString description;
    int line;
};

#endif // EXCEPTION_H
