#!/bin/bash
cd `dirname $0`
lupdate -recursive . -ts translations/gxde-image-viewer_*.ts
