#include "DataService.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QJsonDocument>
#include <QSqlError>
#include <QSqlQuery>
#include <QStringList>
#include <QVariant>
#include <QtMath>

#include <cmath>

DataService::DataService(QObject *parent)
    : QObject(parent)
    , m_currentProfile(new WindProfile(this))
    , m_deviceHealth(new DeviceHealth(this))
    , m_connectionName(QStringLiteral("radar-history-%1").arg(reinterpret_cast<quintptr>(this)))
    , m_lastStoredTimestampMs(0)
{
}

DataService::~DataService()
{
    if (m_database.isOpen()) m_database.close();
    m_database = QSqlDatabase();
    QSqlDatabase::removeDatabase(m_connectionName);
}

bool DataService::initialize(const QString &dataDirectory)
{
    QString directory = dataDirectory;
    if (directory.isEmpty()) directory = qEnvironmentVariable("RADAR_DATA_DIR");
    if (directory.isEmpty()) {
        directory = QDir(QCoreApplication::applicationDirPath()).absoluteFilePath(QStringLiteral("data"));
    }
    QDir dir;
    if (!dir.mkpath(directory)) {
        emit storageError(QStringLiteral("无法创建数据目录：%1").arg(directory));
        return false;
    }

    m_databasePath = QDir(directory).absoluteFilePath(QStringLiteral("radar-history.sqlite"));
    m_database = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), m_connectionName);
    m_database.setDatabaseName(m_databasePath);
    if (!m_database.open()) {
        emit storageError(QStringLiteral("无法打开 SQLite 数据库：%1").arg(m_database.lastError().text()));
        return false;
    }

    QSqlQuery pragma(m_database);
    pragma.exec(QStringLiteral("PRAGMA journal_mode=WAL"));
    pragma.exec(QStringLiteral("PRAGMA synchronous=NORMAL"));
    pragma.exec(QStringLiteral("PRAGMA temp_store=MEMORY"));
    pragma.exec(QStringLiteral("PRAGMA auto_vacuum=INCREMENTAL"));
    if (!createSchema()) return false;

    QSqlQuery latest(m_database);
    if (latest.exec(QStringLiteral("SELECT COALESCE(MAX(timestamp_ms), 0) FROM wind_profiles"))
        && latest.next()) {
        const qint64 latestTimestampMs = latest.value(0).toLongLong();
        m_lastStoredTimestampMs = qMin(latestTimestampMs, QDateTime::currentMSecsSinceEpoch());
    }
    return true;
}

bool DataService::createSchema()
{
    const QStringList statements = {
        QStringLiteral(
            "CREATE TABLE IF NOT EXISTS wind_profiles ("
            "timestamp_ms INTEGER PRIMARY KEY, height_agl_m REAL NOT NULL, "
            "wind_speed_mps REAL NOT NULL, wind_direction_deg REAL NOT NULL, "
            "confidence_pct REAL NOT NULL, valid_gates INTEGER NOT NULL, total_gates INTEGER NOT NULL, "
            "blind_ratio REAL NOT NULL, source_scan_id INTEGER NOT NULL, retrieval_method INTEGER NOT NULL, "
            "payload_codec TEXT NOT NULL, compressed_payload BLOB NOT NULL)"),
        QStringLiteral("CREATE INDEX IF NOT EXISTS idx_wind_profiles_time ON wind_profiles(timestamp_ms)"),
        QStringLiteral(
            "CREATE TABLE IF NOT EXISTS device_health ("
            "timestamp_ms INTEGER PRIMARY KEY, fault_bits INTEGER NOT NULL, cpu_temp_c REAL, fpga_temp_c REAL, "
            "storage_usage REAL, payload_codec TEXT NOT NULL, compressed_payload BLOB NOT NULL)"),
        QStringLiteral(
            "CREATE TABLE IF NOT EXISTS alarm_events ("
            "alarm_id TEXT PRIMARY KEY, first_seen_ms INTEGER, last_seen_ms INTEGER, resolved_ms INTEGER, "
            "severity INTEGER, source INTEGER, code TEXT, payload_codec TEXT NOT NULL, compressed_payload BLOB NOT NULL)"),
        QStringLiteral("CREATE INDEX IF NOT EXISTS idx_alarm_events_time ON alarm_events(first_seen_ms)")
    };
    for (const QString &statement : statements) {
        QSqlQuery query(m_database);
        if (!query.exec(statement)) {
            emit storageError(QStringLiteral("创建历史数据表失败：%1").arg(query.lastError().text()));
            return false;
        }
    }
    return true;
}

