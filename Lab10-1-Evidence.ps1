$Host.UI.RawUI.WindowTitle = "Lab 10.1 - mDNS and HTTP REST - 67030142"
Write-Host "LAB 10.1 | 67030142 Piyawit Pholsri | NeennyP-2.4G" -ForegroundColor Cyan
Write-Host "`n[1] mDNS Resolution and Ping" -ForegroundColor Yellow
ping -4 esp32-node.local -n 2
Write-Host "`n[2] HTTP GET /api/status" -ForegroundColor Yellow
curl.exe -s -i http://esp32-node.local/api/status
Write-Host "`n[3] HTTP POST /api/led -> ON" -ForegroundColor Yellow
Invoke-RestMethod -Method Post -Uri http://esp32-node.local/api/led -ContentType 'application/json' -Body '{"state":true}' | ConvertTo-Json -Compress
Write-Host "`n[4] HTTP POST /api/led -> OFF" -ForegroundColor Yellow
Invoke-RestMethod -Method Post -Uri http://esp32-node.local/api/led -ContentType 'application/json' -Body '{"state":false}' | ConvertTo-Json -Compress
Write-Host "`nHardware test complete on 192.168.1.33" -ForegroundColor Green
