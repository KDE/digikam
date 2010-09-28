#ifndef CONFIG_LENSFUN_H
#define CONFIG_LENSFUN_H

#define CONF_PACKAGE "lensfun"

/* Define path to install lens database XML files */
#define CONF_DATADIR "${KDE4_DATA_INSTALL_DIR}/digikam/lensfun"

#ifdef Q_CC_MSVC
#define PLATFORM_WINDOWS 1
#elseifdef QCC_GNU
#define CONF_COMPILER_GCC 1
#endif

#endif // CONFIG_LENSFUN_H
