#ifndef ANALOG_H
#define ANALOG_H

namespace Analog {

    enum class Analog_type{
        Type1 = 1,
        Type2 = 0
    };

    void SetSteer(Analog_type type, float steer_rotation);
    void SetPWMFrequency();
    void EmergencyStop();
    float GetCurrentValue();

}

#endif // BASE_H