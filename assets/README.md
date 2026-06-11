# RTMZ 3.0 recovered assets

These assets were recovered from the historical `rtmz30.zip` runtime archive
listed by The Hornet Archive and mirrored by Scene.org:

`https://ftp.scene.org/pub/mirrors/hornet/graphics/programs/players/rtmz30.zip`

The original archive also contains `RTMZ.EXE` and `DOS4GW.EXE`; those
executables are intentionally not vendored here. The retained files are the
demo recording, font, documentation, and the `3DS`, `GEM`, and `GEO` model
directories used by the viewer.

The original runtime layout appears to be current-directory based:

```text
RTMZ30/
  RTMZ.EXE
  DOS4GW.EXE
  FONT4X7.PCX
  DEMO.REC
  3DS/
  GEM/
  GEO/
```

The source loads `font4x7.pcx` and the default `DEMO.REC` from the process
current directory, and the Open menu starts from `getcwd()`. A modern build
pipeline should therefore stage these retained files beside the built viewer
executable, or run the executable with this directory as its working directory.
