# grwl - Graphics Windowing Library

Build the C++ API as a static library ONLY!

- This removes any cross platform limitations regarding public STL types and results in fewest limitations overall!
- We will also likely want to port the MacOS code to use as much swift as possible this will vastly increase maintainability.

There will be a shared library version of the API but it will be a C only API wrapper.

- This allows any number of languages to utilize the library without resulting to more difficult things.

- Only option to work around name mangling and STL issues

- Only option to have single Managed library compatible with all native platforms

- Does have a downside where if a replicated class structure is required on the managed side, it will need to be done manually

  

## API Plans

- Backends can be extended by the end user through a public implementing a C++ abstract class based interface
- There should be a concept of feature flags as well, through which multiple interfaces can be used to optionally implement features per platform
  - An example of this would be macos can't support moving a window over the api
  - This would allow the API to move forwards incrementally, and accept pull requests for features.
  - Feature flags could also be marked as unstable allowing iteration without locking in the API
    - Unstable features could output a compile time warning?
    - Also this could help us by having feature deprecation in the future
  - Feature flags should be able to be compiled out, and also disabled at runtime as well as runtime checks for features.
    - Compile Time may be required with certain platforms such as iOS/Android/MacOS because of API limitations on store applications
    - Runtime checks would be rather useful on the application side for enabling functionality across platforms without major code changes or specific platform checking logic
      - The process goes: Check for feature -> if supported implement application logic -> if not skip this functionality

## Potential Features

- Linux input handling should be reworked to use evdev directly for more than just joysticks
  - This will result in overall less code duplication between wayland/x11
  - Should allow support for touch screens which x11 is very much lacking
  - An alternative could be to use libinput

- Touch support
  - Should be possible with evdev on linux for both x11 and wayland otherwise x11 is.. not easy
- Wayland needs this implemented properly: https://github.com/glfw/glfw/pull/1273
- General cross platform display presentation time implementation:
  - mac: CVDisplayLInk: https://developer.apple.com/documentation/corevideo/cvdisplaylink-k0k
  - Windows: DXGI
  - General Vulkan: [VK_KHR_present_wait](https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VK_KHR_present_wait.html),  [`VK_EXT_present_timing`](https://github.com/KhronosGroup/Vulkan-Docs/pull/1364) in the future as well
  - Linux Wayland: wp_presentation: https://wayland.app/protocols/presentation-time
  - Linux X11: Can be done with xcb api's typically used under the hood for compositors X11 via DRI3 + Present extension exposed through some XCB functions
- ICC Profile handing - https://github.com/glfw/glfw/issues/1893 



