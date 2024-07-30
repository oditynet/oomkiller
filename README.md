# oomkiller
Simple oom killer of bad process :) .
Scan mem every 50 ms
Kill process -9

```
g++ oomkiller.cpp -o oomkiller
sudo cp oomkiller /usr/bin
sudo cp oomkilld.service /usr/lib/systemd/system/
sudo systemctl daemon-reload
sudo systemctl enable oomkilld.service
sudo systemctl start oomkilld.service
```
