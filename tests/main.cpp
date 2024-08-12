#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tester.h"

#include <string.h>
#include "interruptManager.h"
#include "tester.h"
#include "mmu.h"
#include "cpu.h"

extern struct tester_operations myops;

static struct tester_flags flags = {
    .keep_going_on_mismatch = 0,
    .enable_cb_instruction_testing = 1,
    .print_tested_instruction = 0,
    .print_verbose_inputs = 0,
};

static void print_usage(char *progname)
{
    printf("Usage: %s [option]...\n\n", progname);
    printf("Game Boy Instruction Tester.\n\n");
    printf("Options:\n");
    printf(" -k, --keep-going       Skip to the next instruction on a mismatch "
           "(instead of aborting all tests).\n");
    printf(" -c, --no-enable-cb     Disable testing of CB prefixed "
           "instructions.\n");
    printf(" -p, --print-inst       Print instruction undergoing tests.\n");
    printf(" -v, --print-input      Print every inputstate that is tested.\n");
    printf(" -h, --help             Show this help.\n");
}

static int parse_args(int argc, char **argv)
{
    for (int i = 1; i < argc; ++i)
    {
        if (strcmp(argv[i], "-k") == 0 || strcmp(argv[i], "--keep-going") == 0)
        {
            flags.keep_going_on_mismatch = 1;
        }
        else if (strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--no-enable-cb") == 0)
        {
            flags.enable_cb_instruction_testing = 0;
        }
        else if (strcmp(argv[i], "-p") == 0 || strcmp(argv[i], "--print-inst") == 0)
        {
            flags.print_tested_instruction = 1;
        }
        else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--print-input") == 0)
        {
            flags.print_verbose_inputs = 1;
        }
        else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0)
        {
            print_usage(argv[0]);
            exit(0);
        }
        else
        {
            print_usage(argv[0]);
            return 1;
        }
    }

    return 0;
}

static size_t instruction_mem_size;
static uint8_t *instruction_mem;

static int num_mem_accesses;
static struct mem_access mem_accesses[16];

class GasyBoyCpuTestHelper
{
public:
    static gasyboy::Mmu *mmu;
    static gasyboy::Registers *registers;
    static gasyboy::Cpu *cpu;
    static gasyboy::InterruptManager *im;

    GasyBoyCpuTestHelper()
    {
        mmu = new gasyboy::Mmu(instruction_mem_size, instruction_mem, &num_mem_accesses, mem_accesses);
        registers = new gasyboy::Registers(*mmu);
        im = new gasyboy::InterruptManager(*mmu, *registers);
        cpu = new gasyboy::Cpu(false, *mmu, *registers, *im);
    }

    ~GasyBoyCpuTestHelper()
    {
        delete mmu;
        delete registers;
        delete cpu;
        delete im;
    }

    int runTester(static tester_flags *flags, tester_operations *myops)
    {
        return tester_run(flags, myops);
    }

    static void resetCpu(size_t tester_instruction_mem_size,
                         uint8_t *tester_instruction_mem)
    {
        mmu = new gasyboy::Mmu(tester_instruction_mem_size, tester_instruction_mem, &num_mem_accesses, mem_accesses);
        registers = new gasyboy::Registers(*mmu);
        im = new gasyboy::InterruptManager(*mmu, *registers);
        cpu = new gasyboy::Cpu(false, *mmu, *registers, *im);
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

gasyboy::Mmu *GasyBoyCpuTestHelper::mmu = nullptr;
gasyboy::Registers *GasyBoyCpuTestHelper::registers = nullptr;
gasyboy::Cpu *GasyBoyCpuTestHelper::cpu = nullptr;
gasyboy::InterruptManager *GasyBoyCpuTestHelper::im = nullptr;

struct tester_operations myops = {
    .init = GasyBoyCpuTestHelper::initCpu,
    .set_state = GasyBoyCpuTestHelper::setCpuState,
    .get_state = GasyBoyCpuTestHelper::getCpuState,
    .step = GasyBoyCpuTestHelper::cpuStep,
};

int main(int argc, char **argv)
{
    if (parse_args(argc, argv))
        return 1;

    return GasyBoyCpuTestHelper().runTester(&flags, &myops);
}