QByteArray DataService::compressedJson(const QJsonObject &object) const
{
    const QByteArray json = QJsonDocument(object).toJson(QJsonDocument::Compact);
    return qCompress(json, 6);
}

void DataService::updateWindProfile(WindProfile *profile)
{
    if (!profile) return;
    m_currentProfile->fromJson(profile->toJson());
    if (m_database.isOpen() && !persistWindProfile(profile)) {
        emit storageError(QStringLiteral("风廓线写入历史数据库失败"));
    }
    emit windProfileUpdated(m_currentProfile);
}

bool DataService::persistWindProfile(const WindProfile *profile)
{
    // History windows follow the local receive clock. The radar observation
    // timestamp remains intact in the compressed profile payload.
    const qint64 receiveTimeMs = QDateTime::currentMSecsSinceEpoch();
    const qint64 timestampMs = qMax(receiveTimeMs, m_lastStoredTimestampMs + 1);
    m_lastStoredTimestampMs = timestampMs;
    QSqlQuery query(m_database);
    query.prepare(QStringLiteral(
        "INSERT OR REPLACE INTO wind_profiles (timestamp_ms,height_agl_m,wind_speed_mps,wind_direction_deg,"
        "confidence_pct,valid_gates,total_gates,blind_ratio,source_scan_id,retrieval_method,payload_codec,compressed_payload) "
        "VALUES (?,?,?,?,?,?,?,?,?,?,?,?)"));
    query.addBindValue(timestampMs);
    query.addBindValue(profile->hubHeightM());
    query.addBindValue(profile->hubWindSpeedMps());
    query.addBindValue(profile->hubWindDirectionDeg());
    query.addBindValue(profile->confidence());
    query.addBindValue(profile->validGateCount());
    query.addBindValue(profile->gateCount());
    query.addBindValue(profile->blindRatio());
    query.addBindValue(profile->sourceScanId());
    query.addBindValue(static_cast<int>(profile->retrievalMethod()));
    query.addBindValue(QStringLiteral("qcompress-zlib-6"));
    query.addBindValue(compressedJson(profile->toJson()));
    if (!query.exec()) {
        emit storageError(query.lastError().text());
        return false;
    }
    emit historyStored(timestampMs);
    return true;
}

void DataService::updateDeviceHealth(DeviceHealth *health)
{
    if (!health) return;
    m_deviceHealth->fromJson(health->toJson());
    if (m_database.isOpen() && !persistDeviceHealth(health)) {
        emit storageError(QStringLiteral("设备遥测写入历史数据库失败"));
    }
    emit deviceHealthUpdated(m_deviceHealth);
}

bool DataService::persistDeviceHealth(const DeviceHealth *health)
{
    QSqlQuery query(m_database);
    query.prepare(QStringLiteral(
        "INSERT OR REPLACE INTO device_health (timestamp_ms,fault_bits,cpu_temp_c,fpga_temp_c,storage_usage,"
        "payload_codec,compressed_payload) VALUES (?,?,?,?,?,?,?)"));
    query.addBindValue(QDateTime::currentMSecsSinceEpoch());
    query.addBindValue(health->faultBits());
    query.addBindValue(health->cpuTemp());
    query.addBindValue(health->fpgaTemp());
    query.addBindValue(health->storageUsageRatio());
    query.addBindValue(QStringLiteral("qcompress-zlib-6"));
    query.addBindValue(compressedJson(health->toJson()));
    if (!query.exec()) {
        emit storageError(query.lastError().text());
        return false;
    }
    return true;
}

void DataService::addAlarm(AlarmEvent *alarm)
{
    if (!alarm) return;
    if (!m_alarms.contains(alarm)) m_alarms.prepend(alarm);
    if (m_database.isOpen() && !persistAlarm(alarm)) {
        emit storageError(QStringLiteral("告警事件写入历史数据库失败"));
    }
    emit alarmRaised(alarm);
}

