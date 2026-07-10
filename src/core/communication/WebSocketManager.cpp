#include "WebSocketManager.h"
#include <QJsonArray>

#ifdef HAS_WEBSOCKETS

WebSocketManager::WebSocketManager(QObject *parent)
    : QObject(parent)
    , m_socket(new QWebSocket(QString(), QWebSocketProtocol::VersionLatest, this))
    , m_reconnectTimer(new QTimer(this))
    , m_heartbeatTimer(new QTimer(this))
    , m_reconnectEnabled(true)
    , m_reconnectInterval(5000)
    , m_heartbeatInterval(30000)
{
    // 连接信号
    connect(m_socket, &QWebSocket::connected, this, &WebSocketManager::onConnected);
    connect(m_socket, &QWebSocket::disconnected, this, &WebSocketManager::onDisconnected);
    connect(m_socket, &QWebSocket::textMessageReceived, this, &WebSocketManager::onTextMessageReceived);
    connect(m_socket, &QWebSocket::binaryMessageReceived, this, &WebSocketManager::onBinaryMessageReceived);
    connect(m_socket, &QWebSocket::errorOccurred, this, &WebSocketManager::onError);

    // 连接定时器
    connect(m_reconnectTimer, &QTimer::timeout, this, &WebSocketManager::onReconnectTimer);
    connect(m_heartbeatTimer, &QTimer::timeout, this, &WebSocketManager::onHeartbeatTimer);
}

WebSocketManager::~WebSocketManager()
{
    disconnectFromServer();
}

bool WebSocketManager::connectToServer(const QUrl &url, const QString &token)
{
    if (m_socket->state() == QAbstractSocket::ConnectingState) {
        return false;
    }

    m_serverUrl = url;
    m_token = token;

    // 设置认证头
    if (!token.isEmpty()) {
        QNetworkRequest request(url);
        request.setRawHeader("Authorization", token.toUtf8());
        m_socket->open(request);
    } else {
        m_socket->open(url);
    }

    return true;
}

void WebSocketManager::disconnectFromServer()
{
    m_reconnectTimer->stop();
    m_heartbeatTimer->stop();
    m_socket->close();
}

bool WebSocketManager::isConnected() const
{
    return m_socket->state() == QAbstractSocket::ConnectedState;
}

void WebSocketManager::sendMessage(const QString &event, const QJsonObject &data)
{
    if (!isConnected()) {
        return;
    }

    QJsonObject message;
    message["event"] = event;
    message["data"] = data;

    QJsonDocument doc(message);
    m_socket->sendTextMessage(doc.toJson());
}

void WebSocketManager::subscribe(const QString &event)
{
    if (!m_subscriptions.contains(event)) {
        m_subscriptions.append(event);

        QJsonObject data;
        data["event"] = event;
        data["action"] = "subscribe";

        sendMessage("subscription", data);
    }
}

void WebSocketManager::unsubscribe(const QString &event)
{
    if (m_subscriptions.contains(event)) {
        m_subscriptions.removeOne(event);

        QJsonObject data;
        data["event"] = event;
        data["action"] = "unsubscribe";

        sendMessage("subscription", data);
    }
}

void WebSocketManager::setReconnectEnabled(bool enabled)
{
    m_reconnectEnabled = enabled;
}

void WebSocketManager::setReconnectInterval(int ms)
{
    m_reconnectInterval = ms;
}

void WebSocketManager::setHeartbeatInterval(int ms)
{
    m_heartbeatInterval = ms;
}

void WebSocketManager::onConnected()
{
    m_reconnectTimer->stop();
    m_heartbeatTimer->start(m_heartbeatInterval);

    // 重新订阅所有事件
    for (const auto &event : m_subscriptions) {
        QJsonObject data;
        data["event"] = event;
        data["action"] = "subscribe";
        sendMessage("subscription", data);
    }

    emit connected();
}

void WebSocketManager::onDisconnected()
{
    m_heartbeatTimer->stop();

    if (m_reconnectEnabled && !m_serverUrl.isEmpty()) {
        m_reconnectTimer->start(m_reconnectInterval);
    }

    emit disconnected();
}

void WebSocketManager::onTextMessageReceived(const QString &message)
{
    processMessage(message);
}

void WebSocketManager::onBinaryMessageReceived(const QByteArray &message)
{
    Q_UNUSED(message)
    // 暂不处理二进制消息
}

void WebSocketManager::onError(QAbstractSocket::SocketError error)
{
    Q_UNUSED(error)
    QString errorMsg = m_socket->errorString();
    emit errorOccurred(errorMsg);
}

void WebSocketManager::onReconnectTimer()
{
    if (!isConnected() && !m_serverUrl.isEmpty()) {
        connectToServer(m_serverUrl, m_token);
    }
}

void WebSocketManager::onHeartbeatTimer()
{
    sendHeartbeat();
}

void WebSocketManager::processMessage(const QString &message)
{
    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());
    if (doc.isNull() || !doc.isObject()) {
        return;
    }

    QJsonObject obj = doc.object();
    QString event = obj["event"].toString();
    QJsonObject data = obj["data"].toObject();

    // 处理心跳响应
    if (event == "heartbeat" || event == "pong") {
        return;
    }

    emit eventReceived(event, data);
}

void WebSocketManager::sendHeartbeat()
{
    if (isConnected()) {
        QJsonObject data;
        data["timestamp"] = QDateTime::currentMSecsSinceEpoch();
        sendMessage("ping", data);
    }
}

#else // HAS_WEBSOCKETS

// 没有 WebSockets 模块时的空实现
WebSocketManager::WebSocketManager(QObject *parent)
    : QObject(parent)
    , m_socket(nullptr)
    , m_reconnectTimer(new QTimer(this))
    , m_heartbeatTimer(new QTimer(this))
    , m_reconnectEnabled(true)
    , m_reconnectInterval(5000)
    , m_heartbeatInterval(30000)
{
}

WebSocketManager::~WebSocketManager() {}

bool WebSocketManager::connectToServer(const QUrl &url, const QString &token)
{
    Q_UNUSED(url)
    Q_UNUSED(token)
    qWarning() << "WebSocketManager: WebSockets module not available";
    return false;
}

void WebSocketManager::disconnectFromServer() {}
bool WebSocketManager::isConnected() const { return false; }
void WebSocketManager::sendMessage(const QString &event, const QJsonObject &data) { Q_UNUSED(event) Q_UNUSED(data) }
void WebSocketManager::subscribe(const QString &event) { Q_UNUSED(event) }
void WebSocketManager::unsubscribe(const QString &event) { Q_UNUSED(event) }
void WebSocketManager::setReconnectEnabled(bool enabled) { m_reconnectEnabled = enabled; }
void WebSocketManager::setReconnectInterval(int ms) { m_reconnectInterval = ms; }
void WebSocketManager::setHeartbeatInterval(int ms) { m_heartbeatInterval = ms; }
void WebSocketManager::onConnected() {}
void WebSocketManager::onDisconnected() {}
void WebSocketManager::onTextMessageReceived(const QString &message) { Q_UNUSED(message) }
void WebSocketManager::onBinaryMessageReceived(const QByteArray &message) { Q_UNUSED(message) }
void WebSocketManager::onError(QAbstractSocket::SocketError error) { Q_UNUSED(error) }
void WebSocketManager::onReconnectTimer() {}
void WebSocketManager::onHeartbeatTimer() {}
void WebSocketManager::processMessage(const QString &message) { Q_UNUSED(message) }
void WebSocketManager::sendHeartbeat() {}

#endif // HAS_WEBSOCKETS
