#!/usr/bin/env bash
set -e

# Paths provided by our Nix Flake environment
SDK=$ANDROID_SDK_ROOT
NDK=$ANDROID_NDK_ROOT
BUILD_TOOLS=$SDK/build-tools/34.0.0
PLATFORM=$SDK/platforms/android-33

# Point directly to the Toolchain
TOOLCHAIN="$NDK/build/cmake/android.toolchain.cmake"

if [ ! -f "$TOOLCHAIN" ]; then
    TOOLCHAIN="$SDK/ndk-bundle/build/cmake/android.toolchain.cmake"
fi

if [ ! -f "$TOOLCHAIN" ]; then
    echo "ERROR: The NDK Toolchain is missing! Did you reload the Nix shell?"
    exit 1
fi

# Safety check to ensure we run from the root folder
if [ ! -d "APK_Make" ]; then
    echo "ERROR: Could not find the 'APK_Make' folder! Make sure your Android Manifest and CMakeLists.txt are in there."
    exit 1
fi

echo "Found Toolchain: $TOOLCHAIN"

echo "=== 1. Compiling Android Library (ARM64) ==="
rm -rf build_android 
rm -f DropCastHost.apk # Failsafe: Nuke the old APK so we don't accidentally copy it!
mkdir -p build_android
cd build_android

# CRITICAL FIX: We explicitly point CMake to ../APK_Make instead of ..
# This guarantees it uses the Android configuration, not the PC configuration!
cmake ../APK_Make -G Ninja \
    -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN" \
    -DANDROID_ABI=arm64-v8a \
    -DANDROID_PLATFORM=android-33 \
    -DCMAKE_BUILD_TYPE=Release

# Use ninja instead of make
ninja
cd ..

echo "=== 2. Packaging APK ==="
APK_DIR="build_android/apk_staging"
rm -rf $APK_DIR
mkdir -p $APK_DIR/lib/arm64-v8a

# Copy our compiled game library into the APK folder structure
cp build_android/libmain.so $APK_DIR/lib/arm64-v8a/

# Explicitly use the Android Manifest from the APK_Make folder
$BUILD_TOOLS/aapt package -f -M APK_Make/AndroidManifest.xml \
    -I $PLATFORM/android.jar \
    -F build_android/app-unaligned.apk \
    $APK_DIR

echo "=== 3. Aligning and Signing ==="
$BUILD_TOOLS/zipalign -f 4 build_android/app-unaligned.apk build_android/app-aligned.apk

# Generate a signing key if we don't have one
if [ ! -f debug.keystore ]; then
    echo "Generating debug keystore..."
    keytool -genkeypair -validity 365 -dname "CN=DropCast" \
        -keystore debug.keystore -storepass android -keypass android \
        -alias androiddebugkey -keyalg RSA
fi

$BUILD_TOOLS/apksigner sign --ks debug.keystore \
    --ks-pass pass:android build_android/app-aligned.apk

mv build_android/app-aligned.apk DropCastHost.apk

echo "==========================================================="
echo "=== SUCCESS: DropCastHost.apk has been created!         ==="
echo "==========================================================="
