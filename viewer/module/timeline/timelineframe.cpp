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
#include "timelineframe.h"
#include "application.h"
#include "controller/configsetter.h"
#include "controller/importer.h"
#include "controller/signalmanager.h"
#include "mvc/timelinemodel.h"
#include "mvc/timelinedelegate.h"
#include "mvc/timelineitem.h"
#include "mvc/timelineview.h"
#include "utils/baseutils.h"
#include "utils/imageutils.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QMutex>
#include <QScrollBar>
#include <QTimer>
#include <QtConcurrent>

namespace {

const int TOP_TOOLBAR_HEIGHT = 39;
const int BOTTOM_TOOLBAR_HEIGHT = 22;
const int MIN_MODEL_THUMBNAIL_SIZE = 192;
const int MODEL_THUMBNAIL_SCALE = 2;
const int INSERT_BATCH_SIZE = 32;
const QString SCANPATHS_GROUP = "SCANPATHSGROUP";
const QString SCANPATHS_KEY = "SCANPATHSKEY";

class LoadThread : public QThread
{
public:
    explicit LoadThread(const DBImgInfoList &infos, int thumbnailSize,
                        TimelineFrame *receiver);

protected:
    void run() Q_DECL_OVERRIDE;

private:
    const QStringList scanpathsHash();
    void enqueueBatch(const TimelineItemDataList &batch);

private:
    DBImgInfoList m_infos;
    int m_thumbnailSize;
    TimelineFrame *m_receiver;
};

}   // namespace

class TopTimelineTip : public QLabel
{
public:
    explicit TopTimelineTip(QWidget *parent) : QLabel(parent) {
        setObjectName("TopTimelineTip");
        setStyleSheet(parent->styleSheet());
        setFixedHeight(24);
        setAttribute(Qt::WA_TransparentForMouseEvents);
    }
    void setLeftMargin(int v) {setContentsMargins(v, 0, 0, 0);}
};

TimelineFrame::TimelineFrame(QWidget *parent)
    : QFrame(parent)
    , m_thumbnailSize(MIN_MODEL_THUMBNAIL_SIZE)
    , m_insertTimer(new QTimer(this))
    , m_pendingScrollRangeUpdate(false)
{
    qRegisterMetaType<TimelineItem::ItemData>("TimelineItem::ItemData");
    qRegisterMetaType<TimelineItemDataList>("TimelineItemDataList");

    m_insertTimer->setInterval(16);
    connect(m_insertTimer, &QTimer::timeout,
            this, &TimelineFrame::processPendingItems);

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    initView();
    layout->addWidget(m_view);

    // Top-Tip
    initTopTip();
    initItems();
    initConnection();
}

void TimelineFrame::clearSelection()
{
    m_view->clearSelection();
}

void TimelineFrame::selectAll()
{
    m_view->selectAll();
}

void TimelineFrame::setIconSize(int size)
{
    m_thumbnailSize = qMax(MIN_MODEL_THUMBNAIL_SIZE,
                           size * MODEL_THUMBNAIL_SCALE);
    m_view->setItemSize(size);
    updateVisibleThumbnails();
}

void TimelineFrame::updateThumbnail(const QString &path)
{
    using namespace utils::image;
    using namespace utils::base;
    auto info = DBManager::instance()->getInfoByPath(path);
    TimelineItem::ItemData data;
    data.isTitle = false;
    data.path = path;

    QBuffer inBuffer( &data.thumbArray );
    inBuffer.open( QIODevice::WriteOnly );
    // write inPixmap into inByteArray
    cutSquareImage(getThumbnail(data.path, true),
                   QSize(m_thumbnailSize, m_thumbnailSize)).save(&inBuffer, "JPG", 90);
    data.timeline = timeToString(info.time, true);

    m_model.updateData(data);
    m_view->update();
}

void TimelineFrame::updateScrollRange()
{
    m_view->updateScrollbarRange();
    // FIXME: the value of m_view's verticzalScrollBar won't change event if
    // it's range is changed, if the scrollbar's slider is at the top, the
    // painting_indexs won't be update, so need to scroll it here to force update
    const int vv = m_view->verticalScrollBar()->value();
    m_view->verticalScrollBar()->setValue(vv + 1);
}

bool TimelineFrame::isEmpty() const
{
    return false;
}

const QString TimelineFrame::currentMonth() const
{
    using namespace utils::base;
    if (m_view->paintingIndexs().length() > 0) {
        QPoint p(m_view->horizontalOffset() + m_view->itemSize() / 2, m_view->itemSize() / 2);
        int count = 0;
        while (count <= 3) {
            QModelIndex index = m_view->indexAt(p);
            QVariantList datas = m_model.data(index, Qt::DisplayRole).toList();
            if (datas.count() == 4) { // There is 4 field data inside TimelineData
                return datas[2].toString();
            }
            else {
                // The p may contain by title-index
                p.setY(p.y() + m_view->titleHeight());
            }
            count ++;
        };
    }

    return QString();
}

