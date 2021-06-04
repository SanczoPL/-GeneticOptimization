#ifndef INCLUDE_SPDLOG_H
#define INCLUDE_SPDLOG_H

#include "spdlog/spdlog.h"
#include "spdlog/cfg/env.h"
#include "spdlog/sinks/stdout_color_sinks.h"


extern std::shared_ptr<spdlog::logger> Logger;
extern std::shared_ptr<spdlog::logger> ErrorLogger;

#endif //INCLUDE_SPDLOG_H
