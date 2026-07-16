#include "ui/ui_theme.h"

#include <algorithm>
#include <iterator>

namespace binify::ui {
namespace {

constexpr COLORREF kPrimary = RGB(0x25, 0x66, 0xE8);
constexpr COLORREF kPrimaryHover = RGB(0x1D, 0x4E, 0xC5);
constexpr COLORREF kDanger = RGB(0xC4, 0x2B, 0x2B);
constexpr COLORREF kDangerHover = RGB(0xA8, 0x22, 0x22);
constexpr COLORREF kSecondary = RGB(0xF4, 0xF6, 0xFA);
constexpr COLORREF kSecondaryHover = RGB(0xE8, 0xEC, 0xF4);
constexpr COLORREF kSecondaryBorder = RGB(0xC9, 0xD1, 0xDE);
constexpr COLORREF kText = RGB(0x1F, 0x29, 0x37);
constexpr COLORREF kTextInverse = RGB(0xFF, 0xFF, 0xFF);

UINT system_dpi() noexcept {
  using GetDpiForSystemFn = UINT(WINAPI*)();
  const HMODULE user32 = GetModuleHandleW(L"user32.dll");
  if (user32 != nullptr) {
    const auto get_dpi_for_system = reinterpret_cast<GetDpiForSystemFn>(GetProcAddress(user32, "GetDpiForSystem"));
    if (get_dpi_for_system != nullptr) {
      const UINT dpi = get_dpi_for_system();
      if (dpi != 0) {
        return dpi;
      }
    }
  }

  HDC dc = GetDC(nullptr);
  const int dpi = dc != nullptr ? GetDeviceCaps(dc, LOGPIXELSX) : 96;
  if (dc != nullptr) {
    ReleaseDC(nullptr, dc);
  }
  return dpi > 0 ? static_cast<UINT>(dpi) : 96U;
}

UINT dpi_for_window(HWND window) noexcept {
  using GetDpiForWindowFn = UINT(WINAPI*)(HWND);
  const HMODULE user32 = GetModuleHandleW(L"user32.dll");
  if (user32 != nullptr) {
    const auto get_dpi_for_window = reinterpret_cast<GetDpiForWindowFn>(GetProcAddress(user32, "GetDpiForWindow"));
    if (get_dpi_for_window != nullptr) {
      const UINT dpi = get_dpi_for_window(window);
      if (dpi != 0) {
        return dpi;
      }
    }
  }

  HDC dc = GetDC(window);
  const int dpi = dc != nullptr ? GetDeviceCaps(dc, LOGPIXELSX) : 96;
  if (dc != nullptr) {
    ReleaseDC(window, dc);
  }
  return dpi > 0 ? static_cast<UINT>(dpi) : 96U;
}

HFONT create_font(UINT dpi, int point_size, LONG weight) {
  const int height = -MulDiv(point_size, static_cast<int>(dpi), 72);
  return CreateFontW(
    height,
    0,
    0,
    0,
    weight,
    FALSE,
    FALSE,
    FALSE,
    DEFAULT_CHARSET,
    OUT_DEFAULT_PRECIS,
    CLIP_DEFAULT_PRECIS,
    CLEARTYPE_QUALITY,
    DEFAULT_PITCH | FF_DONTCARE,
    L"Segoe UI");
}

ButtonRole button_role(HWND button) noexcept {
  return static_cast<ButtonRole>(GetWindowLongPtrW(button, GWLP_USERDATA));
}

void fill_round_rect(HDC dc, RECT rect, int radius, COLORREF color) {
  HBRUSH brush = CreateSolidBrush(color);
  HBRUSH old_brush = static_cast<HBRUSH>(SelectObject(dc, brush));
  HPEN pen = CreatePen(PS_SOLID, 1, color);
  HPEN old_pen = static_cast<HPEN>(SelectObject(dc, pen));
  RoundRect(dc, rect.left, rect.top, rect.right, rect.bottom, radius, radius);
  SelectObject(dc, old_pen);
  SelectObject(dc, old_brush);
  DeleteObject(pen);
  DeleteObject(brush);
}

} // namespace

Theme::~Theme() {
  reset_fonts();
}

void Theme::initialize(HWND window) {
  dpi_ = dpi_for_window(window);

  reset_fonts();
  title_font_ = create_font(dpi_, 18, FW_SEMIBOLD);
  body_font_ = create_font(dpi_, 10, FW_NORMAL);
  small_font_ = create_font(dpi_, 9, FW_NORMAL);
}

int Theme::scale(int value) const noexcept {
  return MulDiv(value, static_cast<int>(dpi_), 96);
}

UINT Theme::dpi() const noexcept {
  return dpi_;
}

HFONT Theme::title_font() const noexcept {
  return title_font_;
}

HFONT Theme::body_font() const noexcept {
  return body_font_;
}

HFONT Theme::small_font() const noexcept {
  return small_font_;
}

void Theme::reset_fonts() {
  if (title_font_ != nullptr) {
    DeleteObject(title_font_);
    title_font_ = nullptr;
  }
  if (body_font_ != nullptr) {
    DeleteObject(body_font_);
    body_font_ = nullptr;
  }
  if (small_font_ != nullptr) {
    DeleteObject(small_font_);
    small_font_ = nullptr;
  }
}

void enable_process_dpi_awareness() noexcept {
  using SetProcessDpiAwarenessContextFn = BOOL(WINAPI*)(DPI_AWARENESS_CONTEXT);
  const HMODULE user32 = GetModuleHandleW(L"user32.dll");
  if (user32 != nullptr) {
    const auto set_process_dpi_awareness_context =
      reinterpret_cast<SetProcessDpiAwarenessContextFn>(GetProcAddress(user32, "SetProcessDpiAwarenessContext"));
    if (set_process_dpi_awareness_context != nullptr &&
        set_process_dpi_awareness_context(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2)) {
      return;
    }
  }

  SetProcessDPIAware();
}

void apply_font(HWND control, HFONT font) noexcept {
  SendMessageW(control, WM_SETFONT, reinterpret_cast<WPARAM>(font), TRUE);
}

int scale_for_system_dpi(int value) noexcept {
  return MulDiv(value, static_cast<int>(system_dpi()), 96);
}

SIZE scale_size_for_system_dpi(int width, int height) noexcept {
  return {scale_for_system_dpi(width), scale_for_system_dpi(height)};
}

void make_transparent_control(HWND control) noexcept {
  SetWindowLongPtrW(control, GWL_EXSTYLE, GetWindowLongPtrW(control, GWL_EXSTYLE) | WS_EX_TRANSPARENT);
}

void make_modern_button(HWND button, ButtonRole role) noexcept {
  SetWindowLongPtrW(button, GWLP_USERDATA, static_cast<LONG_PTR>(role));
  const auto style = GetWindowLongPtrW(button, GWL_STYLE);
  SetWindowLongPtrW(button, GWL_STYLE, (style | BS_OWNERDRAW) & ~BS_DEFPUSHBUTTON);
  SetWindowPos(button, nullptr, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
  InvalidateRect(button, nullptr, TRUE);
}

HBRUSH transparent_control_background(HDC dc) noexcept {
  SetBkMode(dc, TRANSPARENT);
  SetTextColor(dc, kText);
  return static_cast<HBRUSH>(GetStockObject(HOLLOW_BRUSH));
}

void draw_modern_button(const DRAWITEMSTRUCT& item) {
  const auto role = button_role(item.hwndItem);
  const bool pressed = (item.itemState & ODS_SELECTED) != 0;
  const bool focused = (item.itemState & ODS_FOCUS) != 0;

  COLORREF fill = kSecondary;
  COLORREF text_color = kText;
  COLORREF border = kSecondaryBorder;
  if (role == ButtonRole::primary) {
    fill = pressed ? kPrimaryHover : kPrimary;
    text_color = kTextInverse;
    border = fill;
  } else if (role == ButtonRole::danger) {
    fill = pressed ? kDangerHover : kDanger;
    text_color = kTextInverse;
    border = fill;
  } else if (pressed) {
    fill = kSecondaryHover;
  }

  RECT rect = item.rcItem;
  OffsetRect(&rect, 0, pressed ? 1 : 0);

  RECT shadow = rect;
  OffsetRect(&shadow, 0, 1);
  fill_round_rect(item.hDC, shadow, 12, RGB(0xD8, 0xDE, 0xE8));

  HBRUSH brush = CreateSolidBrush(fill);
  HBRUSH old_brush = static_cast<HBRUSH>(SelectObject(item.hDC, brush));
  HPEN pen = CreatePen(PS_SOLID, 1, border);
  HPEN old_pen = static_cast<HPEN>(SelectObject(item.hDC, pen));
  RoundRect(item.hDC, rect.left, rect.top, rect.right, rect.bottom - 1, 12, 12);

  wchar_t text[256]{};
  GetWindowTextW(item.hwndItem, text, static_cast<int>(std::size(text)));
  SetBkMode(item.hDC, TRANSPARENT);
  SetTextColor(item.hDC, text_color);
  DrawTextW(item.hDC, text, -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);

  if (focused) {
    RECT focus = rect;
    InflateRect(&focus, -4, -4);
    DrawFocusRect(item.hDC, &focus);
  }

  SelectObject(item.hDC, old_pen);
  SelectObject(item.hDC, old_brush);
  DeleteObject(pen);
  DeleteObject(brush);
}

void draw_window_background(HWND window, HDC dc, COLORREF background) {
  RECT rect{};
  GetClientRect(window, &rect);
  HBRUSH brush = CreateSolidBrush(background);
  FillRect(dc, &rect, brush);
  DeleteObject(brush);
}

void draw_panel(HDC dc, RECT rect, COLORREF fill, COLORREF border, int radius) {
  RECT shadow = rect;
  OffsetRect(&shadow, 0, 2);
  fill_round_rect(dc, shadow, radius, RGB(0xE3, 0xE8, 0xF0));

  HBRUSH brush = CreateSolidBrush(fill);
  HBRUSH old_brush = static_cast<HBRUSH>(SelectObject(dc, brush));
  HPEN pen = CreatePen(PS_SOLID, 1, border);
  HPEN old_pen = static_cast<HPEN>(SelectObject(dc, pen));
  RoundRect(dc, rect.left, rect.top, rect.right, rect.bottom, radius, radius);
  SelectObject(dc, old_pen);
  SelectObject(dc, old_brush);
  DeleteObject(pen);
  DeleteObject(brush);
}

} // namespace binify::ui