const QStringList TimelineFrame::selectedPaths() const
{
    QStringList sPaths;
    QModelIndexList il = m_view->selectionModel()->selectedIndexes();
    for (QModelIndex index : il) {
        QVariantList datas = index.model()->data(index, Qt::DisplayRole).toList();
        // There is 4 field data inside TimelineData
        if (datas.count() == 4 && !datas[0].toBool()) {
            sPaths << datas[1].toString();
        }
    }
    return sPaths;
}

void TimelineFrame::resizeEvent(QResizeEvent *e)
{
    QFrame::resizeEvent(e);
    m_tip->setFixedWidth(e->size().width());
}

void TimelineFrame::initConnection()
{
    // Item append and remove
    connect(Importer::instance(), &Importer::imported, this, [=] {
        updateScrollRange();
        // Call initItems to reinsert those datas which miss by user scroll view
        initItems();
        TIMER_SINGLESHOT(1000, {m_view->updateView();}, this)
    });
    connect(dApp->signalM, &SignalManager::imagesInserted,
            this, [=] (const DBImgInfoList infos){
        LoadThread *t = new LoadThread(infos, m_thumbnailSize, this);
        connect(t, &LoadThread::finished, this, [=] {
            t->deleteLater();
            m_infos << infos;
            m_pendingScrollRangeUpdate = true;
            if (m_pendingItems.isEmpty()) {
                updateScrollRange();
                m_pendingScrollRangeUpdate = false;
            }
        });
        t->start();
    });
    connect(dApp->signalM, &SignalManager::imagesRemoved,
            this, &TimelineFrame::removeItems);
}

void TimelineFrame::initView()
{
    m_view = new TimelineView(this);
    TimelineDelegate *delegate = new TimelineDelegate();
    connect(delegate, &TimelineDelegate::thumbnailGenerated,
            this, &TimelineFrame::updateThumbnail);
    m_view->setItemDelegate(delegate);
    m_view->setModel(&m_model);
    connect(m_view, &TimelineView::doubleClicked, this, [=] (const QModelIndex &index){
        QVariantList datas = index.model()->data(index, Qt::DisplayRole).toList();
        if (datas.count() == 4) { // There is 4 field data inside TimelineData
            const QString path = datas[1].toString();
            if (!path.isEmpty()) {
                const QStringList paths = m_loadedPaths.isEmpty()
                        ? QStringList(path) : m_loadedPaths;
                emit viewImage(path, paths);
            }
        }
    });
    connect(m_view, &TimelineView::showMenu, this, &TimelineFrame::showMenu);
    connect(m_view, &TimelineView::changeItemSize,
            this, &TimelineFrame::changeItemSize);

    // When user use cursor to drag to select area
    // The signal will be triggered frequently
    // Use timer to reset it
    QTimer *t = new QTimer(this);
    t->setSingleShot(true);
    connect(t, &QTimer::timeout, this, [=] {
        emit selectIndexChanged(m_view->selectionModel()->currentIndex());
    });
    connect(m_view->selectionModel(), &QItemSelectionModel::currentChanged,
            this, [=] (const QModelIndex &current, const QModelIndex &previous){
        Q_UNUSED(previous)
        if (current.isValid())
            t->start(200);
    });
}

void TimelineFrame::initTopTip()
{
    // Top-Tips
    m_tip = new TopTimelineTip(this);
    m_tip.setAnchor(Qt::AnchorTop, this, Qt::AnchorTop);
    // The preButton is anchored to the left of this
    m_tip.setAnchor(Qt::AnchorLeft, this, Qt::AnchorLeft);
    // NOTE: this is a bug of Anchors,the button should be resize after set anchor
    m_tip->setFixedWidth(this->width());
    m_tip.setTopMargin(TOP_TOOLBAR_HEIGHT);
    m_tip->setLeftMargin(m_view->horizontalOffset());
    connect(m_view, &TimelineView::paintingIndexsChanged, this, [=] {
        if (m_view->verticalOffset() > 10)
            m_tip->setText(currentMonth());
        else
            m_tip->setText("");
        m_tip->setLeftMargin(m_view->horizontalOffset());
    });
}

void TimelineFrame::initItems()
{
    auto infos = DBManager::instance()->getAllInfos();

    LoadThread *t = new LoadThread(infos, m_thumbnailSize, this);
    connect(t, &LoadThread::finished, this, [=] {
        t->deleteLater();
        m_infos << infos;
        m_pendingScrollRangeUpdate = true;
        if (m_pendingItems.isEmpty()) {
            updateScrollRange();
            m_pendingScrollRangeUpdate = false;
        }
    });
    t->start();
}

void TimelineFrame::enqueueItems(const TimelineItemDataList &datas)
{
    m_pendingItems << datas;
    if (!m_insertTimer->isActive()) {
        m_insertTimer->start();
    }
}

