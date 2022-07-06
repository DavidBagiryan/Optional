#include <stdexcept>
#include <utility>

// Исключение этого типа должно генерироватся при обращении к пустому optional
class BadOptionalAccess : public std::exception {
public:
    using exception::exception;

    virtual const char* what() const noexcept override {
        return "Bad optional access";
    }
};

template <typename T>
class Optional {
public:
    Optional() = default;
    
    Optional(const T& value) {
        if (is_initialized_) {
            *optional_ = value;
        }
        else {
            optional_ = new (&data_[0]) T(value);
        }
        is_initialized_ = true;
    }
    
    Optional(T&& value) {
        if (is_initialized_) {
            *optional_ = std::move(value);
        }
        else {
            optional_ = new (&data_[0]) T(std::move(value));
        }
        is_initialized_ = true;
    }
    
    Optional(const Optional& other) {
        if (is_initialized_ && other.is_initialized_) {
            *optional_ = *other.optional_;
        }
        else if (!is_initialized_ && other.is_initialized_) {
            optional_ = new (&data_[0]) T(*other.optional_);
            is_initialized_ = true;
        }
        else if (is_initialized_ && !other.is_initialized_) {
            Reset();
        }
    }
    
    Optional(Optional&& other){
        if (is_initialized_ && other.is_initialized_) {
            *optional_ = std::move(*other.optional_);
        }
        else if (!is_initialized_ && other.is_initialized_) {
            optional_ = new (&data_[0]) T(std::move(*other.optional_));
            is_initialized_ = true;
        }
        else if (is_initialized_ && !other.is_initialized_) {
            Reset();
        }
    }

    Optional& operator=(const T& value) {
        if (is_initialized_) {
            *optional_ = value;
        }
        else {
            optional_ = new (&data_[0]) T(value);
        }
        is_initialized_ = true;
        return *this;
    }
    
    Optional& operator=(T&& rhs) {
        if (is_initialized_) {
            *optional_ = std::move(rhs);
        }
        else {
            optional_ = new (&data_[0]) T(std::move(rhs));
        }
        is_initialized_ = true;
        return *this;
    }
    
    Optional& operator=(const Optional& rhs) {
        if (is_initialized_ && rhs.is_initialized_) {
            *optional_ = *rhs.optional_;
        }
        else if (!is_initialized_ && rhs.is_initialized_) {
            optional_ = new (&data_[0]) T(*rhs.optional_);
            is_initialized_ = true;
        }
        else if (is_initialized_ && !rhs.is_initialized_) {
            Reset();
        }
        return *this;
    }
    
    Optional& operator=(Optional&& rhs) {
        if (is_initialized_ && rhs.is_initialized_) {
            *optional_ = std::move(*rhs.optional_);
        }
        else if (!is_initialized_ && rhs.is_initialized_) {
            optional_ = new (&data_[0]) T(std::move(*rhs.optional_));
            is_initialized_ = true;
        }
        else if (is_initialized_ && !rhs.is_initialized_) {
            Reset();
        }
        return *this;
    }

    ~Optional() {
        Reset();
    }

    bool HasValue() const {
        return is_initialized_;
    }

    // Операторы * и -> не должны делать никаких проверок на пустоту Optional.
    // Эти проверки остаются на совести программиста
    T& operator*() & {
        return *optional_;
    }
    const T& operator*() const& {
        return *optional_;
    }
    T&& operator*() && {
        return std::move(*optional_);
    }
    T* operator->() {
        return optional_;
    }
    const T* operator->() const {
        return optional_;
    }

    // Метод Value() генерирует исключение BadOptionalAccess, если Optional пуст
    T& Value() & {
        if (!is_initialized_) throw BadOptionalAccess();
        return *optional_;
    }
    const T& Value() const& {
        if (!is_initialized_) throw BadOptionalAccess();
        return *optional_;
    }
    T&& Value() && {
        if (!is_initialized_) throw BadOptionalAccess();
        return std::move(*optional_);
    }

    void Reset() {
        if (is_initialized_) {
            optional_->~T();
            is_initialized_ = false;
        }
    }
    
    template <typename... Types>
    T& Emplace(Types&&... values) {
        Reset();
        optional_ = new (&data_[0]) T(std::forward<Types>(values)...);
        is_initialized_ = true;
        return *optional_;
    }

private:
    // alignas нужен для правильного выравнивания блока памяти
    alignas(T) char data_[sizeof(T)];
    bool is_initialized_ = false;
    T* optional_ = nullptr;
};