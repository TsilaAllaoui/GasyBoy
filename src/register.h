#ifndef _REGISTER_H_
#define _REGISTER_H_

#include <memory>

using namespace std;

// Register class which represents the combination of two 8bits registers

namespace gasyboy
{
    class Register
    {
    protected:
        // the 16bits value of the cmombine two 8bits register in the cpu
        uint16_t _registerValue;

    public:
        // constructors && destructor
        Register();
        Register(const uint8_t &value);
        ~Register();

        // get the 16bits value of the two combined 8bits
        uint16_t get();

        // get the the lower nibble (ie : the rigth 8bit register)
        uint8_t getRightRegister();

        // get the the higher nibble (ie : the left 8bit register)
        uint8_t getLeftRegister();

        // set the 16bits register value
        void set(const uint16_t &hex);

        // set the rigth 8bit register
        void setRightRegister(const uint8_t &byte);

        // set the rigth 8bit register
        void setLeftRegister(const uint8_t &byte);

        // print the value of the 16bist register
        void show();

        // enum for the register name
        enum class RegisterName
        {
            A = 0,
            B,
            C,
            D,
            E,
            H,
            L,
            F,
            Z,
        };

        enum class RegisterPairName
        {
            AF = 9,
            BC,
            DE,
            HL,
            SP,
            PC
        };

        // enum for flags
        enum class FlagName
        {
            Z = 0x80,
            N = 0x40,
            H = 0x20,
            C = 0x10,
            NZ,
            NC,
        };
    };

    // the register with the flags (ie ; AF register)
    class SpecialRegister : public Register
    {
    public:
        // constructor && destructor
        SpecialRegister();
        SpecialRegister(const uint8_t &value);
        ~SpecialRegister();

        // get/set the appropriate flag
        bool getFlag(const FlagName &reg);
        void setFlag(const FlagName &reg);
        void clearFlag(const FlagName &reg);
    };
}

#endif