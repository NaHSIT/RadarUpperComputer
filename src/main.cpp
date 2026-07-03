#include <QApplication>
#include <QLabel>
#include <QMainWindow>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>
#include <QLineEdit>
#include <QSpinBox>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QStatusBar>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // 设置应用程序信息
    app.setApplicationName("雷达测风系统上位机");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("嵌入式开发团队");

    // 创建主窗口
    QMainWindow mainWindow;
    mainWindow.setWindowTitle("雷达测风系统上位机 V1.0");
    mainWindow.resize(1200, 800);

    // 创建中心部件
    QWidget *centralWidget = new QWidget;
    mainWindow.setCentralWidget(centralWidget);

    // 创建主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);

    // ==================== 标题区域 ====================
    QLabel *titleLabel = new QLabel("雷达测风系统上位机");
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet(
        "font-size: 24px; "
        "font-weight: bold; "
        "color: #2c3e50; "
        "padding: 10px; "
        "background-color: #ecf0f1; "
        "border-radius: 5px;"
    );
    mainLayout->addWidget(titleLabel);

    // ==================== 连接配置区域 ====================
    QGroupBox *connectionGroup = new QGroupBox("连接配置");
    connectionGroup->setStyleSheet(
        "QGroupBox { "
        "font-weight: bold; "
        "border: 2px solid #bdc3c7; "
        "border-radius: 5px; "
        "margin-top: 10px; "
        "padding-top: 10px; "
        "} "
        "QGroupBox::title { "
        "subcontrol-origin: margin; "
        "left: 10px; "
        "padding: 0 5px; "
        "}"
    );

    QHBoxLayout *connLayout = new QHBoxLayout(connectionGroup);

    // IP地址
    QLabel *ipLabel = new QLabel("IP地址:");
    QLineEdit *ipEdit = new QLineEdit("192.168.1.100");
    ipEdit->setMinimumWidth(150);

    // 端口号
    QLabel *portLabel = new QLabel("端口:");
    QSpinBox *portSpin = new QSpinBox;
    portSpin->setRange(1, 65535);
    portSpin->setValue(5000);
    portSpin->setMinimumWidth(80);

    // 连接按钮
    QPushButton *connectBtn = new QPushButton("连接");
    connectBtn->setStyleSheet(
        "QPushButton { "
        "background-color: #27ae60; "
        "color: white; "
        "font-weight: bold; "
        "padding: 8px 16px; "
        "border-radius: 4px; "
        "} "
        "QPushButton:hover { "
        "background-color: #2ecc71; "
        "} "
        "QPushButton:pressed { "
        "background-color: #229954; "
        "}"
    );

    // 断开按钮
    QPushButton *disconnectBtn = new QPushButton("断开");
    disconnectBtn->setStyleSheet(
        "QPushButton { "
        "background-color: #e74c3c; "
        "color: white; "
        "font-weight: bold; "
        "padding: 8px 16px; "
        "border-radius: 4px; "
        "} "
        "QPushButton:hover { "
        "background-color: #ec7063; "
        "} "
        "QPushButton:pressed { "
        "background-color: #cb4335; "
        "}"
    );

    connLayout->addWidget(ipLabel);
    connLayout->addWidget(ipEdit);
    connLayout->addWidget(portLabel);
    connLayout->addWidget(portSpin);
    connLayout->addSpacing(20);
    connLayout->addWidget(connectBtn);
    connLayout->addWidget(disconnectBtn);

    mainLayout->addWidget(connectionGroup);

    // ==================== 控制按钮区域 ====================
    QGroupBox *controlGroup = new QGroupBox("雷达控制");
    controlGroup->setStyleSheet(connectionGroup->styleSheet());

    QHBoxLayout *ctrlLayout = new QHBoxLayout(controlGroup);

    // 启动测量
    QPushButton *startBtn = new QPushButton("启动测量");
    startBtn->setStyleSheet(
        "QPushButton { "
        "background-color: #3498db; "
        "color: white; "
        "font-weight: bold; "
        "padding: 10px 20px; "
        "border-radius: 4px; "
        "} "
        "QPushButton:hover { "
        "background-color: #5dade2; "
        "}"
    );

    // 停止测量
    QPushButton *stopBtn = new QPushButton("停止测量");
    stopBtn->setStyleSheet(
        "QPushButton { "
        "background-color: #95a5a6; "
        "color: white; "
        "font-weight: bold; "
        "padding: 10px 20px; "
        "border-radius: 4px; "
        "} "
        "QPushButton:hover { "
        "background-color: #b2bec3; "
        "}"
    );

    // 波束切换标签
    QLabel *beamLabel = new QLabel("当前波束:");
    QLabel *beamValue = new QLabel("波束0 (0°)");
    beamValue->setStyleSheet("font-weight: bold; color: #2980b9;");

    // 刷新率
    QLabel *refreshLabel = new QLabel("刷新率:");
    QLabel *refreshValue = new QLabel("10 Hz");
    refreshValue->setStyleSheet("font-weight: bold;");

    ctrlLayout->addWidget(startBtn);
    ctrlLayout->addWidget(stopBtn);
    ctrlLayout->addSpacing(30);
    ctrlLayout->addWidget(beamLabel);
    ctrlLayout->addWidget(beamValue);
    ctrlLayout->addSpacing(20);
    ctrlLayout->addWidget(refreshLabel);
    ctrlLayout->addWidget(refreshValue);

    mainLayout->addWidget(controlGroup);

    // ==================== 显示区域 ====================
    QGroupBox *displayGroup = new QGroupBox("数据显示");
    displayGroup->setStyleSheet(connectionGroup->styleSheet());

    QVBoxLayout *dispLayout = new QVBoxLayout(displayGroup);

    // 风场显示标签（占位）
    QLabel *windDisplayLabel = new QLabel("风场显示区域\n\n[极坐标风场图将在此处显示]");
    windDisplayLabel->setAlignment(Qt::AlignCenter);
    windDisplayLabel->setMinimumHeight(400);
    windDisplayLabel->setStyleSheet(
        "background-color: #f8f9fa; "
        "border: 2px dashed #bdc3c7; "
        "border-radius: 10px; "
        "font-size: 18px; "
        "color: #7f8c8d;"
    );
    dispLayout->addWidget(windDisplayLabel);

    mainLayout->addWidget(displayGroup);

    // ==================== 状态栏 ====================
    mainWindow.statusBar()->showMessage("系统状态: 就绪 | 版本: 1.0.0");

    // ==================== 显示窗口 ====================
    mainWindow.show();

    return app.exec();
}
