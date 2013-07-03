virtual_input
=============

create virtual input and manage it in android system

Android:

configure your kernel to support user input driver

put the directy user external directory
lunch your android profile
cd this directory; mm

then push the input file user /system/xbin/ directory on target device

Linux:
just input make in shell

sudo ./input event_file interval on ubuntu


tested on ubuntu and android 4.2

known issues:
the mouse event can not work well in android 4.2
the touch event can not work


