#ifndef ZIGBEE_LOG_H
#define ZIGBEE_LOG_H

//	#define ZIGBEE_LOG_DEF_LEVEL ZIGBEE_LOG_ERROR /**< Default log level */
#define ZIGBEE_LOG_DEF_LEVEL ZIGBEE_LOG_DEBUG /**< Default log level */

/**
 *  @brief Log level
 */
typedef enum {
    ZIGBEE_LOG_ERROR = 0, /**< Error log level */
    ZIGBEE_LOG_WARN = 1, /**< Warning log level */
    ZIGBEE_LOG_NOTICE = 2, /**< Notice log level */
    ZIGBEE_LOG_INFO = 3, /**< Informational log level */
    ZIGBEE_LOG_DEBUG = 4 /**< Debug log level */
} zigbee_log_level_t;

/**
 *  @brief Set the log level
 *
 *  Messages with a severity below this level will be filtered.
 *  Error messages cannot be filtered.
 *
 *  @param[in] level The new log level
 */
void zigbee_log_set_level(zigbee_log_level_t level);

/**
 *  @brief Get the log level
 *
 *  @returns The current log level
 */
zigbee_log_level_t zigbee_log_get_level(void);

/**
 *  @brief Log an error message
 *
 *  @param[in] msg String containing format specifiers
 *  @param[in] ... arguments for the format specifiers
 */
void zigbee_log_error(const char* msg, ...);

/**
 *  @brief Log a warning message
 *
 *  @param[in] msg String containing format specifiers
 *  @param[in] ... arguments for the format specifiers
 */
void zigbee_log_warn(const char* msg, ...);

/**
 *  @brief Log an notice message
 *
 *  @param[in] msg String containing format specifiers
 *  @param[in] ... arguments for the format specifiers
 */
void zigbee_log_notice(const char* msg, ...);

/**
 *  @brief Log an info message
 *
 *  @param[in] msg String containing format specifiers
 *  @param[in] ... arguments for the format specifiers
 */
void zigbee_log_info(const char* msg, ...);

/**
 *  @brief Log a debug message
 *
 *  @param[in] msg String containing format specifiers
 *  @param[in] ... arguments for the format specifiers
 */
void zigbee_log_debug(const char* msg, ...);

#endif
