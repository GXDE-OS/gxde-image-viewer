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
#include "imagestrip.h"
#include "application.h"
#include "utils/imageutils.h"
#include "utils/baseutils.h"

#include <QPainter>
#include <QScrollBar>
#include <QHBoxLayout>
#include <QDebug>
#include <QWheelEvent>
#include <QEnterEvent>
#include <QtConcurrent>

namespace {

const int THUMB_SIZE = 48;
const int ITEM_MARGIN = 4;
const int ITEM_SPACING = 4;
const int STRIP_HEIGHT = THUMB_SIZE + ITEM_MARGIN * 2;
const int ARROW_BUTTON_SIZE = 24;
const int BOTTOM_MARGIN = 4;
const int SLIDE_HINT = 6;

}

ImageStripList::ImageStripList(QWidget *parent)
    : QListView(parent)
{
}

void ImageStripList::wheelEvent(QWheelEvent *e)
{
    if (e->orientation() == Qt::Vertical) {
        QWheelEvent hEvent(e->pos(), e->globalPos(), e->delta() * 2, e->buttons(), e->modifiers(), Qt::Horizontal);
        QCoreApplication::sendEvent(horizontalScrollBar(), &hEvent);
    } else {
        QListView::wheelEvent(e);
    }
    e->accept();
}

ImageStripDelegate::ImageStripDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
    m_borderColor = QColor(0, 0, 0, 40);
    m_selectedBorderColor = QColor(0, 129, 255);
}

void ImageStripDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                               const QModelIndex &index) const
{
    painter->save();
    painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    const bool isSelected = option.state & QStyle::State_Selected;
    const QRect rect = option.rect;

    if (isSelected) {
        QRect bgRect = rect.adjusted(2, 2, -2, -2);
        QPainterPath bgPath;
        bgPath.addRoundedRect(bgRect, 6, 6);
        painter->fillPath(bgPath, QColor(0, 129, 255, 60));
    }

    QPixmap pixmap = index.data(Qt::DecorationRole).value<QPixmap>();

    QSize thumbSize(THUMB_SIZE, THUMB_SIZE);
    QRect clipRect;
    clipRect.setSize(thumbSize);
    clipRect.moveCenter(rect.center());

    QPainterPath thumbPath;
    thumbPath.addRoundedRect(clipRect, 4, 4);
    painter->setClipPath(thumbPath);

    if (!pixmap.isNull()) {
        QPixmap scaled = pixmap.scaled(thumbSize, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        QRect thumbRect;
        thumbRect.setSize(scaled.size().boundedTo(thumbSize));
        thumbRect.moveCenter(rect.center());
        painter->drawPixmap(thumbRect, scaled);
    } else {
        painter->fillRect(clipRect, QColor(200, 200, 200, 80));
    }

    painter->setClipping(false);

    QPen pen;
    if (isSelected) {
        pen.setColor(m_selectedBorderColor);
        pen.setWidth(2);
    } else {
        pen.setColor(m_borderColor);
        pen.setWidth(1);
    }
    painter->setPen(pen);
    painter->drawRoundedRect(clipRect.adjusted(1, 1, -1, -1), 4, 4);

    painter->restore();
}

QSize ImageStripDelegate::sizeHint(const QStyleOptionViewItem &option,
                                   const QModelIndex &index) const
{
    Q_UNUSED(option)
    Q_UNUSED(index)
    return QSize(THUMB_SIZE + ITEM_MARGIN * 2, THUMB_SIZE + ITEM_MARGIN * 2);
}

ImageStrip::ImageStrip(QWidget *parent)
    : BlurFrame(parent)
    , m_listView(new ImageStripList(this))
    , m_model(new QStandardItemModel(this))
    , m_delegate(new ImageStripDelegate(this))
    , m_prevBtn(new DImageButton(this))
    , m_nextBtn(new DImageButton(this))
    , m_hovered(false)
    , m_visibleY(0)
    , m_hiddenY(0)
{
    initUI();
    connect(dApp->viewerTheme, &ViewerThemeManager::viewerThemeChanged,
            this, &ImageStrip::onThemeChanged);
}

void ImageStrip::initUI()
{
    setFixedHeight(STRIP_HEIGHT);
    setBorderRadius(8);
    setBorderWidth(0);
    setCoverBrush(QBrush(QColor(255, 255, 255, 200)));

    m_prevBtn->setObjectName("ImageStripPrevButton");
    m_prevBtn->setFixedSize(ARROW_BUTTON_SIZE, ARROW_BUTTON_SIZE);
    m_nextBtn->setObjectName("ImageStripNextButton");
    m_nextBtn->setFixedSize(ARROW_BUTTON_SIZE, ARROW_BUTTON_SIZE);

    QString prevNormal, prevHover, prevPress, nextNormal, nextHover, nextPress;
    if (dApp->viewerTheme->getCurrentTheme() == ViewerThemeManager::Dark) {
        prevNormal = ":/resources/dark/images/previous_normal.svg";
        prevHover  = ":/resources/dark/images/previous_hover.svg";
        prevPress  = ":/resources/dark/images/previous_press.svg";
        nextNormal = ":/resources/dark/images/next_normal.svg";
        nextHover  = ":/resources/dark/images/next_hover.svg";
        nextPress  = ":/resources/dark/images/next_press.svg";
    } else {
        prevNormal = ":/resources/light/images/previous_normal.svg";
        prevHover  = ":/resources/light/images/previous_hover.svg";
        prevPress  = ":/resources/light/images/previous_press.svg";
        nextNormal = ":/resources/light/images/next_normal.svg";
        nextHover  = ":/resources/light/images/next_hover.svg";
        nextPress  = ":/resources/light/images/next_press.svg";
    }
    m_prevBtn->setNormalPic(prevNormal);
    m_prevBtn->setHoverPic(prevHover);
    m_prevBtn->setPressPic(prevPress);
    m_nextBtn->setNormalPic(nextNormal);
    m_nextBtn->setHoverPic(nextHover);
    m_nextBtn->setPressPic(nextPress);

    m_listView->setModel(m_model);
    m_listView->setItemDelegate(m_delegate);
    m_listView->setFlow(QListView::LeftToRight);
    m_listView->setViewMode(QListView::IconMode);
    m_listView->setWrapping(false);
    m_listView->setSpacing(ITEM_SPACING);
    m_listView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_listView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_listView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_listView->setFocusPolicy(Qt::NoFocus);
    m_listView->setFrameShape(QFrame::NoFrame);
    m_listView->setAttribute(Qt::WA_TranslucentBackground);
    m_listView->viewport()->setAttribute(Qt::WA_TranslucentBackground);

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(ITEM_MARGIN, 0, ITEM_MARGIN, 0);
    layout->setSpacing(4);
    layout->addWidget(m_prevBtn, 0, Qt::AlignVCenter);
    layout->addWidget(m_listView, 1);
    layout->addWidget(m_nextBtn, 0, Qt::AlignVCenter);

    connect(m_listView, &QListView::clicked, this, [=](const QModelIndex &index) {
        if (index.isValid()) {
            QString path = index.data(Qt::UserRole + 1).toString();
            if (!path.isEmpty()) {
                emit imageClicked(path);
            }
        }
    });

    connect(m_prevBtn, &DImageButton::clicked, this, &ImageStrip::previousRequested);
    connect(m_nextBtn, &DImageButton::clicked, this, &ImageStrip::nextRequested);

    connect(m_listView->horizontalScrollBar(), &QScrollBar::valueChanged,
            this, &ImageStrip::loadVisibleThumbnails);
}

void ImageStrip::updateArrowButtonState()
{
    bool hasMultiple = m_model->rowCount() > 1;
    m_prevBtn->setVisible(hasMultiple);
    m_nextBtn->setVisible(hasMultiple);
}

void ImageStrip::updatePosition()
{
    QWidget *p = parentWidget();
    if (!p) return;

    m_visibleY = p->height() - STRIP_HEIGHT - BOTTOM_MARGIN;
    m_hiddenY = p->height() - SLIDE_HINT;

    if (m_hovered) {
        move(x(), m_visibleY);
    } else {
        move(x(), m_hiddenY);
    }

    loadVisibleThumbnails();
}

void ImageStrip::slideIn()
{
    emit requestStopAnimation();
    moveWithAnimation(x(), m_visibleY);
}

void ImageStrip::slideOut()
{
    emit requestStopAnimation();
    moveWithAnimation(x(), m_hiddenY);
}

void ImageStrip::enterEvent(QEvent *e)
{
    Q_UNUSED(e)
    if (!m_hovered) {
        m_hovered = true;
        slideIn();
    }
}

void ImageStrip::leaveEvent(QEvent *e)
{
    Q_UNUSED(e)
    if (m_hovered) {
        m_hovered = false;
        slideOut();
    }
}

void ImageStrip::setPaths(const QStringList &paths)
{
    m_model->clear();
    m_loadedRows.clear();
    for (const QString &path : paths) {
        QStandardItem *item = new QStandardItem();
        item->setData(QPixmap(), Qt::DecorationRole);
        item->setData(path, Qt::UserRole + 1);
        m_model->appendRow(item);
    }
    updateArrowButtonState();
    loadVisibleThumbnails();
}

void ImageStrip::requestThumbnail(int row)
{
    if (row < 0 || row >= m_model->rowCount())
        return;
    if (m_loadedRows.contains(row))
        return;

    m_loadedRows.insert(row);
    QString path = m_model->index(row, 0).data(Qt::UserRole + 1).toString();

    QtConcurrent::run([this, path, row]() {
        QPixmap thumb = utils::image::getThumbnail(path, true);
        if (thumb.isNull()) {
            thumb = utils::image::getThumbnail(path);
        }
        if (!thumb.isNull()) {
            QMetaObject::invokeMethod(this, [this, thumb, path, row]() {
                if (row >= m_model->rowCount())
                    return;
                QModelIndex idx = m_model->index(row, 0);
                if (idx.data(Qt::UserRole + 1).toString() == path) {
                    m_model->setData(idx, thumb, Qt::DecorationRole);
                }
            }, Qt::QueuedConnection);
        }
    });
}

void ImageStrip::loadVisibleThumbnails()
{
    QRect viewportRect = m_listView->viewport()->rect();
    for (int i = 0; i < m_model->rowCount(); ++i) {
        QModelIndex idx = m_model->index(i, 0);
        QRect itemRect = m_listView->visualRect(idx);
        if (itemRect.right() < 0)
            continue;
        if (itemRect.left() > viewportRect.width())
            break;
        requestThumbnail(i);
    }
}

void ImageStrip::setCurrentImage(const QString &path)
{
    m_currentPath = path;
    for (int i = 0; i < m_model->rowCount(); ++i) {
        QModelIndex index = m_model->index(i, 0);
        QString itemPath = index.data(Qt::UserRole + 1).toString();
        if (itemPath == path) {
            m_listView->selectionModel()->setCurrentIndex(index, QItemSelectionModel::ClearAndSelect);
            m_listView->scrollTo(index, QAbstractItemView::PositionAtCenter);
            requestThumbnail(i);
            return;
        }
    }
    m_listView->selectionModel()->clearSelection();
}

void ImageStrip::clear()
{
    m_model->clear();
    m_currentPath.clear();
    m_loadedRows.clear();
    updateArrowButtonState();
}

void ImageStrip::onThemeChanged(ViewerThemeManager::AppTheme theme)
{
    if (theme == ViewerThemeManager::Dark) {
        setCoverBrush(QBrush(QColor(40, 40, 40, 200)));
        m_prevBtn->setNormalPic(":/resources/dark/images/previous_normal.svg");
        m_prevBtn->setHoverPic(":/resources/dark/images/previous_hover.svg");
        m_prevBtn->setPressPic(":/resources/dark/images/previous_press.svg");
        m_nextBtn->setNormalPic(":/resources/dark/images/next_normal.svg");
        m_nextBtn->setHoverPic(":/resources/dark/images/next_hover.svg");
        m_nextBtn->setPressPic(":/resources/dark/images/next_press.svg");
    } else {
        setCoverBrush(QBrush(QColor(255, 255, 255, 200)));
        m_prevBtn->setNormalPic(":/resources/light/images/previous_normal.svg");
        m_prevBtn->setHoverPic(":/resources/light/images/previous_hover.svg");
        m_prevBtn->setPressPic(":/resources/light/images/previous_press.svg");
        m_nextBtn->setNormalPic(":/resources/light/images/next_normal.svg");
        m_nextBtn->setHoverPic(":/resources/light/images/next_hover.svg");
        m_nextBtn->setPressPic(":/resources/light/images/next_press.svg");
    }
}
