#include "mbed.h"
#include "Analog.h"
#include "base.h"

PwmOut MD1_IN(p26);
PwmOut MD2_IN(p25);
DigitalOut MD1_INH(p22, 0);
DigitalOut MD2_INH(p21, 0);


namespace Analog
{
    namespace  {
        float lastValue = 0;
    }

    float GetCurrentValue()
    {
        return lastValue;
    }

    void SetPWMFrequency()
    {
        MD1_IN.period_us(50);
        MD2_IN.period_us(50);
    }

    void EmergencyStop()
    {
        MD1_INH = 0;
        MD2_INH = 0;
        MD1_IN.write(0);
        MD1_IN.write(0);
    }

    void SetSteer(Analog_type type, float steer_rotation)
    {
        //Input comes in as -1.0 to 1.0
        steer_rotation = Base_BoundFloat(steer_rotation, -1.0f, 1.0f); 

        //Convert to between 0 and 1.0  
        steer_rotation = (steer_rotation + 1.0f) / 2.0f;
        

        switch (type) {

            case Analog_type::Type1:

                if(steer_rotation >= 0 && steer_rotation <= 1) {
                
                    MD1_INH = 1;
                    MD2_INH = 0;
                    MD1_IN.write(steer_rotation);
                }
                else {
                    
                    MD1_INH = 0;
                    MD2_INH = 0;
                }
                break;

            case Analog_type::Type2:

                if(steer_rotation >= 0 && steer_rotation < 0.5) {

                    MD1_INH = 0;
                    MD2_INH = 1;
                    MD2_IN.write(abs(steer_rotation - 0.5)*2);
                }
                else if(steer_rotation > 0.5 && steer_rotation <= 1) {

                    MD1_INH = 1;
                    MD2_INH = 0;
                    MD1_IN.write(abs(steer_rotation - 0.5)*2);
                    
                }
                else {

                    MD1_INH = 0;
                    MD2_INH = 0;

                }
                break;
        }
        
    }
}