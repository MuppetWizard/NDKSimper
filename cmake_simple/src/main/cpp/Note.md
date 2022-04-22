**Clang ndk23**  
arm:
--isysroot = Android\ndk\android-ndk-r23b\toolchains\llvm\prebuilt\windows-x86_64\sysroot\usr\ffmpeg.lib\arm-linux-androideabi\21
-isystem = Android\ndk\android-ndk-r23b\toolchains\llvm\prebuilt\windows-x86_64\sysroot\usr\include
-isystem = Android\ndk\android-ndk-r23b\toolchains\llvm\prebuilt\windows-x86_64\sysroot\usr\include\arm-linux-androideabi

aarch64-linux-android:
-isysroot = Android\ndk\android-ndk-r23b\toolchains\llvm\prebuilt\windows-x86_64\sysroot\usr\ffmpeg.lib\aarch64-linux-android
-isystem = Android\ndk\android-ndk-r23b\toolchains\llvm\prebuilt\windows-x86_64\sysroot\usr\include
-isystem = Android\ndk\android-ndk-r23b\toolchains\llvm\prebuilt\windows-x86_64\sysroot\usr\include\aarch64-linux-android

使用clang编译c文件为.a静态库
1、将test.c使用NDK Clang 编译为 test.o 文件，命令如下：
```
<platform>-clang -fPIC -c test.c -o test.o
```
2、使用ar工具将 test.o 生成test.a 静态库，如下：
```
llvm-ar r test.a test.o
```

