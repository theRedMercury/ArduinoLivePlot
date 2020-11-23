using OpenHardwareMonitor.Hardware;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Drawing;
using System.Globalization;
using System.IO.Ports;
using System.Threading;
using System.Windows.Forms;

/*
 * Main Application Tray : Send over serial, Data for arduino plot
 * 
 * Check "param.ini" for set good Serial port
 * 
 */
namespace HardwareMonitorWinIno
{

    class MainMonitor : ApplicationContext
    {
        private static readonly string[] CPU_LOADS = { "/amdcpu/0/load/1", "/amdcpu/0/load/2", "/amdcpu/0/load/3",
                                                      "/amdcpu/0/load/4", "/amdcpu/0/load/5", "/amdcpu/0/load/6" };
        private static readonly string CPU_TEMP = "/amdcpu/0/temperature/0";
        private static readonly string CPU_PWR = "/amdcpu/0/power/0";

        private static readonly string CPU_VCORE = "/lpc/nct6797d/voltage/0";
        private static readonly string CPU_V3 = "/lpc/nct6797d/voltage/2";
        private static readonly string RAM_LOAD = "/ram/load/0";

        private static readonly string GPU_LOAD = "/atigpu/0/load/0";
        private static readonly string GPU_TEMP = "/atigpu/0/temperature/0";
        private static readonly string GPU_PWR = "/atigpu/0/power/0";

        private static readonly string CPU_FAN = "/lpc/nct6797d/fan/1";
        private static readonly string FAN3 = "/lpc/nct6797d/fan/3";
        private static readonly string FAN4 = "/lpc/nct6797d/fan/4";
        private static readonly string GPU_FAN = "/atigpu/0/fan/0";

        private static readonly string NETWORK_INT = "Network Interface";
        private static readonly string NETWORK_BRS = "Bytes Received/sec";
        private static readonly string NETWORK_BSS = "Bytes Sent/sec";

        private readonly Thread mainThread;
        private bool isRunningThread = true;
        private readonly Int32 threadUpdateSleep = 500;

        private readonly SerialPort serialP = null;
        private readonly Int32 baudRate = 115200;
        private readonly Mutex serialMutex = new Mutex();

        private readonly NotifyIcon mainIcon = new NotifyIcon();

        private readonly HardwareMonitorReader hmReader = new HardwareMonitorReader();
        private readonly List<PerformanceCounter> netCounterR = new List<PerformanceCounter>();
        private readonly List<PerformanceCounter> netCounterS = new List<PerformanceCounter>();
  

        public MainMonitor()
        {
            this.mainIcon.Icon = Icon.FromHandle(Resource.on.GetHicon());
            this.mainIcon.Text = "Monitor on";
            this.mainIcon.Visible = true;
            //this.mainIcon.BalloonTipTitle = "Monitor";
            //this.mainIcon.BalloonTipText = "Hello";
            //this.mainIcon.ShowBalloonTip(1000);

            this.mainIcon.ContextMenuStrip = new ContextMenuStrip();
            this.mainIcon.ContextMenuStrip.Items.Add("Pause", null, this.PauseThread);
            this.mainIcon.ContextMenuStrip.Items.Add("Exit", null, this.ExitApp);

            try
            {
                foreach (string name in new PerformanceCounterCategory(NETWORK_INT).GetInstanceNames())
                {
                    this.netCounterR.Add(new PerformanceCounter(NETWORK_INT, NETWORK_BRS, name));
                    netCounterS.Add(new PerformanceCounter(NETWORK_INT, NETWORK_BSS, name));
                }
            }
            catch
            {
                this.mainIcon.Icon = Icon.FromHandle(Resource.off.GetHicon());
                this.mainIcon.Text = "Monitor error : check Network Interface";
            }

            try
            {
                this.serialP = new SerialPort(System.IO.File.ReadAllText(@"param.ini"), baudRate, Parity.None, 8, StopBits.One);
                this.serialP.Close();
                this.OpenPort();
            }
            catch
            {
                this.serialP = null;
                this.mainIcon.Icon = Icon.FromHandle(Resource.off.GetHicon());
                this.mainIcon.Text = "Monitor error : check Serial config";
            }


            if (this.serialP != null)
            {
                this.mainThread = new Thread(new ThreadStart(this.RunThread));
                this.mainThread.Start();
            }
        }

