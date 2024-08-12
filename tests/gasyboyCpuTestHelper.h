#include <string.h>
#include <memory>
#include "interruptManager.h"
#include "tester.h"
#include "mmu.h"
#include "cpu.h"

class GasyBoyCpuTestHelper
{
public:
    static std::shared_ptr<gasyboy::Mmu> mmu;
    static std::shared_ptr<gasyboy::Registers> registers;
    static std::shared_ptr<gasyboy::Cpu> cpu;
    static std::shared_ptr<gasyboy::InterruptManager> im;

    GasyBoyCpuTestHelper() = default;
    ~GasyBoyCpuTestHelper() = default;

    int runTester(tester_flags *flags, tester_operations *myops);

    static void resetCpu(size_t tester_instruction_mem_size,
                         uint8_t *tester_instruction_mem);

    /*
     * Called once during startup. The area of memory pointed to by
     * tester_instruction_mem will contain instructions the tester will inject, and
     * should be mapped read-only at addresses [0,tester_instruction_mem_size).
     */
    static void initCpu(size_t tester_instruction_mem_size,
                        uint8_t *tester_instruction_mem);

    /*
     * Resets the CPU state (e.g., registers) to a given state state.
     */
    static void setCpuState(struct state *state);

    /*
     * Query the current state of the CPU.
     */
    static void getCpuState(struct state *state);

    /*
     * Step a single instruction of the CPU. Returns the amount of cycles this took
     * (e.g., NOP should return 4).
     */
    static int cpuStep(void);
};