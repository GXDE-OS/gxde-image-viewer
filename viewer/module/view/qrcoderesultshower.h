#ifndef QRCODERESULTSHOWER_H
#define QRCODERESULTSHOWER_H

#include <QMainWindow>
#include <QObject>
#include <QWidget>
#include <QGridLayout>
#include <QTextBrowser>
#include <QPushButton>
#include <QLabel>
#include "dmainwindow.h"

DWIDGET_USE_NAMESPACE

class QRCodeResultShower : public DMainWindow
{
    //Q_OBJECT
public:
    explicit QRCodeResultShower(QWidget *parent, QString imagePath);
    static QString qrCodeDecode(QImage image);
    static QString qrCodeDecode(QString path);
    static void Show(QWidget *parent, QString imagePath);

private:
    void copyText();
    void openBrowser();

    QGridLayout m_layout;
    //QTextBrowser m_textShower;
    QLabel m_textShower;
    QPushButton m_copyButton;
    QPushButton m_openBrowserButton;
    QWidget m_centralWidget;
    QLabel m_tipsLabel;
    QString m_imagePath;
};

#endif // QRCODERESULTSHOWER_H
