﻿/*
Copyright (c) 2005-2008, Vsevolod E. Shorin
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Vsevolod E. Shopin nor the names of contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#ifndef COMMANDS_H
#define COMMANDS_H

enum enumMenuCommands
{
	mcMapList = 0x1000,
	mcOpenMap,
	mcOpenTrack,
	mcTrackList,
	mcOpenWaypoints,
	mcOpenColors,
	mcExit,
	mcZoomIn,
	mcZoomOut,
	mcLeft,
	mcRight,
	mcUp,
	mcDown,
	mcLeftClickOrContextMenu,
	mcoMonitorsMode,
	mcNextMonitorsRow,
	mcPrevMonitorsRow,
	mcAddWaypoint,
	mcWaypointsList,
	mcNavigateRecent,
	mcStopNavigating,
	mcNewTrack,
	mcoFollowCursor,
	mcoShowMonitorBar,
	mcoShowRoadName,
	mcoFullScreen,
	mcoBuffered,
	mcoSound,
	mcoKeepBacklight,
	mcSetTrackFolder,
	mcKeymap,
	mcSettings,
	mcSetDetail1,
	mcSetDetail2,
	mcSetDetail3,
	mcSetDetail4,
	mcSetDetail5,
	mcIncreaseDetail,
	mcDecreaseDetail,
	mcConnectPeriod0,
	mcConnectPeriod1,
	mcConnectPeriod2,
	mcConnectPeriod4,
	mcConnectPeriod9,
	mcoConnect,
	mcoShowCenter,
	mcoDebugMode,
	mcRegisterFileTypes,
	mcoWriteConnectionLog,
	mcAbout,
	mcoKeepDeviceOn,
	mcoLowMemory,
	mcNextColors,
	mcoLowLight,
	mcSetTime,
	mcoWriteTrack,
	mcDebugNmeaCommands,
	mcDebugCursorHere,
	mcDebugNoFix,
	mcDebugShowTimes,
	mcUnknownPointTypes,
	mcImportWaypoints,
	mcOpenMapFolder,
	mcoBluetoothOn,
	mcoScreenButtons,
	mcoShowDetailMaps,
	mcoShowPOIs,
	mcoDirectPaint,
	mcoWarnNoGPS,
	mcoRotateMap,
	mcCloseAllMaps,
	mcoShowUnknownTypes,
	mcOpenTranslation,
	mcReplayTrack,
	mcCreateTrafficNodeObsolete,
	mcRefreshTraffic,
	mcAboutTraffic,
	mcoRefreshTrafficOnStartup,
	mcAboutMaps,
	mcCheckLatestVersion,
	mcoShowPolygonLabels,
	mcoShowAreaAsOutline,
	mcoShowTrafficNodes,
	mcSetGoogleMapsFolder,
	mcoDownloadGoogleMaps,
	mcoShowWaypoints,
	mcPrevGMapType,
	mcNextGMapType,
	mcoShowCurrentTrack,
	mcCloseTranslation,
	mcCloseColors,
	mcoShowAreaName,
	mcoShowGarminMaps,
	mcoGoogleZoomLevels,
	mcoShowFastestWay,
	mcoShowTrafficInformation,
	mcoTestServer,
	mcDownlRasterAddCurrentView,
	mcDownlRasterStartWithCurZoom,
	mcDownlRasterByTrack,
	mcoDownloadLowerLevels,
	mcoRasterCacheAutoDelete,
	mcRastMapsDeleteFromCache,
	mcoTrafficFlags,
	mcReplayNMEA,
	mcoLargeMonitors,
	mcDumpNMEA,
	mcoAutoLight,
	mcoLargeFonts,
	mcSearchOSM,
	mcSearchResults,
	mcoAllowInternetAlways, // Also used as a single inet-status flag 
	// if roaming detection not supported
	mcoUseProxy,
	mcTeamSettings,
	mcTeamUpdateNow,
	mcTeamUpdatePeriodically,
	mcoInvertSatelliteImages,
	mcNewWaypointsWPT,
	mcNewWaypointsGPX,
	mcExportWaypointsWPT,
	mcExportWaypointsGPX,
	mcExportWaypointsOSM,
	mcTrackFormatPLT,
	mcTrackFormatGPX,
	mcoQuickReadGPXTrack,
	mcoShowSunAz,
	mcoMultitrackAsSingleTrack,
	mcGoToDemoPoint,
	mcInfoCurTrack,
	mcContextMenu,
	mcEditRoute,
	mcLoadRoute,
	mcSaveRoute,
	mcClearRoute,
	mcInfoCurRoute,
	mcCenterRouteTarget,
	mcNavigatingToPoint,
	mcNavigatingAlongRoute,
	mcoAllowInternetNever,
	mcoAllowInternetHomeOnly,
	mcRastMapsRefreshAll,
	mcRastMapsRefreshInsideRegion,
	mcoHideCacheTiles,
	mcoPlaceholder1,
	mcDownlRasterShowAvailableTiles,

	mcGMapType = 0x2000,
	mcFirstWMSMapType = mcGMapType + 9,
	// ...reserved for Map Types...
	mcLastGMapType = 0x2FFF,
};

#endif // COMMANDS_H
