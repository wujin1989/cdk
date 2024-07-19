![image](https://github.com/wujin1989/cdk/blob/main/docs/images/logo.png)
[![Platform](https://img.shields.io/badge/platform-linux%7Cmacosx%7Cwindows-ff69b4)](https://github.com/wujin1989/cdk/actions/workflows/main.yml)
[![Build](https://github.com/wujin1989/cdk/actions/workflows/main.yml/badge.svg)](https://github.com/wujin1989/cdk/actions/workflows/main.yml)
[![License](https://img.shields.io/badge/license-MIT-black)](LICENSE)
[![CodeQL](https://github.com/wujin1989/cdk/actions/workflows/codeql.yml/badge.svg)](https://github.com/wujin1989/cdk/actions/workflows/codeql.yml)
[![CodeFactor](https://www.codefactor.io/repository/github/wujin1989/cdk/badge)](https://www.codefactor.io/repository/github/wujin1989/cdk)
[![Snyk Security](https://github.com/wujin1989/cdk/actions/workflows/snyk-security.yml/badge.svg)](https://github.com/wujin1989/cdk/actions/workflows/snyk-security.yml)

# Overview
CDK is a clean and concise cross-platform C development kits that is a supplement and extension to the standard C library, rather than a replacement. It provides developers with powerful tools and functions to simplify the process of developing portable C applications. CDK is well-documented, designed to support best coding practices.

# Feature
- High-performance Asynchronous I/O (IOCP, epoll, kqueue)
- Cross-platform Compatibility
- Built-in TLS and DTLS Support
- Built-in TCP Packet Reassembler
- Receive side scaling Support

# Requirement
The compiler needs to support C11.


# Compile
    cmake -B build
    cmake --build build --config Release -j 8

# Documentation
Reference docs/API-Specification.html

If you'd just like to talk, come chat with us [on Discord](https://discord.gg/ty7XHjAwvg).


# License
MIT License

====
Copyright (c), Wu Jin <wujin.developer@gmail.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to
deal in the Software without restriction, including without limitation the
rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
IN THE SOFTWARE.

====
