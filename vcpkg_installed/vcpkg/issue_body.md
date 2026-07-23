Package: egl-registry:x64-windows@2025-05-27

**Host Environment**

- Host: x64-windows
- Compiler: MSVC 19.51.36248.0
- CMake Version: 4.3.1-msvc1
-    vcpkg-tool version: 2026-04-08-e0612b42ce44e55a0e630f2ee9d3c533a63d8bc1
    vcpkg-readonly: true
    vcpkg-scripts version: cb2981c4e03d421fa03b9bb5044cd1986180e7e4

**To Reproduce**

`vcpkg install `

**Failure logs**

```
Downloading https://github.com/KhronosGroup/EGL-Registry/archive/3ae2b7c48690d2ce13cc6db3db02dfc0572be65e.tar.gz -> KhronosGroup-EGL-Registry-3ae2b7c48690d2ce13cc6db3db02dfc0572be65e.tar.gz
error: curl operation failed with error code 35 (SSL connect error).
error: Not a transient network error, won't retry download from https://github.com/KhronosGroup/EGL-Registry/archive/3ae2b7c48690d2ce13cc6db3db02dfc0572be65e.tar.gz
note: If you are using a proxy, please ensure your proxy settings are correct.
Possible causes are:
1. You are actually using an HTTP proxy, but setting HTTPS_PROXY variable to `https://address:port`.
This is not correct, because `https://` prefix claims the proxy is an HTTPS proxy, while your proxy (v2ray, shadowsocksr, etc...) is an HTTP proxy.
Try setting `http://address:port` to both HTTP_PROXY and HTTPS_PROXY instead.
2. If you are using Windows, vcpkg will automatically use your Windows IE Proxy Settings set by your proxy software. See: https://github.com/microsoft/vcpkg-tool/pull/77
The value set by your proxy might be wrong, or have same `https://` prefix issue.
3. Your proxy's remote server is out of service.
If you believe this is not a temporary download server failure and vcpkg needs to be changed to download this file from a different location, please submit an issue to https://github.com/Microsoft/vcpkg/issues
CMake Error at scripts/cmake/vcpkg_download_distfile.cmake:136 (message):
  Download failed, halting portfile.
Call Stack (most recent call first):
  scripts/cmake/vcpkg_from_github.cmake:120 (vcpkg_download_distfile)
  C:/Users/25108/AppData/Local/vcpkg/registries/git-trees/980ed62fded868a1cb66460b32704aca030af013/portfile.cmake:1 (vcpkg_from_github)
  scripts/ports.cmake:206 (include)



```

**Additional context**

<details><summary>vcpkg.json</summary>

```
{
  "name": "example",
  "version": "1.0",
  "dependencies": [
    "glfw3",
    "glm",
    "stb",
    {
      "name": "glad",
      "features": [
        "gl-api-latest"
      ]
    }
  ]
}

```
</details>
