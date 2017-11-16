# ScratchyShow

ScratchyShow is a graphical user interface to [SCRATCHy](https://github.com/OpenTactile/SCRATCHy) and [ITCHy](https://github.com/OpenTactile/SCRATCHy) - The open hardware- and software-plattform for controlling tactile displays.

This Documentation is currently under development.

## Building and installing dependencies
ScratchyShow requires the following dependencies to be installed:
- [libSCRATCHy](https://github.com/OpenTactile/SCRATCHy)
- [libITCHy](https://github.com/OpenTactile/ITCHy)
- [Qt 5.9](https://www.qt.io/)
- [libusb](http://libusb.info/)

## Building ScratchyShow
ScratchyShow uses the qmake build system and is available for linux platforms only. For building and installing the application system-wide, please follow these steps within the base directory:

```
mkdir ScratchyShow-build
cd ScratchyShow-build
qmake ..
make && make install
```

#### Using *fake mode*
If you intend to test and/or extend ScratchyShow on a personal computer (not using the Raspberry Pi 3), you may pass an additional configuration option to qmake that disables some platform specific tests at runtime

```
qmake ../ScratchyShow.pro CONFIG+=fake
make && make install
```
Please make sure that libSCRATCHy and libITCHy are build with this option as well.


## Running the application
*TODO*

#### Basic features
*TODO*

#### Structure of the *working directory*
*TODO*

#### Running without X Server
*TODO*

## Creating scenarios
*TODO*

#### Format of *.tsc* files
An example file is given in the `workingdir/tactileScenes` folder:
```
Example Scenario

Config {
    BackgroundColor = #404040
    Translate[0] = 0.0
    Translate[1] = 0.0
    DisableSecretButton = False
    HideToolbar = False
}

Scene {
    -8;-1.5;3;3 | surface_graphics/circle.png;bump_plates/circle.png   | Frequency;250.0;0.9;false;false;0.0 | true
    -4;-1.5;3;3 | surface_graphics/diamond.png;bump_plates/diamond.png | Frequency;50.0;0.9;false;false;0.2  | true
     0;-1.5;3;3 | surface_graphics/flake.png;bump_plates/flake.png     | Frequency;100.0;0.9;true;false;0.4  | true
     4;-1.5;3;3 | surface_graphics/hexagon.png;bump_plates/hexagon.png | Frequency;250.0;0.9;false;true;0.6  | true
     8;-1.5;3;3 | surface_graphics/plus.png;bump_plates/plus.png       | Frequency;250.0;0.9;true;true;0.8   | true
}
```

Permutation example:
```
Randomization Example Scenario

Config {
    graphics = surface_graphics/circle.png;bump_plates/circle.png
    model = Frequency;250.0;0.5;false;false;0.0
}

Substitute {
    POSX = ( 0.0 | 1.0 | 2.0 | 3.0 | 4.0 | 5.0 | 6.0 | 7.0 )
    POSY = ( 0.0 | 1.0 | 2.0 | 3.0 | 4.0 | 5.0 | 6.0 | 7.0 )
    SIZE = ( 1;1 | 2;2 | 3;3 )
}

Scene {
    %POSX0%;%POSY0%;%SIZE0% | graphics | model | false
    %POSX1%;%POSY1%;%SIZE1% | graphics | model | false
    %POSX2%;%POSY2%;%SIZE2% | graphics | model | false
}
```


#### Calibrating the monitor
In order to take the size of the screen into account (so 1cm travelled with ITCHy translates to 1cm on the screen), please edit the file `hdmiscreen.cpp` within the `view` folder:
```
const float screenWidth = 0.52f;
const float screenHeight = 0.25f;
```
These values are given in metres.
(We will make this configurable through the scene-file in future releases.)

#### Tactile display definition
Currently the definition of the tactile display takes place within the `main.cpp`.
(This will be moved to the scene-file as well in future releases.)
Having a look on lines 65-71 reveals the standard definition:
```
TactileDisplay tactileDisplay;

tactileDisplay << Actuator(QVector2D(-4.0, 0.0), QVector2D(0.71, 12.5),  0, 1, 2)
               << Actuator(QVector2D(-2.0, 0.0), QVector2D(0.71, 12.5),  0, 1, 0)
               << Actuator(QVector2D( 0.0, 0.0), QVector2D(0.71, 12.5),  0, 2, 0)
               << Actuator(QVector2D( 2.0, 0.0), QVector2D(0.71, 12.5),  0, 1, 1)
               << Actuator(QVector2D( 4.0, 0.0), QVector2D(0.71, 12.5),  0, 1, 3);
```
Following this scheme, as many actuators as needed may be added to the tactile display object.
The example given above creates a single-row tactile display definition consisting of five actuators evenly spaced with a distance of 2mm between individual actuators.
The Actuator constructor's arguments are organized as follows:
```
Actuator(position, size, map, port, pin)
```
The first two arguments should be self-explanatory. (The `size` argument is only for cosmetical purposes and defines its graphical representation within the GUI.)
The `map` argument is currently not used internally and may be used on your own behalf (it can be accessed within the model for example).
`port` (ranging from 0 to 128) denotes the hardware address of the SignalBoard that is connected to the specific actuator,
whereas `pin` (ranging from 0 to 3) chooses from its 4 output channels.

In case the display configuration does not match the actual addresses of the SignalBoards, this inconsistency will be detected automatically causing the application to display an error message.

## Implementing own models
Currently there are two pre-defined models available that can be used as a starting point for own experiments.
The `NullModel` resets the SignalBoard's outputs to zero and is intended to act as a placeholder for e.g. adding graphical decorative elements to the scene.
The `FrequencyModel` is a minimal example model that scales a given frequency (and/or the amplitude) linearly with the current velocity of the display and also sets the RGB LED of ITCHy to a specific color that is given in the scene description.

New models have to be registered within the `main.cpp` as follows:
```
Scene scene(&tactileDisplay, currentPosition);

scene.registerModel("Frequency", Factory<FrequencyModel>());
scene.registerModel("Null", Factory<NullModel>());

scene.registerModel("YourModel", Factory<YourModel>());
```

#### C++ interface
New Models can be defined by implementing the `Model` interface.
Here is a minimum example, that just sets the frequency and amplitude of all actuators to a fixed value given in the scene description:
```
#include "model/model.h"

class ExampleModel : public Model
{
public:
    ExampleModel() {}
    virtual ~ExampleModel() {}
    virtual void initialize(const QStringList& options,PositionQuery* position, QRectF bounds)
    {
    	frequency = options[0].toFloat();
        amplitude = options[1].toFloat();
    }

    virtual void apply(const TactileDisplay* display, QVector<FrequencyTable>& tables)
    {
        for(FrequencyTable& tab : tables)
        {
            for(fixed_q5& frequency : tab.frequency)
                frequency = this->frequency;
            for(fixed_q15& amplitude : tab.amplitude)
                amplitude = this->amplitude;
        }
    }

private:
	float frequency = 0.0f;
    float amplitude = 0.0f;
};
```
The `initialize` method will be called once when the scene is loaded and the model instanciated. Here, the `options` list will hold all arguments that have been specified in the scene description file for the particular tactile area. `position` provides a basic interface to ITCHy (or a normal computer-mouse if ITCHy is not connected) that allows to access position information as well as buttons and the RGB LED. This pointer may be saved locally and can then be accessed within the `apply` method. The last argument `bounds` specifies the dimension of the tactile area in metres.

The `apply` method will be called regularly whenever the tactile mouse is positioned within the bounds of the tactile area. The `display` argument gives access to the current *transformed* status of the individual actuators, their actual *absolute* position in metres as well as their current velocity in m/s. In order to change the outputs of the SignalBoards, the values within the `tables` list can be manipulated directly. The indices of the FrequencyTables point to the actuators in the same order as has been specified in the original definition of the TactileDisplay object.

After this new model has been registered within the `main.cpp` by adding
```
scene.registerModel("Example", Factory<ExampleModel>());
```
a new instance of this model can be defined in a scene description file
```
0;0;5;5 | somegraphic.png;somegraphic.png | Example;250.0;0.9 | false
```
This will create a 5cm x 5cm sized tactile area in the center of the screen that causes the whole tactile display to vibrate with a frequency of 250Hz at a high amplitude when moving the tactile mouse over it.


#### Python interface
The Python interface is currently in development and will be exposed using a special pre-defined `PythonModel`. We will give additional examples in future releases.
