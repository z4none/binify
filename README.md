# binify

`binify` is a small Windows utility for turning desktop executables into command-line entries.

Pick a Bin directory, add it to your current-user `PATH`, then right-click an `.exe` file and add it as a command. `binify` creates the entry with a symbolic link, hard link, or a simple CMD wrapper.

[中文说明](README.zh-CN.md)

## Screenshots

![binify settings](screenshot/main.png)

| Add command | Manage entries |
| --- | --- |
| ![Add command dialog](screenshot/add-command.png) | ![Entries tab](screenshot/entries.png) |

![Explorer context menu](screenshot/context.png)

## Features

- Add executable files to a user-managed Bin directory.
- Optionally add the Bin directory to the current user's `PATH`.
- Add an Explorer context-menu action for `.exe` files.
- Choose how entries are created:
  - Auto
  - Symbolic link
  - Hard link
  - CMD wrapper
- Detect duplicate command names and ask whether to overwrite or cancel.
- Normalize command names into safe Windows command entries.
- View, refresh, rename, delete, and open existing Bin entries.
- Read UI language packs from external JSON files.
- Install and uninstall without administrator privileges for the default current-user setup.

## Download

Download the latest installer from the GitHub Releases page.

Recommended installer:

- `binify-*-windows-x64.exe` for most modern Windows PCs.
- `binify-*-windows-x86.exe` for 32-bit Windows systems.

## Requirements

- Windows 10 or later is recommended.
- x64 and x86 builds are supported.
- Some link modes depend on Windows permissions:
  - Symbolic links may require Developer Mode or suitable privileges.
  - Hard links require the source and Bin directory to be on the same volume.
  - CMD wrapper works by generating a small `.bat` file and is the most compatible fallback.

## Basic usage

1. Open `binify`.
2. Choose a Bin directory, for example:

   ```text
   C:\Tools\Bin
   ```

3. Enable "Add Bin directory to current-user PATH" if you want commands to work from any terminal.
4. Enable the Explorer context menu if you want to add commands by right-clicking `.exe` files.
5. Save settings.
6. Right-click an `.exe` file and choose the binify action.
7. Confirm the command name and link mode.

After that, open a new terminal and run the command name you created.

Example:

```powershell
ffmpeg
```

If the current terminal was already open before changing `PATH`, restart the terminal.

## Manage entries

Open the `Entries` tab to:

- Refresh the Bin directory scan.
- Add a new executable.
- Delete an existing entry.
- Rename an entry.
- Open the Bin directory in File Explorer.

Deleting an entry removes only the generated Bin entry, not the original application.

## Build from source

Prerequisites:

- Visual Studio 2026 with C++ desktop tools.
- CMake 3.28 or later.
- Git.
- Inno Setup, only if you want to build the installer.

Build and test Debug:

```powershell
cmake --preset windows-x64-debug
cmake --build --preset windows-x64-debug
ctest --preset windows-x64-debug --output-on-failure
```

Build Release:

```powershell
cmake --preset windows-x64-release
cmake --build --preset windows-x64-release --target binify
```

Build the installer with Inno Setup. If `ISCC.exe` is on `PATH`, call it directly; if you use a portable copy, the project convention is `third_party\innosetup\bin\ISCC.exe`.

```powershell
.\third_party\innosetup\bin\ISCC.exe .\installer\binify.iss /DAppArch=x64
```

The installer is written to:

```text
out\installer
```

## Localization

Language files are JSON files under:

```text
resources\lang
```

Installed language packs are copied to:

```text
<install-dir>\lang
```

New languages can be added by providing another JSON language pack without recompiling the application.

## License

See [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md) for third-party notices.
