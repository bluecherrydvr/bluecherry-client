MPlayer/MEncoder Win32/Win64 binary Builds by Redxii <redxii@users.sourceforge.net> for SMPlayer
Report bugs to http://smplayer.sf.net or http://mplayerwin.sf.net

The included subfont.ttf is DejaVu Serif,
Copyright (c) 2003 by Bitstream, Inc. All Rights Reserved. Bitstream Vera is
a trademark of Bitstream, Inc.
DejaVu changes are in public domain.


The bottom of this file has a list of keyboard shortcuts to control MPlayer.
A complete list of command line parameters is in mplayer.html. More
documentation is in the docs/ directory. Start reading at index.html or
skip right to windows.html for Windows-specific information. Note that
while most of the documentation is written with the Unix version in mind
most things are valid for Windows as well.


The files in this package:
mplayer.exe       - the main executable
mencoder.exe      - a full-featured video encoder
vfw2menc.exe      - a tool used to configure Vfw codecs, please look at
                    docs/menc-feat-video-for-windows.html
MPlayer.html      - usage and command line options
MPlayer.man.html  - usage and command line options
Readme.txt        - this document
fonts/            - FontConfig configuration
licenses/         - license information for 3rd-party libraries used
                    by mplayer
codecs/           - binary codecs (Real, QuickTime etc.) directory;
                    You can download some codecs from our download page:
                    http://www.mplayerhq.hu/design7/dload.html
                    Note: Not supported on 64-bit MPlayer!

mplayer/codecs.conf.in     - codec configuration built into MPlayer/MEncoder, rename to 'codecs.conf'
                             if you wish to modify it 
mplayer/config             - configuration file with adjustable options (see config.sample)
mplayer/input.conf         - keybinding configuration file

A list of Windows frontends for MPlayer can be found at
http://www.mplayerhq.hu/design7/projects.html

A mailing list for Windows-related questions is at
http://lists.mplayerhq.hu/mailman/listinfo/mplayer-users


KEYBOARD CONTROL

general control
  <- and ->
       Seek backward/forward 10 seconds.
  up and down
       Seek backward/forward 1 minute.
  pgup and pgdown
       Seek backward/forward 10 minutes.
  [ and ]
       Decreases/increases current playback speed by 10%.
  { and }
       Halves/doubles current playback speed.
  Backspace
       Reset playback speed to normal.
  < and >
       backward/forward in playlist
  HOME and END
       next/previous playtree entry in the parent list
  INS and DEL
       next/previous alternative source (ASX playlist only)
  p / SPACE
       Pause movie (pressing again unpauses).
  .
       Step forward.  Pressing once  will  pause  movie,  every
       consecutive  press  will play one frame and then go into
       pause mode again (any other key unpauses).
  q / ESC
       Stop playing and quit.
  + and -
       Adjust audio delay by +/- 0.1 seconds.
  / and *
       Decrease/increase volume.
  9 and 0
       Decrease/increase volume.
  m
       Mute sound.
  f
       Toggle fullscreen (also see -fs).
  T
       Toggle stay-on-top (also see -ontop).
  w and e
       Decrease/increase pan-and-scan range.
  o
       Toggle OSD states: none / seek / seek + timer /  seek  +
       timer + total time.
  d
       Toggle frame dropping states: none / skip display / skip
       decoding (see -framedrop and -hardframedrop).
  v
       Toggle subtitle visibility.
  b / j
       Cycle through the available subtitles.
  F
       Toggle displaying "forced subtitles".
  a
       Toggle subtitle aligment: top/middle/bottom.
  z and x
       Adjust subtitle delay by +/- 0.1 seconds.
  r and t
       Move subtitles up/down.
  i
       Set EDL mark.

(The following keys are valid only when using  a  hardware  accelerated
video  output  (xv,  (x)vidix,  (x)mga, etc), or the software equalizer
filter (-vf eq or -vf eq2).

  1 and 2
       Adjust contrast.
  3 and 4
       Adjust brightness.
  5 and 6
       Adjust hue.
  7 and 8
       Adjust saturation.
