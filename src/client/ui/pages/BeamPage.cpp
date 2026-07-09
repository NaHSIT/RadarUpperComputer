#include "BeamPage.h"
#include <QLabel>
#include <QVBoxLayout>

BeamPage::BeamPage(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
}

BeamPage::~BeamPage()
{
}

void BeamPage::setupUI()
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(16, 16, 16, 16);

    QLabel *titleLabel = new QLabel("波束", this);
    titleLabel->setStyleSheet("color: #333; font-size: 18px; font-weight: bold;");
    layout->addWidget(titleLabel);

    QLabel *contentLabel = new QLabel("波束页面开发中...", this);
    contentLabel->setStyleSheet("color: #666; font-size: 14px;");
    contentLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(contentLabel);

    layout->addStretch();
}
