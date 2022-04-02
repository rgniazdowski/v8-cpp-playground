#include <catch2/catch.hpp>
#include <util/Tag.hpp>
#include <util/Bindings.hpp>
#include <resource/ManagedObject.hpp>
//>---------------------------------------------------------------------------------------

class TestExternal;
using TagTestExternal = util::Tag<TestExternal>;
using TestExternalHandle = util::Handle<TagTestExternal>;

class TestExternal
{

}; //> TestExternal

class TestClass;
using TagTestClass = util::Tag<TestClass>;
using TestClassHandle = util::Handle<TagTestClass>;

class TestClass : public resource::ManagedObjectBase
{
    using handle_type = TestClassHandle;

private:
    std::string m_value;

public:
    float m_number;
    int m_id;

protected:
    TestClassHandle m_handle;

public:
    TestClass(uint64_t handle, int id, float number, std::string_view value)
        : m_handle(handle), m_id(id), m_number(number), m_value(value) {}

public:
    const std::string &getValue(void) const
    {
        return m_value;
    }
    void setValue(const std::string &value)
    {
        m_value = value;
    }

public:
    handle_type const &getHandle(void) const
    {
        return m_handle;
    }
    inline util::HandleBase const &getHandleBase(void) const override { return m_handle; }
}; //> TestClass
//>---------------------------------------------------------------------------------------

int TestGlobalFunction(int intValue, const std::string &stringValue)
{
    return intValue + 10 + (int)stringValue.length();
}
//>---------------------------------------------------------------------------------------

TEST_CASE("Create simple bindings", "[bindings]")
{
}
//!---------------------------------------------------------------------------------------

TEST_CASE("Invoke function bindings manually", "[bindings]")
{
    auto pBinding = new util::BindInfoFunction("TestGlobalFunction", &TestGlobalFunction, {"intValue", "stringValue"});
    util::WrappedArgs args = {util::WrappedValue::wrap(123), util::WrappedValue::wrap("Hello World!")};
    int expectedResult = 123 + 10 + (int)strlen("Hello World!");
    CHECK(util::BindingHelper::call<decltype(&TestGlobalFunction)>((void*)nullptr, pBinding, 123, "Hello World!") == expectedResult);
    CHECK(pBinding->call(123, "Hello World!") == expectedResult);
    CHECK(pBinding->callWithArgs(args) == expectedResult);

    auto wrappedResult = pBinding->callWrapped(args);
    CHECK(wrappedResult.get<int>() == expectedResult);
    wrappedResult = util::BindingHelper::callWrapped<void>(nullptr, pBinding, args);
    CHECK(wrappedResult.get<int>() == expectedResult);

    delete pBinding;
    util::reset_arguments(args);
}
//!---------------------------------------------------------------------------------------

TEST_CASE("Invoke member method bindings manually", "[bindings]")
{
}
//!---------------------------------------------------------------------------------------

TEST_CASE("Create complete metadata bindings", "[bindings]")
{
}
//!---------------------------------------------------------------------------------------

TEST_CASE("Invoke bindings from metadata", "[bindings]")
{
}
//!---------------------------------------------------------------------------------------