//
// Created by rokas on 3/2/26.
//

#ifndef DISK_ANALYSIS_CLI_HPP
#define DISK_ANALYSIS_CLI_HPP

#include <string>
#include <CLI/CLI.hpp>

#include "../../defs.hpp"

namespace da {
    struct CliConfig {
        ActionType action = ActionType::Unknown;

        std::string image_path;
        uint32_t target_partition = 0;

        std::string in_path; // For future use (e.g., input file for scripts)
        std::string out_path; // For future use (e.g., output file for scripts)
        std::string log_file; // For future use

        std::string lua_settings; // For future use (e.g., settings for LUA scripts)

        bool json_output = false;
        bool csv_output = false;
    };

    class CliParser {
    public:
        explicit CliParser(std::string app_description, std::string app_name)
            : app(std::move(app_description), std::move(app_name)) {
            setup();
        }

        bool parse(int argc, char** argv) {
            try {
                app.parse(argc, argv);
            } catch (const CLI::ParseError &e) {
                app.exit(e);
                return false;
            }
            return true;
        }

        const CliConfig& getConfig() const { return config; }

    private:
        CLI::App app;
        CliConfig config;

        void setup() {
            // commands
            auto* ls_cmd = app.add_subcommand("ls", "List partitions in the disk image");
            ls_cmd->callback([this]() { config.action = ActionType::ListPartitions; });

            auto* tree_cmd = app.add_subcommand("tree", "Display directory tree of the target partition");
            tree_cmd->add_option("partition", config.target_partition, "Target partition number")
                        ->required();
            tree_cmd->callback([this]() { config.action = ActionType::Tree; });

            auto* extract_cmd = app.add_subcommand("extract", "Extract a file from the target partition");
            extract_cmd->add_option("partition", config.target_partition, "Target partition number")
                        ->required();
            extract_cmd->add_option("in_path", config.in_path, "Path of the file to extract within the partition")
                        ->required();
            extract_cmd->add_option("out_path", config.out_path, "Path to save the extracted file")
                        ->required();
            extract_cmd->callback([this]() { config.action = ActionType::ExtractFile; });

            auto* script_cmd = app.add_subcommand("script", "Run a LUA script");
            script_cmd->add_option("in_path", config.in_path, "Path to the input file for the script")
                        ->required();
            script_cmd->add_option("settings", config.lua_settings = "", "Settings for the LUA script (optional)");
            script_cmd->callback([this]() { config.action = ActionType::RunScript; });

            // image
            app.add_option("image", config.image_path, "Path to disk image or physical device")
               ->required()
               ->check(CLI::ExistingFile);

            // output
            auto* output_group = app.add_option_group("Output Formats", "Select only one output format. Default is human-readable.");

            auto* json_flag = output_group->add_flag("-j,--json", config.json_output, "Output in JSON format");
            auto* csv_flag = output_group->add_flag("-c,--csv", config.csv_output, "Output in CSV format");

            json_flag->excludes(csv_flag);
            csv_flag->excludes(json_flag);

            // log file
            app.add_option("--log", config.log_file, "Path to log file (optional)");

            app.require_subcommand(1); // Require exactly one subcommand
        }
    };
};

#endif //DISK_ANALYSIS_CLI_HPP