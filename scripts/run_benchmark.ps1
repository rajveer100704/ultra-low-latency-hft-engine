# Build in Release mode
Write-Host "Building project in Release mode..."
cmake --build build --config Release

# Run benchmark
Write-Host "Running HFT benchmark..."

$exe = "build\Release\hft_main.exe"
$data = "market_data_record.csv"

if (!(Test-Path $data)) {
    Write-Host "ERROR: Market data file not found!"
    exit
}

$start = Get-Date

& $exe --replay $data --speed 20

$end = Get-Date
$duration = $end - $start

Write-Host "--------------------------------------"
Write-Host "Benchmark Completed"
Write-Host "Total Time: $($duration.TotalSeconds) seconds"
Write-Host "--------------------------------------"
