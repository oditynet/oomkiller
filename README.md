# oomkiller
Simple oom killer of bad process :) .
Scan mem every 50 ms and kill process at signal( -9 )

```
g++ oomkiller.cpp -o oomkiller
sudo cp oomkiller /usr/bin
sudo cp oomkilld.service /usr/lib/systemd/system/
sudo systemctl daemon-reload
sudo systemctl enable oomkilld.service
sudo systemctl start oomkilld.service
```

Log for example:
```
 12:25:08  odity@viva  ~/eclipse-workspace/argv  sudo systemctl status oomkilld.service
● oomkilld.service - Simple OOM Daemon
     Loaded: loaded (/usr/lib/systemd/system/oomkilld.service; enabled; preset: disabled)
     Active: active (running) since Tue 2024-07-30 12:24:56 MSK; 12s ago
 Invocation: c0df0451b2b44ca39c190b49b58e3ca5
       Docs: https://github.com/oditynet/oomkill
   Main PID: 11575 (oomkiller)
      Tasks: 1 (limit: 28622)
     Memory: 1.8M (peak: 2M)
        CPU: 43ms
     CGroup: /system.slice/oomkilld.service
             └─11575 /usr/bin/oomkiller

июл 30 12:24:56 viva systemd[1]: Started Simple OOM Daemon.
июл 30 12:25:05 viva oomkiller[11575]: Alarm: memory 595384Mb
июл 30 12:25:05 viva oomkiller[11575]: Potential: 320 -250 500 10840
июл 30 12:25:05 viva oomkiller[11575]: Potential: 2643 0 666 1263
июл 30 12:25:05 viva oomkiller[11575]: Potential: 2647 0 667 20723
июл 30 12:25:05 viva oomkiller[11575]: Potential: 3080 0 673 147885
июл 30 12:25:05 viva oomkiller[11575]: Potential: 11602 0 879 4703448
июл 30 12:25:05 viva oomkiller[11575]: Error read dir: Success
июл 30 12:25:05 viva oomkiller[11575]: Kill process (mem) of pid 11602
```
