#include <sage3basic.h>
#include <rosePublicConfig.h>
#include <rose_config.h>                                // needed for VERSION with cmake
#include <initialize.h>

#ifdef ROSE_HAVE_GCRYPT_H
#include <gcrypt.h>
#endif
#include <Diagnostics.h>
#include <Sawyer/Synchronization.h>

namespace rose {

using namespace rose::Diagnostics;

#if SAWYER_MULTI_THREADED
static boost::once_flag initFlag = BOOST_ONCE_INIT;
#endif

// The following file variables are protected by mutex_
static SAWYER_THREAD_TRAITS::Mutex mutex_;
static bool isInitialized_ = false;

// Called by boost::call_once if multi-threading is supported, otherwise called directly.
class Initializer {
public:
    void operator()() {
        // Hold the lock for the entire duration of the initialization, not just the part where we update
        // isInitilialized_. This fullfills the documented contract that calling isInitialized() will block if a ROSE
        // initialization is in progress.
        SAWYER_THREAD_TRAITS::LockGuard lock(mutex_);
        if (isInitialized_)
            return;

        //--------------------------------
        // Initialize dependent libraries
        //--------------------------------

        Sawyer::initializeLibrary();

#ifdef ROSE_HAVE_GCRYPT_H
        gcry_check_version(NULL);
#endif

        //---------------------------
        // Initialize ROSE subsystems
        //---------------------------

        Diagnostics::initialize();

        isInitialized_ = true;
    }
};

void
initialize(const char *configToken) {
    Initializer init;

    // Initialize only once
#if SAWYER_MULTI_THREADED
    boost::call_once(initFlag, init);
#else
    if (!::rose::isInitialized())                       // qualified for sake of Microsoft
        init();
#endif

    // Check config token every time called
    if (configToken && *configToken && !checkConfigToken(configToken)) {
        Sawyer::Message::Stream fatal(mlog[FATAL]);
        fatal <<"mismatched headers and libraries\n"
              <<"    application reports  \"" <<StringUtility::cEscape(configToken) <<"\"\n"
              <<"    library reports      \"" <<StringUtility::cEscape(ROSE_CONFIG_TOKEN) <<"\"\n"
              <<"This error is usually caused by specifying inconsistent locations\n"
              <<"for the ROSE header files and ROSE library (or the headers and\n"
              <<"libraries that are ROSE dependencies) when compiling a program that\n"
              <<"uses ROSE.  Please check your compiler version, especially its \"-I\"\n"
              <<"and \"-L\" switches, to ensure it's compatible with how the ROSE\n"
              <<"library was built.  The rose-config command-line tool can give you\n"
              <<"this information if it has been installed.  If you are a ROSE\n"
              <<"developer, this error can happen if you updated your source tree\n"
              <<"but did not do a clean build.\n";
        throw std::runtime_error("ROSE configuration mismatch: caller said \"" +
                                 StringUtility::cEscape(configToken) + "\" but library has \"" +
                                 StringUtility::cEscape(ROSE_CONFIG_TOKEN) + "\"");
    }
}

bool
isInitialized() {
    SAWYER_THREAD_TRAITS::LockGuard lock(mutex_);
    return isInitialized_;
}

bool
checkConfigToken(const char *configToken) {
    return 0 == strcmp(configToken, ROSE_CONFIG_TOKEN);
}

bool
checkVersionNumber(const std::string &need) {
    std::vector<std::string> needParts = rose::StringUtility::split('.', need);
#if defined(ROSE_PACKAGE_VERSION)                       // autoconf
    std::vector<std::string> haveParts = rose::StringUtility::split('.', ROSE_PACKAGE_VERSION);
#elif defined(VERSION)                                  // cmake
    std::vector<std::string> haveParts = rose::StringUtility::split('.', VERSION);
#else
    #error "unknown ROSE version number"
#endif

    for (size_t i=0; i < needParts.size() && i < haveParts.size(); ++i) {
        if (needParts[i] != haveParts[i])
            return needParts[i] < haveParts[i];
    }

    // E.g., need = "1.2" and have = "1.2.x", or vice versa
    return true;
}

} // namespace
