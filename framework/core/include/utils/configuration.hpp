#pragma once

#include <filesystem>
#include <iostream>
#include <toml.hpp>

/**
 * The utility class providing methods for working with the configuration files used in the framework. Note
 * that the configuration files are generated automatically via CMake.
 */
class Configuration {
    // ----------------------------------------------------------------------------
    // Variables
    // ----------------------------------------------------------------------------
protected:

    /** The application runtime directory. */
    std::filesystem::path runtime_directory;

    /** The configuration loaded from the configuration file. */
    toml::value configuration;

    // ----------------------------------------------------------------------------
    // Constructors
    // ----------------------------------------------------------------------------
public:

    /**
     * Constructs a new @link Configuration for the specified application directory.
     *
     * @param 	application_directory	The path to the application directory (often obtained from argv[0]).
     */
    Configuration(const std::string& application_directory) : runtime_directory(std::filesystem::path{application_directory}) {
        runtime_directory.remove_filename();
        std::filesystem::path path{runtime_directory / "configuration.toml"};
        path.make_preferred();

        if(std::filesystem::exists(path)) {
            configuration = toml::parse(path.generic_string());
        }else {
            std::cerr << "Failed to load configuration file from: " << path << std::endl;
        }
    }

    // ----------------------------------------------------------------------------
    // Methods
    // ----------------------------------------------------------------------------
public:
    /** Retrieves a path from the TOML variable specified via key, if no such variable is found in the configuration file, the specified fallback is returned instead. */
    std::filesystem::path get_path(const std::string& key, std::string fallback = "") {
        return toml::find_or<std::string>(configuration, key, fallback);
    }
};