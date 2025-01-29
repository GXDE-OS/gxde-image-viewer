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
#include "timelinedelegate.h"
#include "utils/imageutils.h"
#include "utils/baseutils.h"
#include "application.h"

#include <QDateTime>
#include <QDebug>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPainter>
#include <QPixmapCache>
#include <QStandardItemModel>
#include <QThread>

class TLThumbnailThread : public QThread {
    Q_OBJECT
public:
    explicit TLThumbnailThread(const TimelineItem::ItemData &data);
    const QString path() const;

signals:
    void reGenerated(const QString &path);
    void cached(QString path, QPixmap pixmap);

protected:
    void run() Q_DECL_OVERRIDE;

private:
    QString m_path;
    TimelineItem::ItemData m_data;
};

class RemoveCacheThread : public QThread {
    Q_OBJECT
public:
    explicit RemoveCacheThread(QMap<QString, QPixmap>* map,
                               QQueue<QString> *queue,
                               bool isHead);


protected:
    void run() Q_DECL_OVERRIDE;

private:
    bool m_isHead;
    QQueue<QString> *m_queue;
    QMap<QString, QPixmap>* m_map;
};

class LoadImageThread : public QThread {
    Q_OBJECT
public:
    explicit LoadImageThread(QMap<QString, QPixmap>* map,
                             QQueue<QString> *queue,
                             QString path,
                             const QSize &size);
    const QString path() const;

protected:
    void run() Q_DECL_OVERRIDE;

private:
    QString m_path;
    QSize m_size;
    QQueue<QString> *m_queue;
    QMap<QString, QPixmap>* m_map;
};


TimelineDelegate::TimelineDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
    // Avoid thumbnail-thread being called too ofen
    /*QTimer *t = new QTimer(this);
    connect(t, &QTimer::timeout, this, [=] {
        m_threads.clear();
    });
    t->start(1000);*/

    onThemeChanged(dApp->viewerTheme->getCurrentTheme());
    connect(dApp->viewerTheme, &ViewerThemeManager::viewerThemeChanged, this,
            &TimelineDelegate::onThemeChanged);
}

void TimelineDelegate::onThemeChanged(ViewerThemeManager::AppTheme theme) {
    if (theme == ViewerThemeManager::Dark) {
        m_borderColor = utils::common::DARK_BORDER_COLOR;
        m_dateColor = utils::common::DARK_TITLE_COLOR;
        m_seperatorColor = utils::timeline::DARK_SEPERATOR_COLOR;
        m_defaultThumbnail = utils::common::DARK_DEFAULT_THUMBNAIL;
    } else {
        m_borderColor = utils::common::LIGHT_BORDER_COLOR;
        m_dateColor = utils::common::LIGHT_TITLE_COLOR;
        m_seperatorColor = utils::timeline::LIGHT_SEPERATOR_COLOR;
        m_defaultThumbnail = utils::common::LIGHT_DEFAULT_THUMBNAIL;
    }
}

void TimelineDelegate::removeLock(QString path)
{
    //LoadImageThread *thread = NULL;
    if (m_loadImagethreads.contains(path)) {
        //thread = m_loadImagethreads[path];
        m_loadImagethreads.remove(path);
    }
    auto list = m_loadImagethreads.keys();
    for (int i = 0; i <= (list.count() > 4 ? 4 : list.count() - 1); ++i) {
        if (m_loadImagethreads.contains(list[i])) {
            auto thread = m_loadImagethreads[list[i]];
            /*if (thread) {
                thread->start();
            }*/
            if (thread && !thread->isRunning()) {
                thread->start();
            }
        }
    }

}

QPixmap TimelineDelegate::getThumbnailWithCache(const QString path, const QSize &size)
{
    // 如果已缓存
    if (m_timelineImageCache.contains(path)) {
        return m_timelineImageCache[path];
    }
    if (m_loadImagethreads.keys().contains(path)) {
        return QPixmap();
    }
    // 如果超出缓存数量，则移除最早加入的
    /*if (m_timelineImageCache.count() >= 1000) {
        RemoveCacheThread *remove = new RemoveCacheThread(&m_timelineImageCache,
                                                          &m_addQueue,
                                                          1);
        remove->start();
    }*/
    LoadImageThread *load = new LoadImageThread(&m_timelineImageCache,
                                                &m_addQueue,
                                                path,
                                                size);
    m_loadImagethreads.insert(path, load);
    m_timelineImageCache.insert(path, QPixmap());
    connect(load, &LoadImageThread::finished, this, [=] {
        load->deleteLater();
    });
    load->start();
    return QPixmap();
}



