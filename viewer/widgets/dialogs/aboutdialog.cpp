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
#include "aboutdialog.h"

namespace {

const QString WINDOW_ICON = "";
const QString PRODUCT_ICON = ":/dialogs/images/resources/images/deepin-image-viewer.png";
const QString VERSION_VIEWER = "1.2";

}  // namespace

AboutDialog::AboutDialog()
    : DAboutDialog()
{
    setWindowFlags(this->windowFlags() | Qt::WindowStaysOnTopHint);
    setModal(true);
    setProductIcon(QIcon(PRODUCT_ICON));
    setProductName(tr("GXDE Image Viewer"));
    setVersion(tr("Version:") + VERSION_VIEWER);
    //FIXME: acknowledgementLink is empty!
    setAcknowledgementLink("https://gitee.com/GXDE-OS/gxde-image-viewer/");

    connect(this, SIGNAL(closed()), this, SLOT(deleteLater()));
}
