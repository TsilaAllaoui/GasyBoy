#ifndef _CARTRIDGE_H_
#define _CARTRIDGE_H_

#include <iostream>
#include <fstream>
#include <cstring>
#include <vector>
#include <cmath>
#include <map>

namespace gasyboy
{
    class Cartridge
    {
    private:
        // ROM memory
        std::vector<std::vector<uint8_t>> _romBanks;

        // The current ROM bank
        uint8_t _currentRomBank;

        // RAM banks
        std::vector<std::vector<uint8_t>> _ramBanks;

        // The current RAM Bank
        uint8_t _currentRamBank;

        // For checking if RAM is writable
        bool _isRamEnabled;

        // RTC registers
        uint8_t RTCS, RTCM, RTCH, RTCDL, RTCDH;

        // Current used RTC register mapped in 0xA000-0xBFFFF
        uint8_t _currentRtcReg;

        // ROM/RAM Banking mode
        enum class BankingMode
        {
            MODE_0,
            MODE_1
        };

        // MBC mode
        BankingMode _bankingMode;

        // Number of banks of the ROM
        uint16_t _romBanksCount;

        // Number of banks of the ext RAM
        uint8_t _ramBanksCount;

    public:
        // Constructor/destructor
        Cartridge();
        ~Cartridge() = default;

        // MBC type
        enum class CartridgeType
        {
            ROM_ONLY = 0x00,
            MBC1 = 0x01,
            MBC1_RAM = 0x02,
            MBC1_RAM_BATT = 0x03,
            MBC2 = 0x05,
            MBC2_BATT = 0x06,
            RAM = 0x08,
            RAM_BATT = 0x09,
            MMM01 = 0x0B,
            MMM01_RAM = 0x0C,
            MMM01_RAM_BATT = 0x0D,
            MBC3_TIMER_BATT = 0x0F,
            MBC3_TIMER_RAM_BATT = 0x10,
            MBC3 = 0x11,
            MBC3_RAM = 0x12,
            MBC3_RAM_BATT = 0x13,
            MBC5 = 0x19,
            MBC5_RAM = 0x1A,
            MBC5_RAM_BATT = 0x1B,
            MBC5_RUMBLE = 0x1C,
            MBC5_RUMBLE_RAM = 0x1D,
            MBC5_RUMBLE_RAM_BATT = 0x1E,
            POCKET_CAMERA = 0xFC,
            BANDAI_TAMA5 = 0xFD,
            HuC3 = 0xFE,
            HuC1_RAM_BATT = 0xFF
        };

        struct CartridgeHeader
        {
            std::string name;
            std::string manufacturer;
            std::string cgbSupport;
            std::string licenseeCode;
            bool sgbSupport;
            std::string cartridgeType;
            std::string romSize;
            std::string ramSize;
            bool isJapaneseCartridge;
            uint8_t maskRomVersion;
        };

