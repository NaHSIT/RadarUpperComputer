#include <QApplication>
#include <QMainWindow>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QSpinBox>
#include <QGroupBox>
#include <QStatusBar>
#include <QComboBox>
#include <QTextEdit>
#include <QDateTime>
#include <QGridLayout>
#include <QRadioButton>
#include <QSplitter>
#include <QTableWidget>
#include <QHeaderView>
#include <QFrame>

#include "communication/networkmanager.h"
#include "communication/frame.h"

// 专业配色方案
const QString STYLE_SHEET = R"(
    QMainWindow { background-color: #f5f6fa; }
    QGroupBox {
        font-weight: bold;
        font-size: 12px;
        color: #2c3e50;
        border: 1px solid #dcdde1;
        border-radius: 6px;
        margin-top: 12px;
        padding-top: 16px;
        background-color: white;
    }
    QGroupBox::title {
        subcontrol-origin: margin;
        left: 12px;
        padding: 0 8px;
        background-color: #3498db;
        color: white;
        border-radius: 4px;
    }
    QLabel {
        color: #2c3e50;
        font-size: 12px;
    }
    QLineEdit, QSpinBox, QComboBox {
        padding: 6px;
        border: 1px solid #dcdde1;
        border-radius: 4px;
        background-color: white;
        font-size: 12px;
    }
    QLineEdit:focus, QSpinBox:focus, QComboBox:focus {
        border: 2px solid #3498db;
    }
    QPushButton {
        padding: 8px 16px;
        border-radius: 4px;
        font-weight: bold;
        font-size: 12px;
        border: none;
    }
    QPushButton:disabled {
        background-color: #bdc3c7;
        color: #7f8c8d;
    }
    QTableWidget {
        border: 1px solid #dcdde1;
        border-radius: 4px;
        gridline-color: #ecf0f1;
        background-color: white;
        alternate-background-color: #f8f9fa;
    }
    QTableWidget::item {
        padding: 6px;
    }
    QTableWidget::item:selected {
        background-color: #3498db;
        color: white;
    }
    QHeaderView::section {
        background-color: #34495e;
        color: white;
        padding: 8px;
        border: none;
        font-weight: bold;
    }
    QTextEdit {
        border: 1px solid #dcdde1;
        border-radius: 4px;
        background-color: #2c3e50;
        color: #ecf0f1;
        font-family: Consolas;
        font-size: 11px;
    }
    QStatusBar {
        background-color: #34495e;
        color: white;
    }
)";

class RadarMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    RadarMainWindow(QWidget *parent = nullptr) : QMainWindow(parent)
    {
        setWindowTitle("雷达测风系统 - 监控终端 V1.0");
        resize(1400, 900);
        setStyleSheet(STYLE_SHEET);

        m_network = new NetworkManager(this);
        m_connected = false;
        m_measuring = false;

        setupUI();
        setupConnections();
    }

