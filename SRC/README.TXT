Apr/3/1997
Here it is !
The release 3.0. This is basically the 2.9 with the menu bug fixed,
major hires speedups, Z-Buffer speedups, chrome mapping, more compact
".rec" files and other small things.
The purpose of the 3.0 is to stabilize a version (the original 2.9
is in my computer in Rome) and make it the last version.
I don't consider this engine to be at the state of the art. Hires
is much faster now but the inner loops aren't blazing fast.
The perspective corrected texture mapping (PCTM) is now done using
a division every 16 pixels. This is definitely the best solution
that I know, although the inner loop isn't using FPU/CPU parallelism
as every Pentium oriented optimal PCTM should do.
I'm releasing the source code, even though I'm not 100% proud of
it, because people always asked me.
Objects can have linked objects (lists) only because it's the way
they are organized in the 3DS format. In RTMZ objects aren't
hierarchically structured nor the engine provides for an optimal
culling (tuned for the viewer when you have one object at the time).
For those that want to use the menu system, the API isn't that
simple and I must warn that it's all integrated with the RTMZ-OS
library. This library is on top of DOS and handles the events and
the interface to the devices.

I'm very proud of the advanced (advanced as for a real-time
application) lighting effects because after 2 years only now I
can see games starting to use them. The lighting is quite tricky
about the specularity: the specular vector isn't specular at all !
If I had to introduce a real specular component that would have
thrown away all the concept of fast table-based colored lighting.
Another thing I'm very proud of is the subpixel accuracy.. which
is also the "secret" of the cool smoothness of the animation.
Chrome mapping was added very quickly.. so the code might look
unhappy.
The code is all C+ (better C, extended C or whatever you call it)
but a class to make fractal noise that I use to build the standard
texture map. I made that class long time ago when I was into C++
and I kept that. The engine actually went from C to C++ to C+.
If I had to write a new engine today (almost everyday 8) then I'd
use plain old C.
The closest companion I had in the development of the actual 3D
code is "Advanced Animation and Rendering Techiniques" by
Alan Watt and Mark Watt. This book introduced me to subpixel
rasterization and helped me out about homogeneous coordinates
and lighting effects.
Other material that came handy (the wheel was invented long time
ago..):
  - Polygon rasterization and homogeneous coordinate clipping from
    the Graphics Gems source code by Paul Heckbert.
  - 3DS parser from 3DS2POV by Steve Anger and Jeff Bowermaster.
  - Matrices from MESA library, Graphics Gems, VOGLE, etc etc.
  - VESA support, Phil Jones.
  - Trackball ..cannot find the original source, I think it was an
    SGI demo.

You are free to contact me as long as you don't have to say
things like "I noticed you use alot of ''short'' did you know
they are slow on the Pentium ?". The code is mostly 2 years old
and so on..
If you want to make a game or something, then consider that I
already work for a software house: I'm not the man..
I'm pretty much sick of software rendering and I'm trying to get
some fun with 3Dfx and PSX.

DISCLAMIER:
This software is provided "as is". You use it at your own risk, I'm
not responsible for misuse or any kind of direct/indirect damage
attributable to the software in question.

NOTES:
The archive containing this file is Public Domain material. You can use
it as you want as long as you don't sell it.
You are free to use the source code as long as you don't modify and
redistriubuite it to grant credits for yourself. (But then you wouldn't
care about this note 8).

Have fun

ciaox

Davide Pasca - dpasca@ix.netcom.com
               dpasca@val.net
               http://www.netcom.com/~dpasca
               http://val.net/~dpasca
