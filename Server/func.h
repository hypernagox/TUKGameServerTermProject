#pragma once
#include "pch.h"

std::string WideToUtf8(std::wstring_view wstr)noexcept;

std::wstring Utf8ToWide(std::string_view utf8Str)noexcept;