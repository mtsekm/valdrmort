/*
* If not stated otherwise in this file or this component's LICENSE file the
* following copyright and licenses apply:
*
* Copyright 2024 Sky UK
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#ifndef LOGGER_H
#define LOGGER_H

#include <iostream>
#include <string>

/**
 * Defines the different levels of logging supported.
 */
enum class LogLevel {
    TRACE,  ///< More detailed information for debugging purposes.
    DEBUG,  ///< Detailed information for debugging purposes.
    INFO,   ///< General information about the application’s execution.
    WARN,   ///< Warning messages indicating potential issues.
    ERROR   ///< Error messages indicating serious issues.
};

/**
 * Contains configuration settings for the logger.
 */
namespace LoggerConfig {
    extern LogLevel currentLevel;

    void setLogLevel(LogLevel level);
} // namespace LoggerConfig

LogLevel stringToLogLevel(const std::string &levelStr);

void logMessage(LogLevel level, const std::string &message);

inline void logTrace(const std::string &message) { logMessage(LogLevel::TRACE, message); }
inline void logDebug(const std::string &message) { logMessage(LogLevel::DEBUG, message); }
inline void logInfo(const std::string &message) { logMessage(LogLevel::INFO, message); }
inline void logWarn(const std::string &message) { logMessage(LogLevel::WARN, message); }
inline void logError(const std::string &message) { logMessage(LogLevel::ERROR, message); }

#endif // LOGGER_H
