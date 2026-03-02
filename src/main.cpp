#include "core/context.hpp"
#include "core/disk.hpp"
#include "core/volume.hpp"
#include "ui/cli/cli.hpp"
#include "ui/cli/wrappers.hpp"

int main(int argc, char** argv) {
    da::CliParser parser("DA - Disk Analysis & Forensic Tool", "da");

    if (!parser.parse(argc, argv)) return 1;
    da::Ctx::get().config = parser.getConfig(); // Load CLI parameters into global context

    const auto& cfg = da::Ctx::get().config;

    try {
        da::Ctx::get().disk = std::make_unique<da::Disk>(cfg.image_path);

        switch (cfg.action) {
            case da::ActionType::ListPartitions:
                da::cli::listPartitions();
                break;
            case da::ActionType::Tree:
                break;
            case da::ActionType::ExtractFile:
                break;
            case da::ActionType::RunScript:
                break;
            default:
                std::cerr << "Unknown action" << std::endl;
                return 1;
        }

    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    return 0;
}
