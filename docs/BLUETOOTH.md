# Bluetooth

## Conexión A2DP
```bash
bluetoothctl
scan on
pair AC:EF:92:D0:B5:BB
trust AC:EF:92:D0:B5:BB
connect AC:EF:92:D0:B5:BB
```

## Troubleshooting
### br-connection-adapter-not-powered
El adaptador está bloqueado por RF-kill:
```bash
sudo rfkill unblock bluetooth
sudo hciconfig hci0 up
bluetoothctl power on
```
