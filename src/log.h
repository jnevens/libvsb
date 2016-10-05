#ifndef SRC_LOGGER_H_
#define SRC_LOGGER_H_

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define LOG_ERROR(fmt, ...) fprintf(stderr, fmt "\n", ##__VA_ARGS__)
#define LOG_WARNING(fmt, ...) fprintf(stderr, fmt "\n", ##__VA_ARGS__)
#define LOG_DEBUG(fmt, ...) fprintf(stderr, fmt "\n", ##__VA_ARGS__)
#define LOG_SYSERROR(errcode) fprintf(stderr, "System error %d (%s)\n", errcode, strerror(errcode))

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* SRC_LOGGER_H_ */
