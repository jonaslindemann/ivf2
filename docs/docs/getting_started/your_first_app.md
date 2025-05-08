# Your first app

The easiest way to create an application using the Ivf2 framework is to use the available template application that is available on GitHub. This template contains all the necessary files and folders to get started with your application. You can find the template application [here](https://github.com/jonaslindemann/ivf2_app).

## Create a new application

First we clone the template application from GitHub. Open a terminal and run the following command:

```bash
git clone https://github.com/jonaslindemann/ivf2_app.git
```

This will create a new folder called `ivf2_app` in your current directory. You can rename this folder to whatever you like. For this example, we will keep the name `ivf2_app`.

## Configure and build the application (Windows)

To configure the application you need to run CMake with the following options:

```bash
cmake --preset windows
```

To configure a debug build, use the following command:

```bash
cmake --preset windows-debug
```

To build a release version of the application, use the following command:

```bash
cmake --build --preset release
``` 

To build a debug version of the application, use the following command:

```bash
cmake --build --preset debug
```

There is also a windows batch file that can be used to build the application. To use it, run the following command:

```bash
build_windows.bat [debug|release] [ivf2_path]
```

Where `ivf2_path` is the path to the Ivf2 framework. If you want to use the default path, you can omit this argument. The default path is `..\ivf2`. The `debug` or `release` argument specifies whether you want to build a debug or release version of the application. If you omit this argument, it will default to `release`. For example:
```

This will build the application in the specified mode (debug or release) and use the specified path to the Ivf2 framework. If no path is specified, it will use the default path which is `..\ivf2`.

## Configure and build the application (Linux)

To configure the application you need to run CMake with the following options:

```bash
cmake --preset linux
```
To configure a debug build, use the following command:

```bash
cmake --preset linux-debug
```
To build a release version of the application, use the following command:

```bash
cmake --build --preset release
```
To build a debug version of the application, use the following command:

```bash
cmake --build --preset debug
```

