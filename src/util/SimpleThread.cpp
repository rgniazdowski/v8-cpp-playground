#include <util/SimpleThread.hpp>
#include <BuildConfig.hpp>
#include <string>

#if defined(FG_USING_PLATFORM_WINDOWS)
#include <WindowsStandard.hpp>
#include <processthreadsapi.h>

void util::setThreadName(std::thread &thread, const char *threadName)
{
    std::string narrow(threadName);
    std::wstring wide(narrow.begin(), narrow.end());
    SetThreadDescription(static_cast<HANDLE>(thread.native_handle()), wide.c_str());
}

#elif defined(FG_USING_PLATFORM_LINUX)
#include <sys/prctl.h>
void util::setThreadName(const std::thread &thread, const char *threadName)
{
    prctl(PR_SET_NAME, threadName, 0, 0, 0);
}
#else
void util::setThreadName(const std::thread &thread, const char *threadName)
{
    auto handle = thread.native_handle();
    pthread_setname_np(handle, threadName);
}
#endif
