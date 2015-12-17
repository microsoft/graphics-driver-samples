How to run this test.

1: Establish connection to target device using powershell or SSH.

    Power shell: http://ms-iot.github.io/content/en-US/win10/samples/PowerShell.htm
    SSH:         http://ms-iot.github.io/content/en-US/win10/samples/SSH.htm

2: Copy CubeTest.exe and VC4Test-Cube.fx to target device and place them at same directory, for example, c:\temp.
   You can copy using Windows Explorer or command prompt using IP address, such as \\192.168.1.50\c$.
   (assuming 192.168.1.50 is the IP address of target device).

    FTP: http://ms-iot.github.io/content/en-US/win10/samples/FTP.htm

3: Run CubeTest.exe from powershell or SSH command prompt. 
   Or if you need to debug application. do below.
   3.1: At target device, run "start mwdbgsrv -t tcp:port=60000,IcfEnable" from powershell or SSH.
        This will start remote debugger service on target device.
   3.2: At PC, run "windbg.exe -premote tcp:port=60000,server=192.168.1.50 C:\temp\cubetest.exe"
        (assuming 192.168.1.50 is the IP address of target device).

4: Result is saved at c:\temp\image.bmp