        private void PauseThread(object sender, EventArgs e)
        {
            this.serialMutex.WaitOne();
            try
            {
                if (this.serialP.IsOpen)
                {
                    this.serialP.Close();
                    this.mainIcon.ContextMenuStrip.Items[0].Text = "Resume";
                }
                else
                {
                    this.serialP.Open();
                    this.mainIcon.ContextMenuStrip.Items[0].Text = "Pause";
                }
                this.mainIcon.Text = "Monitor on";
            }
            catch
            {
                this.mainIcon.Text = "Monitor error : Serial Port";
            }

            this.serialMutex.ReleaseMutex();
        }
        private void RunThread()
        {
            while (this.isRunningThread)
            {
                IReadOnlyDictionary<string, double> dictInfo = this.hmReader.GetAllData();

                double cpuTotal = 0;
                foreach (string cpuP in CPU_LOADS)
                {
                    cpuTotal += dictInfo[cpuP];
                }
                cpuTotal /= Convert.ToDouble(CPU_LOADS.Length);

                this.SendSerialMsg("cpu:" + cpuTotal.ToString(CultureInfo.InvariantCulture) + ";" +
                                    dictInfo[CPU_TEMP].ToString(CultureInfo.InvariantCulture) + ";" +
                                    dictInfo[CPU_PWR].ToString(CultureInfo.InvariantCulture) + ";" +
                                    dictInfo[CPU_VCORE].ToString(CultureInfo.InvariantCulture) + ";" +
                                    dictInfo[CPU_V3].ToString(CultureInfo.InvariantCulture));  //Load Totat - Temp - Power - VCore -3VCC

                this.SendSerialMsg("mem:" + dictInfo[RAM_LOAD].ToString(CultureInfo.InvariantCulture));  // Memory

                this.SendSerialMsg("gpu:" + dictInfo[GPU_LOAD].ToString(CultureInfo.InvariantCulture) + ";" +
                                    dictInfo[GPU_TEMP].ToString(CultureInfo.InvariantCulture) + "; " +
                                    dictInfo[GPU_PWR].ToString(CultureInfo.InvariantCulture));  // Load -  Temp - Power

                this.SendSerialMsg("net:" + this.GetTotalValueCounter(this.netCounterR) + ";" + this.GetTotalValueCounter(this.netCounterS));

                this.SendSerialMsg("fan:" + dictInfo[CPU_FAN] + ";" + dictInfo[FAN3] + ";" + dictInfo[FAN4] + ";" + dictInfo[GPU_FAN]);

                Thread.Sleep(this.threadUpdateSleep);
            }
        }

        private int GetTotalValueCounter(List<PerformanceCounter> listCounter)
        {
            float fTotal = 0f;
            foreach (PerformanceCounter pf in listCounter)
            {
                fTotal += pf.NextValue();
            }
            return (int)Math.Round(fTotal / 1024f, 0);
        }

        private void SendSerialMsg(string msg)
        {
            this.serialMutex.WaitOne();
            if (this.serialP != null && this.serialP.IsOpen)
            {
                msg += ";";
                Console.WriteLine(msg);
                bool send = false;
                while (!send)
                {
                    try
                    {
                        this.serialP.Write(msg);
                        this.serialP.Write("\n");
                        send = true;
                        Thread.Sleep(250);
                    }
                    catch
                    {
                        this.OpenPort();
                    }
                }
            }
            this.serialMutex.ReleaseMutex();
        }

        private void OpenPort()
        {
            if (this.serialP != null)
            {
                while (!this.serialP.IsOpen)
                {
                    this.serialP.Open();
                    this.serialP.Write("\n");
                }
            }
        }


        private void ExitApp(object sender, EventArgs e)
        {
            this.isRunningThread = false;
            if (this.serialP != null)
            {
                this.serialP.Close();
            }
            if (this.mainThread != null)
            {
                this.mainThread.Abort();
            }
            this.hmReader.Dispose();

            this.mainIcon.Visible = false;
            Application.Exit();
        }
    }

    internal sealed class HardwareMonitorReader : IDisposable
    {
        private readonly Computer _computer;

        public HardwareMonitorReader()
        {
            _computer = new Computer { CPUEnabled = true, GPUEnabled = true, RAMEnabled = true, FanControllerEnabled = true, MainboardEnabled = true };
            _computer.Open();
        }

        public IReadOnlyDictionary<string, double> GetAllData()
        {
            Dictionary<string, double> coreAndTemperature = new Dictionary<string, double>();
            foreach (IHardware hardware in _computer.Hardware)
            {
                hardware.Update(); //use hardware.Name to get CPU model
                foreach (var sensor in hardware.Sensors)
                {
                    if (sensor.Value.HasValue)
                    {
                        // Console.WriteLine(sensor.Name + sensor.SensorType);
                        coreAndTemperature.Add(sensor.Identifier.ToString(), Math.Round(sensor.Value.Value, 2));
                    }
                }
                if (hardware.SubHardware.Length > 0)
                {
                    hardware.SubHardware[0].Update();
                    foreach (var sensor in hardware.SubHardware[0].Sensors)
                    {
                        if (sensor.Value.HasValue)
                        {
                            // Console.WriteLine(sensor.Name + sensor.SensorType);
                            coreAndTemperature.Add(sensor.Identifier.ToString(), Math.Round(sensor.Value.Value, 3));
                        }
                    }
                }
            }
            return coreAndTemperature;
        }

        public void Dispose()
        {
            try
            {
                _computer.Close();
            }
            catch (Exception)
            {
                //ignore closing errors
            }
        }
    }
}
