## Q: What language is written gpsVP with? ##

A: gpsVP is written in C++ using the WIN32-API. It currently compiles on Windows XP/Vista, Windows Mobile 5/6 (ppc or smartphone) and Windows Mobile 2003 (ppc or smartphone).


## Q: What do I need to compile gpsVP? ##

A1: You need Microsoft Visual Studio 2005 (standard edition or better) and the Windows Mobile SDKs to be able to compile all versions.

A2: You can also use Visual Studio 2008 but the project files needs to be converted. VS will do it automatically, but the repository contains the project files for VS2005, so it might be difficult to check in your changes to the project files back in the repository. Change to the source code are not a problem as long as you don't use functions which are only available in VS2008.

A3: With the free [Visual C++ 2008 Express](http://www.microsoft.com/express/vc/) you can compile **only the Windows XP version** of gpsVP. To compile the Windows Mobile version you need one of the pay version of VS because the Windows Mobile SDK won't install with VS Express.


## Q: Which SDKs do I need? ##

A: You need the [Windows Mobile 5.0 SDK for Smartphone](http://www.microsoft.com/downloads/details.aspx?familyid=DC6C00CB-738A-4B97-8910-5CD29AB5F8D9) and the [Windows Mobile 5.0 SDK for Pocket PC](http://www.microsoft.com/DownLoads/details.aspx?FamilyID=83a52af2-f524-4ec5-9155-717cbe5d25ed). I'm not quite sure anymore but they seems to includes the SDK for WM 2003.

Alternatively you can probably install the [Windows Mobile 5.0 Developer Resource Kit](http://www.microsoft.com/downloads/details.aspx?familyid=3baa5b7d-04c1-4ec2-83dc-61b21ec5fe57) which seems to include both SDK and more (emulators...).

The "Windows Mobile 6 Professional and Standard Software Development Kits Refresh" is not needed.


## Q: How do I compile? ##

A: Open one of the project files (`*`.vcproj), choose if you want to compile the release or the debug version and launch the build.

There are currently 6 project files:
  * VisualPainterXP: for Windows XP/Vista
  * VisualPainterSP: for WM2003 Smartphone (= without touchscreen)
  * VisualPainterWM5: for WM5/6 Smartphone (= without touchscreen)
  * VisualPainterPPC: for WM2003 PPC (= with touchscreen)
  * VisualPainterWM5PPC: for WM5/6 PPC (= with touchscreen)
  * VisualPainterCE5: It is an attempt to compile for Windows CE 5 that is not finished.

Windows Mobile 2003 is based on Windows CE 4.2, Windows Mobile 5 or 6 is based on Windows CE 5. See [The Windows CE article on Wikipedia](http://en.wikipedia.org/wiki/Windows_ce) for an explanation of the relationship from Windows CE to Windows Mobile.

## Q: Why does the compiler generate a `VisualPainter.exe` instead of a `gpsVP.exe`? ##

A: Vsevolod explained the origin of the name in a post on the discussion group: "`VisualPainter` is an original name of the program. When I started the project I first wrote a program that parsed Garmin maps. Then I needed another program to visualize then. And I called it `VisualPainter`. Adding feature by feature I turned it into a navigation program but I really needed no other name for it. But when it came to publish it I had to invent something more descriptive. The main change from `VisualPainter` was that it now could connect to GPS. And I reduced the name to VP and added gps prefix."


## Q: how can I test that my changes work on all platforms? ##

A: You don't have to have one device for each platform. There is a free [device emulator](http://www.microsoft.com/DownLoads/details.aspx?familyid=A6F6ADAF-12E3-4B2F-A394-356E2C2FB114) from Microsoft. Emulator images can also be downloaded for each platforms from Microsft website. (The SDK or DDK includes one version of the emulator)


## Q: How to enable the debug mode? ##

A: There is a debug mode, which add a debug menu to gpsVP with sotme tests functions (like a replay track function). To enable debug mode you will need to create (on Windows XP with `RegEdit.exe`) a new binary value `DebugMode` with the value 01 in the registry under
`HKEY_CURRENT_USER\Software\Vsevolod Shorin\VSMapViewer`.
You don't need to have a debug version of gpsVP, the release version supports this setting too.