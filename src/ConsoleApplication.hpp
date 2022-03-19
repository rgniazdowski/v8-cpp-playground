#pragma once
#ifndef FG_INC_CONSOLE_APPLICATION
#define FG_INC_CONSOLE_APPLICATION

#include <Application.hpp>

template <typename TEngineType>
class ConsoleApplication : public Application<TEngineType>
{
public:
    using base_type = Application<TEngineType>;
    using logger = typename base_type::logger;

    ConsoleApplication(int argc, char *argv[]) : base_type(argc, argv) {}
    virtual ~ConsoleApplication() {}

protected:
    bool preInitStep(void) override
    {
        return true;
    }
    bool postInitStep(void) override
    {
        return true;
    }

    void preLoopStep(void) override {}
    void postLoopStep(void) override {}

    void preQuitStep(void) override {}
    void postQuitStep(void) override {}
}; //# class ConsoleApplication<TEngineType>
//#---------------------------------------------------------------------------------------

#endif //> FG_INC_CONSOLE_APPLICATION
