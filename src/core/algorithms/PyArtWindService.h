#ifndef PYARTWINDSERVICE_H
#define PYARTWINDSERVICE_H

#include <QJsonObject>
#include <QObject>
#include <QProcess>
#include <QString>

class WindProfile;

class PyArtWindService : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool busy READ isBusy NOTIFY busyChanged)

public:
    explicit PyArtWindService(QObject *parent = nullptr);

    void setPythonExecutable(const QString &executable);
    void setServiceDirectory(const QString &directory);
    bool isBusy() const;
    bool checkEnvironment();
    bool submit(const QJsonObject &request);
    static QJsonObject createSyntheticValidationRequest(const QString &outputDirectory);

    static bool applyResultToProfile(const QJsonObject &result, WindProfile *profile,
                                     QString *errorMessage = nullptr);

signals:
    void availabilityChanged(bool available, const QJsonObject &details);
    void busyChanged(bool busy);
    void resultReady(const QJsonObject &result);
    void taskFailed(const QString &errorCode, const QString &message);
    void processLog(const QString &message);

private slots:
    void onReadyReadStandardOutput();
    void onReadyReadStandardError();
    void onFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    bool startProcess(const QStringList &arguments, bool healthCheck);
    QJsonObject parseLastJsonLine(const QByteArray &data, QString *errorMessage) const;

    QProcess m_process;
    QString m_pythonExecutable;
    QString m_serviceDirectory;
    QByteArray m_stdoutBuffer;
    bool m_healthCheck;
};

#endif // PYARTWINDSERVICE_H
