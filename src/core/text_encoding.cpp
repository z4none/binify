#include "core/text_encoding.h"

#include "core/error.h"

namespace binify::core {

namespace {

constexpr char32_t kMaxCodePoint = 0x10FFFF;
constexpr char32_t kHighSurrogateStart = 0xD800;
constexpr char32_t kHighSurrogateEnd = 0xDBFF;
constexpr char32_t kLowSurrogateStart = 0xDC00;
constexpr char32_t kLowSurrogateEnd = 0xDFFF;

bool is_continuation(unsigned char byte) noexcept {
  return (byte & 0xC0U) == 0x80U;
}

Result<char32_t> decode_next_utf8(std::string_view input, std::size_t& index) {
  const auto first = static_cast<unsigned char>(input[index]);
  char32_t code_point = 0;
  std::size_t length = 0;

  if (first <= 0x7FU) {
    code_point = first;
    length = 1;
  } else if ((first & 0xE0U) == 0xC0U) {
    code_point = first & 0x1FU;
    length = 2;
  } else if ((first & 0xF0U) == 0xE0U) {
    code_point = first & 0x0FU;
    length = 3;
  } else if ((first & 0xF8U) == 0xF0U) {
    code_point = first & 0x07U;
    length = 4;
  } else {
    return make_error(ErrorCode::encoding_error, L"Input is not valid UTF-8.");
  }

  if (index + length > input.size()) {
    return make_error(ErrorCode::encoding_error, L"Input is truncated UTF-8.");
  }

  for (std::size_t offset = 1; offset < length; ++offset) {
    const auto byte = static_cast<unsigned char>(input[index + offset]);
    if (!is_continuation(byte)) {
      return make_error(ErrorCode::encoding_error, L"Input contains an invalid UTF-8 continuation byte.");
    }
    code_point = (code_point << 6U) | (byte & 0x3FU);
  }

  const bool overlong =
    (length == 2 && code_point < 0x80U) ||
    (length == 3 && code_point < 0x800U) ||
    (length == 4 && code_point < 0x10000U);
  const bool surrogate = code_point >= kHighSurrogateStart && code_point <= kLowSurrogateEnd;
  if (overlong || surrogate || code_point > kMaxCodePoint) {
    return make_error(ErrorCode::encoding_error, L"Input contains an invalid UTF-8 code point.");
  }

  index += length;
  return code_point;
}

void append_utf8(std::string& output, char32_t code_point) {
  if (code_point <= 0x7FU) {
    output.push_back(static_cast<char>(code_point));
  } else if (code_point <= 0x7FFU) {
    output.push_back(static_cast<char>(0xC0U | (code_point >> 6U)));
    output.push_back(static_cast<char>(0x80U | (code_point & 0x3FU)));
  } else if (code_point <= 0xFFFFU) {
    output.push_back(static_cast<char>(0xE0U | (code_point >> 12U)));
    output.push_back(static_cast<char>(0x80U | ((code_point >> 6U) & 0x3FU)));
    output.push_back(static_cast<char>(0x80U | (code_point & 0x3FU)));
  } else {
    output.push_back(static_cast<char>(0xF0U | (code_point >> 18U)));
    output.push_back(static_cast<char>(0x80U | ((code_point >> 12U) & 0x3FU)));
    output.push_back(static_cast<char>(0x80U | ((code_point >> 6U) & 0x3FU)));
    output.push_back(static_cast<char>(0x80U | (code_point & 0x3FU)));
  }
}

} // namespace

Result<std::wstring> utf8_to_wide(std::string_view input) {
  std::wstring output;
  output.reserve(input.size());

  std::size_t index = 0;
  while (index < input.size()) {
    auto decoded = decode_next_utf8(input, index);
    if (!decoded) {
      return decoded.error();
    }

    const char32_t code_point = decoded.value();
    if constexpr (sizeof(wchar_t) == 2) {
      if (code_point <= 0xFFFFU) {
        output.push_back(static_cast<wchar_t>(code_point));
      } else {
        const char32_t value = code_point - 0x10000U;
        output.push_back(static_cast<wchar_t>(0xD800U + (value >> 10U)));
        output.push_back(static_cast<wchar_t>(0xDC00U + (value & 0x3FFU)));
      }
    } else {
      output.push_back(static_cast<wchar_t>(code_point));
    }
  }

  return output;
}

Result<std::string> wide_to_utf8(std::wstring_view input) {
  std::string output;
  output.reserve(input.size());

  for (std::size_t index = 0; index < input.size(); ++index) {
    char32_t code_point = static_cast<char32_t>(input[index]);

    if constexpr (sizeof(wchar_t) == 2) {
      if (code_point >= kHighSurrogateStart && code_point <= kHighSurrogateEnd) {
        if (index + 1 >= input.size()) {
          return make_error(ErrorCode::encoding_error, L"Input contains an unpaired UTF-16 high surrogate.");
        }
        const char32_t low = static_cast<char32_t>(input[index + 1]);
        if (low < kLowSurrogateStart || low > kLowSurrogateEnd) {
          return make_error(ErrorCode::encoding_error, L"Input contains an invalid UTF-16 surrogate pair.");
        }
        code_point = 0x10000U + (((code_point - kHighSurrogateStart) << 10U) | (low - kLowSurrogateStart));
        ++index;
      } else if (code_point >= kLowSurrogateStart && code_point <= kLowSurrogateEnd) {
        return make_error(ErrorCode::encoding_error, L"Input contains an unpaired UTF-16 low surrogate.");
      }
    }

    append_utf8(output, code_point);
  }

  return output;
}

} // namespace binify::core
