# Smart-Trash-Can
A Smart Trash Can which locates the trash and autonomously moves towards it in an attempt to catch it.

Steps to run the project:
1. Hardware Requirements:
    Microsoft Xbox Kinect v2.0, Windows Laptop.
2. Software requirements:
    - Visual Studio 2013
    - OpenCV 3.10
3. Edit the “OpenCV 3.1.0 64-bit” xml file: Change the path in these lines.
    <AdditionalLibraryDirectories>$(OPENCV)\build\x64\vc12\lib;%(AdditionalLibraryDir ectories)</AdditionalLibraryDirectories>
    <AdditionalDependencies>opencv_world310.lib;%(AdditionalDependencies)</Additi onalDependencies>
4. Compiling Kinect and Windows Code
    - Open the “Gesture” VC++ Project provided in folder Gesture.
    - Go to View -> Other Windows -> Property Manager. Select “Add Existing
    Property Sheet” and add the xml you just edited.
    - Go to Project -> Properties
    - In Configuration Properties, go to VC++ Directories.
    - In the “Include Directories” and “Library Directories”, add the path of your
    OpenCV 3.1.0 “Include” and “Lib” directories.
    - Go to Linker -> Input. In Additional Dependencies, in the empty dialog box paste
    these lines:
    opencv_world310.lib
    Kinect20.lib
    Kinect20.Face.lib Kinect20.Fusion.lib Kinect20.VisualGestureBuilder.lib kernel32.lib
    user32.lib
    gdi32.lib
    winspool.lib
    comdlg32.lib
    advapi32.lib
    shell32.lib
    ole32.lib
    oleaut32.lib
    uuid.lib odbc32.lib odbccp32.lib ws2_32.lib
    Make sure to uncheck “Inherit from parent or project defaults”
    - Press Ok. Save the project using Ctrl + S.
    - Go to Build -> Build Gesture. The project should get built without any errors.
    - Press F5 to run the project.
5. Compiling Gumstix Code.
    - Compile the motor driver code m_driver.c using Makefile. Just type make.
    - Make sure you do /ad/eng/courses/ec/ec535/bashrc_ec535 before doing a
    make.
    - For the Bluetooth code b.c, compile by typing:
    arm-linux-gcc b.c –o b -lbluetooth

You should see few windows pop up. One of them would be feeding camera’s input. Just ignore all this. You are good to go.
