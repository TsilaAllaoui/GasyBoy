#include "mmu.h"
#include <cstring>

Mmu::Mmu( std::string file )
{

    //allocating memory region
    VRAM = new uint8_t[0x2000];
    ExtRAM = new uint8_t[0x2000];
    WorkingRAM = new uint8_t[0x4000];
    
    //erasing memory at boot
    for( int i = 0; i < 0x2000; i++ )
        VRAM[i] = 0;
        
    for( int i = 0; i < 0x2000; i++ )
        ExtRAM[i] = 0x0;
        
    for( int i = 0; i < 0x4000; i++ )
        WorkingRAM[i] = 0x0;
        
    //setting joypad to off
    WorkingRAM[0xFFFF - 0xC000] = 0xFF;
    
    //to check if the gameboy is still in BIOS Bootrom mode
    in_bios = true;
    
    //the game cartridge
    cartridge = new Cartridge();
    
    //loading rom file via file
    cartridge->loadRom( file );
    
    //gamepad
    gamepad = new Gamepad();
    
    //to check if VRAM is modified
    currModifiedTile = -1;
    
    DMARegionWritten = false;
}

Mmu::~Mmu()
{
    delete[] VRAM, ExtRAM, WorkingRAM;
    in_bios = true;
}

void Mmu::disableBios()
{
    in_bios = false;
}

bool Mmu::isInBios()
{
    return in_bios;
}

uint8_t Mmu::read_ram( uint16_t adrr )
{
    if( adrr < 0x100 )
    {
        //if the gameboy is in internal BIOS, in should return BIOS content
        if( in_bios )
            return Bios[adrr];
            
        //return ROM content otherwise
        else return cartridge->RomBankRead( adrr );
    }
    
    //for reading ROM
    else if( adrr >= 0x100 && adrr < 0x8000 )
        return cartridge->RomBankRead( adrr );
        
    // fro reading VRAM
    else if( adrr >= 0x8000 && adrr < 0xA000 )
        return VRAM[adrr - 0x8000];
        
    //for reading ExtRAM
    else if( adrr >= 0xA000 && adrr < 0xC000 )
        return cartridge->RamBankRead( adrr );
        
    //for reading high RAM
    else if( adrr >= 0xC000 && adrr < 0xFF00 )
        return WorkingRAM[adrr - 0xC000];
        
    //for reading I/O region
    if( adrr >= 0xFF00 && adrr <= 0xFFFF )
    {
        if( adrr == 0xFF00 )
            return gamepad->getState();
        else return WorkingRAM[adrr - 0xC000];
    }
    
    //for finding unsupported read
    else exit( 78 );
}

void Mmu::write_ram( uint16_t adrr, uint8_t value )
{
    //if writing to ROM, manage memory banks
    if( adrr < 0x8000 )
    {
        if( cartridge->cartridgeType == 0 )
        {
            //MessageBox(0, "Illegal ROM Wrtite", "Illegal Action", MB_ICONEXCLAMATION | MB_OK);
            return;
        }
        
        cartridge->handleRomMemory( adrr, value );
    }
    
    //if writing to VRAM
    else if( adrr >= 0x8000 && adrr < 0xA000 )
    {
        //for notifying gpu that VRAM was modified && need t
        if( adrr <= 0x97FF )
        {
            int MSB;
            
            if( ( ( adrr & 0xF000 ) ) == 0x8000 )
                MSB = 0;
            else MSB = 1;
            
            int multiplier = ( ( adrr - 0x8000 ) & 0xF00 ) >> 8;
            int index = ( ( adrr - 0x8000 ) & 0xF0 ) >> 4;
            currModifiedTile = ( index + multiplier * 16 + 256 * MSB );
        }
        
        VRAM[adrr - 0x8000] = value;
    }
    
    //writing to ExtRAM
    else if( adrr >= 0xA000 && adrr < 0xC000 )
        cartridge->handleRamMemory( adrr, value );
        
    //writing to WorkingRAM && HighRAM
    else if( adrr >= 0xC000 && adrr <= 0xFFFF )
    {
        //writing to WorkingRAM
        if( adrr >= 0xE000 && adrr <= 0xFE00 )
        {
            WorkingRAM[adrr - 0xC000] = value;
            WorkingRAM[adrr - 0x2000 - 0xC000] = value;
        }
        
        //for unsupported write in WorkingRAM
        else if( adrr >= 0xFEA0 && adrr <= 0xFEFF )
            return;
            
        //I/O && HighRAM Region
        
        
        //Joypad register
        else if( adrr == 0xFF00 )
            gamepad->setState( value );
            
        // for Serail IN/OUT
        else if( adrr == 0xFF02 )       //only for Blargg Test roms debugging, TODO: implement serial transfert protocol
        {
            if( value == 0x81 )
                cout << WorkingRAM[0xFF01 - 0xC000];
        }
        
        //writing to DIV register reset its counter
        else if( adrr == 0xFF04 )
            WorkingRAM[adrr - 0xC000] = 0;
            
        //writing to LY register reset it
        else if( adrr == 0xFF44 )
            WorkingRAM[adrr - 0xC000] = 0;
            
        //OAM DMA Transfert
        else if( adrr == 0xFF46 )
        {
            DoDMATransfert( value );
            return;
        }
        
        //other write
        else WorkingRAM[adrr - 0xC000] = value;
    }
    
    //for unsupported writes
    else exit( 78 );
}

uint8_t Mmu::get_paletteColor( uint8_t index )
{
    uint8_t palette[4] = { 0 };
    palette[0] = read_ram( 0xFF47 ) & 0b00000011;
    palette[1] = ( read_ram( 0xFF47 ) & 0b0000110 ) >> 2;
    palette[2] = ( read_ram( 0xFF47 ) & 0b00110000 ) >> 4;
    palette[3] = ( read_ram( 0xFF47 ) & 0b11000000 ) >> 6;
    return palette[index];
}

void Mmu::directSet( uint16_t adrr, uint8_t value )
{
    WorkingRAM[adrr - 0xC000] = value;
}


void Mmu::DoDMATransfert( uint8_t value )
{
    uint16_t data = ( value << 8 );
    
    for( int i = 0; i < 0xA0; i++ )
    {
        uint8_t tmp = read_ram( data + i );
        directSet( 0xFE00 + i, tmp );
    }
    
    DMARegionWritten = true;
}

uint16_t Mmu::getNext2Bytes( uint16_t adress )
{
    return ( read_ram( adress + 2 ) << 8 ) | read_ram( adress + 1 );
}

uint8_t* Mmu::getVRAM()
{
    return VRAM;
}

Gamepad* Mmu::getGamepad()
{
    return gamepad;
}

bool Mmu::isVramWritten()
{
    return vramWritten;
}

void Mmu::setVramWriteStatus( bool value )
{
    vramWritten = value;
}

void Mmu::unsetDMAWritten()
{
    DMARegionWritten = false;
}
