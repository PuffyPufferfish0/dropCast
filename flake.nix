{
  description = "Custom Console Monorepo - Raylib Edition";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-24.11";
    nixgl.url = "github:guibou/nixGL";
  };

  outputs = { self, nixpkgs, nixgl }:
  let
    system = "x86_64-linux";
    pkgs = import nixpkgs {
      inherit system;
      config.allowUnfree = true;
      config.android_sdk.accept_license = true;
    };

    androidPackages = pkgs.androidenv.composeAndroidPackages {
      cmdLineToolsVersion = "11.0";
      toolsVersion = "26.1.1";
      platformToolsVersion = "35.0.2";
      buildToolsVersions = [ "34.0.0" ];
      platformVersions = [ "33" "34" ];
      ndkVersions = [ "26.1.10909125" ]; 
      abiVersions = [ "arm64-v8a" "armeabi-v7a" ];
    };

    # THE FIX: Use the Default/Intel wrapper, NOT the Nvidia one!
    # This provides the exact 'nixGL' command you used in your first screenshot.
    gpuWrapper = nixgl.packages.${system}.nixGLDefault;

  in
  {
    devShells.${system}.default = pkgs.mkShell {
      buildInputs = with pkgs; [
        cmake
        ninja
        pkg-config
        clang_18
        llvmPackages_18.bintools
        raylib
        glfw
        libglvnd
        mesa
        libGL
        libGLU
        xorg.libX11
        xorg.libXcursor
        xorg.libXrandr
        xorg.libXinerama
        xorg.libXi
        xorg.libXext
        alsa-lib
        jdk17
        androidPackages.androidsdk 
        
        # Add the correct wrapper to the shell
        gpuWrapper
      ];

      shellHook = ''
        export ANDROID_SDK_ROOT="${androidPackages.androidsdk}/libexec/android-sdk"
        export ANDROID_NDK_ROOT="$ANDROID_SDK_ROOT/ndk/26.1.10909125"
        export JAVA_HOME="${pkgs.jdk17}"
        
        echo "Raylib Dev Environment Loaded (Stable 24.11)."
        echo "---"
        echo "To launch your app with Intel acceleration, use:"
        echo "nixGL ./build/Host/HostApp"
      '';
    };
  };
}