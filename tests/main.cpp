#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tester.h"

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

int main(int argc, char **argv)
{
    if (parse_args(argc, argv))
        return 1;

    return tester_run(&flags, &myops);
}
