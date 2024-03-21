#pragma once
#include "ServerCorePch.h"

template<typename T>
class ID_Ptr
{
public:
    static const uint64_t CombineIdPtr(const uint16_t id, const T* const ptr)noexcept { return (static_cast<const uint64_t>(id) << 48) | reinterpret_cast<const uint64_t>(ptr); }
public:
    ID_Ptr()noexcept = default;
    explicit ID_Ptr(const uint16_t id_, const T* const ptr_) noexcept :combine{ CombineIdPtr(id_,ptr_) } {}
public:
    const uint16_t GetID()const noexcept { return static_cast<const uint16_t>(combine >> 48); }
    T* const GetPtr()const noexcept { return reinterpret_cast<T* const>(combine & 0xFFFFFFFFFFFF); }
    operator T* () const noexcept { return GetPtr(); }
private:
    uint64_t combine = 0;
};