Note:  It is assumed that you have some experience in compiling Qt applications under your operating system.  
If you do not have Qt development experience it is recommended that you download the binaries from 
www.bluecherrydvr.com/downloads

If you have general questions about features of the client feel free to post on community.bluecherrydvr.com.  
If you find a bug or want to discuss a pending feature, feel free to use the 'Issues' tab on 
github.com/bluecherrydvr/bluecherry-client.  Occasional support can be found on irc.bluecherry.net/bluecherry

---------

For Ubuntu: see BUILD-UBUNTU.txt
For Windows: see BUILD-WINDOWS.txt

Building:
    mkdir build; cd build
    qmake ../client.pro
    make (or nmake)
    
    For a release build, use: qmake ../client.pro 'CONFIG += release'

Without breakpad: qmake ../client.pro 'CONFIG += no-breakpad'

Breakpad (Windows + MSVC):
    Python is required.
    In the breakpad dir, run 'src/tools/gyp/gyp.bat src/client/windows'
    Open src/client/windows/breakpad_client.sln
    Set build type and architecture as needed
    Build all, then build the application normally.
    
    If CRT linker errors appear, change the CRT linking for all projects except build_all
    to use the multithreaded (debug/release) DLL.
    
Breakpad (Mac):
    Open breakpad/src/client/mac/Breakpad.xcodeproj
    Set built type and architecture
    Under targets, build 'All'
    Copy breakpad/src/client/mac/build/{Debug,Release}/Breakpad.framework to breakpad-bin
    (Optional) Remove unnecessary files from the framework
    Build the application
