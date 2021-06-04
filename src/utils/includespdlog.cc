#include "utils/includespdlog.h"

std::shared_ptr<spdlog::logger> Logger{ spdlog::stdout_color_mt("console") };
std::shared_ptr<spdlog::logger> ErrorLogger{ spdlog::stderr_color_mt("stderr") };
