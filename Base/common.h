#ifndef COMMON_H
#define COMMON_H

namespace Base {

class NonCopyable {
  public:
    explicit NonCopyable() = default;

    // Delete copy constructor
    NonCopyable(const NonCopyable&) = delete;
    // Delete move constructor
    NonCopyable(const NonCopyable&&) = delete;
    // Delete copy assignment operator
    NonCopyable& operator=(const NonCopyable&) = delete;
    // Delete move assignment operator
    NonCopyable& operator=(const NonCopyable&&) = delete;

    virtual ~NonCopyable() = default;
};

} // namespace Base

#endif // COMMON_H
