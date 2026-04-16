# Layuh Build Instructions

## Automatic Build via GitHub Actions

This project is configured to automatically build to `.exe` whenever you push changes to the `main` branch or create a pull request.

### Build Status & Artifacts

1. **Push changes to GitHub:**
   ```bash
   git add .
   git commit -m "Your changes"
   git push origin main
   ```

2. **Check build progress:**
   - Go to your GitHub repository: https://github.com/Verteennn/Xenooooo
   - Click on the **Actions** tab
   - View the latest **Build Layuh to .exe** workflow run

3. **Download compiled EXE:**
   - After the build completes successfully, click on the workflow run
   - Scroll down to **Artifacts** section
   - Download:
     - `layuh-x64-exe` (64-bit version - recommended)
     - `layuh-Win32-exe` (32-bit version - legacy)

### Release Builds

To create an official release with EXE files attached:
```bash
git tag v1.0.0
git push origin v1.0.0
```

The workflow will automatically create a GitHub Release with the compiled `.exe` file.

## Project Information

- **Main Entry Point:** [layuh/intro.cpp](layuh/intro.cpp) - function `main()` at line 394
- **Project Type:** Win32 Console Application
- **Visual Studio Version:** 2022 (v17)
- **Toolset:** MSVC v143/v145
- **Target Platforms:** 
  - x64 (64-bit)
  - Win32 (32-bit)

## Solution File
- [totally not layuh.sln](totally not layuh.sln) - Main Visual Studio solution

## Local Build (requires Visual Studio 2022)

```powershell
msbuild "totally not layuh.sln" /p:Configuration=Release /p:Platform=x64
```

Output: `layuh/x64/Release/layuh.exe`

## Project Structure

```
layuh/
├── intro.cpp       (Main entry point)
├── intro.h
├── app.rc
├── drawing/        (ImGui graphics)
│   ├── imgui/      (ImGui library)
│   └── overlay/    (Overlay implementation)
├── features/       (Main features)
│   ├── combat/
│   ├── visuals/
│   ├── wallcheck/
│   └── misc/
├── protect/        (Obfuscation & protection)
├── util/           (Utilities & helpers)
└── packages.config (Dependencies)
```
