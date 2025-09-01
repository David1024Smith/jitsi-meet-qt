#ifndef LOGMACROS_H
#define LOGMACROS_H

#include "Logger.h"

// 日志宏定义
#define LOG_DEBUG(msg) Logger::instance()->debug(msg)
#define LOG_INFO(msg) Logger::instance()->info(msg)
#define LOG_WARNING(msg) Logger::instance()->warning(msg)
#define LOG_ERROR(msg) Logger::instance()->error(msg)
#define LOG_CRITICAL(msg) Logger::instance()->critical(msg)

#endif // LOGMACROS_H