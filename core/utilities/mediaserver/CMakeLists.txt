#
# Copyright (c) 2010-2018 by Gilles Caulier, <caulier dot gilles at gmail dot com>
# Copyright (c) 2017      by Ahmed Fathi, <ahmed dot fathi dot abdelmageed at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if (POLICY CMP0063)
    cmake_policy(SET CMP0063 NEW)
endif (POLICY CMP0063)

set(upnpsdk_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Platinum/Source/Core/PltAction.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Platinum/Source/Core/PltArgument.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Platinum/Source/Core/PltConstants.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Platinum/Source/Core/PltCtrlPoint.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Platinum/Source/Core/PltCtrlPointTask.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Platinum/Source/Core/PltDatagramStream.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Platinum/Source/Core/PltDeviceData.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Platinum/Source/Core/PltDeviceHost.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Platinum/Source/Core/PltEvent.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Platinum/Source/Core/PltHttp.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Platinum/Source/Core/PltHttpClientTask.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Platinum/Source/Core/PltHttpServer.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Platinum/Source/Core/PltHttpServerTask.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Platinum/Source/Core/PltIconsData.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Platinum/Source/Core/PltMimeType.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Platinum/Source/Core/PltProtocolInfo.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Platinum/Source/Core/PltService.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Platinum/Source/Core/PltSsdp.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Platinum/Source/Core/PltStateVariable.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Platinum/Source/Core/PltTaskManager.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Platinum/Source/Core/PltThreadTask.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Platinum/Source/Core/PltUPnP.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Platinum/Source/Devices/MediaServer/PltDidl.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Platinum/Source/Devices/MediaServer/PltFileMediaServer.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Platinum/Source/Devices/MediaServer/PltMediaBrowser.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Platinum/Source/Devices/MediaServer/PltMediaCache.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Platinum/Source/Devices/MediaServer/PltMediaItem.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Platinum/Source/Devices/MediaServer/PltMediaServer.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Platinum/Source/Devices/MediaServer/ConnectionManagerSCPD.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Platinum/Source/Devices/MediaServer/ContentDirectorySCPD.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Platinum/Source/Devices/MediaServer/ContentDirectorywSearchSCPD.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Platinum/Source/Devices/MediaServer/PltSyncMediaBrowser.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Platinum/Source/Devices/MediaRenderer/PltMediaRenderer.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Platinum/Source/Devices/MediaRenderer/PltMediaController.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Platinum/Source/Devices/MediaRenderer/AVTransportSCPD.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Platinum/Source/Devices/MediaRenderer/RdrConnectionManagerSCPD.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Platinum/Source/Devices/MediaRenderer/RenderingControlSCPD.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Platinum/Source/Devices/MediaConnect/X_MS_MediaReceiverRegistrarSCPD.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Platinum/Source/Devices/MediaConnect/PltMediaConnect.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Neptune/Source/Core/NptAutomaticCleaner.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Neptune/Source/Core/NptBase64.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Neptune/Source/Core/NptBufferedStreams.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Neptune/Source/Core/NptCommon.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Neptune/Source/Core/NptDataBuffer.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Neptune/Source/Core/NptDebug.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Neptune/Source/Core/NptFile.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Neptune/Source/Core/NptHash.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Neptune/Source/Core/NptHttp.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Neptune/Source/Core/NptList.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Neptune/Source/Core/NptLogging.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Neptune/Source/Core/NptMessaging.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Neptune/Source/Core/NptNetwork.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Neptune/Source/Core/NptQueue.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Neptune/Source/Core/NptResults.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Neptune/Source/Core/NptRingBuffer.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Neptune/Source/Core/NptSimpleMessageQueue.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Neptune/Source/Core/NptSockets.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Neptune/Source/Core/NptStreams.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Neptune/Source/Core/NptStrings.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Neptune/Source/Core/NptSystem.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Neptune/Source/Core/NptThreads.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Neptune/Source/Core/NptTime.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Neptune/Source/Core/NptTls.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Neptune/Source/Core/NptUri.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Neptune/Source/Core/NptUtils.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Neptune/Source/Core/NptXml.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Neptune/Source/System/Bsd/NptBsdSockets.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Neptune/Source/System/StdC/NptStdcDebug.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Neptune/Source/System/StdC/NptStdcEnvironment.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Neptune/Source/System/StdC/NptStdcFile.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Neptune/Source/System/Posix/NptPosixEnvironment.cpp
)

if(NOT MSVC)

    # Always compiled excepted under Windows with MSVC compiler.

    list(APPEND upnpsdk_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Neptune/Source/System/Posix/NptPosixSystem.cpp
                             ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Neptune/Source/System/Posix/NptSelectableMessageQueue.cpp
                             ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Neptune/Source/System/Posix/NptPosixQueue.cpp
                             ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Neptune/Source/System/Posix/NptPosixThreads.cpp
                             ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Neptune/Source/System/Posix/NptPosixFile.cpp
    )

endif()

if(WIN32)

    # Under Windows with MXE

    list(APPEND upnpsdk_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Neptune/Source/System/Win32/NptWin32Time.cpp
                             ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Neptune/Source/System/Win32/NptWin32File.cpp
                             ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Neptune/Source/System/Win32/NptWin32Network.cpp
    )

else()

    # For Linux and Apple

    list(APPEND upnpsdk_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Neptune/Source/System/Posix/NptPosixTime.cpp
                             ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Neptune/Source/System/Bsd/NptBsdNetwork.cpp
    )

endif()

if(APPLE)

    # For Apple only

    list(APPEND upnpsdk_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Neptune/Source/System/Apple/NptAppleAutoreleasePool.mm
                             ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Neptune/Source/System/Apple/NptAppleLogConfig.mm
    )

endif()

#if(MSVC)
#
#    list(APPEND upnpsdk_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Neptune/Source/System/Win32/NptWin32Console.cpp
#                             ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Neptune/Source/System/Win32/NptWin32Debug.cpp
#                             ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Neptune/Source/System/Win32/NptWin32DynamicLibraries.cpp
#                             ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Neptune/Source/System/Win32/NptWin32MessageQueue.cpp
#                             ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Neptune/Source/System/Win32/NptWin32Queue.cpp
#                             ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Neptune/Source/System/Win32/NptWin32SerialPort.cpp
#                             ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Neptune/Source/System/Win32/NptWin32System.cpp
#                             ${CMAKE_CURRENT_SOURCE_DIR}/upnpsdk/Neptune/Source/System/Win32/NptWin32Threads.cpp
#    )
#
#endif()

set(libmediaserver_SRCS
    dlnaserver.cpp
    dlnaserverdelegate.cpp
    dmediaserver.cpp
    dmediaservermngr.cpp
    dmediaserverdlg.cpp
    ${upnpsdk_SRCS}
)

include_directories($<TARGET_PROPERTY:Qt5::Widgets,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt5::Xml,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt5::Network,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF5::I18n,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF5::XmlGui,INTERFACE_INCLUDE_DIRECTORIES>
)

add_library(mediaserver_src OBJECT ${libmediaserver_SRCS})
