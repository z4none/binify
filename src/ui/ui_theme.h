#pragma once

#include <windows.h>

#include <string_view>

namespace binify::ui {

enum class ButtonRole {
  secondary,
  primary,
  danger
};

class Theme final {
public:
  Theme() = default;
  Theme(const Theme&) = delete;
  Theme& operator=(const Theme&) = delete;
  ~Theme();

  void initialize(HWND window);
  [[nodiscard]] int scale(int value) const noexcept;
  [[nodiscard]] UINT dpi() const noexcept;
  [[nodiscard]] HFONT title_font() const noexcept;
  [[nodiscard]] HFONT body_font() const noexcept;
  [[nodiscard]] HFONT small_font() const noexcept;

private:
  void reset_fonts();

  UINT dpi_ = 96;
  HFONT title_font_ = nullptr;
  HFONT body_font_ = nullptr;
  HFONT small_font_ = nullptr;
};

void enable_process_dpi_awareness() noexcept;
void apply_font(HWND control, HFONT font) noexcept;
void make_modern_button(HWND button, ButtonRole role) noexcept;
void draw_modern_button(const DRAWITEMSTRUCT& item);
void draw_window_background(HWND window, HDC dc, COLORREF background);
void draw_panel(HDC dc, RECT rect, COLORREF fill, COLORREF border, int radius);

} // namespace binify::ui
