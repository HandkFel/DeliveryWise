# DeliveryWise
// DeliveryWise date: 22.0.2020 //

// Executing the Software //

There is a complete build in the folder DeliveryWise\release. So you don't have to necessarily compile the Software.
You can execute it on Windows by just double clicking on release\DeliveryWise.exe.

// User Guide //

After execution a window opens up. Now follow the instructions on the bottom of the window (steps 1 - 3)

// COMPILING //

The Software was compiled using Qt 5.12.8 (All Qt Versions 4.6 to 5.12 work as well) and the MinGW 32 Bit Compiler. To compile the software:

1. Download and install Qt 5.12.8 (including QtCreator)
2. Open DeliveryWise/DeliveryWise.pro with QtCreator 
3. Configure project (MinGW 32 Bit Compiler; Configuration: Release)
3. In Qt creator under "Projects" -> No shadow build (Build directory should be DeliveryWise\release)
3. Execute qmake
4. Hit the build button
5. Finished!

