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

    GasyBoyCpuTestHelper()
    {
        // mmu = new gasyboy::Mmu(instruction_mem_size, instruction_mem, &num_mem_accesses, mem_accesses);
        // registers = new gasyboy::Registers(*mmu);
        // im = new gasyboy::InterruptManager(*mmu, *registers);
        // cpu = new gasyboy::Cpu(false, *mmu, *registers, *im);
    }

    ~GasyBoyCpuTestHelper()
    {
        // delete mmu;
        // delete registers;
        // delete cpu;
        // delete im;
    }

    int runTester(static tester_flags *flags, tester_operations *myops)
    {
        return tester_run(flags, myops);
    }

    static void resetCpu(size_t tester_instruction_mem_size,
                         uint8_t *tester_instruction_mem)
    {
        mmu.reset(new gasyboy::Mmu(tester_instruction_mem_size, tester_instruction_mem, &num_mem_accesses, mem_accesses));
        registers.reset(new gasyboy::Registers(*mmu));
        im.reset(new gasyboy::InterruptManager(*mmu, *registers));
        cpu.reset(new gasyboy::Cpu(false, *mmu, *registers, *im));
    }

    /*
     * Called once during startup. The area of memory pointed to by
     * tester_instruction_mem will contain instructions the tester will inject, and
     * should be mapped read-only at addresses [0,tester_instruction_mem_size).
     */
    static void initCpu(size_t tester_instruction_mem_size,
                        uint8_t *tester_instruction_mem)
    {
        instruction_mem_size = tester_instruction_mem_size;
        instruction_mem = tester_instruction_mem;

        /* ... Initialize your CPU here ... */
        resetCpu(instruction_mem_size, instruction_mem);
    }

    /*
     * Resets the CPU state (e.g., registers) to a given state state.
     */
    static void setCpuState(struct state *state)
    {
        (void)state;

        num_mem_accesses = 0;

        /* ... Load your CPU with state as described (e.g., registers) ... */
        registers->AF.set(state->reg16.AF);
        registers->BC.set(state->reg16.BC);
        registers->DE.set(state->reg16.DE);
        registers->HL.set(state->reg16.HL);
        registers->setHalted(state->halted);
        registers->PC = state->PC;
        registers->SP = state->SP;
        num_mem_accesses = state->num_mem_accesses;
        im->setMasterInterrupt(state->interrupts_master_enabled);

        int j = 0;
        for (auto &i : state->mem_accesses)
            mem_accesses[j] = i;
    }

    /*
     * Query the current state of the CPU.
     */
    static void getCpuState(struct state *state)
    {
        state->num_mem_accesses = num_mem_accesses;
        memcpy(state->mem_accesses, mem_accesses, sizeof(mem_accesses));

        /* ... Copy your current CPU state into the provided struct ... */
        state->PC = registers->PC;
        state->halted = registers->getHalted();
        state->SP = registers->SP;
        state->reg16.AF = registers->AF.get();
        state->reg16.BC = registers->BC.get();
        state->reg16.DE = registers->DE.get();
        state->reg16.HL = registers->HL.get();
        state->interrupts_master_enabled = im->isMasterInterruptEnabled();
    }

    /*
     * Step a single instruction of the CPU. Returns the amount of cycles this took
     * (e.g., NOP should return 4).
     */
    static int cpuStep(void)
    {
        int cycles = 0;

        /* ... Execute a single instruction in your CPU ... */

        cycles = cpu->step();

        return cycles;
    }
};