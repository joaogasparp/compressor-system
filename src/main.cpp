#include "cli/cli.hpp"
#include <iostream>

int main(int argc, char* argv[]) {
    return compressor::cli::CliApplication::run(argc, argv);
}
