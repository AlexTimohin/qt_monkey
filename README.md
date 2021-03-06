# Qt Monkey [![Travis build status](https://travis-ci.org/Dushistov/qt_monkey.svg?branch=master)](https://travis-ci.org/Dushistov/qt_monkey) [![Appveyor build status](https://ci.appveyor.com/api/projects/status/c6h10uayiqmnr465?svg=true)](https://ci.appveyor.com/project/Dushistov/qt-monkey) [![License](https://img.shields.io/badge/license-BSD-green.svg)](https://github.com/Dushistov/qt_monkey/blob/master/LICENSE)

## Introduction

Qt Monkey is a tool to automate testing of Qt-based applications (widgets only).
It automates creation/modification and running of the tests.
Tests are written in Javascript (Qt supported dialect).

To write your tests, simply run your application (instrumented by Qt Monkey) and click
on widget, input text, etc. As a result, a script in Javascript will be generated
(see https://github.com/Dushistov/qt_monkey/blob/master/tests/test1.js for example).

[![example of script](https://github.com/Dushistov/qt_monkey/blob/master/docs/qtmonkey_gui.png)]

After that you can add suitable asserts and run this script via command line or GUI tool.

[![example of work of runnig script](https://github.com/Dushistov/qt_monkey/blob/master/docs/script_run_demo.gif)]

## Requirements

Qt Monkey requires compiler with C++11 support and Qt 4.x or Qt 5.x.

## How to use

First, download and build Qt Monkey:
```sh
git clone --recursive https://github.com/Dushistov/qt_monkey.git
cd qt_monkey
cmake -DQT_VARIANT="qt5" .
cmake --build .
```

Also you can use -DQT_VARIANT="qt4" for building with Qt 4.x.
Then, integrate Qt Monkey agent into your application:
1. Make sure that you link your application with qtmonkey_agent library
2. Create object of class `qt_monkey_agent::Agent` in the main thread of your application:
```C++
qt_monkey_agent::Agent agent;
```

That's all. Now you can run qtmonkey_gui application
and record or run your own scripts. See https://github.com/Dushistov/qt_monkey/blob/master/tests/test_app/main.cpp
for the more complex usage example.

## Internals

Qt Monkey consists of three parts: Qt Monkey's agent library (qtmonkey_agent),
qtmonkey_app (tool that run test application and cooperates with agent) and qtmonkey_gui.
qtmonkey_app and qtmonkey_gui use json as their preferred data marshalling language
and communicate via stdin/stdout streams. Therefore you can easily replace qtmonkey_gui
with a plugin for your favorite IDE.


