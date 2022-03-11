#ifndef SERVICE_H
#define SERVICE_H

#ifdef __cplusplus
#define SERVICE_EXTERN_C extern "C"
#else /* !__cplusplus */
#define SERVICE_EXTERN_C extern
#endif /* !__cplusplus */

SERVICE_EXTERN_C int service_start(const char* in_username, const char* in_password);
SERVICE_EXTERN_C int service_stop();

#endif /* SERVICE_H */
