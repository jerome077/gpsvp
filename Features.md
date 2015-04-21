_This page is a little bit out of date. Hopefully someone will find the time to add all the new features sooner or later. Meanwhile have a look at the other wiki pages or simply try gpsVP!_

_Some of the features not yet listed here: usage of vector (garmin) and raster maps (web based or scanned maps, user configurable), support for proxy servers for web based maps, GPX and Ozi file formats for waypoints and tracks, standard and UTM coodinates, ..._

# Display #
## Main ##

Main display shows the following items:

  * Some portion of maps loaded into program
  * Cross indicating the centre of map (if turned on)
  * Three or four monitors on bottom or right edge (if turned on)
  * Big black point for current position (is GPS receiver determined location)
  * Satellite icon indicating status of GPS receiver. With red minus when no connection is present, blue question mark when connection is present by location is not determined and green plus when location is determined
  * Current polyline (usually road) name on top edge of scree (if turned on)
  * Current area name on top edge of scree (if turned on)
  * Waypoints earlier added by user
  * Track recorded during current session
  * Tracks explicitly loaded by user
  * Traffic information (if loaded and turned on)
  * Fastest (if available) or just straight way to a destination.
  * Scale

## Monitors mode ##

In monitors mode display shows only monitors. But many of them.
Brightness

You can select low light mode to use your device in darkness.
Maps

gpsVP works with maps in Garmin MapSource format. Although the format was developed by Garmin, free maps in this format are usually produced with cgpsmapper. gpsVP opens several maps and then chooses maps to display based on current position scale and selected detail level.

# Tracks #

gpsVP records a track always when GPS receiver determines location. Track recorded since the program was last started is shown on screen. Recorded tracks are written to card. Each session is written into separate file. Please notice that the files are not deleted automatically. It’s up to you to choose location with enough space and delete old tracks. Only one track format is supported. It’s OziExplorer’s plt.
You can also set minimum track step to preserve card space and memory. When it’s set, the next point is added to track only when it differs from previous more than the step.
There are several ways to use the tracks. You can use the tracks to …

  * … mark some places on your map after you left them.
  * … show you way to your friends.
  * … geocode your photos.
  * … update maps or send your tracks yo people who develop free maps.

By the way I have a project where I accept any tracks, process them and share with map developers. You are welcome to send me your tracks or visit the site of the project (only in Russian yet).

# Monitors #

Monitors are small rectangle areas showing some information. Every monitor has a header indicating what this monitor shows. Here is the complete list of them:

  * Altitude
  * Connection
  * Distance
  * GPS time
  * HDOP
  * Max speed
  * Memory
  * Odo 1
  * Odo 2
  * Odo total
  * Pause timer
  * Satellites
  * Speed

# Waypoints #

gpsVP allows user mark any location with a flag. Such waypoint also has title typed in by user, altitude if it was available while marking the place and proximity radius. Setting proximity radius allows the program warn you with a sound when approaching to the point.
Waypoint are saved to card in OziExplorer’s format (wpt). You can copy this file or add some points to it using any software on you computer.

# Navigation #

gpsVP supports simple navigation. You can select any location and tell the program that you are going there. After that it begins to show you direction to the point. Distance monitors shows distance from current location to the point. Navigation only works when GPS receiver is connected and determined location.

# GPS receiver #

GPS receiver can be connected via BlueTooth. First you need to connect your smartphone to COM port profile of your device. After that you need to select the port you’ve connected to your device in Menu->Setup->Settings dialog.
GPS connection modes

In (Menu->GPS->Connect period) submenu you can change GPS connection from permanent to periodical and set connect period. In such mode gpsVP connects to GPS receiver for several seconds, receives information and then disconnect for selected time. Such mode produces irregular track but may be useful in two cases. First of all it greatly decreses GPS receiver power consumption so it can operate much longer. Secondly it allows several smartphones connect to one receiver at the same time.

# Keyboard layout #

Every operation you can perform with gpsVP is available from menu. To speed up operation you can assign any action to almost any key. By default assignment is the following:

  * joystick left, right, up, down - move map or monitor selection left, up, right or down
  * joystick press - context menu
  * 1/2 - zoom in/out
  * 3 - switch between main display and monitors mode
  * 4/5 - increase/decrease detail
  * 6 - switch fullscreen mode
  * 7/8 - move monitor bar selection up/down
  * 9 - show/hide monitors bar in main display
  * `*` - new waypoint
  * 0 - stop navigating
  * # - navigate one of recent destinations

# Menu #

Menu has the following structure:

  * Maps
    * Map list
    * Open map
  * Tracks
    * Open track
    * Track list
    * Start new track
    * Set track folder
  * Waypoints
    * Open waypoints file
    * Add waypoint
    * Waypoints list
  * Navigation
    * Navigate recent
    * Stop navigating
  * Display
    * View
      * Zoom in
      * Zoom out
      * Left
      * Right
      * Up
      * Down
      * Follow cursor
    * Detail
      * Decrease detail
      * Increase detail
      * Set lowest detail
      * Set low detail
      * Set normal detail
      * Set high detail
      * Set highest detail
    * Monitor bar
      * Previous monitors row
      * Next monitors row
      * Show monitors bar
    * Open color settings
    * Next color scheme
    * Monitors mode
    * Show road name
    * Full screen
    * Show center
    * Low light
  * GPS
    * Connect period
      * Always
      * 1 minute
      * 2 minutes
      * 4 minutes
      * 9 minutes
    * Connect GPS
  * Setup
    * Settings
    * Keymap
    * Register file types
    * About
    * Sound
    * Keep backlight
    * Keep memory low
  * Context menu
  * Exit