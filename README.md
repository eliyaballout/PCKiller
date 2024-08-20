# ***Welcome to PCKiller***



## Introduction

Welcome to PCKiller project, this program is written in C++. <br>
PCKiller is a Windows kernel driver that schedules a forced system shutdown after a specified time. <br>
It also provides functionality to cancel the shutdown or check the remaining time before the system shuts down. <br>
A user-mode application is provided to interact with the driver, allowing you to control the shutdown process via commands sent to the kernel driver using IOCTL. <br><br>


**Warning**

The techniques demonstrated by this project are powerful and can be misused if applied maliciously. This tool is provided with the intention of advancing knowledge and should only be used in ethical hacking scenarios where explicit permission has been obtained. Misuse of this software can result in significant harm and legal consequences. By using this software, you agree to do so responsibly, ethically, and within legal boundaries.

<br><br>




## Service Control Manager (SCM):

Like any other service in Windows, the kernel drivers are managed like services using SCM. <br>

In order to work with services in Windows OS, to create a service and start, stop or delete it you need to get familiar a little bit with Service Control Manager(SCM). <br>

The Service Control Manager (SCM) is a special system process that manages all services on a Windows machine. It handles the following:

1. Starting and stopping services.
2. Sending control requests to services (pause, resume, stop).
3. Maintaining the status of each service.

The `sc` command-line tool is used to communicate with the SCM to install, start, stop, and delete services.

<br><br>




## Key Components

1. **Kernel Driver:** Implements the core functionality for scheduling and managing a forced system shutdown.

2. **User-Mode Application:** Provides a command-line interface to communicate with the kernel driver to schedule, cancel, or check the status of the shutdown.

3. **Shutdown Timer:** Uses a kernel timer and DPC (Deferred Procedure Call) to schedule a delayed shutdown.
   
4. **IOCTL Interface:** Handles user commands for shutdown scheduling, cancellation, and status queries.

<br><br>




## Features

1. **pckiller:** it has a cpp file called [pckiller.cpp](https://github.com/eliyaballout/PCKiller/blob/main/pckiller/pckiller/pckiller.cpp), Main kernel driver implementation.
   
2. **pckillerConsole:** it has a cpp file called [pckillerConsole.cpp](https://github.com/eliyaballout/PCKiller/blob/main/pckillerConsole/pckillerConsole/pckillerConsole.cpp), User-mode application to send commands to the kernel driver.

<br><br>




## Requirements, Installation & Usage

**I will explain here the requirements, installation and the usage of this PCKiller:** <br>

**Requirements:**
1. Ensure you have a C++ compiler, Windows SDK, and WDK (Windows Driver Kit) installed.

2. **Enable Test Mode on Windows:** <br>
    By default, Windows only allows loading of signed drivers to ensure the integrity and authenticity of the drivers being loaded. Enabling Test Mode bypasses this restriction, allowing you to test and debug your driver without needing a digital signature. <br>
    So before installing and running the MiniAV driver, you need to enable Test Mode on Windows. This allows you to load unsigned drivers, which is necessary for development and testing purposes.
    1. **Open Command Prompt(cmd) as Administrator**
   
    2. **Enable Test Mode:**
        ```
        bcdedit /set testsigning on
        ```

    3. **Restart Your Computer:** Restart your computer to apply the changes.
   
    4. **Verify Test Mode:** After restarting, you should see "Test Mode" displayed in the bottom-right corner of your desktop.

<br><br>


**Installation:**
1. Download and extract the [ZIP file](https://github.com/eliyaballout/PCKiller/archive/refs/heads/main.zip).<br>
2. Navigate to **pckiller --> x64 --> Debug**, you will find the `pckiller.sys` file, this is a kernel driver that you need to install on your computer.
3. Navigate to **pckillerConsole --> x64 --> Debug**, you will find the `pckillerConsole.exe` file, this is the executable file that you need to run in order to run the pckiller.

<br><br>


**Usage:**

**Make sure you run the executable in cmd with administrator privileges (Run as Administrator)** <br>

**Creating or installing the kernel driver:**

```
sc create pckiller type= kernel binPath= "C:\Path\To\pckiller.sys"
```
where `"C:\Path\To\pckiller.sys"` should be the full path of the kernel driver (which is located in **pckiller --> x64 --> Debug**).

<br>


**Starting the driver:** <br>
```
sc start pckiller
```
<br>


**Schedule a Shutdown:**
```
pckillerConsole.exe -shutdown <n>
```
where `n` represents the number of seconds before the shutdown is scheduled.

<br>


**Check Remaining Time:**
```
pckillerConsole.exe -remaining
```
<br>


**Cancel the Shutdown:**
```
pckillerConsole.exe -cancel
```

<br><br>



**You also can stop and even delete the kernel driver:**

**Stop:**
```
sc stop pckiller
```
<br>


**delete:**
**Make sure you have stopped the driver before deleting it.**
```
sc delete pckiller
```
<br>



## Ethical Considerations

This tool is intended for educational use only to demonstrate techniques commonly used by forced system shutdown. It should be used in a controlled environment, such as a penetration testing lab, where explicit permission has been granted. Always practice responsible disclosure and use ethical hacking principles.<br><br>




## Technologies Used
<img src="https://github.com/devicons/devicon/blob/master/icons/c/c-original.svg" title="c" alt="c" width="40" height="40"/>&nbsp;
<img src="https://github.com/devicons/devicon/blob/master/icons/cplusplus/cplusplus-original.svg" title="c++" alt="c++" width="40" height="40"/>&nbsp;
<br><br><br>




## Demonstration of the antivirus



<br>