void TimelineDelegate::paint(QPainter *painter,
                          const QStyleOptionViewItem &option,
                          const QModelIndex &index) const
{
    auto data = itemData(index);

    painter->setRenderHints(QPainter::HighQualityAntialiasing |
                            QPainter::SmoothPixmapTransform |
                            QPainter::Antialiasing);

    // Draw Timeline title
    if (data.isTitle) {
        // Draw text

        QPen p(m_dateColor);
        QFont f;f.setPixelSize(12);
        f.setWeight(25);
        painter->setPen(p);
        painter->setFont(f);
        QRect r = option.rect;
        QRect texRect = QRect(r.x(), r.y() + 16, r.width(), r.height());
        painter->drawText(texRect, data.timeline);

        // Draw separator

        QRect sepRect = QRect(r.x(), r.y() + r.height() - 8, r.width(), 1);
        painter->fillRect(sepRect, m_seperatorColor);
    }
    // Draw thumbnail
    else {
        if (data.path.isNull()) return;
        bool selected = false;
        if (/*(option.state & QStyle::State_MouseOver) &&*/
                (option.state & QStyle::State_Selected) != 0) {
            selected = true;
        }

        QRect rect = option.rect;
        QPainterPath bp;
        bp.addRoundedRect(rect, utils::common::BORDER_RADIUS, utils::common::BORDER_RADIUS);
        painter->setClipPath(bp);
        //**draw transparent background
//        QPixmap pm(12, 12);
//        QPainter pmp(&pm);
//        //TODO: the transparent box
//        //should not be scaled with the image
//        pmp.fillRect(0, 0, 6, 6, LIGHT_CHECKER_COLOR);
//        pmp.fillRect(6, 6, 6, 6, LIGHT_CHECKER_COLOR);
//        pmp.fillRect(0, 6, 6, 6, DARK_CHECKER_COLOR);
//        pmp.fillRect(6, 0, 6, 6, DARK_CHECKER_COLOR);
//        pmp.end();
        painter->fillRect(rect, QBrush(utils::common::LIGHT_CHECKER_COLOR));
        using namespace utils::image;

        const auto ratio = qApp->devicePixelRatio();
        QPixmap pix = const_cast<TimelineDelegate*>(this)->getThumbnailWithCache(data.path,
                                            rect.size() * ratio);// = cutSquareImage(getThumbnail(data.path), rect.size() * ratio);
        pix.setDevicePixelRatio(ratio);
        painter->drawPixmap(rect, pix);

        // Draw inside border
        QPen p(selected ? utils::common::BORDER_COLOR_SELECTED : m_borderColor,
               selected ? utils::common::BORDER_WIDTH_SELECTED : utils::common::BORDER_WIDTH);
        painter->setPen(p);
        painter->drawPath(bp);

        QPainterPathStroker stroker;
        stroker.setWidth(selected ? utils::common::BORDER_WIDTH_SELECTED : utils::common::BORDER_WIDTH);
        stroker.setJoinStyle(Qt::RoundJoin);
        QPainterPath borderPath = stroker.createStroke(bp);
        painter->drawPath(borderPath);
    }
}

QSize TimelineDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option)
    if (itemData(index).isTitle) {
        return QSize(10, 30);
    }
    else {
        return QSize(30, 50);//index.model()->data(index, Qt::SizeHintRole).toSize();
    }
}

TimelineItem::ItemData TimelineDelegate::itemData(const QModelIndex &index) const
{
    QVariantList datas = index.model()->data(index, Qt::DisplayRole).toList();
    TimelineItem::ItemData data;
    if (datas.count() == 4) { // There is 4 field data inside TimelineData
        data.isTitle = datas[0].toBool();
        data.path = datas[1].toString();
        data.timeline = datas[2].toString();
        data.thumbArray = datas[3].toByteArray();
    }

    return data;
}

