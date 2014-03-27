/*****************************************************************************************
 * Configuration and hacks to stop the printf madness of the HSFC
 *****************************************************************************************/
#ifndef HSFC_CONFIG_H
#define HSFC_CONFIG_H

#ifdef REMOVE_PRINTF_MADNESS
#define printf( ... )
#endif


#endif /* HSFC_CONFIG_H */
