
// #define X
#ifdef X
#include "gtest/gtest.h"
#include <any>
#include <optional>

struct DataStack {
    template<typename T>
    void emplace(T&& t) {
        push_any(std::any(std::forward<T&&>(t)));
    }

    template<typename T>
    void push(T t) {
        push_any(std::any(t));
    }
    template<typename T>
    std::optional<T> pop() {
        try {
            return std::any_cast<T>(pop_any());
        }
        catch (const std::exception& ex) {
            printf("fail to cast %s", ex.what());
        }
        return {};
    }

    template<typename T>
    void set(T t) {
        set_any(std::any(t));
    }
    template<typename T>
    std::optional<T*> get(size_t i) {
        try {
            return std::any_cast<T>(&get_any(i));
        }
        catch (const std::exception& ex) {
            printf("fail to cast %s", ex.what());
        }
        return {};
    }

    virtual void push_any(std::any&& t) = 0;
    virtual std::any pop_any() = 0;
    virtual std::any& get_any(size_t i) = 0;
    virtual void set_any(std::any&& t, size_t i) = 0;
    virtual void clear() = 0;
    virtual size_t size() = 0;
};

struct DataStackImpl : public DataStack {
    
    std::vector<std::any> m_datas;
    void push_any(std::any&& t) {
        m_datas.emplace_back(t);
    };
    
    std::any pop_any(){
        std::any r;
        swap(m_datas.back(), r);
        m_datas.pop_back();
        return std::move(r);
    }

    std::any& get_any(size_t i) {
        return m_datas.at(i);
    }

    void set_any(std::any&& t, size_t i) {
        std::swap(m_datas.at(i), t);
    }

    void clear() {
        m_datas.clear();
    }

    size_t size() override{
        return m_datas.size();
    }
};

TEST(UTIL, t1) {
    {
        DataStackImpl d;
        d.push(100);
        auto k1 = d.get<int>(0);
        EXPECT_EQ(*k1.value(), 100);
        auto k = d.pop<int>();
        EXPECT_EQ(k.value(), 100);
    }

    {
        DataStackImpl d;
        std::string a = "abcde";
        d.push(a);
        auto k1 = d.get<std::string>(0);
        EXPECT_EQ(*k1.value(), "abcde");
        auto k = d.pop<std::string>();
        EXPECT_EQ(k.value(), "abcde");
    }

    {
        DataStackImpl d;
        struct S {
            int i = 1001;
            std::string s = "abcde";
            S& operator=(S& other) = delete;
            ~S() {
                printf("~SSSSSSSSSSSSSSSSSSSSSSSSS\n");
            }
        };
        S s;
        
        d.emplace(std::move(s));
        
        auto k1 = d.get<S>(0);
        EXPECT_EQ(k1.value()->i, 1001);
        EXPECT_EQ(k1.value()->s, "abcde");
        
        auto k = d.pop<S>();
        EXPECT_EQ(k.value().i, 1001);
        EXPECT_EQ(k.value().s, "abcde");
        EXPECT_EQ(s.s, "abcde");
    }
}
#endif