private slots:
    void onConnectClicked()
    {
        QString ip = m_ipEdit->text();
        int port = m_portSpin->value();
        statusBar()->showMessage("正在连接 " + ip + ":" + QString::number(port) + "...");
        m_network->connectToHost(ip, port, NetworkManager::TCP);
    }

    void onDisconnectClicked()
    {
        m_network->disconnectFromHost();
        m_connected = false;
        m_measuring = false;
        updateUI();
    }

    void onStartMeasurement()
    {
        if (m_network->sendStartMeasure()) {
            m_measuring = true;
            updateUI();
            m_logText->append("[TX] 启动测量命令");
        }
    }

    void onStopMeasurement()
    {
        if (m_network->sendStopMeasure()) {
            m_measuring = false;
            updateUI();
            m_logText->append("[TX] 停止测量命令");
        }
    }

    void onSwitchBeam()
    {
        int beamIndex = m_beamCombo->currentIndex();
        if (m_network->sendSwitchBeam(beamIndex)) {
            m_beamValue->setText(QString("%1 (°%2)").arg(beamIndex).arg(beamIndex * 72));
            m_logText->append("[TX] 切换波束: " + QString::number(beamIndex));
        }
    }

    void onStatusQuery()
    {
        if (m_network->sendStatusQuery()) {
            m_logText->append("[TX] 状态查询");
        }
    }

    void onVersionQuery()
    {
        if (m_network->sendVersionQuery()) {
            m_logText->append("[TX] 版本查询");
        }
    }

    void onConnected()
    {
        m_connected = true;
        updateUI();
        statusBar()->showMessage("已连接: " + m_ipEdit->text() + ":" + QString::number(m_portSpin->value()));
        m_connIndicator->setStyleSheet("background-color: #27ae60; border-radius: 10px; min-width: 20px; max-width: 20px; min-height: 20px; max-height: 20px;");
        m_connStatus->setText("已连接");
        m_connStatus->setStyleSheet("color: #27ae60; font-weight: bold; font-size: 14px;");
        m_logText->append("[SYS] 连接成功");
    }

    void onDisconnected()
    {
        m_connected = false;
        m_measuring = false;
        updateUI();
        statusBar()->showMessage("已断开连接");
        m_connIndicator->setStyleSheet("background-color: #e74c3c; border-radius: 10px; min-width: 20px; max-width: 20px; min-height: 20px; max-height: 20px;");
        m_connStatus->setText("未连接");
        m_connStatus->setStyleSheet("color: #e74c3c; font-weight: bold; font-size: 14px;");
        m_logText->append("[SYS] 连接断开");
    }

    void onDataReceived(const QByteArray& data)
    {
        m_rxBytes += data.size();
    }

    void onFrameReceived(uint16_t command, uint32_t sequence, const QByteArray& payload)
    {
        switch (command) {
            case 0x0080:
                m_logText->append("[RX] 确认响应");
                break;
            case 0x0003:
                parseStatusResponse(payload);
                break;
            case 0x0007:
                m_versionValue->setText(QString::fromLatin1(payload));
                m_logText->append("[RX] 版本: " + QString::fromLatin1(payload));
                break;
            case 0x0301:
                parseWindData(payload);
                break;
            default:
                m_logText->append("[RX] 命令: 0x" + QString::number(command, 16));
                break;
        }
    }

    void onConnectionError(const QString& error)
    {
        m_logText->append("[ERR] " + error);
    }

    void onConnectionStateChanged(NetworkManager::ConnectionState state)
    {
        QString stateStr;
        switch (state) {
            case NetworkManager::DISCONNECTED: stateStr = "未连接"; break;
            case NetworkManager::CONNECTING: stateStr = "连接中..."; break;
            case NetworkManager::CONNECTED: stateStr = "已连接"; break;
            case NetworkManager::ERROR_STATE: stateStr = "错误"; break;
        }
        m_connStatus->setText(stateStr);
    }

    void onStatisticsUpdated(double sendRate, double recvRate)
    {
        m_statsLabel->setText(QString("TX: %1 KB/s  RX: %2 KB/s")
                             .arg(sendRate / 1024, 0, 'f', 1)
                             .arg(recvRate / 1024, 0, 'f', 1));
    }

