#include "services/DataService.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QSqlDatabase>
#include <QSqlQuery>

#include <cmath>
#include <iostream>

namespace {
bool nearNorth(double degrees)
{
    return degrees < 2.0 || degrees > 358.0;
}
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    const QString directory = QDir(QCoreApplication::applicationDirPath())
        .absoluteFilePath(QStringLiteral("data-service-test"));
    QDir().mkpath(directory);
    const QString databasePath = QDir(directory).absoluteFilePath(QStringLiteral("radar-history.sqlite"));
    QFile::remove(databasePath);
    QFile::remove(databasePath + QStringLiteral("-wal"));
    QFile::remove(databasePath + QStringLiteral("-shm"));

    DataService service;
    if (!service.initialize(directory)) {
        std::cerr << "database initialization failed\n";
        return 1;
    }

    const qint64 now = QDateTime::currentMSecsSinceEpoch();
    WindProfile first;
    first.setTimestampUtc(QDateTime::fromMSecsSinceEpoch(now - 500));
    first.setHubHeightM(100.0);
    first.setHubWindSpeedMps(8.0);
    first.setHubWindDirectionDeg(359.0);
    first.setSourceScanId(1);
    service.updateWindProfile(&first);

    WindProfile second;
    second.setTimestampUtc(QDateTime::fromMSecsSinceEpoch(now - 400));
    second.setHubHeightM(100.0);
    second.setHubWindSpeedMps(12.0);
    second.setHubWindDirectionDeg(1.0);
    second.setSourceScanId(2);
    service.updateWindProfile(&second);

    const DataService::WindSeries series = service.queryWindSeries(60, 1);
    if (series.speedMps.size() != 1 || std::abs(series.speedMps.first().y() - 10.0) > 0.01) {
        std::cerr << "wind speed bucket average is incorrect\n";
        return 2;
    }
    if (series.directionDeg.size() != 1 || !nearNorth(series.directionDeg.first().y())) {
        std::cerr << "wind direction circular average is incorrect\n";
        return 3;
    }

    const QString connectionName = QStringLiteral("history-verification");
    {
        QSqlDatabase verification = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), connectionName);
        verification.setDatabaseName(databasePath);
        if (!verification.open()) return 4;
        QSqlQuery query(verification);
        if (!query.exec(QStringLiteral("SELECT payload_codec,compressed_payload FROM wind_profiles LIMIT 1"))
            || !query.next()) return 5;
        if (query.value(0).toString() != QStringLiteral("qcompress-zlib-6")) return 6;
        const QByteArray restored = qUncompress(query.value(1).toByteArray());
        if (QJsonDocument::fromJson(restored).object().isEmpty()) return 7;
        verification.close();
    }
    QSqlDatabase::removeDatabase(connectionName);

    std::cout << "history database, compression and circular aggregation passed\n";
    return 0;
}
