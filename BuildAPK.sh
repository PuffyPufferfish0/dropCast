#!/usr/bin/env bash
set -e

# Paths provided by our Nix Flake environment
SDK=$ANDROID_SDK_ROOT
NDK=$ANDROID_NDK_ROOT
BUILD_TOOLS=$SDK/build-tools/34.0.0
PLATFORM=$SDK/platforms/android-33

# Point directly to the Toolchain instead of searching for it!
TOOLCHAIN="$NDK/build/cmake/android.toolchain.cmake"

# Fallback just in case Nix used the older 'ndk-bundle' naming convention
if [ ! -f "$TOOLCHAIN" ]; then
    TOOLCHAIN="$SDK/ndk-bundle/build/cmake/android.toolchain.cmake"
fi

if [ ! -f "$TOOLCHAIN" ]; then
    echo "============================================================"
    echo "ERROR: The NDK Toolchain is STILL missing!"
    echo "This means your terminal is using the old environment."
    echo ""
    echo "CRITICAL STEP:"
    echo "1. Type 'exit' and press Enter to leave this fish shell."
    echo "2. Run 'nix develop --impure -c fish' to reload it."
    echo "3. Run 'cd APK_Make' and try again."
    echo "============================================================"
    exit 1
fi

echo "Found Toolchain: $TOOLCHAIN"

echo "=== 1. Compiling Android Library (ARM64) ==="
# CRITICAL FIX: Wipe the old CMake cache from the previous failed run!
rm -rf build_android 
mkdir -p build_android
cd build_android

# Use -G Ninja and the direct TOOLCHAIN path
cmake .. -G Ninja \
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

$BUILD_TOOLS/aapt package -f -M AndroidManifest.xml \
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
echo "=== SUCCESS: DropCastHost.apk has been created! ==="