        std::map<uint8_t, std::string> oldLicenseeCodes = {
            {0x00, "None"},
            {0x01, "Nintendo"},
            {0x08, "Capcom"},
            {0x09, "HOT-B"},
            {0x0A, "Jaleco"},
            {0x0B, "Coconuts Japan"},
            {0x0C, "Elite Systems"},
            {0x13, "EA (Electronic Arts)"},
            {0x18, "Hudson Soft"},
            {0x19, "ITC Entertainment"},
            {0x1A, "Yanoman"},
            {0x1D, "Japan Clary"},
            {0x1F, "Virgin Games Ltd."},
            {0x24, "PCM Complete"},
            {0x25, "San-X"},
            {0x28, "Kemco"},
            {0x29, "SETA Corporation"},
            {0x30, "Infogrames"},
            {0x31, "Nintendo"},
            {0x32, "Bandai"},
            {0x34, "Konami"},
            {0x35, "HectorSoft"},
            {0x38, "Capcom"},
            {0x39, "Banpresto"},
            {0x3C, ".Entertainment i"},
            {0x3E, "Gremlin"},
            {0x41, "Ubi Soft"},
            {0x42, "Atlus"},
            {0x44, "Malibu Interactive"},
            {0x46, "Angel"},
            {0x47, "Spectrum Holoby"},
            {0x49, "Irem"},
            {0x4A, "Virgin Games Ltd."},
            {0x4D, "Malibu Interactive"},
            {0x4F, "U.S. Gold"},
            {0x50, "Absolute"},
            {0x51, "Acclaim Entertainment"},
            {0x52, "Activision"},
            {0x53, "Sammy USA Corporation"},
            {0x54, "GameTek"},
            {0x55, "Park Place"},
            {0x56, "LJN"},
            {0x57, "Matchbox"},
            {0x59, "Milton Bradley Company"},
            {0x5A, "Mindscape"},
            {0x5B, "Romstar"},
            {0x5C, "Naxat Soft"},
            {0x5D, "Tradewest"},
            {0x60, "Titus Interactive"},
            {0x61, "Virgin Games Ltd."},
            {0x67, "Ocean Software"},
            {0x69, " EA (Electronic Arts)"},
            {0x6E, "Elite Systems"},
            {0x6F, "Electro Brain"},
            {0x70, "Infogrames5"},
            {0x71, "Interplay Entertainment"},
            {0x72, "Broderbund"},
            {0x73, "Sculptured Software6"},
            {0x75, "The Sales Curve Limited7"},
            {0x78, "THQ"},
            {0x79, "Accolade"},
            {0x7A, "Triffix Entertainment"},
            {0x7C, "Microprose"},
            {0x7F, "Kemco"},
            {0x80, "Misawa Entertainment"},
            {0x83, "Lozc"},
            {0x86, "Tokuma Shoten"},
            {0x8B, "Bullet-Proof Software2"},
            {0x8C, "Vic Tokai"},
            {0x8E, "Ape"},
            {0x8F, "I’Max"},
            {0x91, "Chunsoft Co.8"},
            {0x92, "Video System"},
            {0x93, "Tsubaraya Productions"},
            {0x95, "Varie"},
            {0x96, "Yonezawa/S’Pal"},
            {0x97, "Kemco"},
            {0x99, "Arc"},
            {0x9A, "Nihon Bussan"},
            {0x9B, "Tecmo"},
            {0x9C, "Imagineer"},
            {0x9D, "Banpresto"},
            {0x9F, "Nova"},
            {0xA1, "Hori Electric"},
            {0xA2, "Bandai"},
            {0xA4, "Konami"},
            {0xA6, "Kawada"},
            {0xA7, "Takara"},
            {0xA9, "Technos Japan"},
            {0xAA, "Broderbund"},
            {0xAC, "Toei Animation"},
            {0xAD, "Toho"},
            {0xAF, "Namco"},
            {0xB0, "Acclaim Entertainment"},
            {0xB1, "ASCII Corporation or Nexsoft"},
            {0xB2, "Bandai"},
            {0xB4, "Square Enix"},
            {0xB6, "HAL Laboratory"},
            {0xB7, "SNK"},
            {0xB9, "Pony Canyon"},
            {0xBA, "Culture Brain"},
            {0xBB, "Sunsoft"},
            {0xBD, "Sony Imagesoft"},
            {0xBF, "Sammy Corporation"},
            {0xC0, "Taito"},
            {0xC2, "Kemco"},
            {0xC3, "Square"},
            {0xC4, "Tokuma Shoten"},
            {0xC5, "Data East"},
            {0xC6, "Tonkinhouse"},
            {0xC8, "Koei"},
            {0xC9, "UFL"},
            {0xCA, "Ultra"},
            {0xCB, "Vap"},
            {0xCC, "Use Corporation"},
            {0xCD, "Meldac"},
            {0xCE, "Pony Canyon"},
            {0xCF, "Angel"},
            {0xD0, "Taito"},
            {0xD1, "Sofel"},
            {0xD2, "Quest"},
            {0xD3, "Sigma Enterprises"},
            {0xD4, "ASK Kodansha Co."},
            {0xD6, "Naxat Soft13"},
            {0xD7, "Copya System"},
            {0xD9, "Banpresto"},
            {0xDA, "Tomy"},
            {0xDB, "LJN"},
            {0xDD, "NCS"},
            {0xDE, "Human"},
            {0xDF, "Altron"},
            {0xE0, "Jaleco"},
            {0xE1, "Towa Chiki"},
            {0xE2, "Yutaka"},
            {0xE3, "Varie"},
            {0xE5, "Epcoh"},
            {0xE7, "Athena"},
            {0xE8, "Asmik Ace Entertainment"},
            {0xE9, "Natsume"},
            {0xEA, "King Records"},
            {0xEB, "Atlus"},
            {0xEC, "Epic/Sony Records"},
            {0xEE, "IGS"},
            {0xF0, "A Wave"},
            {0xF3, "Extreme Entertainment"},
            {0xFF, "LJN"},
        };

