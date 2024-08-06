#pragma once

#include <iostream>

using namespace std;

//Register class which represents the combination of two 8bits registers

class Register
{
    protected:
        //the 16bits value of the cmombine two 8bits register in the cpu
        uint16_t registerValue;

    public:
        //constructor && destructor
        Register();
        ~Register();

        //get the 16bits value of the two combined 8bits
        uint16_t get();

        //get the the lower nibble (ie : the rigth 8bit register)
        uint8_t getRigthRegister();

        //get the the higher nibble (ie : the left 8bit register)
        uint8_t getLeftRegister();

        //set the 16bits register value
        void set(uint16_t hex);

        //set the rigth 8bit register
        void setRigthRegister(uint8_t byte);

        //set the rigth 8bit register
        void setLeftRegister(uint8_t byte);
        
        //print the value of the 16bist register
        void show();
};


//the register with the flags (ie ; AF register)
class SpecialRegister : public Register
{
	public:
        //constructor && destructor
        SpecialRegister();
        ~SpecialRegister();

        //get/set the appropriate flag
        bool getFlag(char Reg);
        void setFlag(char Reg);
        void clearFlag(char Reg);
};
