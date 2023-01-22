# VulkanExamples
Examples for WIndows &amp; Wayland/X11 on Vulkan.

Install Vulkan on Ubuntu:
```
sudo add-apt-repository ppa:oibaf/graphics-drivers

sudo apt update

sudo apt install vulkan-sdk

sudo apt install vulkan-tools
```

Install Vukan on Windows:
On Windows 11, you can install the Vulkan development kit by following these steps:

    1) Download the latest version of the LunarG Vulkan SDK from the official website: https://vulkan.lunarg.com/sdk/home

    2) Run the installer and follow the on-screen instructions to install the SDK.

    3) Once the installation is finished, you will have the Vulkan headers and libraries ready to use in your C++ projects. The LunarG SDK will include the headers and libraries for both x86 and x64 architectures.

    4) The installer will also add the necessary environment variables and paths to your system, so that you can use the SDK and build your projects.

    5) You can also install the Vulkan SDK from the Microsoft Store, it's called the "Vulkan Runtime Libraries"

It's important to note that the above steps will install the latest stable version of the LunarG Vulkan SDK at the time of README.

You can check if the installation was successful by running the vulkaninfo.exe command, this command should display information about your system's Vulkan implementation if the installation was successful.

