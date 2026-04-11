#!/usr/bin/env bash
set -e

# Paths provided by our Nix Flake environment
SDK=$ANDROID_SDK_ROOT
NDK=$ANDROID_NDK_ROOT
BUILD_TOOLS=$SDK/build-tools/34.0.0
PLATFORM=$SDK/platforms/android-33
TOOLCHAIN="$NDK/build/cmake/android.toolchain.cmake"

if [ ! -f "$TOOLCHAIN" ]; then
    TOOLCHAIN="$SDK/ndk-bundle/build/cmake/android.toolchain.cmake"
fi

echo "=== 1. Compiling Client Library (ARM64) ==="
rm -rf build_client_android
mkdir -p build_client_android
cd build_client_android

# Pointing to the Client_APK folder
cmake ../Client_APK -G Ninja \
    -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN" \
    -DANDROID_ABI=arm64-v8a \
    -DANDROID_PLATFORM=android-33 \
    -DCMAKE_BUILD_TYPE=Release

ninja
cd ..

echo "=== 2. Packaging Client APK ==="
APK_DIR="build_client_android/apk_staging"
rm -rf $APK_DIR
mkdir -p $APK_DIR/lib/arm64-v8a

cp build_client_android/libmain.so $APK_DIR/lib/arm64-v8a/

$BUILD_TOOLS/aapt package -f -M Client_APK/AndroidManifest.xml \
    -I $PLATFORM/android.jar \
    -F build_client_android/app-unaligned.apk \
    $APK_DIR

echo "=== 3. Aligning and Signing ==="
$BUILD_TOOLS/zipalign -f 4 build_client_android/app-unaligned.apk build_client_android/app-aligned.apk

if [ ! -f debug.keystore ]; then
    keytool -genkeypair -validity 365 -dname "CN=DropCast" \
        -keystore debug.keystore -storepass android -keypass android \
        -alias androiddebugkey -keyalg RSA
fi

$BUILD_TOOLS/apksigner sign --ks debug.keystore \
    --ks-pass pass:android build_client_android/app-aligned.apk

mv build_client_android/app-aligned.apk DropCastClient.apk

echo "==========================================================="
echo "=== SUCCESS: DropCastClient.apk has been created!       ==="
echo "==========================================================="