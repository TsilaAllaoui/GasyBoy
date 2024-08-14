#include "gasyboyCpuTestHelper.h"

static size_t instruction_mem_size;
static uint8_t *instruction_mem;

static int num_mem_accesses;
static struct mem_access mem_accesses[16];

std::shared_ptr<gasyboy::Mmu> GasyBoyCpuTestHelper::mmu = nullptr;
std::shared_ptr<gasyboy::Registers> GasyBoyCpuTestHelper::registers = nullptr;
std::shared_ptr<gasyboy::Cpu> GasyBoyCpuTestHelper::cpu = nullptr;
std::shared_ptr<gasyboy::InterruptManager> GasyBoyCpuTestHelper::im = nullptr;
std::shared_ptr<gasyboy::Gamepad> GasyBoyCpuTestHelper::gamepad = nullptr;

int GasyBoyCpuTestHelper::runTester(tester_flags *flags, tester_operations *myops)
{
    return tester_run(flags, myops);
}

void GasyBoyCpuTestHelper::resetCpu(size_t tester_instruction_mem_size,
                                    uint8_t *tester_instruction_mem)
{
    gamepad.reset(new gasyboy::Gamepad());
    mmu.reset(new gasyboy::Mmu(static_cast<uint8_t>(tester_instruction_mem_size), tester_instruction_mem, &num_mem_accesses, mem_accesses, *gamepad));
    registers.reset(new gasyboy::Registers(*mmu));
    im.reset(new gasyboy::InterruptManager(*mmu, *registers));
    cpu.reset(new gasyboy::Cpu(false, *mmu, *registers, *im));
}

void GasyBoyCpuTestHelper::initCpu(size_t tester_instruction_mem_size,
                                   uint8_t *tester_instruction_mem)
{
    instruction_mem_size = tester_instruction_mem_size;
    instruction_mem = tester_instruction_mem;

    resetCpu(instruction_mem_size, instruction_mem);
}

void GasyBoyCpuTestHelper::setCpuState(struct state *state)
{
    (void)state;

    num_mem_accesses = 0;

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

void GasyBoyCpuTestHelper::getCpuState(struct state *state)
{
    state->num_mem_accesses = num_mem_accesses;
    memcpy(state->mem_accesses, mem_accesses, sizeof(mem_accesses));

    state->PC = registers->PC;
    state->halted = registers->getHalted();
    state->SP = registers->SP;
    state->reg16.AF = registers->AF.get();
    state->reg16.BC = registers->BC.get();
    state->reg16.DE = registers->DE.get();
    state->reg16.HL = registers->HL.get();
    state->interrupts_master_enabled = im->isMasterInterruptEnabled();
}

int GasyBoyCpuTestHelper::cpuStep(void)
{
    int cycles = 0;

    cycles = cpu->step();

    return cycles;
}