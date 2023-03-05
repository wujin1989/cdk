# Overview
CDK is a cross-platform C development library that complements the standard C library rather than replacing it. The library is designed to provide developers with a set of powerful tools and functions to simplify the process of developing portable C applications that can run seamlessly across multiple platforms.

One of the key features of CDK is its clean, concise, and highly-readable codebase. This makes it easy for developers to understand and work with the library, even if they are new to C programming. Additionally, CDK has been developed with a strong focus on ensuring that the code compiles without any warnings on all supported platforms and compilers, which helps to reduce the potential for bugs and other issues.

CDK includes a wide range of functionality, including support for networking, file I/O, and threading development, making it a powerful and versatile library that can be used for a variety of development tasks. Whether you're a seasoned C developer or just starting out, CDK provides a valuable resource that can help you streamline your development process and create robust, reliable software that runs seamlessly on any platform.

In summary, if you're looking for a cross-platform C development library that's easy to work with, well-documented, and designed to support best coding practices, then CDK may be just what you need. Give it a try and see what it can do for your development projects!

# Requirement
The compiler needs to support C11.

# Compile

Windows:

    cmake . -G "Ninja Multi-Config";
    or
    cmake . -G "Visual Studio 17 2022";

    cmake --build . --config Debug -j 8;
    cmake --build . --config Release -j 8;

Linux:

    cmake . -G "Ninja Multi-Config";
	
    cmake --build . --config Debug -j 8;
    cmake --build . --config Release -j 8;

Mac:

    cmake . -G "Ninja Multi-Config";
    or
    cmake . -G "Xcode";

    cmake --build . --config Debug -j 8;
    cmake --build . --config Release -j 8;

# Documentation
Reference docs/cdk-docs.html.


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