        std::map<std::string, std::string> newLicenseeCodes = {
            {"00", "None"},
            {"01", "Nintendo Research & Development 1"},
            {"08", "Capcom"},
            {"13", "EA (Electronic Arts)"},
            {"18", "Hudson Soft"},
            {"19", "B-AI"},
            {"20", "KSS"},
            {"22", "Planning Office WADA"},
            {"24", "PCM Complete"},
            {"25", "San-X"},
            {"28", "Kemco"},
            {"29", "SETA Corporation"},
            {"30", "Viacom"},
            {"31", "Nintendo"},
            {"32", "Bandai"},
            {"33", "Ocean Software/Acclaim Entertainment"},
            {"34", "Konami"},
            {"35", "HectorSoft"},
            {"37", "Taito"},
            {"38", "Hudson Soft"},
            {"39", "Banpresto"},
            {"41", "Ubi Soft"},
            {"42", "Atlus"},
            {"44", "Malibu Interactive"},
            {"46", "Angel"},
            {"47", "Bullet-Proof Software"},
            {"49", "Irem"},
            {"50", "Absolute"},
            {"51", "Acclaim Entertainment"},
            {"52", "Activision"},
            {"53", "Sammy USA Corporation"},
            {"54", "Konami"},
            {"55", "Hi Tech Expressions"},
            {"56", "LJN"},
            {"57", "Matchbox"},
            {"58", "Mattel"},
            {"59", "Milton Bradley Company"},
            {"60", "Titus Interactive"},
            {"61", "Virgin Games Ltd."},
            {"64", "Lucasfilm Games"},
            {"67", "Ocean Software"},
            {"69", "EA (Electronic Arts)"},
            {"70", "Infogrames"},
            {"71", "Interplay Entertainment"},
            {"72", "Broderbund"},
            {"73", "Sculptured Software"},
            {"75", "The Sales Curve Limited"},
            {"78", "THQ"},
            {"79", "Accolade"},
            {"80", "Misawa Entertainment"},
            {"83", "lozc"},
            {"86", "Tokuma Shoten"},
            {"87", "Tsukuda Original"},
            {"91", "Chunsoft Co."},
            {"92", "Video System"},
            {"93", "Ocean Software/Acclaim Entertainment"},
            {"95", "Varie"},
            {"96", "Yonezawa/s’pal"},
            {"97", "Kaneko"},
            {"99", "Pack-In-Video"},
            {"9H", "Bottom Up"},
            {"A4", "Konami (Yu-Gi-Oh!)"},
            {"BL", "MTO"},
            {"DK", "Kodansha"}};

        // Cartridge header informations
        CartridgeHeader _cartridgeHeader;
        // Cartridge type
        CartridgeType _cartridgeType;

        // Loading ROM
        void loadRom(const std::string &filename);

        // For debugging
        void loadRom(uint8_t size, uint8_t *mem);

        // Set MBC type
        void setMBCType(const uint8_t &value);

        // Get/Set rom banks number
        void setRomBankNumber(const uint8_t &value);
        uint16_t getRomBanksNumber();

        // Get/Set rom banks number
        void setRamBankNumber(const uint8_t &value);
        uint8_t getRamBanksNumber();

        // ROM Bank reading
        uint8_t romBankRead(const uint16_t &adrr);

        // RAM Bank reading
        uint8_t ramBankRead(const uint16_t &adrr);

        // To handle all bank changes
        void mbcRomWrite(const uint16_t &adrr, const uint8_t &value);
        void mbcRamWrite(const uint16_t &adrr, const uint8_t &value);

        // To load rom from byte array
        void loadRomFromByteArray(const std::vector<uint8_t> &byteArray);

        // Get rom content
        std::vector<std::vector<uint8_t>> getRomBanks();

        // Get ram content
        std::vector<std::vector<uint8_t>> getRamBanks();

        // Get cartridge header infos
        void getCartridgeHeaderInfos();

        // Log cartridge header infos
        void logCartridgeHeaderInfos();

        // Get cartridge type as string from byte
        std::string cartridgeTypeStr(const uint8_t &byte);

        // Rom size string from byte
        std::string romSizeStr(const uint8_t &byte);

        // Ram size string from byte
        std::string ramSizeStr(const uint8_t &byte);
    };
}

#endif