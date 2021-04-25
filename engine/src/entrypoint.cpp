#include <volk.h>

// https://stackoverflow.com/questions/5919996/how-to-detect-reliably-mac-os-x-ios-linux-windows-in-c-preprocessor
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
//define something for Windows (32-bit and 64-bit, this part is common)
#ifdef _WIN64
//define something for Windows (64-bit only)
#else
//define something for Windows (32-bit only)
#endif
#elif __APPLE__
#include <TargetConditionals.h>
#if TARGET_IPHONE_SIMULATOR
// iOS Simulator
#elif TARGET_OS_IPHONE
// iOS device
#elif TARGET_OS_MAC
// Other kinds of Mac OS
#else
#error "Unknown Apple platform"
#endif
#elif __linux__
// linux
#include "platform/unix_platform.h"

int main(int argc, char *argv[])
{
    engine::UnixPlatform platform(engine::UnixType::Linux, argc, argv);

#elif __unix__ // all unices not caught above
// Unix
#elif defined(_POSIX_VERSION)
// POSIX
#else
#error "Unknown compiler"
#endif

#ifndef DEBUG
try
{
#endif
    std::unique_ptr<engine::Application> app = std::make_unique<engine::Application>();

    if (platform.Initialize(std::move(app)))
    {
        platform.MainLoop();
        platform.Terminate(engine::ExitCode::Success);
    }
    else
    {
        platform.Terminate(engine::ExitCode::UnableToRun);
    }
#ifndef DEBUG
}
catch (const std::exception &e)
{
    std::cout << e.what();
    platform.Terminate(engine::ExitCode::FatalError);
}
#endif
}