private:
    void parseStatusResponse(const QByteArray& payload)
    {
        if (payload.size() < 15) return;
        const char* data = payload.constData();
        int state = data[0];
        float temp = *reinterpret_cast<const float*>(data + 7);
        float voltage = *reinterpret_cast<const float*>(data + 11);

        QString stateStr = (state == 3) ? "测量中" : (state == 2) ? "运行中" : "空闲";
        m_stateValue->setText(stateStr);
        m_tempValue->setText(QString("%1 ℃").arg(temp, 0, 'f', 1));
        m_voltValue->setText(QString("%1 V").arg(voltage, 0, 'f', 2));
        m_logText->append("[RX] 状态: " + stateStr);
    }

    void parseWindData(const QByteArray& payload)
    {
        if (payload.size() < 32) return;
        const char* data = payload.constData();
        int offset = 8;

        uint8_t beamIndex = data[offset++];
        uint16_t gateCount = (data[offset] << 8) | data[offset + 1];
        offset += 2;
        float resolution;
        memcpy(&resolution, data + offset, 4);
        offset += 4;
        float maxRange;
        memcpy(&maxRange, data + offset, 4);
        offset += 4;
        offset += 3;

        m_beamValue->setText(QString("%1 (°%2)").arg(beamIndex).arg(beamIndex * 72));

        QVector<float> speeds;
        for (int i = 0; i < 30 && offset + 4 <= payload.size(); i++) {
            float speed;
            memcpy(&speed, data + offset, 4);
            speeds.append(speed);
            offset += 4;
        }

        QVector<float> dirs;
        for (int i = 0; i < 30 && offset + 4 <= payload.size(); i++) {
            float dir;
            memcpy(&dir, data + offset, 4);
            dirs.append(dir);
            offset += 4;
        }

        offset += 30 * 4;

        QVector<int> confs;
        for (int i = 0; i < 30 && offset < payload.size(); i++) {
            confs.append(data[offset++]);
        }

        m_windTable->setRowCount(0);
        for (int i = 0; i < speeds.size(); i++) {
            m_windTable->insertRow(i);
            m_windTable->setItem(i, 0, new QTableWidgetItem(QString::number((i + 1) * (int)resolution)));
            m_windTable->setItem(i, 1, new QTableWidgetItem(QString::number(speeds[i], 'f', 2)));
            m_windTable->setItem(i, 2, new QTableWidgetItem(QString::number(dirs[i], 'f', 1)));
            m_windTable->setItem(i, 3, new QTableWidgetItem(QString::number(confs[i])));
        }

        if (!speeds.isEmpty()) {
            float avg = 0;
            for (float s : speeds) avg += s;
            avg /= speeds.size();
            m_avgSpeedValue->setText(QString("%1 m/s").arg(avg, 0, 'f', 2));
        }

        m_dataCount++;
        m_dataCountValue->setText(QString::number(m_dataCount));
        m_logText->append("[RX] 风场: 波束" + QString::number(beamIndex) + ", " + QString::number(speeds.size()) + "层");
    }

    void updateUI()
    {
        m_connectBtn->setEnabled(!m_connected);
        m_disconnectBtn->setEnabled(m_connected);
        m_startBtn->setEnabled(m_connected && !m_measuring);
        m_stopBtn->setEnabled(m_connected && m_measuring);
        m_beamCombo->setEnabled(m_connected);
        m_statusBtn->setEnabled(m_connected);
        m_versionBtn->setEnabled(m_connected);
    }

    void setupUI()
    {
        QWidget *central = new QWidget;
        setCentralWidget(central);
        QVBoxLayout *mainLayout = new QVBoxLayout(central);
        mainLayout->setSpacing(8);
        mainLayout->setContentsMargins(12, 12, 12, 12);

        // 标题栏
        QWidget *header = new QWidget;
        header->setStyleSheet("background-color: #34495e; border-radius: 6px; padding: 8px;");
        QHBoxLayout *headerLayout = new QHBoxLayout(header);
        QLabel *title = new QLabel("📡 雷达测风系统监控终端");
        title->setStyleSheet("color: white; font-size: 18px; font-weight: bold;");
        headerLayout->addWidget(title);
        headerLayout->addStretch();
        m_connIndicator = new QLabel;
        m_connIndicator->setStyleSheet("background-color: #e74c3c; border-radius: 10px; min-width: 20px; max-width: 20px; min-height: 20px; max-height: 20px;");
        headerLayout->addWidget(m_connIndicator);
        m_connStatus = new QLabel("未连接");
        m_connStatus->setStyleSheet("color: #e74c3c; font-weight: bold; font-size: 14px;");
        headerLayout->addWidget(m_connStatus);
        mainLayout->addWidget(header);

        // 主内容区
        QSplitter *splitter = new QSplitter(Qt::Vertical);

        // 上半部分
        QWidget *topWidget = new QWidget;
        QHBoxLayout *topLayout = new QHBoxLayout(topWidget);
        topLayout->setSpacing(12);

        // 左侧：连接和控制
        QWidget *leftPanel = new QWidget;
        QVBoxLayout *leftLayout = new QVBoxLayout(leftPanel);
        leftLayout->setSpacing(12);

        // 连接配置
        QGroupBox *connGroup = new QGroupBox("连接配置");
        QGridLayout *connLayout = new QGridLayout(connGroup);
        connLayout->setSpacing(8);
        connLayout->addWidget(new QLabel("IP地址:"), 0, 0);
        m_ipEdit = new QLineEdit("192.168.163.128");
        connLayout->addWidget(m_ipEdit, 0, 1);
        connLayout->addWidget(new QLabel("端口号:"), 1, 0);
        m_portSpin = new QSpinBox;
        m_portSpin->setRange(1, 65535);
        m_portSpin->setValue(5000);
        connLayout->addWidget(m_portSpin, 1, 1);
        QHBoxLayout *btnLayout = new QHBoxLayout;
        m_connectBtn = new QPushButton("连接");
        m_connectBtn->setStyleSheet("background-color: #27ae60; color: white;");
        btnLayout->addWidget(m_connectBtn);
        m_disconnectBtn = new QPushButton("断开");
        m_disconnectBtn->setStyleSheet("background-color: #e74c3c; color: white;");
        m_disconnectBtn->setEnabled(false);
        btnLayout->addWidget(m_disconnectBtn);
        connLayout->addLayout(btnLayout, 2, 0, 1, 2);
        leftLayout->addWidget(connGroup);

        // 雷达控制
        QGroupBox *ctrlGroup = new QGroupBox("雷达控制");
        QGridLayout *ctrlLayout = new QGridLayout(ctrlGroup);
        ctrlLayout->setSpacing(8);
        QHBoxLayout *ctrlBtnLayout = new QHBoxLayout;
        m_startBtn = new QPushButton("▶ 启动测量");
        m_startBtn->setStyleSheet("background-color: #3498db; color: white;");
        m_startBtn->setEnabled(false);
        ctrlBtnLayout->addWidget(m_startBtn);
        m_stopBtn = new QPushButton("⏹ 停止测量");
        m_stopBtn->setStyleSheet("background-color: #95a5a6; color: white;");
        m_stopBtn->setEnabled(false);
        ctrlBtnLayout->addWidget(m_stopBtn);
        ctrlLayout->addLayout(ctrlBtnLayout, 0, 0, 1, 2);
        ctrlLayout->addWidget(new QLabel("波束选择:"), 1, 0);
        m_beamCombo = new QComboBox;
        m_beamCombo->addItems({"波束0 (0°)", "波束1 (72°)", "波束2 (144°)", "波束3 (216°)", "波束4 (288°)"});
        m_beamCombo->setEnabled(false);
        ctrlLayout->addWidget(m_beamCombo, 1, 1);
        QHBoxLayout *queryBtnLayout = new QHBoxLayout;
        m_statusBtn = new QPushButton("查询状态");
        m_statusBtn->setStyleSheet("background-color: #9b59b6; color: white;");
        m_statusBtn->setEnabled(false);
        queryBtnLayout->addWidget(m_statusBtn);
        m_versionBtn = new QPushButton("查询版本");
        m_versionBtn->setStyleSheet("background-color: #8e44ad; color: white;");
        m_versionBtn->setEnabled(false);
        queryBtnLayout->addWidget(m_versionBtn);
        ctrlLayout->addLayout(queryBtnLayout, 2, 0, 1, 2);
        leftLayout->addWidget(ctrlGroup);

        leftLayout->addStretch();
        topLayout->addWidget(leftPanel, 1);

        // 中间：系统状态
        QWidget *midPanel = new QWidget;
        QVBoxLayout *midLayout = new QVBoxLayout(midPanel);
        midLayout->setSpacing(12);

        QGroupBox *sysGroup = new QGroupBox("系统状态");
        QGridLayout *sysLayout = new QGridLayout(sysGroup);
        sysLayout->setSpacing(10);

        sysLayout->addWidget(new QLabel("连接状态:"), 0, 0);
        m_connStatusLabel = new QLabel("--");
        m_connStatusLabel->setStyleSheet("font-weight: bold;");
        sysLayout->addWidget(m_connStatusLabel, 0, 1);

        sysLayout->addWidget(new QLabel("当前波束:"), 1, 0);
        m_beamValue = new QLabel("0 (°0)");
        m_beamValue->setStyleSheet("font-weight: bold; color: #3498db;");
        sysLayout->addWidget(m_beamValue, 1, 1);

        sysLayout->addWidget(new QLabel("系统状态:"), 2, 0);
        m_stateValue = new QLabel("--");
        m_stateValue->setStyleSheet("font-weight: bold;");
        sysLayout->addWidget(m_stateValue, 2, 1);

        sysLayout->addWidget(new QLabel("温度:"), 3, 0);
        m_tempValue = new QLabel("-- ℃");
        sysLayout->addWidget(m_tempValue, 3, 1);

        sysLayout->addWidget(new QLabel("电压:"), 4, 0);
        m_voltValue = new QLabel("-- V");
        sysLayout->addWidget(m_voltValue, 4, 1);

        sysLayout->addWidget(new QLabel("固件版本:"), 5, 0);
        m_versionValue = new QLabel("--");
        sysLayout->addWidget(m_versionValue, 5, 1);

        sysLayout->addWidget(new QLabel("数据包数:"), 6, 0);
        m_dataCountValue = new QLabel("0");
        m_dataCountValue->setStyleSheet("font-weight: bold; color: #e74c3c;");
        sysLayout->addWidget(m_dataCountValue, 6, 1);

        midLayout->addWidget(sysGroup);
        midLayout->addStretch();
        topLayout->addWidget(midPanel, 1);

        // 右侧：风场数据
        QWidget *rightPanel = new QWidget;
        QVBoxLayout *rightLayout = new QVBoxLayout(rightPanel);
        rightLayout->setSpacing(12);

        QGroupBox *windGroup = new QGroupBox("风场数据");
        QVBoxLayout *windLayout = new QVBoxLayout(windGroup);

        // 平均风速
        QWidget *avgWidget = new QWidget;
        QHBoxLayout *avgLayout = new QHBoxLayout(avgWidget);
        avgLayout->setContentsMargins(0, 0, 0, 0);
        QLabel *avgLabel = new QLabel("平均风速:");
        avgLabel->setStyleSheet("font-size: 14px;");
        m_avgSpeedValue = new QLabel("-- m/s");
        m_avgSpeedValue->setStyleSheet("font-size: 24px; font-weight: bold; color: #e74c3c;");
        avgLayout->addWidget(avgLabel);
        avgLayout->addStretch();
        avgLayout->addWidget(m_avgSpeedValue);
        windLayout->addWidget(avgWidget);

        // 风场表格
        m_windTable = new QTableWidget;
        m_windTable->setColumnCount(4);
        m_windTable->setHorizontalHeaderLabels({"距离(m)", "风速(m/s)", "风向(°)", "置信度(%)"});
        m_windTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        m_windTable->setAlternatingRowColors(true);
        m_windTable->setEditTriggers(QTableWidget::NoEditTriggers);
        m_windTable->setSelectionBehavior(QTableWidget::SelectRows);
        m_windTable->verticalHeader()->setVisible(false);
        windLayout->addWidget(m_windTable);

        rightLayout->addWidget(windGroup);
        topLayout->addWidget(rightPanel, 2);

        splitter->addWidget(topWidget);

        // 下半部分：日志
        QWidget *bottomWidget = new QWidget;
        QVBoxLayout *bottomLayout = new QVBoxLayout(bottomWidget);
        QGroupBox *logGroup = new QGroupBox("通信日志");
        QVBoxLayout *logLayout = new QVBoxLayout(logGroup);
        m_logText = new QTextEdit;
        m_logText->setReadOnly(true);
        m_logText->setMaximumHeight(120);
        logLayout->addWidget(m_logText);
        bottomLayout->addWidget(logGroup);
        splitter->addWidget(bottomWidget);

        splitter->setSizes({550, 120});
        mainLayout->addWidget(splitter);

        // 状态栏
        m_statsLabel = new QLabel("TX: 0 KB/s | RX: 0 KB/s");
        statusBar()->addPermanentWidget(m_statsLabel);
        statusBar()->showMessage("就绪 - 请连接雷达设备");
    }

    void setupConnections()
    {
        connect(m_connectBtn, &QPushButton::clicked, this, &RadarMainWindow::onConnectClicked);
        connect(m_disconnectBtn, &QPushButton::clicked, this, &RadarMainWindow::onDisconnectClicked);
        connect(m_startBtn, &QPushButton::clicked, this, &RadarMainWindow::onStartMeasurement);
        connect(m_stopBtn, &QPushButton::clicked, this, &RadarMainWindow::onStopMeasurement);
        connect(m_beamCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &RadarMainWindow::onSwitchBeam);
        connect(m_statusBtn, &QPushButton::clicked, this, &RadarMainWindow::onStatusQuery);
        connect(m_versionBtn, &QPushButton::clicked, this, &RadarMainWindow::onVersionQuery);

        connect(m_network, &NetworkManager::connected, this, &RadarMainWindow::onConnected);
        connect(m_network, &NetworkManager::disconnected, this, &RadarMainWindow::onDisconnected);
        connect(m_network, &NetworkManager::dataReceived, this, &RadarMainWindow::onDataReceived);
        connect(m_network, &NetworkManager::frameReceived, this, &RadarMainWindow::onFrameReceived);
        connect(m_network, &NetworkManager::connectionError, this, &RadarMainWindow::onConnectionError);
        connect(m_network, &NetworkManager::connectionStateChanged, this, &RadarMainWindow::onConnectionStateChanged);
        connect(m_network, &NetworkManager::statisticsUpdated, this, &RadarMainWindow::onStatisticsUpdated);
    }

    NetworkManager *m_network;
    bool m_connected;
    bool m_measuring;
    int m_dataCount = 0;
    uint64_t m_rxBytes = 0;

    QLabel *m_connIndicator;
    QLabel *m_connStatus;
    QLineEdit *m_ipEdit;
    QSpinBox *m_portSpin;
    QPushButton *m_connectBtn;
    QPushButton *m_disconnectBtn;
    QPushButton *m_startBtn;
    QPushButton *m_stopBtn;
    QComboBox *m_beamCombo;
    QPushButton *m_statusBtn;
    QPushButton *m_versionBtn;
    QLabel *m_connStatusLabel;
    QLabel *m_beamValue;
    QLabel *m_stateValue;
    QLabel *m_tempValue;
    QLabel *m_voltValue;
    QLabel *m_versionValue;
    QLabel *m_dataCountValue;
    QLabel *m_avgSpeedValue;
    QLabel *m_statsLabel;
    QTableWidget *m_windTable;
    QTextEdit *m_logText;
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("雷达测风系统上位机");
    RadarMainWindow window;
    window.show();
    return app.exec();
}

#include "main.moc"