void TimelineFrame::processPendingItems()
{
    int count = 0;
    while (!m_pendingItems.isEmpty() && count < INSERT_BATCH_SIZE) {
        insertItem(m_pendingItems.takeFirst());
        ++count;
    }

    if (m_pendingItems.isEmpty()) {
        m_insertTimer->stop();
        if (m_pendingScrollRangeUpdate) {
            updateScrollRange();
            m_pendingScrollRangeUpdate = false;
        }
    }
}

void TimelineFrame::insertItem(const TimelineItem::ItemData &data)
{
    // Do not update model if user is scroll and importing, the missing datas
    // will be insert again after import thread finished by call initItems
    if (Importer::instance()->isRunning() && m_view->isScrolling()) {
        return;
    }
    m_model.appendData(data);
    if (!m_loadedPathSet.contains(data.path)) {
        m_loadedPathSet.insert(data.path);
        m_loadedPaths << data.path;
    }
}

void TimelineFrame::updateVisibleThumbnails()
{
    for (const QModelIndex &index : m_view->paintingIndexs()) {
        QVariantList datas = index.model()->data(index, Qt::DisplayRole).toList();
        if (datas.count() == 4 && !datas[0].toBool()) {
            updateThumbnail(datas[1].toString());
        }
    }
}

void TimelineFrame::removeItem(const DBImgInfo &info)
{
    TimelineItem::ItemData data;
    data.isTitle = false;
    data.path = info.filePath;
    data.timeline = utils::base::timeToString(info.time, true);

    // NOTE: THIS IS IMPORTANT
    // clear the selection to avoid call selectedPaths read some invalid data
    clearSelection();

    m_model.removeData(data);
    m_infos.removeAll(info);
    m_loadedPathSet.remove(info.filePath);
    m_loadedPaths.removeAll(info.filePath);

    m_view->updateScrollbarRange();
    m_view->updateView();
}

void TimelineFrame::removeItems(const DBImgInfoList &infos)
{
    // NOTE: THIS IS IMPORTANT
    // clear the selection to avoid call selectedPaths read some invalid data
    clearSelection();

    for (auto info : infos) {
        TimelineItem::ItemData data;
        data.isTitle = false;
        data.path = info.filePath;
        data.timeline = utils::base::timeToString(info.time, true);


        m_model.removeData(data);
        m_infos.removeAll(info);
        m_loadedPathSet.remove(info.filePath);
        m_loadedPaths.removeAll(info.filePath);

    }
    m_view->updateScrollbarRange();
    m_view->updateView();
}

LoadThread::LoadThread(const DBImgInfoList &infos, int thumbnailSize,
                       TimelineFrame *receiver)
    : QThread(nullptr)
    , m_infos(infos)
    , m_thumbnailSize(thumbnailSize)
    , m_receiver(receiver)
{

}

void LoadThread::enqueueBatch(const TimelineItemDataList &batch)
{
    if (!m_receiver || batch.isEmpty()) {
        return;
    }

    TimelineFrame *receiver = m_receiver;
    const TimelineItemDataList batchCopy = batch;
    QTimer::singleShot(0, receiver, [receiver, batchCopy] {
        receiver->enqueueItems(batchCopy);
    });
}

void LoadThread::run()
{
    using namespace utils::base;
    using namespace utils::image;

    const QStringList hashs = scanpathsHash();
    TimelineItemDataList batch;
    for (auto info : m_infos) {
        // Do not check the thumbnail for unplug devices' image
        if (onMountDevice(info.filePath) && ! mountDeviceExist(info.filePath)) {
            if (! hashs.contains(info.dirHash)) {
                continue;
            }
        }
        else if (! QFileInfo(info.filePath).exists() || ! hashs.contains(info.dirHash)) {
            continue;
        }
        TimelineItem::ItemData data;
        data.isTitle = false;
        data.path = info.filePath;

        QBuffer inBuffer( &data.thumbArray );
        inBuffer.open( QIODevice::WriteOnly );
        // write inPixmap into inByteArray
        if ( ! cutSquareImage(getThumbnail(data.path, true),
                              QSize(m_thumbnailSize, m_thumbnailSize)).save(&inBuffer, "JPG", 90 )) {
//             errorPaths << info.filePath;
        }
        data.timeline = timeToString(info.time, true);

        batch << data;
        if (batch.count() >= INSERT_BATCH_SIZE) {
            enqueueBatch(batch);
            batch.clear();
        }
    }

    enqueueBatch(batch);
}

const QStringList LoadThread::scanpathsHash()
{
    QStringList paths = dApp->setter->value(SCANPATHS_GROUP, SCANPATHS_KEY)
            .toString().split(",");
    paths.removeAll("");
    QStringList hashs;
    for (auto path : paths) {
        hashs << utils::base::hash(path);
    }
    hashs << utils::base::hash(QString());
    return hashs;
}
