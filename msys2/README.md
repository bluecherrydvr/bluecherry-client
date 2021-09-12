# Building for Windows using MSYS2

## Installing MSYS2
Download the installer from [official site](https://www.msys2.org/) or [GitHub Releases page](https://github.com/msys2/msys2-installer/releases).

Run the installer. When done, run "MSYS2 MSYS" from Start menu.  

Update the package database and base packages:

```bash
pacman -Syu
```

Terminal will be closed. Run "MSYS2 MSYS" from Start menu again and update the rest of base packages:

```bash
pacman -Su
```

## Installing developer tools and dependency packages
Run "MSYS2 MSYS" from Start menu and install packages:

```bash
pacman -S base-devel git mingw-w64-{i686,x86_64}-toolchain mingw-w64-{i686,x86_64}-nsis mingw-w64-{i686,x86_64}-ffmpeg mingw-w64-{i686,x86_64}-mpv mingw-w64-{i686,x86_64}-qt5
```

## Building client application and installer
Run "MSYS2 MinGW 32-bit" or "MSYS2 MinGW 64-bit" from Start Menu.

Clone repository:

```bash
cd /c/$ProjectDir/

git clone https://github.com/bluecherrydvr/bluecherry-client.git
```

Build:

```bash
cd bluecherry-client/msys2/

./build-client.sh

./build-installer.sh
```

>Note: `build-installer.sh` shell script relies on `copydlldeps.sh` shell script from MXE (M cross environment).
It is copied from MXE repository [mxe/tools/copydlldeps.sh](https://github.com/mxe/mxe/blob/74dd24ca82a5e48ccaf0b549eaef1db07ed382ad/tools/copydlldeps.sh).  
This is a leftover after porting from MXE to MSYS2 building environment.

Run client application without installing:

```bash
bluechery-client
```
