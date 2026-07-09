#include "SpectrumPage.h"
#include <QLabel>
#include <QVBoxLayout>

SpectrumPage::SpectrumPage(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
}

SpectrumPage::~SpectrumPage()
{
}

void SpectrumPage::setupUI()
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(16, 16, 16, 16);

    QLabel *titleLabel = new QLabel("频谱", this);
    titleLabel->setStyleSheet("color: #333; font-size: 18px; font-weight: bold;");
    layout->addWidget(titleLabel);

    QLabel *contentLabel = new QLabel("频谱页面开发中...", this);
    contentLabel->setStyleSheet("color: #666; font-size: 14px;");
    contentLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(contentLabel);

    layout->addStretch();
}
