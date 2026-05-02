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
#ifndef IMAGESTRIP_H
#define IMAGESTRIP_H

#include "widgets/blureframe.h"
#include "controller/dbmanager.h"
#include "controller/viewerthememanager.h"

#include <QListView>
#include <QStyledItemDelegate>
#include <QStandardItemModel>
#include <dimagebutton.h>

DWIDGET_USE_NAMESPACE

class ImageStripDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit ImageStripDelegate(QObject *parent = nullptr);

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const override;

private:
    QColor m_borderColor;
    QColor m_selectedBorderColor;
};

class ImageStripList : public QListView
{
    Q_OBJECT
public:
    explicit ImageStripList(QWidget *parent = nullptr);

protected:
    void wheelEvent(QWheelEvent *e) override;
};

class ImageStrip : public BlurFrame
{
    Q_OBJECT
public:
    explicit ImageStrip(QWidget *parent = nullptr);

    void setPaths(const QStringList &paths);
    void setCurrentImage(const QString &path);
    void clear();
    void loadVisibleThumbnails();

signals:
    void imageClicked(const QString &path);
    void previousRequested();
    void nextRequested();

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    void initUI();
    void onThemeChanged(ViewerThemeManager::AppTheme theme);
    void updateArrowButtonState();
    void requestThumbnail(int row);

    ImageStripList *m_listView;
    QStandardItemModel *m_model;
    ImageStripDelegate *m_delegate;
    DImageButton *m_prevBtn;
    DImageButton *m_nextBtn;
    QString m_currentPath;
    QSet<int> m_loadedRows;
};

#endif // IMAGESTRIP_H
