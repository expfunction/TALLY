/* Copyright (c) 2026 Burak Yazar */

#ifndef STEAM_H
#define STEAM_H

#ifdef PLATFORM_WIN64

#ifdef __cplusplus
extern "C"
{
#endif

    int steam_init(void);
    void steam_update(void);
    void steam_shutdown(void);

#ifdef __cplusplus
}
#endif

#else /* Non-Steam builds (e.g. DOS) */

#define steam_init() (1)
#define steam_update()
#define steam_shutdown()

#endif

#endif /* STEAM_H */
