#pragma once

namespace binify::ui::text {

inline constexpr wchar_t kAppTitle[] = L"binify";
inline constexpr wchar_t kSettingsTitle[] = L"binify settings";
inline constexpr wchar_t kAddCommandTitle[] = L"Add command with binify";
inline constexpr wchar_t kUninstallTitle[] = L"binify uninstall cleanup";

inline constexpr wchar_t kSave[] = L"Save";
inline constexpr wchar_t kCancel[] = L"Cancel";
inline constexpr wchar_t kCreate[] = L"Create";
inline constexpr wchar_t kBrowse[] = L"Browse...";
inline constexpr wchar_t kOpenBin[] = L"Open Bin directory";
inline constexpr wchar_t kPathToggle[] = L"Add Bin directory to current-user PATH";
inline constexpr wchar_t kContextMenuToggle[] = L"Enable Explorer context menu for .exe files";
inline constexpr wchar_t kLinkModeHelp[] =
  L"Auto tries Symbolic Link, then Hard Link, then CMD Wrapper. CMD Wrapper creates a simple .bat file.";

} // namespace binify::ui::text
