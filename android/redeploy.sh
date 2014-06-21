#!/bin/bash -ex
(cd ../cross-android/&&make)
\cp -a ../cross-android/src/freedink libs/armeabi/libmain.so
ant debug
ant installd
adb shell am start -a android.intenon.MAIN -n org.freedink/org.freedink.FreeDink
