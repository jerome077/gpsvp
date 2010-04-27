/*
Copyright (c) 2009, Maurits van Dueren den Hollander
See EGM96Geoid.c for more info
*/

#ifndef EGM96GEOID_H
#define EGM96GEOID_H

/* This is for C++ and does no harm in C */
#ifdef __cplusplus
extern "C" {
#endif

void EGM96init(wchar_t *sGeoidMode);
void EGM96Geoid(double dLat, double dLon, double *dWsg84Alt, double *dGeoidAlt);

#ifdef __cplusplus
}
#endif


#endif // EGM96GEOID_H
