#pragma once
#include "StringUtils.h"

class ConsoleVariableParameter;

enum class ConsoleVariableFlag : uint32_t
{
    NONE = 0
};

class ConsoleVariableSystem
{
public:
    
    static ConsoleVariableSystem* get();

    virtual ConsoleVariableParameter* getVariable(StringUtils::StringHash hash) = 0;
    virtual int* getIntVariable(StringUtils::StringHash hash) = 0;
    virtual float* getFloatVariable(StringUtils::StringHash hash) = 0;
    virtual const char* getStringVariable(StringUtils::StringHash hash) = 0;
    virtual void setIntVariable(StringUtils::StringHash hash, int newValue) = 0;
    virtual void setFloatVariable(StringUtils::StringHash hash, float newValue) = 0;
    virtual void setStringVariable(StringUtils::StringHash hash, const char* newValue) = 0;
    virtual ConsoleVariableParameter* createIntVariable(int defaultValue, int currentValue, const char* name, const char* description) = 0;
    virtual ConsoleVariableParameter* createFloatVariable(float defaultValue, float currentValue, const char* name, const char* description) = 0;
    virtual ConsoleVariableParameter* createStringVariable(const char* defaultValue, const char* currentValue, const char* name, const char* description) = 0;
};
