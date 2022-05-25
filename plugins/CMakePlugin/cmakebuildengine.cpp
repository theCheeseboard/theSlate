#include "cmakebuildengine.h"

#include "cmakerunconfiguration.h"
#include "project.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSharedPointer>
#include <QStandardPaths>
#include <tlogger.h>

struct CmakeBuildEnginePrivate {
        QString cmakePath;
};

CmakeBuildEngine::CmakeBuildEngine(QObject* parent) :
    BuildEngine{parent} {
    d = new CmakeBuildEnginePrivate();

    d->cmakePath = QStandardPaths::findExecutable("cmake");
}

CmakeBuildEngine::~CmakeBuildEngine() {
    delete d;
}

QString CmakeBuildEngine::cmakeExecutable() {
    return d->cmakePath;
}

QList<RunConfigurationPtr> CmakeBuildEngine::runConfigurationsFromConfigurationFile(QJsonObject configurationFile, ProjectPtr project) {
    if (d->cmakePath.isEmpty()) {
        tWarn("CmakeBuildEngine") << "CMake project detected but CMake not found";
        return {};
    }

    QList<RunConfigurationPtr> runConfigurations;
    for (const QString& key : configurationFile.keys()) {
        runConfigurations.append(RunConfigurationPtr(new CmakeRunConfiguration(project, this, key, configurationFile.value(key).toObject())));
    }
    return runConfigurations;
}

QList<RunConfigurationPtr> CmakeBuildEngine::discoverRunConfigurations(ProjectPtr project) {
    QDir runConfigurationCache = project->projectDir(Project::RunConfigurationCache);
    QFile cmakeFile(runConfigurationCache.absoluteFilePath("cmake.json"));
    if (runConfigurationCache.entryList().contains("cmake.json")) {
        // Read the CMake configuration file and create run configuration
        if (!cmakeFile.open(QFile::ReadOnly)) return {}; // Error reading CMake configuration file
        QByteArray jsonContents = cmakeFile.readAll();
        cmakeFile.close();

        QJsonParseError parseError;
        QJsonObject jsonObject = QJsonDocument::fromJson(jsonContents, &parseError).object();
        if (parseError.error != QJsonParseError::NoError) return {}; // Error parsing JSON file
        return runConfigurationsFromConfigurationFile(jsonObject, project);
    }

    QDir projectDir = project->projectDir();
    if (!projectDir.entryList().contains("CMakeLists.txt")) return {}; // Not a CMake controlled project

    // Generate a default CMake configuration file
    QJsonObject rootObject;

    QJsonObject debugObject;
    debugObject.insert("cmake-args", QJsonArray({"-DCMAKE_BUILD_TYPE=Debug", "-GNinja"}));
    rootObject.insert("Debug", debugObject);

    QJsonObject releaseObject;
    releaseObject.insert("cmake-args", QJsonArray({"-DCMAKE_BUILD_TYPE=Release", "-GNinja"}));
    rootObject.insert("Release", releaseObject);

    cmakeFile.open(QFile::WriteOnly);
    cmakeFile.write(QJsonDocument(rootObject).toJson());
    cmakeFile.close();

    return runConfigurationsFromConfigurationFile(rootObject, project);
}
