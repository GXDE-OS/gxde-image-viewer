/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "settingswindow.h"
#include "titleframe.h"
#include "contentsframe.h"
#include "utils/baseutils.h"
#include <DTitlebar>
#include <QApplication>
#include <QDebug>
#include <QFrame>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QMouseEvent>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWindow>

SettingsWindow::SettingsWindow(QWidget *parent)
    :DMainWindow(parent)
{
    setWindowModality(Qt::ApplicationModal);
    setWindowFlags(Qt::WindowStaysOnTopHint|
                   Qt::FramelessWindowHint);
    setStyleSheet(utils::base::getFileContent(
                      ":/settings/qss/resources/qss/settings.qss"));

    if (titlebar()) titlebar()->setFixedHeight(0);
    setFixedSize(680, 560);

    // Main
    QFrame *f = new QFrame;
    setCentralWidget(f);
    QVBoxLayout *mainLayout = new QVBoxLayout(f);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // Contents
    QHBoxLayout *cbl = new QHBoxLayout;
    cbl->setContentsMargins(0, 0, 0, 0);
    cbl->setSpacing(0);

    // TitleFrame
    TitleFrame *tf = new TitleFrame;
    cbl->addWidget(tf);
    ContentsFrame *cf = new ContentsFrame(this);
    cbl->addWidget(cf);

    connect(tf, &TitleFrame::clicked,
            cf, &ContentsFrame::setCurrentID);
    connect(cf, &ContentsFrame::currentFieldChanged,
            tf, &TitleFrame::setCurrentID);

    mainLayout->addLayout(cbl);
}

void SettingsWindow::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Escape) {
        this->close();
    }

    DMainWindow::keyPressEvent(e);
}

// Patched by CharOfString: 修补程序使得在Wayland下可以拖拽标题栏
void SettingsWindow::mousePressEvent(QMouseEvent* e) {
    // 如果是Wayland，且鼠标在标题栏上被按下，则记录拖拽开始的坐标
    // 不急着调startSystemMove，为了保证双击最大化没毛病等到mouseMoveEvent再调用
    if (e->button() == Qt::LeftButton
            && QGuiApplication::platformName() == "wayland") {
        mIsMousePressed = true;
        mDragPosStart = e->globalPos();
        return;
    }
    DMainWindow::mousePressEvent(e);
}

void SettingsWindow::mouseMoveEvent(QMouseEvent* e) {
    // 如果上述mousePressEvent中的条件满足，并且鼠标移动的距离超过Qt定义的拖动距离...
    // 那么就可以使用startSystemMove开始拖动了（喜
    // 这里曼哈顿距离够用了
    if (mIsMousePressed
            && (e->globalPos() - mDragPosStart).manhattanLength()
               > QApplication::startDragDistance()) {
        mIsMousePressed = false;
        if (QWindow *handle = windowHandle()) {
            handle->startSystemMove();
        } else {
            qCritical() << "Failed to get window handle during window move"
                << "action halted!!";
        }  // 还是那句话：获取不到handle的话那很有生活了
        return;
    }
    DMainWindow::mouseMoveEvent(e);
}

void SettingsWindow::mouseReleaseEvent(QMouseEvent* e) {
    // 释放鼠标时，需要重置状态
    mIsMousePressed = false;
    DMainWindow::mouseReleaseEvent(e);
}
// Patch结束
