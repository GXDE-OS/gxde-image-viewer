cmake_minimum_required(VERSION 3.10)
project(gxde-image-viewer VERSION 1.7.2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# 可选项: 开启包括但不限于相册、时间线、设置在内的所有功能
# 此可选项默认开启
option(FULL_FUNCTIONALITY "Enable full functionality (album, timeline, settings, etc.)" ON)

find_package(Qt5 REQUIRED COMPONENTS
    Core Gui Widgets Sql DBus Concurrent Svg X11Extras PrintSupport)
find_package(Qt5OpenGL QUIET)

find_package(PkgConfig REQUIRED)
pkg_check_modules(DTKWidget REQUIRED dtkwidget)
pkg_check_modules(X11 REQUIRED x11)
pkg_check_modules(Xext REQUIRED xext)
pkg_check_modules(LibExif REQUIRED libexif)
pkg_check_modules(GIO REQUIRED gio-unix-2.0)
find_library(FREEIMAGE_LIBRARY freeimage REQUIRED)

# 对于sw_64架构，启用 -mieee 选项以支持IEEE 754标准的浮点运算
if(CMAKE_SYSTEM_PROCESSOR MATCHES "sw_64")
    add_compile_options(-mieee)
endif()

set(FRAME_SOURCES
    frame/bottomtoolbar.cpp
    frame/extensionpanel.cpp
    frame/mainwidget.cpp
    frame/mainwindow.cpp
    frame/toptoolbar.cpp
)

set(MODULE_SOURCES
    # view
    module/view/navigationwidget.cpp
    module/view/viewpanel.cpp
    module/view/contents/ttlcontent.cpp
    module/view/contents/ttmcontent.cpp
    module/view/contents/imageinfowidget.cpp
    module/view/viewpanel_menu.cpp
    module/view/viewpanel_floating.cpp
    module/view/scen/imageview.cpp
    module/view/scen/graphicsitem.cpp
    module/view/thumbnailwidget.cpp
    module/view/lockwidget.cpp
    module/view/imagestrip.cpp
    module/view/qrcoderesultshower.cpp

    # slideshow
    module/slideshow/slideeffect.cpp
    module/slideshow/slideeffect_blinds.cpp
    module/slideshow/slideeffect_enter.cpp
    module/slideshow/slideeffectplayer.cpp
    module/slideshow/slideeffect_tile.cpp
    module/slideshow/slideshowpanel.cpp
    module/slideshow/slideeffect_switcher.cpp
    module/slideshow/slideeffect_circle.cpp
)

if(FULL_FUNCTIONALITY)
    list(APPEND MODULE_SOURCES
        # timeline
        module/timeline/timelinepanel.cpp
        module/timeline/timelinepanel_menu.cpp
        module/timeline/mvc/timelinedelegate.cpp
        module/timeline/mvc/timelineitem.cpp
        module/timeline/mvc/timelinemodel.cpp
        module/timeline/mvc/timelineview.cpp
        module/timeline/timelineframe.cpp
        module/timeline/contents/timelinebtcontent.cpp

        # album
        module/album/albumdelegate.cpp
        module/album/albumpanel.cpp
        module/album/albumsview.cpp
        module/album/imagesview.cpp
        module/album/topalbumtips.cpp
        module/album/contents/albumbtcontent.cpp
    )
endif()

# JQLibrary
set(JQLIBRARY_SOURCES
    module/JQLibrary/src/JQQRCodeReader/JQQRCodeReader.cpp
    module/JQLibrary/src/JQQRCodeReader/zxing/bigint/BigInteger.cc
    module/JQLibrary/src/JQQRCodeReader/zxing/bigint/BigIntegerAlgorithms.cc
    module/JQLibrary/src/JQQRCodeReader/zxing/bigint/BigIntegerUtils.cc
    module/JQLibrary/src/JQQRCodeReader/zxing/bigint/BigUnsigned.cc
    module/JQLibrary/src/JQQRCodeReader/zxing/bigint/BigUnsignedInABase.cc
    module/JQLibrary/src/JQQRCodeReader/zxing/zxing/aztec/decoder/Decoder1.cpp
    module/JQLibrary/src/JQQRCodeReader/zxing/zxing/aztec/detector/Detector1.cpp
    module/JQLibrary/src/JQQRCodeReader/zxing/zxing/aztec/AztecDetectorResult.cpp
    module/JQLibrary/src/JQQRCodeReader/zxing/zxing/aztec/AztecReader.cpp
    module/JQLibrary/src/JQQRCodeReader/zxing/zxing/common/detector/MonochromeRectangleDetector.cpp
    module/JQLibrary/src/JQQRCodeReader/zxing/zxing/common/detector/WhiteRectangleDetector.cpp
    module/JQLibrary/src/JQQRCodeReader/zxing/zxing/common/reedsolomon/GenericGF.cpp
    module/JQLibrary/src/JQQRCodeReader/zxing/zxing/common/reedsolomon/GenericGFPoly.cpp
    module/JQLibrary/src/JQQRCodeReader/zxing/zxing/common/reedsolomon/ReedSolomonDecoder.cpp
    module/JQLibrary/src/JQQRCodeReader/zxing/zxing/common/reedsolomon/ReedSolomonException.cpp
    module/JQLibrary/src/JQQRCodeReader/zxing/zxing/common/BitArray.cpp
    module/JQLibrary/src/JQQRCodeReader/zxing/zxing/common/BitArrayIO.cpp
    module/JQLibrary/src/JQQRCodeReader/zxing/zxing/common/BitMatrix.cpp
    module/JQLibrary/src/JQQRCodeReader/zxing/zxing/common/BitSource.cpp
    module/JQLibrary/src/JQQRCodeReader/zxing/zxing/common/CharacterSetECI.cpp
    module/JQLibrary/src/JQQRCodeReader/zxing/zxing/common/DecoderResult.cpp
    module/JQLibrary/src/JQQRCodeReader/zxing/zxing/common/DetectorResult.cpp
    module/JQLibrary/src/JQQRCodeReader/zxing/zxing/common/GlobalHistogramBinarizer.cpp
    module/JQLibrary/src/JQQRCodeReader/zxing/zxing/common/GreyscaleLuminanceSource.cpp
    module/JQLibrary/src/JQQRCodeReader/zxing/zxing/common/GreyscaleRotatedLuminanceSource.cpp
    module/JQLibrary/src/JQQRCodeReader/zxing/zxing/common/GridSampler.cpp
    module/JQLibrary/src/JQQRCodeReader/zxing/zxing/common/HybridBinarizer.cpp
    module/JQLibrary/src/JQQRCodeReader/zxing/zxing/common/IllegalArgumentException.cpp
    module/JQLibrary/src/JQQRCodeReader/zxing/zxing/common/PerspectiveTransform.cpp
    module/JQLibrary/src/JQQRCodeReader/zxing/zxing/common/Str.cpp
    module/JQLibrary/src/JQQRCodeReader/zxing/zxing/common/StringUtils.cpp
    module/JQLibrary/src/JQQRCodeReader/zxing/zxing/datamatrix/decoder/BitMatrixParser1.cpp
    module/JQLibrary/src/JQQRCodeReader/zxing/zxing/datamatrix/decoder/DataBlock1.cpp
    module/JQLibrary/src/JQQRCodeReader/zxing/zxing/datamatrix/decoder/DecodedBitStreamParser1.cpp
    module/JQLibrary/src/JQQRCodeReader/zxing/zxing/datamatrix/decoder/Decoder2.cpp
    module/JQLibrary/src/JQQRCodeReader/zxing/zxing/datamatrix/detector/CornerPoint.cpp
    module/JQLibrary/src/JQQRCodeReader/zxing/zxing/datamatrix/detector/Detector2.cpp
    module/JQLibrary/src/JQQRCodeReader/zxing/zxing/datamatrix/detector/DetectorException.cpp
    module/JQLibrary/src/JQQRCodeReader/zxing/zxing/datamatrix/DataMatrixReader.cpp
    module/JQLibrary/src/JQQRCodeReader/zxing/zxing/datamatrix/Version1.cpp
    module/JQLibrary/src/JQQRCodeReader/zxing/zxing/multi/qrcode/detector/MultiDetector.cpp
    module/JQLibrary/src/JQQRCodeReader/zxing/zxing/multi/qrcode/detector/MultiFinderPatternFinder1.cpp
    module/JQLibrary/src/JQQRCodeReader/zxing/zxing/multi/qrcode/QRCodeMultiReader.cpp
    module/JQLibrary/src/JQQRCodeReader/zxing/zxing/multi/ByQuadrantReader.cpp
    module/JQLibrary/src/JQQRCodeReader/zxing/zxing/multi/GenericMultipleBarcodeReader.cpp
    module/JQLibrary/src/JQQRCodeReader/zxing/zxing/multi/MultipleBarcodeReader.cpp
    module/JQLibrary/src/JQQRCodeReader/zxing/zxing/oned/CodaBarReader.cpp
    module/JQLibrary/src/JQQRCodeReader/zxing/zxing/oned/Code128Reader.cpp
    module/JQLibrary/src/JQQRCodeReader/zxing/zxing/oned/Code39Reader.cpp
    module/JQLibrary/src/JQQRCodeReader/zxing/zxing/oned/Code93Reader.cpp
    module/JQLibrary/src/JQQRCodeReader/zxing/zxing/oned/EAN13Reader.cpp
    module/JQLibrary/src/JQQRCodeReader/zxing/zxing/oned/EAN8Reader.cpp
    module/JQLibrary/src/JQQRCodeReader/zxing/zxing/oned/ITFReader.cpp
    module/JQLibrary/src/JQQRCodeReader/zxing/zxing/oned/MultiFormatOneDReader.cpp
    module/JQLibrary/src/JQQRCodeReader/zxing/zxing/oned/MultiFormatUPCEANReader.cpp
    module/JQLibrary/src/JQQRCodeReader/zxing/zxing/oned/OneDReader.cpp
    module/JQLibrary/src/JQQRCodeReader/zxing/zxing/oned/OneDResultPoint.cpp
    module/JQLibrary/src/JQQRCodeReader/zxing/zxing/oned/UPCAReader.cpp
    module/JQLibrary/src/JQQRCodeReader/zxing/zxing/oned/UPCEANReader.cpp
    module/JQLibrary/src/JQQRCodeReader/zxing/zxing/oned/UPCEReader.cpp
    module/JQLibrary/src/JQQRCodeReader/zxing/zxing/pdf417/decoder/ec/ErrorCorrection.cpp
    module/JQLibrary/src/JQQRCodeReader/zxing/zxing/pdf417/decoder/ec/ModulusGF.cpp
    module/JQLibrary/src/JQQRCodeReader/zxing/zxing/pdf417/decoder/ec/ModulusPoly.cpp
    module/JQLibrary/src/JQQRCodeReader/zxing/zxing/pdf417/decoder/BitMatrixParser2.cpp
    module/JQLibrary/src/JQQRCodeReader/zxing/zxing/pdf417/decoder/DecodedBitStreamParser2.cpp
    module/JQLibrary/src/JQQRCodeReader/zxing/zxing/pdf417/decoder/Decoder3.cpp
    module/JQLibrary/src/JQQRCodeReader/zxing/zxing/pdf417/detector/Detector3.cpp
    module/JQLibrary/src/JQQRCodeReader/zxing/zxing/pdf417/detector/LinesSampler.cpp
    module/JQLibrary/src/JQQRCodeReader/zxing/zxing/pdf417/PDF417Reader.cpp
    module/JQLibrary/src/JQQRCodeReader/zxing/zxing/qrcode/decoder/BitMatrixParser3.cpp
    module/JQLibrary/src/JQQRCodeReader/zxing/zxing/qrcode/decoder/DataBlock2.cpp
    module/JQLibrary/src/JQQRCodeReader/zxing/zxing/qrcode/decoder/DataMask.cpp
    module/JQLibrary/src/JQQRCodeReader/zxing/zxing/qrcode/decoder/DecodedBitStreamParser3.cpp
    module/JQLibrary/src/JQQRCodeReader/zxing/zxing/qrcode/decoder/Decoder4.cpp
    module/JQLibrary/src/JQQRCodeReader/zxing/zxing/qrcode/decoder/Mode.cpp
    module/JQLibrary/src/JQQRCodeReader/zxing/zxing/qrcode/detector/AlignmentPattern.cpp
    module/JQLibrary/src/JQQRCodeReader/zxing/zxing/qrcode/detector/AlignmentPatternFinder.cpp
    module/JQLibrary/src/JQQRCodeReader/zxing/zxing/qrcode/detector/Detector4.cpp
    module/JQLibrary/src/JQQRCodeReader/zxing/zxing/qrcode/detector/FinderPattern.cpp
    module/JQLibrary/src/JQQRCodeReader/zxing/zxing/qrcode/detector/FinderPatternFinder2.cpp
    module/JQLibrary/src/JQQRCodeReader/zxing/zxing/qrcode/detector/FinderPatternInfo.cpp
    module/JQLibrary/src/JQQRCodeReader/zxing/zxing/qrcode/ErrorCorrectionLevel.cpp
    module/JQLibrary/src/JQQRCodeReader/zxing/zxing/qrcode/FormatInformation.cpp
    module/JQLibrary/src/JQQRCodeReader/zxing/zxing/qrcode/QRCodeReader.cpp
    module/JQLibrary/src/JQQRCodeReader/zxing/zxing/qrcode/Version2.cpp
    module/JQLibrary/src/JQQRCodeReader/zxing/zxing/BarcodeFormat.cpp
    module/JQLibrary/src/JQQRCodeReader/zxing/zxing/Binarizer.cpp
    module/JQLibrary/src/JQQRCodeReader/zxing/zxing/BinaryBitmap.cpp
    module/JQLibrary/src/JQQRCodeReader/zxing/zxing/ChecksumException.cpp
    module/JQLibrary/src/JQQRCodeReader/zxing/zxing/DecodeHints.cpp
    module/JQLibrary/src/JQQRCodeReader/zxing/zxing/Exception.cpp
    module/JQLibrary/src/JQQRCodeReader/zxing/zxing/FormatException.cpp
    module/JQLibrary/src/JQQRCodeReader/zxing/zxing/InvertedLuminanceSource.cpp
    module/JQLibrary/src/JQQRCodeReader/zxing/zxing/LuminanceSource.cpp
    module/JQLibrary/src/JQQRCodeReader/zxing/zxing/MultiFormatReader.cpp
    module/JQLibrary/src/JQQRCodeReader/zxing/zxing/Reader.cpp
    module/JQLibrary/src/JQQRCodeReader/zxing/zxing/Result.cpp
    module/JQLibrary/src/JQQRCodeReader/zxing/zxing/ResultIO.cpp
    module/JQLibrary/src/JQQRCodeReader/zxing/zxing/ResultPoint.cpp
    module/JQLibrary/src/JQQRCodeReader/zxing/zxing/ResultPointCallback.cpp
)

set(WIDGETS_SOURCES
    widgets/importframe.cpp widgets/popupmenustyle.cpp widgets/thumbnaillistview.cpp
    widgets/tooltip.cpp widgets/imagebutton.cpp widgets/blureframe.cpp
    widgets/processtooltip.cpp widgets/scrollbar.cpp widgets/scaleslider.cpp
    widgets/slider.cpp widgets/blureinfoframe.cpp widgets/separator.cpp
    widgets/thumbnaildelegate.cpp widgets/loadingicon.cpp widgets/pushbutton.cpp
    widgets/themewidget.cpp widgets/formlabel.cpp widgets/elidedlabel.cpp
    widgets/returnbutton.cpp widgets/dspinner.cpp widgets/toast.cpp
    widgets/printoptionspage.cpp widgets/printhelper.cpp
)

if(FULL_FUNCTIONALITY)
    list(APPEND WIDGETS_SOURCES
        widgets/dialogs/filedeletedialog.cpp widgets/dialogs/albumdeletedialog.cpp
        widgets/dialogs/albumcreatedialog.cpp widgets/dialogs/dirimportdialog.cpp
        widgets/dialogs/aboutdialog.cpp widgets/dialogs/imginfodialog.cpp
        widgets/dialogs/dialog.cpp
    )
endif()

set(UTILS_SOURCES
    utils/imageutils.cpp utils/baseutils.cpp
    utils/shortcut.cpp utils/snifferimageformat.cpp
)

set(CONTROLLER_SOURCES
    controller/signalmanager.cpp controller/wallpapersetter.cpp
    controller/commandline.cpp controller/configsetter.cpp
    controller/globaleventfilter.cpp controller/viewerthememanager.cpp
)

if(FULL_FUNCTIONALITY)
    list(APPEND CONTROLLER_SOURCES
        controller/importer.cpp controller/divdbuscontroller.cpp
        controller/dbmanager.cpp controller/exporter.cpp
    )
endif()

set(SERVICE_SOURCES
    service/deepinimageviewerdbus.cpp service/mimesappsmanager.cpp
)

set(SETTINGS_SOURCES
    settings/settingswindow.cpp settings/titlebutton.cpp settings/titleframe.cpp
    settings/contentsframe.cpp
    settings/slideshow/slideshowframe.cpp settings/shortcut/shortcutframe.cpp
    settings/title.cpp
    settings/slideshow/slideshowpreview.cpp settings/shortcut/shortcuteditor.cpp
)

set(DIRWATCHER_SOURCES
    dirwatcher/scanpathsdialog.cpp dirwatcher/scanpathsitem.cpp
    dirwatcher/volumemonitor.cpp
)

set(MAIN_SOURCES main.cpp application.cpp)

set(RESOURCES
    resources.qrc frame/frametheme.qrc module/view/viewtheme.qrc
    widgets/widgetstheme.qrc dirwatcher/dirwatcher.qrc
)

if(FULL_FUNCTIONALITY)
    list(APPEND RESOURCES
        module/timeline/timelinetheme.qrc module/album/albumtheme.qrc
        widgets/dialogs/dialogs.qrc settings/settings.qrc
    )
endif()

# AUTOMOC需要的头
file(GLOB ALL_HEADERS
    "*.h"
    "controller/*.h" "dirwatcher/*.h" "frame/*.h"
    "module/*.h"
    "module/album/*.h" "module/album/contents/*.h"
    "module/slideshow/*.h"
    "module/timeline/*.h" "module/timeline/contents/*.h" "module/timeline/mvc/*.h"
    "module/view/*.h" "module/view/contents/*.h" "module/view/scen/*.h"
    "module/JQLibrary/include/JQQRCodeReader/JQQRCodeReader.h"
    "service/*.h"
    "settings/*.h" "settings/shortcut/*.h" "settings/slideshow/*.h"
    "utils/*.h"
    "widgets/*.h" "widgets/dialogs/*.h"
)

list(REMOVE_ITEM ALL_HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/widgets/bluredialog.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/module/view/scen/imagewidget.h"
)

add_executable(${PROJECT_NAME}
    ${ALL_HEADERS}
    ${FRAME_SOURCES} ${MODULE_SOURCES} ${JQLIBRARY_SOURCES}
    ${WIDGETS_SOURCES} ${UTILS_SOURCES} ${CONTROLLER_SOURCES}
    ${SERVICE_SOURCES} ${SETTINGS_SOURCES} ${DIRWATCHER_SOURCES}
    ${MAIN_SOURCES} ${RESOURCES}
)

set(LOCAL_MOC_SOURCES
    widgets/blureinfoframe.cpp
    widgets/thumbnaildelegate.cpp
    module/album/imagesview.cpp
    module/timeline/mvc/timelinedelegate.cpp
    module/view/contents/imageinfowidget.cpp
)
set_property(SOURCE ${LOCAL_MOC_SOURCES} PROPERTY SKIP_AUTOMOC ON)
foreach(MOC_CPP ${LOCAL_MOC_SOURCES})
    get_filename_component(MOC_BASENAME ${MOC_CPP} NAME_WE)
    qt5_generate_moc(${CMAKE_CURRENT_SOURCE_DIR}/${MOC_CPP}
        "${CMAKE_CURRENT_BINARY_DIR}/${MOC_BASENAME}.moc")
    target_sources(${PROJECT_NAME} PRIVATE
        "${CMAKE_CURRENT_BINARY_DIR}/${MOC_BASENAME}.moc")
endforeach()

target_compile_definitions(${PROJECT_NAME} PRIVATE
    APPSHAREDIR="/usr/share/gxde-image-viewer"
    QT_MESSAGELOGCONTEXT
    ZXING_ICONV_CONST DISABLE_LIBRARY_FEATURES NO_ICONV
    JQQRCODEREADER_COMPILE_MODE_STRING="SRC"
    JQQRCODEREADER_VERSIONSTRING="1.6"
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/utils
    ${DTKWidget_INCLUDE_DIRS}
    ${X11_INCLUDE_DIRS} ${Xext_INCLUDE_DIRS}
    ${LibExif_INCLUDE_DIRS} ${GIO_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/module/JQLibrary/include/JQQRCodeReader
    ${CMAKE_CURRENT_SOURCE_DIR}/module/JQLibrary/src/JQQRCodeReader/zxing
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt5::Core Qt5::Gui Qt5::Widgets Qt5::Sql Qt5::DBus Qt5::Concurrent
    Qt5::Svg Qt5::X11Extras Qt5::PrintSupport
    ${DTKWidget_LIBRARIES} ${X11_LIBRARIES} ${Xext_LIBRARIES}
    ${LibExif_LIBRARIES} ${GIO_LIBRARIES} ${FREEIMAGE_LIBRARY}
    ${CMAKE_THREAD_LIBS_INIT} ${CMAKE_DL_LIBS}
)

if(Qt5OpenGL_FOUND)
    target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::OpenGL)
endif()

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
