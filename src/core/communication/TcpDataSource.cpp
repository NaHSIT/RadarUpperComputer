#include "TcpDataSource.h"

#include <QNetworkProxy>

TcpDataSource::TcpDataSource(QObject *parent)
    : QObject(parent)
    , m_socket(new QTcpSocket(this))
    , m_reconnectTimer(new QTimer(this))
    , m_state(ConnectionState::Offline)
    , m_reconnectEnabled(true)
    , m_reconnectInterval(5000)
    , m_timeout(3000)
    , m_port(0)
{
    // Radar devices are addressed on a private LAN and must never inherit a
    // desktop or IDE proxy configuration.
    m_socket->setProxy(QNetworkProxy::NoProxy);
    connect(m_socket, &QTcpSocket::connected, this, &TcpDataSource::onConnected);
    connect(m_socket, &QTcpSocket::disconnected, this, &TcpDataSource::onDisconnected);
    connect(m_socket, &QTcpSocket::readyRead, this, &TcpDataSource::onReadyRead);
    connect(m_socket, &QTcpSocket::errorOccurred, this, &TcpDataSource::onError);
    connect(m_reconnectTimer, &QTimer::timeout, this, &TcpDataSource::onReconnectTimer);
}

TcpDataSource::~TcpDataSource()
{
    disconnectFromHost();
}

bool TcpDataSource::connectToHost(const QString &host, quint16 port)
{
    const auto socketState = m_socket->state();
    if (socketState == QAbstractSocket::HostLookupState
        || socketState == QAbstractSocket::ConnectingState
        || socketState == QAbstractSocket::ConnectedState) {
        return false;
    }

    m_reconnectTimer->stop();
    m_host = host;
    m_port = port;

    setState(ConnectionState::Connecting);
    m_socket->connectToHost(host, port);

    return true;
}

void TcpDataSource::disconnectFromHost()
{
    m_reconnectTimer->stop();
    m_socket->abort();
    setState(ConnectionState::Offline);
}

bool TcpDataSource::isConnected() const
{
    return m_socket->state() == QAbstractSocket::ConnectedState;
}

bool TcpDataSource::sendBytes(const QByteArray &data)
{
    if (!isConnected()) {
        return false;
    }

    qint64 written = m_socket->write(data);
    return written == data.size();
}

void TcpDataSource::setReconnectEnabled(bool enabled)
{
    m_reconnectEnabled = enabled;
}

void TcpDataSource::setReconnectInterval(int ms)
{
    m_reconnectInterval = ms;
}

void TcpDataSource::setTimeout(int ms)
{
    m_timeout = ms;
}

void TcpDataSource::onConnected()
{
    setState(ConnectionState::Online);
    m_reconnectTimer->stop();
}

void TcpDataSource::onDisconnected()
{
    setState(ConnectionState::Offline);

    if (m_reconnectEnabled && !m_host.isEmpty()) {
        m_reconnectTimer->start(m_reconnectInterval);
    }
}

void TcpDataSource::onReadyRead()
{
    QByteArray data = m_socket->readAll();
    if (!data.isEmpty()) {
        emit bytesReceived(data);
    }
}

void TcpDataSource::onError(QAbstractSocket::SocketError error)
{
    QString errorMsg = m_socket->errorString();
    emit errorOccurred(errorMsg);

    if (error == QAbstractSocket::SocketTimeoutError) {
        if (m_state == ConnectionState::Connecting) {
            setState(ConnectionState::Offline);
            if (m_socket->state() != QAbstractSocket::UnconnectedState) {
                m_socket->abort();
            }
        } else {
            setState(ConnectionState::DataTimeout);
        }
    } else if (m_state == ConnectionState::Connecting
               || error == QAbstractSocket::ConnectionRefusedError
               || error == QAbstractSocket::HostNotFoundError
               || error == QAbstractSocket::NetworkError) {
        setState(ConnectionState::Offline);
        // QTcpSocket may emit errorOccurred before it leaves ConnectingState.
        // Abort the old attempt so a manual retry cannot overlap it.
        if (m_socket->state() != QAbstractSocket::UnconnectedState) {
            m_socket->abort();
        }
    } else {
        setState(ConnectionState::ProtocolError);
    }
}

void TcpDataSource::onReconnectTimer()
{
    if (m_state == ConnectionState::Offline && !m_host.isEmpty()) {
        connectToHost(m_host, m_port);
    }
}

void TcpDataSource::setState(ConnectionState state)
{
    if (m_state != state) {
        m_state = state;
        emit stateChanged(state);
    }
}
