#include "PyArtWindService.h"

#include "domain/RangeGate.h"
#include "domain/WindProfile.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QSaveFile>
#include <QUuid>
#include <QtMath>

PyArtWindService::PyArtWindService(QObject *parent)
    : QObject(parent)
    , m_pythonExecutable(QStringLiteral("python"))
    , m_healthCheck(false)
{
    const QDir applicationDir(QCoreApplication::applicationDirPath());
    const QString installedPath = applicationDir.absoluteFilePath(QStringLiteral("pyart-service"));
    const QString developmentPath = applicationDir.absoluteFilePath(QStringLiteral("../../../pyart-service"));
    m_serviceDirectory = QDir(installedPath).exists() ? installedPath : QDir(developmentPath).absolutePath();

    connect(&m_process, &QProcess::readyReadStandardOutput,
            this, &PyArtWindService::onReadyReadStandardOutput);
    connect(&m_process, &QProcess::readyReadStandardError,
            this, &PyArtWindService::onReadyReadStandardError);
    connect(&m_process, qOverload<int, QProcess::ExitStatus>(&QProcess::finished),
            this, &PyArtWindService::onFinished);
}

void PyArtWindService::setPythonExecutable(const QString &executable)
{
    if (!executable.trimmed().isEmpty()) m_pythonExecutable = executable.trimmed();
}

void PyArtWindService::setServiceDirectory(const QString &directory)
{
    m_serviceDirectory = QDir(directory).absolutePath();
}

bool PyArtWindService::isBusy() const
{
    return m_process.state() != QProcess::NotRunning;
}

bool PyArtWindService::checkEnvironment()
{
    return startProcess({QStringLiteral("-m"), QStringLiteral("radar_pyart_service.worker"),
                         QStringLiteral("--health")}, true);
}

bool PyArtWindService::submit(const QJsonObject &request)
{
    if (isBusy()) {
        emit taskFailed(QStringLiteral("E_SERVICE_BUSY"), QStringLiteral("Py-ART 算法服务正在处理其他任务。"));
        return false;
    }
    if (request.value(QStringLiteral("taskId")).toString().isEmpty()) {
        emit taskFailed(QStringLiteral("E_TASK_ID"), QStringLiteral("Py-ART 任务缺少 taskId。"));
        return false;
    }

    QString requestDirectory = request.value(QStringLiteral("output")).toObject()
        .value(QStringLiteral("directory")).toString();
    if (requestDirectory.isEmpty()) {
        requestDirectory = QDir(m_serviceDirectory).absoluteFilePath(QStringLiteral("data/pyart-output"));
    }
    requestDirectory = QDir(requestDirectory).absoluteFilePath(QStringLiteral(".requests"));
    QDir().mkpath(requestDirectory);
    const QString requestPath = requestDirectory + QLatin1Char('/')
        + QUuid::createUuid().toString(QUuid::WithoutBraces) + QStringLiteral(".json");
    QSaveFile file(requestPath);
    if (!file.open(QIODevice::WriteOnly)
        || file.write(QJsonDocument(request).toJson(QJsonDocument::Indented)) < 0
        || !file.commit()) {
        emit taskFailed(QStringLiteral("E_REQUEST_WRITE"), QStringLiteral("无法写入 Py-ART 任务文件。"));
        return false;
    }

    return startProcess({QStringLiteral("-m"), QStringLiteral("radar_pyart_service.worker"),
                         QStringLiteral("--request"), QDir::toNativeSeparators(requestPath)}, false);
}

