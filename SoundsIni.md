# Introduction #

Since a long time sounds can be activated in gpsVP (Menu Setup/Sound). There is a single default sound which is played on following events:
  * When connection to gps is established or lost.
  * When approaching a waypoint (depending on the radius of each waypoint)

Starting with gpsVP v0.4.24 it is possible to configure an alternative wav file and there are a few more events.

# Details #

To avoid losing the configuration while updating to a new version of gpsvp, the configuration of sounds is not done through the registry but through an ini file "Sounds.ini" which must be manually created and placed in the same folder as gpsvp.exe

Example for Sounds.ini:
```
[System]
GPS=boing.wav
Proximity=DEFAULT

[Altitude]
X000=frog.wav
X100=horse-gallop.wav
X200=sheep.wav
X300=cow.wav
X400=horse.wav
X500=peacock.wav
X600=rooster.wav
X700=duck.wav
X800=cuckoo.wav
X900=seagull.wav
```

| **Section** | **Key** | **Description** | **If missing** |
|:------------|:--------|:----------------|:---------------|
| `System` | `GPS` | When connection to gps is established or lost | default sound |
| `System` | `Proximity` | When approaching a waypoint (depending on the radius of each waypoint) | default sound |
| `Altitude` | `X000` | When crossing the altitude of 0m, 1000m, 2000m, ... Going up or down, sound will be played again only if crossing the altitude again after being at least 50m higher or lower | no sound |
| `Altitude` | `X100` | When crossing the altitude of 100m, 1100m, 2100m, ... | no sound |
| `Altitude` | `X200` | When crossing the altitude of 200m, 1200m, 2200m, ... | no sound |
| `Altitude` | `X300` | When crossing the altitude of 300m, 1300m, 2300m, ... | no sound |
| `Altitude` | `X400` | When crossing the altitude of 400m, 1400m, 2400m, ... | no sound |
| `Altitude` | `X500` | When crossing the altitude of 500m, 1500m, 2500m, ... | no sound |
| `Altitude` | `X600` | When crossing the altitude of 600m, 1600m, 2600m, ... | no sound |
| `Altitude` | `X700` | When crossing the altitude of 700m, 1700m, 2700m, ... | no sound |
| `Altitude` | `X800` | When crossing the altitude of 800m, 1800m, 2800m, ... | no sound |
| `Altitude` | `X900` | When crossing the altitude of 900m, 1900m, 2900m, ... | no sound |

Notes:
  * The wav file must use a codec that can be played with the mobile phone. "PCM U8" works usually fine, avoid MPEG codecs. Try to play the each wav file with the media player on the mobile phone to test it.
  * If a file can't be found (or if the name is DEFAULT) the default sound will be played. The sound is included in gpsVP.exe so that no wav file is necessary.
  * Don't forget to also copy the wav files on the mobile phone.