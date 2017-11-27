#ifndef CONFIG_H
#define CONFIG_H

#include<array>
#include<model/tactiledisplay.h>
#include<model/actuator.h>
#include<QVector2D>

constexpr std::array<float, 2>
screenDimensions()
{
    return {
      0.52,     // Screen width in metres
      0.25      // Screen height in metres
    };
}

constexpr int
updateLimit()
{
    return 2000; // Limit update rate to 1000 Hz
}

inline TactileDisplay
tactileDisplayConfig()
{
    TactileDisplay display;

    display << Actuator(QVector2D(-4, 0), QVector2D(0.71, 12.5),  0, 1, 2)
            << Actuator(QVector2D(-2, 0), QVector2D(0.71, 12.5),  0, 1, 0)
            << Actuator(QVector2D( 0, 0), QVector2D(0.71, 12.5),  0, 2, 0)
            << Actuator(QVector2D( 2, 0), QVector2D(0.71, 12.5),  0, 1, 1)
            << Actuator(QVector2D( 4, 0), QVector2D(0.71, 12.5),  0, 1, 3);

    return display;
}

#endif // CONFIG_H