QJsonObject PyArtWindService::createSyntheticValidationRequest(const QString &outputDirectory)
{
    constexpr int rayCount = 36;
    constexpr int gateCount = 30;
    constexpr double elevationDeg = 15.0;
    constexpr double eastwardWindMps = 6.0;
    constexpr double northwardWindMps = 3.0;

    QJsonArray azimuths;
    QJsonArray elevations;
    QJsonArray ranges;
    QJsonArray velocities;
    QJsonArray cnrValues;
    for (int gate = 0; gate < gateCount; ++gate) ranges.append((gate + 1) * 40.0);
    for (int ray = 0; ray < rayCount; ++ray) {
        const double azimuthDeg = ray * (360.0 / rayCount);
        const double azimuthRad = qDegreesToRadians(azimuthDeg);
        const double elevationRad = qDegreesToRadians(elevationDeg);
        const double radialVelocity = qCos(elevationRad)
            * (eastwardWindMps * qSin(azimuthRad) + northwardWindMps * qCos(azimuthRad));
        azimuths.append(azimuthDeg);
        elevations.append(elevationDeg);
        QJsonArray rayVelocities;
        QJsonArray rayCnr;
        for (int gate = 0; gate < gateCount; ++gate) {
            rayVelocities.append(radialVelocity);
            rayCnr.append(15.0);
        }
        velocities.append(rayVelocities);
        cnrValues.append(rayCnr);
    }

    const QString taskId = QStringLiteral("pyart-ui-validation-%1")
        .arg(QDateTime::currentDateTimeUtc().toString(QStringLiteral("yyyyMMdd-HHmmss")));
    QJsonObject observation{
        {QStringLiteral("timestampUtc"), QDateTime::currentDateTimeUtc().toString(Qt::ISODate)},
        {QStringLiteral("instrumentName"), QStringLiteral("RadarUpperComputer-PyART-Validation")},
        {QStringLiteral("latitudeDeg"), 39.9042},
        {QStringLiteral("longitudeDeg"), 116.4074},
        {QStringLiteral("altitudeM"), 50.0},
        {QStringLiteral("azimuthDeg"), azimuths},
        {QStringLiteral("elevationDeg"), elevations},
        {QStringLiteral("rangeM"), ranges},
        {QStringLiteral("radialVelocityMps"), velocities},
        {QStringLiteral("cnrDb"), cnrValues}
    };
    QJsonObject algorithm{
        {QStringLiteral("name"), QStringLiteral("pyart-vad-browning")},
        {QStringLiteral("validRayMin"), 16},
        {QStringLiteral("minimumCnrDb"), -22.0}
    };
    QJsonArray formats{QStringLiteral("json"), QStringLiteral("csv"), QStringLiteral("netcdf"),
                       QStringLiteral("png"), QStringLiteral("cfradial")};
    QJsonObject output{{QStringLiteral("directory"), outputDirectory},
                       {QStringLiteral("formats"), formats}};
    return {
        {QStringLiteral("schemaVersion"), QStringLiteral("radar.wind-retrieval.request/1.0")},
        {QStringLiteral("taskId"), taskId},
        {QStringLiteral("taskType"), QStringLiteral("wind-retrieval")},
        {QStringLiteral("observation"), observation},
        {QStringLiteral("algorithm"), algorithm},
        {QStringLiteral("output"), output}
    };
}

bool PyArtWindService::applyResultToProfile(const QJsonObject &result, WindProfile *profile,
                                            QString *errorMessage)
{
    if (!profile || !result.value(QStringLiteral("success")).toBool()) {
        if (errorMessage) *errorMessage = result.value(QStringLiteral("errorMessage")).toString(
            QStringLiteral("Py-ART 结果无效。"));
        return false;
    }

    const QJsonArray levels = result.value(QStringLiteral("levels")).toArray();
    if (levels.isEmpty()) {
        if (errorMessage) *errorMessage = QStringLiteral("Py-ART 结果不包含风廓线层。" );
        return false;
    }

    profile->clearRangeGates();
    profile->setTimestampUtc(QDateTime::fromString(
        result.value(QStringLiteral("observationTimeUtc")).toString(), Qt::ISODate));
    profile->setGateCount(levels.size());
    profile->setRetrievalMethod(WindRetrievalMethod::PyArtVad);
    double previousHeight = 0.0;
    for (int index = 0; index < levels.size(); ++index) {
        const QJsonObject level = levels.at(index).toObject();
        auto *gate = new RangeGate(profile);
        const double heightAgl = level.value(QStringLiteral("heightAglM")).toDouble();
        gate->setGateIndex(level.value(QStringLiteral("levelIndex")).toInt(index));
        gate->setHeightM(heightAgl);
        gate->setDistanceM(heightAgl);
        gate->setWindSpeedMps(level.value(QStringLiteral("windSpeedMps")).toDouble());
        gate->setWindDirectionDeg(level.value(QStringLiteral("windFromDirectionDeg")).toDouble());
        gate->setEastwardWindMps(level.value(QStringLiteral("eastwardWindMps")).toDouble(qQNaN()));
        gate->setNorthwardWindMps(level.value(QStringLiteral("northwardWindMps")).toDouble(qQNaN()));
        gate->setUpwardWindMps(qQNaN());
        gate->setValidBeamCount(36);
        gate->setRetrievalResidualMps(qQNaN());
        gate->setConfidence(level.value(QStringLiteral("confidencePct")).toDouble());
        gate->setStatusFlags({level.value(QStringLiteral("qualityFlag")).toInt(1) == 0
                                  ? StatusCode::Valid : StatusCode::Invalid});
        profile->addRangeGate(gate);
        if (index == 1) profile->setRangeResolutionM(qRound(heightAgl - previousHeight));
        previousHeight = heightAgl;
    }

    const QJsonObject first = levels.first().toObject();
    profile->setHubHeightM(first.value(QStringLiteral("heightAglM")).toDouble());
    profile->setHubWindSpeedMps(first.value(QStringLiteral("windSpeedMps")).toDouble());
    profile->setHubWindDirectionDeg(first.value(QStringLiteral("windFromDirectionDeg")).toDouble());
    emit profile->dataUpdated();
    return true;
}

