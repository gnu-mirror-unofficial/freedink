all: build.xml
	(cd ../cross-android/ && make)
	cp -a ../cross-android/src/freedink libs/armeabi/libmain.so
	arm-linux-androideabi-strip libs/armeabi/*.so
	ant debug
	ant installd
	adb shell am start -a android.intenon.MAIN -n org.freedink/org.freedink.FreeDink

update:
	adb shell am force-stop org.freedink
	adb push ../cross-android/src/freedink /data/data/org.freedink/lib/libmain.so
	adb shell am start -a android.intenon.MAIN -n org.freedink/org.freedink.FreeDink

SDK_TARGET=12
build.xml:
	android update project --name freedink --path . --target android-$(SDK_TARGET)
