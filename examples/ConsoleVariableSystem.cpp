#include "ConsoleVariableSystem.h"

#include <string>
#include <unordered_map>

enum class ConsoleVariableType : char
{
    INT,
    FLOAT,
    STRING
};

class ConsoleVariableParameter
{
public:

    friend class ConsoleVariableSystemImplementation;

    uint32_t arrayIndex;
    ConsoleVariableType type;
    ConsoleVariableFlag flag;
    std::string name;
    std::string description;
};

template<typename T>
struct ConsoleVariableStorage
{
    T initial;
    T current;
    ConsoleVariableParameter* parameter;
};

template<typename T>
struct ConsoleVariableArray
{
    ConsoleVariableStorage<T>* variables;
    uint32_t lastVariable{ 0 };
    size_t arraySize{ 0 };

    ConsoleVariableArray(size_t size)
    : arraySize(size)
    {
        variables = new ConsoleVariableStorage<T>[size];
    }

    ~ConsoleVariableArray()
    {
        if (variables != nullptr)
        {
            delete variables;
        }
    }

    T* getCurrent(uint32_t index)
    {
        return &variables[index].current;
    }

    void setCurrent(uint32_t index, T newValue)
    {
        variables[index].current = newValue;
    }

    uint32_t pushBack(T newValue, ConsoleVariableParameter* parameter)
    {
        uint32_t index = lastVariable;

        if (index < arraySize)
        {
            variables[index].initial = newValue;
            variables[index].current = newValue;
            variables[index].parameter = parameter;

            parameter->arrayIndex = index;

            lastVariable++;
        }

        return index;
    }
};

class ConsoleVariableSystemImplementation : public ConsoleVariableSystem
{
public:

    constexpr static size_t MAX_INT_VARIABLES = 16;
    ConsoleVariableArray<int> intArray{ MAX_INT_VARIABLES };

    constexpr static size_t MAX_FLOAT_VARIABLES = 16;
    ConsoleVariableArray<float> floatArray{ MAX_FLOAT_VARIABLES };

    constexpr static size_t MAX_STRING_VARIABLES = 16;
    ConsoleVariableArray<std::string> stringArray{ MAX_STRING_VARIABLES };
    
    template<typename T>
    ConsoleVariableArray<T>* getVariableArray();

    template<typename T>
	T* getVariableCurrent(uint32_t namehash) {
		ConsoleVariableParameter* parameter = getVariable(namehash);
		if (!parameter)
        {
			return nullptr;
		}
		else
        {
			return getVariableArray<T>()->getCurrent(parameter->arrayIndex);
		}
	}

	template<typename T>
	void setVariableCurrent(uint32_t namehash, T value)
	{
		ConsoleVariableParameter* parameter = getVariable(namehash);
		if (parameter)
		{
			getVariableArray<T>()->setCurrent(parameter->arrayIndex, value);
		}
	}

    ConsoleVariableParameter* getVariable(StringUtils::StringHash hash) override final;
    int* getIntVariable(StringUtils::StringHash hash) override final;
    float* getFloatVariable(StringUtils::StringHash hash) override final;
    const char* getStringVariable(StringUtils::StringHash hash) override final;
    void setIntVariable(StringUtils::StringHash hash, int newValue) override final;
    void setFloatVariable(StringUtils::StringHash hash, float newValue) override final;
    void setStringVariable(StringUtils::StringHash hash, const char* newValue) override final;
    ConsoleVariableParameter* createIntVariable(int defaultValue, int currentValue, const char* name, const char* description) override final;
    ConsoleVariableParameter* createFloatVariable(float defaultValue, float currentValue, const char* name, const char* description) override final;
    ConsoleVariableParameter* createStringVariable(const char* defaultValue, const char* currentValue, const char* name, const char* description) override final;

private:

	ConsoleVariableParameter* initVariable(const char* name, const char* description);
	std::unordered_map<uint32_t, ConsoleVariableParameter> savedVariables;
};

template<>
ConsoleVariableArray<int>* ConsoleVariableSystemImplementation::getVariableArray()
{
    return &intArray;
}

template<>
ConsoleVariableArray<float>* ConsoleVariableSystemImplementation::getVariableArray()
{
    return &floatArray;
}

template<>
ConsoleVariableArray<std::string>* ConsoleVariableSystemImplementation::getVariableArray()
{
    return &stringArray;
}

ConsoleVariableSystem* ConsoleVariableSystem::get()
{
    static ConsoleVariableSystemImplementation consoleVariableSystem{};
    return &consoleVariableSystem;
}

ConsoleVariableParameter* ConsoleVariableSystemImplementation::initVariable(const char* name, const char* description)
{
    if (getVariable(name)) return nullptr;

    uint32_t namehash = StringUtils::StringHash{ name };
	savedVariables[namehash] = ConsoleVariableParameter{};

	ConsoleVariableParameter& newParameter = savedVariables[namehash];

	newParameter.name = name;
	newParameter.description = description;

	return &newParameter;
}

ConsoleVariableParameter* ConsoleVariableSystemImplementation::getVariable(StringUtils::StringHash hash)
{
    auto it = savedVariables.find(hash);

	if (it != savedVariables.end())
	{
		return &(*it).second;
	}

	return nullptr;
}

int* ConsoleVariableSystemImplementation::getIntVariable(StringUtils::StringHash hash)
{
    return getVariableCurrent<int>(hash);
}

float* ConsoleVariableSystemImplementation::getFloatVariable(StringUtils::StringHash hash)
{
    return getVariableCurrent<float>(hash);
}

const char* ConsoleVariableSystemImplementation::getStringVariable(StringUtils::StringHash hash)
{
    return getVariableCurrent<std::string>(hash)->c_str();
}

void ConsoleVariableSystemImplementation::setIntVariable(StringUtils::StringHash hash, int newValue)
{
    setVariableCurrent<int>(hash, newValue);
}

void ConsoleVariableSystemImplementation::setFloatVariable(StringUtils::StringHash hash, float newValue)
{
    setVariableCurrent<float>(hash, newValue);
}

void ConsoleVariableSystemImplementation::setStringVariable(StringUtils::StringHash hash, const char* newValue)
{
    setVariableCurrent<std::string>(hash, newValue);
}

ConsoleVariableParameter* ConsoleVariableSystemImplementation::createIntVariable(int defaultValue, int currentValue, const char* name, const char* description)
{
	ConsoleVariableParameter* newParameter = initVariable(name, description);
	if (!newParameter) return nullptr;

	newParameter->type = ConsoleVariableType::INT;
	
    getVariableArray<int>()->pushBack(currentValue, newParameter);

	return newParameter;
}

ConsoleVariableParameter* ConsoleVariableSystemImplementation::createFloatVariable(float defaultValue, float currentValue, const char* name, const char* description)
{
	ConsoleVariableParameter* newParameter = initVariable(name, description);
	if (!newParameter) return nullptr;

	newParameter->type = ConsoleVariableType::FLOAT;
	
    getVariableArray<float>()->pushBack(currentValue, newParameter);

	return newParameter;
}

ConsoleVariableParameter* ConsoleVariableSystemImplementation::createStringVariable(const char* defaultValue, const char* currentValue, const char* name, const char* description)
{
	ConsoleVariableParameter* newParameter = initVariable(name, description);
	if (!newParameter) return nullptr;

	newParameter->type = ConsoleVariableType::FLOAT;
	
    getVariableArray<std::string>()->pushBack(currentValue, newParameter);

	return newParameter;
}