void PyArtWindService::onReadyReadStandardOutput()
{
    m_stdoutBuffer.append(m_process.readAllStandardOutput());
}

void PyArtWindService::onReadyReadStandardError()
{
    const QString message = QString::fromUtf8(m_process.readAllStandardError()).trimmed();
    if (!message.isEmpty()) emit processLog(message);
}

void PyArtWindService::onFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    m_stdoutBuffer.append(m_process.readAllStandardOutput());
    QString parseError;
    const QJsonObject response = parseLastJsonLine(m_stdoutBuffer, &parseError);
    m_stdoutBuffer.clear();
    emit busyChanged(false);

    if (m_healthCheck) {
        const bool available = exitStatus == QProcess::NormalExit && exitCode == 0
            && response.value(QStringLiteral("available")).toBool();
        emit availabilityChanged(available, response);
        m_healthCheck = false;
        return;
    }
    if (response.isEmpty()) {
        emit taskFailed(QStringLiteral("E_PROCESS_OUTPUT"), parseError);
        return;
    }
    if (!response.value(QStringLiteral("success")).toBool()) {
        emit taskFailed(response.value(QStringLiteral("errorCode")).toString(QStringLiteral("E_ALGORITHM")),
                        response.value(QStringLiteral("errorMessage")).toString(QStringLiteral("Py-ART 任务失败。")));
        return;
    }
    emit resultReady(response);
}

bool PyArtWindService::startProcess(const QStringList &arguments, bool healthCheck)
{
    if (isBusy()) return false;
    if (!QDir(m_serviceDirectory).exists()) {
        emit taskFailed(QStringLiteral("E_SERVICE_PATH"),
                        QStringLiteral("Py-ART 服务目录不存在：%1").arg(m_serviceDirectory));
        return false;
    }
    m_healthCheck = healthCheck;
    m_stdoutBuffer.clear();
    QProcessEnvironment environment = QProcessEnvironment::systemEnvironment();
    const QString sourcePath = QDir(m_serviceDirectory).absoluteFilePath(QStringLiteral("src"));
    const QString oldPythonPath = environment.value(QStringLiteral("PYTHONPATH"));
    environment.insert(QStringLiteral("PYTHONPATH"), oldPythonPath.isEmpty()
                           ? sourcePath : sourcePath + QDir::listSeparator() + oldPythonPath);
    environment.insert(QStringLiteral("PYTHONUTF8"), QStringLiteral("1"));
    environment.insert(QStringLiteral("MPLBACKEND"), QStringLiteral("Agg"));
    environment.insert(QStringLiteral("PYART_QUIET"), QStringLiteral("1"));
    m_process.setProcessEnvironment(environment);
    m_process.setWorkingDirectory(m_serviceDirectory);
    m_process.start(m_pythonExecutable, arguments);
    if (!m_process.waitForStarted(3000)) {
        emit taskFailed(QStringLiteral("E_PROCESS_START"), m_process.errorString());
        return false;
    }
    emit busyChanged(true);
    return true;
}

QJsonObject PyArtWindService::parseLastJsonLine(const QByteArray &data, QString *errorMessage) const
{
    const QList<QByteArray> lines = data.split('\n');
    for (auto iterator = lines.crbegin(); iterator != lines.crend(); ++iterator) {
        const QByteArray line = iterator->trimmed();
        if (!line.startsWith('{')) continue;
        QJsonParseError parseError;
        const QJsonDocument document = QJsonDocument::fromJson(line, &parseError);
        if (parseError.error == QJsonParseError::NoError && document.isObject()) return document.object();
    }
    if (errorMessage) *errorMessage = QStringLiteral("Py-ART 未返回有效 JSON。原始输出：%1")
        .arg(QString::fromUtf8(data).right(500));
    return {};
}
