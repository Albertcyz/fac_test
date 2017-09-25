
/**
 *  @file zigbee_log.c
 *
 *  @brief Source file for the zigbee gateway logging module
 */

#include <stdio.h>
#include <stdarg.h>
#include "zigbee_log.h"
#include <time.h>

static zigbee_log_level_t zigbee_log_level = ZIGBEE_LOG_DEF_LEVEL; /**< Log level used to filter log messages */

void zigbee_log_set_level(zigbee_log_level_t level)
{
    switch (level) {
    case ZIGBEE_LOG_WARN: /**< Warning log level */
    case ZIGBEE_LOG_NOTICE: /**< Notice warning level */
    case ZIGBEE_LOG_INFO: /**< Informational warning level */
    case ZIGBEE_LOG_DEBUG: /**< Debug warning level */
        zigbee_log_level = level;
        break;
    default:
        zigbee_log_level = ZIGBEE_LOG_DEF_LEVEL;
    }
}

zigbee_log_level_t zigbee_log_get_level(void)
{
    return zigbee_log_level;
}

void zigbee_log_error(const char* msg, ...)
{
    va_list arg_list;

    va_start(arg_list, msg);
    if (ZIGBEE_LOG_ERROR <= zigbee_log_level) {
		time_t rawtime;
		struct tm * timeinfo;
		time(&rawtime);
		timeinfo = localtime(&rawtime);
        printf("Error  : ");
        vprintf(msg, arg_list);
		printf("\t%s", asctime(timeinfo));
    }
    va_end(arg_list);
}

void zigbee_log_warn(const char* msg, ...)
{
    va_list arg_list;

    va_start(arg_list, msg);
    if (ZIGBEE_LOG_WARN <= zigbee_log_level) {
		time_t rawtime;
		struct tm * timeinfo;
		time(&rawtime);
		timeinfo = localtime(&rawtime);
        printf("Warning: ");
        vprintf(msg, arg_list);
		printf("\t%s", asctime(timeinfo));
    }
    va_end(arg_list);
}

void zigbee_log_notice(const char* msg, ...)
{
    va_list arg_list;

    va_start(arg_list, msg);
    if (ZIGBEE_LOG_NOTICE <= zigbee_log_level) {
		time_t rawtime;
		struct tm * timeinfo;
		time(&rawtime);
		timeinfo = localtime(&rawtime);
        printf("Notice : ");
        vprintf(msg, arg_list);
		printf("\t%s", asctime(timeinfo));
		//printf("\n");
    }
    va_end(arg_list);
}

void zigbee_log_info(const char* msg, ...)
{
    va_list arg_list;

    va_start(arg_list, msg);
    if (ZIGBEE_LOG_INFO <= zigbee_log_level) {
		time_t rawtime;
		struct tm * timeinfo;
		time(&rawtime);
		timeinfo = localtime(&rawtime);
        printf("Info   : ");
        vprintf(msg, arg_list);
		printf("\t%s", asctime(timeinfo));
        //printf("\n");
    }
    va_end(arg_list);
}

void zigbee_log_debug(const char* msg, ...)
{
    va_list arg_list;

    va_start(arg_list, msg);
    if (ZIGBEE_LOG_DEBUG <= zigbee_log_level) {
		time_t rawtime;
		struct tm * timeinfo;
		time(&rawtime);
		timeinfo = localtime(&rawtime);
		printf("Debug  : ");
		vprintf(msg, arg_list);
		printf("\t%s", asctime(timeinfo));
        //printf("\n");
    }
    va_end(arg_list);
}
