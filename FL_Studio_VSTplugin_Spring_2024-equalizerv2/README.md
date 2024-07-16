
# EqualizerV2 - Work in Progress

## Introduction
This project is designed to provide users with a custom plugin for digital audio workstations (DAWs). The plugin can be built and installed on various operating systems using the recommended IDEs.

## Prerequisites
Before you begin, ensure you have met the following requirements:
- Windows, MacOS, or Linux operating system
- C++ installed
- Download and install one of the recommended IDEs based on your operating system:
  - **Windows**: [Visual Studio Code](https://code.visualstudio.com/)
  - **MacOS**: [Xcode](https://developer.apple.com/xcode/)
  - **Linux**: [CLion](https://www.jetbrains.com/clion/)

## Installation

### Downloading the Project
1-1. Clone the repository to your local machine using:
   ```bash
   git clone https://github.com/seet-lab/FL_Studio_VSTplugin_Spring_2024.git
   ```
1-2. If command line does not work, clone the repository using the GitHub Desktop:
   [GitHub Desktop](https://desktop.github.com)

### Download the Juce Framework
2. Download the Juce Framework from [Juce Download](https://juce.com/download).

### Building the Project
3. Open the project in the IDE you installed:
   - For Visual Studio Code, open the project folder.
   - For Xcode, open the `.xcodeproj` file.
   - For CLion, open the `.clion` file.
4. Build the project by following the build instructions specific to your IDE.

### Installing the Plugin
5. Locate the built `.vst3` file in your project’s output directory.
6. Move or copy the `.vst3` file to your local plugin folder:
   - **Windows**: Typically `C:\Program Files\Common Files\VST3`
   - **MacOS**: Typically `/Library/Audio/Plug-Ins/VST3`
7. Ensure your Digital Audio Workstation (DAW) is set up to scan this folder for plugins.

## Running the Plugin
8. Open your DAW.
9. Find and load the new plugin from the plugin list in your DAW.

## Support
For support, email us at [gongij01@pfw.edu](mailto:gongij01@pfw.edu).
