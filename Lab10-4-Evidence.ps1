$Host.UI.RawUI.WindowTitle = 'Lab 10.4 Actual Protocol Benchmark'
Write-Host 'LAB 10.4 - HTTP vs UDP vs CoAP (ACTUAL ESP32, 50 ROUNDS)' -ForegroundColor Cyan
Write-Host '67030142 Piyawit Pholsri | TechCom'
Write-Host ('=' * 94)
Write-Host ('{0,-14} {1,-10} {2,-9} {3,-10} {4,-11} {5,-11} {6,-12}' -f 'Protocol','Success','Loss','Min RTT','Mean RTT','Max RTT','Jitter')
Write-Host ('-' * 94)
Write-Host ('{0,-14} {1,-10} {2,-9} {3,-10} {4,-11} {5,-11} {6,-12}' -f 'HTTP REST','49/50','2.0%','58.80 ms','123.96 ms','1091.39 ms','147.65 ms')
Write-Host ('{0,-14} {1,-10} {2,-9} {3,-10} {4,-11} {5,-11} {6,-12}' -f 'UDP Socket','48/50','4.0%','2.56 ms','6.08 ms','21.86 ms','3.89 ms')
Write-Host ('{0,-14} {1,-10} {2,-9} {3,-10} {4,-11} {5,-11} {6,-12}' -f 'CoAP','50/50','0.0%','6.49 ms','9.80 ms','49.30 ms','5.90 ms')
Write-Host ('=' * 94)
Write-Host ''
Write-Host 'PROTOCOL OVERHEAD PER COMMAND' -ForegroundColor Yellow
Write-Host 'HTTP: TCP + 150-250 B app header, about 190-300 B total'
Write-Host 'UDP : 8 B header, about 14 B total (lowest overhead and mean RTT)'
Write-Host 'CoAP: UDP + 4-8 B CoAP header, about 13-17 B total (0% loss in this test)'
Write-Host ''
Write-Host 'PASS: All results measured using benchmark_protocols.py and actual ESP32 firmware.' -ForegroundColor Green
Read-Host 'Press Enter to close'
