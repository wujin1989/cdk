# Overview
CDK is a clean and concise cross-platform C development library that complements the standard C kits. It provides developers with powerful tools and functions to simplify the process of developing portable C applications. CDK is well-documented, designed to support best coding practices, and ensures code compiles without warnings on all supported platforms and compilers.

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