QPixmap TimelineDelegate::thumbnail(const TimelineItem::ItemData &data) const
{
    QPixmap thumb;
    thumb.loadFromData(data.thumbArray);
    if (thumb.isNull()) {
        if (! QPixmapCache::find("NO_IMAGE_TMP_KEY", &thumb)) {
            const QSize ms(utils::common::THUMBNAIL_MAX_SCALE_SIZE,
                           utils::common::THUMBNAIL_MAX_SCALE_SIZE);
            thumb = utils::image::cutSquareImage(QPixmap(m_defaultThumbnail), ms);
            QPixmapCache::insert("NO_IMAGE_TMP_KEY", thumb);
        }
    }
    startThumbnailThread(data);

    return thumb;
}

void TimelineDelegate::startThumbnailThread(const TimelineItem::ItemData &data) const
{
    if (m_threads.keys().contains(data.path)) {
        return;
    }

    TLThumbnailThread *t = new TLThumbnailThread(data);
    connect(t, &TLThumbnailThread::finished, this, [=] {
        t->deleteLater();
        m_threads.remove(t->path());
    });
    connect(t, &TLThumbnailThread::reGenerated,
            this, &TimelineDelegate::thumbnailGenerated);
    m_threads.insert(data.path, t);
    t->start();
}

#include "timelinedelegate.moc"
LoadImageThread::LoadImageThread(QMap<QString, QPixmap>* map,
                                 QQueue<QString> *queue,
                                 QString path,
                                 const QSize &size)
{
    m_map = map;
    m_path = path;
    m_queue = queue;
    m_size = size;
}

void LoadImageThread::run()
{
    if (m_map->contains(m_path)) {
        if (!m_map->value(m_path).isNull()) {
            return;
        }
    }
    /*if (m_map->count() >= 300) {
        auto index = m_map->keys().indexOf(m_path);
        auto all = m_map->count();
        m_map->insert(m_map->keys().at(all - index - 1), QPixmap());
    }*/
    using namespace utils::image;
    m_map->insert(m_path, cutSquareImage(getThumbnail(m_path), m_size));
    m_queue->append(m_path);
}

const QString LoadImageThread::path() const
{
    return m_path;
}


RemoveCacheThread::RemoveCacheThread(QMap<QString, QPixmap>* map,
                                     QQueue<QString> *queue,
                                     bool isHead)
{
    m_map = map;
    m_isHead = isHead;
    m_queue = queue;
}

void RemoveCacheThread::run()
{
    if (m_isHead) {
        m_map->remove(m_queue->head());
        m_queue->removeFirst();
        return;
    }
    m_map->remove(m_queue->last());
    m_queue->removeLast();
}

TLThumbnailThread::TLThumbnailThread(const TimelineItem::ItemData &data)
    : QThread()
    , m_data(data)
{

}

const QString TLThumbnailThread::path() const
{
    return m_data.path;
}

void TLThumbnailThread::run()
{
    using namespace utils::base;
    using namespace utils::image;

    // Do not check the thumbnail for unplug devices' image
    if (onMountDevice(m_data.path) && ! mountDeviceExist(m_data.path)) {
        return;
    }

    QString thumbMTime = getThumbnail(m_data.path, true).toImage().text("Thumb::MTime");
    QString sourceMTime = QString::number(QFileInfo(m_data.path).lastModified().toTime_t());
    // If the file has been changed and thumbnail didn't
    if (thumbMTime != sourceMTime) {
        generateThumbnail(m_data.path);
        // Emit this signal to make model update
        emit reGenerated(m_data.path);
    }
    else {
        QPixmap thumb;
        thumb.loadFromData(m_data.thumbArray);
        // The thumbnail may regenerate by other model
        if (thumb.toImage().text("Thumb::MTime") != sourceMTime) {
            generateThumbnail(m_data.path);
            emit reGenerated(m_data.path);
        }
    }
}
