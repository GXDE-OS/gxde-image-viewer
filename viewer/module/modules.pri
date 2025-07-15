!isEmpty(FULL_FUNCTIONALITY) {
    include (timeline/timeline.pri)
    include (album/album.pri)
}

include (view/view.pri)
#include (edit/edit.pri)
include (slideshow/slideshow.pri)
# 引入识别二维码的第三方库
JQQRCODEREADER_COMPILE_MODE = SRC
include (JQLibrary/JQQRCodeReader.pri)

HEADERS += \
    $$PWD/modulepanel.h
