#include "qrcoderesultshower.h"
#include "JQQRCodeReader.h"
#include "utils/imageutils.h"
#include "dtitlebar.h"
#include <QDesktopServices>
#include <QClipboard>
#include <QApplication>

QRCodeResultShower::QRCodeResultShower(QWidget *parent, QString imagePath)
    : DMainWindow(parent)
{
    this->titlebar()->setTitle(tr("Decode result"));
    this->setCentralWidget(&m_centralWidget);
    m_centralWidget.setLayout(&m_layout);

    m_layout.addWidget(&m_tipsLabel, 0, 0);
    m_layout.addWidget(&m_textShower, 1, 0, 1, 3);
    m_layout.addWidget(&m_copyButton, 2, 1);
    m_layout.addWidget(&m_openBrowserButton, 2, 2);

    m_textShower.setText(qrCodeDecode(imagePath));
    m_textShower.setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
    m_textShower.setWordWrap(true);

    m_tipsLabel.setText(tr("Result:"));
    m_copyButton.setText(tr("Copy text"));
    m_openBrowserButton.setText(tr("Open Browser"));

    // connect
    connect(&m_copyButton, &QPushButton::clicked, this, &QRCodeResultShower::copyText);
    connect(&m_openBrowserButton, &QPushButton::clicked, this, &QRCodeResultShower::openBrowser);

    // 增加窗口大小
    setMinimumWidth(width() * 5);
}

void QRCodeResultShower::Show(QWidget *parent, QString imagePath)
{
    QRCodeResultShower *shower = new QRCodeResultShower(parent, imagePath);
    shower->show();
    // 释放 QRCodeResultShower 对象
    connect(shower, &QRCodeResultShower::close, [shower](){
        delete shower;
    });
}

QString QRCodeResultShower::qrCodeDecode(QImage image)
{
    JQQRCodeReader qrCodeReader;
    return qrCodeReader.decodeImage(image);
}

QString QRCodeResultShower::qrCodeDecode(QString path)
{
    JQQRCodeReader qrCodeReader;
    return qrCodeReader.decodeImage(utils::image::getRotatedImage(path));
}

void QRCodeResultShower::copyText()
{
    QApplication::clipboard()->setText(m_textShower.text());
}

void QRCodeResultShower::openBrowser()
{
    QDesktopServices::openUrl(m_textShower.text());
}