bool DataService::persistAlarm(const AlarmEvent *alarm)
{
    QSqlQuery query(m_database);
    query.prepare(QStringLiteral(
        "INSERT OR REPLACE INTO alarm_events (alarm_id,first_seen_ms,last_seen_ms,resolved_ms,severity,source,code,"
        "payload_codec,compressed_payload) VALUES (?,?,?,?,?,?,?,?,?)"));
    query.addBindValue(alarm->alarmId());
    query.addBindValue(alarm->firstSeen().toMSecsSinceEpoch());
    query.addBindValue(alarm->lastSeen().toMSecsSinceEpoch());
    query.addBindValue(alarm->resolvedAt().isValid() ? alarm->resolvedAt().toMSecsSinceEpoch() : QVariant());
    query.addBindValue(static_cast<int>(alarm->severity()));
    query.addBindValue(static_cast<int>(alarm->source()));
    query.addBindValue(alarm->code());
    query.addBindValue(QStringLiteral("qcompress-zlib-6"));
    query.addBindValue(compressedJson(alarm->toJson()));
    if (!query.exec()) {
        emit storageError(query.lastError().text());
        return false;
    }
    return true;
}

QList<AlarmEvent *> DataService::activeAlarms() const
{
    QList<AlarmEvent *> active;
    for (AlarmEvent *alarm : m_alarms) {
        if (alarm && !alarm->isResolved()) active.append(alarm);
    }
    return active;
}

DataService::WindSeries DataService::queryWindSeries(int windowSeconds, int maximumPoints) const
{
    WindSeries result;
    if (!m_database.isOpen() || windowSeconds <= 0 || maximumPoints <= 0) return result;

    // Include the small monotonic adjustment used when multiple frames arrive
    // within the same millisecond.
    const qint64 nowMs = QDateTime::currentMSecsSinceEpoch();
    const qint64 endMs = nowMs + 1000;
    const qint64 startMs = nowMs - static_cast<qint64>(windowSeconds) * 1000;
    const qint64 bucketMs = qMax<qint64>(1000,
        qCeil(static_cast<double>(windowSeconds) * 1000.0 / maximumPoints));

    QSqlQuery query(m_database);
    query.prepare(QStringLiteral(
        "SELECT timestamp_ms,wind_speed_mps,wind_direction_deg FROM wind_profiles "
        "WHERE timestamp_ms>=? AND timestamp_ms<=? ORDER BY timestamp_ms"));
    query.addBindValue(startMs);
    query.addBindValue(endMs);
    if (!query.exec()) return result;

    qint64 currentBucket = -1;
    qint64 lastTimestamp = 0;
    double speedSum = 0.0;
    double directionSinSum = 0.0;
    double directionCosSum = 0.0;
    int count = 0;
    const auto flushBucket = [&]() {
        if (count <= 0) return;
        result.speedMps.append(QPointF(lastTimestamp, speedSum / count));
        double direction = qRadiansToDegrees(std::atan2(directionSinSum, directionCosSum));
        if (direction < 0.0) direction += 360.0;
        result.directionDeg.append(QPointF(lastTimestamp, direction));
    };

    while (query.next()) {
        const qint64 timestamp = query.value(0).toLongLong();
        const qint64 bucket = qMin<qint64>((timestamp - startMs) / bucketMs,
                                           maximumPoints - 1);
        if (currentBucket >= 0 && bucket != currentBucket) {
            flushBucket();
            speedSum = directionSinSum = directionCosSum = 0.0;
            count = 0;
        }
        currentBucket = bucket;
        lastTimestamp = timestamp;
        speedSum += query.value(1).toDouble();
        const double radians = qDegreesToRadians(query.value(2).toDouble());
        directionSinSum += std::sin(radians);
        directionCosSum += std::cos(radians);
        ++count;
    }
    flushBucket();
    return result;
}

QVector<QPointF> DataService::windSpeedHistory(int minutes) const
{
    return queryWindSeries(minutes * 60).speedMps;
}

QVector<double> DataService::windDirectionHistory(int minutes) const
{
    QVector<double> values;
    const QVector<QPointF> points = queryWindSeries(minutes * 60).directionDeg;
    values.reserve(points.size());
    for (const QPointF &point : points) values.append(point.y());
    return values;
}
